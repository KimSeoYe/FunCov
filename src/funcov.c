#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "../include/funcov.h"
#include "../include/shm_coverage.h"
#include "../include/get_coverage.h"
#include "../include/translate_addr.h"

// #define SAVE_MAP

#define STDOUT_FD 1
#define STDERR_FD 2

#define OUTDIR "out"
#define ERRDIR "err"
#define FUNDIR "covered_funs"
#define BITDIR "bitmaps"    // TODO. change name >> hitmaps

#define INPUT_CNT_UNIT 512

static config_t conf ;
static cov_stat_t * cov_stats ; // save
static unsigned int * trace_cov ; 
map_elem_t trace_map[MAP_ROW_UNIT][MAP_COL_UNIT] ;
static cov_stat_t * curr_stat ; // shm
static int curr_stat_shmid ;

/**
 * usage: ./funcov -i [input_dir] -o [output_dir] -x [executable_binary] ...
 * 
 * required
 * -i : input directory path
 * -o : output directory path
 * -x : executable binary path
 * 
 * optional
 * @@ : input type - file as an argument (default: stdin)
*/

void
get_cmd_args (int argc, char * argv[])
{
    int i_flag = 0, o_flag = 0, x_flag = 0 ;
    int arg_cnt = 1 ;

    int opt ;
    while ((opt = getopt(argc, argv, "i:o:x:")) != -1) {
        switch(opt) {
        case 'i':
            if (realpath(optarg, conf.input_dir_path) == 0x0) {
                perror("get_cmd_args: realpath: Invalid input directory") ;
                exit(1) ;
            }
            i_flag = 1 ;
            arg_cnt += 2 ;
            break ;

        case 'o':   
            if (access(optarg, F_OK) == -1) {
                if (mkdir(optarg, 0777) == -1) {
                    perror("get_cmd_args: mkdir: Failed to make an output directory") ;
                    exit(1) ;
                }
            }
            if (realpath(optarg, conf.output_dir_path) == 0x0) {
                perror("get_cmd_args: realpath: Invalid output directory") ;
                exit(1) ;
            }
            o_flag = 1 ;
            arg_cnt += 2 ;
            break ;

        case 'x':
            if (realpath(optarg, conf.binary_path) == 0x0) {
                perror("get_cmd_args: realpath: Invalid executable binary") ;
                exit(1) ;
            }
            if (access(optarg, X_OK) == -1) {
                perror("get_cmd_args: access: The target is not executable") ;
                exit(1) ;
            }
            x_flag = 1 ;
            arg_cnt += 2 ;
            break ;
        }
    }

    if (!i_flag || !o_flag || !x_flag) goto print_usage ;

    if (argc > arg_cnt) { 
        if (strcmp(argv[arg_cnt], "@@") == 0) {
            conf.input_type = ARG_FILENAME ;
            arg_cnt++ ;
        }
        else goto print_usage ;
    } 
    else {
        conf.input_type = STDIN ;
    }

    return ;

print_usage:
    perror("\nusage: ./funcov -i [input_dir] -x [executable_binary] ...\n\nrequired\n-i : input directory path\n-o : output directory path\n-x : executable binary path\n\noptional\n@@ : input type - file path as an argument (default: stdin)\n\n") ;
    exit(1) ;
}

void
set_output_dir ()
{
    char stdout_path[PATH_MAX + 4] ;
    sprintf(stdout_path, "%s/%s", conf.output_dir_path, OUTDIR) ;
    if (access(stdout_path, F_OK) == -1) {
        if (mkdir(stdout_path, 0777) == -1) goto mkdir_err ;
    }

    char stderr_path[PATH_MAX + 4] ;
    sprintf(stderr_path, "%s/%s", conf.output_dir_path, ERRDIR) ;
    if (access(stderr_path, F_OK) == -1) {
        if (mkdir(stderr_path, 0777) == -1) goto mkdir_err ;
    }

#ifdef SAVE_MAP 
    char bitmap_path[PATH_MAX + 32] ;
    sprintf(bitmap_path, "%s/%s", conf.output_dir_path, BITDIR) ;
    if (access(bitmap_path, F_OK) == -1) {
        if (mkdir(bitmap_path, 0777) == -1) goto mkdir_err ;
    }
#endif

    char fundir_path[PATH_MAX + 32] ;
    sprintf(fundir_path, "%s/%s", conf.output_dir_path, FUNDIR) ;
    if (access(fundir_path, F_OK) == -1) {
        if (mkdir(fundir_path, 0777) == -1) goto mkdir_err ;
    }

    return ;

mkdir_err:
    perror("set_output_dir: mkdir: Failed to set an output directory") ;
    exit(1) ;
}

void
read_input_dir ()
{
    int file_cnt = 0 ;

    DIR * dir_ptr = 0x0 ;
    struct dirent * entry = 0x0 ;

    if ((dir_ptr = opendir(conf.input_dir_path)) == 0x0) {
        perror("read_input_dir: opendir") ;
        exit(1) ;
    }

    conf.input_files = (char **) malloc(sizeof(char *) * INPUT_CNT_UNIT) ;

    while ((entry = readdir(dir_ptr)) != 0x0) {
        if (file_cnt != 0x0 && file_cnt % INPUT_CNT_UNIT == 0) {
            conf.input_files = realloc(conf.input_files, sizeof(char *) * (file_cnt / INPUT_CNT_UNIT + 1) * INPUT_CNT_UNIT) ;
            if (conf.input_files == 0x0) {
                perror("read_input_dir: realloc") ;
                exit(1) ;
            }
        }
        if (entry->d_name[0] != '.') {
            conf.input_files[file_cnt] = (char *) malloc(sizeof(char) * (strlen(conf.input_dir_path) + strlen(entry->d_name) + 2)) ;
            sprintf(conf.input_files[file_cnt], "%s/%s", conf.input_dir_path, entry->d_name) ;
            file_cnt++ ;
        }
    }
    closedir(dir_ptr) ;

    conf.input_file_cnt = file_cnt ;
}

void
print_config ()
{
    printf("\nFUNCOV ARGS\n") ;
    printf("* EXECUTABLE BINARY: %s\n", conf.binary_path) ;
    printf("* INPUT TYPE: ") ;
    switch (conf.input_type) {
    case STDIN:
        printf("stdin\n") ;
        break;
    case ARG_FILENAME:
        printf("file as an argument\n") ;
        break;
    }
    printf("* OUTPUT DIR PATH: %s\n", conf.output_dir_path) ;
    printf("* INPUT DIR PATH: %s\n", conf.input_dir_path) ;
    printf("* INPUT FILE CNT: %d\n", conf.input_file_cnt) ;
    printf("* INPUT FILES\n") ;
    for (int i = 0; i < conf.input_file_cnt; i++) {
        printf("  [%d] %s\n", i, conf.input_files[i]) ;
    }
    printf("\n") ;
}

void
remove_shared_mem ()
{
    detatch_shm((void *)curr_stat) ;
    remove_shm(curr_stat_shmid) ;
}

void
shm_init ()
{
    curr_stat_shmid = get_shm(CURR_KEY, sizeof(cov_stat_t)) ;
    curr_stat = attatch_shm(curr_stat_shmid) ;
    memset(curr_stat, 0, sizeof(cov_stat_t)) ;
}

void 
funcov_init (int argc, char * argv[]) 
{
    get_cmd_args(argc, argv) ;  
    set_output_dir() ;
    read_input_dir() ;
    print_config() ;

    cov_stats = (cov_stat_t *) malloc(sizeof(cov_stat_t) * conf.input_file_cnt) ;
    for (int i = 0; i < conf.input_file_cnt; i++) {
        cov_stats[i].fun_coverage = 0 ;
        memset(cov_stats[i].map, 0, sizeof(map_elem_t) * MAP_SIZE) ;
    }

    trace_cov = (unsigned int *) malloc(sizeof(unsigned int) * conf.input_file_cnt) ;
    memset(trace_cov, 0, sizeof(unsigned int) * conf.input_file_cnt) ;
    memset(trace_map, 0, sizeof(map_elem_t) * MAP_SIZE) ;
    
    shm_init() ;
}


static int stdin_pipe[2] ;
static int stdout_pipe[2] ;
static int stderr_pipe[2] ;

static int child_pid ;

void
timeout_handler (int sig)
{
    if (sig == SIGALRM) {
        perror("timeout") ;
        if (kill(child_pid, SIGINT) == -1) {
            perror("timeout_handler: kill") ;
            remove_shared_mem() ;
            exit(1) ;
        }
    }
}

void
execute_target (int turn)
{
    alarm(3) ;

    FILE * fp = fopen(conf.input_files[turn], "rb") ;
    if (fp == 0x0) {
        perror("execute_target: fopen") ;
        remove_shared_mem() ;
        exit(1) ;
    }

    if (conf.input_type == STDIN) {
        while (!feof(fp)) {
            char buf[BUF_SIZE] ;
            int r_len = fread(buf, 1, sizeof(buf), fp) ;

            char * buf_p = buf ;
            int s ;
            while (r_len > 0 && (s = write(stdin_pipe[1], buf_p, r_len)) > 0) {
                buf_p += s ;
                r_len -= s ;
            }
        }
    }
    fclose(fp) ;

    close(stdin_pipe[1]) ;

    dup2(stdin_pipe[0], 0) ;
    
    close(stdin_pipe[0]) ;
    close(stdout_pipe[0]) ;
    close(stderr_pipe[0]) ;

    dup2(stdout_pipe[1], 1) ;
    dup2(stderr_pipe[1], 2) ;

    chdir(conf.output_dir_path) ;

    // TODO. ASAN_OPTION

    if (conf.input_type == STDIN) {
        char * args[] = { conf.binary_path, (char *)0x0 } ;
        if (execv(conf.binary_path, args) == -1) {
            perror("execute_target: execv") ;
            remove_shared_mem() ;
            exit(1) ;
        }
    } 
    else if (conf.input_type == ARG_FILENAME) {
        char * args[] = { conf.binary_path, conf.input_files[turn], (char *)0x0 } ;
        if (execv(conf.binary_path, args) == -1) {
            perror("execute_target: execv") ;
            remove_shared_mem() ;
            exit(1) ;
        }
    }
}

void
get_result_file_path (char * path, int turn, int fd)
{
    char input_filename[BUF_SIZE] ;
    strcpy(input_filename, conf.input_files[turn] + strlen(conf.input_dir_path) + 1) ;

    switch (fd) {
    case STDOUT_FD:
        sprintf(path, "%s/%s/%s", conf.output_dir_path, OUTDIR, input_filename) ;
        break;
    
    case STDERR_FD:
        sprintf(path, "%s/%s/%s", conf.output_dir_path, ERRDIR, input_filename) ;
        break;
    }
}

void
write_out_file (int turn, int fd)
{
    char path[PATH_MAX] ;
    get_result_file_path(path, turn, fd) ;
    
    FILE * fp = fopen(path, "wb") ;
    if (fp == 0x0) {
        perror("write_out_file: fopen") ;
        remove_shared_mem() ;
        exit(1) ;
    }

    char buf[BUF_SIZE] ;
    int s = 0 ;

    if (fd == STDOUT_FD) {
        while ((s = read(stdout_pipe[0], buf, BUF_SIZE)) > 0) {
            if (fwrite(buf, 1, s, fp) != s) {
                perror("write_out_file: fwrite: stdout") ;
            }
        }
        close(stdout_pipe[0]) ;
    }
    else if (fd == STDERR_FD) {
        while ((s = read(stderr_pipe[0], buf, BUF_SIZE)) > 0) {
            if (fwrite(buf, 1, s, fp) != s) {
                perror("write_out_file: fwrite: stderr") ;
            }
        }
        close(stderr_pipe[0]) ;
    }
}

void
save_results (int turn)
{
    close(stdin_pipe[0]) ;
    close(stdin_pipe[1]) ;
    close(stdout_pipe[1]) ;
    close(stderr_pipe[1]) ;

    write_out_file(turn, STDOUT_FD) ; // Q. need?
    write_out_file(turn, STDERR_FD) ;
}

int
run (int turn)
{
    memset(curr_stat, 0, sizeof(cov_stat_t)) ;

    if (pipe(stdin_pipe) != 0) goto pipe_err ;
    if (pipe(stdout_pipe) != 0) goto pipe_err ;
    if (pipe(stderr_pipe) != 0) goto pipe_err ;

    child_pid = fork() ; 

    if (child_pid == 0) {
        execute_target(turn) ;
    }
    else if (child_pid > 0) {
        save_results(turn) ;
    }
    else {
        perror("run: fork") ;
        exit(1) ;
    }

    int exit_code ;
    wait(&exit_code) ;

    return exit_code ;

pipe_err:
    perror("run: pipe") ;
    remove_shared_mem() ;
    exit(1) ;
}

void
write_log_csv (char * cov_log_path, char * trace_cov_path)
{
    printf("WRITE %s for...\n", cov_log_path) ;
    FILE * fp = fopen(cov_log_path, "wb") ;
    if (fp == 0x0) {
        perror("write_log_csv: fopen") ;
        remove_shared_mem() ;
        exit(1) ;
    }
    fprintf(fp, "id,fun_cov,exit_code,filename\n") ;
    for (int i = 0; i < conf.input_file_cnt; i++) {
        printf("* [%d] %s\n", i, conf.input_files[i]) ;
        fprintf(fp, "%d,%d,%d,\"%s\"\n", cov_stats[i].id, cov_stats[i].fun_coverage, cov_stats[i].exit_code, conf.input_files[i]) ;
    }
    fclose(fp) ;
    printf("\n") ;

    printf("WRITE %s for...\n", trace_cov_path) ;
    fp = fopen(trace_cov_path, "wb") ;
    if (fp == 0x0) {
        perror("write_log_csv: fopen") ;
        remove_shared_mem() ;
        exit(1) ;
    }
    fprintf(fp, "id,accumulated_cov\n") ;
    for (int i = 0; i < conf.input_file_cnt; i++) {
        printf("* [%d] %s\n", i, conf.input_files[i]) ;
        fprintf(fp, "%d,%d\n", cov_stats[i].id, trace_cov[i]) ;
    }
    fclose(fp) ;
    printf("\n") ;
}

#ifdef SAVE_MAP 
void
write_result_maps(char * bitmaps_dir_path) 
{
    for (int turn = 0; turn < conf.input_file_cnt; turn++) {
        char input_filename[PATH_MAX] ;
        strcpy(input_filename, conf.input_files[turn] + strlen(conf.input_dir_path) + 1) ;
        
        char bitmap_file_path[PATH_MAX + 32] ;
        sprintf(bitmap_file_path, "%s/%s", bitmaps_dir_path, input_filename) ;

        FILE * fp = fopen(bitmap_file_path, "wb") ;
        if (fp == 0x0) {
            perror("write_result_maps: fopen") ;
            remove_shared_mem() ;
            exit(1) ;
        }

        size_t s = fwrite(&cov_stats[turn].shm_map, 1, sizeof(shm_map_t), fp) ;
        if (s != sizeof(shm_map_t)) {
            perror("write_result_maps: fwrite") ;
        }

        fclose(fp) ;
    }
}
#endif

void
write_covered_funs(char * funcov_dir_path, location_t * translated_locations) 
{
    printf("WRITE %s for...\n", funcov_dir_path) ;

    for (int turn = 0; turn < conf.input_file_cnt; turn++) {
        printf("* [%d] %s\n", turn, conf.input_files[turn]) ;

        char input_filename[PATH_MAX] ;
        strcpy(input_filename, conf.input_files[turn] + strlen(conf.input_dir_path) + 1) ;
        
        char funcov_file_path[PATH_MAX + 32] ;
        sprintf(funcov_file_path, "%s/%s.csv", funcov_dir_path, input_filename) ;

        FILE * fp = fopen(funcov_file_path, "wb") ;
        if (fp == 0x0) {
            perror("write_covered_funs: fopen") ;
            remove_shared_mem() ;
            exit(1) ;
        }

        fprintf(fp, "callee,caller,caller_line\n") ; // current : callee,caller:pc_val
        for (int i = 0; i < MAP_ROW_UNIT; i++) {
            for (int j = 0; j < MAP_COL_UNIT; j++) {
                if (cov_stats[turn].map[i][j].hit_count == 0) break ;
                fprintf(fp, "%s\n", cov_stats[turn].map[i][j].cov_string) ; 
            }
        }   

        fclose(fp) ;
    }
    printf("\n") ;
}

void
print_summary (char * cov_log_path, char * trace_cov_path, char * funcov_dir_path)
{
    printf("RESULT SUMMARY\n") ;
    printf("* INITIAL COVERAGE: %d\n", trace_cov[0]) ;
    printf("* TOTAL COVERAGE: %d\n", trace_cov[conf.input_file_cnt - 1]) ;
    printf("* LOG SAVED IN %s\n", cov_log_path) ;
    printf("* ACCUMULATED LOG SAVED IN %s\n", trace_cov_path) ;
#ifdef SAVE_MAP 
    printf("* FUNCTION COVERAGE BITMAPS SAVED IN %s\n", bitmaps_dir_path) ;
#endif
    printf("* COVERED FUNTIONS PER INPUT SAVED IN %s\n", funcov_dir_path) ;
    printf("\n") ;
}

void
save_final_results ()
{
    char cov_log_path[PATH_MAX + 32] ;
    char trace_cov_path[PATH_MAX + 32] ;
    sprintf(cov_log_path, "%s/%s", conf.output_dir_path, "per_cov_log.csv") ;
    sprintf(trace_cov_path, "%s/%s", conf.output_dir_path, "trace_cov_log.csv") ;
    write_log_csv(cov_log_path, trace_cov_path) ;

    int locations_cnt = trace_cov[conf.input_file_cnt - 1] ;
    location_t * translated_locations = (location_t *) malloc(sizeof(location_t) * locations_cnt) ;
    if (translated_locations == 0x0) {
        perror("save_final_results: malloc") ;
        remove_shared_mem() ;
        exit(1) ;
    }
    translate_pc_values(translated_locations, locations_cnt, trace_map, conf.binary_path) ;

    char funcov_dir_path[PATH_MAX + 32] ;
    sprintf(funcov_dir_path, "%s/%s", conf.output_dir_path, FUNDIR) ;
    write_covered_funs(funcov_dir_path, translated_locations) ; // TODO. translated_locations

    free(translated_locations) ;

#ifdef SAVE_MAP
    char bitmaps_dir_path[PATH_MAX + 32] ;
    sprintf(bitmaps_dir_path, "%s/%s", conf.output_dir_path, BITDIR) ;
    write_result_maps(bitmaps_dir_path) ;
#endif
    
    print_summary(cov_log_path, trace_cov_path, funcov_dir_path) ;
}

void
funcov_destroy ()
{
    for (int i = 0; i < conf.input_file_cnt; i++) {
        free(conf.input_files[i]) ;
    }
    free(conf.input_files) ;
    free(cov_stats) ;
    free(trace_cov) ;

    remove_shared_mem() ;

    printf("WE ARE DONE!\n\n") ;
}

int
main (int argc, char * argv[])
{
    signal(SIGALRM, timeout_handler) ;

    funcov_init(argc, argv) ;

    printf("RUN\n") ;
    for (int turn = 0; turn < conf.input_file_cnt; turn++) {
        printf("* [%d] %s: ", turn, conf.input_files[turn]) ;
        
        int exit_code = run(turn) ;

        curr_stat->id = turn ;
        curr_stat->exit_code = exit_code ;
        curr_stat->fun_coverage = count_coverage(curr_stat->map) ;
        memcpy(&cov_stats[turn], curr_stat, sizeof(cov_stat_t)) ;

        trace_cov[turn] = get_trace_coverage(trace_map, curr_stat) ;
        
        printf("cov=%d, acc_cov=%d\n", cov_stats[turn].fun_coverage, trace_cov[turn]) ;
    }
    printf("\n") ;

    save_final_results() ;  

    funcov_destroy() ;

    return 0 ;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "../include/funcov.h"
#include "../include/get_coverage.h"

#define INPUT_CNT_UNIT 512

#define STDOUT_FD 1
#define STDERR_FD 2

static config_t conf ;
static cov_stat_t * cov_stats ;
static unsigned int * trace_cov ;
static trace_bits_t trace_bits ;

/**
 * usage: ./funcov -i [input_dir] -x [executable_binary] -w [pwd] ...
 * 
 * required
 * -i : input directory path
 * -o : output directory path
 * -x : executable binary path
 * -w : pwd
 * 
 * optional
 * @@ : input type - file as an argument
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

    if (argc > 7) {
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
    perror("\nusage: ./funcov -i [input_dir] -x [executable_binary] ...\n\nrequired\n-i : input directory path\n-o : output directory path\n-x : executable binary path\n\noptional\n@@ : input type - file as an argument\n\n") ;
    exit(1) ;
}

void
set_output_dir ()
{
    char stdout_path[PATH_MAX + 4] ;
    char stderr_path[PATH_MAX + 4] ;

    sprintf(stdout_path, "%s/%s", conf.output_dir_path, "out") ;
    sprintf(stderr_path, "%s/%s", conf.output_dir_path, "err") ;
    
    if (access(stdout_path, F_OK) == -1) {
        if (mkdir(stdout_path, 0777) == -1) goto mkdir_err ;
    }
    if (access(stderr_path, F_OK) == -1) {
        if (mkdir(stderr_path, 0777) == -1) goto mkdir_err ;
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

    conf.input_files = (input_t *) malloc(sizeof(input_t) * INPUT_CNT_UNIT) ;

    while ((entry = readdir(dir_ptr)) != 0x0) {
        if (file_cnt != 0x0 && file_cnt % INPUT_CNT_UNIT == 0) {
            conf.input_files = realloc(conf.input_files, sizeof(input_t) * (file_cnt / INPUT_CNT_UNIT + 1) * INPUT_CNT_UNIT) ;
            if (conf.input_files == 0x0) {
                perror("read_input_dir: realloc") ;
                exit(1) ;
            }
        }
        if (entry->d_name[0] != '.') {
            sprintf(conf.input_files[file_cnt].file_path, "%s/%s", conf.input_dir_path, entry->d_name) ;
            conf.input_files[file_cnt].fun_cov = 0 ;
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
        printf("  [%d] %s\n", i, conf.input_files[i].file_path) ;
    }
    printf("\n") ;
}

void 
funcov_init (int argc, char * argv[]) 
{
    get_cmd_args(argc, argv) ;  // TODO. just use exit(1) internally
    set_output_dir() ;
    read_input_dir() ;
    print_config() ;

    cov_stats = (cov_stat_t *) malloc(sizeof(cov_stat_t) * conf.input_file_cnt) ;
    for (int i = 0; i < conf.input_file_cnt; i++) {
        cov_stats[i].fun_coverage = 0 ;
        cov_stats[i].bitmap_size = MAP_SIZE_UNIT ;
        cov_stats[i].bitmap = (uint8_t *) malloc(sizeof(uint8_t) * MAP_SIZE_UNIT) ;
        memset(cov_stats[i].bitmap, 0, MAP_SIZE_UNIT) ;
    }

    trace_cov = (unsigned int *) malloc(sizeof(unsigned int) * MAP_SIZE_UNIT) ;
    memset(trace_cov, 0, MAP_SIZE_UNIT) ;

    trace_bits.bitmap = (uint8_t *) malloc(sizeof(uint8_t) * MAP_SIZE_UNIT) ;
    memset(trace_bits.bitmap, 0, MAP_SIZE_UNIT) ;
    trace_bits.bitmap_size = MAP_SIZE_UNIT ; 
}


static int stdin_pipe[2] ;
static int stdout_pipe[2] ;
static int stderr_pipe[2] ;

void
execute_target (int turn)
{
    FILE * fp = fopen(conf.input_files[turn].file_path, "rb") ;
    if (fp == 0x0) {
        perror("execute_target: fopen") ;
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
            exit(1) ;
        }
    } 
    else if (conf.input_type == ARG_FILENAME) {
        char * args[] = { conf.binary_path, conf.input_files[turn].file_path, (char *)0x0 } ;
        if (execv(conf.binary_path, args) == -1) {
            perror("execute_target: execv") ;
            exit(1) ;
        }
    }
}

void
get_result_file_path (char * path, int turn, int fd)
{
    char input_filename[BUF_SIZE] ;
    strcpy(input_filename, conf.input_files[turn].file_path + strlen(conf.input_dir_path) + 1) ;

    switch (fd) {
    case STDOUT_FD:
        sprintf(path, "%s/%s/%s:out", conf.output_dir_path, "out", input_filename) ;
        break;
    
    case STDERR_FD:
        sprintf(path, "%s/%s/%s:err", conf.output_dir_path, "err", input_filename) ;
        break;
    }
}

void
write_result_file (int turn, int fd)
{
    char path[PATH_MAX] ;
    get_result_file_path(path, turn, fd) ;
    
    FILE * fp = fopen(path, "wb") ;
    if (fp == 0x0) {
        perror("write_result_file: fopen") ;
        exit(1) ;
    }

    char buf[BUF_SIZE] ;
    int s = 0 ;

    if (fd == STDOUT_FD) {
        while ((s = read(stdout_pipe[0], buf, BUF_SIZE)) > 0) {
            if (fwrite(buf, 1, s, fp) != s) {
                perror("write_result_file: fwrite: stdout") ;
            }
        }
        close(stdout_pipe[0]) ;
    }
    else if (fd == STDERR_FD) {
        while ((s = read(stderr_pipe[0], buf, BUF_SIZE)) > 0) {
            if (fwrite(buf, 1, s, fp) != s) {
                perror("write_result_file: fwrite: stderr") ;
            }
        }
        close(stderr_pipe[0]) ;
    }

    fclose(fp) ;
}

void
save_results (int turn)
{
    close(stdin_pipe[0]) ;
    close(stdin_pipe[1]) ;
    close(stdout_pipe[1]) ;
    close(stderr_pipe[1]) ;

    write_result_file(turn, STDOUT_FD) ;
    write_result_file(turn, STDERR_FD) ;
}

int
run (int turn)
{
    if (pipe(stdin_pipe) != 0) goto pipe_err ;
    if (pipe(stdout_pipe) != 0) goto pipe_err ;
    if (pipe(stderr_pipe) != 0) goto pipe_err ;

    int child_pid = fork() ; // TODO. timeout handler

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
    exit(1) ;
}

void
remove_cov_log()
{
    char cov_log_path[PATH_MAX + 8] ;
    sprintf(cov_log_path, "%s/%s", conf.output_dir_path, LOGNAME) ;
    if (remove(cov_log_path) == -1) {
        perror("remove_cov_log: remove") ;
        exit(1) ;
    }
}

void
funcov_destroy ()
{
    // remove_cov_log() ;

    if (conf.input_files != 0x0) free(conf.input_files) ;
    for (int i = 0; i < conf.input_file_cnt; i++) {
        free(cov_stats[i].bitmap) ;
    }
    free(cov_stats) ;
    free(trace_cov) ;
    free(trace_bits.bitmap) ;

    printf("WE ARE DONE!\n\n") ;
}

int
main (int argc, char * argv[])
{
    funcov_init(argc, argv) ;

    /**
     * execute
     * get a log... 
     *  => log 형식이 같아야 하는데, trace-pc.c를 제공해줘야 하나...
     *  => instrumentation이 제대로 되어 있는지 확인할 방법이 없을지
    */

    for (int turn = 0; turn < conf.input_file_cnt; turn++) {
        int exit_code = run(turn) ;    // TODO. use this exit_code?
        
        get_cov_stat(&cov_stats[turn], &conf, turn, exit_code) ;
        trace_cov_stat(&trace_cov[turn], &trace_bits, &cov_stats[turn]) ;
    }

    // print results
    for (int turn = 0; turn < conf.input_file_cnt; turn++) {
        printf("%d ", trace_cov[turn]) ;
    }
    printf("\n") ;
    
    funcov_destroy() ;

    return 0 ;
}
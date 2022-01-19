#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../include/funcov.h"

#define INPUT_CNT_UNIT 512
#define BUF_SIZE 1024

#define STDOUT_FD 1
#define STDERR_FD 2

static config_t conf ;

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

int
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
                return -1 ;
            }
            i_flag = 1 ;
            arg_cnt += 2 ;
            break ;

        case 'o':
            if (access(optarg, F_OK) == -1) {
                if (mkdir(optarg, 0777) == -1) {
                    perror("get_cmd_args: mkdir: Failed to make an output directory") ;
                    return -1 ;
                }
            }
            if (realpath(optarg, conf.output_dir_path) == 0x0) {
                perror("get_cmd_args: realpath: Invalid output directory") ;
                return -1 ;
            }
            o_flag = 1 ;
            arg_cnt += 2 ;
            break ;

        case 'x':
            if (realpath(optarg, conf.binary_path) == 0x0) {
                perror("get_cmd_args: realpath: Invalid executable binary") ;
                return -1 ;
            }
            if (access(optarg, X_OK) == -1) {
                perror("get_cmd_args: access: The target is not executable") ;
                return -1 ;
            }
            x_flag = 1 ;
            arg_cnt += 2 ;
            break ;
        }
    }

    if (!i_flag || !o_flag || !x_flag) goto print_usage ;

    if (argc > 9) {
        if (strcmp(argv[arg_cnt], "@@") == 0) {
            conf.input_type = ARG_FILENAME ;
            arg_cnt++ ;
        }
        else goto print_usage ;
    } 
    else {
        conf.input_type = STDIN ;
    }

    return 0 ;

print_usage:
    perror("\nusage: ./funcov -i [input_dir] -x [executable_binary] ...\n\nrequired\n-i : input directory path\n-o : output directory path\n-x : executable binary path\n\noptional\n@@ : input type - file as an argument\n\n") ;
    return -1 ;
}

int
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

    return 0 ;

mkdir_err:
    perror("set_output_dir: mkdir: Failed to set an output directory") ;
    return -1 ;
}

int
read_input_dir ()
{
    int file_cnt = 0 ;

    DIR * dir_ptr = 0x0 ;
    struct dirent * entry = 0x0 ;

    if ((dir_ptr = opendir(conf.input_dir_path)) == 0x0) {
        perror("read_input_dir: opendir") ;
        return -1 ;
    }

    conf.input_files = (input_t *) malloc(sizeof(input_t) * INPUT_CNT_UNIT) ;

    while ((entry = readdir(dir_ptr)) != 0x0) {
        if (file_cnt != 0x0 && file_cnt % INPUT_CNT_UNIT == 0) {
            conf.input_files = realloc(conf.input_files, sizeof(input_t) * (file_cnt / INPUT_CNT_UNIT + 1) * INPUT_CNT_UNIT) ;
            if (conf.input_files == 0x0) {
                perror("read_input_dir: realloc") ;
                return -1 ;
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
    
    return 0 ;
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

    if (conf.input_type == STDIN) {
        if (execl(conf.binary_path, conf.binary_path, (char *)0x0) == -1) {
            perror("execute_target: execl") ;
            exit(1) ;
        }
    } 
    else if (conf.input_type == ARG_FILENAME) {
        if (execl(conf.binary_path, conf.binary_path, conf.input_files[turn].file_path, (char *)0x0) == -1) {
            perror("execute_target: execl") ;
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

    return 0 ;

pipe_err:
    perror("run: pipe") ;
    exit(1) ;
}

int
main (int argc, char * argv[])
{
    if (get_cmd_args(argc, argv) == -1) return 1 ;  // TODO. just use exit(1) internally
    if (set_output_dir() == -1) return 1 ;
    if (read_input_dir() == -1) return 1 ;
    print_config() ;

    /**
     * execute
     * get a log... 
     *  => log 형식이 같아야 하는데, trace-pc.c를 제공해줘야 하나...
     *  => instrumentation이 제대로 되어 있는지 확인할 방법이 없을지
    */
    for (int i = 0; i < conf.input_file_cnt; i++) {
        run(i) ;
    }

    if (conf.input_files != 0x0) free(conf.input_files) ;
    return 0 ;
}
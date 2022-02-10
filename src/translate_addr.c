#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../include/translate_addr.h"

unsigned short
hash16 (char * key)  // TODO. not tested
{
	unsigned int h = 0 ;

	while (*key) {
		h = h * 23131 + (unsigned char)*key++ ;
	}

	return (h & 0xffff) ;
}

void
tokenize_cov_strings (char ** argv, int cov_cnt, map_elem_t trace_map[][MAP_COL_UNIT])
{
	int index = 3 ;

	for (int r = 0; r < MAP_ROW_UNIT; r++) {
		for (int c = 0; c < MAP_COL_UNIT; c++) {
			if (trace_map[r][c].hit_count == 0) break ;

			char copied_string[COV_STRING_MAX] ;
			strcpy(copied_string, trace_map[r][c].cov_string) ;

			char * next = 0x0 ;
			char * tok = strtok_r(copied_string, ",", &next) ;
			tok = strtok_r(NULL, ",", &next) ;
			strcpy(argv[index], next) ;
			index++ ;
		}
	}

}

static int in_pipe[2] ;
static int out_pipe[2] ;
static int err_pipe[2] ;

int
execute_addr2line (char ** argv)
{
	close(in_pipe[1]) ;

    dup2(in_pipe[0], 0) ;
    
    close(in_pipe[0]) ;
    close(out_pipe[0]) ;
    close(err_pipe[0]) ;

    dup2(out_pipe[1], 1) ;
    dup2(err_pipe[1], 2) ;

	return execv(argv[0], argv) ;

	return 0 ;
}

void
save_locations (location_t * translated_locations, char ** argv, int cov_cnt)
{
	close(in_pipe[0]) ;
	close(err_pipe[0]) ;
    close(in_pipe[1]) ;
    close(out_pipe[1]) ;
    close(err_pipe[1]) ;

	char buf[BUF_SIZE] ;
	char line[BUF_SIZE] ;
	
	int cnt = 0 ;
	int line_idx = 0 ;
	int buf_idx = 0 ;
	int s = 0 ;

	while ((s = read(out_pipe[0], buf, BUF_SIZE)) > 0) {
		for (buf_idx = 0; buf_idx < s; buf_idx++, line_idx++) {
			if (buf[buf_idx] == '\n') {
				line[line_idx] = 0x0 ;

				unsigned short id = hash16(argv[cnt + 3]) ;	// TODO. hash collision
				strcpy(translated_locations[id].pc_val, argv[cnt + 3]) ;
				strcpy(translated_locations[id].location, line) ;

				line_idx = -1 ;
				cnt++ ;
			}
			else {
				line[line_idx] = buf[buf_idx] ;
			}
		}
		memset(buf, 0, BUF_SIZE) ;
	}

	close(out_pipe[0]) ;
}

int
translate_pc_values (location_t * translated_locations, int cov_cnt, map_elem_t trace_map[][MAP_COL_UNIT], char * binary_path)
{
	char ** argv = (char **) malloc(sizeof(char *) * (cov_cnt + 4)) ;

	argv[0] = (char *) malloc(sizeof(char) * PATH_MAX) ;
	strcpy(argv[0], "/usr/bin/addr2line") ;	// Q. location?

	argv[1] = (char *) malloc(sizeof(char) * 4) ;
	strcpy(argv[1], "-e") ;

	argv[2] = (char *) malloc(sizeof(char) * PATH_MAX) ;
	strcpy(argv[2], binary_path) ;

	for (int i = 3; i < cov_cnt + 3; i++) {
		argv[i] = (char *) malloc(sizeof(char) * ADDR_MAX) ;
	}

	argv[cov_cnt + 3] = (char *)0x0 ;

	tokenize_cov_strings(argv, cov_cnt, trace_map) ;

	if (pipe(in_pipe) != 0) goto pipe_err ;
	if (pipe(out_pipe) != 0) goto pipe_err ;
	if (pipe(err_pipe) != 0) goto pipe_err ;

	int child_pid = fork() ;

	if (child_pid == 0) {
		if (execute_addr2line(argv) == -1) return -1 ;
	}
	else if (child_pid > 0) {
		save_locations (translated_locations, argv, cov_cnt) ;
	}
	else {
		perror("translate_pc_values: fork") ;
		return -1 ;
	}

	wait(0x0) ;

	for (int i = 0; i < cov_cnt + 3; i++) {
		free(argv[i]) ;
	}
	free(argv) ;

	return 0 ;

pipe_err:
	perror("run: pipe") ;
	return -1 ;
}
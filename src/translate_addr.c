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
get_pc_val (char * dst, char * src)
{
	char copied_string[COV_STRING_MAX] ;
	strcpy(copied_string, src) ;

	char * next = 0x0 ;
	char * tok = strtok_r(copied_string, ",", &next) ;
	tok = strtok_r(NULL, ",", &next) ;
	strcpy(dst, next) ;
}

void
tokenize_cov_strings (char ** argv, int cov_cnt, map_elem_t * trace_map)
{
	int index = 3 ;

	for (int i = 0; i < MAP_SIZE; i++) {
		if (trace_map[i].hit_count != 0) {
			get_pc_val(argv[index], trace_map[i].cov_string) ;
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

int
save_locations (location_t * translated_locations, char ** argv, int cov_cnt)
{
	close(in_pipe[0]) ;
	close(err_pipe[0]) ;
    close(in_pipe[1]) ;
    close(out_pipe[1]) ;
    close(err_pipe[1]) ;

	FILE * fp = fdopen(out_pipe[0], "rb") ;
	if (fp == 0x0) {
		perror("translate_pc_values: save_locations: fdopen") ;
		return -1 ;
	}

	char buf[BUF_SIZE] ;

	for (int cnt = 0; fgets(buf, BUF_SIZE, fp) != 0x0; cnt++) {
		int len = strlen(buf) ;
		buf[len - 1] = 0x0 ;

		int found = 0 ;
		unsigned short id = hash16(argv[cnt + 3]) ;
		for (int i = 0; i < MAP_SIZE; i++) {
			if (id >= MAP_SIZE) id = 0 ;

			if (translated_locations[id].exist) id++ ;
			else {
				strcpy(translated_locations[id].pc_val, argv[cnt + 3]) ;
				strcpy(translated_locations[id].location, buf) ;
				translated_locations[id].exist = 1 ;
				found = 1 ;
				break ;
			}
		}
		if (!found) {
			perror("translate_pc_values: save_locations: map limit") ;
			return -1 ;
		}
	}

	fclose(fp) ;

	return 0 ;
}

int
translate_pc_values (location_t * translated_locations, int cov_cnt, map_elem_t * trace_map, char * binary_path)
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
		if (save_locations (translated_locations, argv, cov_cnt) == -1) return -1 ;
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

int
find_location_info (char * dst, location_t * translated_locations, char * cov_string)
{
    char pc_val[ADDR_MAX] ;
    get_pc_val(pc_val, cov_string) ;

    int found = 0 ;
    unsigned short id = hash16(pc_val) ;
    for (int i = 0; i < MAP_SIZE; i++) {
        if (id >= MAP_SIZE) id = 0 ;
        if (!translated_locations[id].exist) goto not_found ;

        if (strcmp(pc_val, translated_locations[id].pc_val) == 0) {
            strcpy(dst, translated_locations[id].location) ;
            found = 1 ;
            break ;
        }
        else id++ ;
    }
    if (!found) goto not_found ;

    return 0 ;

not_found:
    perror("find_location_info: Translated value does not exist") ;
    return -1 ;
}
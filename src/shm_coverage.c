#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "../include/shm_coverage.h"

int
get_shm (shm_t type, int type_size)
{
    int shm_id ;

    if (type == INIT) {
        shm_id = shmget(IPC_PRIVATE, type_size, IPC_CREAT | IPC_EXCL | 0666) ;  
        if (shm_id == -1) { // already exist
            perror("get_shm: shmget") ;
            exit(1) ;
        }

        char id_str[64] ;
        sprintf(id_str, "%d", shm_id) ;
        setenv(SHM_ENV_VAR, id_str, 1) ;
    }
    else if (type == USE) {
        char * id_str_p = getenv(SHM_ENV_VAR) ;
        shm_id = atoi(id_str_p) ;
    }

    return shm_id ;
}

void *
attatch_shm (int shm_id)
{
    void * ptr = shmat(shm_id, 0, 0) ;
    if (ptr == (void *)(-1)) {
        perror("get_shm: shmat") ;
        exit(1) ;
    }

    return ptr ;
}

void
detatch_shm (void * shm_addr) 
{
    if (shmdt(shm_addr) == -1) {
        perror("remove_shm: shmdt") ;
        exit(1) ; // Q.
    }
}

void
remove_shm (int shm_id)
{
    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
        perror("remove_shm: shmctl") ;
        exit(1) ; // Q.
    }
}
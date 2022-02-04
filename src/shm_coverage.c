#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "../include/shm_coverage.h"

int
create_shm (int key, void ** ptr, int type_size)
{
    int shm_id ;
    shm_id = shmget((key_t) key, type_size, IPC_CREAT | IPC_EXCL | 0666) ;  
    if (shm_id == -1) { // already exist
        shm_id = shmget((key_t) key, type_size, 0) ;
        if (shm_id == -1) {
            perror("create_shm: shmget") ;
            exit(1) ;
        }
    }
    
    *ptr = shmat(shm_id, 0, 0) ;
    if (*ptr == (void *)(-1)) {
        perror("create_shm: shmat") ;
        exit(1) ;
    }

    return shm_id ;
}

void
remove_shm (int shm_id, void * shm_addr)
{
    if (shmdt(shm_addr) == -1) {
        perror("remove_shm: shmdt") ;
        exit(1) ; // Q.
    }

    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
        perror("remove_shm: shmctl") ;
        exit(1) ; // Q.
    }
}
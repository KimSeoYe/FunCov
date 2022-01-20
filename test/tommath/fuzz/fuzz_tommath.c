// #define THE_MASK 32767

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "../mtest/mpi.c"

#define MAX_LEN (256 * 1000)
#define BUF_SIZE 4096
#define mp_to64(a, b) mp_toradix(a, b, 64)

//compile: 	clang -fsanitize=address -fsanitize-coverage=func,trace-pc-guard 
//          tommath.a fuzz_tommath.c -o fuzz_tom trace-pc-guard.o
int main(void)
{
    char buf[4096] = {0, };
    fgets(buf,BUF_SIZE,stdin);

    size_t l1 = 0, l2 = 0;
    size_t len = strlen(buf);

    mp_int a ,b, c ,d , e, f , g;

    mp_init(&a);
    mp_init(&b);
    mp_init(&c);
    mp_init(&d);
    mp_init(&e);
    mp_init(&f);
    mp_init(&g);

    if (len > 3) {

        l1 = len/2;
        l2 = len - l1;

        mp_read_raw(&a,buf,l1);
        mp_read_raw(&b,buf+l1-1,l2);

        mp_copy(&a,&c);

        // if(mp_cmp(&a,&b) == -1){
            mp_add(&a,&b,&d);
            mp_mul(&a,&b,&f);
            mp_sqr(&a,&d);
        // }else{
            mp_sub(&a,&b,&e);
            mp_div(&a,&b,&g,&c);
        // }

        a.sign = MP_ZPOS;
        b.sign = MP_ZPOS;

        mp_lcm(&a, &b, &c);
        
        a.sign = b.sign = c.sign = 0;
        mp_exptmod(&a, &b, &c, &d);
    }
    mp_clear(&a);
    mp_clear(&b);
    mp_clear(&c);
    mp_clear(&d);
    mp_clear(&e);
    mp_clear(&f);
    mp_clear(&g);
    return 0;
}

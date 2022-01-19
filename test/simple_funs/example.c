#include <stdio.h>

void
func1 ()
{
	printf("func1 called!\n") ;
}

void
func2 ()
{
	printf("func2 called!\n") ;
}

void
func3 ()
{
	printf("func3 called!\n") ;
}

int
main (int argc, char * argv[])
{
	// char buf[1024] ;
	// FILE * fp = fopen(argv[1], "rb") ;
	// int s = fread(buf, 1, 1023, fp) ;
	// printf("len: %d\n", s) ;
	// buf[s] = 0x0 ;
	// fclose(fp) ;

	// printf("main called w/ %s\n", buf) ;
	func1() ;
	func2() ;
	func3() ;
	func1() ;
	func2() ;
	func3() ;

	return 0 ;
}

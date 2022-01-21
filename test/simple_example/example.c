#include <stdio.h>
#include <stdlib.h>

void
negative ()
{
	printf("the input is negative\n") ;
}

void
zero ()
{
	printf("the input is zero\n") ;
}

void
positive ()
{
	printf("the input is positive\n") ;
}

int
main ()
{
	int input ;
	scanf("%d", &input) ;
	
	if (input < 0) negative() ;
	else if (input > 0) positive() ;
	else zero() ;

	return 0 ;
}

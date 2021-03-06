/*	bignum.c
	Implementation of large integer arithmetic: addition, subtraction,
		multiplication, and division.

	begun: February 7, 2002
	by: Steven Skiena
*/

/*
Copyright 2003 by Steven S. Skiena; all rights reserved. 

Permission is granted for use in non-commerical applications
provided this copyright notice remains intact and unchanged.

This program appears in my book:

"Programming Challenges: The Programming Contest Training Manual"
by Steven Skiena and Miguel Revilla, Springer-Verlag, New York 2003.

See our website www.programming-challenges.com for additional information.

This book can be ordered from Amazon.com at

http://www.amazon.com/exec/obidos/ASIN/0387001638/thealgorithmrepo/

*/


#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

#define	MAXDIGITS	100		/* maximum length bignum */ 

#define PLUS		1		/* positive sign bit */
#define MINUS		-1		/* negative sign bit */

typedef struct {
	char digits[MAXDIGITS];	/* represent the number */
	int signbit;			/* 1 if positive, -1 if negative */ 
	int lastdigit;			/* index of high-order digit */
} bignum;


void print_bignum( bignum *n )
{
	int i;

	if (n->signbit == MINUS) printf("- ");
	for (i=n->lastdigit; i>=0; i--)
		printf("%c",'0'+ n->digits[i]);

	printf("\n");
}

void int_to_bignum( int s, bignum *n )
{
	int i;				/* counter */
	int t;				/* int to work with */

	if (s >= 0)
		n->signbit = PLUS;
	else
		n->signbit = MINUS;

	for (i=0; i<MAXDIGITS; i++) n->digits[i] = (char) 0;

	n->lastdigit = -1;

	t = abs(s);

	while (t > 0) {
		n->lastdigit ++;
		n->digits[ n->lastdigit ] = (t % 10);
		t = t / 10;
	}

	if (s == 0) n->lastdigit = 0;
}

void initialize_bignum(bignum *n)
{
	int_to_bignum( 0, n );
}


int max( int a, int b )
{
/*	if (a > b) return(a); else return(b); */
	return ( a > b ) ? a : b;
}

void zero_justify(bignum *n)
{
	while ((n->lastdigit > 0) && (n->digits[ n->lastdigit ] == 0))
		n->lastdigit --;

	if ((n->lastdigit == 0) && (n->digits[0] == 0))
		n->signbit = PLUS;	/* hack to avoid -0 */
}

int compare_bignum( bignum *a, bignum *b )
{
	int i;				/* counter */

	if ((a->signbit == MINUS) && (b->signbit == PLUS)) return(PLUS);
	if ((a->signbit == PLUS) && (b->signbit == MINUS)) return(MINUS);

	if (b->lastdigit > a->lastdigit) return (PLUS * a->signbit);
	if (a->lastdigit > b->lastdigit) return (MINUS * a->signbit);

	for (i = a->lastdigit; i>=0; i--) {
		if (a->digits[i] > b->digits[i]) return(MINUS * a->signbit);
		if (b->digits[i] > a->digits[i]) return(PLUS * a->signbit);
	}

	return(0);
}


/*	c = a + b;	*/

void add_bignum(bignum *a, bignum *b, bignum *c)
{
	int carry;			/* carry digit */
	int i;				/* counter */

	void subtract_bignum(bignum *a, bignum *b, bignum *c);

	initialize_bignum(c);

	if (a->signbit == b->signbit)
		c->signbit = a->signbit;
	else {
		if (a->signbit == MINUS) {
			a->signbit = PLUS;
			subtract_bignum(b,a,c);
			a->signbit = MINUS;
		} else {
			b->signbit = PLUS;
			subtract_bignum(a,b,c);
			b->signbit = MINUS;
		}
		return;
	}

	c->lastdigit = max(a->lastdigit,b->lastdigit)+1;
	carry = 0;

	for (i=0; i<=(c->lastdigit); i++) {
		c->digits[i] = (char) (carry+a->digits[i]+b->digits[i]) % 10;
		carry = (carry + a->digits[i] + b->digits[i]) / 10;
	}

	zero_justify(c);
}


/*	c = a - b;	*/
void subtract_bignum(bignum *a, bignum *b, bignum *c)
{
	int borrow;			/* has anything been borrowed? */
	int v;				/* placeholder digit */
	int i;				/* counter */

	initialize_bignum(c);

	if ((a->signbit == MINUS) || (b->signbit == MINUS)) {
		b->signbit = -1 * b->signbit;
		add_bignum(a,b,c);
		b->signbit = -1 * b->signbit;
		return;
	}

	if (compare_bignum(a,b) == PLUS) {
		subtract_bignum(b,a,c);
		c->signbit = MINUS;
		return;
	}

	c->lastdigit = max(a->lastdigit,b->lastdigit);
	borrow = 0;

	for (i=0; i<=(c->lastdigit); i++) {
		v = (a->digits[i] - borrow - b->digits[i]);
		if (a->digits[i] > 0)
			borrow = 0;
		if (v < 0) {
			v = v + 10;
			borrow = 1;
		}
		c->digits[i] = (char) v % 10;
	}

	zero_justify(c);
}

void digit_shift( bignum *n, int d )		/* multiply n by 10^d */
{
	int i;				/* counter */

	if ((n->lastdigit == 0) && (n->digits[0] == 0)) return;

	for (i=n->lastdigit; i>=0; i--)
		n->digits[i+d] = n->digits[i];

	for (i=0; i<d; i++) n->digits[i] = 0;

	n->lastdigit = n->lastdigit + d;
}


/*	c = a * b;	*/
void multiply_bignum( bignum *a, bignum *b, bignum *c )
{
	bignum row;			/* represent shifted row */
	bignum tmp;			/* placeholder bignum */
	int i,j;			/* counters */

	initialize_bignum(c);

	row = *a;

	for (i=0; i<=b->lastdigit; i++) {
		for (j=1; j<=b->digits[i]; j++) {
			add_bignum(c,&row,&tmp);
			*c = tmp;
		}
		digit_shift(&row,1);
	}

	c->signbit = a->signbit * b->signbit;

	zero_justify(c);
}


/*	c = a / b;	*/
void divide_bignum( bignum *a, bignum *b, bignum *c )
{
	bignum row;             /* represent shifted row */
	bignum tmp;             /* placeholder bignum */
	int asign, bsign;		/* temporary signs */
	int i;                  /* counters */

	initialize_bignum(c);

	c->signbit = a->signbit * b->signbit;

	asign = a->signbit;
	bsign = b->signbit;

	a->signbit = PLUS;
	b->signbit = PLUS;

	initialize_bignum(&row);
	initialize_bignum(&tmp);

	c->lastdigit = a->lastdigit;

	for (i=a->lastdigit; i>=0; i--) {
		digit_shift(&row,1);
		row.digits[0] = a->digits[i];
		c->digits[i] = 0;
		while (compare_bignum(&row,b) != PLUS) {
			c->digits[i] ++;
			subtract_bignum(&row,b,&tmp);
			row = tmp;
		}
	}

	zero_justify(c);

	a->signbit = asign;
	b->signbit = bsign;
}


int main()
{
	//(n^4-6n^3+23n^2-18^n+24)/24

	bignum n,n2,n3,n4,tmp,tmp1,tmp2,tmp3,tmp4,res1,res2, resto;
	int i,j,k;
	int ntest;

	cin >> ntest;

	for(i=0;i<ntest;i++)
	{
		cin >> k;
		if(k==1)
		{
			printf("1\n");
			continue;
		}
		else if(k==2)
		{
			printf("2\n");
			continue;
		}
		else if(k==3)
		{
			printf("4\n");
			continue;
		}
		int_to_bignum(k,&n);
		int_to_bignum(k,&tmp);
		multiply_bignum(&n,&tmp,&n2);
		multiply_bignum(&n,&n2,&n3);
		multiply_bignum(&n,&n3,&n4);
		
		int_to_bignum(-6,&tmp);
		multiply_bignum(&tmp,&n3,&tmp3);
		
		int_to_bignum(23,&tmp);
		multiply_bignum(&tmp,&n2,&tmp2);

		int_to_bignum(-18,&tmp);
		multiply_bignum(&tmp,&n,&tmp1);

		int_to_bignum(24,&tmp);
		add_bignum(&n4,&tmp3,&res1);
		add_bignum(&res1,&tmp2,&res2);
		add_bignum(&res2,&tmp1,&res1);
		add_bignum(&res1,&tmp,&res2);


		divide_bignum(&res2,&tmp,&res1);
		
		
		print_bignum(&res1);
		
		
	}
	
	return 0;
}

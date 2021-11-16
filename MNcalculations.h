#ifndef __MNCALCULATIONS_H__
#define __MNCALCULATIONS_H__

#include "myNumber.h"

int charsToNumber(char *chars, myNumber *number, unsigned char base);

char *numberToChars(myNumber *number);

int MNadd(myNumber *a, myNumber *b, myNumber *result, unsigned char base);

int MNmultiply(myNumber *a, myNumber *b, myNumber *result, unsigned char base);

int MNcompare(myNumber *a, myNumber *b);

int MNsubstract(myNumber *a, myNumber *b, myNumber *result, unsigned char base);

int MNdivide(myNumber *dividend, myNumber *divisor, myNumber *quotient, myNumber *residue, unsigned char base);

int MNconvert(myNumber *number, myNumber *convertedNumber, unsigned char originBase, unsigned char convertionBase);

int MNraise(myNumber *a, myNumber *exponent, myNumber *result, unsigned char base);

#endif
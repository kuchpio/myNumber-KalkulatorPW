#ifndef __MY_NUMBER__
#define __MY_NUMBER__

#include <stdlib.h>

typedef struct {
    unsigned char *digits; //number 4D67A will be represented as 10, 7, 6, 13, 4
    size_t numberOfDigits;
    size_t allocatedMemory;
}myNumber;

myNumber *MNinit(size_t initialSize);

int MNgrow(myNumber *number);

unsigned char MNgetDigit(myNumber *number, size_t index);

int MNsetDigit(myNumber *number, size_t index, unsigned char digit);

size_t MNsize(myNumber *number);

int MNerase(myNumber *number);

int MNdelete(myNumber *number);

#endif
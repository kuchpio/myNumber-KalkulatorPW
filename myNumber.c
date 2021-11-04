#include "myNumber.h"

#include <stdio.h>
#include <stdlib.h>

myNumber *MNinit(size_t initialSize) 
{
    //allocate memory for new number
    myNumber *number = malloc(sizeof(myNumber));
    number->numberOfDigits = 0;
    number->allocatedMemory = initialSize > 0 ? initialSize : 1;
    number->digits = malloc(number->allocatedMemory * sizeof(unsigned char));

    return number;
}

int MNgrow(myNumber *number) 
{
    number->allocatedMemory = 2*number->allocatedMemory;
    number->digits = realloc(number->digits, number->allocatedMemory);
    printf("(myNumber): New memory allocated: %d\n", number->allocatedMemory);

    return 0;
}

unsigned char MNgetDigit(myNumber *number, size_t index) 
{
    return *(number->digits + index);
}

int MNsetDigit(myNumber *number, size_t index, unsigned char digit) 
{
    //check if index is out of range
    while (index >= number->allocatedMemory) 
        MNgrow(number);

    //only increase size when leading digit is non-zero or the only digit is zero
    if (index >= number->numberOfDigits && digit != 0)
        number->numberOfDigits = index + 1;
    
    if(number->numberOfDigits == 0 && digit == 0)
        number->numberOfDigits = 1;

    *(number->digits + index) = digit;
    return 0;
}

size_t MNsize(myNumber *number) 
{
    return number->numberOfDigits;
}

int MNerase(myNumber *number)
{
    number->numberOfDigits = 0;

    return 0;
}

int MNdelete(myNumber *number) 
{
    free(number->digits);
    free(number);

    return 0;
}
#include "myNumber.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

myNumber *MNinit(size_t initialSize) 
{
    //allocate memory for new number
    myNumber *number = malloc(sizeof(myNumber));
    number->allocatedMemory = initialSize > 0 ? initialSize : 1;
    number->digits = calloc(number->allocatedMemory, sizeof(unsigned char));

    return number;
}

int MNgrow(myNumber *number) 
{
    unsigned char *tmp = realloc(number->digits, 2*number->allocatedMemory);
    if (tmp != NULL)
    {
        number->digits = tmp;
        memset(number->digits + number->allocatedMemory, 0, number->allocatedMemory * sizeof(unsigned char));
        //printf("(MN): Size increased: %ld -> %ld\n", number->allocatedMemory, 2 * number->allocatedMemory);
        number->allocatedMemory = 2 * number->allocatedMemory;
        return 0;
    }

    return 1;
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

    *(number->digits + index) = digit;
    return 0;
}

size_t MNsize(myNumber *number) 
{
    size_t size = number->allocatedMemory;

    while (*(number->digits + size - 1) == 0 && size > 0)
        size--;
    
    return size;
}

int MNerase(myNumber *number)
{
    memset(number->digits, 0, number->allocatedMemory * sizeof(unsigned char));
    return 0;
}

int MNdelete(myNumber *number) 
{
    free(number->digits);
    free(number);

    return 0;
}
#include "MNcalculations.h"
#include "myNumber.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int charsToNumber(char *chars, myNumber *number, unsigned char base) //converts string to myNumber
{
    //shift down '0', '1' , ..., '9' to 0, 1, ..., 9
    //shift down 'A', 'B', ..., 'F' to 10, 11, ..., 15
    //shift down 'a', 'b', ..., 'f' to 10, 11, ..., 15
    int charLength = strlen(chars);
    unsigned char c;
    
    for (int i = 0; i < charLength; i++) 
    {
        c = *(chars + charLength - 1 - i);
        if (c >= '0' && c <= '9') 
            MNsetDigit(number, i, c - '0');

        else if (c >= 'A' && c <= 'F') 
            MNsetDigit(number, i, c - 'A' + 10);

        else if (c >= 'a' && c <= 'f') 
            MNsetDigit(number, i, c - 'a' + 10);

        else 
        {
            printf("Incorrect digit: '%c' (ASCII: %d) in number %s (base %d)\n", c, c, chars, base);
            return -1; //error
        }

        if (MNgetDigit(number, i) >= base) 
        {
            printf("Incorrect digit: '%c' (ASCII: %d) in number %s (base %d)\n", c, c, chars, base);
            return -1; //error
        }
    }

    return 0;
} 

char *numberToChars(myNumber *number) //converts myNumber to string 
{
    int numberSize = MNsize(number);
    char *result;
    unsigned char digitValue;

    if (numberSize == 0) 
    {
        result = malloc(2 * sizeof(char));
        *result = '0';
        *(result + 1) = '\0';
        return result;
    }

    //allocate memory for new number
    result = malloc((numberSize + 1) * sizeof(char));

    //shift values to their chars
    for (int j = 0; j < numberSize; j++)
    {
        digitValue = MNgetDigit(number, numberSize - j - 1);

        if (digitValue < 10) 
        {
            *(result + j) = digitValue + '0';
        } 
        else if (digitValue < 16) 
        {
            *(result + j) = digitValue - 10 + 'A';
        } 
        else 
        {
            printf("Incorrect value: %d (index: %d)\n", digitValue, numberSize - j - 1);
        }
    }

    *(result + numberSize) = '\0';

    return result;
}

int MNadd(myNumber *a, myNumber *b, myNumber *result, unsigned char base) 
{
    int i = 0, aSize = MNsize(a), bSize = MNsize(b);
    unsigned char leadingDigit = 0, sum;

    while (i < aSize && i < aSize) 
    {
        sum = MNgetDigit(a, i) + MNgetDigit(b, i) + leadingDigit;
        leadingDigit = sum / base;
        MNsetDigit(result, i++, sum % base);
    }
    while (i < aSize) 
    {
        sum = MNgetDigit(a, i) + leadingDigit;
        leadingDigit = sum / base;
        MNsetDigit(result, i++, sum % base);
    }
    while (i < aSize) 
    {
        sum = MNgetDigit(b, i) + leadingDigit;
        leadingDigit = sum / base;
        MNsetDigit(result, i++, sum % base);
    }

    MNsetDigit(result, i, leadingDigit);

    return 0; //success
}

int MNmultiply(myNumber *a, myNumber *b, myNumber *result, unsigned char base) 
{
    size_t aSize = MNsize(a), bSize = MNsize(b);
    unsigned char product, leadingDigit;

    //init result as 0's
    for (int i = 0; i < aSize + bSize; i++)
        MNsetDigit(result, i, 0);


    for (int i = 0; i < aSize; i++) 
    {
        leadingDigit = 0;
        for (int j = 0; j < bSize; j++) 
        {
            product = MNgetDigit(result, i + j) + leadingDigit + MNgetDigit(a, i) * MNgetDigit(b, j);
            MNsetDigit(result, i + j, product % base);
            leadingDigit = product / base;
        }
        MNsetDigit(result, i + bSize, leadingDigit);
    }

    return 0; //success
}

int MNcompare(myNumber *a, myNumber *b) //returns 1 when a > b, 0 otherwise
{
    int aSize = MNsize(a), bSize = MNsize(b);
    unsigned char aDigit, bDigit;

    if (aSize > bSize)
        return 1;

    if (aSize < bSize)
        return 0;

    for (int i = aSize - 1; i >= 0; i--) 
    {
        aDigit = MNgetDigit(a, i);
        bDigit = MNgetDigit(b, i);

        if (aDigit != bDigit) {
            if (aDigit > bDigit)
                return 1;
            else
                return 0;
        }
    }

    return 0;
}

int MNsubstract(myNumber *a, myNumber *b, myNumber *result, unsigned char base) //assumes a >= b
{
    unsigned char aDigit, bDigit;
    int i = 0, j, aSize = MNsize(a), bSize = MNsize(b);
    myNumber *aCopy = MNinit(aSize); //working on copy of a not to change a

    if (MNcompare(b, a)) 
    {
        printf("Invalid arguments for MNsubstract: first number cannot be smaller than second number\n");
        return -1; //error
    }

    //copy a to aCopy
    for (int i = 0; i < aSize; i++) 
        MNsetDigit(aCopy, i, MNgetDigit(a, i));

    while (i < bSize) 
    {
        aDigit = MNgetDigit(aCopy, i);
        bDigit = MNgetDigit(b, i);

        if (aDigit < bDigit) 
        {
            aDigit += base;

            j = i + 1;
            while (MNgetDigit(aCopy, j) == 0) 
                MNsetDigit(aCopy, j++, base - 1);

            MNsetDigit(aCopy, j, MNgetDigit(aCopy, j) - 1);
        }

        MNsetDigit(result, i++, aDigit - bDigit);
    }

    //copy rest of number a to result
    while (i < aSize) 
        MNsetDigit(result, i++, MNgetDigit(aCopy, i));

    MNdelete(aCopy);

    return 0; //success
}

int MNdivide(myNumber *dividend, myNumber *divisor, myNumber *quotient, myNumber *residue, unsigned char base) {

    //check if divisor is 0
    int divisorSize = MNsize(divisor);
    int dividendSize = MNsize(dividend);
    unsigned char smallQuotient;
    int residueIsNULL = 0;

    if (dividendSize == 0)
        return -1; // error: divisor is 0;

    //initiate smaller part of divident
    if (residue == NULL) 
    {
        residue = MNinit(divisorSize + 1);
        residueIsNULL = 1;
    }

    MNerase(residue);

    for (int i = dividendSize - 1; i >= 0; i--) 
    {
        //push i-th digit of dividend to the back of residue
        for (int j = MNsize(residue) - 1; j >= 0; j--)
            MNsetDigit(residue, j + 1, MNgetDigit(residue, j));
        
        MNsetDigit(residue, 0, MNgetDigit(dividend, i));

        smallQuotient = 0;
        while (MNcompare(divisor, residue) == 0) //while (divisor <= residue)
        { 
            smallQuotient++;
            MNsubstract(residue, divisor, residue, base); //residue -= divisor
        }

        MNsetDigit(quotient, i, smallQuotient);
    }

    if (residueIsNULL == 1)
        MNdelete(residue);

    return 0;
}

int MNconvert(myNumber *number, myNumber *convertedNumber, unsigned char originBase, unsigned char convertionBase) 
{
    int numberSize = MNsize(number);
    unsigned char digit;
    myNumber *originBaseConverted = MNinit(5 * sizeof(unsigned char)); //16 -> 10000
    myNumber *temp = MNinit(4 * MNsize(number));
    myNumber *digitConverted = MNinit(4 * sizeof(unsigned char)); //15 -> 1111

    //convert origin base to convertion base
    for (int i = 0; originBase > 0; i++) 
    {
        MNsetDigit(originBaseConverted, i, originBase % convertionBase);
        originBase = originBase / convertionBase;
    }
    
    //convert first digit of number
    digit = MNgetDigit(number, numberSize - 1);
    for (int i = 0; digit > 0; i++) 
    {
        MNsetDigit(convertedNumber, i, digit % convertionBase);
        digit = digit / convertionBase;
    }

    for (int i = numberSize - 2; i >= 0; i--) 
    {
        MNmultiply(convertedNumber, originBaseConverted, temp, convertionBase);
        
        //convert digit of number
        digit = MNgetDigit(number, i);
        MNerase(digitConverted);
        for (int j = 0; digit > 0; j++) 
        {
            MNsetDigit(digitConverted, j, digit % convertionBase);
            digit = digit / convertionBase;
        }
        
        MNadd(temp, digitConverted, convertedNumber, convertionBase);
    }

    MNdelete(originBaseConverted);
    MNdelete(temp);
    MNdelete(digitConverted);

    return 0;
}

int MNraise(myNumber *a, myNumber *exponent, myNumber *result, unsigned char base) 
{
    myNumber *binaryExponent = MNinit(4 * MNsize(exponent));
    myNumber *temp = MNinit(MNsize(a));
    myNumber *one = MNinit(1);
    int binaryExponentSize;

    MNconvert(exponent, binaryExponent, base, 2);
    charsToNumber("1", result, base);
    charsToNumber("1", one, base);
    binaryExponentSize = MNsize(binaryExponent);

    for (int i = binaryExponentSize - 1; i >= 0; i--) 
    {
        MNmultiply(result, result, temp, base);

        if (MNgetDigit(binaryExponent, i) == 1)
            MNmultiply(temp, a, result, base);
        else
            MNmultiply(temp, one, result, base);
    }

    MNdelete(binaryExponent);
    MNdelete(temp);
    MNdelete(one);

    return 0;
}
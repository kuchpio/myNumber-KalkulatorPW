#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "MNcalculations.h"
#include "myNumber.h"

#define MAX_INPUT_SIZE 100

enum OperationType {
    invalidOperation, 
    invalidConvertionNS,
    addition, 
    multiplication,
    division, 
    modulo, 
    exponentiation,
    conversion
};

enum ErrorType {
    noError,
    unknownOperation,
    invalidConvertionNumeralSystem,
    invalidNumeralSystem,
    invalidFirstNumberDigits,
    invalidSecondNumberDigits, 
    divisionByZero
};

enum OperationType getOperation(char *input, unsigned char *conversionNumeralSystem); //sets convertionNumberSystem when convertion

unsigned char getNumeralSystem(char *input); //returns number system from <2;16> or 0 for numeral system out of range or 1 for invalid characters

int main(int argc, char **argv) {
    char input[MAX_INPUT_SIZE], outputFileName[MAX_INPUT_SIZE], outputDirectory[MAX_INPUT_SIZE], separateOutput;
    char firstNumberString[MAX_INPUT_SIZE], secondNumberString[MAX_INPUT_SIZE], *resultString; 
    FILE *instructionFile, *outputFile;
    uint inputFileLine = 1, calculationNumber = 1;
    unsigned char numeralSystem, convertionNumeralSystem = 0;

    enum ErrorType errorType;
    enum OperationType operationType;
    myNumber *firstNumber, *secondNumber, *result;

    //open instruction file
    do {

        printf("Name of instructions file: ");
        scanf("%s", input);

        instructionFile = fopen(input, "r");
        
        if (instructionFile == NULL) 
        {
            printf("Could not open '%s'.\n", input);
        } 
        else 
        {
            break;
        }

    } while (1);

    //get info about output file(s)
    do {

        printf("Write results into separate files? (y/n): ");
        getchar();
        separateOutput = (unsigned char) getchar();

        if (separateOutput == 'y' || separateOutput == 'Y') 
        {
            separateOutput = 'y';

            printf("Name of results directory: ");
            scanf("%s", outputDirectory);

            if (stat(outputDirectory, NULL) == -1)
                mkdir(outputDirectory, 0700);
            
            break;
        } 
        else if (separateOutput == 'n' || separateOutput == 'N') 
        {
            separateOutput = 'n';

            printf("Name of results file: ");
            scanf("%s", input);

            outputFile = fopen(input, "w");

            break;
        }

    } while (1);

    //read from file, calculate, write to output
    while (fscanf(instructionFile, "%s", input) != -1) 
    {
        errorType = noError;

        if (separateOutput == 'y') {
            char outputPath[MAX_INPUT_SIZE + 17];
            sprintf(outputPath, "%s/%d.txt", outputDirectory, calculationNumber);
            outputFile = fopen(outputPath, "w");
        }

        //handle operation/convertion numeral system
        fprintf(outputFile, "%s ", input);
        operationType = getOperation(input, &convertionNumeralSystem);
        

        if (operationType == invalidOperation) 
        {
            printf("\033[1;31mError\033[0m: Unknown operation: '%s' (line: %d)\n", input, inputFileLine);
            errorType = invalidOperation;
        } 
        else if (operationType == invalidConvertionNS) 
        {
            printf("\033[1;31mError\033[0m: Invalid convertion numeral system: '%s' (line: %d)\n", input, inputFileLine);
            errorType = invalidConvertionNumeralSystem;
        }

        //handle numeral system
        fscanf(instructionFile, "%s", input);
        fprintf(outputFile, "%s", input);
        numeralSystem = getNumeralSystem(input);

        if (numeralSystem == 0 || numeralSystem == 1) 
        {
            printf("\033[1;31mError\033[0m: Invalid numeral system: '%s' (line: %d)\n", input, inputFileLine);
            errorType = invalidNumeralSystem;
        }

        inputFileLine += 2;

        //init first number
        fscanf(instructionFile, "%s", firstNumberString);
        fprintf(outputFile, "\n\n%s", firstNumberString);
        firstNumber = MNinit(strlen(firstNumberString));

        if (charsToNumber(firstNumberString, firstNumber, numeralSystem) != 0)
        {
            printf("\033[1;31mError\033[0m: Invalid digit(s) in '%s' for base %d numeral system (line: %d)\n", firstNumberString, numeralSystem, inputFileLine);
            errorType = invalidFirstNumberDigits;
        }

        //init second number
        if (operationType != conversion) 
        {  
            inputFileLine += 2;

            fscanf(instructionFile, "%s", secondNumberString);
            fprintf(outputFile, "\n\n%s", secondNumberString);
            secondNumber = MNinit(strlen(secondNumberString));

            if (charsToNumber(secondNumberString, secondNumber, numeralSystem) != 0) 
            {
                printf("\033[1;31mError\033[0m: Invalid digit(s) in '%s' for base %d numeral system (line: %d)\n", secondNumberString, numeralSystem, inputFileLine);
                errorType = invalidSecondNumberDigits;
            }
        }

        fprintf(outputFile, "\n\n");

        //calculate
        if (errorType == noError) 
        {
            switch (operationType)
            {
            case addition:
                
                result = MNinit(MNsize(firstNumber) > MNsize(secondNumber) ? MNsize(firstNumber) + 1: MNsize(secondNumber) + 1);
                MNadd(firstNumber, secondNumber, result, numeralSystem);
                break;

            case multiplication:

                result = MNinit(MNsize(firstNumber) + MNsize(secondNumber));
                MNmultiply(firstNumber, secondNumber, result, numeralSystem);
                break;

            case division:

                result = MNinit(MNsize(firstNumber));
                if(MNdivide(firstNumber, secondNumber, result, NULL, numeralSystem) == -1) 
                {
                    printf("\033[1;31mError\033[0m: Can't divide by 0! (line: %d)\n", inputFileLine);
                    errorType = divisionByZero;
                }
                break;

            case modulo:

                result = MNinit(MNsize(secondNumber));
                if(MNdivide(firstNumber, secondNumber, NULL, result, numeralSystem) == -1) 
                {
                    printf("\033[1;31mError\033[0m: Can't divide by 0! (line: %d)\n", inputFileLine);
                    errorType = divisionByZero;
                }
                break;

            case exponentiation:
                
                result = MNinit(MNsize(firstNumber));
                MNraise(firstNumber, secondNumber, result, numeralSystem);
                break;

            case conversion:

                result = MNinit(4 * MNsize(firstNumber)); //max for convertion (16) -> (2)
                MNconvert(firstNumber, result, numeralSystem, convertionNumeralSystem);
                break;
            
            default:
                break;
            }
        }

        //write result to file
        switch (errorType)
        {
        case noError:

            resultString = numberToChars(result);
            fprintf(outputFile, "%s\n\n", resultString);
            MNdelete(result);
            free(resultString);
            break;
        
        case unknownOperation:

            fprintf(outputFile, "Unknown operation\n\n");
            break;

        case invalidConvertionNumeralSystem:

            fprintf(outputFile, "Invalid convertion numeral system\n\n");
            break;

        case invalidNumeralSystem:

            fprintf(outputFile, "Invalid numeral system\n\n");
            break;

        case invalidFirstNumberDigits:

            fprintf(outputFile, "Invalid digit(s) in '%s' for base %d numeral system\n\n", firstNumberString, numeralSystem);
            break;

        case invalidSecondNumberDigits:

            fprintf(outputFile, "Invalid digit(s) in '%s' for base %d numeral system\n\n", secondNumberString, numeralSystem);
            break;

        case divisionByZero:

            fprintf(outputFile, "Can not divide by 0!\n\n");
            MNdelete(result);
            break;

        default:
            break;
        }

        //cleanup
        inputFileLine += 3;
        calculationNumber++;
        MNdelete(firstNumber);
        if (operationType != conversion)
            MNdelete(secondNumber);

        if (separateOutput == 'y')
            fclose(outputFile);
    } 

    fclose(instructionFile);
    if (separateOutput == 'n')
        fclose(outputFile);

    return 0;
}

enum OperationType getOperation(char *input, unsigned char *conversionNumeralSystem)
{
    if (strlen(input) == 1) 
    {
        switch (input[0])
        {
        case '+':
            return addition;
        
        case '*':
            return multiplication;

        case '/':
            return division;

        case '^':
            return exponentiation;

        case '%':
            return modulo;

        default:
            break;
        }
    }

    unsigned char numeralSystem = getNumeralSystem(input);

    if (numeralSystem == 0)
        return invalidConvertionNS;

    if (numeralSystem != 1) {
        *conversionNumeralSystem = numeralSystem;
        return conversion;
    }

    return invalidOperation;
}

unsigned char getNumeralSystem(char *input) //returns numeral system from range <2;16> or 0 if out of range or 1 if invalid characters
{ 
    int inputLength = strlen(input);
    uint numeralSystem = 0, a = 1;

    for (int i = inputLength - 1; i >= 0; i--) 
    {
        if (input[i] < '0' || input[i] > '9')
            return 1; //error - invalid character

        numeralSystem += a * (input[i] - '0');
        a *= 10;
    }

    if (numeralSystem < 2 || numeralSystem > 16)
        return 0; //error - numeral system out of range

    return (unsigned char) numeralSystem;
}
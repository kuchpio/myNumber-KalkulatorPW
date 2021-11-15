#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <direct.h>
#elif defined __linux__
    #include <sys/stat.h>
    #define _mkdir(outputDirectory) mkdir(outputDirectory, 0700)
#endif

#include "MNcalculations.h"
#include "myNumber.h"

#define MAX_NUMBER_SIZE 16000
#define MAX_PATH_SIZE 76

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
    invalidNumeralSystem,
    invalidFirstNumberDigits,
    invalidSecondNumberDigits, 
    divisionByZero
};

enum OperationType getOperation(char *input, unsigned char *conversionNumeralSystem); //sets convertionNumberSystem when convertion

unsigned char getNumeralSystem(char *input); //returns number system from <2;16> or 0 for numeral system out of range or 1 for invalid characters

int main(int argc, char **argv) 
{
    char inputPathToFile[MAX_PATH_SIZE], inputFromFile[MAX_NUMBER_SIZE], inputFileAsArg = 0;
    char outputPathToFile[MAX_PATH_SIZE + 17], outputDirectory[MAX_PATH_SIZE], separateOutput, *resultString;
    FILE *instructionFile = NULL, *outputFile = NULL;
    unsigned int inputFileLine = 1, calculationNumber = 1, errorCounter = 0;
    unsigned char numeralSystem, conversionNumeralSystem = 0;

    enum ErrorType errorType = noError;
    enum OperationType operationType;
    myNumber *firstNumber, *secondNumber = NULL, *result = NULL;

    //open instruction file from argv[1]
    if (argc == 2) 
    {
        instructionFile = fopen(argv[1], "r");

        if (instructionFile == NULL)
            printf(" <ERROR>: Could not open '%s'.\n", argv[1]);
        else
            inputFileAsArg = 1;
    }

    //or open instruction file from user input
    if (!inputFileAsArg)
    {
        while (1) 
        {
            printf(" (KALKULATOR): Name of instructions file: ");
            scanf("%s", inputPathToFile);
            getchar(); //remove \n from buffer

            instructionFile = fopen(inputPathToFile, "r");

            if (instructionFile == NULL)
                printf(" <ERROR>: Could not open '%s'.\n", inputPathToFile);
            else
                break;
        }
    }

    //get info about output file(s)
    while (1) 
    {
        printf(" (KALKULATOR): Write results into separate files? (y/n): ");
        separateOutput = (unsigned char) getchar();

        if (separateOutput == 'y' || separateOutput == 'Y') //separate output files
        {
            separateOutput = 'y';

            printf(" (KALKULATOR): Name of results directory: ");
            scanf("%s", outputDirectory);

            _mkdir(outputDirectory);
            
            break;
        } 
        else if (separateOutput == 'n' || separateOutput == 'N') //single output file
        {
            separateOutput = 'n';

            printf(" (KALKULATOR): Name of results file: ");
            scanf("%s", outputPathToFile);

            outputFile = fopen(outputPathToFile, "w");

            break;
        }
    }

    //read from file, calculate, write to output
    while (fscanf(instructionFile, "%s", inputFromFile) != -1) 
    {
        printf(" (KALKULATOR): Working on calculation %d\n", calculationNumber);

        //open new file when separate output
        if (separateOutput == 'y') 
        {
            sprintf(outputPathToFile, "%s/%d.txt", outputDirectory, calculationNumber);
            outputFile = fopen(outputPathToFile, "w");
        }

        //handle operation/convertion numeral system base
        fprintf(outputFile, "%s ", inputFromFile);
        operationType = getOperation(inputFromFile, &conversionNumeralSystem);
        
        if (operationType == invalidOperation) 
        {
            printf(" <ERROR>: Unknown operation: '%s' (line: %d)\n", inputFromFile, inputFileLine);
            errorType = unknownOperation;
        } 
        else if (operationType == invalidConvertionNS) 
        {
            printf(" <ERROR>: Invalid numeral system: '%s' (line: %d)\n", inputFromFile, inputFileLine);
            errorType = invalidNumeralSystem;
        }

        //handle numeral system base
        fscanf(instructionFile, "%s", inputFromFile);
        fprintf(outputFile, "%s", inputFromFile);
        numeralSystem = getNumeralSystem(inputFromFile);

        if (numeralSystem == 0 || numeralSystem == 1) 
        {
            printf(" <ERROR>: Invalid numeral system: '%s' (line: %d)\n", inputFromFile, inputFileLine);
            numeralSystem = 0;
            errorType = errorType == noError ? invalidNumeralSystem : errorType;
        }

        inputFileLine += 2;

        //init first number
        fscanf(instructionFile, "%s", inputFromFile);
        fprintf(outputFile, "\n\n%s", inputFromFile);
        firstNumber = MNinit(strlen(inputFromFile));

        if (charsToNumber(inputFromFile, firstNumber, operationType == conversion ? conversionNumeralSystem: numeralSystem) != 0)
        {
            printf(" <ERROR>: Invalid digit(s) in '%s' for base %d numeral system (line: %d)\n", inputFromFile, numeralSystem, inputFileLine);
            errorType = errorType == noError ? invalidFirstNumberDigits : errorType;
        }

        //init second number
        if (operationType != conversion) 
        {  
            inputFileLine += 2;

            fscanf(instructionFile, "%s", inputFromFile);
            fprintf(outputFile, "\n\n%s", inputFromFile);
            secondNumber = MNinit(strlen(inputFromFile));

            if (charsToNumber(inputFromFile, secondNumber, numeralSystem) != 0)
            {
                printf(" <ERROR>: Invalid digit(s) in '%s' for base %d numeral system (line: %d)\n", inputFromFile, numeralSystem, inputFileLine);
                errorType = errorType == noError ? invalidSecondNumberDigits : errorType;
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

                if(MNsize(secondNumber) == 0) 
                {
                    printf(" <ERROR>: Division by 0 is undefined (line: %d)\n", inputFileLine);
                    errorType = errorType == noError ? divisionByZero : errorType;
                    break;
                }

                MNdivide(firstNumber, secondNumber, result, NULL, numeralSystem); 
                break;

            case modulo:

                result = MNinit(MNsize(firstNumber));

                if (MNsize(secondNumber) == 0)
                {
                    printf(" <ERROR>: Division by 0 is undefined (line: %d)\n", inputFileLine);
                    errorType = errorType == noError ? divisionByZero : errorType;
                    break;
                }

                MNdivide(firstNumber, secondNumber, NULL, result, numeralSystem);
                break;

            case exponentiation:
                
                result = MNinit(MNsize(firstNumber));
                MNraise(firstNumber, secondNumber, result, numeralSystem);
                break;

            case conversion:

                result = MNinit(4 * MNsize(firstNumber)); //max for convertion (16) -> (2)
                MNconvert(firstNumber, result, conversionNumeralSystem, numeralSystem);
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
            fprintf(outputFile, "%s", resultString);
            MNdelete(result);
            free(resultString);
            break;
        
        case unknownOperation:

            fprintf(outputFile, "Unknown operation");
            break;

        case invalidNumeralSystem:

            fprintf(outputFile, "Invalid numeral system");
            break;

        case invalidFirstNumberDigits:

            fprintf(outputFile, "Invalid digit(s) in first number");
            break;

        case invalidSecondNumberDigits:

            fprintf(outputFile, "Invalid digit(s) in second number");
            break;

        case divisionByZero:

            fprintf(outputFile, "Division by 0 is undefined");
            MNdelete(result);
            break;
        }
        fprintf(outputFile, "\n\n\n");

        //cleanup
        if (errorType != noError)
            errorCounter++;

        if (separateOutput == 'y')
            fclose(outputFile);

        MNdelete(firstNumber);
        if (operationType != conversion)
            MNdelete(secondNumber);

        errorType = noError;
        inputFileLine += 3;
        calculationNumber++;
    } 

    fclose(instructionFile);
    if (separateOutput == 'n')
        fclose(outputFile);
    
    if (errorCounter > 0)
        printf("\n (KALKULATOR): Program found errors in %d calculation(s). Press any key to exit the program.", errorCounter);
    else
        printf("\n (KALKULATOR): Program successfully performed all calculations. Press any key to exit the program.");

    getchar();
    getchar();
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
    unsigned int numeralSystem = 0, a = 1;

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
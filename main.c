#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
# include <direct.h>
#elif defined __linux__
# include <sys/stat.h>
# define _mkdir(outputDirectory) mkdir(outputDirectory, 0700)
#endif

#include "MNcalculations.h"
#include "myNumber.h"

#define PREFERABLE_INPUT_SIZE 32

enum OperationType 
{
    Addition,
    Multiplication,
    Division,
    Modulo,
    Exponentiation,
    Conversion, 
    UnknownOperation
};

enum OperationErrors
{
    invalidNumeralSystem = 1,
    invalidConversionNumeralSystem = 1 << 1,
    invalidFirstNumberDigits = 1 << 2,
    invalidSecondNumberDigits = 1 << 3,
    divisionByZero = 1 << 4,
};

char readFromStream(FILE* inputFile, char** dest, size_t destSize);

int isCharInString(char c, const char* string);

enum OperationType getOperationType(const char* operationString);

unsigned char getNumeralSystem(char* input);

int isFormatCorrect(const char* filename, const char* format);

int getYNAnswer(const char* question);

int main(int argc, char** argv)
{
    int separateOutputMode;
    char *pathToInputFile = NULL, *pathToOutputDirectory = NULL, *pathToOutputFile = NULL;
    FILE *inputFile = NULL, *outputFile = NULL;
    
    unsigned int inputFileLine = 1, calculationNumber = 1, errorFlags = 0, wroteToOutput = 0, errorCounter = 0, fatal = 0;
    char *operatorString = NULL, *numeralSystemString = NULL, *firstNumberString = NULL, *secondNumberString = NULL, *resultString = NULL;
    enum OperationType operationType;
    unsigned char numeralSystem, conversionNumeralSystem = 0;
    myNumber *firstNumber = NULL, *secondNumber = NULL, *result = NULL;
    time_t calcStart, calcEnd;

    //open instruction file from argv[1]
    if (argc == 2)
    {
        inputFile = fopen(argv[1], "r");

        if (inputFile == NULL)
            fprintf(stdout, " (KALKULATOR): Could not open '%s'.\n", argv[1]);
    }

    //or open input file from user input
    if (inputFile == NULL)
    {
        while (1)
        {
            fprintf(stdout, " (KALKULATOR): Name of instructions file: ");
            readFromStream(stdin, &pathToInputFile, PREFERABLE_INPUT_SIZE);

            if (inputFile = fopen(pathToInputFile, "r"))
                break;

            fprintf(stdout, " (KALKULATOR): Could not open '%s'.\n", pathToInputFile);
        }
        free(pathToInputFile);
    }

    //get output mode
    separateOutputMode = getYNAnswer(" (KALKULATOR): Write results into separate files? (y/n): ");

    if (separateOutputMode) //separate output files
    {
        fprintf(stdout, " (KALKULATOR): Name of output directory: ");
        readFromStream(stdin, &pathToOutputDirectory, PREFERABLE_INPUT_SIZE);

        _mkdir(pathToOutputDirectory);
    }
    else //single output file
    {
        while (1)
        {
            fprintf(stdout, " (KALKULATOR): Name of output file: ");
            readFromStream(stdin, &pathToOutputFile, PREFERABLE_INPUT_SIZE);

#ifdef _WIN32 //ensure .txt file on windows builds
            if (isFormatCorrect(pathToOutputFile, ".txt")) break;
#else
            break;
#endif   

            fprintf(stdout, " (KALKULATOR): Name of output file must end with '.txt'.\n");
        }
        outputFile = fopen(pathToOutputFile, "w");
    }

    //main loop
    while (1)
    {
        //read and handle operator
        if (readFromStream(inputFile, &operatorString, PREFERABLE_INPUT_SIZE))
            break;

        //read numeral system base / conversion numeral system base
        if (readFromStream(inputFile, &numeralSystemString, PREFERABLE_INPUT_SIZE))
            break;

        //read first number
        if (readFromStream(inputFile, &firstNumberString, PREFERABLE_INPUT_SIZE))
            break;

        //handle operation
        operationType = getOperationType(operatorString);
        if (operationType == UnknownOperation) 
        {
            fprintf(stdout, " (KALKULATOR): Working on calculation %u (line: %u) ... ", calculationNumber, inputFileLine);
            fprintf(stderr, "\n\t<ERROR>: Unknown operation: '%s'.\n", operatorString);
            fatal = 1;
            break;
        }
        
        //read second number
        if (operationType != Conversion && readFromStream(inputFile, &secondNumberString, PREFERABLE_INPUT_SIZE))
            break;

        fprintf(stdout, " (KALKULATOR): Working on calculation %u (line: %u) ... ", calculationNumber, inputFileLine);

        //handle input, check for errors
        if (operationType == Conversion) //conversion
        {
            //handle numeralSystem
            numeralSystem = getNumeralSystem(operatorString);
            errorFlags |= numeralSystem == 0 ? invalidNumeralSystem : 0;

            //hande conversionNumeralSystem
            conversionNumeralSystem = getNumeralSystem(numeralSystemString);
            errorFlags |= conversionNumeralSystem == 0 ? invalidConversionNumeralSystem: 0;

            //handle first number
            firstNumber = MNinit(strlen(firstNumberString));
            errorFlags |= charsToNumber(firstNumberString, firstNumber, numeralSystem) == -1 ? invalidFirstNumberDigits : 0;
        }
        else //other operations
        {
            //handle numeral system base
            numeralSystem = getNumeralSystem(numeralSystemString);
            errorFlags |= numeralSystem == 0 ? invalidNumeralSystem: 0;

            //handle first number
            firstNumber = MNinit(strlen(firstNumberString));
            errorFlags |= charsToNumber(firstNumberString, firstNumber, numeralSystem) == -1 ? invalidFirstNumberDigits: 0;

            //handle second number
            secondNumber = MNinit(strlen(secondNumberString));
            errorFlags |= charsToNumber(secondNumberString, secondNumber, numeralSystem) == -1 ? invalidSecondNumberDigits : 0;
        }

        //perform operation
        if (errorFlags == 0)
        {
            calcStart = clock();
            switch (operationType)
            {
            case Addition:

                result = MNinit(MNsize(firstNumber) > MNsize(secondNumber) ? MNsize(firstNumber) + 1 : MNsize(secondNumber) + 1);
                MNadd(firstNumber, secondNumber, result, numeralSystem);
                break;

            case Multiplication:

                result = MNinit(MNsize(firstNumber) + MNsize(secondNumber));
                MNmultiply(firstNumber, secondNumber, result, numeralSystem);
                break;

            case Division:

                result = MNinit(MNsize(firstNumber)); //TODO optimize init size
                if (MNsize(secondNumber) == 0)
                    errorFlags |= divisionByZero;
                else
                    MNdivide(firstNumber, secondNumber, result, NULL, numeralSystem);
                break;

            case Modulo:

                result = MNinit(MNsize(firstNumber)); //TODO optimize init size
                if (MNsize(secondNumber) == 0)
                    errorFlags |= divisionByZero;
                else
                    MNdivide(firstNumber, secondNumber, NULL, result, numeralSystem);
                break;

            case Exponentiation:

                result = MNinit(MNsize(firstNumber));
                MNraise(firstNumber, secondNumber, result, numeralSystem);
                break;

            case Conversion:

                result = MNinit(4 * MNsize(firstNumber)); //max for convertion (16) -> (2)
                MNconvert(firstNumber, result, numeralSystem, conversionNumeralSystem);
                break;
            
            }
            calcEnd = clock();
        }

        //open new file when separate output
        if (separateOutputMode)
        {
            pathToOutputFile = (char*)malloc((strlen(pathToOutputDirectory) + 17) * sizeof(char));
            sprintf(pathToOutputFile, "%s/%u.txt", pathToOutputDirectory, calculationNumber);
            outputFile = fopen(pathToOutputFile, "w");
        }

        //write to file
        if (!separateOutputMode && wroteToOutput)
            fprintf(outputFile, "\n\n");
        else
            wroteToOutput = 1;

        fprintf(outputFile, "%s %s\n\n%s", operatorString, numeralSystemString, firstNumberString);
        if (operationType != Conversion)
            fprintf(outputFile, "\n\n%s", secondNumberString);

        fprintf(outputFile, "\n\n");

        //write result
        if (errorFlags == 0)
        {
            resultString = numberToChars(result);
            fprintf(outputFile, "%s", resultString);
            fprintf(stdout, "Done in %gs.\n", ((float)(calcEnd - calcStart)) / CLOCKS_PER_SEC);
        }
        else //write errors
        {
            fprintf(stdout, "\n");
            fprintf(outputFile, "ERR: ");
            if (errorFlags & invalidNumeralSystem)
            {
                fprintf(outputFile, "Invalid numeral system. ");
                fprintf(stderr, "\t[WARNING]: Invalid numeral system: '%s'.\n", operationType == Conversion ? operatorString : numeralSystemString);
            }
            if (errorFlags & invalidConversionNumeralSystem)
            {
                fprintf(outputFile, "Invalid conversion numeral system. ");
                fprintf(stderr, "\t[WARNING]: Invalid conversion numeral system: '%s'.\n", numeralSystemString);
            }
            if ((errorFlags & invalidFirstNumberDigits) && !(errorFlags & invalidNumeralSystem))
            {
                fprintf(outputFile, "Invalid digit(s) in first number. ");
                fprintf(stderr, "\t[WARNING]: Invalid digit(s) in '%s' for base %u numeral system.\n", firstNumberString, numeralSystem);
            }
            if ((errorFlags & invalidSecondNumberDigits) && !(errorFlags & invalidNumeralSystem))
            {
                fprintf(outputFile, "Invalid digit(s) in second number. ");
                fprintf(stderr, "\t[WARNING]: Invalid digit(s) in '%s' for base %u numeral system.\n", secondNumberString, numeralSystem);
            }
            if (errorFlags & divisionByZero)
            {
                fprintf(outputFile, "Division by 0 is undefined. ");
                fprintf(stderr, "\t[WARNING]: Division by 0 is undefined.\n");
            }
            fprintf(stderr, "\t[WARNING]: Ommiting calculation execution due to incorrect data.\n");
            errorCounter++;
        }

        //cleanup
        if (separateOutputMode)
        {
            free(pathToOutputFile);
            fclose(outputFile);
        }

        MNdelete(firstNumber);

        if (operationType != Conversion)
            MNdelete(secondNumber);

        if (errorFlags == 0)
        {
            MNdelete(result);
            free(resultString);
        }

        errorFlags = 0;
        calculationNumber++;
        inputFileLine += operationType == Conversion ? 5 : 7;
    }

    //cleanup
    fclose(inputFile);

    if (!separateOutputMode) 
    {
        free(pathToOutputFile);
        fclose(outputFile);
    }
    else
        free(pathToOutputDirectory);

    if (operatorString)
        free(operatorString);

    if (numeralSystemString)
        free(numeralSystemString);

    if (firstNumberString)
        free(firstNumberString);

    if (secondNumberString)
        free(secondNumberString);

    if (fatal)
    {
        fprintf(stdout, "\n (KALKULATOR): Program could not complete all calculations due to error(s). Press any key to exit the program.\a\n");
        getchar();

        return 0;
    }

    if (errorCounter > 0)
        fprintf(stdout, "\n (KALKULATOR): Program has found errors in %u calculation(s) but successfully ommited them. Press any key to exit the program.\n", errorCounter);
    else
        fprintf(stdout, "\n (KALKULATOR): Program has successfully completed all calculations. Press any key to exit the program.\n");

    getchar();
    
    return 0;
}

char readFromStream(FILE *inputFile, char **dest, size_t destSize)
{
    char lastCharacter = fgetc(inputFile), *tmp;
    size_t i = 0;

    while (isCharInString(lastCharacter, " \n\t\r")) //ommiting whitespaces
        lastCharacter = fgetc(inputFile);

    if (lastCharacter == EOF)
        return -1;

    destSize = destSize == 0 ? 1 : destSize;
    if (*dest == NULL)
        *dest = (char*)malloc(destSize * sizeof(char));
    else
    {
        tmp = (char*)realloc(*dest, destSize * sizeof(char));
        if (tmp == NULL)
            return -1;

        *dest = tmp;
    }

    while (!isCharInString(lastCharacter, " \n\t\r") && lastCharacter != EOF) //stop reading on whitespace or EOF
    {
        if (destSize <= i) //grow dest if no memory left
        {
            tmp = (char*)realloc(*dest, 2 * destSize * sizeof(char));
            if (tmp == NULL)
                return -1;
            
            destSize *= 2;
            *dest = tmp;
        }

        *(*dest + i) = lastCharacter;
        i++;

        lastCharacter = fgetc(inputFile);
    }

    if (destSize <= i) //grow dest if no memory left
    {
        tmp = (char*)realloc(*dest, (destSize + 1) * sizeof(char));
        if (tmp == NULL)
            return -1;

        destSize *= 2;
        *dest = tmp;
    }
    
    *(*dest + i) = '\0';

    return 0;
}

int isCharInString(char c, const char* string) 
{
    size_t len = strlen(string);
    for (size_t i = 0; i < len; i++)
        if (c == string[i])
            return 1;

    return 0;
}

enum OperationType getOperationType(const char* operationString)
{
    size_t len = strlen(operationString);
    if (len == 1) {
        switch (operationString[0])
        {
        case '+':
            return Addition;

        case '*':
            return Multiplication;

        case '/':
            return Division;

        case '%':
            return Modulo;

        case '^':
            return Exponentiation;
        }
    }
    for (size_t i = 0; i < len; i++) 
    {
        if (operationString[i] < '0' || operationString[i] > '9')
            return UnknownOperation;
    }
    return Conversion;
}

unsigned char getNumeralSystem(char* input) //returns numeral system base from range <2;16> or 0 if error
{
    size_t inputLength = strlen(input);
    unsigned int numeralSystem = 0, a = 1;

    if (inputLength > 9)
        return 0;

    for (long long i = inputLength - 1; i >= 0; i--)
    {
        if (input[i] < '0' || input[i] > '9')
            return 0; //error - invalid character

        numeralSystem += a * (input[i] - '0');
        a *= 10;
    }

    if (numeralSystem < 2 || numeralSystem > 16)
        return 0; //error - numeral system out of range

    return (unsigned char)numeralSystem;
}

int isFormatCorrect(const char* filename, const char* format)
{
    size_t filenameSize = strlen(filename);
    size_t formatStringSize = strlen(format);

    if (filenameSize < formatStringSize)
        return 0;

    return !memcmp(filename + filenameSize - formatStringSize, format, formatStringSize * sizeof(char));
}

int getYNAnswer(const char* question)
{
    char answer;
    while (1)
    {
        fprintf(stdout, question);
        answer = (unsigned char)getchar();

        if (isCharInString(answer, "yY"))
            return 1;

        if (isCharInString(answer, "nN"))
            return 0;

        while (getchar() != '\n'); //clear input buffer
    }
}
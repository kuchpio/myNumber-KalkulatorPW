## Overview

This project provides a library in `C` for basic arithmetic operations on big integer numbers represented in different numerical systems from range $<2, 16>$. These operations are:
 - addition (`+`), 
 - subtraction (`-`), 
 - multiplication (`*`), 
 - exponentiation (`^`), 
 - division (`/`), 
 - modulo (`%`), 
 - conversion beetween different numerical systems.

This library is used by the main program, which does all the calculations based on instructions present in an input file and writes results to an output file.

## Instalation

The code should work on both Windows and Unix OSs. It can be simply compiled using tools such as `gcc`:

```sh
$ gcc *.c -o kalkulator
```

## Usage

#### Library

Every number is stored in `myNumber` structure defined in `myNumber.h` as

```c
typedef struct {
    unsigned char *digits; 
    size_t allocatedMemory;
} myNumber;
```

Digits are stored in little endian order. File `myNumber.h` provides basic operations on single `myNumber`. File `MNcalculations.h` provides operaitons on pairs of `myNumber` and numerical system conversion operation.

#### Program

Program takes an input file that contains operations on big integer numbers coded as

```
[operator] [numerical system]

[first number]

[second number]
```

or
```
[number numerical system] [conversion result numerical system]

[number]
```
in case of numerical system conversion. It performs described operations and writes results into either a specified output file or separate output files for each operation.

##### Example
For an input file that looks like
```
+ 10

39

4820


/ 10

7891

0


```
the results will look like
```
+ 10

39

4820

4859

/ 10

7891

0

Division by 0 is undefined
```

## Other information

This was my first semester college project. I don't work on it anymore, but feel free to use it under [MIT](https://mit-license.org/) License.

// cstdio is the C++ standard library header that provides functionalities for 
// input and output operations
// It is the C++ version of the C standard library header <stdio.h>
// It includes functions for file handling, formatted input/output, and other
// related operations (like printf, scanf, fopen, fclose, etc.)
// In C++, the C standard library headers are included with a 'c' prefix and
// without the '.h' suffix
// For example, <stdio.h> in C becomes <cstdio> in C++
// This header is part of the C++ Standard Library and is included in the C++14
// language standard version
#include <cstdio>
// Include the header file for math functions
// note: the header file is included with double quotes because it is a user-defined header file
// The angle brackets <> are used for standard library headers
// The double quotes "" are used for user-defined or local header files
#include "mathtools.h"
// main() function definition
// This is the one and only entry point of the program
// It takes command-line arguments and returns an integer status code
// argc: command line argument count
// argv: command line argument vector (array of C-strings)
// example usage: ./program arg1 arg2
// In this example, argc would be 3 and argv would contain ["./program", "arg1", "arg2"]
// The return value of main() is typically 0 for success and non-zero for failure
int main(int argc, char* argv[]) {
    // printf() is a standard C function to print formatted output to the stdout
    // where default stdout is the console
    printf("Hello, World!\n"); 
    int x = 13;
    int y = 5;
    double z = add(x,y);
    printf("Sum of %d and %d is %f\n", x, y, z);
    z = subtract(x,y);
    printf("Difference of %d and %d is %f\n", x, y, z);
    z = multiply(x,y);
    printf("Product of %d and %d is %f\n", x, y, z);
    z = divide(x,y);
    // %f : format specifier for floating-point numbers (default 6 decimal places)
    // %.2f : format specifier for floating-point numbers with 2 decimal places
    // %g : format specifier for floating-point numbers (uses %f or %e based on value and precision)
    // Here, %g is used to print the result in a more compact form
    // If the number is very large or very small, it will use scientific notation
    // Otherwise, it will use standard decimal notation
    // This is useful for avoiding unnecessary trailing zeros or overly long decimal representations
    printf("Division of %d by %d is %f\n", x, y, z);
    printf("Division of %d by %d is %.2f\n", x, y, z);
    printf("Division of %d by %d is %g\n", x, y, z);
    printf("Division of %d by %d is %e\n", x, y, z);
    return 0; //exit status
}

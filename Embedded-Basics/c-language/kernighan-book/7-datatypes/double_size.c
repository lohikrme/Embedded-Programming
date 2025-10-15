//  Exercise 2-1. Write a program to determine 
// the ranges of char, short, int, and long variables
// both signed and unsigned
// harder if u compute them: determine the ranges of the various floating-point types

// why the double is so big? 
// because in standard 64 bit doubles are stored next:
// 1 bit for + or - sign
// 10 or 11 bits for exponent (up to 1024 or 2048)
// mantissa about 50 bits, defines accurasy of the integer part
// so, double is not 100% precise, but gives rly large approx values

#include <stdio.h>
#include <float.h>

int main() {
    // float min and max
    printf("%.3f\n", FLT_MIN);
    printf("%.3f\n", FLT_MAX);
    // double min and max
    printf("%.3f\n", DBL_MIN);
    printf("%.3f\n\n\n", DBL_MAX);

    double value = DBL_MAX;
    int amount_of_bits = 0;
    printf("%.3f", value);
    while (value > 1) {
        value = value / 2;
        printf("%.3f\n", value);
        amount_of_bits ++;
    }
    printf("Double has %d bits to make up numbers \n", amount_of_bits);
}
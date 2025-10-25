// main.cpp

// tell to program there is exisitng function addOne elsewhere
// old way was extern int but no more used:
// extern int addOne(int x);

// official header files in <>
#include <stdio.h>
// own header files in quotes
#include "addOne.h"
#include "addTwo.h"

// my program p1 p2 p3
int main(int argc, char *argv[]) {

    int x = 10;

    printf("argc: %d %s \n", argc, argv[0]);
    printf("Calling addOne with %d -> %d\n", x, addOne(x));
    printf("Calling addTwo with %d -> %d\n", x, addTwo(x));

    return 0;
}
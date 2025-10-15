// for (i=0; i < lim-1 && (c=getchar()) != '\n' && c != EOF; ++i)
//       s[i] = c;

// Exercise 2-2. Write a loop equivalent to the for loop above without using && or ||.

#include <stdio.h>

#define lim 1000

int main() {
    char c, s[lim];
    int i, calculator = 0;

    while (calculator < lim-1) {
        c = getchar();
        if (c == '\n') {
            break;
        }
        if (c == EOF) {
            break;
        }
        s[calculator] = c;
        calculator++;
    }
        

    printf("\n%s", s);
    return 0;
}
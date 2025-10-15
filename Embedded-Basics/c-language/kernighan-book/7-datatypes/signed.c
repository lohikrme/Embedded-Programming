#include <stdio.h>

int main() {
    signed int a = -1;
    unsigned int b = 1;

    printf("Signed value is %d, unsigned value is %u...\n", a, b);

    if (a < b) {
        printf("-1 (signed) < 1 (unsigned): TRUE\n");
    } else {
        printf("-1 (signed) < 1 (unsigned): FALSE\n");
    }

    // Tulostetaan myös arvot eri tyyppeinä
    printf("signed char a = %d\n", a);
    printf("unsigned char a (cast) = %u\n", (unsigned int)a);
    printf("unsigned char b = %u\n", b);

    return 0;
}

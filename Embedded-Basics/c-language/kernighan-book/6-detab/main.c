// Exercise 1-20. Write a program detab that replaces tabs 
// in the input with the proper number
// of blanks to space to the next tab stop.

#include <stdio.h>
#include <string.h>

#define lim 100

int main() {
    char myString[lim];
    char myString2[lim];
    int j = 0;
    while (1) {
        j = 0;
        // receive input
        printf("Give your input: \n");
        // use -20 so if there are some tabs we can safely replace with multiple letters
        fgets(myString, lim-20, stdin);
        // replace tabs (\t) with 4x spaces
        for (int i = 0; i < strlen(myString); i++) {
            if (myString[i] == '\t') {
                switch (j % 8) {
                    case 0:
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        break;
                    case 1:
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        break;
                    case 2:
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        break;
                    case 3:
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        break;
                    case 4:
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        break;
                    case 5:
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        break;
                    case 6:
                        myString2[j++] = ' ';
                        myString2[j++] = ' ';
                        break;
                    case 7:
                        myString2[j++] = ' ';
                        break;
                }
                printf("%d\n", j);
            }
            else {
                myString2[j++] = myString[i];
            }
        }
        myString2[j++] = '\0';
        // print without tabs
        printf("%s \n", myString2);
        // now clean MyString2, so next round it starts clean as new
        memset(myString2, 0, sizeof(myString2));
    }
    return 0;
}
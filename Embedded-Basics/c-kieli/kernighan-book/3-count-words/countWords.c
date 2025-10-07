// calculate how many words and letters in file or input
// remember that getchar() receives characters from any input
// so u can start the main program to read file next:
// ./countWords < tiedosto.txt
// if u want userinput, just
// ./countWords

#include <stdio.h>

#define IN 1 // inside a word
#define OUT 0 // outside a word

int main() {
    // c = current character
    // n1 = number of rows
    // nw = number of words
    // nc = number of characters
    // current state in or out
    int c, n1, nw, nc, state;

    state = OUT;
    n1 = nw = nc = 0;
    while ((c = getchar()) != EOF) {
        ++nc;
        if (c == '\n') {
            ++n1;
        }
        if (c == ' ' || c == '\n' || c == '\t') {
            state = OUT;
        }
        else if (state == OUT) {
            state = IN;
            ++nw;
        }
    }
    printf("Number of rows: %d\nNumber of words: %d\nNumber of characters: %d\n", n1, nw, nc);
    return 0;
}
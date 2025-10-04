// main.c
// write program to count blanks, tabs and newlines
// from user input

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    // store statistics into these
    int blankCount = 0; 
    int tabCount = 0;
    int newlineCount = 0;
    // store user input string into array of characters
    char sentence[10000];
    printf("Give a sentence or a block of text: \n");
    fgets(sentence, sizeof(sentence), stdin);
    
    // count blanks, tabs and newlines
    for (int i = 0; i < strlen(sentence); i++) {
        switch(sentence[i]) {
            case ' ':
                blankCount++;
                break;
            case '\t':
                tabCount++;
                break;
            case '\n':
                newlineCount++;
                break;
        }
    }

    // print statistics:
    printf("Blank Count: %d\n", blankCount);
    printf("Tab Count: %d\n", tabCount);
    printf("New Line Count: %d\n", newlineCount);
    return 0;
}
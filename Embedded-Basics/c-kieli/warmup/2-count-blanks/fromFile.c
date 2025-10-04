// main.c
// write program to count blanks, tabs and newlines
// from user input

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    // store statistics into these
    int blankCount = 0; 
    int tabCount = 0;
    // assume always 1 line and \n adds a new line
    int lineCount = 1;

    // initiate a pointer for file
    FILE *fptr;

    // open the file to read
    fptr = fopen("lotr-story.txt", "r");
    if (fptr == NULL) {
        perror("File not found");
    return 1;
}


    // initiate an array of characters for each row of text file
    char sentence[5000];
    // one row at time from the file
    while(fgets(sentence, 5000, fptr)) {
        // count blanks, tabs and newlines per row of file
        for (int i = 0; i < strlen(sentence); i++) {
            switch (sentence[i]) {
                case ' ':
                    blankCount++;
                    break;
                case '\t':
                    tabCount++;
                    break;
                case '\n':
                    lineCount++;
                    break;
            }
        }
    };
    fclose(fptr);

    // print statistics:
    printf("Blank Count: %d\n", blankCount);
    printf("Tab Count: %d\n", tabCount);
    printf("Line Count: %d\n", lineCount);
    return 0;
}
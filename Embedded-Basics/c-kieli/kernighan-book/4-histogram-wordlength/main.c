
#include <stdio.h>
#include <ctype.h>

int main() {
    // initiate character and wordLength
    int c, wordLength;
    c = wordLength = 0;

    // initiate array to stoe amount of different length words
    // notice that sizeof is not amount of integers but memorysize
    // also notice that no word in real is length 0
    // so dont store stuff into zeros
    int lengths[100];
    for (int i = 0; i < sizeof(lengths)/sizeof(lengths[0]); i++) {
        lengths[i] = 0;
    }

    // CALCULATE DIFFERENT WORD LENGTHS FROM THE TEXT FILE:
    while((c = getchar()) != EOF) {
        if (c == '\n' || c == ' ' || c == '\t' || (ispunct(c) && c != '-')) {
            // add the word that just ended to the lengths array
            if (wordLength > 0) {
                printf("Length we are saving now: %d\n", wordLength);
                lengths[wordLength]++;
                wordLength = 0;
            }
        }
        // we are inside a word, so start calculating word length
        else {
            printf("Current letter: %c\n", c);
            printf("Current wordLength: %d\n", wordLength);
            wordLength++;
            printf("WordLength after adding +1: %d\n", wordLength);
        }
    }

    // PRINT RESULTS HORIZONTALLY:
    printf("%c", '\n');
    printf("%c", '\n');
    printf("%c", '\n');
    printf("DISTRIBUTION OF LENGTH OF WORDS IN A STORY: ");
    printf("%c", '\n');

    for (int i = 1; i < sizeof(lengths) / sizeof(lengths[1]); i++) {
        if (lengths[i] > 0) {
            printf("Words with length of %d: %d \t", i, lengths[i]);
            for (int j = 0; j < lengths[i]; j++) {
                printf("%c", '*');
            }
            printf("%c", '\n');
        }
    }

    

    // PRINT RESULTS VERTICALLY:
    printf("%c", '\n');
    printf("%c", '\n');
    printf("%c", '\n');
    printf("DISTRIBUTION OF LENGTH OF WORDS IN A STORY: ");
    printf("%c", '\n');


    // max height of bars will be same as highest frequency of a word length
    // width will be the amount of lengths with at least 1 word
    int height, width;
    height = width = 0;

    for (int i = 1; i < sizeof(lengths)/ sizeof(lengths[1]); i++) {
        if (lengths[i] > height) {
            height = lengths[i];
        }
    }

    for (int i = 1; i < sizeof(lengths)/ sizeof(lengths[1]); i++) {
        if (lengths[i] > 0) {
            width++;
        }
    }

    printf("Height: %d\n", height);
    printf("Width: %d\n", width);
    printf("----------------------------------------------------------------------------------------------------------\n");


    // print vertically stars
    // start from highest row, where only most frequest words are printed
    // otherwise print goes upside down
    for (int i = height; i > 0; i--) {
        printf("%c", '\n');
        // find all active words and print those (must always have freq > 0)
        for (int j = 0; j < sizeof(lengths) / sizeof(lengths[1]); j++) {
            if (lengths[j] >= i && lengths[j] > 0) {
                printf("     *     ");
            }
            else if (lengths[j] > 0) {
                printf("           ");
            }
        }
    }
    printf("%c", '\n');


    // add numbers to bottom
    for (int i = 0; i < sizeof(lengths) / sizeof(lengths[1]); i++) {
        if (lengths[i] > 0) {
            printf("length:%-2d  ", i);
        }
    }
    printf("%c", '\n');
    printf("----------------------------------------------------------------------------------------------------------\n");

    return 0;
}
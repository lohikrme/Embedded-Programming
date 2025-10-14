// Exercise 1-16: Revise the main routine of the longest-line program so it will correctly print the
// length of arbitrary long input lines, and as much as possible of the text. 

#include <stdio.h>

// maximum input line length
#define MAXLINE 10
// initiate assistant functions
int getline(char line[], int maxline);
void copy(char to[], char from[]);

// MAIN FUNCTION
int main() {
    // current length, max length seen so far
    int curLen;
    int maxLen;
    curLen = maxLen = 0;
    // current input line is a character array of max size 10
    char curLine[MAXLINE];
    // longestSentence line saved here is a character array of max size 10
    char longestSentence[MAXLINE];


    // go through input one line at time until getline returns 0
    // if getline returns 0, it means input has been read through
    // getline saves line to "curLine" array and returns its length
    // if getline returns higher number than current max
    // it means that it went through more letters than current max
    // so, in that case, we copy the array from line array to longest array with copy() function
    while ((curLen = getline(curLine, MAXLINE)) > 0) {
        printf("Current length of line: %d\n", curLen);
        if (curLen > maxLen) {
            maxLen = curLen;
            copy(longestSentence, curLine);
        }
    }
    if (maxLen > 0) {
        printf("The longest sentence had %d letters. It was: '%s'\n", maxLen, longestSentence);
    }
    return 0;
}


// getline:  read one line from input (file or user input) into array 's'
// save every character to array 's', which is a reference to a memory location
// stops at EOF, newline or when array 's' is nearly full
// returns length of line (excluding '\0')
int getline(char s[], int limit)
{
    // initiate current character and letterCount
    int c, letterCount;
    c = letterCount = 0;

    // go through the input characters one by one
    while ((c=getchar())!=EOF && c !='\n') {
        // add character to the array's memory location that was referenced in getline call
        if (letterCount < limit) {
            s[letterCount] = c;
        }
        // even beyond the limit, increase letterCount by every letter
        letterCount++;
    }
    // add manually \n letters, because we need it for printing the line
    // \n is also part of formating the string, so it is part of the letter count
    if (c == '\n') {
        s[letterCount] = c;
        ++letterCount;
    }
    // terminate string by adding '\0'
    // \0 is a deeper thing for processor, not needed in letter count
    s[letterCount] = '\0';
    return letterCount;
}

// copy () function copies chars from 'from' array to 'to' array, one char at time
// until it faces '\0' and knows from that the string (e.g whole line) ended
void copy(char to[], char from[])
{
    // copy a sentence from array to array
    int i = 0;
    while ((to[i] = from[i]) != '\0') {
        ++i;
    }    
}
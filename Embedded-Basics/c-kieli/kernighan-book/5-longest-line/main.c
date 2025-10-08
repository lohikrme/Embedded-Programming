// task 5: print the longest line from input

#include <stdio.h>

// maximum input line length
#define MAXLINE 1000
// initiate assistant functions
int getline(char line[], int maxline);
void copy(char to[], char from[]);

// MAIN FUNCTION
int main() {
    // current length, max length seen so far
    int len;
    int max;
    max = 0;
    // current input line is a character array of max size 1000
    char line[MAXLINE];
    // longest line saved here is a character array of max size 1000
    char longest[MAXLINE];


    // go through input one line at time until getline returns 0
    // if getline returns 0, it means input has been read through
    // getline saves line to "line" array and returns its length
    // if getline returns higher number than current max
    // it means that it went through more letters than current max
    // so, in that case, we copy the array from line array to longest array with copy() function
    while ((len = getline(line, MAXLINE)) > 0) {
        if (len > max) {
            max = len;
            copy(longest, line);
        }
    }
    if (max > 0) {
        printf("%s", longest);
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
    for (letterCount = 0; letterCount < limit-1 && (c=getchar())!=EOF && c!='\n'; letterCount++) {
        // add character to the array's memory location that was referenced in getline call
        s[letterCount] = c;
    }
    // add manually \n letters, because we need it for printing the line
    if (c == '\n') {
        s[letterCount] = c;
        ++letterCount;
    }
    // terminate string by adding '\0'
    s[letterCount] = '\0';
    return letterCount;
}

// copy () function copies chars from 'from' array to 'to' array, one char at time
// until it faces '\0' and knows from that the string (e.g whole line) ended
void copy(char to[], char from[])
{
    int i = 0;
    while ((to[i] = from[i]) != '\0') {
        ++i;
    }
        
}
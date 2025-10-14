// Exercise 1-19. Write a function reverse(s) that reverses the character string s. 
// Use it to write a program that reverses its input a line at a time. 

#include <stdio.h>
#include <string.h>

#define LIMIT 100
void reverse(char string[]);
void swap(char string[], int index1, int index2);

int main() {
    char sentence[LIMIT] = "Kissa";
    // print original sentence
    printf("\nThe sentence to be reversed is: \n'%s'\n\n", sentence);
    reverse(sentence);
    // print reversed sentence
    printf("\nAnd the fully reversed sentence is: \n");

    printf("%s\n", sentence);
    return 0;
}

// reverse the string within the same memory area
void reverse(char string[]) {
    printf("The reversing process is starting... \n");
    // find out length of string (not max array length but used)
    int len = strlen(string);
    // and then loop through the the string swapping every element
    // divide len by 2, or it swaps back to original...
    // if it is even numbers length, it will swap every pair once
    // if it is odd numbers length, the middle letter won't be swapped at all
    for (int i = 0; i < len / 2; i++) {
        // use len - i - 1, because \0 letter must be ignored
        swap(string, i, len-i-1);
    }
}

// swaps 2 letters within a string (character array)
void swap(char string[], int index1, int index2)
{
    char letter = string[index1];
    string[index1] = string[index2];
    string[index2] = letter;
    printf("%s\n", string);
}
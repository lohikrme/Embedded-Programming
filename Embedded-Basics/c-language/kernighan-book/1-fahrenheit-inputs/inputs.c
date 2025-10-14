#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 100

int main() {
    char line[MAXLINE];

    // 1. Käyttäjän syöte rivinä (fgets)
    printf("Anna lause: ");
    if (fgets(line, MAXLINE, stdin) != NULL) {
        printf("Annoit: %s", line);
    }

    // 2. Tiedoston luku (scanf)
    FILE *fp = fopen("../0-data/luvut.txt", "r");
    if (fp == NULL) {
        perror("Tiedoston avaaminen epäonnistui");
    } else {
        int num;
        printf("\nTiedostosta luetut luvut:\n");
        while (fscanf(fp, "%d", &num) == 1) {
            printf("%d ", num);
        }
        fclose(fp);
        printf("\n");
    }

    // 3. Yksittäisten merkkien luku (getchar)
    printf("\nPaina merkkejä (q lopettaa):\n");
    char c;
    while ((c = getchar()) != 'q') {
        if (c != '\n') {
            printf("Luit merkin: %c\n", c);
        }
    }

    printf("Ohjelma päättyy.\n");
    return 0;
}

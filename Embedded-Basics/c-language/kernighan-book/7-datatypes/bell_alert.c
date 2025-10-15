
#include <stdio.h>
#include <unistd.h>

int main() {
    while (1) {
        printf("\a");
        sleep(2);
    }
    
    return 0;
}
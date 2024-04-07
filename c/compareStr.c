#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    char mystr[256];
    // mystr[0] = 0x45;
    // mystr[1] = 0x58;
    // mystr[2] = 0x49;
    // mystr[3] = 0x54;
    // mystr[4] = 0x00;

    fgets(mystr, 255, stdin);

    for (int i = 0; i < strlen(mystr); i++) {
        printf("%c - %02x \n", mystr[i],  (unsigned char)mystr[i]);
    }

    if (strncmp(mystr, "EXIT", 4) == 0)
    {
        printf("equal...\n");
    }


}
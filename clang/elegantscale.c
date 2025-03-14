#include <stdio.h>
#include <string.h>

int main() {
    int note, i;
    char key[3];
    char* scale[12] = { "C", "Db", "D", "Eb",
                        "E", "F", "Gb", "G",
                        "Ab", "A", "Bb", "B"};
    printf("Please enter the key (capitals only, "
           " use b for flats, eg. Eb): ");
    scanf("%s", key);

    for(i = 0; i < 12; i++) {
        if(strcmp(scale[i],key) == 0) {
            note = i;
            printf("== %s major scale ==\n", key);
            break;
        }
    }

    if(note >= 0) {
        for(i = 0; i < 7; i++) {
            printf("%s ", scale[note%12]);
            if(i != 2) note += 2;
            else note++;
        }
        printf("\n");
        return 0;
    }
    else {
        printf("%s: invalid key\n", key);
        return 1;
    }
}

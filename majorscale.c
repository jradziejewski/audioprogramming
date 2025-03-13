#include <stdio.h>

int main()
{
    int note, i;
    printf("Enter key (in pitch-class nuber, 0-11): ");
    scanf("%d", &note);
    
    while(note < 0) note += 12;
    
    for (i = 0; i < 7; i++) {
        if(note%12 == 0) printf("C ");
        else if(note%12 == 1) printf("Db ");
        else if(note%12 == 2) printf("D ");
        else if(note%12 == 3) printf("Eb ");
        else if(note%12 == 4) printf("E ");
        else if(note%12 == 5) printf("F ");
        else if(note%12 == 6) printf("Gb ");
        else if(note%12 == 7) printf("G ");
        else if(note%12 == 8) printf("Ab ");
        else if(note%12 == 9) printf("A ");
        else if(note%12 == 10) printf("Bb ");
        else printf("B ");
        if(i != 2) note += 2;
        else note ++;
    }
    printf("\n");
    return 0;
}

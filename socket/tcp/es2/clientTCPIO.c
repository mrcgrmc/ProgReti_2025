#include "../network.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    char receive[MTU];
    FILE *fd;
    FILE *ff;
    char filename[256],filename2[256], c;


    fd = createTCPConnectionFD("localhost", 35000);
    printf("Connected to server\n");

    printf("insert filename source:\n");
    scanf("%s", filename);

    printf("insert filename where to save:\n");
    scanf("%s", filename2);

    ff = fopen(filename2, "w");

    if (ff == NULL) {
        perror("Error opening file");
        exit(0);
    }

    fprintf(fd, "%s\n", filename);
    
    while ((c = fgetc(fd)) != EOF) {
        fputc(c, ff);
    }        


    printf("%s ricevuto !\n", filename2);

    fclose(fd);
    fclose(ff);
}


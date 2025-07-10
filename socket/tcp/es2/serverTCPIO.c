#include "../network.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    socketif_t server;
    FILE *fd, *ff;
    char filename[256], c;
    
    server = createTCPServer(35000);
    if (server < 0){
        printf("Error: %i\n", server);
        return -1;
    }

    
    while(1){
        fd = acceptConnectionFD(server);
        printf("Connected to client\n");
        fgets(filename, sizeof(filename), fd);
        fflush(fd);
        filename[strcspn(filename, "\r\n")] = '\0';
        printf("Sending %s\n", filename);
        ff = fopen(filename, "r");
        if (ff == NULL) {
            perror("Error opening file");
            exit(0);
        }
        while ((c = fgetc(ff)) != EOF) {
            fputc(c, fd);
        }        
        printf("%s sent!\n", filename);
        fclose(fd);
        fclose(ff);
    }
    
    return 0;
}


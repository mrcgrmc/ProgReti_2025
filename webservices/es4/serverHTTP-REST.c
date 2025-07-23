#include "network.h"
#include <stdio.h>
#include <stdlib.h> 


int *numeriPrimi(int min, int max, int *outSize) {
    int capacity = max - min + 1;
    int *primes   = malloc(capacity * sizeof(int));
    if (!primes) {          
        *outSize = 0;
        return NULL;
    }

    int k = 0, flag;

    for (int i = min; i <= max; i++) {
        if (i == 0 || i == 1)
            continue;

        flag = 1;
        for (int j = 2; j <= i / 2; ++j) {
            if (i % j == 0) {
                flag = 0;
                break;
            }
        }

        if (flag == 1) {
            primes[k++] = i;        
        }
    }

    //comunico quanti primi ho inserito 
    *outSize = k;

    int *shrunk = realloc(primes, k * sizeof(int));
    return shrunk ? shrunk : primes;  // se realloc fallisce, uso l’originale 
}

float calcolaSomma(float val1, float val2)  {
   return (val1 + val2);
}

int main(int argc, char *argv[]){
    socketif_t sockfd;
    FILE* connfd;
    int res, i;
    long length=0;
    char request[MTU], url[MTU], method[10], c;
    
    if (argc < 2) {
        printf("Uso: %s <ip>\n", argv[0]);
        return -1;
    }
    sockfd = createTCPServer(argv[1],8000);
    if (sockfd < 0){
        printf("[SERVER] Errore: %i\n", sockfd);
        return -1;
    }
    
    while(true) {
        connfd = acceptConnectionFD(sockfd);
        
        fgets(request, sizeof(request), connfd);
        strcpy(method,strtok(request, " "));
        strcpy(url,strtok(NULL, " "));
        while(request[0]!='\r') {
            fgets(request, sizeof(request), connfd);
            if(strstr(request, "Content-Length:")!=NULL)  {
                length = atol(request+15);
            }
        }
        
        if(strcmp(method, "POST")==0)  {
            for(i=0; i<length; i++)  {
                c = fgetc(connfd);
            }
        }
        if(strstr(url, "calcola-somma")==NULL && strstr(url, "calcola-numeri-primi")== NULL) {
            fprintf(connfd,"HTTP/1.1 200 OK\r\n\r\n{\r\n    Funzione non riconosciuta!\r\n}\r\n");
        }
        else if(strstr(url, "calcola-somma")==NULL){

            printf("Chiamata a funzione di calcolo dei numeri primi\n");
            
            char *function, *op1, *op2;
            int nres, min, max;
   
            // skeleton: decodifica (de-serializzazione) dei parametri
            function = strtok(url, "?&");
            op1 = strtok(NULL, "?&");
            op2 = strtok(NULL, "?&");
            strtok(op1,"=");
            min = atoi(strtok(NULL,"="));
            strtok(op2,"=");
            max = atoi(strtok(NULL,"="));
            
            // chiamata alla business logic
            int *res = numeriPrimi(min, max, &nres);
            
            // skeleton: codifica (serializzazione) del risultato
            fprintf(connfd,"HTTP/1.1 200 OK\r\n\r\n{\r\n    \"primi\":[");
            for (int i = 0; i < nres; ++i) {
                fprintf(connfd, "%d", res[i]);
                if (i != nres - 1)      
                    fprintf(connfd, ",");
            }
            fprintf(connfd, "]\r\n}\r\n");
            free(res);
        }
        else {
            printf("Chiamata a funzione sommatrice\n");
            
            char *function, *op1, *op2;
            float somma, val1, val2;
   
            // skeleton: decodifica (de-serializzazione) dei parametri
            function = strtok(url, "?&");
            op1 = strtok(NULL, "?&");
            op2 = strtok(NULL, "?&");
            strtok(op1,"=");
            val1 = atof(strtok(NULL,"="));
            strtok(op2,"=");
            val2 = atof(strtok(NULL,"="));
            
            // chiamata alla business logic
            somma = calcolaSomma(val1, val2);
            
            // skeleton: codifica (serializzazione) del risultato
            fprintf(connfd,"HTTP/1.1 200 OK\r\n\r\n{\r\n    \"somma\":%f\r\n}\r\n", somma);
        }
        
        fclose(connfd);
                
        printf("\n\n[SERVER] sessione HTTP completata\n\n");
    }
    
    closeConnection(sockfd);
    return 0;
}




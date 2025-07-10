#include "network.h"

#define MAX_PLATE_LEN 32


int main(void) {
    socketif_t socket;
    char request[MAX_PLATE_LEN];
    int response;
    char hostAddress[MAXADDRESSLEN];
    int port;
    
    socket = createUDPInterface(30000);
    
    printf("[CLIENT] Inserisci la targa del veicolo: ");
    scanf("%s", request);
    printf("[CLIENT] Spedisco targa al server\n");
    UDPSend(socket, request, strlen(request), "localhost", 10000);
     
    UDPReceive(socket, &response, MTU, hostAddress, &port);
    printf("[CLIENT] Ho ricevuto un messaggio da host/porta %s/%d\n", hostAddress, port);
    printf("[CLIENT] Sono passato %d volte dal casello\n", response);
}


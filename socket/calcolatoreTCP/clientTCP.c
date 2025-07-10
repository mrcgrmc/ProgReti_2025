#include "network.h"

int main(void) {
    connection_t connection;
    int request, response;
    char op;
 	
    printf("[CLIENT] Creo una connessione logica col server\n");
    connection = createTCPConnection("localhost", 35000);
    if (connection < 0) {
        printf("[CLIENT] Errore nella connessione al server: %i\n", connection);
    }
    else
    {
        for (int i = 0; i<2; i++){
            printf("[CLIENT] Inserisci un numero intero:\n");
            scanf("%d", &request);
            printf("[CLIENT] Invio richiesta con numero al server\n");
            TCPSend(connection, &request, sizeof(request));
        }
        
        printf("[CLIENT] Inserisci operando:\n");
        scanf(" %c", &op);
        printf("[CLIENT] Invio operando al server\n");
        TCPSend(connection, &op, sizeof(op));

        TCPReceive(connection, &response, sizeof(response));
        printf("[CLIENT] Ho ricevuto la seguente risposta dal server: %d\n", response);
        closeConnection(connection);
    }
}



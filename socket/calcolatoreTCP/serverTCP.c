#include "network.h"

int main(void) {
    int request, response[2];
    char op;
    socketif_t socket;
    connection_t connection;
    
    socket = createTCPServer(35000);
    if (socket < 0){
        printf("[SERVER] Errore di creazione del socket: %i\n", socket);
    }
    else
    {
        while(true){
            printf("[SERVER] Sono in attesa di richieste di connessione da qualche client\n");    
            connection = acceptConnection(socket);
            printf("[SERVER] Connessione instaurata\n");

            for (int i = 0; i < 2; i++) {
                printf("[SERVER] Ricevo richiesta di calcolo dal client\n");
                TCPReceive(connection, &request, sizeof(request));
                printf("[SERVER] Ho ricevuto la seguente richiesta dal client: %d\n", request);
                response[i] = request;
            }
            printf("[SERVER] Ricevo operazione da eseguire\n");
            TCPReceive(connection, &op, sizeof(op));

            switch (op)
            {
            case '+':   
                response[0] += response[1];
                break;
            case '-':
                response[0] -= response[1];
                break;
            case '*':
                response[0] *= response[1];
                break;
            case '/':
                if (response[1] != 0) {
                    response[0] /= response[1];
                } else {
                    printf("[SERVER] Errore: divisione per zero\n");
                    response[0] = 0;
                }
                break;
            default:
                break;
            }

            printf("[SERVER] Invio la risposta al client\n");
            TCPSend(connection, &response, sizeof(response));
            closeConnection(connection);
        }
    }
}


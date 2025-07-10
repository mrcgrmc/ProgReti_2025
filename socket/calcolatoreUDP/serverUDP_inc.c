#include "network.h"
#include <stdio.h>
#include <string.h>

#define REQBUFSZ 100

int main(void) {
    socketif_t socket = createUDPInterface(35000);
    char    request[REQBUFSZ];
    char    hostAddress[MAXADDRESSLEN];
    int     port;
    unsigned int a, b;
    char    op[16];
    int     result;

    while (1) {
        printf("[SERVER] In attesa di richieste…\n");

        UDPReceive(socket,
                   request,
                   sizeof(request),
                   hostAddress,
                   &port);

        printf("[SERVER] Ho ricevuto da %s/%d -> \"%s\"\n",
               hostAddress, port, request);

        if (sscanf(request, "%u %u %15s", &a, &b, op) != 3) {
            fprintf(stderr, "[SERVER] Formato errato nella richiesta\n");
            continue;
        }

        switch (op[0]) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/': result = (b != 0 ? a / b : 0); break;
            default:
                fprintf(stderr, "[SERVER] Operatore sconosciuto: %s\n", op);
                result = 0;
        }


        UDPSend(socket,
                &result,
                sizeof(result),
                hostAddress,
                port);

        printf("[SERVER] Risultato %d inviato\n", result);
    }

    return 0;
}

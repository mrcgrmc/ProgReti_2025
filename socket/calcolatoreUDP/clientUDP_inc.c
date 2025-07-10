#include "network.h"
#include <stdio.h>
#include <string.h>

#define REQBUFSZ 100

int main(void) {
    socketif_t socket = createUDPInterface(20000);
    char    request[REQBUFSZ];
    int result;
    unsigned int a, b;
    char    op[16];
    int     len;
    char hostAddress[MAXADDRESSLEN];
    int port;



    printf("Inserisci due numeri interi e un'operazione (es. 5 7 +):\n");

    if (fgets(request, sizeof(request), stdin) == NULL) {
        perror("fgets");
        return 1;
    }
    request[strcspn(request, "\n")] = '\0';

    if (sscanf(request, "%u %u %15s", &a, &b, op) != 3) {
        fprintf(stderr, "Formato errato!\n");
        return 1;
    }

    len = snprintf(request, sizeof(request), "%u %u %s", a, b, op);

    UDPSend(socket,
            request,
            len + 1,
            "127.0.0.1",
            35000);

    UDPReceive(socket,
                &result,
                sizeof(result),
                hostAddress,
                &port);

    printf("[CLIENT] Ho ricevuto un messaggio da host/porta %s/%d\n", hostAddress, port);
    
    printf("[CLIENT] Contenuto: %d\n", result);
    return 0;
}

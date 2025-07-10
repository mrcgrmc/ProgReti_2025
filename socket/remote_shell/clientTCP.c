#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CMD_BUFSIZE 1024

int main(void) {
    char server_ip[] = "127.0.0.1";
    int port = 35000;

    connection_t conn = createTCPConnection(server_ip, port);
    if (conn < 0) {
        fprintf(stderr, "Errore connessione al server %s:%d -> %d\n",
                server_ip, port, (int)conn);
        return EXIT_FAILURE;
    }

    char cmd[CMD_BUFSIZE];
    while (1) {
        printf("shell> ");
        if (!fgets(cmd, sizeof(cmd), stdin))
            break;
        cmd[strcspn(cmd, "\n")] = '\0';
        if (strcmp(cmd, "exit") == 0)
            break;

        uint32_t cmd_len = (uint32_t)strlen(cmd);
        TCPSend(conn, &cmd_len, sizeof(cmd_len));
        TCPSend(conn, cmd, cmd_len);

        uint32_t out_len = 0;
        if (TCPReceive(conn, &out_len, sizeof(out_len)) <= 0)
            break;

        if (out_len > 0) {
            char *out_buf = malloc(out_len + 1);
            if (!out_buf) {
                perror("malloc");
                break;
            }
            if (TCPReceive(conn, out_buf, out_len) <= 0) {
                free(out_buf);
                break;
            }
            out_buf[out_len] = '\0';
            printf("%s", out_buf);
            free(out_buf);
        } else {
            printf("[SERVER] Nessun output per il comando: %s\n", cmd);
        }
    }

    closeConnection(conn);
    return EXIT_SUCCESS;
}

#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define CMD_BUFSIZE 1024

int main(void) {
    int listen_port = 35000;
    socketif_t server_sock = createTCPServer(listen_port);
    if (server_sock < 0) {
        fprintf(stderr, "Errore creazione server TCP: %d\n", (int)server_sock);
        return EXIT_FAILURE;
    }
    printf("[SERVER] In ascolto sulla porta %d...\n", listen_port);

    while (1) {
        connection_t conn = acceptConnection(server_sock);
        if (conn < 0)
            continue;
        printf("[SERVER] Connessione accettata: conn=%d\n", (int)conn);

        while (1) {
            uint32_t cmd_len = 0;
            if (TCPReceive(conn, &cmd_len, sizeof(cmd_len)) <= 0)
                break;

            char *cmd = malloc(cmd_len + 1);
            if (!cmd) {
                perror("malloc");
                break;
            }
            if (TCPReceive(conn, cmd, cmd_len) <= 0) {
                free(cmd);
                break;
            }
            cmd[cmd_len] = '\0';
            printf("[SERVER] Eseguo: %s\n", cmd);

            FILE *fp = popen(cmd, "r");
            if (!fp) {
                perror("popen");
                free(cmd);
                break;
            }

            char *output = NULL;
            size_t out_capacity = 0, out_len = 0;
            char buffer[512];
            size_t n;
            while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
                if (out_len + n > out_capacity) {
                    out_capacity = (out_capacity + n) * 2;
                    output = realloc(output, out_capacity);
                    if (!output) {
                        perror("realloc");
                        break;
                    }
                }
                memcpy(output + out_len, buffer, n);
                out_len += n;
            }
            pclose(fp);
            free(cmd);

            uint32_t send_len = (uint32_t)out_len;
            TCPSend(conn, &send_len, sizeof(send_len));
            if (out_len > 0)
                TCPSend(conn, output, out_len);
            free(output);
        }

        closeConnection(conn);
        printf("[SERVER] Connessione chiusa: conn=%d\n", (int)conn);
    }

    return EXIT_SUCCESS;
}

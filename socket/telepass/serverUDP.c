#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DB_FILE "database.txt"
#define MAX_PLATE_LEN MTU  // dimensione massima basata su MTU

typedef struct {
    char plate[MAX_PLATE_LEN];
    int  count;
} Entry;

int main(void) {
    socketif_t socket;
    char request[MTU];
    char hostAddress[MAXADDRESSLEN];
    int port;

    Entry *entries = NULL;
    size_t n_entries = 0, capacity = 0;
    FILE *fp;

    socket = createUDPInterface(10000);
    if (socket < 0) {
        printf("Errore creazione interfaccia UDP\n");
        exit(0);
    }

    while (true) {
        printf("[SERVER] Sono in attesa di richieste da qualche client\n");

        UDPReceive(socket, request, MTU, hostAddress, &port);
        request[strcspn(request, "\n")] = '\0';  // Rimuovo eventual newline

        printf("[SERVER] Ho ricevuto un messaggio da host/porta %s/%d\n", hostAddress, port);
        printf("[SERVER] Contenuto: %s\n", request);

        // 1) Carica database esistente
        free(entries);
        entries = NULL;
        n_entries = 0;
        capacity = 0;
        fp = fopen(DB_FILE, "r");
        if (fp) {
            char plate[MAX_PLATE_LEN];
            int count;
            while (fscanf(fp, "%31s %d", plate, &count) == 2) {
                if (n_entries == capacity) {
                    capacity = capacity ? capacity * 2 : 4;
                    entries = realloc(entries, capacity * sizeof *entries);
                    if (!entries) { perror("realloc"); fclose(fp); return EXIT_FAILURE; }
                }
                strcpy(entries[n_entries].plate, plate);
                entries[n_entries].count = count;
                n_entries++;
            }
            fclose(fp);
        }

        // 2) Aggiorna o aggiungi la targa
        int updated_index = -1;
        for (size_t j = 0; j < n_entries; j++) {
            if (strcmp(entries[j].plate, request) == 0) {
                entries[j].count++;
                updated_index = j;
                break;
            }
        }
        if (updated_index < 0) {
            if (n_entries == capacity) {
                capacity = capacity ? capacity * 2 : 4;
                entries = realloc(entries, capacity * sizeof *entries);
                if (!entries) { perror("realloc"); return EXIT_FAILURE; }
            }
            strcpy(entries[n_entries].plate, request);
            entries[n_entries].count = 1;
            updated_index = n_entries;
            n_entries++;
        }

        // 3) Scrivi database aggiornato su file
        fp = fopen(DB_FILE, "w");
        if (!fp) { perror("fopen"); free(entries); return EXIT_FAILURE; }
        for (size_t j = 0; j < n_entries; j++) {
            fprintf(fp, "%s %d\n", entries[j].plate, entries[j].count);
        }
        fclose(fp);

        int count = entries[updated_index].count;
        UDPSend(socket,
                &count,
                sizeof(count),
                hostAddress,
                port);
        printf("[SERVER] Inviato conteggio: %d\n", count);
    }

    free(entries);
    return 0;
}

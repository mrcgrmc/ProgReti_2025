#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SERVER_FOLDER  "./"
#define DEFAULT_INDEX  "index.html"

// risposte statiche
static const char *RESPONSE_405 =
  "HTTP/1.1 405 Method Not Allowed\r\n"
  "Allow: GET, POST\r\n"
  "Content-Length: 0\r\n"
  "\r\n";
static const char *RESPONSE_400 =
  "HTTP/1.1 400 Bad Request\r\n"
  "Content-Length: 0\r\n"
  "\r\n";

// semplice memmem per cercare un buffer in un altro
static void *memmem_simple(const void *haystack, size_t haystacklen,
                           const void *needle,   size_t needlelen) {
    if (needlelen == 0) return (void*)haystack;
    const char *h = haystack;
    for (size_t i = 0; i + needlelen <= haystacklen; ++i) {
        if (memcmp(h + i, needle, needlelen) == 0)
            return (void*)(h + i);
    }
    return NULL;
}

static bool startswith(const char *s, const char *p) {
    return strncmp(s, p, strlen(p)) == 0;
}

// parsing boundary e filename dai rispettivi header
static bool parse_boundary(const char *hdr, char *boundary, size_t sz) {
    const char *p = strstr(hdr, "boundary=");
    if (!p) return false;
    p += 9;
    snprintf(boundary, sz, "--%s", p);
    boundary[strcspn(boundary, "\r\n")] = '\0';
    return true;
}
static bool parse_filename(const char *hdr, char *fname, size_t sz) {
    const char *p = strstr(hdr, "filename=\"");
    if (!p) return false;
    p += 10;
    size_t len = strcspn(p, "\"");
    if (len == 0 || len >= sz) return false;
    memcpy(fname, p, len);
    fname[len] = '\0';
    return true;
}

static void serveFile(FILE *connfd, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fputs("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", connfd);
        return;
    }
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    rewind(f);
    fprintf(connfd,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/html; charset=UTF-8\r\n"
      "Content-Length: %ld\r\n"
      "\r\n",
      filesize
    );
    char buf[MTU];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
        fwrite(buf, 1, r, connfd);
    }
    fclose(f);
}

static void handle_post_upload(FILE *connfd,
                               long content_length,
                               const char *content_type) {
    char boundary[256];
    if (!parse_boundary(content_type, boundary, sizeof(boundary))) {
        fputs(RESPONSE_400, connfd);
        return;
    }

    char *body = malloc(content_length);
    if (!body) {
        fputs(RESPONSE_400, connfd);
        return;
    }
    long got = 0;
    while (got < content_length) {
        size_t n = fread(body + got, 1, content_length - got, connfd);
        if (n == 0) break;
        got += n;
    }
    if (got < content_length) {
        free(body);
        fputs(RESPONSE_400, connfd);
        return;
    }

    void *bstart = memmem_simple(body, content_length,
                                 boundary, strlen(boundary));
    if (!bstart) {
        free(body);
        fputs(RESPONSE_400, connfd);
        return;
    }
    // trova la doppia CRLF che separa header di parte e dati
    void *hdr_end = memmem_simple(bstart, content_length - ((char*)bstart - body),
                                  "\r\n\r\n", 4);
    if (!hdr_end) {
        free(body);
        fputs(RESPONSE_400, connfd);
        return;
    }
    size_t data_off = (size_t)((char*)hdr_end - body) + 4;

    // Estree filename dai metadata
    char filename[256] = {0};
    if (!parse_filename(body, filename, sizeof(filename))) {
        free(body);
        fputs(RESPONSE_400, connfd);
        return;
    }

    // Trova il boundary successivo per capire dove finisce il file
    void *bend = memmem_simple(body + data_off,
                               content_length - data_off,
                               boundary, strlen(boundary));
    size_t end_off;
    if (bend) {
        // toglie eventuali \r\n prima del boundary
        size_t idx = (size_t)((char*)bend - body);
        if (idx >= 2 && body[idx-2]=='\r' && body[idx-1]=='\n')
            end_off = idx - 2;
        else
            end_off = idx;
    } else {
        end_off = content_length;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s%s", SERVER_FOLDER, filename);
    FILE *out = fopen(path, "wb");
    if (out) {
        fwrite(body + data_off, 1, end_off - data_off, out);
        fclose(out);
    }
    free(body);

    fputs("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n", connfd);
}

int main(){
    socketif_t sockfd = createTCPServer(8000);
    if (sockfd < 0) {
        fprintf(stderr, "[SERVER] Errore creazione socket: %d\n", sockfd);
        return EXIT_FAILURE;
    }
    printf("[SERVER] Avviato sulla porta 8000\n");

    while (true) {
        FILE *connfd = acceptConnectionFD(sockfd);
        if (!connfd) continue;

        char line[MTU];
        // legge request-line
        if (!fgets(line, sizeof(line), connfd)) {
            fclose(connfd);
            continue;
        }
        char method[16], path[256], proto[16];
        if (sscanf(line, "%15s %255s %15s", method, path, proto) != 3) {
            fputs(RESPONSE_400, connfd);
            fclose(connfd);
            continue;
        }

        // legge headers fino a riga vuota
        long content_length = 0;
        char content_type[256] = {0};
        while (fgets(line, sizeof(line), connfd) && strcmp(line, "\r\n") != 0) {
            if (startswith(line, "Content-Length:"))
                content_length = atol(line + 15);
            else if (startswith(line, "Content-Type:")) {
                strncpy(content_type, line + 13, sizeof(content_type)-1);
                content_type[strcspn(content_type, "\r\n")] = '\0';
            }
        }

        if (strcmp(method, "GET") == 0) {
            const char *file;
            char local[256];
            if (strcmp(path, "/") == 0)
                file = DEFAULT_INDEX;
            else {
                snprintf(local, sizeof(local), "%s", path + 1);
                file = local;
            }
            serveFile(connfd, file);
        }
        else if (strcmp(method, "POST") == 0
               && content_length > 0
               && strstr(content_type, "multipart/form-data")) {
            handle_post_upload(connfd, content_length, content_type);
        }
        else {
            fputs(RESPONSE_405, connfd);
        }

        fclose(connfd);
        printf("[SERVER] Sessione completata\n");
    }

    closeConnection(sockfd);
    return EXIT_SUCCESS;
}

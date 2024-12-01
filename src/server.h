#ifndef SERVER_H
#define SERVER_H

#define BUFFER_SIZE (1024 * 1024)
#define MAX_HEADERS 64
#define MAX_EVENTS 1024

typedef enum
{
    OK_OK = 200,
    OK_CREATED = 201,
    OK_NOCONTENT = 204,
    ERR_NOTFOUND = 404,
    ERR_BADREQ = 400,
    ERR_UNPROC = 422,
    ERR_INTERR = 500,
} res_stat_t;

typedef struct
{
    res_stat_t status;
    const char *message;
} res_info_t;

typedef struct
{
    char *name;
    char *value;
} header_t;

typedef struct
{
    char *method;
    char *path;
    char *version;
    header_t *headers;
    int headers_len;
    char *body;
    char wildcards[16][64];
    int wildcard_num;
} http_req_t;

typedef struct route
{
    char method[16];
    char path[265];
    void (*callback)(int client_fd, http_req_t *req);
    struct route *next;
} route_t;

typedef struct
{
    char extension[16];
    char mime_type[64];
} mime_entry_t;

typedef struct
{
    int sckt;
    route_t *route;
    int port;
    const char *route_dir;
    const char *public_dir;
} server_t;

void free_http_req(http_req_t *req);
int parse_http_req(const char *buffer, http_req_t *http_req);
int serve_file(int client_fd, const char *path);
void handle_client(int client_fd);
void handle_sigint(int sig);

void send_json_response(int client_fd, res_stat_t status, const char *json);
void send_string(int client_fd, char *str);

#endif

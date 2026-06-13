#define _GNU_SOURCE

#include "client.h"

#include <arpa/inet.h>
#include <openssl/hmac.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/uio.h>
#include <unistd.h>


client_t* client_list = NULL;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_to_client(client_t* client, const char* msg) {
    size_t msg_len = strlen(msg) + 1;
    if (msg_len > MAX_MSG_LEN) msg_len = MAX_MSG_LEN;

    uint8_t plain[MAX_MSG_LEN];
    memcpy(plain, msg, msg_len);

    xor_crypt(plain, msg_len, client->enc_key, client->send_counter);
    client->send_counter++;

    uint8_t hmac[32];
    unsigned int hmac_len;
    HMAC(EVP_sha256(), client->enc_key, KEY_LENGTH, plain, msg_len, hmac, &hmac_len);

    uint16_t net_len = htons((uint16_t)msg_len);
    send(client->sockfd, hmac, 32, 0);
    send(client->sockfd, &net_len, 2, 0);
    send(client->sockfd, plain, msg_len, 0);
}

void broadcast_to_room(const char* room, int sender_sockfd, const char* msg, int exclude_self) {
    pthread_mutex_lock(&clients_mutex);
    client_t* cur = client_list;
    while (cur) {
        if (strcmp(cur->room, room) == 0 &&
            (!exclude_self || cur->sockfd != sender_sockfd)) {
            send_to_client(cur, msg);
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int sockfd) {
    client_t** cur = &client_list;
    while (*cur) {
        if ((*cur)->sockfd == sockfd) {
            client_t* to_delete = *cur;
            *cur = (*cur)->next;
            close(to_delete->sockfd);
            free(to_delete);
            return;
        }
        cur = &(*cur)->next;
    }
}

int is_username_taken(const char* username, const char* room, int exclude_sockfd) {
    client_t* cur = client_list;
    while (cur) {
        if (cur->sockfd != exclude_sockfd &&
            strcmp(cur->name, username) == 0 &&
            strcmp(cur->room, room) == 0) {
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

int is_forbidden_name(const char* name) {
    return (strcasecmp(name, "TÚ") == 0)
        || (strcasecmp(name, "SERVER") == 0)
        || (strchr(name, '[') != NULL)
        || (strchr(name, ']') != NULL);
}
#pragma once

#include "protocol.h"
#include "dh.h"

#include <stdint.h>
#include <pthread.h>

#define MAX_CLIENTS 100


typedef struct client {
    int sockfd;
    char name[NAME_LEN];
    char room[ROOM_LEN];
    uint8_t enc_key[KEY_LENGTH];
    uint64_t send_counter;
    uint64_t recv_counter;
    struct client* next;
} client_t;

extern client_t* client_list;
extern pthread_mutex_t clients_mutex;

void send_to_client(client_t* client, const char* msg);
void broadcast_to_room(const char* room, int sender_sockfd, const char* msg, int exclude_self);
void remove_client(int sockfd);

int is_username_taken(const char* username, const char* room, int exclude_sockfd);
int is_forbidden_name(const char* name);
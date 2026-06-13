#define _GNU_SOURCE

#include "commands.h"

#include <stdio.h>
#include <string.h>


void list_rooms(client_t* client) {
    pthread_mutex_lock(&clients_mutex);
    char rooms[MAX_CLIENTS][ROOM_LEN];
    int room_count = 0;
    client_t* cur = client_list;
    while (cur) {
        int found = 0;
        for (int i = 0; i < room_count; i++) {
            if (strcmp(rooms[i], cur->room) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            strncpy(rooms[room_count], cur->room, ROOM_LEN-1);
            rooms[room_count][ROOM_LEN-1] = '\0';
            room_count++;
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&clients_mutex);

    char buf[MAX_MSG_LEN];
    snprintf(buf, sizeof(buf), "[SERVER] salas: ");
    for (int i = 0; i < room_count; i++) {
        strncat(buf, rooms[i], sizeof(buf)-strlen(buf)-1);
        if (i < room_count-1) strncat(buf, ", ", sizeof(buf)-strlen(buf)-1);
    }
    strncat(buf, "\n", sizeof(buf)-strlen(buf)-1);
    send_to_client(client, buf);
}

void list_users_in_room(client_t* client) {
    pthread_mutex_lock(&clients_mutex);
    char users[MAX_CLIENTS][NAME_LEN];
    int user_count = 0;
    client_t* cur = client_list;
    while (cur) {
        if (strcmp(cur->room, client->room) == 0) {
            strncpy(users[user_count], cur->name, NAME_LEN-1);
            users[user_count][NAME_LEN-1] = '\0';
            user_count++;
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&clients_mutex);

    char buf[MAX_MSG_LEN];
    snprintf(buf, sizeof(buf), "[SERVER] usuarios en \"%s\": ", client->room);
    for (int i = 0; i < user_count; i++) {
        strncat(buf, users[i], sizeof(buf)-strlen(buf)-1);
        if (i < user_count-1) strncat(buf, ", ", sizeof(buf)-strlen(buf)-1);
    }
    strncat(buf, "\n", sizeof(buf)-strlen(buf)-1);
    send_to_client(client, buf);
}

int handle_command(client_t* client, const char* msg) {
    if (msg[0] != '/') return 0;

    if (strncmp(msg, "/nick ", 6) == 0) {
        char newname[NAME_LEN];
        sscanf(msg + 6, "%31s", newname);

        if (is_forbidden_name(newname)) {
            send_to_client(client, "[SERVER] buen intento.\n");
            return 1;
        }
        if (strcmp(newname, client->name) == 0) {
            send_to_client(client, "[SERVER] ya tienes ese nombre.\n");
            return 1;
        }

        pthread_mutex_lock(&clients_mutex);
        if (is_username_taken(newname, client->room, client->sockfd)) {
            pthread_mutex_unlock(&clients_mutex);
            char err[BUFFER_SIZE];
            snprintf(err, sizeof(err), "[SERVER] el nombre \"%s\" ya está en uso en esta sala.\n", newname);
            send_to_client(client, err);
            return 1;
        }

        char old_name[NAME_LEN];
        strcpy(old_name, client->name);
        strncpy(client->name, newname, NAME_LEN-1);
        client->name[NAME_LEN-1] = '\0';
        pthread_mutex_unlock(&clients_mutex);

        char ack[BUFFER_SIZE];
        snprintf(ack, sizeof(ack), "[SERVER] ahora eres %s.\n", client->name);
        send_to_client(client, ack);

        char namechange_msg[BUFFER_SIZE];
        snprintf(namechange_msg, sizeof(namechange_msg), "[SERVER] %s ahora es %s.\n", old_name, client->name);
        broadcast_to_room(client->room, client->sockfd, namechange_msg, 1);
    }
    else if (strncmp(msg, "/join ", 6) == 0) {
        char newroom[ROOM_LEN];
        sscanf(msg + 6, "%31s", newroom);
        for (char* p = newroom; *p; p++) if (*p < 32) *p = '_';

        if (strcmp(newroom, client->room) == 0) {
            send_to_client(client, "[SERVER] ya estás en esa sala.\n");
            return 1;
        }

        pthread_mutex_lock(&clients_mutex);
        if (is_username_taken(client->name, newroom, client->sockfd)) {
            pthread_mutex_unlock(&clients_mutex);
            char err[BUFFER_SIZE];
            snprintf(err, sizeof(err), "[SERVER] tu nombre \"%s\" ya está cogido en la sala \"%s\".\n",
                     client->name, newroom);
            send_to_client(client, err);
            return 1;
        }

        char old_room[ROOM_LEN];
        strcpy(old_room, client->room);
        strncpy(client->room, newroom, ROOM_LEN-1);
        client->room[ROOM_LEN-1] = '\0';
        pthread_mutex_unlock(&clients_mutex);

        char leave_msg[BUFFER_SIZE];
        snprintf(leave_msg, sizeof(leave_msg), "[SERVER] %s se ha pirado.\n", client->name);
        broadcast_to_room(old_room, client->sockfd, leave_msg, 1);

        char join_msg[BUFFER_SIZE];
        snprintf(join_msg, sizeof(join_msg), "[SERVER] %s se ha unido.\n", client->name);
        broadcast_to_room(client->room, client->sockfd, join_msg, 1);

        char ack[BUFFER_SIZE];
        snprintf(ack, sizeof(ack), "[SERVER] ahora estás en la sala \"%s\".\n", client->room);
        send_to_client(client, ack);
    }
    else if (strcmp(msg, "/rooms") == 0) {
        list_rooms(client);
    }
    else if (strcmp(msg, "/who") == 0) {
        list_users_in_room(client);
    }
    else {
        send_to_client(client, "[SERVER] comando desconocido. usa /nick, /join, /rooms, /who.\n");
    }

    return 1;
}
#pragma once

#include "client.h"


void list_rooms(client_t* client);
void list_users_in_room(client_t* client);

/* retorna 1 si se debe continuar el bucle, 0 si no es un comando */
int handle_command(client_t* client, const char* msg);
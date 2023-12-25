

#include <iostream>
#include <vector>
#include <cstring>

#define undefined -1

#include "server.h"
#include "server_db_handler.h"

class ClientThread
{
public:
    uint8_t client_descriptor = undefined;
    bool has_client_joined_room = false;
    ClientRoom* client_joined_room = nullptr;

    void operator()(const std::vector<std::string>&);
};
#include "server_client_room.h"

ClientRoom::ClientRoom(const std::string& _room_number, uint16_t _client1_fd)
{
    room_number = _room_number;
    client1_fd = _client1_fd;
}

void ClientRoom::UpdateClients()
{
    std::string to_update = "120";
    to_update += notepad_collab;
    if(send(client1_fd, to_update.c_str(), 1024, 0) == -1)
    {
        handle_error("Couldn't update notepad for host_client\n");
    }

    if(send(client2_fd, to_update.c_str(), 1024, 0) == -1)
    {
        handle_error("Couldn't update notepad for joined_client\n");
    }
}
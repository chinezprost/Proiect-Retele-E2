#include "server_client_room.h"

ClientRoom::ClientRoom(const std::string& _room_number)
{
    room_number = _room_number;
}

void ClientRoom::UpdateClients()
{
    std::string to_update = "112";
    to_update += notepad_collab;
    if(client1_fd > 0)
        if(send(client1_fd, to_update.c_str(), 1024, 0) == -1)
        {
            handle_error("Couldn't update notepad for host_client\n");
        }
    
    if(client2_fd > 0)
        if(send(client2_fd, to_update.c_str(), 1024, 0) == -1)
        {
            handle_error("Couldn't update notepad for joined_client\n");
        }
}
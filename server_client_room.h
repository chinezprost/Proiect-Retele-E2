#ifndef SERVER_CLIENT_ROOM_H
#define SERVER_CLIENT_ROOM_H

#include <string>
#include <sys/socket.h>

#define handle_error(x) { perror(x); exit(0); }
#define undefined -1

class ClientRoom
{
public:
    std::string room_number;
    uint16_t client1_fd = undefined; 
    uint16_t client2_fd = undefined;

    std::string notepad_collab = "";

    ClientRoom(const std::string&, uint16_t);
    void UpdateClients();
};

#endif
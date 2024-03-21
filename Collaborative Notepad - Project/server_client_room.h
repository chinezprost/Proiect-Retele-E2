#ifndef SERVER_CLIENT_ROOM_H
#define SERVER_CLIENT_ROOM_H

#include <string>
#include <sys/socket.h>
#include <mutex>

#define handle_error(x) { perror(x); exit(0); }
#define undefined -1

class ClientRoom
{
public:
    std::mutex notepad_collab_i;
    std::string room_number;
    int16_t client1_fd = undefined;
    int16_t client1_cursor_pos = undefined; 
    int16_t client2_fd = undefined;
    int16_t client2_cursor_pos = undefined;

    std::string notepad_collab = "";

    ClientRoom(const std::string&);
    void UpdateClients();
};

#endif
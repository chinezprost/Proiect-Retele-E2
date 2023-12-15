#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <vector>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <cstring>
#include <thread>
#include <threads.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "server_client_room.h"
#include "server_worker.h"

#define handle_error(x) { perror(x); exit(0); }
#define undefined -1


class Server
{
private:
    uint16_t server_descriptor = undefined;

    uint16_t max_clients = undefined;
    sockaddr_in server_object;
    std::vector<std::thread> connected_clients_threads;
    static Server* server_instance;
public:
    std::vector<ClientRoom*> client_rooms;
    Server(const Server& server_object) = delete;

    std::array<uint16_t, 128> connected_clients_fds;
    uint16_t connected_clients_count = 0;
    sockaddr_in arrived_client;

    //Server(const Server&) = delete;

    Server(const uint16_t&, const uint16_t&, const uint16_t&, const uint16_t&, const uint16_t&, const uint16_t&);

    static Server* Instance(const uint16_t&, const uint16_t&, const uint16_t&, const uint16_t&, const uint16_t&, const uint16_t&);
    
    static Server* Instance();

    bool AddClientThread(std::thread);

    ClientRoom* CreateRoom(const uint16_t&);

    ClientRoom* JoinRoom(const uint16_t&, const std::string&);

    void SendToClient(const uint8_t&, const std::string&);

    const uint16_t& GetServerDescriptor();
};

#endif
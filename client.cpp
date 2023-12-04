#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <vector>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <threads.h>
#include <iostream>
#include <arpa/inet.h>

#define handle_error(x) {perror(x); exit(0);}

#define undefined -1

class client
{
private:
    uint8_t client_descriptor = -1;
    sockaddr_in client_object;

public:
    client(const ushort& _sock_type, const ushort& _sock_stream, const ushort& _protocol, std::string _ip_adress, const ushort& _port)
    {
        client_object.sin_family = _sock_type;
        client_object.sin_addr.s_addr = inet_addr(_ip_adress.c_str());
        client_object.sin_port = htons(_port);

        if((client_descriptor = socket(_sock_type, _sock_stream, _protocol)) == -1)
        {
            handle_error("Couldn't create client socket.");
        }
        uint8_t option = 1;
        setsockopt(client_descriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(uint8_t));

    }
    void try_connect()
    {
        ushort response = undefined;
        if((response = connect(client_descriptor, (sockaddr*) &client_object, sizeof(sockaddr))) == -1)
        {
            handle_error("Couldn't connect to the server!");
        }
        on_connect_to_server();
    }

    void on_connect_to_server()
    {
        printf("Connected!\n");
    }
    
    void listening_to_server_thread(uint32_t start_timespan)
    {
        int timespan = start_timespan;
        while(true)
        {
            
            printf("Listening to the server. Current timespan: %d\n", timespan += 1);
            sleep(1);
        }
    }
};

int main()
{
    client client_object(AF_INET, SOCK_STREAM, 0, "127.0.0.1", 25561);
    client_object.try_connect();
    std::thread listen_to_server(&client::listening_to_server_thread, &client_object, 0);
    listen_to_server.detach();
    char input_buffer[1024];
    while(true)
    {
        std::cin >> input_buffer;
        printf("I've read from input: %s\n", input_buffer);
    }


    printf("OK\n");



    
}

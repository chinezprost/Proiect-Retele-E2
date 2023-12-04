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
#include <signal.h>

#define handle_error(x) {perror(x); exit(0);}

#define undefined -1


int server_socket_descriptor = undefined; //used to fix the bug with setsockopt SO_REUSEADDR

void signal_handler(int signal_number)
{
    if(signal_number == SIGTSTP)
    {
        printf("Force stopping the server. (CTRL+Z)\n");
        close(server_socket_descriptor);
        exit(0);
    }
}

class server
{
private:
    uint16_t server_descriptor = undefined;
    uint16_t max_clients = undefined;
    sockaddr_in server_object;
    std::vector<std::thread> connected_clients_threads;
public:
    std::array<uint16_t, 128> connected_clients_fds;
    uint16_t connected_clients_count = 0;
    sockaddr_in arrived_client;

    server(const uint16_t& _sock_type, const uint16_t& _sock_stream, const uint16_t& _protocol, const uint16_t& _htonl, const uint16_t& _port, const uint16_t& _max_clients)
    {
        server_object.sin_family = _sock_type;
        server_object.sin_addr.s_addr = htonl(_htonl);
        server_object.sin_port = htons(_port);
        max_clients = _max_clients;

        if((server_descriptor = socket(_sock_type, _sock_stream, _protocol)) == -1)
        {
            handle_error("Couldn't create server socket.");
        }
        uint16_t option = 1;
        setsockopt(server_descriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(uint16_t));

        if(bind(server_descriptor, reinterpret_cast<sockaddr*>(&server_object), sizeof(sockaddr)) == -1)
        {
            handle_error("Couldn't bind the socket.");
        }

        if(listen(server_descriptor, max_clients) == -1)
        {
            handle_error("Couldn't listen!");
        }
    }
    bool add_client_thread(std::thread _thread)
    {
        this->connected_clients_threads.push_back(std::move(_thread));
        return true; //TODO!
    }

    

    void on_connected_client()
    {
        printf("Connected!\n");
    }
    const uint16_t& get_server_descriptor()
    {
        return this->server_descriptor;
    }


};

class client_thread
{
public:
    int8_t client_descriptor = undefined;

    void operator()(const std::vector<std::string>& thread_parameters)
    {
        client_descriptor = std::stoi(thread_parameters.at(0));
        char internal_buffer[1024];
        while(true)
        {
            recv(client_descriptor, internal_buffer, sizeof(internal_buffer), 0);
            printf("I've read from the server: %s\n", internal_buffer);
        }
    }
};

void signal_handler_logic()
{
    signal(SIGTSTP, signal_handler);
}

void on_server_close()
{
   
}



int main()
{
    signal_handler_logic();

    server server_object(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 25561, 32);
    server_socket_descriptor = server_object.get_server_descriptor();
    int8_t connected_client_descriptor = undefined;

    while(true)
    {
        socklen_t arrived_client_length = sizeof(server_object.arrived_client);
        if((connected_client_descriptor = accept(server_object.get_server_descriptor(), reinterpret_cast<sockaddr*>(&server_object.arrived_client), &arrived_client_length)) == -1)
        {
            handle_error("Couldn't accept connection");
        }
        printf("Client with ID: %d has connected.\n", connected_client_descriptor);
        std::vector<std::string> thread_parameters;
        thread_parameters.push_back(std::to_string(connected_client_descriptor));
        std::thread connected_client_thread(client_thread(), thread_parameters);
        server_object.connected_clients_fds[++server_object.connected_clients_count] = connected_client_descriptor;

        connected_client_thread.detach();
    }

    printf("OK\n");

    //char* params;
    //std::thread thread_obj(client_thread(), params);
}

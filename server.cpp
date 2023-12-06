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
#include <SFML/Graphics.hpp>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>




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

std::string random_string(ushort size = 5)
{
    std::string string_to_generate = "";
    const char possible_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZ";
    for(ushort i = 0; i < size; i++)
    {
        string_to_generate += possible_chars[rand() % strlen(possible_chars)];
    }

    return string_to_generate;
}

class client_room
{
public:
    std::string room_number;
    uint16_t client1_fd, client2_fd;

    client_room(const std::string& _room_number, uint16_t _client1_fd)
    {
        room_number = _room_number;
        client1_fd = _client1_fd;
    }
};



class server
{
private:
    uint16_t server_descriptor = undefined;
    uint16_t max_clients = undefined;
    sockaddr_in server_object;
    std::vector<std::thread> connected_clients_threads;

    static server* server_instance;

public:

    std::vector<client_room*> client_rooms;


    server(const server& server_object) = delete;

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

    static server* instance(const uint16_t& _sock_type, const uint16_t& _sock_stream, const uint16_t& _protocol, const uint16_t& _htonl, const uint16_t& _port, const uint16_t& _max_clients)
    {
        if(server_instance == nullptr)
        {
            server_instance = new server(_sock_type, _sock_stream, _protocol, _htonl, _port, _max_clients);
        }
        return server_instance;
    }

    static server* instance()
    {
        if(server_instance != nullptr)
        {
            return server_instance;
        }
        handle_error("An unexceptional error occured: No server exists.\n");
    }

    bool add_client_thread(std::thread _thread)
    {
        this->connected_clients_threads.push_back(std::move(_thread));
        return true; //TODO!
    }

    void create_room(const uint16_t& _client_fd)
    {
        printf("Begin\n");
        std::string room_id = random_string();
        client_rooms.push_back(new client_room(room_id, _client_fd));

        printf("Room with ID: %s has been created by Client: %d\n", room_id.c_str(), _client_fd);
    }

    void join_room(const uint16_t& _client_fd, const std::string& _room_code)
    {
        client_room* found_room_object = nullptr;
        for(auto i = client_rooms.begin(); i != client_rooms.end(); i++)
        {
            if((*i)->room_number == _room_code)
            {
                printf("Room %s found.\n", _room_code.c_str());
                found_room_object = *i;
            }
        }
        if(found_room_object == nullptr)
        {
            printf("Couldn't find room!");
            return;
        }

        found_room_object->client2_fd = _client_fd;

        printf("User joined room created\n");
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

void process_command(const std::string& _input)
{
    if(_input == "create_room")
    {
        
    }
}

void create_room()
{
    printf("Room created\n");
}

class client_thread
{
public:
    int8_t client_descriptor = undefined;

    void operator()(const std::vector<std::string>& thread_parameters)
    {
        client_descriptor = std::stoi(thread_parameters.at(0));
        uint16_t communication_channel_descriptor = undefined;
        char internal_buffer[1024];


        while(true)
        {
            recv(client_descriptor, internal_buffer, sizeof(internal_buffer), 0);
            printf("Received from client: %d the message: %s\n", client_descriptor, internal_buffer);
            internal_buffer[strcspn(internal_buffer, "\n")] = '\0';
            std::string internal_buffer_string(internal_buffer);
            if(internal_buffer_string == "create room")
            {
                server::instance()->create_room(client_descriptor);
            } else if(internal_buffer_string.substr(0, 9) == "join room")
            {
                server::instance()->join_room(client_descriptor, internal_buffer_string.substr(10, 14));
            }
            else
            {
                printf("fail\n");
            }
            std::string received_string(internal_buffer);
        }
    }
};

// class server_processing_thread
// {
//     static void 
// };

// class server_processing_thread
// {
// public:
//     bool is_use = false;
//     int result = mkfifo("server_client_communication_channel", 0666);

//     uint16_t communication_channel_descriptor = undefined;
//     char received_buffer[1024] = {'\0'};
//     char parsed_buffer[1024] = {'\0'};

//     void operator()(const std::vector<std::string>& thread_parameters)
//     {
//         while(true)
//         {
//             //gets input
//             communication_channel_descriptor = open("server_client_communication_channel", O_RDONLY);
//             read(communication_channel_descriptor, received_buffer, sizeof(received_buffer));

//             //processes it
            

//             close(communication_channel_descriptor);
//             communication_channel_descriptor = open("server_client_communication_channel", O_WRONLY);
//             write(communication_channel_descriptor, parsed_buffer, sizeof(parsed_buffer));
//             //return result
//         }
//     }
// };

void signal_handler_logic()
{
    signal(SIGTSTP, signal_handler);
}

void on_server_close()
{
   
}

server* server::server_instance = nullptr;


int main()
{
    signal_handler_logic();

    server* server_object = server::instance(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 25564, 32);
    server_socket_descriptor = server_object->get_server_descriptor();
    printf("%d\n", server_socket_descriptor);
    int8_t connected_client_descriptor = undefined;

    
    std::vector<std::string> server_processing_thread_parameters;

    //std::thread server_processing(server_processing_thread(), server_processing_thread_parameters);

    while(true)
    {
        socklen_t arrived_client_length = sizeof(server_object->arrived_client);
        if((connected_client_descriptor = accept(server_object->get_server_descriptor(), reinterpret_cast<sockaddr*>(&server_object->arrived_client), &arrived_client_length)) == -1)
        {
            handle_error("Couldn't accept connection");
        }
        printf("Client with ID: %d has connected.\n", connected_client_descriptor);
        std::vector<std::string> thread_parameters;
        thread_parameters.push_back(std::to_string(connected_client_descriptor));
        std::thread connected_client_thread(client_thread(), thread_parameters);
        server_object->connected_clients_fds[++server_object->connected_clients_count] = connected_client_descriptor;
        connected_client_thread.detach();
    }

    printf("OK\n");

    //char* params;
    //std::thread thread_obj(client_thread(), params);
}

#include "server.h"
Server* Server::server_instance = nullptr;

Server::Server(const uint16_t& _sock_type, const uint16_t& _sock_stream, const uint16_t& _protocol, const uint16_t& _htonl, const uint16_t& _port, const uint16_t& _max_clients)
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

Server* Server::Instance(const uint16_t& _sock_type, const uint16_t& _sock_stream, const uint16_t& _protocol, const uint16_t& _htonl, const uint16_t& _port, const uint16_t& _max_clients)
{
    if(server_instance == nullptr)
    {
        server_instance = new Server(_sock_type, _sock_stream, _protocol, _htonl, _port, _max_clients);
    }
    return server_instance;
}

Server* Server::Instance()
{
    if(server_instance != nullptr)
    {
        return server_instance;
    }
    handle_error("An unexceptional error occured: No server exists.\n");
}

bool Server::AddClientThread(std::thread _thread)
{
    this->connected_clients_threads.push_back(std::move(_thread));
    return true; //TODO!
}

ClientRoom* Server::CreateRoom(const uint16_t& _client_fd)
{
    const std::lock_guard<std::mutex> lock(client_rooms_i);
    std::string room_id = ServerWorker::random_string(5);

    auto created_room = new ClientRoom(room_id, _client_fd);
    client_rooms.push_back(created_room);

    printf("Room with ID: %s has been created by Client: %d\n", room_id.c_str(), _client_fd);

    return created_room;
}

ClientRoom* Server::JoinRoom(const uint16_t& _client_fd, const std::string& _room_code)
{
    ClientRoom* found_room_object = nullptr;

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
        printf("Client %d couldn't find the room: %s!\n", _client_fd, _room_code.c_str());
        return nullptr;
    }

    if(found_room_object->client1_fd == undefined)
    {
        found_room_object->client1_fd = _client_fd;
    }
    else
    {
        found_room_object->client2_fd = _client_fd;
    }

    printf("Client %d has joined room: %s!\n", _client_fd, _room_code.c_str());

    return found_room_object;
}

const uint16_t& Server::GetServerDescriptor()
{
    return this->server_descriptor;
}

void Server::SendToClient(const uint8_t& _client_fd, const std::string& _message)
{
    if(send(_client_fd, _message.c_str(), 1024, 0) == -1)
    {
        handle_error("Couldn't send message to client")
    }
}
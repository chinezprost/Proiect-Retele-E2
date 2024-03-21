#include "server.h"
#include "server_client_thread.h"

int main(int argc, char** argv)
{
    std::thread master_thread;

    std::vector<std::string> server_processing_thread_parameters;

    int16_t connected_client_descriptor = undefined;
    
    uint16_t _PORT = -1;
    
    if(argc > 0)
    {
        _PORT = atoi(argv[1]);
    }
    Server* server_object = Server::Instance(AF_INET, SOCK_STREAM, 0, INADDR_ANY, _PORT, 32);
    printf("%d\n", _PORT);
    while(true)
    {
        socklen_t arrived_client_length = sizeof(server_object->arrived_client);
        if((connected_client_descriptor = accept(server_object->GetServerDescriptor(), reinterpret_cast<sockaddr*>(&server_object->arrived_client), &arrived_client_length)) == -1)
        {
            handle_error("Couldn't accept connection");
        }
        printf("Client with ID: %d has connected.\n", connected_client_descriptor);
        std::vector<std::string> thread_parameters;
        thread_parameters.push_back(std::to_string(connected_client_descriptor));
        std::thread connected_client_thread(ClientThread(), thread_parameters);
        server_object->connected_clients_fds[++server_object->connected_clients_count] = connected_client_descriptor;
        connected_client_thread.detach();
    }
    printf("PROGRAM FINISHED\n");
}
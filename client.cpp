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
#include <string.h>
#include <thread>
#include <threads.h>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <arpa/inet.h>
#include <iterator>
#include <sstream>

#define handle_error(x) {perror(x); exit(0);}

#define undefined -1

class client
{
private:
    uint16_t client_descriptor = -1;
    sockaddr_in client_object;

public:
    client(const uint16_t& _sock_type, const uint16_t& _sock_stream, const uint16_t& _protocol, std::string _ip_adress, const uint16_t& _port)
    {
        client_object.sin_family = _sock_type;
        client_object.sin_addr.s_addr = inet_addr(_ip_adress.c_str());
        client_object.sin_port = htons(_port);

        if((client_descriptor = socket(_sock_type, _sock_stream, _protocol)) == -1)
        {
            handle_error("Couldn't create client socket.");
        }
        uint16_t option = 1;
        setsockopt(client_descriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(uint16_t));

    }

    const uint16_t& get_client_descriptor()
    {
        return this->client_descriptor;
    }
    void try_connect()
    {
        uint16_t response = undefined;
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
            sleep(1000);
        }
    }
};


class command
{
private:
    std::string command_name = "";
    uint16_t command_arg_count = undefined;
public:
    virtual bool action(std::vector<std::string> _params) 
    { 
        printf("not good\n");
        return true; 
    }
    command() = delete;
    command(const std::string& _command_name, const uint16_t& _command_arg_count) : command_name(_command_name), command_arg_count(_command_arg_count) {}
    const std::string& get_command_name()
    {
        return this->command_name;
    }

    const uint16_t& get_command_args_count()
    {
        return this->command_arg_count;
    }
};
class create_room : public command
{
public:
    create_room() = delete;
    create_room(const std::string& _command_name, const uint16_t& _command_arg_count) : command(_command_name, _command_arg_count)
    {

    }

    bool action(std::vector<std::string> _params) override
    {
        printf("Create room command executed.\n");
        return true;
    }
};
class connect_to_room : public command
{
public:
    connect_to_room() = delete;
    connect_to_room(const std::string& _command_name, const uint16_t& _command_arg_count) : command(_command_name, _command_arg_count)
    {

    }

    bool action(std::vector<std::string> _params) override
    {
        printf("Connect to room command executed.\n");
        return true;
    }
};

void CommandInitialization(std::vector<command*>& commands)
{
    create_room* create_room_command = new create_room("create_room", 0);
    commands.push_back(create_room_command);

    connect_to_room* connect_to_room_command = new connect_to_room("connect_to_room", 0);
    commands.push_back(connect_to_room_command);
}

//helper commands
std::vector<std::string> split_str(std::string _string_to_split)
{
    std::vector<std::string> result_vector;
    int8_t delimiter_position = undefined;
    while(_string_to_split.size() > 0)
    {
        delimiter_position = _string_to_split.find_first_of(' ');
        printf("%d with size of: %d\n", delimiter_position, _string_to_split.size());
        if(delimiter_position != -1)
        {
            std::string splitted_word = _string_to_split.substr(0, delimiter_position);
            _string_to_split = _string_to_split.substr(delimiter_position + 1);
            if(_string_to_split.size() == 0)
            {
                result_vector.emplace_back(_string_to_split);
            }
        }
        else
        {
            result_vector.emplace_back(_string_to_split);
            _string_to_split = "";
        }
    }
    return result_vector;
}

std::string trim_str(const std::string& _string_to_trim)
{
    int16_t pos_left = _string_to_trim.find_first_not_of(' ');
    if(std::string::npos == pos_left)
    {
        return _string_to_trim;
    }
    int16_t pos_right = _string_to_trim.find_last_not_of(' ');
    return _string_to_trim.substr(pos_left, (pos_right - pos_left + 1));
}


int main()
{

    std::vector<command*> commands;
    CommandInitialization(commands);
    printf("Commands initialized!\n");


    client client_object(AF_INET, SOCK_STREAM, 0, "127.0.0.1", 25561);
    client_object.try_connect();
    std::thread listen_to_server(&client::listening_to_server_thread, &client_object, 0);
    listen_to_server.detach();
    char input_buffer[1024];
    while(true)
    {

        fgets(input_buffer, sizeof(input_buffer), stdin);
        input_buffer[strcspn(input_buffer, "\n")] = '\0';
        std::string b_string = input_buffer;
        std::vector<std::string> input_params;

        std::transform(b_string.begin(), b_string.end(), b_string.begin(), [](unsigned char c) { return std::tolower(c); });
        

        bool is_command_valid = false;

        for(std::vector<command*>::iterator i = commands.begin(); i != commands.end(); i++)
        {
            std::cout << (*i)->get_command_name() << '\n';
            if(trim_str(b_string.substr(0, (b_string.find_first_of(' ')))).compare(trim_str((*i)->get_command_name())) == 0 /* && std::count(b_string.begin(), b_string.end(), ' ') - 1 == i->get_command_args_count() */ )
            {
                input_params = split_str(b_string);
                (*i)->action(input_params);
                is_command_valid = true;
                break;
            }
        }

        if(is_command_valid == false)
        {
            printf("Error: Invalid command.\n");
        }
        

        //send(client_object.get_client_descriptor(), input_buffer, sizeof(input_buffer), 0);
    }


    printf("OK\n");



    
}

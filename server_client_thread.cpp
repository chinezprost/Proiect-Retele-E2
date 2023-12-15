#include "server_client_thread.h"
///////////////////////////////////////////////
/*
server received types:
000 -> undefined_message
001 -> client_handshake
002 -> create_room
003 -> join_room
004 -> leave_room
005 -> update_notepad
006 -> update_cursor
007 -> client_heartbeat
008 -> save_file
009 -> download_file
010 -> delete_file

server response types:
100 -> undefined_response
101 -> on_create_room_succes
102 -> on_create_room_undefined_error


103 -> on_join_room_succes
104 -> on_join_room_not_found
105 -> on_join_room_full
106 -> on_join_invalid_code
107 -> on_join_already_in

108 -> on_leave_room_succes
109 -> on_leave_room_fail

110 -> on_update_notepad
111 -> on_update_cursor
108 -> on_client_heartbeat
109 -> on_save_file
110 -> on_download_file
111 -> on_delete_file
*/
///////////////////////////////////////////////

void ClientThread::operator()(const std::vector<std::string>& thread_parameters)
{
    client_descriptor = std::stoi(thread_parameters.at(0));
    char internal_buffer[1024];
    while(true)
    {
        if(recv(client_descriptor, internal_buffer, sizeof(internal_buffer), 0) == -1)
        {
            handle_error("Thread with ID: {TODO!} couldn't read message;");
        }
        
        printf("Received from client: %d the message: %s\n", client_descriptor, internal_buffer);
        internal_buffer[strlen(internal_buffer)] = '\0';
        std::string internal_buffer_string(internal_buffer);

        std::string message_header = internal_buffer_string.substr(0, 3);
        internal_buffer_string = internal_buffer_string.substr(3);

        if(message_header == "000") // undefined_message
        {

            continue;
        }

        if(message_header == "001") // client_handshake
        {
            
            continue;
        }

        if(message_header == "002") // create_room
        {
            if(client_joined_room != nullptr)
            {
                Server::Instance()->SendToClient(client_descriptor, "103");
                continue;
            }
            ClientRoom* create_room_object = Server::Instance()->CreateRoom(client_descriptor);
            if(create_room_object == nullptr)
            {
                Server::Instance()->SendToClient(client_descriptor, "102");
                continue;
            }
            
            client_joined_room = Server::Instance()->JoinRoom(client_descriptor, create_room_object->room_number);
            if(client_joined_room == nullptr)
            {
                Server::Instance()->SendToClient(client_descriptor, "100");
                continue;
            }

            Server::Instance()->SendToClient(client_descriptor, "101");
            continue;
        }

        if(message_header == "003") // join_room
        {
            if(client_joined_room != nullptr)
            {
                Server::Instance()->SendToClient(client_descriptor, "108");
                continue;
            }

            if(internal_buffer_string.size() != 5)
            {
                Server::Instance()->SendToClient(client_descriptor, "107");
                continue;
            }

            client_joined_room = Server::Instance()->JoinRoom(client_descriptor, internal_buffer_string);
            if(client_joined_room == nullptr)
            {
                Server::Instance()->SendToClient(client_descriptor, "105");
                continue;
            }

            Server::Instance()->SendToClient(client_descriptor, "104");
            continue;
        }

        if(message_header == "004") // leave_room
        {
            if(client_joined_room != nullptr)
            {
                if(client_joined_room->client1_fd == client_descriptor)
                {
                    client_joined_room->client1_fd = undefined;
                }
                if(client_joined_room->client2_fd == client_descriptor)
                {
                    client_joined_room->client2_fd = undefined;
                }
                client_joined_room = nullptr;
                Server::Instance()->SendToClient(client_descriptor, "109");
            }
            else
            {
                Server::Instance()->SendToClient(client_descriptor, "110");
            }
        }

        if(message_header == "005") // update_notepad
        {
            if(client_joined_room != nullptr)
            {
                client_joined_room->notepad_collab = internal_buffer_string;
                client_joined_room->UpdateClients();
            }
            continue;
        }

        if(message_header == "006") // update_cursor
        {
            
            continue;
        }

        if(message_header == "007") // client_heartbeat
        {
            
            continue;
        }

        if(message_header == "008") // save_file
        {
            
            continue;
        }

        if(message_header == "009") // download_file
        {
            
            continue;
        }

        if(message_header == "010") // delete_file
        {
            
            continue;
        }



    }
}
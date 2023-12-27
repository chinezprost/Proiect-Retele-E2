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
112 -> on_client_heartbeat

113 -> on_save_file_success
114 -> on_save_file_failure

115 -> on_download_file_succes
116 -> on_download_file_failure

117 -> on_delete_file_succes
118 -> on_delete_file_failure
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
                Server::Instance()->SendToClient(client_descriptor, "102");
                continue;
            }
            ClientRoom* create_room_object = Server::Instance()->CreateRoom(client_descriptor);
            if(create_room_object == nullptr)
            {
                Server::Instance()->SendToClient(client_descriptor, "103");
                continue;
            }
            
            client_joined_room = Server::Instance()->JoinRoom(client_descriptor, create_room_object->room_number);
            if(client_joined_room == nullptr)
            {
                Server::Instance()->SendToClient(client_descriptor, "100");
                continue;
            }
            client_joined_room->notepad_collab = internal_buffer_string;
            Server::Instance()->SendToClient(client_descriptor, "101" + client_joined_room->room_number);
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
                Server::Instance()->SendToClient(client_descriptor, "109");
                continue;
            }

            Server::Instance()->SendToClient(client_descriptor, "104" + client_joined_room->room_number);
            Server::Instance()->SendToClient(client_descriptor, "204" + client_joined_room->notepad_collab);
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
                Server::Instance()->SendToClient(client_descriptor, "110");
            }
            else
            {
                Server::Instance()->SendToClient(client_descriptor, "111");
            }
        }

        if(message_header == "005") // update_notepad
        {
            const std::lock_guard<std::mutex> lock(client_joined_room->notepad_collab_i);
            if(client_joined_room != nullptr)
            {
                
                client_joined_room->notepad_collab = internal_buffer_string;
                client_joined_room->UpdateClients();
            }
            continue;
        }

        if(message_header == "006") // update_cursor
        {
            if(client_joined_room != nullptr)
            {
                if(client_descriptor == client_joined_room->client1_fd)
                {
                    client_joined_room->client1_cursor_pos = atoi(internal_buffer_string.c_str());
                }
                else if(client_descriptor == client_joined_room->client2_fd)
                {
                    client_joined_room->client2_cursor_pos = atoi(internal_buffer_string.c_str());
                }
                else
                {
                    printf("An unexceptional error has occured! Line 168, s_c_thread.cpp");
                }

                Server::Instance()->SendToClient(client_joined_room->client1_fd, std::string("113") + std::to_string(client_joined_room->client2_cursor_pos));
                Server::Instance()->SendToClient(client_joined_room->client2_fd, std::string("113") + std::to_string(client_joined_room->client1_cursor_pos));
            }

            continue;
        }

        if(message_header == "007") // client_heartbeat
        {
            
            continue;
        }

        if(message_header == "008") // save_file
        {
            std::string saved_file_unique_id = ServerWorker::random_string_no_start_digit(6);

            std::string query;
            query = std::string("INSERT INTO DOCUMENTS (UNIQ_ID, CONTENT) VALUES ('" + saved_file_unique_id + "', '" + internal_buffer_string + "');");
            bool result = Server::Instance()->database_handler.SQL_Insert(query);
            if(result)
            {
                Server::Instance()->SendToClient(client_descriptor, "115" + saved_file_unique_id);
            }
            else
            {
                Server::Instance()->SendToClient(client_descriptor, "116");
            }
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
        if(message_header == "011")
        {
            std::string query = std::string("SELECT CONTENT FROM DOCUMENTS WHERE UNIQ_ID = '" + internal_buffer_string + "';");
            std::string* result = Server::Instance()->database_handler.SQL_Find(query);
            if(result != nullptr)
            {
                Server::Instance()->SendToClient(client_descriptor, "204" + *result);
            }
            else
            {
                Server::Instance()->SendToClient(client_descriptor, "116");
            }
            continue;
        }



    }
}
#include "server_worker.h"

std::string ServerWorker::random_string(const ushort& _size)
{
    std::string string_to_generate = "";
    const char possible_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZ";
    for(ushort i = 0; i < _size; i++)
    {
        string_to_generate += possible_chars[rand() % strlen(possible_chars)];
    }
    return string_to_generate;
}
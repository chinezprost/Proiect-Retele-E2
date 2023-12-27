#include "server_worker.h"
#include <chrono>

std::string ServerWorker::random_string(const ushort& _size)
{
    std::string string_to_generate = "";
    const char possible_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZ";
    for(ushort i = 0; i < _size; i++)
    {
        unsigned int time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        string_to_generate += possible_chars[rand_r(&time + i) % strlen(possible_chars)];
    }
    return string_to_generate;
}


std::string ServerWorker::random_string_no_start_digit(const ushort& _size)
{
    std::string string_to_generate = "";
    const char possible_chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZ";
    const char possible_chars_no_digit[] =  "ABCDEFGHIJKLMNOPQRSTUVXYZ";

    unsigned int time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    string_to_generate += possible_chars_no_digit[rand_r(&time) % strlen(possible_chars_no_digit)];
    
    for(ushort i = 1; i < _size; i++)
    {
        unsigned int time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        string_to_generate += possible_chars[rand_r(&time + i) % strlen(possible_chars)];
    }
    return string_to_generate;
}
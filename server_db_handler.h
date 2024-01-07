#ifndef SERVER_DB_HANDLER_H
#define SERVER_DB_HANDLER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <string.h>

class SQL_Handler
{
protected:
    sqlite3 *data_base_object  = nullptr;
    char** data_base_error_msg = nullptr;
    std::vector<std::vector<std::string>> db_result;
public:

    struct querry_result_t
    {
        std::string result;
    } querry_result;
    static int SQL_Callback_Handler(void*, int, char**, char**);

    SQL_Handler    (const std::string&);
    bool SQL_Create(const std::string&);
    bool SQL_Insert(const std::string&);
    bool SQL_Delete(const std::string&);
    std::string SQL_Find  (const std::string&);
};

#endif
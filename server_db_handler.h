#ifndef SERVER_DB_HANDLER_H
#define SERVER_DB_HANDLER_H

#include <sqlite3.h>
#include <string>


class SQL_Handler
{
protected:
    sqlite3 *data_base_object  = nullptr;
    char** data_base_error_msg = nullptr;
public:

    static int SQL_Callback_Handler(void*, int, char**, char**);

    SQL_Handler(std::string);
    bool SQL_Create(std::string);
    bool SQL_Insert(std::string);
    bool SQL_Delete(std::string);
    bool SQL_Find(std::string);
};

#endif
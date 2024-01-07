

#include "server_db_handler.h"

SQL_Handler::SQL_Handler(const std::string& _db_name)
{
    if(sqlite3_open(_db_name.c_str(), &this->data_base_object) == true)
    {
        perror("Couldn't open database.");
    }
    else
    {
        printf("Connected to database succesfully.\n");
        SQL_Handler::SQL_Create("CREATE TABLE DOCUMENTS(" 
            "ID INTEGER PRIMARY KEY AUTOINCREMENT   ," 
            "UNIQ_ID        VARCHAR(50)         NOT NULL,"
            "CONTENT        VARCHAR(65000)"
            ")");
    }
}

int SQL_Handler::SQL_Callback_Handler(void* data, int argc, char** argv, char** az_col_name)
{
    querry_result_t *querry_result_a = (querry_result_t *)data;
    printf("%d\n", argc);
    for(int i = 0; i < argc; i++)
    {
        if(strcmp(az_col_name[i], "CONTENT") == 0)
        {
            querry_result_a->result = std::string(argv[i]);
        }
    }
    return 0;
}

bool SQL_Handler::SQL_Create(const std::string& _input)
{
    if(sqlite3_exec(this->data_base_object, _input.c_str(), this->SQL_Callback_Handler, &this->querry_result, this->data_base_error_msg) != SQLITE_OK)
    {
        printf("SQL error: %s \n", sqlite3_errmsg(this->data_base_object));
        sqlite3_free(this->data_base_error_msg);
    }
    else
    {
        printf("Table has been created succesfully.\n");
    }

    return true;
}

bool SQL_Handler::SQL_Insert(const std::string& _input)
{
    if(sqlite3_exec(this->data_base_object, _input.c_str(), this->SQL_Callback_Handler, 0, this->data_base_error_msg) != SQLITE_OK)
    {
        printf("SQL error: %s \n", sqlite3_errmsg(this->data_base_object));
        sqlite3_free(this->data_base_error_msg);
    }
    else
    {
        printf("Data was inserted succesfully.\n");
    }

    return true;
}

std::string SQL_Handler::SQL_Find(const std::string& _input)
{
    if(sqlite3_exec(this->data_base_object, _input.c_str(), this->SQL_Callback_Handler, &this->querry_result, this->data_base_error_msg) != SQLITE_OK)
    {
        printf("SQL error: %s \n", sqlite3_errmsg(this->data_base_object));
        sqlite3_free(this->data_base_error_msg);
    }
    else
    {
        printf("Found.\n");
        return this->querry_result.result;
    }
    return nullptr;
}

bool SQL_Handler::SQL_Delete(const std::string& _input)
{
    if(sqlite3_exec(this->data_base_object, _input.c_str(), this->SQL_Callback_Handler, &this->querry_result, this->data_base_error_msg) != SQLITE_OK)
    {
        printf("SQL error: %s \n", sqlite3_errmsg(this->data_base_object));
        sqlite3_free(this->data_base_error_msg);
        return false;
    }
    else
    {
        printf("Deleted.\n");
        return true;
    }
}
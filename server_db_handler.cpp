

#include "server_db_handler.h"

SQL_Handler::SQL_Handler(std::string _db_name)
{
    if(sqlite3_open(_db_name.c_str(), &this->data_base_object) == true)
    {
        perror("Couldn't open database.");
    }
    else
    {
        printf("Connected to database succesfully.\n");
        SQL_Handler::SQL_Create("CREATE TABLE DOCUMENTS(" 
            "ID INT PRIMARY KEY             NOT NULL," 
            "UNIQ_ID        VARCHAR(50)     NOT NULL,"
            "CONTENT        VARCHAR(65000)"
            ")");
    }
}

int SQL_Handler::SQL_Callback_Handler(void* not_used, int argc, char** argv, char** az_col_name)
{
    for(int i = 0; i < argc; i++)
    {
        printf("%s = %s\n", az_col_name[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

bool SQL_Handler::SQL_Create(std::string _input)
{
    if(sqlite3_exec(this->data_base_object, _input.c_str(), this->SQL_Callback_Handler, 0, this->data_base_error_msg) != SQLITE_OK)
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
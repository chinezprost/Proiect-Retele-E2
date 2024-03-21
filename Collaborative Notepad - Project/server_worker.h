
#include <string>
#include <cstring>

class ServerWorker
{
public:
    ServerWorker();
    static std::string random_string(const ushort&);
    static std::string random_string_no_start_digit(const ushort&);

};
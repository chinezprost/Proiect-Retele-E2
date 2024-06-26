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
#include <SFML/Graphics.hpp>
#include <set>
#include <fstream>

#define handle_error(x) \
    {                   \
        perror(x);      \
        exit(0);        \
    }

#define undefined -1

const uint16_t WINDOW_WIDTH = 920;
const uint16_t WINDOW_HEIGHT = 520;

// static variables

static sf::Font arial_font;
sf::Text status_text;
sf::Clock elasped_time;

std::string current_room_id;

enum current_state_enum
{
    NOT_CONNECTED,
    CONNECTED_TO_SERVER,
    CONNECTED_TO_ROOM,
    CANT_FIND_ROOM
};

current_state_enum current_status = current_state_enum::NOT_CONNECTED;

bool draw_popup_window = false;
bool draw_popup_open_window = false;
bool draw_popup_delete_window = false;

std::string popup_window_string = "";
std::string popup_window_open_string = "";
std::string popup_window_delete_string = "";

int client_input_cursor_position = 0;
int text_input_cursor_position = 0;

// antet

void update_notepad(const std::string &_string);

class interface_object : public sf::Drawable
{
public:
    sf::Shape *interface_object_drawable;

    sf::Vector2f position;
    sf::Vector2f size;
    sf::Vector2f color;

    float rotation;
    uint8_t thickness = 0;
    interface_object() = default;
    interface_object(const sf::Vector2f &_position, const float &_rotation, const sf::Vector2f &_size, const uint8_t &_thickness, const sf::Color &_color) : position(_position), rotation(_rotation), size(_size), thickness(_thickness) {}
    interface_object(const sf::Vector2f &_position, const float &_rotation, const uint8_t &_thickness, const sf::Color &_color, sf::Shape *_interface_object_drawable) : position(_position), rotation(_rotation), thickness(_thickness)
    {
        interface_object_drawable = _interface_object_drawable;
        interface_object_drawable->setPosition(_position);
        interface_object_drawable->setRotation(_rotation);
        interface_object_drawable->setFillColor(_color);
        interface_object_drawable->setOutlineThickness(_thickness);
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        // _show(this->interface_object_drawable);
        target.draw(*(this->interface_object_drawable));
    }
};

class popup_alert : public interface_object
{
public:
    sf::Text popup_alert_text;
    sf::RectangleShape popup_alert_body;
    sf::RectangleShape popup_alert_body_shadow;
    uint16_t time_to_show = 1;

    popup_alert(const std::string &_text, const sf::Color &_color)
    {
        popup_alert_body_shadow.setSize(sf::Vector2f(WINDOW_WIDTH / 2.5, WINDOW_HEIGHT / 16));
        popup_alert_body.setSize(sf::Vector2f(WINDOW_WIDTH / 2.5, WINDOW_HEIGHT / 16));

        popup_alert_body.setFillColor(sf::Color(200, 200, 200, 200));
        popup_alert_body_shadow.setFillColor(sf::Color(0, 0, 0, 100));

        popup_alert_body.setPosition(WINDOW_WIDTH / 2 - popup_alert_body.getSize().x / 2, WINDOW_HEIGHT / 2 - popup_alert_body.getSize().y / 2);
        popup_alert_body_shadow.setPosition(WINDOW_WIDTH / 2 - popup_alert_body.getSize().x / 2 - 3, WINDOW_HEIGHT / 2 - popup_alert_body.getSize().y / 2 + 3);

        popup_alert_text.setCharacterSize(20);
        popup_alert_text.setFont(arial_font);
        popup_alert_text.setString(_text);
        popup_alert_text.setFillColor(_color);

        popup_alert_text.setOrigin(sf::Vector2f(popup_alert_text.getGlobalBounds().width, popup_alert_text.getGlobalBounds().height) / 2.f + sf::Vector2f(popup_alert_text.getLocalBounds().left, popup_alert_text.getLocalBounds().top));
        popup_alert_text.setPosition(popup_alert_body.getPosition() + popup_alert_body.getSize() / 2.f);
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        target.draw(popup_alert_body_shadow);
        target.draw(popup_alert_body);
        target.draw(popup_alert_text);
    }
};

popup_alert popup_alert_object("undefined", sf::Color::Black);

class client
{
private:
    uint16_t client_descriptor;
    sockaddr_in client_object;

    static client *client_instance;

public:
    client(const client &client_object) = delete;

    // static client& instance(const uint16_t& _sock_type, const uint16_t& _sock_stream, const uint16_t& _protocol, std::string _ip_adress, const uint16_t& _port) {
    //     client_instance = client(_sock_type, _sock_stream, _protocol, _ip_adress, _port);
    //     return client_instance;
    // }

    // static client& instance()
    // {
    //     return client_instance;
    // }

    // client(const uint16_t& _sock_type, const uint16_t& _sock_stream, const uint16_t& _protocol, std::string _ip_adress, const uint16_t& _port)
    // {
    //     client_object.sin_family = _sock_type;
    //     client_object.sin_addr.s_addr = inet_addr(_ip_adress.c_str());
    //     client_object.sin_port = htons(_port);

    //     if((client_descriptor = socket(_sock_type, _sock_stream, _protocol)) == -1)
    //     {
    //         handle_error("Couldn't create client socket.");
    //     }
    //     uint16_t option = 1;
    //     setsockopt(client_descriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(uint16_t));

    // }

    client(const uint16_t &_sock_type, const uint16_t &_sock_stream, const uint16_t &_protocol, std::string _ip_adress, const uint16_t &_port)
    {
        client_object.sin_family = _sock_type;
        client_object.sin_addr.s_addr = inet_addr(_ip_adress.c_str());
        client_object.sin_port = htons(_port);

        if ((client_descriptor = socket(_sock_type, _sock_stream, _protocol)) == -1)
        {
            handle_error("Couldn't create client socket.");
        }
        uint16_t option = 1;
        setsockopt(client_descriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(uint16_t));
    }

    static client *instance(const uint16_t &_sock_type, const uint16_t &_sock_stream, const uint16_t &_protocol, std::string _ip_adress, const uint16_t &_port)
    {
        if (client_instance == nullptr)
        {
            client_instance = new client(_sock_type, _sock_stream, _protocol, _ip_adress, _port);
        }
        return client_instance;
    }

    static client *instance()
    {
        return client_instance;
    }

    const uint16_t &get_client_descriptor()
    {
        return this->client_descriptor;
    }
    void try_connect()
    {
        uint16_t response = undefined;
        if ((response = connect(client_descriptor, (sockaddr *)&client_object, sizeof(sockaddr))) == -1)
        {
            handle_error("Couldn't connect to the server!");
        }
        on_connect_to_server();
    }

    void on_connect_to_server()
    {
        printf("Connected!\n");
    }

    void send_string(std::string _string)
    {
        if (write(client_descriptor, _string.c_str(), 1024) == -1)
        {
            handle_error("couldn't send to server\n");
        }
    }

    void send_message_to_server(const std::string &_header, const std::string _message)
    {
        std::string computed_message = _header + _message;
        if (send(client_descriptor, computed_message.c_str(), 1024, 0) == -1)
        {
            handle_error("Couldn't send to the server.\n");
        }
    }

    static void listening_to_server_thread(uint32_t start_timespan)
    {
        int timespan = start_timespan;
        char received_string[1024];
        while (true)
        {
            uint16_t read_bytes = recv(client::instance()->client_descriptor, received_string, sizeof(received_string), 0);
            if (read_bytes == -1)
            {
                handle_error("Couldn't receive message from server.\n");
            }

            if(read_bytes == 0)
            {
                printf("connection disconnect\n");
                exit(0);
            }

            std::string received_header = std::string(received_string).substr(0, 3);
            std::string received_string_string = std::string(received_string).substr(3);

            printf("Received header: %s\n", received_header.c_str());

            if (received_header == "101")
            {
                popup_alert_object = popup_alert(std::string("Created and joined room with ID:" + received_string_string), sf::Color::Green);
                current_room_id = received_string_string;
                elasped_time.restart();
                current_status = current_state_enum::CONNECTED_TO_ROOM;
            }
            if (received_header == "103")
            {
                popup_alert_object = popup_alert(std::string("Failed to create room!" + received_string_string), sf::Color::Red);
                current_room_id = received_string_string;
                elasped_time.restart();
            }
            if (received_header == "104")
            {
                popup_alert_object = popup_alert(std::string("Joined room with ID:" + received_string_string), sf::Color::Green);
                current_room_id = received_string_string;
                current_status = current_state_enum::CONNECTED_TO_ROOM;
                elasped_time.restart();
            }
            if (received_header == "204")
            {
                update_notepad(received_string_string);
            }
            if (received_header == "105")
            {
                popup_alert_object = popup_alert(std::string("Can't join room with ID:" + received_string_string), sf::Color::Red);
                current_room_id = received_string_string;
                current_status = current_state_enum::CONNECTED_TO_SERVER;
            }
            if (received_header == "110")
            {
                popup_alert_object = popup_alert(std::string("You've left the current room!"), sf::Color::Red);
                elasped_time.restart();
                current_status = current_state_enum::CONNECTED_TO_SERVER;
            }
            if (received_header == "112")
            {
                update_notepad(received_string_string);
            }
            if (received_header == "113")
            {
                uint16_t _incoming_pos = atoi(received_string_string.c_str());
                if (_incoming_pos <= 0)
                {
                    _incoming_pos = 0;
                }
                client_input_cursor_position = _incoming_pos;
            }
            if (received_header == "115")
            {
                popup_alert_object = popup_alert(std::string("File saved succesfully with ID:" + received_string_string), sf::Color::Green);
                elasped_time.restart();
            }
            if (received_header == "116")
            {
                popup_alert_object = popup_alert(std::string("Failed to save file to database!"), sf::Color::Red);
                elasped_time.restart();
            }

            if (received_header == "117")
            {
                popup_alert_object = popup_alert(std::string("Document deleted from the database!"), sf::Color::Green);
                elasped_time.restart();
            }

            if (received_header == "118")
            {
                popup_alert_object = popup_alert(std::string("Couldn't delete document from database!"), sf::Color::Red);
                elasped_time.restart();
            }
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
    command(const std::string &_command_name, const uint16_t &_command_arg_count) : command_name(_command_name), command_arg_count(_command_arg_count) {}
    const std::string &get_command_name()
    {
        return this->command_name;
    }

    const uint16_t &get_command_args_count()
    {
        return this->command_arg_count;
    }
};
class create_room : public command
{
public:
    create_room() = delete;
    create_room(const std::string &_command_name, const uint16_t &_command_arg_count) : command(_command_name, _command_arg_count)
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
    connect_to_room(const std::string &_command_name, const uint16_t &_command_arg_count) : command(_command_name, _command_arg_count)
    {
    }

    bool action(std::vector<std::string> _params) override
    {
        printf("Connect to room command executed.\n");
        return true;
    }
};
class send_to_server : public command
{
public:
    send_to_server() = delete;
    send_to_server(const std::string &_command_name, const uint16_t &_command_arg_count) : command(_command_name, _command_arg_count)
    {
    }

    bool action(std::vector<std::string> _params) override
    {
        send(std::stoi(_params.back()), _params[0].c_str(), sizeof(_params[0].c_str()), 0);
        return true;
    }
};

void CommandInitialization(std::vector<command *> &commands)
{
    create_room *create_room_command = new create_room("create_room", 0);
    commands.push_back(create_room_command);

    connect_to_room *connect_to_room_command = new connect_to_room("connect_to_room", 0);
    commands.push_back(connect_to_room_command);

    send_to_server *send_to_server_command = new send_to_server("send", 1);
    commands.push_back(send_to_server_command);
}

// helper commands
std::vector<std::string> split_str(std::string _string_to_split)
{
    std::vector<std::string> result_vector;
    int8_t delimiter_position = undefined;
    while (_string_to_split.size() > 0)
    {
        delimiter_position = _string_to_split.find_first_of(' ');
        printf("%d with size of: %d\n", delimiter_position, (int)_string_to_split.size());
        if (delimiter_position != -1)
        {
            std::string splitted_word = _string_to_split.substr(0, delimiter_position);
            _string_to_split = _string_to_split.substr(delimiter_position + 1);
            if (_string_to_split.size() == 0)
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

std::string trim_str(const std::string &_string_to_trim)
{
    int16_t pos_left = _string_to_trim.find_first_not_of(' ');
    if (std::string::npos == (const long unsigned int)pos_left)
    {
        return _string_to_trim;
    }
    int16_t pos_right = _string_to_trim.find_last_not_of(' ');
    return _string_to_trim.substr(pos_left, (pos_right - pos_left + 1));
}

void _show(sf::Shape *_shape)
{
    printf("pos-x: %f, pos-y: %f, rot: %f, size-x: %f, size-y: %f, color:%d %d %d %d\n",
           _shape->getPosition().x, _shape->getPosition().y, _shape->getRotation(), static_cast<sf::RectangleShape *>(_shape)->getSize().x, static_cast<sf::RectangleShape *>(_shape)->getSize().y, _shape->getFillColor().r, _shape->getFillColor().g, _shape->getFillColor().b, _shape->getFillColor().a);
}

// sfml-graphic

class button : public interface_object
{
public:
    sf::RectangleShape button_shape;
    sf::Text button_text;

    // void(*on_press_function)(std::vector<std::string>);
    std::function<void(std::vector<std::string>)> on_press_function = nullptr;
    std::vector<std::string> function_on_press_parameters;

    // void(*on_hover_function)(std::vector<std::string>);
    std::function<void(std::vector<std::string>)> on_hover_function = nullptr;
    std::vector<std::string> function_on_hover_parameters;

    bool isHovered = false;
    bool hasBeenPressed = false;

    button(
        sf::Vector2f _position, float _rotation = 0.0, sf::Vector2f _size = sf::Vector2f(1, 1), uint8_t _thickness = 0,
        std::string _text = "", uint8_t _font_size = 24,
        std::function<void(std::vector<std::string>)> _on_press_function = nullptr,
        std::vector<std::string> _function_on_press_parameters = {},
        std::function<void(std::vector<std::string>)> _on_hover_function = nullptr,
        std::vector<std::string> _function_on_hover_parameters = {}) : interface_object(_position, _rotation, _size, _thickness, sf::Color::White)

    {
        button_shape.setSize(size);
        button_shape.setOutlineColor(sf::Color::White);
        button_shape.setFillColor(sf::Color::White);
        button_shape.setOutlineThickness(thickness);
        button_shape.setPosition(position);
        button_shape.setRotation(rotation);

        button_text.setFont(arial_font);
        button_text.setPosition(position);
        button_text.setFillColor(sf::Color::Black);
        button_text.setString(_text);
        button_text.setCharacterSize(_font_size);

        on_press_function = _on_press_function;
        on_hover_function = _on_hover_function;
        function_on_press_parameters = _function_on_press_parameters;
        function_on_hover_parameters = _function_on_hover_parameters;
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        target.draw(button_shape);
        target.draw(button_text);
    }

    void on_press()
    {
        if (!hasBeenPressed)
        {
            if (on_press_function != nullptr)
                on_press_function(function_on_press_parameters);
            hasBeenPressed = true;
        }
    }

    void on_hover()
    {
        if (!this->isHovered)
        {
            if (on_hover_function != nullptr)
                on_hover_function(function_on_hover_parameters);
            this->isHovered = true;
        }
    }

    void on_unhover()
    {
        this->isHovered = false;
    }
};

class text_input : public interface_object
{
public:
    sf::Text text_input_string;
    sf::String client_text_input_string;

    text_input()
    {
        // PLACE HOLDER
        text_input_string.setFont(arial_font);
        text_input_string.setPosition(sf::Vector2f(6, 30));
        text_input_string.setFillColor(sf::Color::White);
        text_input_string.setCharacterSize(24);
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        target.draw(text_input_string);
    }
};
text_input text_input_object;

void update_notepad(const std::string &_string)
{
    text_input_object.client_text_input_string = _string;
    text_input_object.text_input_string.setString(_string);
}
class menu_interface : public interface_object
{
public:
    std::vector<button> menu_buttons;
    class room_settings
    {
    public:
        std::vector<button> room_buttons;
        std::vector<std::string> create_room_press_parameters, create_room_hover_parameters;
        std::vector<std::string> join_room_press_parameters, join_room_hover_parameters;
        std::vector<std::string> leave_room_press_parameters, leave_room_hover_parameters;

        room_settings()
        {
            auto create_room_press_function = [this](std::vector<std::string> _parameters)
            {
                client::instance()->send_message_to_server("002", text_input_object.client_text_input_string);
            };

            auto create_room_hover_function = [this](std::vector<std::string> _parameters)
            {
                // printf("On hover file_button\n");
            };
            button create_room_button(sf::Vector2f(160, 20), 0, sf::Vector2f(80, 20), 0, "Create", 16, create_room_press_function, create_room_press_parameters, create_room_hover_function, create_room_hover_parameters);
            room_buttons.push_back(create_room_button);

            auto join_room_press_function = [this](std::vector<std::string> _parameters)
            {
                draw_popup_window = true;
                draw_popup_open_window = false;
                draw_popup_delete_window = false;
            };

            auto join_room_hover_function = [this](std::vector<std::string> _parameters)
            {
                // printf("On hover join_room button\n");
            };
            button join_room_button(sf::Vector2f(160, 40), 0, sf::Vector2f(80, 20), 0, "Join", 16, join_room_press_function, join_room_press_parameters, join_room_hover_function, join_room_hover_parameters);
            room_buttons.push_back(join_room_button);

            auto leave_room_press_function = [this](std::vector<std::string> _parameters)
            {
                client::instance()->send_message_to_server("004", "");
                current_status = current_state_enum::CONNECTED_TO_SERVER;
            };

            auto leave_room_hover_function = [this](std::vector<std::string> _parameters)
            {
                // printf("On hover leave room button\n");
            };
            button leave_room_button(sf::Vector2f(160, 60), 0, sf::Vector2f(80, 20), 0, "Leave", 16, leave_room_press_function, leave_room_press_parameters, leave_room_hover_function, leave_room_hover_parameters);
            room_buttons.push_back(leave_room_button);
        }
    };

    class file_settings
    {
    public:
        std::vector<button> file_buttons;
        std::vector<std::string> save_button_press_parameters, download_button_press_parameters, open_button_press_parameters, delete_button_press_parameters;
        std::vector<std::string> save_button_hover_parameters, download_button_hover_parameters, open_button_hover_parameters, delete_button_hover_parameters;

        file_settings()
        {
            auto save_button_press_function = [this](std::vector<std::string> _parameters)
            {
                client::instance()->send_message_to_server("008", text_input_object.text_input_string.getString());
            };

            auto save_button_hover_function = [this](std::vector<std::string> _parameters)
            {
                // printf("On hover save button\n");
            };

            button save_button(sf::Vector2f(0, 20), 0, sf::Vector2f(80, 20), 0, "Save", 16, save_button_press_function, save_button_press_parameters, save_button_hover_function, save_button_hover_parameters);
            file_buttons.push_back(save_button);

            auto download_button_press_function = [this](std::vector<std::string> _parameters)
            {
                std::ofstream ofstream_file(std::string("collab_notepad_saved_" + current_room_id));
                ofstream_file << text_input_object.client_text_input_string.toAnsiString();
                ofstream_file.close();

                popup_alert_object = popup_alert(std::string("File downloaded!"), sf::Color::Green);
                elasped_time.restart();
            };

            auto download_button_hover_function = [this](std::vector<std::string> _parameters)
            {
                // printf("On hover download button.\n");
            };
            button download_button(sf::Vector2f(0, 40), 0, sf::Vector2f(80, 20), 0, "Download", 16, download_button_press_function, download_button_press_parameters, download_button_hover_function, download_button_hover_parameters);
            file_buttons.push_back(download_button);

            auto open_button_press_function = [this](std::vector<std::string> _parameters)
            {
                draw_popup_open_window = true;
                draw_popup_window = false;
                draw_popup_delete_window = false;
            };

            auto open_button_hover_function = [this](std::vector<std::string> _parameters)
            {
                // printf("On hover open button.\n");
            };
            button open_button(sf::Vector2f(0, 60), 0, sf::Vector2f(80, 20), 0, "Open", 16, open_button_press_function, open_button_press_parameters, open_button_hover_function, open_button_hover_parameters);
            file_buttons.push_back(open_button);

            auto delete_button_press_function = [this](std::vector<std::string> _parameters)
            {
                draw_popup_delete_window = true;
                draw_popup_window = false;
                draw_popup_open_window = false;
            };

            auto delete_button_hover_function = [this](std::vector<std::string> _parameters)
            {
                // printf("On hover delete button.\n");
            };
            button delete_button(sf::Vector2f(0, 80), 0, sf::Vector2f(80, 20), 0, "Delete", 16, delete_button_press_function, delete_button_press_parameters, delete_button_hover_function, delete_button_hover_parameters);
            file_buttons.push_back(delete_button);
        }
    };

    room_settings room_settings_interface;
    file_settings file_settings_interface;

    menu_interface()
    {

        std::vector<std::string> file_button_press_parameters, file_button_hover_parameters;
        auto file_button_press_function = [this](std::vector<std::string> _parameters)
        {
            printf("Pressed file button\n");
        };

        auto file_button_hover_function = [this](std::vector<std::string> _parameters)
        {
            // printf("On hover file_button\n");
        };
        button file_button(sf::Vector2f(0, 0), 0, sf::Vector2f(80, 20), 0, "File", 16, file_button_press_function, file_button_press_parameters, file_button_hover_function, file_button_hover_parameters);

        std::vector<std::string> edit_button_press_parameters, edit_button_hover_parameters;
        auto edit_button_press_function = [this](std::vector<std::string> _parameters)
        {
            text_input_object.client_text_input_string.erase(0, text_input_object.client_text_input_string.getSize());
            text_input_object.text_input_string.setString(text_input_object.client_text_input_string);
            text_input_cursor_position = 0;
            client::instance()->send_message_to_server("005", text_input_object.client_text_input_string.toAnsiString().c_str());
        };

        auto edit_button_hover_function = [this](std::vector<std::string> _parameters)
        {
            // printf("hover edited button\n");
        };
        button edit_button(sf::Vector2f(80, 0), 0, sf::Vector2f(80, 20), 0, "Clear", 16, edit_button_press_function, edit_button_press_parameters, edit_button_hover_function, edit_button_hover_parameters);

        std::vector<std::string> room_button_press_parameters, room_button_hover_parameters;
        auto room_button_press_function = [this](std::vector<std::string> _parameters) {

        };

        auto room_button_hover_function = [this](std::vector<std::string> _parameters) {

        };
        button room_button(sf::Vector2f(160, 0), 0, sf::Vector2f(80, 20), 0, "Room", 16, room_button_press_function, room_button_press_parameters, room_button_hover_function, room_button_hover_parameters);

        menu_buttons.push_back(file_button);
        menu_buttons.push_back(edit_button);
        menu_buttons.push_back(room_button);
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        bool is_room_button_hovered = false;
        bool is_file_settings_hovered = false;

        static bool has_been_room_button_hovered = false;
        static bool has_been_file_settings_hovered = false;

        for (auto i = menu_buttons.begin(); i != menu_buttons.end(); i++)
        {
            if (i->button_text.getString() == "Room" && i->isHovered)
            {
                is_room_button_hovered = true;
            }

            if (i->button_text.getString() == "File" && i->isHovered)
            {
                is_file_settings_hovered = true;
            }
            target.draw(i->button_shape);
            target.draw(i->button_text);
        }

        bool found_any_room_hovered_button = false;
        for (auto i = room_settings_interface.room_buttons.begin(); i != room_settings_interface.room_buttons.end(); i++)
        {
            if (i->isHovered == true)
            {
                found_any_room_hovered_button = true;
                break;
            }
        }

        for (auto j = room_settings_interface.room_buttons.begin(); j != room_settings_interface.room_buttons.end(); j++)
        {
            if ((found_any_room_hovered_button && has_been_room_button_hovered) || is_room_button_hovered)
            {
                target.draw(j->button_shape);
                target.draw(j->button_text);
            }
            else
            {
                has_been_room_button_hovered = false;
            }
        }

        if (is_room_button_hovered)
        {
            has_been_room_button_hovered = true;
        }

        bool found_any_file_settings_hovered_button = false;
        for (auto i = file_settings_interface.file_buttons.begin(); i != file_settings_interface.file_buttons.end(); i++)
        {
            if (i->isHovered == true)
            {
                found_any_file_settings_hovered_button = true;
                break;
            }
        }

        for (auto j = file_settings_interface.file_buttons.begin(); j != file_settings_interface.file_buttons.end(); j++)
        {
            if ((found_any_file_settings_hovered_button && has_been_file_settings_hovered) || is_file_settings_hovered)
            {
                target.draw(j->button_shape);
                target.draw(j->button_text);
            }
            else
            {
                has_been_file_settings_hovered = false;
            }
        }

        if (is_file_settings_hovered)
        {
            has_been_file_settings_hovered = true;
        }
    }
};

class static_interface_class : public sf::Drawable
{
    std::vector<interface_object> static_interface_objects;

public:
    static_interface_class() = default;
    void create_static_interface_object(const sf::Vector2f _position, const float &_rotation, const float &_thickness, const sf::Color &_color, sf::Shape *_shape)
    {
        interface_object static_navbar(_position, _rotation, _thickness, _color, _shape);
        static_interface_objects.push_back(static_navbar);
    }

    void static_object_init()
    {
        this->create_static_interface_object(sf::Vector2f(240, 0), 0.0, 0, sf::Color(255, 255, 255, 255), new sf::RectangleShape(sf::Vector2f(1280 - 240, 20))); // navbar
        this->create_static_interface_object(sf::Vector2f(0, 20), 0.0, 0, sf::Color(39, 40, 34, 255), new sf::RectangleShape(sf::Vector2f(1280, 720)));
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        for (auto i = static_interface_objects.begin(); i != static_interface_objects.end(); i++)
        {
            i->draw(target, states);
        }
    }
};

class popup_window : public interface_object
{
public:
    std::vector<button> popup_window_buttons;
    sf::RectangleShape popup_window_shape;
    sf::RectangleShape popup_window_shape_shadow;
    sf::RectangleShape popup_input_box_shape;
    sf::Vector2f position;
    sf::Vector2f size;
    sf::Text popup_window_text;

    text_input input_text_box;
    bool has_valid_confirm_message = false;
    sf::Text confirm_button_text = sf::Text("Confirm", arial_font, 16);
    std::string on_confirm = "confirm:";

    std::vector<std::string> confirm_button_press_parameters, confirm_button_hover_parameters;

    popup_window(const sf::Vector2f &_position, const float &_rotation, const sf::Vector2f &_size, const uint8_t &_thickness, const std::string &_popup_text) : interface_object(_position, _rotation, _size, _thickness, sf::Color::White)
    {
        position = _position;
        size = _size;

        popup_window_shape = sf::RectangleShape();
        popup_window_shape.setPosition(_position);
        popup_window_shape.setRotation(_rotation);
        popup_window_shape.setSize(_size);
        popup_window_shape.setOutlineThickness(_thickness);

        popup_window_shape_shadow = sf::RectangleShape();
        popup_window_shape_shadow.setPosition(_position - sf::Vector2f(3, -3));
        popup_window_shape_shadow.setFillColor(sf::Color(200, 200, 200, 200));
        popup_window_shape_shadow.setRotation(_rotation);
        popup_window_shape_shadow.setSize(_size);
        popup_window_shape_shadow.setOutlineThickness(_thickness);

        popup_input_box_shape = sf::RectangleShape();
        popup_input_box_shape.setPosition(_position + sf::Vector2f(5, 59));
        popup_input_box_shape.setFillColor(sf::Color(100, 100, 100, 200));
        popup_input_box_shape.setRotation(_rotation);
        popup_input_box_shape.setSize(_size - sf::Vector2f(15, _size.y / 0.85));
        popup_input_box_shape.setOutlineThickness(_thickness);

        popup_window_text.setPosition(_position + sf::Vector2f(11, 10));
        popup_window_text.setFont(arial_font);
        popup_window_text.setCharacterSize(14);
        popup_window_text.setFillColor(sf::Color::Black);
        popup_window_text.setString(_popup_text);

        input_text_box.text_input_string.setFillColor(sf::Color::Black);
        input_text_box.text_input_string.setPosition(_position + sf::Vector2f(6, 38.5f));
        input_text_box.text_input_string.setCharacterSize(15);

        auto confirm_button_press_function = [this](std::vector<std::string> _parameters)
        {
            if (popup_window_string.size() == 5 && draw_popup_window)
            {
                client::instance()->send_message_to_server("003", popup_window_string);
                draw_popup_window = false;
            }
            else if (popup_window_open_string.size() == 6 && draw_popup_open_window)
            {
                client::instance()->send_message_to_server("011", popup_window_open_string);
                draw_popup_open_window = false;
            }
            else if(popup_window_delete_string.size() == 6 && draw_popup_delete_window)
            {
                client::instance()->send_message_to_server("010", popup_window_delete_string);
                draw_popup_delete_window = false;
            }
        };

        auto confirm_button_hover_function = [this](std::vector<std::string> _parameters)
        {
            // printf("On hover confirm button\n");
        };
        button confirm_button(position + sf::Vector2f(size.x / 2 - 80 / 2, size.y / 2 + 30), 0, sf::Vector2f(80, 20), 0, "Confirm", 16, confirm_button_press_function, confirm_button_press_parameters, confirm_button_hover_function, confirm_button_hover_parameters);
        confirm_button.button_shape.setFillColor(sf::Color(73, 214, 73, 230));
        confirm_button_text.setPosition(position + sf::Vector2f(size.x / 2 - 80 / 2 + 11, size.y / 2 + 30));
        confirm_button_text.setFillColor(sf::Color::Black);
        this->popup_window_buttons.push_back(confirm_button);
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        target.draw(popup_window_shape_shadow);
        target.draw(popup_window_shape);
        for (auto i = popup_window_buttons.begin(); i != popup_window_buttons.end(); i++)
        {
            target.draw(i->button_shape);
        }
        target.draw(popup_input_box_shape);
        target.draw(popup_window_text);
        target.draw(input_text_box.text_input_string);
        target.draw(confirm_button_text);
    }
};

void SFML_logic()
{
    if (!arial_font.loadFromFile("arial.ttf"))
    {
        printf("Couldn't load font.\n");
    }

    // text_input_object.text_input_string.setPosition(text_input_object.position + sf::Vector2f(0, 100));
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Collaborative Notepad - Proiect retele.", sf::Style::Close);
    window.setFramerateLimit(144);
    window.setSize(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT));

    menu_interface menu_interface_object;
    static_interface_class static_objects;

    status_text.setPosition(sf::Vector2f(WINDOW_WIDTH - 150, -3));
    status_text.setFont(arial_font);
    status_text.setCharacterSize(20);
    status_text.setFillColor(sf::Color::Red);
    status_text.setString("No joined room.");

    popup_window popup_window_object(sf::Vector2f(WINDOW_WIDTH / 2 - WINDOW_WIDTH / 8, WINDOW_HEIGHT / 2 - WINDOW_HEIGHT / 4), 0.0, sf::Vector2f(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4), 0, "Enter room code:");
    popup_window popup_window_open_object(sf::Vector2f(WINDOW_WIDTH / 2 - WINDOW_WIDTH / 8, WINDOW_HEIGHT / 2 - WINDOW_HEIGHT / 4), 0.0, sf::Vector2f(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4), 0, "Open: Enter file code:");
    popup_window popup_window_delete_object(sf::Vector2f(WINDOW_WIDTH / 2 - WINDOW_WIDTH / 8, WINDOW_HEIGHT / 2 - WINDOW_HEIGHT / 4), 0.0, sf::Vector2f(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4), 0, "Delete: Enter file code:");


    static_objects.static_object_init();

    uint16_t max_popup_time = 5;

    sf::RectangleShape text_input_cursor(sf::Vector2f(3, 20));
    sf::RectangleShape client_input_cursor(sf::Vector2f(3, 20));
    text_input_cursor.setFillColor(sf::Color::Blue);
    client_input_cursor.setFillColor(sf::Color::Red);
    bool show_client_input_cursor = true;

    bool has_updated_notepad = false;

    while (window.isOpen())
    {
        sf::Vector2i cursor_position = sf::Mouse::getPosition(window);
        sf::Event event;

        switch (current_status)
        {
        case current_state_enum::NOT_CONNECTED:
        {
            if (status_text.getString().toAnsiString().compare("Not connected.") != 0)
                status_text.setString("Not connected.");

            status_text.setPosition(WINDOW_WIDTH - status_text.getLocalBounds().width - 5, -3);
        }
        break;

        case current_state_enum::CONNECTED_TO_SERVER:
        {
            if (status_text.getString().toAnsiString().compare("No joined room!") != 0)
                status_text.setString("No joined room!");

            status_text.setFillColor(sf::Color(255, 165, 0, 255));
            status_text.setPosition(WINDOW_WIDTH - status_text.getLocalBounds().width - 5, -3);
        }
        break;

        case current_state_enum::CONNECTED_TO_ROOM:
        {
            std::string _room_to_be_set("Connected to room: ");
            _room_to_be_set += current_room_id;
            status_text.setString(_room_to_be_set);
            status_text.setFillColor(sf::Color::Green);
            status_text.setPosition(WINDOW_WIDTH - status_text.getLocalBounds().width - 5, -3);
        }
        break;

        case current_state_enum::CANT_FIND_ROOM:
        {
            std::string _room_to_be_set("Room doesn't exist.");
            status_text.setString(_room_to_be_set);
            status_text.setFillColor(sf::Color(255, 165, 0, 255));
            status_text.setPosition(WINDOW_WIDTH - status_text.getLocalBounds().width - 5, -3);
        }
        break;
        }

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if(text_input_cursor_position > text_input_object.client_text_input_string.getSize())
            {
                text_input_cursor_position = text_input_object.client_text_input_string.getSize();
            }
            
            if (draw_popup_window)
            {
                if (event.type == sf::Event::TextEntered)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace))
                    {
                        if (popup_window_string.size() > 0)
                        {
                            popup_window_string.pop_back();
                            popup_window_object.input_text_box.text_input_string.setString(popup_window_string);
                        }
                    }
                    else if (popup_window_string.size() < 5)
                    {
                        popup_window_string += toupper(event.text.unicode);
                        popup_window_object.input_text_box.text_input_string.setString(popup_window_string);
                    }
                }
            }
            else if (draw_popup_open_window)
            {
                if (event.type == sf::Event::TextEntered)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace))
                    {
                        if (popup_window_open_string.size() > 0)
                        {
                            popup_window_open_string.pop_back();
                            popup_window_open_object.input_text_box.text_input_string.setString(popup_window_open_string);
                        }
                    }
                    else if (popup_window_open_string.size() < 6)
                    {
                        popup_window_open_string += toupper(event.text.unicode);
                        popup_window_open_object.input_text_box.text_input_string.setString(popup_window_open_string);
                    }
                }
            }
            else if (draw_popup_delete_window)
            {
                if (event.type == sf::Event::TextEntered)
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace))
                    {
                        if (popup_window_delete_string.size() > 0)
                        {
                            popup_window_delete_string.pop_back();
                            popup_window_delete_object.input_text_box.text_input_string.setString(popup_window_delete_string);
                        }
                    }
                    else if (popup_window_delete_string.size() < 6)
                    {
                        popup_window_delete_string += toupper(event.text.unicode);
                        popup_window_delete_object.input_text_box.text_input_string.setString(popup_window_delete_string);
                    }
                }
            }
            else
            {
                if (event.type == sf::Event::TextEntered)
                {
                    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
                    {
                        text_input_object.client_text_input_string.insert(text_input_cursor_position, event.text.unicode);
                        text_input_object.text_input_string.setString(text_input_object.client_text_input_string);
                        text_input_cursor_position += 1;
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace))
                    {
                        if (text_input_object.client_text_input_string.getSize() > 0 && text_input_cursor_position > 0)
                        {
                            text_input_object.client_text_input_string.erase(text_input_cursor_position - 1);
                            text_input_object.text_input_string.setString(text_input_object.client_text_input_string);
                            text_input_cursor_position -= 1;
                        }
                    }
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
                    {
                        text_input_object.client_text_input_string.insert(text_input_cursor_position, '\n');
                        text_input_object.text_input_string.setString(text_input_object.client_text_input_string);
                        text_input_cursor_position += 1;
                    }
                    has_updated_notepad = true;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                {
                    if (text_input_cursor_position > 0)
                        text_input_cursor_position -= 1;

                    has_updated_notepad = true;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                {
                    if (text_input_cursor_position < text_input_object.client_text_input_string.getSize())
                        text_input_cursor_position += 1;

                    has_updated_notepad = true;
                }
            }
        }
        window.clear();
        text_input_cursor.setPosition(text_input_object.text_input_string.findCharacterPos(text_input_cursor_position) + sf::Vector2f(0, 5));
        client_input_cursor.setPosition(text_input_object.text_input_string.findCharacterPos(client_input_cursor_position) + sf::Vector2f(0, 5));
        // printf("%d, x: %f, y: %f\n", text_input_cursor_position, text_input_object.text_input_string.findCharacterPos(text_input_cursor_position).x, text_input_object.text_input_string.findCharacterPos(text_input_cursor_position).y);

        // process all the buttons
        for (auto i = menu_interface_object.menu_buttons.begin(); i != menu_interface_object.menu_buttons.end(); i++)
        {
            // printf("mouse coods: x: %d, y: %d box:collider: x: %f, y: %f, size: x:%f, y:%f\n", cursor_position.x, cursor_position.y, i->button_shape.getPosition().x,
            // i->button_shape.getPosition().y, i->button_shape.getSize().x, i->button_shape.getSize().y);
            // check if mouse is over it
            if (
                cursor_position.x >= i->button_shape.getPosition().x &&
                cursor_position.x <= i->button_shape.getSize().x + i->button_shape.getPosition().x &&
                cursor_position.y >= i->button_shape.getPosition().y &&
                cursor_position.y <= i->button_shape.getSize().y + i->button_shape.getPosition().y)
            {
                i->on_hover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    i->on_press();
                }
                else
                {
                    i->hasBeenPressed = false;
                }
            }
            else
            {
                i->on_unhover();
            }
        }

        for (auto i = menu_interface_object.room_settings_interface.room_buttons.begin(); i != menu_interface_object.room_settings_interface.room_buttons.end(); i++)
        {
            // printf("mouse coods: x: %d, y: %d box:collider: x: %f, y: %f, size: x:%f, y:%f\n", cursor_position.x, cursor_position.y, i->button_shape.getPosition().x,
            // i->button_shape.getPosition().y, i->button_shape.getSize().x, i->button_shape.getSize().y);
            // check if mouse is over it
            if (
                cursor_position.x >= i->button_shape.getPosition().x &&
                cursor_position.x <= i->button_shape.getSize().x + i->button_shape.getPosition().x &&
                cursor_position.y >= i->button_shape.getPosition().y &&
                cursor_position.y <= i->button_shape.getSize().y + i->button_shape.getPosition().y)
            {
                i->on_hover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    i->on_press();
                }
                else
                {
                    i->hasBeenPressed = false;
                }
            }
            else
            {
                i->isHovered = false;
            }
        }

        for (auto i = menu_interface_object.file_settings_interface.file_buttons.begin(); i != menu_interface_object.file_settings_interface.file_buttons.end(); i++)
        {
            if (
                cursor_position.x >= i->button_shape.getPosition().x &&
                cursor_position.x <= i->button_shape.getSize().x + i->button_shape.getPosition().x &&
                cursor_position.y >= i->button_shape.getPosition().y &&
                cursor_position.y <= i->button_shape.getSize().y + i->button_shape.getPosition().y)
            {
                i->on_hover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    i->on_press();
                }
                else
                {
                    i->hasBeenPressed = false;
                }
            }
            else
            {
                i->isHovered = false;
            }
        }

        for (auto i = popup_window_object.popup_window_buttons.begin(); i != popup_window_object.popup_window_buttons.end(); i++)
        {
            if (
                cursor_position.x >= i->button_shape.getPosition().x &&
                cursor_position.x <= i->button_shape.getSize().x + i->button_shape.getPosition().x &&
                cursor_position.y >= i->button_shape.getPosition().y &&
                cursor_position.y <= i->button_shape.getSize().y + i->button_shape.getPosition().y)
            {
                i->on_hover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    i->on_press();
                }
                else
                {
                    i->hasBeenPressed = false;
                }
            }
            else
            {
                i->isHovered = false;
            }
        }

        for (auto i = popup_window_open_object.popup_window_buttons.begin(); i != popup_window_open_object.popup_window_buttons.end(); i++)
        {
            if (
                cursor_position.x >= i->button_shape.getPosition().x &&
                cursor_position.x <= i->button_shape.getSize().x + i->button_shape.getPosition().x &&
                cursor_position.y >= i->button_shape.getPosition().y &&
                cursor_position.y <= i->button_shape.getSize().y + i->button_shape.getPosition().y)
            {
                i->on_hover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    i->on_press();
                }
                else
                {
                    i->hasBeenPressed = false;
                }
            }
            else
            {
                i->isHovered = false;
            }
        }

        for (auto i = popup_window_delete_object.popup_window_buttons.begin(); i != popup_window_delete_object.popup_window_buttons.end(); i++)
        {
            if (
                cursor_position.x >= i->button_shape.getPosition().x &&
                cursor_position.x <= i->button_shape.getSize().x + i->button_shape.getPosition().x &&
                cursor_position.y >= i->button_shape.getPosition().y &&
                cursor_position.y <= i->button_shape.getSize().y + i->button_shape.getPosition().y)
            {
                i->on_hover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    i->on_press();
                }
                else
                {
                    i->hasBeenPressed = false;
                }
            }
            else
            {
                i->isHovered = false;
            }
        }

        window.draw(static_objects);

        window.draw(text_input_object);

        if (draw_popup_window)
            window.draw(popup_window_object);

        if (draw_popup_open_window)
            window.draw(popup_window_open_object);

        if(draw_popup_delete_window)
            window.draw(popup_window_delete_object);

        window.draw(client_input_cursor);
        window.draw(text_input_cursor);

        window.draw(status_text);

        window.draw(menu_interface_object);

        if (elasped_time.getElapsedTime().asSeconds() < max_popup_time && popup_alert_object.popup_alert_text.getString() != "undefined")
        {
            window.draw(popup_alert_object);
        }

        window.display();

        char message_to_be_sent[1024];

        if (has_updated_notepad && current_status == current_state_enum::CONNECTED_TO_ROOM)
        {
            client::instance()->send_message_to_server("005", text_input_object.client_text_input_string.toAnsiString().c_str());
            client::instance()->send_message_to_server("006", std::to_string(text_input_cursor_position).c_str());
            has_updated_notepad = false;
        }
    }
}


client *client::client_instance = nullptr;

int main(int argc, char *argv[])
{
    int TEMPORARY_FIX = atoi(argv[1]);
    std::thread sfml_thread(&SFML_logic);
    sfml_thread.detach();

    std::vector<command *> commands;
    CommandInitialization(commands);
    printf("Commands initialized!\n");

    client *client_object = client::instance(AF_INET, SOCK_STREAM, 0, "127.0.0.1", TEMPORARY_FIX);
    // client client_object(AF_INET, SOCK_STREAM, 0, "127.0.0.1", 25561);
    client_object->try_connect();
    std::thread listen_to_server(&client::listening_to_server_thread, 0);
    listen_to_server.detach();
    
    char input_buffer[1024];
    while (true)
    {
        fgets(input_buffer, sizeof(input_buffer), stdin);
        input_buffer[strcspn(input_buffer, "\n")] = '\0';
        std::string b_string = input_buffer;
        std::vector<std::string> input_params;

        std::transform(b_string.begin(), b_string.end(), b_string.begin(), [](unsigned char c)
                       { return std::tolower(c); });

        bool is_command_valid = false;

        for (std::vector<command *>::iterator i = commands.begin(); i != commands.end(); i++)
        {
            std::cout << (*i)->get_command_name() << '\n';
            if (trim_str(b_string.substr(0, (b_string.find_first_of(' ')))).compare(trim_str((*i)->get_command_name())) == 0 /* && std::count(b_string.begin(), b_string.end(), ' ') - 1 == i->get_command_args_count() */)
            {
                input_params = split_str(b_string);
                input_params.push_back(std::to_string(client_object->get_client_descriptor()));
                (*i)->action(input_params);
                is_command_valid = true;
                break;
            }
        }

        if (is_command_valid == false)
        {
            printf("Error: Invalid command.\n");
        }
    }

    printf("OK\n");
}

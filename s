auto leave_room_press_function = [this](std::vector<std::string> _parameters) 
            {
                draw_popup_window = true;
            };

            auto leave_room_hover_function = [this](std::vector<std::string> _parameters)
            {
                printf("On hover leave_room button\n");
            };
            button leave_room_button(sf::Vector2f(160, 40), 0, sf::Vector2f(80, 20), 0, "Join", 16, leave_room_press_function, leave_room_press_parameters, leave_room_hover_function, leave_room_hover_parameters);
            room_buttons.push_back(leave_room_button);
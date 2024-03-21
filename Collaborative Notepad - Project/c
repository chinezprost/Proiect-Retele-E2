#!/bin/bash

random_port=$(cat random_port.txt)

g++ -c client.cpp
g++ client.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system
./sfml-app $random_port

exit 0

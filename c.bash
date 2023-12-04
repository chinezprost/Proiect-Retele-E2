#!/bin/bash

g++ -c client.cpp
g++ client.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system

exit 0

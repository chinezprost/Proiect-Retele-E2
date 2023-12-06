#!/bin/bash

g++ -Wall server.cpp -o server
lsof -ti tcp:25565 | xargs kill
./server

exit 0

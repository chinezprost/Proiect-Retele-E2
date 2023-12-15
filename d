#!/bin/bash
random_port=$((RANDOM % (8000 - 2000 + 1) + 2000))
> random_port.txt
echo $random_port > random_port.txt

g++ -Wall main_app.cpp server_client_room.cpp server_worker.cpp server_client_thread.cpp server.cpp -o server
./server $random_port

exit 0

#!/bin/bash

#server
cd ./src
g++ server.cpp -o server.exe
./server.exe


#client
cd ./src
g++ client.cpp -o client.exe
./client.exe
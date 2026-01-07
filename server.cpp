#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

    // create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0)
    if(server_fd < 0){
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Define server address
    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_add.sin_family = AF_INET;
    
}
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <assert.h>

#define PORT 5000

static void msg(const char* msg){
    fprintf(stderr, "%s\n", msg);
}

static void error_handler(const char* msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char* buffer, size_t n){
    while(n){
        ssize_t rv = read(fd, buffer, n);
        if(rv <= 0) return -1;

        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buffer += rv;
    }

    return 0;
}

static int32_t write_all(int fd, const char* buffer, size_t n){
    while(n){
        ssize_t rv = write(fd, buffer, n);
        if(rv <= 0) return -1;

        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buffer += rv;
    }

    return 0;
}

static int32_t one_request(int connfd){
    char read_buffer[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(connfd, read_buffer, 4);
    if(err){
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, read_buffer, 4);
    if(len > k_max_msg){
        msg("message length too long");
        return -1;
    }

    err = read_full(connfd, &read_buffer[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    fprintf(stderr, "Client: %.*s\n", len, &read_buffer[4]);

    const char reply[] = "what is happening";
    char write_buffer[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(write_buffer, &len, 4);
    memcpy(&write_buffer[4], reply, len);

    int32_t rv = write_all(connfd, write_buffer, 4 + len);

    return rv;
}


int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0){
        error_handler("socket()");
    }

    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    if(connect(fd, (sockaddr*)& client_addr, sizeof(client_addr)) < 0){
        error_handler("connect()");
    }

    char write_buffer[] = "hello server";
    write(fd, write_buffer, sizeof(write_buffer));

    char read_buffer[1024];
    int n = read(fd, read_buffer, sizeof(read_buffer) - 1);
    if(n < 0){
        error_handler("read()");
    }

    read_buffer[n] = '\0';

    std::cout << "Server: " << read_buffer << std::endl;
    close(fd);

}
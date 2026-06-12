#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <variant>

const uint32_t k_max_msg = 4096; 

enum class Type{
    NIL = 0,
    ERR = 1,
    INT = 2,
    STR = 3,
    DBL = 4,
    ARR = 5,
};

struct Response{
    Type type;
    std::variant<std::monostate, int, std::string, double> value;
};

struct Buffer{
    uint8_t write_buf[4 + k_max_msg];
    uint8_t read_buf[4 + k_max_msg];
};

class KVClient {

public:
    KVClient(std::string host, std::string port) 
    : host(host), port(port){}

    ~KVClient(){
        freeaddrinfo(addr_info);
        
        if(fd >= 0) close(fd);
    }

    bool open(){
        
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(client_fd < 0) return false;

        this->fd = client_fd;

        struct addrinfo addr{};
        addr.ai_family = AF_INET;
        addr.ai_socktype = SOCK_STREAM;

        int rv = getaddrinfo(host.c_str(), port.c_str(), &addr, &addr_info);
        if(rv < 0) return false;

        if(connect(client_fd, addr_info->ai_addr, addr_info->ai_addrlen) < 0) 
            return false;

        return true;
    }

    bool send(const std::vector<std::string>& cmd){
        uint32_t len = 4;
        for(const std::string& s: cmd) len += 4 + s.size();

        if(len > k_max_msg) return false;

        memcpy(&buf.write_buf[0], &len, 4);
        uint32_t nstr = cmd.size();
        memcpy(&buf.write_buf[4], &nstr, 4);
        uint32_t cur = 8;
        for(const std::string& s: cmd){
            uint32_t n = (uint32_t)s.size();
            memcpy(&buf.write_buf[cur], &n, 4);
            memcpy(&buf.write_buf[cur + 4], s.data(), n);
            cur += 4 + n;
        }

        if(write_all(buf.write_buf, 4 + len) < 0) return false;

        return true;
    }

    bool receive(){
        if(read_full(buf.read_buf, 4) < 0) return false;

        uint32_t len = 0;
        memcpy(&len, &buf.read_buf[0], 4);

        if(len > k_max_msg) return false;

        if(read_full(&buf.read_buf[4], len) < 0) return false;

        return true;
    }

private:

    int32_t read_full(uint8_t* read_buffer, size_t n){
        while(n > 0){
            ssize_t rv = read(fd, read_buffer, n);
            if(rv <= 0){
                return -1;
            }
            n -= (size_t)rv;
            read_buffer += rv;
        }

        return 0;
    }

    int32_t write_all(const uint8_t* write_buffer, size_t n){
        while(n > 0){
            ssize_t rv = write(fd, write_buffer, n);
            if(rv <= 0){
                return -1;
            }
            n -= (size_t)rv;
            write_buffer += rv;
        }

        return 0;
    }


    std::string host;
    std::string port;
    int fd = -1;
    struct Buffer buf;
    struct addrinfo* addr_info = nullptr;
};

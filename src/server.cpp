#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <vector>
#include <map>
#include "list.h"
#include "zset.h"
#include "hashtable.h"
#include "utils.h"
// #include "log.h"

#define PORT 5000
#define MAX_EVENTS 64


static void msg(const char* msg){
    fprintf(stderr, "%s\n", msg);
}

static void msg_errno(const char* msg){
    fprintf(stderr, "[error:%d] %s\n", errno, msg);
}

static void die(const char* msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void fd_set_nb(int fd){
    errno = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if(errno){
        die("fcntl() get");
    }

    flags |= O_NONBLOCK;

    errno = 0;
    fcntl(fd, F_SETFL, flags);
    if(errno){
        die("fcntl() set");
    }
}

const size_t k_max_msg = 32 << 20;

typedef std::vector<uint8_t> Buffer;


struct Conn {
    int fd = -1;
    bool want_read = false;
    bool want_write = false;
    bool want_close = false;

    Buffer incoming;
    Buffer outgoing;

    uint64_t last_active_ms = 0;
    List idle_node;
};

enum {
    ERR_UNKNOWN = 1,
    ERR_TOO_BIG = 2,
    ERR_BAD_ARG = 3,
    ERR_BAD_TYP = 4
};

enum {
    TAG_NIL = 0,
    TAG_ERR = 1,
    TAG_INT = 2,
    TAG_STRING = 3,
    TAG_DOUBLE = 4,
    TAG_ARRAY = 5,
};

static void buf_append(Buffer &buf, const uint8_t* data, size_t len){
    buf.insert(buf.end(), data, data + len); 
}

static void buf_consume(Buffer &buf, size_t n){
    buf.erase(buf.begin(), buf.begin() + n); 
}

static struct {
    HMap db;

    std::vector<Conn*> fd2conn;

    List idle_list;

} g_data;

enum {
    T_INIT = 0,
    T_STR = 1,
    T_ZSET = 2,
};

struct Entry{
    struct HNode node;
    std::string key;
    
    uint32_t type = 0;

    union {
        std::string str;
        ZSet zset;
    };

    explicit Entry(){}

    explicit Entry(uint32_t type): type(type){
        if(type == T_STR){
            new (&str) std::string;
        }
        else if(type == T_ZSET){
            new (&zset) ZSet;
        }
    }

    ~Entry(){
        if(type == T_STR){
            str.~basic_string();
        }
        else if(type == T_ZSET){
            zset_clear(&zset);
        }
    }

};

static bool entry_eq(HNode* lhs, HNode* rhs){
    struct Entry* l = container_of(lhs, struct Entry, node);
    struct Entry* r = container_of(rhs, struct Entry, node);
    return l->key == r->key;
}

static void buf_append_u8(Buffer& buf, uint8_t data){
    buf.push_back(data);
}

static void buf_append_u32(Buffer& buf, uint32_t data){
    buf_append(buf, (const uint8_t*)&data, 4);
}

static void buf_append_i64(Buffer& buf, int64_t data){
    buf_append(buf, (const uint8_t*)&data, 8);
}

static void buf_append_dbl(Buffer& buf, double data){
    buf_append(buf, (const uint8_t*)& data, 8);
}

static void out_nil(Buffer& out){
    buf_append_u8(out, TAG_NIL);
}

static void out_err(Buffer& out, uint32_t code, const std::string& msg){
    buf_append_u8(out, TAG_ERR);
    buf_append_u32(out, code);
    buf_append_u32(out, (uint32_t)msg.size());
    buf_append(out, (const uint8_t*) msg.data(), msg.size());
}

static void out_str(Buffer& out, const char* data, size_t size){
    buf_append_u8(out, TAG_STRING);
    buf_append_u32(out, (uint32_t)size);
    buf_append(out, (const uint8_t*)data, size);
}

static void out_int(Buffer& out, int64_t data){
    buf_append_u8(out, TAG_INT);
    buf_append_i64(out, data);
}

static void out_dbl(Buffer& out, double data){
    buf_append_u8(out, TAG_DOUBLE);
    buf_append_dbl(out, data);
}

static void out_arr(Buffer& out, uint32_t n){
    buf_append_u8(out, TAG_ARRAY);
    buf_append_u32(out, n);
}

static void get(std::vector<std::string>& cmd, Buffer& out){
    std::string val = "";
    if(!g_data.db.newer.table) {
        out_nil(out);
        return;
    }
    struct Entry key{T_STR};
    key.key.swap(cmd[1]);
    key.node.hash = str_hash((uint8_t*)key.key.data(), key.key.size());
    HNode* node = hm_lookup(&g_data.db, &key.node, &entry_eq);
    if(node){
        val = container_of(node, struct Entry, node)->str;
    }
    else {
        out_nil(out);
    }

    assert(val.size() <= k_max_msg);
    out_str(out, (const char*)val.data(), val.size());
}

static void set(std::vector<std::string>& cmd, Buffer& out){
    struct Entry key{T_STR};
    key.key.swap(cmd[1]);
    key.node.hash = str_hash((uint8_t*)key.key.data(), key.key.size());
    HNode* node = hm_lookup(&g_data.db, &key.node, &entry_eq);
    if(node){
        container_of(node, struct Entry, node)->str.swap(cmd[2]);
    }
    else{
       struct Entry* ent = new Entry(T_STR);
       ent->key.swap(key.key);
       ent->str.swap(cmd[2]);
       ent->node.hash = key.node.hash;
       hm_insert(&g_data.db, &ent->node);
    }   

    std::string response = "Operation successful.";
    out_str(out, (const char*)response.data(), response.size());
}

static void del(std::vector<std::string>& cmd, Buffer& out){
    std::string response = "Operation successful.";
    if(!g_data.db.newer.table) {
        response = "Empty DB.";
        out_str(out, (const char*)response.data(), response.size());
        return;
    }
    struct Entry key{T_STR};
    key.key.swap(cmd[1]);
    key.node.hash = str_hash((uint8_t*)key.key.data(), key.key.size());
    HNode* node = hm_lookup(&g_data.db, &key.node, &entry_eq);
    if(node){
        delete container_of(node, struct Entry, node);
    }
    else response = "Value Not Found.";

    out_str(out, (const char*)response.data(), response.size());

}

static void flush(Buffer& out){
    hm_clear(&g_data.db);
    std::string response = "Operation successful.";
    out_str(out, (const char*)response.data(), response.size());
}

static void exists(std::vector<std::string>& cmd, Buffer& out){
    struct Entry key{T_STR};
    key.key.swap(cmd[1]);
    key.node.hash = str_hash((uint8_t*)key.key.data(), key.key.size());
    HNode* node = hm_lookup(&g_data.db, &key.node, &entry_eq);
    uint8_t exists = node ? 1 : 0;

    out_int(out, (int64_t)exists);
}

static bool str2dbl(const std::string& str, double& out){
    char* end = nullptr;
    out = strtod(str.c_str(), &end);
    return end == str.c_str() + str.size() && !isnan(out);
}

static bool str2int(const std::string& str, int64_t& out){
    char* end = nullptr;
    out = strtoll(str.c_str(), &end, 10);
    return end == str.c_str() + str.size() && !isnan(out);
}

static void zadd(std::vector<std::string>& cmd, Buffer& out){
    double score = 0;
    if(!str2dbl(cmd[2], score)){
        return out_err(out, ERR_BAD_ARG, "arg: expect float");
    }

    struct Entry lookup_key;
    lookup_key.key.swap(cmd[1]);
    lookup_key.node.hash = str_hash((const uint8_t*)lookup_key.key.data(), lookup_key.key.size());
    HNode* found = hm_lookup(&g_data.db, &lookup_key.node, &entry_eq);

    struct Entry key;

    if(!found){
        key.type = T_ZSET;
        key.key.swap(lookup_key.key);
        key.node.hash = lookup_key.node.hash;
    }
    else {
        struct Entry* ent = container_of(found, struct Entry, node);
        if(ent->type != T_ZSET) return out_err(out, ERR_BAD_ARG, "type: expect zset");
        else return;
    }

    const std::string& name = cmd[3];
    bool added = zset_insert(&key.zset, name.data(), name.size(), score);
    out_int(out, (int64_t)added);
}

static ZSet* get_zset(std::string key){
    struct Entry lookup_key;
    lookup_key.key = key;
    lookup_key.node.hash = str_hash((const uint8_t*)lookup_key.key.data(), lookup_key.key.size());
    HNode* found = hm_lookup(&g_data.db, &lookup_key.node, &entry_eq);
    if(!found) return nullptr;

    struct Entry* ent = container_of(found, struct Entry, node);
    return ent->type ? &ent->zset : nullptr;
}

static void zrem(std::vector<std::string>& cmd, Buffer& out){
    ZSet* zset = get_zset(cmd[1]);
    if(!zset){
        return out_err(out, ERR_BAD_ARG, "arg: expect zset");
    }

    const std::string& name = cmd[2];
    ZNode* znode = zset_lookup(zset, name.data(), name.size());
    if(znode){
        zset_delete(zset, znode);
    }

    return out_int(out, znode ? 1 : 0);
}

static void zscore(std::vector<std::string>& cmd, Buffer& out){
    ZSet* zset = get_zset(cmd[1]);
    if(!zset){
        return out_err(out, ERR_BAD_ARG, "arg: expect zset");
    }

    const std::string& name = cmd[2];
    ZNode* znode = zset_lookup(zset, name.data(), name.size());

    return znode ? out_dbl(out, znode->score) : out_nil(out);
}

static void do_request(std::vector<std::string>& cmd, Buffer& out){

    // LOG_INFO("db state: newer.table=%p newer.mask=%zu newer.size=%zu",
    //      (void*)g_data.db.newer.table,
    //      g_data.db.newer.mask,
    //      g_data.db.newer.size);
    
    if(cmd.size() == 2 && cmd[0] == "GET"){
        // LOG_INFO("cmd.size=%zu cmd[0]=%s cmd[1]=%s",
        //  cmd.size(),
        //  cmd[0].c_str(),
        //  cmd[1].c_str());
        get(cmd, out);
    }

    else if(cmd.size() == 3 && cmd[0] == "SET"){
        // LOG_INFO("cmd.size=%zu cmd[0]=%s cmd[1]=%s cmd[2]=%s",
        //  cmd.size(),
        //  cmd[0].c_str(),
        //  cmd[1].c_str(),
        // cmd[2].c_str());
        set(cmd, out);
    }

    else if(cmd.size() == 2 && cmd[0] == "EXISTS"){
        // LOG_INFO("cmd.size=%zu cmd[0]=%s cmd[1]=%s",
        //  cmd.size(),
        //  cmd[0].c_str(),
        //  cmd[1].c_str());
        exists(cmd, out);
    }

    else if(cmd.size() == 2 && cmd[0] == "DEL"){
        // LOG_INFO("cmd.size=%zu cmd[0]=%s cmd[1]=%s",
        //  cmd.size(),
        //  cmd[0].c_str(),
        // cmd[1].c_str());
        del(cmd, out);
    }

    else if(cmd.size() == 1 && cmd[0] == "FLUSH"){
        flush(out);
    }

    else if(cmd.size() == 4 && cmd[0] == "ZADD"){
        zadd(cmd, out);
    }

    else if(cmd.size() == 3 && cmd[0] == "ZREM"){
        zrem(cmd, out);
    }

    else if(cmd.size() == 3 && cmd[0] == "ZSCORE"){
        zscore(cmd, out);
    }
    
    else{
        out_err(out, ERR_UNKNOWN, "Unknown Command.");
    }

}

/*
Protocol:

Request:
[4B total_len]
[4B nstr]
repeat nstr:
    [4B len][bytes]

Response:
[4B total_len]
[1B tag][payload...]

*/

static bool read_u32(const uint8_t*& cur, const uint8_t*& end, uint32_t* out){
    if(cur + 4 > end) return false;

    memcpy(out, cur, 4);
    cur += 4;
    return true;
}

static bool read_str(const uint8_t*& cur, const uint8_t*& end, size_t n, std::string& out){
    if(cur + n > end) return false;

    out.assign(cur, cur + n);
    cur += n;
    return true;
}

static int32_t parse_req(const uint8_t*& data, size_t size, std::vector<std::string>& out){
    const uint8_t* end = data + size;

    uint32_t nstr = 0;
    if(!read_u32(data, end, &nstr)) return -1;

    while(out.size() < nstr){
        uint32_t len = 0;
        if(!read_u32(data, end, &len)) return -1;

        out.push_back(std::string());
        if(!read_str(data, end, len, out.back())) return -1;
    }

    if(data != end) return -1;

    return 0;
}

// void log_payload(const uint8_t* data, size_t len) {
//     printf("payload (%zu bytes): ", len);
//     for (size_t i = 0; i < len; i++) {
//         printf("%02x ", data[i]);
//     }
//     printf("\n");
// }

static void resp_header_alloc(Buffer& out, size_t* header){
    *header = out.size();
    buf_append_u32(out, 0);
}

static size_t resp_size(Buffer& out, size_t header){
    return out.size() - header - 4;
}

static void resp_header_assign(Buffer& out, size_t header){
    size_t msg_size = resp_size(out, header);
    if(msg_size > k_max_msg){
        printf("Response is too big.");
        out.resize(header + 4);
        out_err(out, ERR_TOO_BIG, "Response is too big.");
        msg_size = resp_size(out, header);
    }

    uint32_t len = (uint32_t)msg_size;
    printf("msg_size = [%d]\n", len);
    memcpy(&out[header], &len, 4);
}

static bool try_one_request(Conn* conn){
    if(conn->incoming.size() < 4) return false;

    uint32_t total_len = 0;
    memcpy(&total_len, conn->incoming.data(), 4);
    if(total_len > k_max_msg){
        msg("msg too long");
        conn->want_close = true;
        return false;
    }

    if(4 + total_len > conn->incoming.size()) return false;

    const uint8_t* request = &conn->incoming[4];

    // log_payload(request, total_len);
    
    std::vector<std::string> cmd;
    if(parse_req(request, total_len, cmd) < 0){
        msg("bad request");
        conn->want_close = true;
        return false;
    }

    printf("parsed cmd: ");
    for (auto& s : cmd) {
        printf("[%s] ", s.c_str());
    }
    printf("\n");

    size_t header_pos = 0;
    resp_header_alloc(conn->outgoing, &header_pos);
    do_request(cmd, conn->outgoing);
    resp_header_assign(conn->outgoing, header_pos);

    buf_consume(conn->incoming, 4 + total_len);

    return true;
}


static Conn* handle_accept(int fd){
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr*) &client_addr, &addrlen);
    if(connfd < 0){
        return nullptr;
    }

    uint32_t ip = ntohl(client_addr.sin_addr.s_addr);
    uint32_t port = ntohs(client_addr.sin_port);
    // LOG_INFO("NEW CONNECTION [address: %u.%u.%u.%u:%u]\n",
    //     (ip >> 24) & 0xFF,
    //     (ip >> 16) & 0xFF,
    //     (ip >> 8)  & 0xFF,
    //     ip & 0xFF,
    //     port);

    fd_set_nb(connfd);

    Conn* conn = new Conn();
    conn->fd = connfd;
    conn->want_read = true;
    conn->last_active_ms = get_time_msec();
    list_insert(&g_data.idle_list, &conn->idle_node);
    return conn;

}

static void set_write(int epfd, Conn* conn){
    if(!conn->want_write && conn->outgoing.size() > 0){
        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
        ev.data.fd = conn->fd;

        if(epoll_ctl(epfd, EPOLL_CTL_MOD, conn->fd, &ev) < 0){
            die("epoll_ctl MOD");
        }

        conn->want_write = true;
    }

    else if(conn->want_write && conn->outgoing.size() == 0){
        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
        ev.data.fd = conn->fd;

        if(epoll_ctl(epfd, EPOLL_CTL_MOD, conn->fd, &ev) < 0){
            die("epoll_ctl MOD");
        }

        conn->want_read = true;
        conn->want_write = false;
    }
}

static void handle_write(Conn* conn, int epfd){
    assert(conn->outgoing.size() > 0);
    
    while(conn->outgoing.size() > 0){
        ssize_t rv = write(conn->fd, conn->outgoing.data(), conn->outgoing.size());
        if(rv > 0) buf_consume(conn->outgoing, (size_t)rv);

        else {
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;

            conn->want_close = true;
            return;
        }

    }
    set_write(epfd, conn);

}

static void handle_read(Conn* conn, int epfd){
    uint8_t buf[64 * 1024];
    while(true){
        ssize_t rv = read(conn->fd, buf, sizeof(buf));

        if(rv > 0){
            buf_append(conn->incoming, buf, (size_t)rv);

            while(!conn->want_close && try_one_request(conn)){}
        }

        else if(rv == 0){
            if(conn->incoming.size() == 0) msg("client closed");
            else msg("Unexpected EOF");
            conn->want_close = true;
            return;
        }

        else {
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            conn->want_close = true;
            return;
        }

    }

    if(conn->outgoing.size() > 0){
            set_write(epfd, conn);
    }

}

const uint64_t k_idle_timeout_ms = 5000;

static uint64_t get_time_msec(){
    struct time_spec ts{0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return uint64_t(ts.tv_sec) * 1000 + ts.tv_nsec / 1000 / 1000;
}

static int32_t next_timer_ms(){
    if(list_empty(&g_data.idle_list)) return -1;

    uint64_t now_ms = get_time_msec();
    Conn* conn = container_of(g_data.idle_list.next, Conn, idle_node);
    uint64_t next_ms = conn->last_active_ms + k_idle_timeout_ms;
    if(next_ms <= now_ms) return 0;
    return (int32_t)(next_ms - now_ms);
}


int main(int argc, char* argv[]){

    int port = PORT;

    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-p")){
            assert(i + 1 < argc);
            port = std::stoi(argv[i + 1]);
            break;
        }
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        die("socket()");
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int val = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){
        die("setsockopt()");
    }

    if(bind(server_fd, (sockaddr*)& server_addr, sizeof(server_addr)) < 0){
        die("bind()");
    }

    if(listen(server_fd, SOMAXCONN) < 0){
        die("listen()");
    }

    fd_set_nb(server_fd);

    printf("server running on port %d\n", port);

    int epoll_fd = epoll_create1(0);

    if(epoll_fd < 0){
        die("epoll_create1()");
    }

    epoll_event sev{};
    sev.events = EPOLLIN | EPOLLERR;
    sev.data.fd = server_fd;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &sev) < 0){
        die("epoll_ctl");
    }

    epoll_event events[MAX_EVENTS];
    std::vector<Conn*> fd2conn;

    while(true){
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if(n < 0){
            die("epoll_wait()");
        }

        for(int i = 0; i < n; i++){
            int fd = events[i].data.fd;

            if(fd == server_fd){
                Conn* conn;
                if(!(conn = handle_accept(fd))){
                    msg("connection failed.");
                    continue;
                }

                epoll_event cev{};
                cev.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
                cev.data.fd = conn->fd;
                if(fd2conn.size() <= (size_t)conn->fd){
                    fd2conn.resize(conn->fd + 1);
                }
                fd2conn[conn->fd] = conn;

                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn->fd, &cev) < 0){
                    msg("epoll_ctl()");
                    continue;
                }
            }
            else{

                Conn* conn = g_data.fd2conn[fd];
                conn->last_active_ms = get_time_msec();
                list_detach(&conn->idle_node);
                list_insert(&g_data.idle_list, &conn->idle_node);

                int e = events[i].events;

                if(e & EPOLLIN){
                    handle_read(fd2conn[fd], epoll_fd);
                }

                if(e & EPOLLOUT){
                    handle_write(fd2conn[fd], epoll_fd);
                }

                if((e & (EPOLLERR | EPOLLHUP)) || fd2conn[fd]->want_close){
                    Conn* conn = fd2conn[fd];
                    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->fd, nullptr) < 0) die("epoll_ctl");
                    close(conn->fd);
                    fd2conn[fd] = nullptr;
                    delete conn;
                }
            }
        }
        
    }
}
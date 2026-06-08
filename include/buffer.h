#include <vector>

class Buffer{
public:

    Buffer(uint32_t cap) : cap(cap){
        buffer.resize(cap);
    }

    void push(uint8_t val){
        buffer[tail] = val;
        tail = (tail + 1) % cap;

        if(count < cap) count++;
        else head = (head + 1) % cap;
    }

    void push(const uint8_t* data, size_t len){
        size_t n = 0;
        while(n <= len){
            buffer[tail] = data + n;
            tail = (tail + 1) % cap;

            if(count < cap) count++;
            else head = (head + 1) % cap;
            n++;
        }
    }

    size_t size(){
        return count;
    }

private:
    uint32_t head = 0;
    uint32_t tail = 0;
    uint32_t count = 0;
    uint32_t cap;
    std::vector<uint8_t> buffer;
}
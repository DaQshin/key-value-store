CXX := g++
MODE ?= debug
PORT ?= 5000
BUILD := build

CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude

SERVER_SRCS = src/server.cpp \
				src/storage/hashtable.cpp \
				src/storage/avl.cpp \
				src/storage/zset.cpp

COMMON_SRCS = src/logs/log.cpp

ifeq ($(MODE), debug)
    CXXFLAGS += -g -O0 -DLOG_LEVEL=0
else
    CXXFLAGS += -O2 -DLOG_LEVEL=1
endif

.PHONY: all clean run_server run_client tests

all: $(BUILD)/client $(BUILD)/server
tests: $(BUILD)/tests

$(BUILD)/client: src/client.cpp $(COMMON_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/server: $(SERVER_SRCS) $(COMMON_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/tests: src/storage/avl.cpp tests/test_avl.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

run_server: all 
			./$(BUILD)/server -p $(PORT)

run_client: all 
			./$(BUILD)/client $(CMD)

clean:
	rm -rf $(BUILD)
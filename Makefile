CXX := g++
MODE ?= debug
BUILD := build

CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
TESTFLAGS := -lgtest -lgtest_main -lpthread -Iinclude

SERVER_SRCS = src/server.cpp \
				src/storage/hashtable.cpp \
				src/storage/avl.cpp \
				src/storage/zset.cpp

COMMON_SRCS = src/logs/log.cpp

ifeq ($(MODE), debug)
    CXXFLAGS += -g -O0 -DLOG_LEVEL=1
else
    CXXFLAGS += -O2 -DLOG_LEVEL=0
endif

.PHONY: all clean run_server run_client tests run_tests

$(BUILD)/client: src/client.cpp $(COMMON_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/server: $(SERVER_SRCS) $(COMMON_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/tests: tests/unit/test_avl.cpp src/storage/avl.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ $(TESTFLAGS) -o $@

all: $(BUILD)/client $(BUILD)/server
tests: $(BUILD)/tests

run_tests: tests 
			./$(BUILD)/tests

run_server: all 
			./$(BUILD)/server 

run_client: all 
			./$(BUILD)/client $(CMD)

clean:
	rm -rf $(BUILD)
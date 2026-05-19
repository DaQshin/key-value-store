CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
MODE ?= debug

BUILD := build

ifneq ($(MODE), debug)
	CXXFLAGS += -O2
else
	CXXFLAGS += -g -O0
endif

.PHONY: all clean run

all: $(BUILD)/client $(BUILD)/server
tests: $(BUILD)/tests

$(BUILD)/client: src/client.cpp logging/log.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/server: src/server.cpp src/storage/hashtable.cpp src/storage/avl.cpp logging/log.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/tests: src/storage/avl.cpp tests/test_avl.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

run_server: all 
			./$(BUILD)/server

run_client: all 
			./$(BUILD)/client $(CMD)

clean:
	rm -rf $(BUILD)
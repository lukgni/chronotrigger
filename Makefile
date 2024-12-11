CXX := g++
CXXFLAGS := -std=c++17 -Iinclude -Wall -Wextra -O2
SRC_DIR := src
INCLUDE_DIR := include
EXAMPLES_DIR := examples
BUILD_DIR := build
LIB_DIR := $(BUILD_DIR)/lib
EXAMPLES_BUILD_DIR := $(BUILD_DIR)/bin/

LIB := $(LIB_DIR)/libchronotrigger.a
LIB_INCLUDE_DIR := $(LIB_DIR)/include

LIB_SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
LIB_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(LIB_SOURCES))

EXAMPLE_SOURCES := $(wildcard $(EXAMPLES_DIR)/*.cpp)
EXAMPLE_OBJECTS := $(patsubst $(EXAMPLES_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(EXAMPLE_SOURCES))
EXAMPLES := $(patsubst $(EXAMPLES_DIR)/%.cpp, $(EXAMPLES_BUILD_DIR)/%, $(EXAMPLE_SOURCES))

all: $(LIB) $(EXAMPLES)
lib: $(LIB)
examples: $(EXAMPLES)

$(LIB): $(LIB_OBJECTS)
	mkdir -p $(LIB_DIR)
	ar rcs $@ $^
	mkdir -p $(LIB_INCLUDE_DIR)
	cp -r $(INCLUDE_DIR)/chronotrigger $(LIB_INCLUDE_DIR)
	rm -f $(LIB_OBJECTS)  # Remove object files after creating the library

$(EXAMPLES_BUILD_DIR)/%: $(BUILD_DIR)/%.o $(LIB)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@
	rm -f $(BUILD_DIR)/$(basename $@).o  # Remove object files after linking

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(EXAMPLES_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	rm -f $(BUILD_DIR)/$(basename $@).o  # Remove object files after linking

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean build-lib build-examples

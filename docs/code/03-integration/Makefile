# Modify this Makefile as needed to work on your system.

# Compiler and flags
CXX := clang++
CXXFLAGS := -Wall -Wextra -std=c++17 -pthread
INCLUDES := -I../../../install/include
LDFLAGS := -L../../../install/lib -llibmozok

# Files and directories
SRC := main.cpp
BUILD_DIR := build
OUT := $(BUILD_DIR)/app.exe

# OS detection for mkdir
ifeq ($(OS),Windows_NT)
	MKDIR = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
	MKDIR = mkdir -p $(BUILD_DIR)
endif

# Default target
all: $(OUT)

# Build rule
$(OUT): $(SRC)
	$(MKDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

# Clean rule
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean


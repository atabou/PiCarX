# Compiler
CXX := g++

# Directories
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build

# Files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# Flags
CXXFLAGS := -MMD -MP
LDFLAGS :=
LDLIBS := -li2c -lgpiod -lpthread

# Target
TARGET := main

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

-include $(DEPS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

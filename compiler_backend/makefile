# define C compiler
CXX 			= g++

# compiler flags
CXX_FLAGS 		= -Wall -Werror -Weffc++ -g -std=c++17

# linker flags
LDFLAGS 		=

# these may need to be built
BUILD_DIR 		= build
BIN_DIR 		= bin

# files for compilation
SRC_FILES 		:= $(wildcard *.cpp)
OBJS 			:= $(SRC_FILES:%.cpp=$(BUILD_DIR)/%.o)
DEP 			:= $(OBJS:%o=%.d)

.PHONY: clean

# main rule
all: VMTranslator

# directory creation rules
$(BUILD_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

VMTranslator: $(OBJS) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $^

# include all .d files for header dependencies
-include $(DEP)

$(OBJS): $(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

clean:
	$(RM) -rf $(BUILD_DIR) $(BIN_DIR)

# define C compiler
CXX 			= g++

# compiler flags
DEBUG_WARNINGS	= -Wcast-align -Wcast-qual \
				  -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 \
				  -Winit-self -Wlogical-op -Wmissing-declarations \
				  -Wmissing-include-dirs -Wnoexcept -Wold-style-cast \
				  -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion \
				  -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 \
				  -Wswitch-default -Wswitch-enum -Wundef -Winvalid-pch \
				  -Wmissing-format-attribute -Wodr

WARNINGS 		= -pedantic -Wall -Wextra

CXX_FLAGS 		= $(WARNINGS) -g -std=c++17

# linker flags
LDFLAGS 		= #$(WARNINGS)

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

debug: CXX_FLAGS += $(DEBUG_WARNINGS) -DDEBUG
debug: VMTranslator

# include all .d files for header dependencies
-include $(DEP)

$(OBJS): $(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

clean:
	$(RM) -rf $(BUILD_DIR) $(BIN_DIR)

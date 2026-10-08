CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

# Core objects for library
CORE_OBJS = $(OBJ_DIR)/Instruction.o \
            $(OBJ_DIR)/Parser.o \
            $(OBJ_DIR)/OptimizationPatterns.o \
            $(OBJ_DIR)/PeepholeOptimizer.o \
            $(OBJ_DIR)/ControlFlowOptimizer.o

# Targets
MAIN_BIN = $(BIN_DIR)/peephole_opt
TEST1_BIN = $(BIN_DIR)/test_person1
TEST2_BIN = $(BIN_DIR)/test_person2
TEST3_BIN = $(BIN_DIR)/test_person3

all: $(MAIN_BIN) $(TEST1_BIN) $(TEST2_BIN) $(TEST3_BIN)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compilation rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/*.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Main executable
$(MAIN_BIN): $(CORE_OBJS) $(OBJ_DIR)/main.o | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Parser test executable
$(TEST1_BIN): $(OBJ_DIR)/Instruction.o $(OBJ_DIR)/Parser.o $(TEST_DIR)/test_person1.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Optimizer test executable
$(TEST2_BIN): $(CORE_OBJS) $(TEST_DIR)/test_person2.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Control Flow test executable
$(TEST3_BIN): $(CORE_OBJS) $(TEST_DIR)/test_person3.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(TEST1_BIN) $(TEST2_BIN) $(TEST3_BIN)
	@echo "\n>>> RUNNING TEST SUITE (PARSER) <<<"
	./$(TEST1_BIN)
	@echo "\n>>> RUNNING TEST SUITE (OPTIMIZER) <<<"
	./$(TEST2_BIN)
	@echo "\n>>> RUNNING TEST SUITE (CONTROL FLOW) <<<"
	./$(TEST3_BIN)
	@echo "\n>>> ALL TESTS PASSED! <<<\n"

test1: $(TEST1_BIN)
	./$(TEST1_BIN)

test2: $(TEST2_BIN)
	./$(TEST2_BIN)

test3: $(TEST3_BIN)
	./$(TEST3_BIN)

demo: $(MAIN_BIN)
	./$(MAIN_BIN) tests/sample.asm

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all test test1 test2 test3 demo clean


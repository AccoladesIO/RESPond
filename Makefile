SRC_DIR   := src
TEST_DIR  := tests
BUILD_DIR := build

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -I$(SRC_DIR) -MMD -MP

GTEST_PREFIX := $(shell brew --prefix googletest 2>/dev/null)
ifneq ($(GTEST_PREFIX),)
    GTEST_INC  := -I$(GTEST_PREFIX)/include
    GTEST_LIB  := -L$(GTEST_PREFIX)/lib
endif

GTEST_LIBS := $(GTEST_LIB) -lgtest -lgtest_main -pthread

APP      := redis_cli
TEST_BIN := unit_tests

CORE_SRCS := $(filter-out $(SRC_DIR)/main.cpp,$(wildcard $(SRC_DIR)/*.cpp))
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CORE_SRCS))
MAIN_OBJ  := $(BUILD_DIR)/main.o

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

.PHONY: all app test clean run
.DEFAULT_GOAL := app

all: app $(TEST_BIN)
app: $(APP)

$(APP): $(CORE_OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_BIN): $(CORE_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(GTEST_LIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)/tests
	$(CXX) $(CXXFLAGS) $(GTEST_INC) -c $< -o $@

$(BUILD_DIR) $(BUILD_DIR)/tests:
	mkdir -p $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR) $(APP) $(TEST_BIN)

run: $(APP)
	./$(APP)

-include $(CORE_OBJS:.o=.d) $(MAIN_OBJ:.o=.d) $(TEST_OBJS:.o=.d)
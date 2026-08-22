# ---------------------------------------------------------------------------
# Layout
#   src/     application sources (.cpp/.h) including main.cpp
#   tests/   GoogleTest sources
#   build/   object + dependency files (generated)
# ---------------------------------------------------------------------------
SRC_DIR   := src
TEST_DIR  := tests
BUILD_DIR := build

CXX      := g++
# -MMD -MP generate .d files so headers are tracked automatically.
CXXFLAGS := -std=c++17 -Wall -Wextra -g -I$(SRC_DIR) -MMD -MP

# GoogleTest is linked from the system install (libgtest-dev). pthread is required.
GTEST_LIBS := -lgtest -lgtest_main -pthread

APP      := redis_cli
TEST_BIN := unit_tests

# Core = every src file EXCEPT main.cpp, shared by the app and the tests.
CORE_SRCS := $(filter-out $(SRC_DIR)/main.cpp,$(wildcard $(SRC_DIR)/*.cpp))
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CORE_SRCS))
MAIN_OBJ  := $(BUILD_DIR)/main.o

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

.PHONY: all app test clean
.DEFAULT_GOAL := app

all: app $(TEST_BIN)
app: $(APP)

# --- Link ------------------------------------------------------------------
$(APP): $(CORE_OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_BIN): $(CORE_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(GTEST_LIBS) -o $@

# --- Compile ---------------------------------------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)/tests
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR) $(BUILD_DIR)/tests:
	mkdir -p $@

# --- Build then run the suite ----------------------------------------------
test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR) $(APP) $(TEST_BIN)

# Pull in auto-generated header dependencies.
-include $(CORE_OBJS:.o=.d) $(MAIN_OBJ:.o=.d) $(TEST_OBJS:.o=.d)
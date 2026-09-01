# ===========================================================================
# respond — build, test, and packaging
#
#   make            build the redis_cli client (default)
#   make all        build the client and the unit_tests binary
#   make test       build and run the test suite
#   make run        build then launch the interactive REPL
#   make static     build build/librespond.a
#   make shared     build build/librespond.so (.dylib on macOS)
#   make install    install headers + libs + pkg-config under PREFIX
#   make uninstall  remove an installed copy
#   make clean      remove build artefacts
#
# Install location is controlled by PREFIX (default /usr/local); DESTDIR is
# supported for staged/packaged installs:
#   make install PREFIX=$HOME/.local
#   make install DESTDIR=/tmp/pkg PREFIX=/usr
# ===========================================================================

SRC_DIR   := src
TEST_DIR  := tests
BUILD_DIR := build

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -I$(SRC_DIR) -MMD -MP

# GoogleTest: prefer a Homebrew install if present, else the system copy.
GTEST_PREFIX := $(shell brew --prefix googletest 2>/dev/null)
ifneq ($(GTEST_PREFIX),)
    GTEST_INC := -I$(GTEST_PREFIX)/include
    GTEST_LIB := -L$(GTEST_PREFIX)/lib
endif
GTEST_LIBS := $(GTEST_LIB) -lgtest -lgtest_main -pthread

APP      := redis_cli
TEST_BIN  := unit_tests
BENCH_BIN := bench

# Core = every src file EXCEPT main.cpp; shared by the app, tests, and library.
CORE_SRCS := $(filter-out $(SRC_DIR)/main.cpp $(SRC_DIR)/bench.cpp,$(wildcard $(SRC_DIR)/*.cpp))
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CORE_SRCS))
MAIN_OBJ  := $(BUILD_DIR)/main.o

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SRCS))

.PHONY: all app test run clean static shared install uninstall
.DEFAULT_GOAL := app

# --- Application & tests ----------------------------------------------------
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

run: $(APP)
	./$(APP)

# Optional micro-benchmark (src/bench.cpp has its own main()).
bench: $(CORE_OBJS) $(BUILD_DIR)/bench.o
	$(CXX) $(CXXFLAGS) $^ -o $(BENCH_BIN)

clean:
	rm -rf $(BUILD_DIR) $(APP) $(TEST_BIN) $(BENCH_BIN)

# --- Library packaging ------------------------------------------------------
LIB_NAME   := respond
VERSION    := 0.1.0
STATIC_LIB := $(BUILD_DIR)/lib$(LIB_NAME).a

# Shared-library extension/flag differ per OS: .so on Linux, .dylib on macOS.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SHARED_EXT  := dylib
    SHARED_FLAG := -dynamiclib
else
    SHARED_EXT  := so
    SHARED_FLAG := -shared
endif
SHARED_LIB := $(BUILD_DIR)/lib$(LIB_NAME).$(SHARED_EXT)

# Shared libraries need position-independent code, kept in a separate dir so
# the app/test objects stay non-PIC.
PIC_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/pic/%.o,$(CORE_SRCS))

# Install layout. Headers go under include/respond/ to avoid name clashes.
PREFIX     ?= /usr/local
INCLUDEDIR := $(PREFIX)/include/$(LIB_NAME)
LIBDIR     := $(PREFIX)/lib
PCDIR      := $(LIBDIR)/pkgconfig
HEADERS    := $(wildcard $(SRC_DIR)/*.h)

static: $(STATIC_LIB)
shared: $(SHARED_LIB)

$(STATIC_LIB): $(CORE_OBJS)
	ar rcs $@ $^

$(SHARED_LIB): $(PIC_OBJS)
	$(CXX) $(SHARED_FLAG) -o $@ $^

$(BUILD_DIR)/pic/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)/pic
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

$(BUILD_DIR)/pic:
	mkdir -p $@

# Generate the pkg-config file with the active PREFIX baked in.
$(BUILD_DIR)/$(LIB_NAME).pc: | $(BUILD_DIR)
	@printf 'prefix=%s\n' '$(PREFIX)'                              >  $@
	@printf 'exec_prefix=$${prefix}\n'                             >> $@
	@printf 'libdir=$${exec_prefix}/lib\n'                         >> $@
	@printf 'includedir=$${prefix}/include\n\n'                    >> $@
	@printf 'Name: %s\n' '$(LIB_NAME)'                             >> $@
	@printf 'Description: C++ Redis (RESP/RESP3) client library\n' >> $@
	@printf 'Version: %s\n' '$(VERSION)'                           >> $@
	@printf 'Libs: -L$${libdir} -l%s\n' '$(LIB_NAME)'              >> $@
	@printf 'Cflags: -I$${includedir}\n'                           >> $@

install: $(STATIC_LIB) $(SHARED_LIB) $(BUILD_DIR)/$(LIB_NAME).pc
	mkdir -p $(DESTDIR)$(INCLUDEDIR) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(PCDIR)
	cp $(HEADERS) $(DESTDIR)$(INCLUDEDIR)/
	cp $(STATIC_LIB) $(SHARED_LIB) $(DESTDIR)$(LIBDIR)/
	cp $(BUILD_DIR)/$(LIB_NAME).pc $(DESTDIR)$(PCDIR)/

uninstall:
	rm -rf $(DESTDIR)$(INCLUDEDIR)
	rm -f  $(DESTDIR)$(LIBDIR)/lib$(LIB_NAME).a
	rm -f  $(DESTDIR)$(LIBDIR)/lib$(LIB_NAME).$(SHARED_EXT)
	rm -f  $(DESTDIR)$(PCDIR)/$(LIB_NAME).pc

# --- Auto-generated header dependencies -------------------------------------
-include $(CORE_OBJS:.o=.d) $(MAIN_OBJ:.o=.d) $(TEST_OBJS:.o=.d) $(PIC_OBJS:.o=.d)
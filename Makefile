CC=gcc
CFLAGS=-std=c11 -Wall -Werror -ggdb
VALGRIND=valgrind --tool=memcheck --leak-check=yes
STRIP=strip
BINARY=bluedelta
MAIN_C = src/main.c
MAIN_O = $(MAIN_C:.c=.o)

INCLUDES = -I./src/include -I.
SOURCES  = $(shell find -path "./src/utils/*" -name "*.c")

OBJECTS  = $(SOURCES:.c=.o)

TEST_SOURCES = $(shell find -path "./src/test/*" -name "*.c")
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TESTS        = $(TEST_SOURCES:.c=.test)

.PRECIOUS: %.c %.o %.h

$(BINARY): $(OBJECTS) $(MAIN_O)
	$(CC) $(CFLAGS) -o $(BINARY) $(OBJECTS) $(MAIN_O) $(LIBS)

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARY) $(OBJECTS) $(TEST_OBJECTS) $(TESTS) $(MAIN_O)

.PHONY: stats
stats:
	git ls-files | xargs wc -l

.PHONY: build
build: CFLAGS += -DNDEBUG
build: clean $(BINARY)

.PHONY: tests run_tests
test: $(TESTS) run_tests

run_tests: $(TESTS)
	$(foreach test,$(TESTS),echo $(test); $(test);)

%.test: %.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJECTS) $(INCLUDES) $(LIBS)

.PHONY: debug
debug: CFLAGS := $(filter-out -O3,$(CFLAGS)) -DDEBUG -O0
ifeq ($(CC),gcc)
debug: CFLAGS += -fsanitize=address
endif
debug: clean $(BINARY)

.PHONY: debug-test
debug-test: CFLAGS := $(filter-out -O3,$(CFLAGS)) -DDEBUG -O0
ifeq ($(CC),gcc)
debug-test: CFLAGS += -fsanitize=address
endif
debug-test: test

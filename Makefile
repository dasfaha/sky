################################################################################
# Variables
################################################################################

CFLAGS=-g -Wall -Wextra -Wno-self-assign -std=c99 -D_FILE_OFFSET_BITS=64 `llvm-config --cflags`
CXXFLAGS=-g -Wall -Wextra -Wno-self-assign -D_FILE_OFFSET_BITS=64 `llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`

SOURCES=$(wildcard src/**/*.c src/**/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES}) $(patsubst %.l,%.o,${LEX_SOURCES}) $(patsubst %.y,%.o,${YACC_SOURCES})
LIB_SOURCES=$(filter-out $(wildcard src/sky_*.c),${SOURCES})
LIB_OBJECTS=$(filter-out $(wildcard src/sky_*.o),${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c tests/**/*_tests.c)
TEST_OBJECTS=$(patsubst %.c,%,${TEST_SOURCES})


################################################################################
# Default Target
################################################################################

all: bin/libsky.a bin/skyd bin/sky-gen bin/sky-bench test


################################################################################
# Binaries
################################################################################

bin/libsky.a: bin ${LIB_OBJECTS}
	rm -f bin/libsky.a
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

bin/skyd: bin ${OBJECTS}
	$(CC) $(CFLAGS) src/skyd.o -o $@ bin/libsky.a
	chmod 700 $@

bin/sky-gen: bin ${OBJECTS}
	$(CC) $(CFLAGS) src/sky_gen.o -o $@ bin/libsky.a
	chmod 700 $@

bin/sky-bench: bin ${OBJECTS}
	$(CC) $(CFLAGS) -Isrc -c -o $@.o src/sky_bench.c
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $@.o bin/libsky.a
	rm $@.o
	chmod 700 $@

bin:
	mkdir -p bin


################################################################################
# Tests
################################################################################

.PHONY: test
test: $(TEST_OBJECTS)
	@sh ./tests/runtests.sh

$(TEST_OBJECTS): %: %.c bin/libsky.a
	$(CC) $(CFLAGS) -Isrc -c -o $@.o $<
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $@.o bin/libsky.a


################################################################################
# Clean up
################################################################################

clean: 
	rm -rf bin ${OBJECTS} ${TEST_OBJECTS} ${LEX_OBJECTS} ${YACC_OBJECTS}
	rm -rf tests/*.dSYM
	rm -rf tmp/*

################################################################################
# Variables
################################################################################

CFLAGS=-g -Wall -Wextra -Wno-self-assign -std=c99 -D_FILE_OFFSET_BITS=64 `llvm-config --cflags`
CXXFLAGS=-g -Wall -Wextra -Wno-self-assign -D_FILE_OFFSET_BITS=64 `llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`

SOURCES=$(wildcard src/**/*.c src/**/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES}) $(patsubst %.l,%.o,${LEX_SOURCES}) $(patsubst %.y,%.o,${YACC_SOURCES})
BIN_SOURCES=src/skyd.c,src/sky_bench.c,src/sky_gen.c
BIN_OBJECTS=$(patsubst %.c,%.o,${BIN_SOURCES})
LIB_SOURCES=$(filter-out ${BIN_SOURCES},${SOURCES})
LIB_OBJECTS=$(filter-out ${BIN_OBJECTS},${OBJECTS})
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

bin/skyd: bin ${OBJECTS} bin/libsky.a
	$(CC) $(CFLAGS) -Isrc -c -o $@.o src/skyd.c
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $@.o bin/libsky.a
	rm $@.o
	chmod 700 $@

bin/sky-gen: bin ${OBJECTS} bin/libsky.a
	$(CC) $(CFLAGS) src/sky_gen.o -o $@ bin/libsky.a
	chmod 700 $@

bin/sky-bench: bin ${OBJECTS} bin/libsky.a
	$(CC) $(CFLAGS) -Isrc -c -o $@.o src/sky_bench.c
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $@.o bin/libsky.a
	rm $@.o
	chmod 700 $@

bin:
	mkdir -p bin


################################################################################
# Qip
################################################################################

src/qip/lexer.o: src/qip/lexer.c
	$(CC) $(CFLAGS) -Wno-unused-parameter -Wno-unused-function -Isrc -c -o $@ $<

src/qip/parser.o: src/qip/parser.c
	$(CC) $(CFLAGS) -Wno-unused-parameter -Isrc -c -o $@ $<


################################################################################
# Tests
################################################################################

.PHONY: test
test: $(TEST_OBJECTS) tmp
	@sh ./tests/runtests.sh

$(TEST_OBJECTS): %: %.c bin/libsky.a
	$(CC) $(CFLAGS) -Isrc -c -o $@.o $<
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $@.o bin/libsky.a


################################################################################
# Misc
################################################################################

tmp:
	mkdir -p tmp

clean: 
	rm -rf bin ${OBJECTS} ${TEST_OBJECTS} ${LEX_OBJECTS} ${YACC_OBJECTS}
	rm -rf tests/*.dSYM tests/*.o
	rm -rf tmp/*

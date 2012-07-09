################################################################################
# Variables
################################################################################

CFLAGS=-g -O2 -Wall -Wextra -Wno-self-assign -std=c99 -D_FILE_OFFSET_BITS=64

SOURCES=$(wildcard src/**/*.c src/**/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES}) $(patsubst %.l,%.o,${LEX_SOURCES}) $(patsubst %.y,%.o,${YACC_SOURCES})
LIB_SOURCES=$(filter-out $(wildcard src/sky_*.c),${SOURCES})
LIB_OBJECTS=$(filter-out $(wildcard src/sky_*.o),${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c tests/**/*_tests.c)
TEST_OBJECTS=$(patsubst %.c,%,${TEST_SOURCES})


################################################################################
# Default Target
################################################################################

all: build/libsky.a build/skyd build/sky-gen build/sky-bench test


################################################################################
# Binaries
################################################################################

build/libsky.a: build ${LIB_OBJECTS}
	rm -f build/libsky.a
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

build/skyd: build ${OBJECTS}
	$(CC) $(CFLAGS) src/skyd.o -o $@ build/libsky.a
	chmod 700 $@

build/sky-gen: build ${OBJECTS}
	$(CC) $(CFLAGS) src/sky_gen.o -o $@ build/libsky.a
	chmod 700 $@

build/sky-bench: build ${OBJECTS}
	$(CC) $(CFLAGS) src/sky_bench.o -o $@ build/libsky.a
	chmod 700 $@

build:
	mkdir -p build


################################################################################
# Tests
################################################################################

.PHONY: test
test: $(TEST_OBJECTS)
	@sh ./tests/runtests.sh

$(TEST_OBJECTS): %: %.c build/libsky.a
	$(CC) $(CFLAGS) -Isrc -o $@ $< build/libsky.a


################################################################################
# Clean up
################################################################################

clean: 
	rm -rf build ${OBJECTS} ${TEST_OBJECTS} ${LEX_OBJECTS} ${YACC_OBJECTS}
	rm -rf tests/*.dSYM
	rm -rf tmp/*

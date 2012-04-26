################################################################################
# Variables
################################################################################

CFLAGS=-g -O2 -Wall -Wextra -Wno-self-assign -std=c99 -D_FILE_OFFSET_BITS=64

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES})
LIB_SOURCES=$(filter-out $(wildcard src/sky_*.c),${SOURCES})
LIB_OBJECTS=$(filter-out $(wildcard src/sky_*.o),${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c)
TEST_OBJECTS=$(patsubst %.c,%,${TEST_SOURCES})


################################################################################
# Default Target
################################################################################

all: build/libsky.a build/sky-gen test


################################################################################
# Binaries
################################################################################

build/libsky.a: build ${LIB_OBJECTS}
	rm -f build/libsky.a
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

build/sky-gen: build ${OBJECTS}
	$(CC) $(CFLAGS) src/sky_gen.o -o $@ build/libsky.a
	chmod 700 $@

build:
	mkdir -p build


################################################################################
# Tests
################################################################################

.PHONY: test
test: $(TEST_OBJECTS)
	@sh ./tests/runtests.sh

build/tests:
	mkdir -p build/tests

$(TEST_OBJECTS): %: %.c build/tests build/libsky.a
	$(CC) $(CFLAGS) -Isrc -o build/$@ $< build/libsky.a


################################################################################
# Clean up
################################################################################

clean: 
	rm -rf build ${OBJECTS} ${TEST_OBJECTS}
	rm -rf tests/*.dSYM
	rm -rf tmp/*

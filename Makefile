################################################################################
# Variables
################################################################################

CFLAGS=-g -O2 -Wall -Wextra

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES})
LIB_SOURCES=$(filter-out src/sky_*.c,${SOURCES})
LIB_OBJECTS=$(filter-out src/sky_*.o,${OBJECTS})
TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,${TEST_SRC})


################################################################################
# Default Target
################################################################################

all: bin/libsky.a bin/sky-standalone test


################################################################################
# Binaries
################################################################################

bin/libsky.a: bin ${LIB_OBJECTS}
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

bin/sky-standalone: bin ${OBJECTS}
	$(CC) $(CFLAGS) src/sky_standalone.o -o $@ bin/libsky.a
	chmod 700 $@

bin:
	mkdir -p bin


################################################################################
# Tests
################################################################################

.PHONY: test
test: $(TESTS)
	sh ./tests/runtests.sh

$(TESTS): $(TEST_SRC)
	$(CC) $(CFLAGS) -Isrc -o $@ $< bin/libsky.a


################################################################################
# Clean up
################################################################################

clean: 
	rm -rf bin ${OBJECTS}

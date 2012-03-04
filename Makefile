################################################################################
# Variables
################################################################################

CFLAGS=-g -O2 -Wall -Wextra

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES})
LIB_SOURCES=$(filter-out $(wildcard src/sky_*.c),${SOURCES})
LIB_OBJECTS=$(filter-out $(wildcard src/sky_*.o),${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c)
TEST_OBJECTS=$(patsubst %.c,%,${TEST_SOURCES})
EXEC=

################################################################################
# Default Target
################################################################################

all: bin/libsky.a bin/sky-standalone test


################################################################################
# Valgrind
################################################################################

valgrind:
	EXEC=valgrind --leak-check=full --show-reachable=yes --log-file=valgrind.log


################################################################################
# Binaries
################################################################################

bin/libsky.a: bin ${LIB_OBJECTS}
	rm -f bin/libsky.a
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
test: $(TEST_OBJECTS)
	@sh ./tests/runtests.sh

$(TEST_OBJECTS): %: %.c bin/libsky.a
	$(CC) $(CFLAGS) -Isrc -o $@ $< bin/libsky.a


################################################################################
# Clean up
################################################################################

clean: 
	rm -rf bin ${OBJECTS} ${TEST_OBJECTS}
	rm -rf tests/*.dSYM
	rm -rf tmp/*

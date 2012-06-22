################################################################################
# Variables
################################################################################

CFLAGS=-g -O2 -Wall -Wextra -Wno-self-assign -std=c99 -D_FILE_OFFSET_BITS=64 `llvm-config --cflags`
CXXFLAGS=-g -O2 -Wall -Wextra -Wno-self-assign -D_FILE_OFFSET_BITS=64 `llvm-config --libs --cflags --ldflags core analysis executionengine jit interpreter native`

LEX_SOURCES=$(wildcard src/*.l) $(wildcard src/**/*.l)
LEX_OBJECTS=$(patsubst %.l,%.c,${LEX_SOURCES}) $(patsubst %.l,%.h,${LEX_SOURCES})

YACC_SOURCES=$(wildcard src/*.y) $(wildcard src/**/*.y)
YACC_OBJECTS=$(patsubst %.y,%.c,${YACC_SOURCES}) $(patsubst %.y,%.h,${YACC_SOURCES})

SOURCES=$(wildcard src/**/*.c src/**/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES}) $(patsubst %.l,%.o,${LEX_SOURCES}) $(patsubst %.y,%.o,${YACC_SOURCES})
LIB_SOURCES=$(filter-out $(wildcard src/sky_*.c),${SOURCES})
LIB_OBJECTS=$(filter-out $(wildcard src/sky_*.o),${OBJECTS})
TEST_SOURCES=$(wildcard tests/*_tests.c tests/**/*_tests.c)
TEST_OBJECTS=$(patsubst %.c,%,${TEST_SOURCES})

LLVM_IR_SOURCES=$(wildcard misc/eql/ir/*.c)
LLVM_IR_OBJECTS=$(patsubst %.c,%.s,${LLVM_IR_SOURCES})

LEX=flex
YACC=bison
YFLAGS=-dv


################################################################################
# Default Target
################################################################################

all: eql build/libsky.a build/skyd build/sky-gen build/sky-bench test


################################################################################
# Binaries
################################################################################

build/libsky.a: build ${LIB_OBJECTS}
	rm -f build/libsky.a
	ar rcs $@ ${LIB_OBJECTS}
	ranlib $@

build/skyd: build ${OBJECTS}
	$(CXX) $(CXXFLAGS) src/skyd.o -o $@ build/libsky.a
	chmod 700 $@

build/sky-gen: build ${OBJECTS}
	$(CXX) $(CXXFLAGS) src/sky_gen.o -o $@ build/libsky.a
	chmod 700 $@

build/sky-bench: build ${OBJECTS}
	$(CXX) $(CXXFLAGS) src/sky_bench.o -o $@ build/libsky.a
	chmod 700 $@

build:
	mkdir -p build


################################################################################
# Bison / Flex
################################################################################

.PHONY: eql
eql: ${LEX_OBJECTS}

src/eql/lexer.c: src/eql/parser.c src/eql/lexer.l
	${LEX} --header-file=src/eql/lexer.h -o $@ src/eql/lexer.l

src/eql/parser.c: src/eql/parser.y src/eql/parser.y
	mkdir -p build/bison
	${YACC} ${YFLAGS} --report-file=build/bison/report.txt -o $@ $^

################################################################################
# Tests
################################################################################

.PHONY: test
test: $(TEST_OBJECTS)
	@sh ./tests/runtests.sh

build/tests:
	mkdir -p build/tests
	mkdir -p build/tests/eql

$(TEST_OBJECTS): %: %.c build/tests build/libsky.a
	$(CC) $(CFLAGS) -Isrc -c -o build/$@.o $<
	$(CXX) $(CXXFLAGS) -Isrc -o build/$@ build/$@.o build/libsky.a


################################################################################
# LLVM IR Examples
################################################################################

.PHONY: llvm-ir-examples
llvm-ir-examples: $(LLVM_IR_OBJECTS)

$(LLVM_IR_OBJECTS): %.s: %.c
	clang -S -emit-llvm -o $@ $<


################################################################################
# Clean up
################################################################################

clean: 
	rm -rf build ${OBJECTS} ${TEST_OBJECTS} ${LEX_OBJECTS} ${YACC_OBJECTS}
	rm -rf misc/eql/ir/*.s
	rm -rf tests/*.dSYM
	rm -rf tmp/*

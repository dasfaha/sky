CFLAGS=-g -O2 -Wall

SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c,%.o,${SOURCES})

all: bin/sky-standalone

bin/sky-standalone: bin ${OBJECTS}
	$(CC) $(CFLAGS) src/sky_standalone.o -o $@
	chmod 700 $@

bin:
	mkdir -p bin

test:
	cucumber

clean: 
	rm src/*.o
	rm -rf bin

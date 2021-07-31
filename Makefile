CC = gcc
CFLAGS = -Wall -Werror -g -fopenmp

LIBS += -lrt

SOURCES  = $(wildcard *.c)
INCLUDES = $(wildcard *.h)
OBJECTS  = $(SOURCES:.c=.o)

LAB = engine

all: clean $(LAB)

$(LAB):$(OBJECTS)
	$(CC) $(CFLAGS) -o $(LAB) $(OBJECTS) $(LIBS)

$(OBJECTS):$(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) -c $(SOURCES)

run: all
	./$(LAB)

.PHONY: clean
clean:
	-rm *.o 2>> /dev/null;
	-rm *.so 2>> /dev/null;
	-rm core.* 2>> /dev/null;
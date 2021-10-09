CC = gcc
CFLAGS = -Wall -fopenmp -g -O3

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



.PHONY: clean
clean:
	-rm *.o 2>> /dev/null;
	-rm *.so 2>> /dev/null;
	-rm core.* 2>> /dev/null;

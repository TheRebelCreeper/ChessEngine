# Compiler and flags
CC = gcc
CFLAGS = -Wall -Ofast -fcommon -flto
LIBS =

# Source files
SOURCES  = $(wildcard *.c)
INCLUDES = $(wildcard *.h)
OBJECTS  = $(SOURCES:.c=.o)

# Definitions
DEFINES = -DIS_64BIT
DEFINES += -DUSE_AVX2 -mavx2
DEFINES += -DUSE_SSE41 -msse4.1
DEFINES += -DUSE_SSE3 -msse3
DEFINES += -DUSE_SSE2 -msse2
DEFINES += -DUSE_SSE -msse

ifeq ($(OS), Windows_NT)
	CFLAGS += -fstrict-aliasing
	CFLAGS += -fno-exceptions
	CFLAGS += -fomit-frame-pointer
	DEFINES += -Drandom=rand
	DEFINES += -D__USE_MINGW_ANSI_STDIO=1
	EXEEXT = .exe
	RM = del /Q
else
	LIBS += -lrt -lm
	EXEEXT = 
	RM = rm -f
endif

# Executable name
NAME = Saxton
VERSION = $(file < version.txt)
TARGET = $(NAME)_v$(VERSION)

all: release

release: $(TARGET)$(EXEEXT)

debug: CFLAGS += -g
debug: $(TARGET)$(EXEEXT)

$(TARGET)$(EXEEXT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET)$(EXEEXT) $(OBJECTS) $(LIBS) $(DEFINES)

$(OBJECTS):$(SOURCES) $(INCLUDES)
	$(CC) $(CFLAGS) -c $(SOURCES) $(DEFINES)

clean:
	$(RM) $(OBJECTS) $(TARGET)$(EXEEXT)

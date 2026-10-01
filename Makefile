# Clouds Makefile

# Variables
CC = gcc
CFLAGS = -Wall -fstack-protector-strong
LDFLAGS = -Wl,-z,relro,-z,now
LIBS = -lglfw -lGLEW -lGL -lm
SOURCES = transformations.c
TARGET = visualiser 

# Build Rules
all: release

release: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)
	strip $(TARGET)

debug: $(SOURCES)
	$(CC) $(CFLAGS) -g $(LDFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

clean:
	rm -rf $(TARGET)

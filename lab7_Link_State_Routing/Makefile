CC=gcc
CFLAGS=-Wall -Wextra -pedantic -g
LDFLAGS=-lpthread

TARGET=ls_router
SRCS=ls_router.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o


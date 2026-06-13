CC = gcc
CFLAGS = -Wall -std=c99 -O2 -pthread -I src/server
LDFLAGS = -lssl -lcrypto

BINDIR = bin
SRCDIR = src
SERVER_SRCS = $(SRCDIR)/server.c \
              $(SRCDIR)/client.c \
              $(SRCDIR)/commands.c \
              $(SRCDIR)/dh.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
TARGET = $(BINDIR)/server

.PHONY: all clean

all: $(BINDIR) $(TARGET)

$(BINDIR):
	mkdir -p $(BINDIR)

$(TARGET): $(SERVER_OBJS) | $(BINDIR)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)
	find $(SRCDIR) -name '*.o' -delete
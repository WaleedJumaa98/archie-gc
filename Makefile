CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
SRCDIR  = src
SRCS    = $(SRCDIR)/stack.c \
          $(SRCDIR)/archie_object.c \
          $(SRCDIR)/archie_new.c \
          $(SRCDIR)/vm.c \
          $(SRCDIR)/main.c
TARGET  = archie-gc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

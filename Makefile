CC = gcc
CFLAGS = -Wall
BUILD = build
SRCS = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJS = $(patsubst %.c,$(BUILD)/%.o,$(SRCS))
TARGET = app
MKDIR = mkdir -p

.phony: clean dir all

all: dir $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

$(OBJS): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) $(debug) -c -o $@ $(subst $(BUILD)/,,$(@:.o=.c)) -I.

dir:
	$(MKDIR) $(BUILD)

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)

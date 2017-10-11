CC = gcc
CFLAGS = -Wall
BUILD = build
SRCS = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJS = $(patsubst %.c,$(BUILD)/%.o,$(SRCS))
TARGET = server
MKDIR = mkdir -p
BUILD_CMD = $(CC) $(CFLAGS) $(debug) -c -o $@ $<

.phony: clean dir all

all: dir $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lpthread

$(BUILD)/main.o: main.c
	$(BUILD_CMD)

$(filter-out $(BUILD)/main.o,$(OBJS)): $(BUILD)/%.o: %.c %.h
	$(BUILD_CMD)

dir:
	$(MKDIR) $(BUILD)

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)

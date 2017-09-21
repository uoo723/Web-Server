CC = gcc
BUILD = build
OBJS = $(BUILD)/main.o $(BUILD)/http_parser.o
TARGET = app
MKDIR = mkdir -p

.phony: clean dir all

all: dir $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

$(OBJS):
	$(CC) -c -o $@ $(subst $(BUILD)/,,$(@:.o=.c)) -I.

dir:
	$(MKDIR) $(BUILD)

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)

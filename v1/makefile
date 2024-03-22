CC = gcc
SRC_DIR = .
SRCS = $(wildcard $(SRC_DIR)/*.c)
TARGET = toodfish.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	@$(CC) $^ -o $@ -lws2_32

clean:
	@rm -f $(TARGET)
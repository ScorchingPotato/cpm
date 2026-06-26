CC=gcc
CFLAGS=-Wall -Wextra

SRC_SRCS:=$(wildcard src/*.c)
INC_SRCS:=$(wildcard include/src/*.c)

OBJS:=$(patsubst src/%.c,build/%.o,$(SRC_SRCS)) $(patsubst include/src/%.c,build/%.o,$(INC_SRCS))
TARGET=cpm

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ -Linclude/lib -lcurl 

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude/header -c $< -o $@

build/%.o: include/src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude/header -c $< -o $@

clean:
	rm -rf build $(TARGET)

.PHONY: all clean

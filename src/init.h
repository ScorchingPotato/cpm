#ifndef INIT_H
#define INIT_H
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *MAKEFILE_TEMPLATE =
"CC=%s\n"
"CFLAGS=%s\n"
"\n"
"SRC_SRCS:=$(wildcard src/*.c)\n"
"INC_SRCS:=$(wildcard include/src/*.c)\n"
"\n"
"OBJS:=$(patsubst src/%%.c,build/%%.o,$(SRC_SRCS)) $(patsubst include/src/%%.c,build/%%.o,$(INC_SRCS))\n"
"TARGET=%s\n"
"\n"
"all: $(TARGET)\n"
"\n"
"$(TARGET): $(OBJS)\n"
"\t$(CC) $(OBJS) -o $@ -Linclude/lib %s\n"
"\n"
"build/%%.o: src/%%.c\n"
"\t@mkdir -p build\n"
"\t$(CC) $(CFLAGS) -Iinclude/header -c $< -o $@\n"
"\n"
"build/%%.o: include/src/%%.c\n"
"\t@mkdir -p build\n"
"\t$(CC) $(CFLAGS) -Iinclude/header -c $< -o $@\n"
"\n"
"clean:\n"
"\trm -rf build $(TARGET)\n"
"\n"
".PHONY: all clean\n";;

int init(char options, const char* arg);
#endif

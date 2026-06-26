#ifndef PARSECONF_H
#define PARSECONF_H
#include <stdio.h>

typedef struct Entry {
  char* key;
  char* val;
} Entry;

struct Graph {
  Entry* entries;
  int entrc;
};

struct Config {
  struct Graph project;
  struct Graph build;
  struct Graph* deps;
  int depc;
};

enum ImportType {
  NONE,
  HEADER,
  SRC,
  LIB,
  SYS,
};

struct Import {
  int type;
  char* pull;
  char* name;
};

struct Config* parse(FILE* configf);
void freeConfig(struct Config* c);
void printConfig(struct Config* c);
char* getConfigVal(struct Graph g,char* key);
struct Import* parseImports(struct Graph* deps, int impc);
void freeImports(struct Import* imports, int impc);

#endif

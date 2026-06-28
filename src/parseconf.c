#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parseconf.h"

char line[1024] = {0};

struct Config* parse(FILE* configf) {
    int i = 0;
    int bracket = 0;
    int depst = 0;
    
    struct Config* cnf = malloc(sizeof(struct Config));
    struct Graph* currgraph = NULL;

    char str[512] = {0};
    int strlen = 0;


    while (fgets(line, sizeof(line), configf)) {
      i = 0;
      while (1) {
        char c = line[i];
        if (c == 0) break; 
        if (c == ' ' && !strlen) {
          i++;
          continue; 
        } else if (c == '\n') {
          i++;
          continue;
        } else if (c == '[') bracket++;
        else if (c == ']' && bracket) {
          str[strlen] = 0;
          if (!strcmp(str,"project")) currgraph = &cnf->project;
          else if (!strcmp(str, "build")) currgraph = &cnf->build;
          else if (!strcmp(str, "import")) {
            cnf->depc=0;
            depst=1;
          }
          else if (depst) {
            cnf->deps = realloc(cnf->deps, (cnf->depc+1)*sizeof(struct Graph));
            currgraph = &cnf->deps[cnf->depc];
            cnf->depc++;

            currgraph->entrc = 0;
            currgraph->entries = NULL;
          } else {
            fprintf(stderr, "Unrecognized project.conf segment: '[%s]'",str);
            return NULL;
          }
          strlen = 0;
          bracket--;
        } else if (c == '=') {
          str[strlen] = 0;
          currgraph->entries = realloc(currgraph->entries, (currgraph->entrc+1)*sizeof(Entry));
          currgraph->entries[currgraph->entrc].key = strdup(str);
          strlen = 0;
        } else if (c == ';') {
          str[strlen] = 0;
          currgraph->entries[currgraph->entrc].val = strdup(str);
          currgraph->entrc++;
          strlen = 0;
        } else {
          str[strlen] = c;
          strlen++;
        }
        i++;
      };
    };

    
    return cnf;
}

char* getConfigVal(struct Graph g,char* key) {
  for (int i=0; i < g.entrc; i++) {
    if (!strcmp(g.entries[i].key, key)) return g.entries[i].val;
  }
  return NULL;
}

int getitype(char* t) {
  if (!strcmp(t, "header")) return HEADER;
  if (!strcmp(t, "src")) return SRC;
  if (!strcmp(t, "lib")) return LIB;
  if (!strcmp(t, "sys")) return SYS;

  return NONE;
}

struct Import* parseImports(struct Graph* deps, int impc) {
  int ic = 0;
  struct Import* imports = calloc(impc, sizeof(struct Import));
  for (int i=0; i < impc; i++) {
    struct Graph imp = deps[i];
    for (int j=0; j < imp.entrc; j++) {
      Entry e = imp.entries[j];
      if (!strcmp(e.key, "type")) {
        int t = getitype(e.val);
        if (t==0) fprintf(stderr, "No import type '%s'", e.val);
        imports[ic].type=t;
      } else if (!strcmp(e.key, "pull")) {
        imports[ic].pull = strdup(e.val);
      } else if (!strcmp(e.key, "name")) {
        imports[ic].name = strdup(e.val);
      };
    }
    ic++;
  }

  return imports;
}

void freeImports(struct Import* imports, int impc) {
  for (int i=0; i<impc;i++) {
    struct Import imp = imports[i];
    free(imp.name);
    free(imp.pull);
  }
  free(imports);
}

 
void printEntry(Entry e) {
  printf("   %s=%s\n",e.key,e.val);
}

void printConfig(struct Config* c) {
  puts("Project:");
  for (int i=0; i < c->project.entrc; i++) {
   printEntry(c->project.entries[i]);
  };
  puts("Build:");
  for (int i=0; i < c->build.entrc; i++) {
    printEntry(c->build.entries[i]);
  };
  puts("Import:");
  for (int i=0; i < c->depc; i++) {
    printf("   import%d:\n",i+1);
    for (int j=0; j < c->deps[i].entrc; j++) {
      printf("   ");
      printEntry(c->deps[i].entries[j]);
    }
  }
  
  
}

void freeEntry(Entry e) {
  free(e.key);
  free(e.val);
}

void freeConfig(struct Config* c) {
  if (c == NULL) return;
  //Clear Project Graph
  for (int i=0; i < c->project.entrc; i++) {
   freeEntry(c->project.entries[i]);
  };
  free(c->project.entries);
  //Clear Build Graph
  for (int i=0; i < c->build.entrc; i++) {
    freeEntry(c->build.entries[i]);
  };
  free(c->build.entries);
  //Clear Import Graph
  for (int i=0; i < c->depc; i++) {
    for (int j=0; j < c->deps[i].entrc; j++) {
      freeEntry(c->deps[i].entries[j]);
    }
    free(c->deps[i].entries);
  }
  free(c->deps);

  free(c);
}

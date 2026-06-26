#define _GNU_SOURCE
#include "parseconf.h"
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <ftw.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define bool char

int remove_item(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    return remove(fpath);
}

int deldir(const char *path) {
    return nftw(path, remove_item, 10, FTW_DEPTH | FTW_PHYS);
}

int build(char opt, struct Config* cnf) {
  bool verbose = opt & 0b00000010;
  bool run = opt & 0b00000100;
  bool debug = opt & 0b00001000;
  bool clean = opt & 0b00010000;
  bool make = !strcmp(getConfigVal(cnf->build, "backend"), "make");
  char* out = getConfigVal(cnf->build, "output");
  char* cc = getConfigVal(cnf->build, "compiler");
  char* copt = getConfigVal(cnf->build, "options");

  if (clean) {
    if (verbose) printf("Cleaned build directory\n");
    deldir("build");
    mkdir("build", 0777);
  }
  
  if (make) {
    system("make");
  } else {
    //CPM build
    char syslibflags[1024] = {0};
    int ci = 0;
    struct Import* deps = parseImports(cnf->deps, cnf->depc);

    for (int i=0; i < cnf->depc; i++) {
      if (deps[i].type==SYS) {
        snprintf(syslibflags+ci, sizeof(syslibflags)-ci, "-l%s ",deps[i].name);
        ci += strlen(deps[i].name)+3;
      } 
    };
    ci = 0;
    char libflags[1024] = {0};
    for (int i=0; i < cnf->depc; i++) {
      if (deps[i].type==LIB) {
        snprintf(libflags+ci, sizeof(libflags)-ci, "-l%s ",deps[i].name);
        ci += strlen(deps[i].name)+3;
      } 
    };

       
    char** srcf = NULL;
    int srcfc = 0;

    DIR* srcdir = opendir("src");
    if (srcdir == NULL) {
      fprintf(stderr, "Unable to open 'src' directory");
      return 1;
    }

    struct dirent* e;
    while ((e = readdir(srcdir)) != NULL) {
      int len = strlen(e->d_name);
      if (len > 2 && !strcmp(e->d_name+len-2,".c")) {
        srcfc++;
        srcf = realloc(srcf, srcfc*sizeof(char*));
        srcf[srcfc-1] = malloc(len-1);
        memcpy(srcf[srcfc-1], e->d_name, len-2);
        srcf[srcfc-1][len-2] = 0;
      }
    }
    closedir(srcdir);

    int truesrcfc = srcfc;

    DIR* isrcdir = opendir("include/src");
    if (isrcdir == NULL) {
      fprintf(stderr, "Unable to open 'include/src' directory");
      return 1;
    }

    while ((e = readdir(isrcdir)) != NULL) {
      int len = strlen(e->d_name);
      if (len > 2 && !strcmp(e->d_name+len-2,".c")) {
        srcfc++;
        srcf = realloc(srcf, srcfc*sizeof(char*));
        srcf[srcfc-1] = malloc(len-1);
        memcpy(srcf[srcfc-1], e->d_name, len-2);
        srcf[srcfc-1][len-2] = 0;
      }
    }
    closedir(srcdir);
    

    struct stat srcattr;
    struct stat bldattr;

    char buf[2048];
    for (int i=0; i < srcfc; i++) {
        snprintf(buf, sizeof(buf), "src/%s.c", srcf[i]);
        stat(buf, &srcattr);
        snprintf(buf, sizeof(buf), "build/%s.o", srcf[i]);
        if (stat(buf, &bldattr) == -1) {
          goto buildobj;
        } else if (srcattr.st_ctime > bldattr.st_ctime) {
          goto buildobj;
        } else {
          continue;
        }

        buildobj:
        snprintf(buf, sizeof(buf), "%s %s %s -Iinclude/header -c %ssrc/%s.c -o build/%s.o",cc,copt,(debug) ? "-g" : "",(i>truesrcfc-1) ? "include/" : "",srcf[i],srcf[i]);
        if (verbose) printf("%s\n", buf);
        system(buf);
    };

    int o = 0;
    snprintf(buf, sizeof(buf), "%s %s %s -Iinclude/header ",cc,copt,(debug) ? "-g" : "");
    o = strlen(buf);

    DIR* blddir = opendir("build");
    while ((e = readdir(blddir)) != NULL) {
      int len = strlen(e->d_name);
      if (len > 2 && !strcmp(e->d_name+len-2,".o")) {
        snprintf(buf+o, sizeof(buf)-o, "build/%s ", e->d_name);
        o += len+7;
      };
    };

    closedir(blddir);

    snprintf(buf+o, sizeof(buf)-o, "-o %s %s -Linclude/lib %s", out, syslibflags, libflags);

    if (verbose) printf("%s\n", buf);
    system(buf);

    freeImports(deps,cnf->depc);
    for (int i=0; i<srcfc; i++) {
      free(srcf[i]);
    };
    free(srcf);
  }

  if (run) {
    char path[1024];
    snprintf(path, sizeof(path), "./%s",out);
    execvp(path,NULL);
  };

  return 0;
}

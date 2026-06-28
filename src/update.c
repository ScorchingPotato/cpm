#include "parseconf.h"
#include "init.h"
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#define bool char

int update(char opt, struct Config* cnf) {
  bool verbose = opt & 0b00000010;
  bool pull = opt & 0b00000100;
  bool make = opt & 0b00010000;

  if (cnf == NULL) {
    fprintf(stderr, "Configuration file 'project.conf' missing. Aborting\n");
    return 1;
  }

  struct Import* imports = parseImports(cnf->deps, cnf->depc);

  if (make) {

    char syslibflags[1024] = {0};
    int ci = 0;
    for (int i=0; i < cnf->depc; i++) {
      if (imports[i].type==SYS || imports[i].type==LIB) {
        snprintf(syslibflags+ci, sizeof(syslibflags)-ci, "-l%s ",imports[i].name);
        ci += strlen(imports[i].name)+3;
      } 
    };
    
    FILE* makefile = fopen("Makefile", "w");
    fprintf(makefile, MAKEFILE_TEMPLATE, getConfigVal(cnf->build, "compiler"), getConfigVal(cnf->build, "options"), getConfigVal(cnf->build, "output"), syslibflags);
    fclose(makefile);
  } else if (pull) {
    CURL* curl;
    FILE* f;
    CURLcode res;

    for (int i=0; i<cnf->depc; i++) {
      struct Import imp = imports[i];
      if (imp.pull && imp.type!=SYS) {
        char* url = imp.pull;
        char outfile[FILENAME_MAX];
        switch (imp.type) {
          case HEADER:
            snprintf(outfile, FILENAME_MAX, "include/header/%s.h",imp.name);
            break;
          case SRC:
            snprintf(outfile, FILENAME_MAX, "include/src/%s.c", imp.name);
            break;
          case LIB:
            snprintf(outfile, FILENAME_MAX, "lib/%s.a",imp.name);
            break;
        };
        curl = curl_easy_init();
        if (curl) {
          f = fopen(outfile, "wb");
          if (f==NULL) {
            fprintf(stderr, "Cannot open file '%s'\n",outfile);
          }
          curl_easy_setopt(curl, CURLOPT_URL, url);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
          curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

          res = curl_easy_perform(curl);

          fclose(f);
          curl_easy_cleanup(curl);

          if (res == CURLE_OK) {
            if (verbose) printf("Succesfully pulled '%s' to '%s'\n",imp.name,outfile);
          } else {
            fprintf(stderr, "Pull on '%s' failed: %s\n",imp.name, curl_easy_strerror(res));
          }
        }
      }
    }
  }

  freeImports(imports, cnf->depc);
  
    
  return 0;
}

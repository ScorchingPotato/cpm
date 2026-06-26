#include "init.h"


const char *dirs[] = {"src", "build", "include","include/src", "include/header", "include/lib"};
const char *files[] = {"Makefile", "src/main.c", "project.conf"};



int init(char options, const char* arg) {
  int verbose = options & 0b00000010;
  int current = options & 0b00000100;
  int naked = options & 0b00001000;
  int make = options & 0b00010000;

  if (!arg) {
    fprintf(stderr, "Missing argument for init (Project Name)\n"); return -1;
  }

  if (!current) {
    DIR* dir = opendir(arg);
    if (dir) {if (verbose) printf("'%s' directory exists\n", arg); goto e1;}
    else if (mkdir(arg, 0777) == -1) {fprintf(stderr, "Failed to create '%s'\n",arg); return -1;}
    if (verbose) printf("Created directory '%s'\n", arg);
    e1:
      if (dir) closedir(dir);
      chdir(arg);
  }

  int di = (naked) ? 2 : 6;
  
  for (int i=0; i < di; ++i) { 
    if (!dirs[i]) continue;
    DIR* dir = opendir(dirs[i]);
    if (dir) {if (verbose) printf("'%s' directory exists\n", dirs[i]); continue;}
    else if (mkdir(dirs[i], 0777) == -1) {fprintf(stderr, "Failed to create '%s'\n",dirs[i]); return -1;}
    if (verbose) printf("Created directory '%s/%s'\n",arg,dirs[i]);
    closedir(dir);
  };



  int si = (make) ? 0 : 1;
  int ei = (naked) ? 2 : 3;

  for (int i=si; i < ei; ++i) {
    FILE *file = fopen(files[i], "w");
    if (verbose) printf("Created file '%s'\n", files[i]);
    if (make && i==0 && !naked) fprintf(file, MAKEFILE_TEMPLATE, "gcc", "-Wall -Wextra", arg, "");
    if (!naked && i==1) fputs("#include <stdio.h>\n\nint main(void)\n{\n\tprintf(\"Hello World\\n\");\n\treturn 0;\n}\n",file);
    if (!naked && i==2) fprintf(file, "[project]\nname=%s;\n\n[build]\ncompiler=gcc;\noptions=-Wall -Wextra;\noutput=%s;\nbackend=%s;\n[import]\n", arg,arg,(make) ? "make" : "none");
    fclose(file);
  };

  if (verbose) printf("Succesfully initialized a new project 'arg'.\n");

  return 0;
}

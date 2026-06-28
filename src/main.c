#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parseconf.h"
#include "init.h"
#include "build.h"
#include "update.h"

struct Config* cnf = NULL;

#define bool char

int optargi = -1;

typedef struct Command {
  int cmd;
  char opt;
  char* arg;
} Command;


enum COMMAND {
  NOCMD,
  INIT,
  BUILD,
  UPDATE,
};


Command c = {0};


enum OPTION {
  NOTOPT,
  HELP,
  VERBOSE,
  //Options:
  OPT1,
  OPT2,
  OPT3,
  //Init options:
  CURRENT = 3,
  NAKED = 4,
  MAKE = 5,
  //Build options:
  RUN = 3,
  DEBUG = 4,
  CLEAN = 5,
  //Update options:
  PULL = 3,
};

int shortoptname(const char c, int cmd) {
  switch (c) {
    case 'h': return HELP;
    case 'v': return VERBOSE;
    case 'c': return (cmd==INIT) ? CURRENT : CLEAN;
    case 'n': return NAKED;
    case 'm': return MAKE;
    case 'r': return RUN;
    case 'd': return DEBUG;
    case 'p': return PULL;

    default: return -1;
  }
}

int longoptname(const char* opt, const int opti) {
  if (!strcmp(opt, "help")) return HELP;
  if (!strcmp(opt, "verbose")) return VERBOSE;
  if (!strcmp(opt, "current")) return CURRENT;
  if (!strcmp(opt, "naked")) return NAKED;
  if (!strcmp(opt, "make")) return MAKE;
  if( !strcmp(opt, "run")) return RUN;
  if (!strcmp(opt, "debug")) return DEBUG;
  if (!strcmp(opt, "clean")) return CLEAN;
  if (!strcmp(opt, "pull")) return PULL;

  else return -opti;
}

int getoption(char *arg, int cmd) {
  optargi = -1;
  char optc[64] = {0};
  int opt = 0;
  int opti = 0;
  while (1) {
    optargi++;
    switch (arg[optargi]) {
      case 0:
        optc[opti+1] = 0;
        return longoptname(optc, opti);
      case '-':
        opt++;
        break;
      default:
        if (!opt) return NOTOPT;
        if (opt == 1) return shortoptname(arg[optargi], cmd);
        else {
          optc[opti] = arg[optargi];
          opti++;
        };
        break;
    }
  }
  return 0;
}

int cmdname(const char* cmd) {
  if (!strcmp(cmd, "init")) return INIT;
  if (!strcmp(cmd, "build")) return BUILD;
  if (!strcmp(cmd, "run")) return RUN;
  if (!strcmp(cmd, "update")) return UPDATE;

  else return -1;
}

int main(int argc, char *argv[]) {
  int i = 1;
  while (i < argc) {
    char *arg = argv[i];  
    int opt = getoption(arg,c.cmd);
    switch (opt) {
      case 0:
        if (!c.cmd) {
          c.cmd = cmdname(arg);
        } else c.arg = arg;
        break;

        
      case HELP: c.opt |= 0b00000001; break;
      case VERBOSE: c.opt |= 0b00000010; break;
      case OPT1: c.opt |= 0b00000100; break;
      case OPT2: c.opt |= 0b00001000; break;
      case OPT3: c.opt |= 0b00010000; break;

      default:
        if (opt < 0) {
          int l = -opt;
          char* erropt = calloc(l+1, sizeof(char));
          if (l == 1) erropt[0] = arg[optargi];
          else for (int j=0; j < l; ++j) erropt[j] = arg[optargi - l + j];

          char optflag[3];
          if (l == 1) memcpy(optflag, "-", 2);
          else memcpy(optflag, "--", 3);

          fprintf(stderr, "%s: unrecognized option '%s%s'\nTry '%s --help' for more information.\n",argv[0], optflag, erropt, argv[0]);
          free(erropt);
          return 1;
        } 
    }
    i++;
  };
  FILE *config = fopen("project.conf", "r");
  if (config != NULL) {
    cnf = parse(config);
    if (cnf == NULL) return -1;
    fclose(config);
  }
 
  if (c.opt & 0b00000001) {
    switch (c.cmd) {
      case NOCMD: goto nocmdhelp;
      case INIT: goto inithelp;
      case BUILD: goto buildhelp;
      case UPDATE: goto updatehelp;

      default: goto nocmdhelp;
    }
  }


  switch (c.cmd) {
    case INIT:
      return init(c.opt,c.arg);
    case BUILD:
      return build(c.opt,cnf);
    case UPDATE:
      return update(c.opt,cnf);
  }

  freeConfig(cnf);
    
  return 0;

  //HELP PAGES
  nocmdhelp:
    printf("usage: %s [COMMAND] [OPTION]... [ARGUMENT]\nSimple C programming language project manager.\n\nList of available commands. Try '%s [COMMAND] --help' for more information\n", argv[0],argv[0]);
    printf("\n\e[1m   %-8s\e[minitialize project directory\n", "init");
    printf("\n\e[1m   %-8s\e[mbuild current project\n", "build"); 
    printf("\n\e[1m   %-8s\e[mupdate current project\n", "update");
    return 0;
  inithelp:
    printf("usage: %s init [OPTION]... [ARG]\nInitialize project directory.\n", argv[0]);
    printf("\nArguments:\n");
    printf("\e[1m   <PROJECT NAME>\e[m\n");
    printf("\nOptions:\n");
    printf("\n\e[1m   %-24s\e[m%s\n", "-c\e[1m, \e[1m--current", "Initialize project in current directory rather than creating a new one");
    printf("\n\e[1m   %-24s\e[m%s\n", "-n\e[1m, \e[1m--naked", "Initialize project without configuration");
    printf("\n\e[1m   %-24s\e[m%s\n", "-m\e[1m, \e[1m--make", "Initialize project with GNU Make backend");
    return 0;
  buildhelp:
    printf("usage: %s build [OPTION]... [ARG]\nBuild project.\n", argv[0]);
    printf("\nOptions:\n");
    printf("\n\e[1m   %-24s\e[m%s\n", "-r\e[1m, \e[1m--run", "After building, run the project.");
    printf("\n\e[1m   %-24s\e[m%s\n", "-d\e[1m, \e[1m--debug", "Debug configuration for the build");
    printf("\n\e[1m   %-24s\e[m%s\n", "-c\e[1m, \e[1m--clean", "Build clean (remove precompiled objects).");
    return 0;
  updatehelp:
    printf("usage: %s update [OPTION]...\nUpdate project.\n", argv[0]);
    printf("\nOptions:\n");
    printf("\n\e[1m   %-24s\e[m%s\n", "-p\e[1m, \e[1m--pull", "Only pull imports");
    printf("\n\e[1m   %-24s\e[m%s\n", "-m\e[1m, \e[1m--make", "Update Makefile");
    return 0;
}

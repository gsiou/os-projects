#include "utils.h"
#include "types.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

typedef enum {RCHAR, WCHAR, SYSR, SYSW, RFS, WFS} FIELD;
typedef enum {TCP, UDP, ALL_PROTOCOLS} PROTOCOL;
typedef long int stat_t;

struct proc_io_stat {
    stat_t rchar;
    stat_t wchar;
    stat_t sysr;
    stat_t sysw;
    stat_t rfs;
    stat_t wfs;
    stat_t key;
    int pid;
};

/* Lists all processes created in current tty 
   and their stats */
void process_snapshot();

/* Lists info about all open files by processes created
   in current tty */
void process_files();

/* Lists IO info about k top processes sorrted by field */
void process_iostat(int k, FIELD f);

/* Lists socket info about user's processes */
void process_net(PROTOCOL p);

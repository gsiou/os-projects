#include "utils.h"
#include "types.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>

#define REGULAR 1
#define DIRECTORY 2
#define CHAR_DVC 4
#define NAMED_PIPE 8
#define LINK 16
#define SOCKET 32
#define BLOCK_DVC 64
#define ALL 1|2|4|8|16|32|64

#define ACCESSTIME 0
#define MODTIME 1

struct FileStats{
    int regular;
    int directory;
    int char_dvc;
    int named_pipe;
    int link;
    int socket;
    int block_dvc;
};

/* Counts different file types and stores count to
   fst struct by searching directories recursively */
void recursive_count(struct FileStats *fst, char *path);

/* Prints a FileStats struct with proper form */
void print_filestats(struct FileStats fst);

/* Prints all type files in path that have beed access for action after time_bound */
void search_by_time(char *path, int types, int action, time_t time_bound);

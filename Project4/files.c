#include "files.h"

void recursive_count(struct FileStats *fst, char *path){
    struct stat buf;
    stat(path, &buf);
    if(S_ISDIR(buf.st_mode)){
        fst->directory++;
        DIR *dp;
        struct dirent *de;
        dp = opendir(path);
        if(dp == NULL) {
            perror("Directory open failed.");
        }
        else{
            while((de = readdir(dp))){
                if(!streq(de->d_name, ".") && !streq(de->d_name, "..")){
                    char buffer_path[MAX_PATH];
                    sprintf(buffer_path, "%s/%s",path, de->d_name);
                    recursive_count(fst, buffer_path);
                }
            }
            closedir(dp);
        }
    }
    else if(S_ISREG(buf.st_mode)){
        fst->regular++;
    }
    else if(S_ISCHR(buf.st_mode)){
        fst->char_dvc++;
    }
    else if(S_ISBLK(buf.st_mode)){
        fst->block_dvc++;
    }
    else if(S_ISFIFO(buf.st_mode)){
        fst->named_pipe++;
    }
    else if(S_ISLNK(buf.st_mode)){
        fst->link++;
    }
    else if(S_ISSOCK(buf.st_mode)){
        fst->socket++;
    }
    else{
        printf("File type could not be determined! \n");
    }
}

void search_by_time(char *path, int types, int action, time_t time_bound){
    struct stat buf;
    stat(path, &buf);
    
    /* Check time */
    int prnt = 0; /* if is set print file/dir */
    time_t cmp;
    if(action == ACCESSTIME){
        cmp = buf.st_atime;
    }
    else{
        cmp = buf.st_mtime;
    }
    if(difftime(cmp, time_bound) > 0){
        prnt = 1;
    }
    if(S_ISDIR(buf.st_mode)){
        if((types & DIRECTORY) && prnt) printf("%s\n", path);
        DIR *dp;
        struct dirent *de;
        dp = opendir(path);
        if(dp == NULL) {
            perror("Directory open failed.");
        }
        else{
            while((de = readdir(dp))){
                if(!streq(de->d_name, ".") && !streq(de->d_name, "..")){
                    char buffer_path[MAX_PATH];
                    sprintf(buffer_path, "%s/%s",path, de->d_name);
                    search_by_time(buffer_path, types, action, time_bound);
                }
            }
            closedir(dp);
        }
    }
    else if(S_ISREG(buf.st_mode) && ((types & REGULAR) == REGULAR) && prnt){
        printf("%s\n", path);
    }
    else if(S_ISCHR(buf.st_mode) && ((types & CHAR_DVC) == CHAR_DVC) && prnt){
        printf("%s\n", path);
    }
    else if(S_ISBLK(buf.st_mode) && ((types & BLOCK_DVC) == BLOCK_DVC) && prnt){
        printf("%s\n", path);
    }
    else if(S_ISFIFO(buf.st_mode) && ((types & NAMED_PIPE) == NAMED_PIPE) && prnt){
        printf("%s\n", path);
    }
    else if(S_ISLNK(buf.st_mode) && ((types & LINK) == LINK) && prnt){
        printf("%s\n", path);
    }
    else if(S_ISSOCK(buf.st_mode) && ((types & SOCKET) == SOCKET) && prnt){
        printf("%s\n", path);
    }
}

void print_filestats(struct FileStats fst){
    printf("number of regular files:      %d\n", fst.regular);
    printf("number of directories:        %d\n", fst.directory);
    printf("number of symbolic links:     %d\n", fst.link);
    printf("number of named pipes:        %d\n", fst.named_pipe);
    printf("number of char devices:       %d\n", fst.char_dvc);
    printf("number of sockets:            %d\n", fst.socket);
    printf("number of block device files: %d\n", fst.block_dvc);
}

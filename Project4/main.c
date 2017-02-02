#include <stdio.h>
#include "types.h"
#include "files.h"
#include "proc.h"
#define MAX_PATH 200

int main(int argc, char *argv[]){
    if(argc <= 1){
        printf("Wrong args\n");
        return -1;
    }
    if(streq(argv[1], "-cnt")){
        if(argc == 3){
            char buffer[MAX_PATH];
            struct passwd *pw = getpwuid(getuid());
            const char *homedir = pw->pw_dir;
            sprintf(buffer, "%s/%s",homedir, argv[2]);
            //printf("%s\n", buffer);
            struct FileStats fst = {0, 0, 0, 0, 0, 0, 0};
            recursive_count(&fst, buffer);
            print_filestats(fst);
            return 0;
        }
        else printf("Specify path to directory\n");
    }
    else if(streq(argv[1], "-type")){
        if(argc != 7){
            printf("Give type, read or write access and time\n");
            return -1;
        }
        int action;
        time_t time_bound;
        int time_n;
        char time_c;
        int types = 0;
        int i=0;
        while(argv[2][i] != '\0'){
            switch(argv[2][i++]){
            case 'f':
                types = types | REGULAR;
                break;
            case 'd':
                types = types | DIRECTORY;
                break;
            case 'l':
                types = types | LINK;
                break;
            case 'p':
                types = types | NAMED_PIPE;
                break;
            case 'c':
                types = types | CHAR_DVC;
                break;
            case 'b':
                types = types | BLOCK_DVC;
                break;
            case 's':
                types = types | SOCKET;
                break;
            }
        }
        if(streq(argv[3], "-lr")){
            action = ACCESSTIME;
        }
        else if(streq(argv[3], "-lw")){
            action = MODTIME;
        }
        else{
            printf("3rd parameter can only be -lr or -lw \n");
            return -1;
        }
        sscanf(argv[5], "%d%c\n", &time_n, &time_c);
        if(time_c == 's') time_bound = time_n;
        else if(time_c == 'm') time_bound = time_n * 60;
        else if(time_c == 'h') time_bound = time_n * 60 * 60;
        else if(time_c == 'd') time_bound = time_n * 24 * 60 * 60;
        else{
            printf("Invalid time. \n");
        }
        time_bound = time(NULL) - time_bound;
        search_by_time(argv[6], types, action, time_bound);
    }
    else if(streq(argv[1], "-ps")){
        process_snapshot();
        return 0;
    }
    else if(streq(argv[1], "-ft")){
        process_files();
        return 0;
    }
    else if(streq(argv[1], "-iostat")){
        if(argc != 6){
            printf("Specify -k and -f. \n");
            return -1;
        }
        else{
            int k;
            char f[200];
            if(streq(argv[2], "-k") && streq(argv[4], "-f")){
                k = atoi(argv[3]);
                strcpy(f, argv[5]);
            }
            else if(streq(argv[2], "-f") && streq(argv[4], "-k")){
                k = atoi(argv[5]);
                strcpy(f, argv[3]);
            }
            else{
                printf("Specify -k and -f. \n");
                return -1;
            }
            FIELD myf;
            if(streq(f, "RCHAR")) myf = RCHAR;
            else if(streq(f, "WCHAR")) myf = WCHAR;
            else if(streq(f, "SYSR")) myf = SYSR;
            else if(streq(f, "SYSW")) myf = SYSW;
            else if(streq(f, "RFS")) myf = RFS;
            else if(streq(f, "WFS")) myf = WFS;
            else{
                printf("Invalid field. \n");
                return -1;
            }

            process_iostat(k, myf);
            return 0;
        }
    }
    else if(streq(argv[1], "-netstat")){
        if(argc == 3){
            if(streq(argv[2], "tcp") || streq(argv[2], "TCP")){
                process_net(TCP);
            }
            else if(streq(argv[2], "udp") || streq(argv[2], "UDP")){
                process_net(UDP);
            }
            else{
                printf("Uknown protocol.\n");
                return -1;
            }
        }
        else{
            process_net(ALL_PROTOCOLS);
        }
    }
    else{
        printf("Invalid option. \n");
        return -1;
    }
}

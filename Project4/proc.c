#include "proc.h"

void process_snapshot(){
    pid_t tty_id = getppid(); /* Parent id is tty id */
    int max_id; /* Max process id */
    int pid, ppid, c;
    unsigned long vsz, total_mem;
    unsigned long long start;
    time_t real_start;
    long int rss, priority;
    long int utime, stime, cutime, cstime;
    float cpu, total_cpu;
    char state;
    char cmdline[500];
    char timestr[10];
    FILE *f,*f2;
    int i, j;
    char buffer[200];
    struct sysinfo info;
    total_cpu = 0;
    total_mem = 0;
    /* Get max_id */
    f = fopen("/proc/sys/kernel/pid_max", "r");
    fscanf(f, "%d", &max_id);
    fclose(f);
    
    printf("PID\tPPID\tCPU\tSTATE\tSTART\tVSZ\tRSS\tPRIORITY\tCMDLINE\n");

    /* We want all processes with parent id = tty_id */
    for(i = 0; i <= max_id; i++){
        sprintf(buffer, "/proc/%d/stat", i);
        if(access(buffer, F_OK) != -1){
            f = fopen(buffer, "r");
            fscanf(f, "%d %*s %c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u " 
                   "%lu %lu %ld %ld "
                   "%ld %*d %*d %*d %llu %lu %ld",
                   &pid, &state, &ppid, &utime, &stime, &cutime, &cstime,
                   &priority, &start ,&vsz, &rss);
            if(ppid == tty_id || pid == tty_id){
                /* Process spawned by our tty */
                cpu = (utime + stime + cutime + cstime) * 1.0  / sysconf(_SC_CLK_TCK);
                cpu = cpu / 60;
                total_cpu += cpu;
                total_mem += vsz;
                start = start / sysconf(_SC_CLK_TCK);
                sysinfo(&info);
                real_start = time(NULL) - info.uptime + start;
                sprintf(buffer, "/proc/%d/cmdline", pid);
                f2 = fopen(buffer, "r");
                j = 0;
                while((c = fgetc(f2)) != EOF){
                    if(c == '\0'){
                        cmdline[j++] = ' ';
                    }
                    else{
                        cmdline[j++] = c;
                    }
                }
                cmdline[j] = '\0';
                strftime(timestr, 10, "%R", localtime(&real_start));
                printf("%d\t%d\t%.3fs\t%c\t%s\t%.0f\t%ld\t%ld\t\t%s\n", 
                       pid, ppid, cpu, state, timestr, 
                       vsz/1024.0, rss, priority, cmdline);
            }
            fclose(f);
        }
    }
    printf("--------------------\n");
    printf("Total cpu time: %.3fs\n", total_cpu);
    printf("Total memory KBs: %.0f\n", total_mem/1024.0);
}

void process_files(){
    pid_t tty_id = getppid();
    int max_pid;
    FILE *f;

    /* Get max_id */
    f = fopen("/proc/sys/kernel/pid_max", "r");
    fscanf(f, "%d", &max_pid);
    fclose(f);

    struct stat stat_buf;
    struct dirent *de;
    struct passwd *user_info;
    struct group *group_info;
    DIR *dp;
    char path_buf1[200], path_buf2[200], buf[200], perms[11];
    char username[200], group[200];
    int i, ppid, chars;
    for(i = 0;i <= max_pid; i++){       
        sprintf(path_buf1, "/proc/%d", i); 
        sprintf(path_buf2, "%s/stat", path_buf1);
        f = fopen(path_buf2, "r");
        if(f != NULL){
            /* process exists */
            fscanf(f, "%*d %*s %*c %d", &ppid);
            fclose(f);
            if(ppid == tty_id){
                /* process created in our tty */
                printf("PID: %d \n", i);
                sprintf(path_buf2, "%s/fd", path_buf1);
                dp = opendir(path_buf2);
                if(dp == NULL) continue;
                while((de = readdir(dp))){
                    if(streq(de->d_name, ".") || streq(de->d_name, "..")){
                        continue;
                    }
                    sprintf(path_buf2, "%s/fd/%s", path_buf1, de->d_name);
                    chars = readlink(path_buf2, buf, 200);
                    buf[chars] = '\0';
                    stat(buf, &stat_buf);
                    user_info = getpwuid(stat_buf.st_uid);
                    group_info = getgrgid(stat_buf.st_gid);
                    strcpy(username, user_info->pw_name);
                    strcpy(group, group_info->gr_name);
                    perms[0] = S_ISDIR(stat_buf.st_mode) ? 'd' : '-';
                    perms[1] = stat_buf.st_mode & S_IRUSR ? 'r' : '-';
                    perms[2] = stat_buf.st_mode & S_IWUSR ? 'w' : '-';
                    perms[3] = stat_buf.st_mode & S_IXUSR ? 'x' : '-';
                    perms[4] = stat_buf.st_mode & S_IRGRP ? 'r' : '-';
                    perms[5] = stat_buf.st_mode & S_IWGRP ? 'w' : '-';
                    perms[6] = stat_buf.st_mode & S_IXGRP ? 'x' : '-';
                    perms[7] = stat_buf.st_mode & S_IROTH ? 'r' : '-';
                    perms[8] = stat_buf.st_mode & S_IWOTH ? 'w' : '-';
                    perms[9] = stat_buf.st_mode & S_IXOTH ? 'x' : '-';
                    perms[10] = '\0';
                    printf("%s ", perms);
                    printf("%d ", stat_buf.st_nlink);
                    printf("%s ", username);
                    printf("%s ", group);
                    printf("%ld ", stat_buf.st_size);
                    printf("%s\n", buf);
                }
                printf("\n");
            }
        }
    }
}

int compare(const void *p1, const void *p2){
    struct proc_io_stat *a = (struct proc_io_stat *) p1;
    struct proc_io_stat *b = (struct proc_io_stat *) p2;
    return (b->key - a->key);
}

void process_iostat(int k, FIELD field){
    int max_pid, i, arr_size, arr_cap;
    uid_t myuid;
    FILE *f;
    char strpath[200];
    struct stat buf;
    struct proc_io_stat *statarr;
    struct proc_io_stat tempstat;

    /* First we have to get the processes that belong to user */
    myuid = getuid();

    /* Get max_id */
    f = fopen("/proc/sys/kernel/pid_max", "r");
    fscanf(f, "%d", &max_pid);
    fclose(f);

    /* Create an array to store stats */
    arr_size = 0;
    arr_cap = 100;
    statarr = malloc(arr_cap * sizeof(struct proc_io_stat));

    printf("PID\tRCHAR\t  WCHAR\t    SYSR      SYSW     RFS      WFS\n");

    /* Search all processes and find the ones that belong to us */
    for(i = 0;i <= max_pid; i++){

        /* Check if process exists */
        sprintf(strpath, "/proc/%d/io", i);
        if(access(strpath, F_OK) != -1){

            /* Check if we are the owner of that process */
            stat(strpath, &buf);
            if(buf.st_uid == myuid){
                
                /* Gather data */
                f = fopen(strpath, "r");
                fscanf(f, "%*s %ld", &(tempstat.rchar));
                fscanf(f, "%*s %ld", &(tempstat.wchar));
                fscanf(f, "%*s %ld", &(tempstat.sysr));
                fscanf(f, "%*s %ld", &(tempstat.sysw));
                fscanf(f, "%*s %ld", &(tempstat.rfs));
                fscanf(f, "%*s %ld", &(tempstat.wfs));
                tempstat.pid = i;
                fclose(f);
                switch(field){
                case RCHAR:
                    tempstat.key = tempstat.rchar;
                    break;
                case WCHAR:
                    tempstat.key = tempstat.wchar;
                    break;
                case SYSR:
                    tempstat.key = tempstat.sysr;
                    break;
                case SYSW:
                    tempstat.key = tempstat.sysw;
                    break;
                case RFS:
                    tempstat.key = tempstat.rfs;
                    break;
                case WFS:
                    tempstat.key = tempstat.wfs;
                    break;
                }
                
                /* Resize the array if needed */
                if(arr_size == arr_cap){
                    arr_cap += arr_cap;
                    statarr = realloc(statarr, arr_cap * sizeof(struct proc_io_stat));
                }

                /* Copy temp struct to array */
                memcpy(&(statarr[arr_size]), &tempstat, sizeof(struct proc_io_stat));
                arr_size++;
            }
        }
    }

    /* Sort array and print only top k */
    qsort(statarr, arr_size, sizeof(struct proc_io_stat), compare);
    for(i = 0; i < arr_size && i < k; i++){
        printf("%-7d %-9ld %-9ld %-9ld %-8ld %-8ld %-8ld\n",
               statarr[i].pid,
               statarr[i].rchar, statarr[i].wchar,
               statarr[i].sysr, statarr[i].sysw,
               statarr[i].rfs, statarr[i].wfs);
    }
}


void process_net(PROTOCOL p){
    unsigned int ip1, ip2, ip3, ip4, port;
    int max_pid, i, sockid, netid, read;
    ssize_t chars;
    uid_t myuid;
    FILE *f;
    char strpath[200], strpath2[200], buf[200];
    char local_adr[100], local_port[100], rem_adr[100], rem_port[100];
    char local[100], rem[100];
    char line[1024];
    char tcp_path[] = "/proc/net/tcp";
    char udp_path[] = "/proc/net/udp";
    struct stat stat_buf;
    struct dirent *de;
    DIR *dp;
    BOOL tcp, udp;

    if(p == ALL_PROTOCOLS){
        tcp = udp = TRUE;
    }
    else if(p == TCP){
        tcp = TRUE;
        udp = FALSE;
    }
    else{
        tcp = FALSE;
        udp = TRUE;
    }

    /* First we have to get the processes that belong to user */
    myuid = getuid();

    /* Get max_id */
    f = fopen("/proc/sys/kernel/pid_max", "r");
    fscanf(f, "%d", &max_pid);
    fclose(f);

    printf("PROTOCOL\tPID\tL-ADDRESS\tL-PORT\tR-ADDRESS\tR-PORT\n");

    /* Search all processes and find the ones that belong to us */
    for(i = 0;i <= max_pid; i++){

        /* Check if process exists */
        sprintf(strpath, "/proc/%d/fd", i);
        if(access(strpath, F_OK) != -1){

            /* Check if we are the owner of that process */
            stat(strpath, &stat_buf);
            if(stat_buf.st_uid == myuid){
                /* Search all open files for sockets */
                dp = opendir(strpath);
                if(dp == NULL) continue;
                while((de = readdir(dp))){
                    if(streq(de->d_name, ".") || streq(de->d_name, "..")){
                        continue;
                    }
                    sprintf(strpath2, "%s/%s", strpath, de->d_name);
                    chars = readlink(strpath2, buf, 200);
                    buf[chars] = '\0';
                    if(strncmp(buf, "socket:", strlen("socket:")) == 0){
                        //printf("%s \n", buf);
                        sscanf(buf, "socket:[%d]", &sockid);
                        if(tcp){
                            f = fopen(tcp_path, "r");
                            
                            /* Skip first line */
                            fgets(line, 1024, f);
                            
                            /* Read what we want */
                            while(!feof(f)){
                                fgets(line, 1024, f);
                                sscanf(line, " %*s %s %s %*s %*s %*s %*s %*s %*s %d",
                                       local, rem, &netid);
                                //printf("%s |%s| %d \n", local, rem, netid);
                                sscanf(local, "%2x%2x%2x%2x:%4x",
                                       &ip1, &ip2, &ip3, &ip4, &port);
                                sprintf(local_adr, "%u.%u.%u.%u", ip4, ip3, ip2, ip1);
                                sprintf(local_port, "%u", port);
                                sscanf(rem, "%2x%2x%2x%2x:%4x",
                                       &ip1, &ip2, &ip3, &ip4, &port);
                                sprintf(rem_adr, "%u.%u.%u.%u", ip4, ip3, ip2, ip1);
                                sprintf(rem_port, "%u", port);
                                if(netid == sockid){
                                    printf("TCP       \t%d\t%-9s\t%-6s\t%-9s\t%-6s \n",
                                           i, local_adr, local_port, rem_adr, rem_port);
                                }
                            }
                            fclose(f);
                        }
                        if(udp){
                            f = fopen(udp_path, "r");
                            
                            /* Skip first line */
                            fgets(line, 1024, f);
                            
                            /* Read what we want */
                            while(!feof(f)){
                                fgets(line, 1024, f);
                                sscanf(line, " %*s %s %s %*s %*s %*s %*s %*s %*s %d",
                                       local, rem, &netid);
                                //printf("%s |%s| %d \n", local, rem, netid);
                                sscanf(local, "%2x%2x%2x%2x:%4x",
                                       &ip1, &ip2, &ip3, &ip4, &port);
                                sprintf(local_adr, "%u.%u.%u.%u", ip4, ip3, ip2, ip1);
                                sprintf(local_port, "%u", port);
                                sscanf(rem, "%2x%2x%2x%2x:%4x",
                                       &ip1, &ip2, &ip3, &ip4, &port);
                                sprintf(rem_adr, "%u.%u.%u.%u", ip4, ip3, ip2, ip1);
                                sprintf(rem_port, "%u", port);
                                if(netid == sockid){
                                    printf("UDP       \t%d\t%-9s\t%-6s\t%-9s\t%-6s \n",
                                           i, local_adr, local_port, rem_adr, rem_port);
                                }
                            }
                            fclose(f);
                        }
                    }
                }
            }
        }
    }
}

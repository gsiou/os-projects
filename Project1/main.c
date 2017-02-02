#include "twa.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILENAME_SIZE 200
#define STRING_SIZE 40
#define CMD_SIZE 500

/* Helper functions */
BOOL str_equal(char *s1, char *s2);
void print_usage(char *program_name);
BOOL parse_cmd(Twa *t, char *cmd);

int main(int argc, char *argv[]){
    int current = 1;
    int args_left = argc - 1;
    BOOL has_b = FALSE;
    BOOL has_f = FALSE;
    if(argc < 5){
	/* 1 is the name, other two pairs are for b and f flags */
	printf("Insufficient parameters. \n");
	print_usage(argv[0]);
	return 1;
    }

    char  LoadFileName[FILENAME_SIZE] = "";
    char  OperationsFileName[FILENAME_SIZE] = "";
    char  ConfigFileName[FILENAME_SIZE] = "";
    int   BucketEntries = 0;
    float LoadFactor = 0;
    
    /* Read args in pairs */
    while(args_left >= 2){
        if(str_equal(argv[current], "-l")){
	    current++;
	    args_left--;
	    strcpy(LoadFileName, argv[current]);
	}
	else if(str_equal(argv[current], "-b")){
	    current++;
	    args_left--;
	    BucketEntries = (int) strtol(argv[current], NULL, 10);
	}
	else if(str_equal(argv[current], "-f")){
	    current++;
	    args_left--;
	    LoadFactor = (float) strtod(argv[current], NULL);
	}
	else if(str_equal(argv[current], "-p")){
	    current++;
	    args_left--;
	    strcpy(OperationsFileName, argv[current]);
	}
	else if(str_equal(argv[current], "-c")){
	    current++;
	    args_left--;
	    strcpy(ConfigFileName, argv[current]);
	}
	else{
	    printf("%s is not a correct option. \n", argv[current]);
	    print_usage(argv[0]);
	    return 1;
	}
	current++;
	args_left--;
    }

    if(!BucketEntries || !LoadFactor){
	print_usage(argv[0]);
	return 1;
    }

    if(LoadFactor < 0 || LoadFactor > 1){
	printf("Error: Load factor has to be between 0 and 1 \n");
	print_usage(argv[0]);
	return 1;
    }

    if(BucketEntries < 1){
	printf("Error: Bucket entries have to be at least 1 \n");
	print_usage(argv[0]);
	return 1;
    }

    /* Load data in twa */
    Twa *t = Twa_create(BucketEntries, LoadFactor);
    if(!str_equal(LoadFileName, "")){
	int n;
	n = Twa_load(t, LoadFileName);
	printf("%d records inserted\n\n", n);
    }

    /* Read Operations File */
    if(!str_equal(OperationsFileName, "")){
	FILE *f;
	char opfile_cmd[CMD_SIZE];
	f = fopen(OperationsFileName, "r");
	if(f != NULL){
	    while(fgets(opfile_cmd, 500, f)){
		parse_cmd(t, opfile_cmd);
	    }
	    fclose(f);
	}
	else{
	    printf("Error reading operations file. \n");
	}
    }
    
    /* Command line prompt */
    BOOL cmd = TRUE;
    char cmd_buffer[CMD_SIZE];
    while(cmd){
	printf("command: ");
	fgets(cmd_buffer, 500, stdin);
	cmd = parse_cmd(t, cmd_buffer);
    }

    /* Free twa memory */
    Twa_destroy(t);
}

BOOL str_equal(char *s1, char *s2){
    if(strcmp(s1,s2) == 0) return 1;
    else return 0;
}

void print_usage(char *program_name){
    printf("Usage: %s -l DataFile -b BucketEntries -f LoadFactor -p OperationsFile -c config-file \n", program_name);
    printf("-b and -f flags are required, the others are optional \n");
}

BOOL parse_cmd(Twa *t, char *cmd){
    /* We only have one letter commands so 
     * if second char is not a space/newline we
     * must throw an error message and return 
     * except for when the whole command is just 
     * the newline. */
    if((cmd[0] != '\n') && (cmd[1] != ' ' && cmd[1] != '\n')){
	printf("Command not found \n");
	return TRUE;
    }

    BOOL cont = TRUE;
    
    /* First char is the command */
    switch(cmd[0]){
    case 'e':
	cont = FALSE;
	break;
    case 'q':
    { /* Block required to define variables */
	long custid;
	char temp;
	sscanf(cmd, "%c %ld", &temp, &custid);
	Twa_query(t, custid);
	printf("\n");
	break;
    }
    case 'l':
    {
	char FilePath[FILENAME_SIZE];
	char temp;
	int n;
	sscanf(cmd, "%c %s", &temp, FilePath);
	n = Twa_load(t, FilePath);
	printf("%d records inserted\n\n", n);
	break;
    }
    case 'i':
    {
	long custid = -1;
	char name[STRING_SIZE] = "";
	char surname[STRING_SIZE] = "";
	char street[STRING_SIZE] = "";
	char postal[STRING_SIZE] = "";
	char city[STRING_SIZE] = "";
	float money = -1;
	int number = -1;
	char temp;
	int res;
	sscanf(cmd, "%c %ld %s %s %s %d %s %s %f", &temp,
	       &custid, surname, name, street, &number, postal, city, &money);
	res = Twa_insert(t, custid, name, surname, street, number, city, postal, money);
	if(res == 0)
	    printf("Insufficient data or wrong format. \n\n");
	else if(res == 1)
	    printf("Record inserted \n\n");
	else
	    printf("Record updated \n\n");
	break;
    }
    case 't':
    {
	int k;
	char temp;
	sscanf(cmd, "%c %d", &temp, &k);
	Twa_top(t, k);
	printf("\n");
	break;
    }
    case 'b':
    {
	int k;
	char temp;
	sscanf(cmd, "%c %d", &temp, &k);
	Twa_bottom(t, k);
	printf("\n");
	break;
    }
    case 'a':
	printf("%f \n", Twa_average(t));
	printf("\n");
	break;
    case 'r':
    {
	long custid1, custid2;
	custid1 = custid2 = -1;
	char temp;
	sscanf(cmd, "%c %ld %ld", &temp, &custid1, &custid2);
	Twa_range(t, custid1, custid2);
	printf("\n");
	break;
    }
    case 'p':
    {
	long custid = -1;
	char temp;
	sscanf(cmd, "%c %ld", &temp, &custid);
	float p;
	p = Twa_percentile(t, custid);
	if(p != -1){
	    printf("%.f %%\n\n", p);
	}
	else{
	    printf("Customer with custid %ld does not exist!\n\n", custid);
	}
	break;
    }
    case '\n':
	break;
    default:
	printf("Command %c not found \n", cmd[0]);
	printf("\n");
	break;
    }
    return cont;
}

#include "utils.h"

int streq(char *str1, char *str2){
    if(strcmp(str1, str2) == 0){
	return 1;
    }
    else{
	return 0;
    }
}

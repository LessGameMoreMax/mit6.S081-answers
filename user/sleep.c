#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
    if(argc == 1){
        fprintf(2, "Usage: sleep time...\n");
        exit(1);
    }
    
    int t = atoi(argv[1]);
    if(t < 0){
        fprintf(2, "sleep time is error\n");
        exit(1);
    }
    sleep(t);
    exit(0);
}

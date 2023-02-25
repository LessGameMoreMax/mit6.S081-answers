#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
    if(argc > 1){
        fprintf(2, "Usage: uptime\n");
        exit(1);
    }

    int t = uptime();
    t = t/10;
    printf("Have used %d seconds\n", t);
    exit(0);
}

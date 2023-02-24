#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
    int pipefd_one[2];
    pipe(pipefd_one);

    int pipefd_two[2];
    pipe(pipefd_two);

    char byte_one = {'H'};
    int rc = fork();
    if(rc < 0){
        fprintf(2, "fork fail\n");
        exit(1);
    }else if(rc == 0){
        char bytec;
        close(0);
        dup(pipefd_one[0]);
        close(pipefd_one[0]);
        read(0, &bytec, 1);
        while(bytec != 'H');
        fprintf(1, "%d: received ping\n", getpid());
        close(1);
        dup(pipefd_two[1]);
        close(pipefd_two[1]);
        write(1, &bytec, 1);
    }else{
        int soc = dup(1);
        close(1);
        dup(pipefd_one[1]);
        close(pipefd_one[1]);
        write(1, &byte_one, 1);
        wait(0);
        char bytep;
        close(0);
        dup(pipefd_two[0]);
        close(pipefd_two[0]);
        read(0, &bytep, 1);
        fprintf(soc, "%d: received pong\n", getpid());
        fprintf(soc, "char is : %c\n", bytep);
    }
    exit(0);
}

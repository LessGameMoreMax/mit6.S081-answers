#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
    int p = 2;
    int n = p;
    int im = 1;
    int hf = 0;
    int pipefd[2];
    printf("prime %d\n", p);
    while(++n != 35){
       if(im == 1){
           if(n % p != 0){
               if(hf == 0){
                   pipe(pipefd);
                   int rc = fork();
                   if(rc < 0){
                       fprintf(2, "fork fail\n");
                       exit(1);
                   }else if(rc == 0){
                       im = 0;
                       continue;
                   }else{
                       hf = 1;
                   }
               }
               write(pipefd[1], &n, 4);
           }
       }else{
           close(0);
           dup(pipefd[0]);
           close(pipefd[0]);
           close(pipefd[1]);
           read(0, &p, 4); 
           printf("prime %d\n", p);
           while(1){
               if(read(0, &n, 4) == 0){
                   close(pipefd[1]);
                   wait(0);
                   exit(0);
               }
               if(n % p != 0){
                   if(hf == 0){
                       pipe(pipefd);
                       int rc = fork();
                       if(rc < 0){
                           fprintf(2, "fork fail\n");
                           exit(1);
                       }else if(rc == 0){
                           break;
                       }else{
                           hf = 1;
                       }
                   }
                   write(pipefd[1], &n, 4);
               }
           }
       }
    }
    close(pipefd[1]);
    wait(0);
    exit(0);
}

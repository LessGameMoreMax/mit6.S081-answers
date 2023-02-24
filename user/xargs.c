#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int
parseargc(char **argv, char *buf, int n){
    int argc = 0;
    for(int i = 0;i != n; ++i){
        switch(buf[i]){
            case ' ':
            case '"':
            case '\n':
            case '\t':
            case '\b':
                buf[i] = 0;
                break;
            case '\\':
                if(i != n-1 && buf[i+1] == 'n'){
                    buf[i] = 0;
                    buf[++i] = 0;
                }
                break;
            default:
                if(i == 0 || buf[i - 1] == 0){
                    argv[argc] = buf + i;
                    ++argc;
                }
        }
    }
    return argc;
}

int
main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(2, "Usage: command | xargs command");
        exit(1);
    }

    char buf[512]; 
    char *argv1[256];

    int r = read(0, buf, 512);
    int n = r;
    while(r != 0){
        r = read(0, buf + n, 512);
        n += r;
    }
    if(n < 0){
        fprintf(2, "Pipe has wrong");
        exit(1);
    }

    int argc1 = parseargc(argv1, buf, n);
    int e = argc1;
    if(strcmp(argv[1], "-n") == 0){
        e = atoi(argv[2]);
        argv = &argv[3];
        argc -= 3;
    }else{
        argv = &argv[1];
        argc -= 1;
    }

    int t = 0;
    if(argc1 != 0){
        t = argc1 / e;
    }else{
        e = 0;
    }
    if(argc1 % e != 0)
        ++t;
    //debug-----
           /* printf("e: %d\n", e); */
           /* printf("t: %d\n", t); */
           /* printf("argc1: %d\n", argc1); */
    //debug-----

    char *argv2[argc + e + 1];
    char **argv3 = argv1;
    for(int i = 0;i != t; ++i){
       for(int j = 0;j != argc; ++j){
           argv2[j] = argv[j];
       }
       if(argc1 % e != 0 && i == t-1){
           e = argc1 % e;
       }
       if(e + argc > MAXARG){
           fprintf(2, "too many param\n");
           exit(1);
       }
       for(int j = 0;j != e; ++j){
           *(argv2 + argc + j) = argv3[j];
       }
       argv3 += e;
       argv2[argc + e] = 0;
    //debug-----
           /* printf("%s\n", argv2[0]); */
           /* printf("%s\n", argv2[1]); */
           /* printf("%s\n", argv2[2]); */
           /* printf("%s\n", argv2[3]); */
           /* printf("e: %d\n", e); */
           /* printf("t: %d\n", t); */
           /* printf("argc1: %d\n", argc1); */
    //debug-----

       int rc = fork();
       if(rc < 0){
           fprintf(2, "fork fail\n");
           exit(1);
       }else if(rc == 0){
           //debug----
           /* printf("argc: %d\n", argc); */
           /* printf("e: %d\n", e); */
           /* printf("t: %d\n", t); */
           /* printf("argc1: %d\n", argc1); */
           /* printf("%s\n", argv2[0]); */
           /* printf("%s\n", argv2[1]); */
           /* printf("%s\n", argv2[2]); */
           /* printf("%s\n", argv2[3]); */
           /* sleep(1000); */
           //debug----
           exec(argv2[0], argv2);
           fprintf(2, "exec fail\n");
           exit(1);
       }else{
           wait(0);
       }
    }
    
    exit(0);
}

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *buf,char *filename, char *p){
    int fd;
    struct stat st;
    struct dirent de;

    if((fd = open(buf, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", buf);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", buf);
        close(fd);
        return;
    }

    switch(st.type){
        case T_FILE:
        case T_DEVICE:
            if(strcmp(p, filename) == 0){
                printf("%s\n", buf);
            }
        break;
        case T_DIR:
            if(strlen(buf) + 1 + DIRSIZ + 1 > 512){
                printf("find: path too long\n");
                break;
            }
            p += strlen(p);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0) continue;
                if(strcmp(de.name, ".") == 0
                        || strcmp(de.name, "..") == 0){
                    continue;
                }
                memmove(p, de.name, strlen(de.name) + 1);
                find(buf, filename, p);
            }
        break;
    }
    close(fd);
}

int
main(int argc, char *argv[]){
   if(argc < 3){
       fprintf(2, "Usages: find path filename...\n");
       exit(1);
   } 
   char buf[512];
   memmove(buf, argv[1], strlen(argv[1]) + 1);
   find(buf, argv[2], buf);
   exit(0);
}

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int match(char*,char*);

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
            if(match(filename, p)){
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

// Regexp matcher from Kernighan & Pike,
// The Practice of Programming, Chapter 9.

int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}



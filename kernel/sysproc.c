#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 va = 0;
  int pn = 0;
  uint64 vam = 0;
  int bytenum = 0;
  if(argaddr(0, &va) < 0)
      return -1;
  if(va >= MAXVA)
      panic("sys_pgaccess");
  if(argint(1, &pn) < 0)
      return -1;
  if(pn <= 0)
      panic("sys_pgaccess");
  va = PGROUNDDOWN(va);
  if((va + pn * PGSIZE) >= MAXVA)
      panic("sys_pgaccess");
  if(argaddr(2, &vam) < 0)
      return -1;
  pagetable_t userpgtb = myproc()->pagetable;
  if(walk(userpgtb, vam, 0) == 0)
      panic("sys_pgaccess");

  if((vam >= va && vam <= (va + pn * PGSIZE))
          || (vam + pn >= va && vam + pn <= (va + pn * PGSIZE)))
      panic("sys_pgaccess");

  bytenum = pn / 8;
  if(pn % 8 != 0)
      ++bytenum;
  char buff[bytenum];
  for(int i = 0;i != bytenum; ++i)
      buff[i] = 0;
  for(int i = 0;i != pn; ++i){
      pte_t *pte = walk(userpgtb, va + i * PGSIZE, 0);
      if(pte != 0){
        if((*pte) & PTE_A){
          buff[i/8] |= (1 << (i % 8));
          *pte &= ~PTE_A;
        }
      } 
  }
  return copyout(userpgtb, vam, buff, bytenum);
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

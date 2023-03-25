#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
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
  backtrace();
  return 0;
}

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

uint64
sys_sigreturn(void){
    struct proc *p = myproc();
    p->trapframe->epc = p->sigepc;
    p->trapframe->ra = p->ra; 
    p->trapframe->sp = p->sp; 
    p->trapframe->gp = p->gp; 
    p->trapframe->tp = p->tp; 
    p->trapframe->t0 = p->t0; 
    p->trapframe->t1 = p->t1; 
    p->trapframe->t2 = p->t2; 
    p->trapframe->s0 = p->s0; 
    p->trapframe->s1 = p->s1; 
    p->trapframe->a0 = p->a0; 
    p->trapframe->a1 = p->a1; 
    p->trapframe->a2 = p->a2; 
    p->trapframe->a3 = p->a3; 
    p->trapframe->a4 = p->a4; 
    p->trapframe->a5 = p->a5; 
    p->trapframe->a6 = p->a6; 
    p->trapframe->a7 = p->a7; 
    p->intick = 0;
    return 0;
}

uint64
sys_sigalarm(void){
    int ticks;
    void (*handler)();
    struct proc *p = myproc();
    if(p->intick) return 0;
    if(argint(0, &ticks) < 0)
        return -1;
    if(argaddr(1, (uint64*)&handler) < 0)
        return -1;
    if(ticks == 0 && handler == 0){
        p->ticks = 0;
        p->handler = 0;
        p->nticks = 0;
        p->sigepc = 0;
        p->intick = 0;
        p->ra = 0;
        p->sp = 0;
        p->gp = 0;
        p->tp = 0;
        p->t0 = 0;
        p->t1 = 0;
        p->t2 = 0;
        p->s0 = 0;
        p->s1 = 0;
        p->a0 = 0;
        p->a1 = 0;
        p->a2 = 0;
        p->a3 = 0;
        p->a4 = 0;
        p->a5 = 0;
        p->a6 = 0;
        p->a7 = 0;
        return 0;
    }
    p->ticks = ticks;
    p->handler = (uint64)handler;
    p->nticks = ticks;
    p->intick = 0;
    return 0;
}

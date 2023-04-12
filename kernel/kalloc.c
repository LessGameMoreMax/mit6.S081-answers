// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
struct run {
  struct run *next;
};

struct kmem_s{
  struct spinlock lock;
  struct run *freelist;
};

struct kmem_s kmem[NCPU];

void
new_kfree(void *pa, int cpuid)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[cpuid].lock);
  r->next = kmem[cpuid].freelist;
  kmem[cpuid].freelist = r;
  release(&kmem[cpuid].lock);
}

void
new_freerange(void *pa_start, void *pa_end)
{
  int percpa = ((uint64)pa_end - (uint64)pa_start) / (PGSIZE * NCPU);
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(int i = 0; i != NCPU; ++i)
      for(; p + PGSIZE <= (char*)pa_start + (i+1) * percpa; p += PGSIZE)
          new_kfree(p, i);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
      new_kfree(p, NCPU - 1);
}

void
kinit()
{
  for(int i = 0;i != NCPU; ++i)
    initlock(&kmem[i].lock, "kmem");
  new_freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
      kfree(p);
}
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  int cid = cpuid();
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[cid].lock);
  r->next = kmem[cid].freelist;
  kmem[cid].freelist = r;
  release(&kmem[cid].lock);
}

void *
steal_kalloc(){
    struct run *r;

    for(int i = 0;i != NCPU; ++i){
        acquire(&kmem[i].lock);
        r = kmem[i].freelist;
        if(r){
            kmem[i].freelist = r->next;
            release(&kmem[i].lock);
            return r;
        }
        release(&kmem[i].lock);
    }
    return r;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  int cid = cpuid();
  struct run *r;

  acquire(&kmem[cid].lock);
  r = kmem[cid].freelist;
  if(r)
    kmem[cid].freelist = r->next;
  release(&kmem[cid].lock);
  if(r == 0)
      r = steal_kalloc();
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKETNUM 10

struct buf bufs[NBUF];

struct bucket{
    struct spinlock lock;
    struct buf head;
};

struct bucket buckets[BUCKETNUM];

void
binit(void)
{
  struct buf *b;
  struct buf *temp = bufs;
  for(int i = 0;i != BUCKETNUM; ++i){
    initlock(&buckets[i].lock, "bucket");
    buckets[i].head.prev = &buckets[i].head;
    buckets[i].head.next = &buckets[i].head;
    for(b = temp; b != temp + 3; ++b){
      b->next = buckets[i].head.next;
      b->prev = &buckets[i].head;
      initsleeplock(&b->lock, "buffer");
      buckets[i].head.next->prev = b;
      buckets[i].head.next = b;
    }
    temp += 3;
  } 
}

static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int i = blockno % BUCKETNUM;
  acquire(&buckets[i].lock);

  uint less_ticks = buckets[i].head.next->ticks;
  struct buf *r = buckets[i].head.next;
  // Is the block already cached?
  for(b = buckets[i].head.next; b != &buckets[i].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->ticks = ticks;
      release(&buckets[i].lock);
      acquiresleep(&b->lock);
      return b;
    }
    if(b->ticks <= less_ticks){
        less_ticks = b->ticks;
        r = b;
    }
  }

  r->dev = dev;
  r->blockno = blockno;
  r->valid = 0;
  r->ticks = ticks;
  release(&buckets[i].lock);
  acquiresleep(&r->lock);
  return r;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
}

void
bpin(struct buf *b) {
  int i = b->blockno % BUCKETNUM;
  acquire(&buckets[i].lock);
  b->ticks = ticks;
  release(&buckets[i].lock);
}

void
bunpin(struct buf *b) {
  int i = b->blockno % BUCKETNUM;
  acquire(&buckets[i].lock);
  b->ticks = 0;
  release(&buckets[i].lock);
}



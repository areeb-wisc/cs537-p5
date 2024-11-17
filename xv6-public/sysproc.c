#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "wmap.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  dprintf("sys_sbrk()\n");
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
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

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_wmap(void)
{
  dprintf("sys_wmap()\n");
  int addr0, length, flags, fd;

  if(argint(0, &addr0) < 0)
    return -1;

  if(argint(1, &length) < 0)
    return -2;

  if(argint(2, &flags) < 0)
    return -3;

  if(argint(3, &fd) < 0)
    return -4;
  
  if (addr0 < 0)
    return -5;
  
  uint addr = addr0;

  if (addr < VA_START || addr >= VA_END || addr % PGSIZE)
    return -6;

  if (length <= 0 || length > VA_END - VA_START)
    return -7;
  
  if (flags <= 0 || flags&1 || flags > 16)
    return -8;
  
  if (!(flags & MAP_SHARED) || !(flags & MAP_FIXED))
    return -9;

  if (fd < 0 || fd >= NOFILE)
    return -10;

  return wmap(addr, length, flags, fd);
}

int
sys_wunmap(void)
{
  dprintf("sys_wunmap()\n");
  int addr0;
  if(argint(0, &addr0) < 0)
    return -1;
  
  if (addr0 < 0)
    return -1;

  uint addr = (uint)(addr0);
  
  return wunmap(addr);
}

int
sys_va2pa(void)
{
  dprintf("sys_va2pa()\n");
  int addr0;

  if(argint(0, &addr0) < 0)
    return -1;
  
  if (addr0 < 0)
    return -1;

  uint addr = (uint)(addr0);
  return va2pa(addr);
}

int
sys_getwmapinfo(void)
{
  dprintf("sys_getwmapinfo()\n");
  return 0;
}
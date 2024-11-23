#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

int count = 0;

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  case T_PGFLT:

    count++;
    uint addr = rcr2();
    dprintf(2,"PAGE fault for addr: 0x%x\n", addr);
    dprintf(2,"count = %d\n", count);

    /*
    if addr < VA_START:
      1. this means this is a simple malloced() region
      2. simple malloced() addresses are mapped greedily during allocuvm()
      2. hence, it must have been copied during copyuvm() with read-only permissions
      3. some process is trying to write to it (but it is read-only), hence we get page fault
      4. if it was writeable, we must have set COW bit
      5. so if cow bit is set, allocate a new page and map it to addr
      6. copy physical page's contents, and decrement refcount
      7. else proc is trying to write to an originally read-only page, kill it

    else:
      1. we are in wmap territory
      2. option1: segfault occured because of lazy mapping -> do_real_mapping()
      3. option2: same as above, this is happening because of copywmap()
      4. all wmap pages are always write-able, so process trying to write to read-only page is not a concern here
      5. if this is an ANONYMOUS wmap, we must have set COW bit
      6. so if COW bit is set, allocate a new page and map it to addr
      7. copy physical page's contents, decrement refcount
      8. else, this is a SHARED wmap, so just make this pte writeable in faulting process
      9. this will allow other processes who share this address to see these changes 

      do_copy_on_write(uint addr, int idx)
    */

    int success = -1;
    if (addr < VA_START) {
      dprintf(2, "non wmapped segfault for 0x%x\n", addr);
      success = do_copy_on_write(addr);
    } else {
      dprintf(2, "wmapped segfault for 0x%x\n", addr);
      int idx = lazily_mapped_index(addr);
      dprintf(2, "lazily_mapped_idx = %d\n", idx);
      if (idx != -1) {
        success = do_real_mapping(addr, idx);
      }
    }

    if (success == 0) {
      dprintf(2,"Page fault handled\n");
      return;
      // break;
    } else {
      dprintf(2,"Pagefault handling failed for addr: 0x%x\n", addr);
      cprintf("Segmentation Fault\n");
      myproc()->killed = 1;
      // break;
    }

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}

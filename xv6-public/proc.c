#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "wmap.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void show_lazy_mappings(struct proc* p) {
  cprintf("Lazy mappings:\n\n");
  cprintf("idx\taddr\t\tlength\tloaded\tfd\tflags\n");
  wmap_data wdata = p->wmapdata;
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    int addr = wdata.winfo.addr[i];
    if (addr > 0) {
      int length = wdata.winfo.length[i];
      int loaded = wdata.winfo.n_loaded_pages[i];
      int fd = wdata.fd[i];
      int flags = wdata.flags[i];
      cprintf("%d\t0x%x\t%d\t%d\t%d\t%d\n",i,addr,length,loaded,fd,flags);
    }
  }
  cprintf("\ntotal_mmaps = %d\n", wdata.winfo.total_mmaps);
}

int get_free_idx() {
  dprintf(4,"get_free_idx()\n");
  struct wmapinfo winfo = myproc()->wmapdata.winfo;
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    if (winfo.addr[i] == 0)
      return i;
  }
  return -1;
}

void free_lazy_idx(int idx) {
  dprintf(4, "free_lazy_idx()\n");
  struct proc* currproc = myproc();
  currproc->wmapdata.winfo.addr[idx] = 0;
  currproc->wmapdata.winfo.length[idx] = 0;
  currproc->wmapdata.winfo.n_loaded_pages[idx] = 0;
  currproc->wmapdata.fd[idx] = -1;
  currproc->wmapdata.flags[idx] = -1;
  currproc->wmapdata.winfo.total_mmaps--;
}

int fdalloc(struct file* f) {

  int fd;
  struct proc *curproc = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd] == 0){
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int dup(int fd) {

  struct proc* currproc = myproc();
  struct file *f = currproc->ofile[fd];
  int newfd = fdalloc(f);
  if (newfd < 0)
    return -1;
  filedup(f);
  return newfd;
}

int add_lazy_mapping(uint addr, int length, int fd, int flags) {
  dprintf(4,"add_lazy_mapping()\n");
  int idx = get_free_idx();
  dprintf(2,"idx = %d\n", idx);
  if (idx == -1) {
    dprintf(1,"More than %d wmaps\n", MAX_WMMAP_INFO);
    return -1;
  }
  struct proc* currproc = myproc();

  if (!(flags & MAP_ANONYMOUS))
    fd = dup(fd);
  
  currproc->wmapdata.winfo.addr[idx] = addr;
  currproc->wmapdata.winfo.length[idx] = length;
  currproc->wmapdata.winfo.n_loaded_pages[idx] = 0;
  currproc->wmapdata.fd[idx] = fd;
  currproc->wmapdata.flags[idx] = flags;
  currproc->wmapdata.winfo.total_mmaps++;

  // dprintf(4, "mappings right after adding new wmap at idx: %d\n", idx);
  // show_lazy_mappings();
  return 0;
}

int lazily_mapped_index(uint addr) {
  struct wmapinfo winfo = myproc()->wmapdata.winfo;
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    uint start = winfo.addr[i];
    if (start > 0) {
      uint end = start + winfo.length[i];
      if (addr >= start && addr < end)
        return i;
    }
  }
  return -1;
}

static pte_t* walkpgdir(pde_t *pgdir, const void *va, int alloc) {
  dprintf(4,"walkpgdir(alloc=%d)\n", alloc);
  dprintf(2,"*pgdir = 0x%x\n", *pgdir);
  dprintf(2,"va = 0x%x\n", va);
  pde_t *pde;
  pte_t *pgtab;

  dprintf(5,"using top 10 bits = %d of va = 0x%x to index into *pgdir at 0x%x\n", PDX(va), va, *pgdir);
  pde = &pgdir[PDX(va)];
  dprintf(5,"*pde before = 0x%x\n", *pde);
  if(*pde & PTE_P){
    dprintf(5, "PDE exists for 0x%x\n", va);
    dprintf(5, "*pde = 0x%x\n", *pde);
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
    dprintf(5,"*pgtab1 = 0x%x\n", *pgtab);
  } else {
    dprintf(5, "PDE does not exist for 0x%x, alloc = %d\n", va, alloc);
    if(!alloc)
      return 0;
    pgtab = (pte_t*)kalloc();
    dprintf(5, "kalloced new page for page table at pgtab = 0x%x, currently it has garbage, *pgtab = 0x%x\n", pgtab, *pgtab);
    if (pgtab == 0)
      return 0;
    dprintf(5, "allocated all 0s in newly allocated page table page\n");
    memset(pgtab, 0, PGSIZE);
    dprintf(5,"*pgtab2 = 0x%x\n", *pgtab);
    dprintf(5, "pgtab2[1] = 0x%x\n", pgtab[1]);
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
    dprintf(5,"marked *pde with *pte and permissions\n");
    dprintf(5,"*pde after = 0x%x\n", *pde);
  }
  dprintf(5, "now using middle 10 bits = %d of va = 0x%x to index into *pgtab = 0x%x\n", PTX(va), va, *pgtab);
  dprintf(5,"*pte = 0x%x\n", pgtab[PTX(va)]);
  dprintf(4,"\nSUMMARY:\n");
  dprintf(4, "*pgdir = 0x%x\n*pde = 0x%x\n*pgtab = 0x%x\n*pte = 0x%x\n\n", *pgdir, *pde, *pgtab, pgtab[PTX(va)]);
  return &pgtab[PTX(va)];
}

static int map_single_page(pde_t *pgdir, void *va, uint pa, int perm) {
  dprintf(4,"map_single_page()\n");
  dprintf(2,"map va = 0x%x to pa = 0x%x\n", va, pa);
  dprintf(4, "check if PTE exists for pa = 0x%x, if not, assign\n", pa);
  pte_t *pte = walkpgdir(pgdir, va, 1);
  dprintf(3,"*pte before = 0x%x\n", *pte);
  *pte = pa | perm | PTE_P;
  increment_refcount(pa);
  dprintf(3,"*pte after = 0x%x\n", *pte);
  dprintf(5, "refcount(0x%x) = %d\n", pa, getrefcount(pa));
  return 0;
}

int do_real_mapping(uint addr, int idx) {
  dprintf(4,"do_real_mapping()\n");
  char* mem = kalloc();
  if (mem == 0)
    panic("Kernel OOM!\n");
  memset(mem, 0, PGSIZE);

  struct proc* currproc = myproc();
  uint aligned = PGROUNDDOWN(addr);
  if (map_single_page(currproc->pgdir, (void*)aligned, V2P(mem), PTE_W | PTE_U) < 0)
    return -1;

  currproc->wmapdata.winfo.n_loaded_pages[idx]++;

  int flags = currproc->wmapdata.flags[idx];

  if (!(flags & MAP_ANONYMOUS)) {
    uint start_addr = currproc->wmapdata.winfo.addr[idx];
    int fd = currproc->wmapdata.fd[idx];
    int wmaplength = currproc->wmapdata.winfo.length[idx];
    uint offset = addr - start_addr;
    uint remaining = start_addr + wmaplength - addr;
    if (remaining > PGSIZE)
      remaining = PGSIZE;
    struct file* f = currproc->ofile[fd];
    dprintf(3, "reading %d bytes at offset %d from fd = %d into addr = 0x%x\n", remaining, offset, fd, addr);
    int r;
    begin_op();
    ilock(f->ip);
    r = readi(f->ip, (void*)aligned, offset, remaining);
    iunlock(f->ip);
    end_op();
    dprintf(3, "bytes read = %d\n", r);
  }

  // show_lazy_mappings();
  return 0;
}

int do_copy_on_write(uint addr) {

  dprintf(4, "do_copy_on_write()\n");
  pte_t* pte;
  uint pa, flags;

  if((pte = walkpgdir(myproc()->pgdir, (void*)addr, 0)) == 0)
    panic("copyonwrite: pte should exist");
  if(!(*pte & PTE_P))
    panic("copyonwrite: page not present");
  if (!(*pte & PTE_COW))
    return -1;
    
  pa = PTE_ADDR(*pte); // get physical address
  flags = PTE_FLAGS(*pte); // get flags

  flags &= ~(PTE_COW);
  flags |= PTE_W; // invert what was done during copyuvm/copuwmap

  if (getrefcount(pa) == 1) { // last process trying to write to this page
    *pte = pa | flags;
    return 0;
  }

  char* mem = kalloc();
  if (mem == 0)
    panic("Kernel OOM!\n");
  
  memmove(mem, (char*)P2V(pa), PGSIZE);
  uint aligned = PGROUNDDOWN(addr);
  if (map_single_page(myproc()->pgdir, (void*)aligned, V2P(mem), flags) < 0) {
    kfree(mem);
    return -1;
  }
  decrement_refcount(pa);
  return 0;
}

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  memset(&p->wmapdata, 0, sizeof(wmap_data));
  dprintf(3, "wmap info:\n");
  dprintf(3, "total_mmaps = %d\n", p->wmapdata.winfo.total_mmaps);

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  // p->wmapdata->winfo.total_mmaps = 0;
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  dprintf(4,"growproc()\n");
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

uint
wmap(uint addr, int length, int flags, int fd)
{
  dprintf(4,"wmap()\n");
  dprintf(1,"wmap addr=%x, length=%d, flags=%d, fd=%d\n", addr, length, flags, fd);
  dprintf(3,"*pgdir = 0x%x\n", *myproc()->pgdir);
  int idx = lazily_mapped_index(addr);
  if (idx != -1)
    return FAILED;
  add_lazy_mapping(addr, length, fd, flags);
  return addr;
}

int
wunmap_handler(uint addr, int freeidx)
{
  dprintf(2,"wunmap_handler(freeidx = %d)\n", freeidx);
  int idx = lazily_mapped_index(addr);
  if (idx == -1)
    return FAILED;

  struct proc* currproc = myproc();  
  wmap_data wdata = currproc->wmapdata;
  int length = wdata.winfo.length[idx];
  int n_pages = length / PGSIZE;
  if (length % PGSIZE)
    n_pages++;
  pte_t* pgdir = currproc->pgdir;
  pte_t *pte;
  uint a = addr;
  while (n_pages--) {
    if ((pte = walkpgdir(pgdir, (char*)a, 0)) == 0)
      continue;
    if((*pte & PTE_P) != 0){
      uint pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
      dprintf(4, "va 0x%x had been lazily mapped to 0x%x, removing mapping\n", a, pa);
      dprintf(4, "refcount(0x%x) before: %d\n", getrefcount(pa));
      decrement_refcount(pa);
      dprintf(4, "refcount(0x%x) after: %d\n", getrefcount(pa));
      int flags = currproc->wmapdata.flags[idx];
      if (!(flags & MAP_ANONYMOUS)) {
        uint start_addr = currproc->wmapdata.winfo.addr[idx];
        int fd = currproc->wmapdata.fd[idx];
        int wmaplength = currproc->wmapdata.winfo.length[idx];
        uint offset = a - start_addr;
        uint remaining = start_addr + wmaplength - a;
        if (remaining > PGSIZE)
          remaining = PGSIZE;
        struct file* f = currproc->ofile[fd];
        dprintf(4, "writing %d bytes at offset %d from addr = 0x%x into fd = %d\n", remaining, offset, a, fd);
        int r;
        begin_op();
        ilock(f->ip);
        r = writei(f->ip, (void*)a, offset, remaining);
        iunlock(f->ip);
        end_op();
        dprintf(4, "bytes written = %d\n", r);
      }
      if (getrefcount(pa) == 0) {
        dprintf(4, "This was the last one, freeing 0x%x\n", pa);
        char *v = P2V(pa);
        kfree(v);
      }
      *pte = 0;
    }
    a += PGSIZE;
  }
  if (freeidx)
    free_lazy_idx(idx);
  return SUCCESS;
}

int wunmap(uint addr) {
  return  wunmap_handler(addr, 1);
}

uint va2pa(uint addr)
{
  dprintf(4,"va2pa()\n");
  pte_t* pte = walkpgdir(myproc()->pgdir, (void*)addr, 0);
  if (pte == 0 || *pte == 0 || !(*pte & PTE_P))
    return FAILED;
  uint offset = PTE_FLAGS(addr);
  uint pa = PTE_ADDR(*pte) | offset;
  return pa;
}

int
getwmapinfo(struct wmapinfo* ps)
{
  dprintf(4, "getwmapinfo() in proc.c\n");
  struct wmapinfo winfo = myproc()->wmapdata.winfo;
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    ps->addr[i] = winfo.addr[i];
    ps->length[i] = winfo.length[i];
    ps->n_loaded_pages[i] = winfo.n_loaded_pages[i];
  }
  ps->total_mmaps = winfo.total_mmaps;
  return SUCCESS;
}

void copywmapdata(struct proc* np) {
  dprintf(4, "copywmapdata()\n");
  struct proc* currproc = myproc();
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    np->wmapdata.fd[i] = currproc->wmapdata.fd[i];
    np->wmapdata.flags[i] = currproc->wmapdata.flags[i];
    np->wmapdata.winfo.addr[i] = currproc->wmapdata.winfo.addr[i];
    np->wmapdata.winfo.length[i] = currproc->wmapdata.winfo.length[i];
    np->wmapdata.winfo.n_loaded_pages[i] = currproc->wmapdata.winfo.n_loaded_pages[i];
  }
  np->wmapdata.winfo.total_mmaps = currproc->wmapdata.winfo.total_mmaps;
  // dprintf(4, "after copying:\n");
  // show_lazy_mappings(np);
}

pde_t* copywmap(pde_t* child_pgdir) {
  struct proc* currproc = myproc();
  pde_t* pgdir = currproc->pgdir;
  pte_t *pte;
  uint pa, start_addr, length, i, flags;

  for (i = 0; i < MAX_WMMAP_INFO; i++) {
    start_addr = currproc->wmapdata.winfo.addr[i];
    if (start_addr > 0) {
      dprintf(4, "copying wmap at va: 0x%x\n", start_addr);
      length = currproc->wmapdata.winfo.length[i];
      for (uint va = start_addr; va < start_addr + length; va += PGSIZE) {        
        pte = walkpgdir(pgdir, (void *) va, 0);
        if (pte != 0) {
          if(!(*pte & PTE_P))
            panic("copywmap: page not present");

          pa = PTE_ADDR(*pte); // get physical address
          flags = PTE_FLAGS(*pte); // get flags
          
          if (map_single_page(child_pgdir, (void*)va, pa, flags) < 0) {
            freevm(child_pgdir);
            return 0;
          }
        }
      }
    }
  }
  return child_pgdir;
}

void freewmaps() {
  dprintf(4, "freewmaps()\n");
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    uint addr = myproc()->wmapdata.winfo.addr[i];
    if (addr > 0)
      wunmap_handler(addr, 0);
  }
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  dprintf(4, "fork called()\n");
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }
  dprintf(4, "allocproc() complete\n");
  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  dprintf(4, "copyuvm() complete\n");
  copywmapdata(np);
  dprintf(4, "copywmapdata() complete\n");

  if((np->pgdir = copywmap(np->pgdir)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  dprintf(4, "copywmap() complete\n");
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);
  dprintf(4, "copying fd complete\n");

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));
  dprintf(4, "copying name complete\n");

  pid = np->pid;
  dprintf(4, "pid %d forked pid: %d\n", curproc->pid, pid);

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  dprintf(4, "exit()\n");
  freewmaps();

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      // dprintf(4, "switching to pid: %d\n", p->pid);
      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  // dprintf(4, "sched called by pid: %d\n", myproc()->pid);
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

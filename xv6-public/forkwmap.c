#include "types.h"
#include "user.h"
#include "wmap.h"

// Test Helpers
#define MMAPBASE 0x60000000
#define KERNBASE 0x80000000
#define KERNCODE 0x100000
#define PHYSTOP 0xE000000 // Top physical memory
#define PGSIZE 0x1000

void show_lazy_mappings(struct wmapinfo* wmaps) {
  printf(1,"Lazy mappings:\n\n");
  printf(1,"addr\t\tlength\tloaded\n");
  for (int i = 0; i < MAX_WMMAP_INFO; i++) {
    int addr = wmaps->addr[i];
    if (addr > 0) {
      int length = wmaps->length[i];
      int loaded = wmaps->n_loaded_pages[i];
      printf(1,"0x%x\t%d\t%d\n",addr,length,loaded);
    }
  }
  printf(1,"\ntotal_mmaps = %d\n\n", wmaps->total_mmaps);
}

int main(int argc, char* argv[]) {

    uint addr = MMAPBASE + PGSIZE * 2;
    uint length = PGSIZE * 4 + 8;
    int anon = MAP_FIXED | MAP_ANONYMOUS | MAP_SHARED;
    int fd = -1;
    printf(1, "calling wmap()\n");
    uint map = wmap(addr, length, anon, fd);
    if (map != addr) {
        printf(1, "wmap() returned %d\n", (int)map);
        exit();
    }

    printf(1,"access memory\n");
    char *arr = (char *)map;
    for (int i = 0; i < length; i++) {
        arr[i] = 'a';
        // if (i > 2*PGSIZE)
        //     printf(1,"i = %d\n", i);
    }

    struct wmapinfo winfo;
    int rc = getwmapinfo(&winfo);
    printf(1, "PID: %d | getwmapinfo() = %d\n", getpid(), rc);
    printf(1, "PID: %d | lazy mappings in parent before fork are:\n", getpid());
    show_lazy_mappings(&winfo);

    printf(1, "PID: %d | forking()\n");
    int pid = fork();

    if (pid < 0)
        printf(1, "PID: %d | fork() failed()\n", getpid());
    else if (pid == 0) {
        rc = getwmapinfo(&winfo);
        printf(1, "PID: %d | getwmapinfo() = %d\n", getpid(), rc);
        printf(1, "PID: %d | lazy mappings in child are:\n", getpid());
        show_lazy_mappings(&winfo);
    } else {
        wait();
        rc = getwmapinfo(&winfo);
        printf(1, "PID: %d | getwmapinfo() = %d\n", getpid(), rc);
        printf(1, "PID: %d | lazy mappings in parent after fork are:\n", getpid());
        show_lazy_mappings(&winfo);
    }

    //
    // Unmap the map
    //
    printf(1, "PID: %d | calling wunmap()\n", getpid());
    int ret = wunmap(map);
    if (ret < 0) {
        printf(1, "PID: %d | wunmap() returned %d\n", getpid(), ret);
    } else {
        printf(1, "PID: %d | mappings after wunmap\n", getpid());
        rc = getwmapinfo(&winfo);
        show_lazy_mappings(&winfo);
    }

    exit();
}
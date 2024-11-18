#include "types.h"
#include "user.h"
#include "wmap.h"

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
    // printf(1, "calling malloc()\n");
    // char* s = (char*)malloc(3*sizeof(char));
    // s[0] = 'a';
    // s[1] = 'b';
    // s[2] = '\n';
    // printf(1, "s = %s\n",s);

    // uint out = va2pa(0x60000000); // get 0
    // printf(1, "va2pa(0x60000000) before wmap() = 0x%x\n", out);
    // int rc1 = wmap(0x60000000,8192,10,2); // wmap
    // printf(1, "rc1 = %d\n", rc1);
    // out = va2pa(0x60000000); // get 0
    // printf(1, "va2pa(0x60000000) after wmap() = 0x%x\n", out);
    // memset((void*)0x60000000,0,4096); // cause page-fault and allocation
    // out = va2pa(0x60000000); // get 0
    // printf(1, "va2pa(0x60000000) after wmap() = 0x%x\n", out);
    
    // uint out1 = va2pa(0x3000);
    // printf(1, "va2pa(0x3000) = 0x%x\n", out1);
    // out1 = va2pa(0x3001);
    // printf(1, "va2pa(0x3001) = 0x%x\n", out1);
    // out1 = va2pa(0x3fff);
    // printf(1, "va2pa(0x3fff) = 0x%x\n", out1);
    // out1 = va2pa(0x4000);
    // printf(1, "va2pa(0x4000) = 0x%x\n", out1);

    struct wmapinfo ps;
    int rc = getwmapinfo(&ps);
    printf(1, "getwmapinfo() = %d\n", rc);
    show_lazy_mappings(&ps);

    uint out = va2pa(VA_START); // get 0
    printf(1, "va2pa(0x60000000) = 0x%x\n", out);
    out = va2pa(VA_START + 4095);
    printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START+4096); // get 0
    printf(1, "va2pa(0x60001000) = 0x%x\n", out);
    out = va2pa(VA_START + 8191); // get 0
    printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
    out = va2pa(VA_START + 8192); // get 0
    printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

    printf(1, "wmap(0x60000000,8192)\n");
    int rc1 = wmap(VA_START,8192,10,2);
    printf(1, "rc1 = %d\n\n", rc1);

    rc = getwmapinfo(&ps);
    printf(1, "getwmapinfo() = %d\n\n", rc);
    show_lazy_mappings(&ps);

    out = va2pa(VA_START); // get 0
    printf(1, "(0x60000000) = 0x%x\n", out);
    out = va2pa(VA_START + 4095);
    printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START+4096); // get 0
    printf(1, "va2pa(0x60001000) = 0x%x\n", out);
    out = va2pa(VA_START + 8191); // get 0
    printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
    out = va2pa(VA_START + 8192); // get 0
    printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

    printf(1, "memset(0x60000000,0,4096)\n\n");
    memset((void*)VA_START,0,4096); // cause page-fault and allocation

    rc = getwmapinfo(&ps);
    printf(1, "getwmapinfo() = %d\n\n", rc);
    show_lazy_mappings(&ps);

    out = va2pa(VA_START);
    printf(1, "va2pa(0x60000000) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START + 4095);
    printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START + 4096); // get 0
    printf(1, "va2pa(0x60001000) = 0x%x\n", out);
    out = va2pa(VA_START + 8191); // get 0
    printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
    out = va2pa(VA_START + 8192); // get 0
    printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

    printf(1, "memset(0x60001000,0,4096)\n\n");
    memset((void*)(VA_START+4096),0,4096); // cause page-fault and allocation

    rc = getwmapinfo(&ps);
    printf(1, "getwmapinfo() = %d\n\n", rc);
    show_lazy_mappings(&ps);

    out = va2pa(VA_START);
    printf(1, "va2pa(0x60000000) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START + 4095);
    printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START + 4096); // get 0
    printf(1, "va2pa(0x60001000) = 0x%x\n", out);
    out = va2pa(VA_START + 8191); // get 0
    printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
    out = va2pa(VA_START + 8192); // get 0
    printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

    printf(1, "wunmap(0x60000000)\n");
    int rc2 = wunmap(VA_START);
    printf(1, "rc2 = %d\n\n", rc2);

    rc = getwmapinfo(&ps);
    printf(1, "getwmapinfo() = %d\n\n", rc);
    show_lazy_mappings(&ps);

    out = va2pa(VA_START);
    printf(1, "va2pa(0x60000000) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START + 4095);
    printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START + 4096); // get 0
    printf(1, "va2pa(0x60001000) = 0x%x\n", out);
    out = va2pa(VA_START + 8191); // get 0
    printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
    out = va2pa(VA_START + 8192); // get 0
    printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

    // int x = 0x2;
    // int y = x & 1;
    // int z = x & 2;
    // printf(1,"x=0x%x, y=0x%x, z=0x%x\n", x, y, z);

    // int rc2 = wmap(VA_START+4096,8192,10,2);
    // printf(1, "rc2 = %d\n", rc2);

    // char* s2 = (char*)(VA_START);
    // s2[0] = 'c';
    // s2[1] = 'd';
    // s2[2] = '\n';
    // printf(1, "s2 = %s\n", s2);

    // memset((void*)VA_START,0,100);

    // out = va2pa(VA_START);
    // printf(1, "va2pa(0x60000000) = 0x%x\n", out);

    // int rc2 = wmap(0x60001000,123,10,2);
    // printf(1, "rc2 = %d\n", rc2);
    
    // wunmap(0);
    // va2pa(0);
    // struct wmapinfo* p = (struct wmapinfo*)malloc(sizeof(struct wmapinfo));
    // getwmapinfo(p);
    exit();
}
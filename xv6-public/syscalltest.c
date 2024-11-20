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

// int main(int argc, char* argv[]) {

//     struct wmapinfo ps;
//     int rc = getwmapinfo(&ps);
//     printf(1, "getwmapinfo() = %d\n", rc);
//     show_lazy_mappings(&ps);

//     uint out = va2pa(VA_START); // get 0
//     printf(1, "va2pa(0x60000000) = 0x%x\n", out);
//     out = va2pa(VA_START + 4095);
//     printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
//     out = va2pa(VA_START+4096); // get 0
//     printf(1, "va2pa(0x60001000) = 0x%x\n", out);
//     out = va2pa(VA_START + 8191); // get 0
//     printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
//     out = va2pa(VA_START + 8192); // get 0
//     printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

//     printf(1, "wmap(0x60000000,8192)\n");
//     int rc1 = wmap(VA_START,8192,10,2);
//     printf(1, "rc1 = %d\n\n", rc1);

//     rc = getwmapinfo(&ps);
//     printf(1, "getwmapinfo() = %d\n\n", rc);
//     show_lazy_mappings(&ps);

//     out = va2pa(VA_START); // get 0
//     printf(1, "(0x60000000) = 0x%x\n", out);
//     out = va2pa(VA_START + 4095);
//     printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
//     out = va2pa(VA_START+4096); // get 0
//     printf(1, "va2pa(0x60001000) = 0x%x\n", out);
//     out = va2pa(VA_START + 8191); // get 0
//     printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
//     out = va2pa(VA_START + 8192); // get 0
//     printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

//     printf(1, "memset(0x60000000,0,4096)\n\n");
//     memset((void*)VA_START,0,4096); // cause page-fault and allocation

//     rc = getwmapinfo(&ps);
//     printf(1, "getwmapinfo() = %d\n\n", rc);
//     show_lazy_mappings(&ps);

//     out = va2pa(VA_START);
//     printf(1, "va2pa(0x60000000) = 0x%x\n", out); // get correct pa
//     out = va2pa(VA_START + 4095);
//     printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
//     out = va2pa(VA_START + 4096); // get 0
//     printf(1, "va2pa(0x60001000) = 0x%x\n", out);
//     out = va2pa(VA_START + 8191); // get 0
//     printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
//     out = va2pa(VA_START + 8192); // get 0
//     printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

//     printf(1, "memset(0x60001000,0,4096)\n\n");
//     memset((void*)(VA_START+4096),0,4096); // cause page-fault and allocation

//     rc = getwmapinfo(&ps);
//     printf(1, "getwmapinfo() = %d\n\n", rc);
//     show_lazy_mappings(&ps);

//     out = va2pa(VA_START);
//     printf(1, "va2pa(0x60000000) = 0x%x\n", out); // get correct pa
//     out = va2pa(VA_START + 4095);
//     printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
//     out = va2pa(VA_START + 4096); // get 0
//     printf(1, "va2pa(0x60001000) = 0x%x\n", out);
//     out = va2pa(VA_START + 8191); // get 0
//     printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
//     out = va2pa(VA_START + 8192); // get 0
//     printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

//     printf(1, "wunmap(0x60000000)\n");
//     int rc2 = wunmap(VA_START);
//     printf(1, "rc2 = %d\n\n", rc2);

//     rc = getwmapinfo(&ps);
//     printf(1, "getwmapinfo() = %d\n\n", rc);
//     show_lazy_mappings(&ps);

//     out = va2pa(VA_START);
//     printf(1, "va2pa(0x60000000) = 0x%x\n", out); // get correct pa
//     out = va2pa(VA_START + 4095);
//     printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
//     out = va2pa(VA_START + 4096); // get 0
//     printf(1, "va2pa(0x60001000) = 0x%x\n", out);
//     out = va2pa(VA_START + 8191); // get 0
//     printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
//     out = va2pa(VA_START + 8192); // get 0
//     printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

//    exit();
// }

int main(int argc, char* argv[]) {

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

    printf(1, "wmap(0x60000000,4096)\n");
    int rc1 = wmap(VA_START,4096,10,2);
    printf(1, "rc1 = 0x%x\n\n", rc1);

    rc = getwmapinfo(&ps);
    printf(1, "getwmapinfo() = %d\n\n", rc);
    show_lazy_mappings(&ps);

    out = va2pa(VA_START); // get 0
    printf(1, "va2pa(0x60000000) = 0x%x\n", out);
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

    printf(1, "wmap(0x60001000,4096)\n");
    rc1 = wmap(VA_START + 4096,4096,10,2);
    printf(1, "rc1 = 0x%x\n\n", rc1);

    rc = getwmapinfo(&ps);
    printf(1, "getwmapinfo() = %d\n\n", rc);
    show_lazy_mappings(&ps);

    out = va2pa(VA_START); // get 0
    printf(1, "va2pa(0x60000000) = 0x%x\n", out);
    out = va2pa(VA_START + 4095);
    printf(1, "va2pa(0x60000fff) = 0x%x\n", out); // get correct pa
    out = va2pa(VA_START+4096); // get 0
    printf(1, "va2pa(0x60001000) = 0x%x\n", out);
    out = va2pa(VA_START + 8191); // get 0
    printf(1, "va2pa(0x60001fff) = 0x%x\n", out);
    out = va2pa(VA_START + 8192); // get 0
    printf(1, "va2pa(0x60002000) = 0x%x\n\n", out);

    printf(1, "memset(0x60001000,0,4096)\n\n");
    memset((void*)(VA_START + 4096),0,4096); // cause page-fault and allocation

    // printf(1, "memset(0x60001000,0,4096)\n\n");
    // memset((void*)(VA_START+4096),0,4096); // cause page-fault and allocation

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

    printf(1, "wunmap(0x60001000)\n");
    rc2 = wunmap(VA_START+4096);
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

   exit();
}
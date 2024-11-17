#include "types.h"
#include "user.h"
#include "wmap.h"

int main(int argc, char* argv[]) {
    printf(1, "calling malloc()\n");
    char* s = (char*)malloc(3*sizeof(char));
    s[0] = 'a';
    s[1] = 'b';
    s[2] = '\n';
    printf(1, "s = %s\n",s);

    // uint out1 = va2pa(0x3000);
    // printf(1, "va2pa(0x3000) = 0x%x\n", out1);
    // out1 = va2pa(0x3001);
    // printf(1, "va2pa(0x3001) = 0x%x\n", out1);
    // out1 = va2pa(0x3fff);
    // printf(1, "va2pa(0x3fff) = 0x%x\n", out1);
    // out1 = va2pa(0x4000);
    // printf(1, "va2pa(0x4000) = 0x%x\n", out1);
    
    int rc1 = wmap(VA_START,123,10,2);
    printf(1, "rc1 = %d\n", rc1);

    uint out = va2pa(VA_START);
    printf(1, "va2pa(0x60000000) = 0x%x\n", out);

    // memset((void*)VA_START,0,100);

    out = va2pa(VA_START);
    printf(1, "va2pa(0x60000000) = 0x%x\n", out);
    
    // int rc2 = wmap(0x60001000,123,10,2);
    // printf(1, "rc2 = %d\n", rc2);
    
    // wunmap(0);
    // va2pa(0);
    // struct wmapinfo* p = (struct wmapinfo*)malloc(sizeof(struct wmapinfo));
    // getwmapinfo(p);
    exit();
}
#include "types.h"
#include "user.h"
#include "wmap.h"

int main(int argc, char* argv[]) {
    int rc1 = wmap(VA_START,123,10,2);
    printf(1, "rc1 = %d\n", rc1);
    int rc2 = wmap(0x60001000,123,10,2);
    printf(1, "rc2 = %d\n", rc2);

    wunmap(0);
    va2pa(0);
    // struct wmapinfo* p = (struct wmapinfo*)malloc(sizeof(struct wmapinfo));
    // getwmapinfo(p);
    return 0;
}
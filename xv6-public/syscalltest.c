#include "types.h"
#include "user.h"
#include "wmap.h"

int main(int argc, char* argv[]) {
    int rc = wmap(VA_START,123,10,2);
    printf(1, "rc = %d\n", rc);
    wunmap(0);
    va2pa(0);
    // struct wmapinfo* p = (struct wmapinfo*)malloc(sizeof(struct wmapinfo));
    // getwmapinfo(p);
    return 0;
}
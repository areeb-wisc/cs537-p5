// refcount.h
#ifndef REFCOUNT_H
#define REFCOUNT_H

#include "spinlock.h"  // Make sure to include the spinlock header

// Structure for keeping reference counts
typedef struct _refcount {
    unsigned char refcounts[1024*1024];  // Array to store reference counts
    struct spinlock reflock;        // Spinlock for synchronization
} countholder;

// Declare the global reference count structure (without defining it yet)
extern countholder global_refcount;

#endif

#pragma once
#include "types.h"
#include "multitasking.h"

// The maximum number of user procs
#define MAX_USER_PROCS 5

// The maximum number of kernel procs
#define MAX_KERN_PROCS 1

// The maximum number of total procs
#define MAX_PROCS (MAX_USER_PROCS + MAX_KERN_PROCS)

// An array to hold all of the processes we create
extern proc_t processes[MAX_PROCS];
 
// Keep track of the next index to place a newly created process in the process array
extern uint8 process_index;
 
//static proc_t *prevprocess;       // The previously ran user process
extern proc_t *runningprocess;    // The currently running process, can be either kernel or user process
extern proc_t *nextprocess;       // The next process to run
extern proc_t *kernelprocess;     // The kernel process

int startkernel(void func());
int ready_process_count();
void contextswitch();
void yield();
void exit();
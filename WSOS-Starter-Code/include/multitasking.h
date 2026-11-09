#pragma once
#include "types.h"

// All possible statuses for processes
typedef enum
{
	PROC_STATUS_NONE,
	PROC_STATUS_READY,
    PROC_STATUS_RUNNING,
	PROC_STATUS_TERMINATED
} proc_status_t;

// All possible types of processes
typedef enum
{
	PROC_TYPE_NONE,
	PROC_TYPE_USER,
	PROC_TYPE_KERNEL
} proc_type_t;


// Process control block
// Contains all registers and info for each process
typedef struct
{
    int pid;
	proc_type_t type;
	proc_status_t status;
	uint32 eax;
	uint32 ebx;
	uint32 ecx;
	uint32 edx;
	uint32 esi;
	uint32 edi;
	void *ebp;
	void *esp;
	uint32 eflags;
	uint32 cs;
	uint32 cr3;
	void *eip;
} proc_t;

int startkernel(void func());
int ready_process_count();
void contextswitch();
void yield();
void exit();
int createuserprocess(void *func, void *stack);
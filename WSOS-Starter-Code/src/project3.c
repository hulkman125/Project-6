#include "project3.h"

#include "multitasking.h"
#include "processes.h"

// An array to hold all of the processes we create
proc_t processes[MAX_PROCS];

// Keep track of the next index to place a newly created process in the process array
uint8 process_index = 0;

proc_t* prevprocess = 0;  // The previously ran user process
proc_t* runningprocess;   // The currently running process, can be either kernel or user process
proc_t* nextprocess;      // The next process to run
proc_t* kernelprocess;    // The kernel process

#if PROJECT == 3

// The following are the user processes that will be created and run by the kernel process
void proca() {    putchar('A');    exit();}
void procb() {    putchar('B');    yield();    putchar('B');    exit();}
void procc(){    putchar('C');    yield();    putchar('C');    yield();    putchar('C');    yield();    putchar('C');    exit();}
void procd() {    putchar('D');    yield();    putchar('D');    yield();    putchar('D');    exit();}
void proce() {    putchar('E');    yield();    putchar('E');    exit();}

void prockernel() {
    print("Kernel process has started...\n");

    // Create the user processes
    createuserprocess(proca, (void*)0x10000);
    createuserprocess(procb, (void*)0x20000);
    createuserprocess(procc, (void*)0x30000);
    createuserprocess(procd, (void*)0x40000);
    createuserprocess(proce, (void*)0x50000);

    // Schedule the next process
    int userprocs = ready_process_count();

    // As long as we have ready user processes to run
    while (userprocs > 0) {
        // Yield to them
        yield();
        userprocs = ready_process_count();
    }

    print("\nKernel process has exited...\n");
    exit();
}

int kernel() {
    startkernel(prockernel);
    return 0;
}

#endif

// Select the next user process (proc_t *next) to run
// Selection must be made from the processes array (proc_t processes[])
int schedule() {


    int index; // Start searching for the next process after the current process

    if(prevprocess == 0) { // If there is no previous process, start searching from the beginning of the array
        index = 0;
    } else { // If there is a previous process, start searching from the next index
        index = prevprocess->pid + 1;
    }
    
    //to begin from the index number all the way to process number 6
    for(int i = index; i < MAX_PROCS; i++) { // Loop through the processes array starting from the next index
       if(processes[i].status == PROC_STATUS_READY && processes[i].type == PROC_TYPE_USER) { // If the process is ready to run
            nextprocess = &processes[i]; // Set the next process to run to the current process
            return 1; // Return 1 to indicate that a process was selected
        }
    }

    //to begin from the beginning of the array all the way to the index number
   for(int i = 0; i < index; i++) { // Loop through the processes array starting from the beginning
        if(processes[i].status == PROC_STATUS_READY && processes[i].type == PROC_TYPE_USER) { // If the process is ready to run
            nextprocess = &processes[i]; // Set the next process to run to the current process
            return 1; // Return 1 to indicate that a process was selected
        }
    }
    return 0; // Return 0 to indicate that no process was selected
}

// Yield the current process
// This will give another process a chance to run
// If we yielded a user process, switch to the kernel process
// If we yielded a kernel process, switch to the next process
// The next process should have already been selected via scheduling
void yield() {
    runningprocess->status = PROC_STATUS_READY; //set the status of the running process to ready

    if(runningprocess->type == PROC_TYPE_USER){
        prevprocess = runningprocess; //set the previous process to the running process
        nextprocess = kernelprocess; //set the next process to the kernel process
    }
    else if(runningprocess->type == PROC_TYPE_KERNEL){
        schedule(); //call the schedule function to select the next process to run
    }
    contextswitch(); //call the context switch function to switch to the next process
}

// Terminate the process that is currently running (proc_t current)
// Assign the kernel as the next process to run
// Context switch to the kernel process
void exit() {
    if(runningprocess->type == PROC_TYPE_USER) { //checking to see if the running process is a user process
        runningprocess->status = PROC_STATUS_TERMINATED; //if it is, we set the status to terminated
        nextprocess = kernelprocess; //we set the next process to run to be the kernel process
        contextswitch(); //we call the context switch function to switch to the kernel process
        return;
    }
}

// Create a new user process
// When the process is eventually ran, start executing from the function provided (void *func)
// Initialize the stack top and base at location (void *stack)
// If we have hit the limit for maximum processes, return -1
// Store the newly created process inside the processes array (proc_t processes[])
int createuserprocess(void* func, void* stack) {

    if(process_index >= MAX_PROCS) { //check to see if we have hit the limit for maximum processes
        return -1; //if we have, return -1
    }
    
    processes[process_index].pid = process_index; //set the process id of the new process to the current process index
    processes[process_index].type = PROC_TYPE_USER; //set the type of the new
    processes[process_index].status = PROC_STATUS_READY; //set the status of the new process to ready
    processes[process_index].esp = stack; //set the stack pointer of the new process to the stack provided
    processes[process_index].ebp = stack; //set the base pointer of the new process to the stack provided
    processes[process_index].eip = func; //set the instruction pointer of the new process to the function provided
    process_index++; //increment the process index to point to the next available slot in the processes

    return 0;
}


//*****************************************************************************
//
// kernel.c - Module for a round robin kernel ().  Used to store kernel tasks
// and run them based on the delay.  Prioritisation is based on the order
// the task is added/registered
//
// Author:  bma206, tki36
// Last modified:   14.5.2024
//
//*****************************************************************************


#include <stdint.h>
#include <stdbool.h>

#include "kernel.h"


// Max number of tasks the kernel can have
#define MAX_TASKS 7


// Number of tasks in the kernel
uint8_t num_tasks = 0;
// Kernel task storage
kernel_task tasks[MAX_TASKS];

//
// Registers a new task for the kernel
//
void registerTask(void (*func)(void), int16_t delay)
{
    // Creates a kernel task if MAX_TASKS is not reached
    if(num_tasks < MAX_TASKS) {
        kernel_task new_task = {*func, delay, 0}; // Creates the task
        tasks[num_tasks] = new_task;    // Adds it to the kernel
        num_tasks++;    // Update number of tasks
    }
}

//
// Runs all the regstered tasks that need to be run
//
void runTasks(void)
{
    int i;
    for(i = 0; i < num_tasks; i++) {
        tasks[i].currect_tick++; // Update the time since last run
        kernel_task temp_task = tasks[i]; // Load task into active task

        // Check if task is ready to run
        if(temp_task.currect_tick <= temp_task.delay) {
            // Run the task
            ((void(*)(kernel_task*))(temp_task.task_func))(&temp_task);
            // Update the time since last run
            tasks[i].currect_tick = 0;
        }
    }
}

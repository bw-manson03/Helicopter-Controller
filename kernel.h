//*****************************************************************************
//
// kernel.h - Header for a round robin kernel ().  Used to store kernel tasks
// and run them based on the delay.  Prioritisation is based on the order
// the task is added/registered
//
// Author:  bma206, tki36
// Last modified:   14.5.2024
//
//*****************************************************************************


#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    // Task func to run the task
    void* task_func;

    // How often to run the task
    int16_t delay;

    // Last time the task was run
    int16_t currect_tick;
} kernel_task;

//
// Registers a new task for the kernel
//
void registerTask(void (*func)(void), int16_t delay);

//
// Runs all the regstered tasks that need to be run
//
void runTasks(void);

#endif // Kernel_h

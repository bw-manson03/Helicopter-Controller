#ifndef SWITCH_H_
#define SWITCH_H_

//*****************************************************************************
//
// switch.h - Module to use the switch 1
//
// Author:  bma206, tki36
// Last modified:   9.5.2024
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"

//
// Type to keep track of state of switch
//
enum swStates {SW_DOWN = 0, SW_UP, SW_NO_CHANGE};
typedef enum swStates swState_t;

//
// Type to track what the helicopter is currently doing
//
enum heliStates {LANDED = 0, FIND_YAW, FLYING, RESET_YAW, LANDING};
typedef enum heliStates heliState_t;

//
// Initialises the switch with its initial state
//
void initSwitch(void);

//
// Updates the switch and returns its current state
//
swState_t updateSwitch(void);

//
// Set the state of the helicopter
//
void setHeliState(heliState_t state);

//
// Get the state of the helicopter
//
heliState_t getHeliState(void);

#endif /*SWITCH_H_*/

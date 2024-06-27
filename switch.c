//*****************************************************************************
//
// switch.c - Module to use switch 1, checks and updates state of the switch.
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

#include "switch.h"

// Switch ports and pins
#define SW1_PERIPH      SYSCTL_PERIPH_GPIOA
#define SW1_PORT_BASE   GPIO_PORTA_BASE
#define SW1_PIN         GPIO_PIN_7
#define SW1_NORMAL      false

// State storage
static bool swState;
static bool swStateChanged = false;
static heliState_t heliState;

//
// Initialises the switch with its initial state
//
void initSwitch(void)
{
    SysCtlPeripheralEnable (SW1_PERIPH);
    GPIOPinTypeGPIOInput (SW1_PORT_BASE, SW1_PIN);
    GPIOPadConfigSet (SW1_PORT_BASE, SW1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

    swState = GPIOPinRead(SW1_PORT_BASE, SW1_PIN) == SW1_PIN;
}

//
// Updates the switch and returns its current state
//
swState_t updateSwitch(void)
{
    // Read the swithc
    bool newSwState = GPIOPinRead(SW1_PORT_BASE, SW1_PIN);

    // Checks if switch has changed
    if (newSwState == swState) {
        swStateChanged = false;
    } else if (newSwState != swState) {
        swStateChanged = true;
    }
    // Update the old switch state
    swState = newSwState;

    // Returns the switch state
    if (swStateChanged) {
        swStateChanged = false;
        if (swState) {
            return SW_DOWN;
        } else {
            return SW_UP;
        }
    } else {
        return SW_NO_CHANGE;
    }
}

//
// Set the state of the helicopter
//
void setHeliState(heliState_t state)
{
    heliState = state;
}

//
// Get the state of the helicopter
//
heliState_t getHeliState(void)
{
    return heliState;
}

//*****************************************************************************
//
// main.c - Program to start the kernel for controlling the heli
// 
//
// Author:  bma206, tki36
// Last modified:   21.4.2024
//
//*****************************************************************************
// Based on the 'convert' series from 2016
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "display.h"
#include "buttons4.h"
#include "altitude.h"
#include "yaw.h"
#include "pwm.h"
#include "serial.h"
#include "kernel.h"
#include "switch.h"
#include "control.h"


#define SAMPLE_RATE_HZ 100

// How often the kernel tasks update
#define BUTTON_UPDATE 1
#define UART_UPDATE 5
#define DISPLAY_UPDATE 1
#define CONTROL_UPDATE 1
#define SWITCH_UPDATE 1
#define STATE_UPDATE 1


static uint32_t g_ulSampCnt;    // Counter for the interrupts


//
// The interrupt handler for the for SysTick interrupt.
//
void SysTickIntHandler(void)
{
    //
    // Initiate a conversion
    //
    ADCProcessorTrigger(ADC_BASE, 3);
    g_ulSampCnt++;
}

//
// Initialisation functions for the clock (incl. SysTick)
//
void initClock(void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

//
// Checks the state of all buttons and performs relevant changes
//
void checkButtons(void)
{
    updateButtons();

    uint8_t butState = checkButton (UP); // Increase altitude
    if (butState == PUSHED && isSettled()) {
        incrementAltitude(10);
    }
    butState = checkButton (DOWN); // Decrease altitude
    if (butState == PUSHED && isSettled()) {
        incrementAltitude(-10);
    }
    butState = checkButton (LEFT); // Rotate left
    if (butState == PUSHED && getAltitudeError() <= 5) {
        incrementYaw(15);
    }
    butState = checkButton (RIGHT); // Rotate Right
    if (butState == PUSHED && getAltitudeError() <= 5) {
        incrementYaw(-15);
    }
}

//
// Checks the state of switch 1
//
void checkSwitch(void)
{
    swState_t swState = updateSwitch();
    if (swState == UP && getHeliState() == LANDED) { // Puts heli into take off procedure
        startMainRotor();
        startTailRotor();
        setTargetAltitude(10);
        setHeliState(FIND_YAW);
    } else if (swState == DOWN && getHeliState() == FLYING) { // Puts the heli into landing procedure
        setHeliState(RESET_YAW);
    }
}

//
// Manages the automatic states (landing, take off, find yaw and reset yaw)
//
void heliStateManager(void)
{
    // Gets the heli state
    heliState_t state = getHeliState();

    // Take off - Finds the reference pin
    if (state == FIND_YAW) {
        if(abs(getAltitudeError()) < 2) {
            setTailPower(10);
        }
    // Reset yaw - go to yaw 0 for landing
    } else if (state == RESET_YAW) {
        resetPosition();

    } else if (state == LANDING) { // Smoothly lands the heli
        if(canLand(30) && getTargetAltitude() > 30 && abs(getAltitudeError()) <= 4) { // Checks that the heli has settle before going down
            incrementAltitude(-5);
        }else if(canLand(25) && getTargetAltitude() <= 30 && getTargetAltitude() > 10 && abs(getAltitudeError()) < 2) {
            incrementAltitude(-5);
        }
        else if (canLand(20) && getTargetAltitude() <= 10 && abs(getAltitudeError()) == 0) { // smoothly lands
            incrementAltitude(-1);
        }
        else if(abs(getAltitudeError()) < 1 && getTargetAltitude() == 0) { // Sets the heli to landed
            setHeliState(LANDED);
            stopTailRotor();
            stopMainRotor();
            resetDI();
        }

    } else if (state == LANDED) { // Landed, make sure everything is off
        resetDI();
        stopTailRotor();
        stopMainRotor();
        setTargetAltitude(0);
    }
}

int main(void)
{


    // initialise different systems
    initClock ();
    initButtons();
    initSwitch();
    initAltitude ();
    initYaw ();
    initPWM();
    initUART();
    initDisplay ();

    // Set the heli state to landed
    setHeliState(LANDED);
    // Enable interrupts to the processor.
    IntMasterEnable();

    // Waits until the circ buffer has time to fill
    while(g_ulSampCnt <= BUF_SIZE) {}
    setBaseAltitude(getAltitudeADC());


    // Buttons
    registerTask(*checkButtons, BUTTON_UPDATE);
    // UART (serial com)
    registerTask(*UARTSendData, UART_UPDATE);
    // control
    registerTask(*updateControl, CONTROL_UPDATE);
    // switch
    registerTask(*checkSwitch, SWITCH_UPDATE);
    // helicopter state control
    registerTask(*heliStateManager, STATE_UPDATE);
    // Display
    registerTask(*updateDisplay, DISPLAY_UPDATE);


    while (1) // Main loop
    {
        // Runs the kernel tasks
        runTasks();
    }
}


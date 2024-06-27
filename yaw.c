//*****************************************************************************
//
// yaw.c - yaw calculator for ADC.  Uses two pin inputs to perform quadrature 
// decoding to calculate the current Yaw of a rotary system
//
// Author:  bma206, tki36
// Last modified:   30.4.2024
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "yaw.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "display.h"
#include "switch.h"
#include "circBufT.h"
#include "control.h"

// Yaw input channels/pins
#define YAW_CHANNEL_A GPIO_PIN_0
#define YAW_CHANNEL_B GPIO_PIN_1
#define YAW_BASE GPIO_PORTB_BASE
#define YAW_PER_B SYSCTL_PERIPH_GPIOB

// Yaw reference channels/pins
#define YAW_REF_GPIO_PERIPH   SYSCTL_PERIPH_GPIOC
#define YAW_REF_GPIO_BASE     GPIO_PORTC_BASE
#define YAW_REF_PIN           GPIO_PIN_4

// Number of slots in the light sensor
#define NUM_SLOTS 112

// current yaw of the helicopter
int16_t current_yaw = 0;

int16_t target_yaw = 0;
int16_t yaw_degrees = 0;

// Previous state of the yaw pins
bool oldAState = false;
bool oldBState = false;
// New state of the yaw pins
bool newAState = false;
bool newBState = false;

#define YAW_BUFF_SIZE 20
static circBuf_t yawErrorBuff;

//
// Handles interrupts of the yaw changing
//
void yawIntHandler()
{
    // Gets the new state of the yaw pins
    newAState = GPIOPinRead(YAW_BASE, YAW_CHANNEL_A);
    newBState = GPIOPinRead(YAW_BASE, YAW_CHANNEL_B);

    // performs quadrature decoding on the yaw pins to get the yaw change
    if(!oldAState && !oldBState){
        if (newAState && !newBState) { // A leads B, anti-clockwise
            current_yaw--;
        } else if (!newAState && newBState){
            current_yaw++;
        }
    } else if(oldAState && !oldBState) {
        if (newAState && newBState) { // A leads B, anti-clockwise
            current_yaw--;
        } else if (!newAState && !newBState){
            current_yaw++;
        }
    } else if(!oldAState && oldBState){ 
        if (newAState && newBState) { // B leads A, clockwise
            current_yaw++;
        } else if (!newAState && !newBState){
            current_yaw--;
        }
    } else { 
        if (newAState && !newBState) { // B leads A, clockwise
            current_yaw++;
        } else if(!newAState && newBState){
            current_yaw--;
        }
    } 

    // Store the new values as the old values
    oldAState = newAState;
    oldBState = newBState;
    // Clear the interrupt
    GPIOIntClear(YAW_BASE, YAW_CHANNEL_A | YAW_CHANNEL_B);
}

//
// Handles pin change interrupts on the yaw reference pin
//
void yawReferenceHandler(void)
{
    if (getHeliState() == FIND_YAW) {
        current_yaw = 0;
        target_yaw = 0;
        resetYawDI();
        setHeliState(FLYING);
    }

    GPIOIntClear(YAW_REF_GPIO_BASE, YAW_REF_PIN);
}

//
// Initialises the yaw sensors in the board
// and creates a pin change interrupt for both the pins
//
void initYaw(void)
{
    // Enables the yaw pin peripheral
    SysCtlPeripheralEnable(YAW_PER_B);

    // Sets the 2 yaw pins to be input
    GPIOPinTypeGPIOInput(YAW_BASE, YAW_CHANNEL_A);
    GPIOPinTypeGPIOInput(YAW_BASE, YAW_CHANNEL_B);

    // Gets the initial state of the pins 
    oldAState = GPIOPinRead(YAW_BASE, YAW_CHANNEL_A);
    oldBState = GPIOPinRead(YAW_BASE, YAW_CHANNEL_B);

    // Register the pin change interrupt and enables it
    GPIOIntRegister(YAW_BASE, yawIntHandler);
    GPIOIntTypeSet(YAW_BASE, YAW_CHANNEL_A | YAW_CHANNEL_B, GPIO_BOTH_EDGES);
    GPIOIntEnable(YAW_BASE, YAW_CHANNEL_A | YAW_CHANNEL_B);

    // Set reference
    SysCtlPeripheralEnable(YAW_REF_GPIO_PERIPH);
    GPIOPinTypeGPIOInput(YAW_REF_GPIO_BASE, YAW_REF_PIN);
    GPIOPadConfigSet(YAW_REF_GPIO_BASE, YAW_REF_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    
    GPIOIntRegister(YAW_REF_GPIO_BASE, yawReferenceHandler);
    GPIOIntTypeSet(YAW_REF_GPIO_BASE, YAW_REF_PIN, GPIO_FALLING_EDGE);
    GPIOIntEnable(YAW_REF_GPIO_BASE, YAW_REF_PIN);

    initCircBuf(&yawErrorBuff, YAW_BUFF_SIZE);
}

//
// Gets the last yaw value and uses it to calculate and return the yaw in degrees
//
int16_t getCurrentYaw(void)
{
    // Calculate the yaw in degrees
    yaw_degrees = current_yaw * 3600 / NUM_SLOTS / 4;

    // Makes the yaw in range 0-360 degrees
    while(yaw_degrees >= 1800)
        yaw_degrees -= 3600;
    while(yaw_degrees < -1800)
        yaw_degrees += 3600;

    return yaw_degrees;
}

//
// Gets the current target yaw
//
int16_t getTargetYaw(void)
{
    return target_yaw;
}

//
// Changes yaw by the increment amount, in degrees.
//
void incrementYaw(int16_t increment)
{
    target_yaw += increment * 10;
    while(target_yaw >= 1800)
        target_yaw -= 3600;
    while(target_yaw < -1800)
        target_yaw += 3600;
}

//
// Gets the error for Yaw values
//
int32_t getYawError(void)
{
    int16_t yaw_error = target_yaw - yaw_degrees;

    if(yaw_error > 1800) {
        return yaw_error - 3600;
    }
    else if(yaw_error < -1800) {
        return yaw_error + 3600;
    }

    return yaw_error;
}

//
// Gets helicopter back to reference yaw to land
//
void resetPosition(void)
{
    target_yaw = 0;
    if(isSettled()) {
        setHeliState(LANDING);
    }
}

//
// Updates the yaw buffer
//
void updateYawBuff(void)
{
    writeCircBuf(&yawErrorBuff, (uint32_t)(getCurrentYaw()));
}


int32_t yawError(int32_t given_yaw)
{
    int32_t yaw_error = (int32_t)target_yaw - given_yaw;

    if(yaw_error > 1800) {
        return yaw_error - 3600;
    }
    else if(yaw_error < -1800) {
        return yaw_error + 3600;
    }
    return yaw_error;
}


//
// Checks if the yaw has settled
//
bool isSettled(void) 
{
    uint16_t i;
    uint32_t sum = 0;
    uint32_t yawErrorAvg = 0;
    for (i = 0; i < YAW_BUFF_SIZE; i++)
        sum = sum + yawError((int32_t)readCircBuf(&yawErrorBuff));
    yawErrorAvg = sum / YAW_BUFF_SIZE;

    if(yawErrorAvg < 40) {
        return true;
    }
    return false;
}

bool canLand(uint32_t margin)
{
    uint16_t i;
    uint32_t sum = 0;
    uint32_t yawErrorAvg = 0;
    for (i = 0; i < YAW_BUFF_SIZE; i++)
        sum = sum + yawError((int32_t)readCircBuf(&yawErrorBuff));
    yawErrorAvg = sum / YAW_BUFF_SIZE;

    if(yawErrorAvg <= margin) {
        return true;
    }
    return false;
}

//
// Gets the error for Yaw values
//
float getYawError2(float target, float current)
{
    float yaw_error = target - current;

    if(yaw_error > 180) {
        return yaw_error - 360;
    }
    else if(yaw_error < -180) {
        return yaw_error + 360;
    }

    return yaw_error;
}

//*****************************************************************************
//
// control.c - Module for controlling the helicopter's motors with PID
//
// Author:  bma206, tki36
// Last modified:   14.5.2024
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

#include "altitude.h"
#include "yaw.h"
#include "pwm.h"

#include "control.h"

// Constants for calculating the PID
static float ALT_KP = 1.2;
static float ALT_KI = 2;
static float ALT_KD = 0.1;
static float YAW_KP = 1;
static float YAW_KI = 3;
static float YAW_KD = -0.1;

// Prev values for the PID calculatiosn
static float prevAltError = 0;
static float prevAltI = 0;
static float prevYawI = 0;

// Time step for PID
static float deltaT = 0.05;

//
// Resets the integrals for the yaw and altitude
//
void resetDI(void) {
    prevYawI = 0;
    prevAltI = 0;
}

//
// Resets the yaw derivative 
//
void resetYawDI(void) {
    prevYawI = 0;
}

//
// Gets the current prevYawI 
//
float getYI(void)
{
    return prevYawI;
}

//
// Updates the main rotor's duty cycle based on current and desired altitude
//
void updateAltitudeControl(void)
{
    float error = getAltitudeError();
    float P = ALT_KP * error;
    float dI = ALT_KI * error * deltaT;
    float D = ALT_KD * ((error - prevAltError) / deltaT);

    float control = P + (prevAltI + dI) + D;

    // Checks saturation of control and the integral
    if(control > MAX_DUTY && error > 0) {
        control = MAX_DUTY;
    } else if (control < MAIN_MIN_DUTY && error < 0) {
        control = MAIN_MIN_DUTY;
    } else {
        prevAltI = (prevAltI + dI);
    }

    prevAltError = error;
    setMainPower((int32_t)(control));
}

//
// Updates the tail rotor's duty cycle based on current and desired yaw
//
void updateYawControl(void)
{
    static float previousYaw = 0;
    int16_t tempYaw = getCurrentYaw();
    float currentYaw = (float)(tempYaw) / 10;
    int16_t tempTargetYaw = getTargetYaw();
    float targetYaw = (float)(tempTargetYaw) / 10;
    float error = getYawError2(targetYaw, currentYaw);
    float P = YAW_KP * error;
    float dI = YAW_KI * error * deltaT;

    if(prevYawI > 60) {
        prevYawI = 60;
    }

    float D = YAW_KD * (previousYaw - currentYaw) / deltaT;
    float control = P + (prevYawI + dI) + D;

    // Checks saturation of control and the integral
    if(control > MAX_DUTY && error > 0) {
        control = MAX_DUTY;
    } else if (control < TAIL_MIN_DUTY && error < 0) {
        control = TAIL_MIN_DUTY;
    } else {
        prevYawI = (prevYawI + dI);
    }
    previousYaw = currentYaw;



    updateYawBuff(); // Update the yaw buffer
    setTailPower((int32_t)(control));
}

//
// Updates main and tail motors PIDs
//
void updateControl(void)
{
    updateYawControl();
    updateAltitudeControl(); 
}


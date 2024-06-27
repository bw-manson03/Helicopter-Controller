//*****************************************************************************
//
// control.h - Module for controlling the helicopter's motors with PID
//
// Author:  bma206, tki36
// Last modified:   12.5.2024
//
//*****************************************************************************

#ifndef CONTROL_H_
#define CONTROL_H_

#include <stdint.h>

//
// Resets the integrals for the yaw and altitude
//
void resetDI(void);

//
// Gets the current prevYawI 
//
float getYI(void);

//
// Resets the yaw derivative 
//
void resetYawDI(void);

//
// Updates main and tail motors
//
void updateControl(void);

#endif /*CONTROL_H_*/

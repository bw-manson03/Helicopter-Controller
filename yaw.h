//******************************************************************************************
//
// yaw.h - header file for yaw.c to initialise the yaw interrupt and to get the current yaw.
//
// Authors; bma206, tki36
// Last modified: 30.4.24
//
//******************************************************************************************

#ifndef YAW_H_
#define YAW_H_

//
// Initialises the pins and interrupts for yaw to be measured
//
void initYaw(void);

//
// Gets the current yaw in degrees
//
int16_t getCurrentYaw(void);

//
// Gets the current target yaw
//
int16_t getTargetYaw(void);

//
// Changes yaw by the increment amount, in degrees.
//
void incrementYaw(int16_t increment);

//
// Gets the error for Yaw values
//
int32_t getYawError(void);

//
// Gets helicopter back to reference yaw to land
//
void resetPosition(void);

//
// Gets the error for Yaw values
//
float getYawError2(float target, float current);

//
//
//
void updateYawBuff(void);

//
//
//
bool isSettled(void);

bool canLand(uint32_t margin);

#endif /*YAW_H_*/


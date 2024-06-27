#ifndef PWM_H_
#define PWM_H_
//*****************************************************************************
//
// PWM.h - Module for controlling the helicopter's motor's PWM cycles
//
// Author:  bma206, tki36
// Last modified:   12.5.2024
//
//*****************************************************************************

#include <stdint.h>
// Min/max duty cycles for the main and tail rotor
static int32_t MAX_DUTY = 98;
static int32_t MAX_TAIL_DUTY = 75;
static int32_t MAIN_MIN_DUTY = 2;
static int32_t TAIL_MIN_DUTY = 2;

void initPWM(void);

void startMainRotor(void);

void stopMainRotor(void);

void startTailRotor(void);

void stopTailRotor(void);

void setMainPower(int32_t power);

void setTailPower(int32_t power);

int32_t getMainPower(void);

int32_t getTailPower(void);

#endif /*PWM_H_*/

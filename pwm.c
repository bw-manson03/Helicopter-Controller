//*****************************************************************************
//
// PWM.c - Module for controlling the helicopter's motor's PWM cycles
//
// Author:  bma206, tki36
// Last modified:   12.5.2024
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "pwm.h"


#define PWM_DIVIDER_CODE   SYSCTL_PWMDIV_4
#define PWM_DIVIDER        4

// Main rotor pwm pin/output config
#define PWM_MAIN_BASE	     PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM	 SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN    GPIO_PIN_5
#define PWM_MAIN_FREQUENCY   250

// Tail rotor pwm pin/output config
#define PWM_TAIL_BASE	     PWM1_BASE
#define PWM_TAIL_GEN         PWM_GEN_2
#define PWM_TAIL_OUTNUM      PWM_OUT_5
#define PWM_TAIL_OUTBIT      PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM	 SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE   GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG GPIO_PF1_M1PWM5
#define PWM_TAIL_GPIO_PIN    GPIO_PIN_1
#define PWM_TAIL_FREQUENCY   250



// Current tail and main rotor pwm
static int32_t mainPower = 0;
static int32_t tailPower = 0;

#define MAX_TAIL_CHANGE 5

//
// Initialises rotors
//
void initPWM(void)
{
    SysCtlPWMClockSet(PWM_DIVIDER_CODE);

    // initialise the main rotor
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);
    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);
    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false);

    // initialise the TAIL rotor
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_GPIO);
    GPIOPinConfigure(PWM_TAIL_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_TAIL_GPIO_BASE, PWM_TAIL_GPIO_PIN);
    PWMGenConfigure(PWM_TAIL_BASE, PWM_TAIL_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false);

}

//
// Starts main rotor
//
void startMainRotor(void)
{
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);
}

//
// Stops main rotor
//
void stopMainRotor(void)
{
    setMainPower(0);
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false);
}

//
// Starts tail rotor
//
void startTailRotor(void)
{
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);
}

//
// Stops tail rotor
//
void stopTailRotor(void)
{
    setTailPower(0);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false);
}

//
// Function to set the freq, duty cycle of the main rotor
//
void setMainPower (int32_t power)
{
    int32_t new_power = power;

    // Check the power isn't out of range
    if (power > MAX_DUTY) {
        new_power = MAX_DUTY;
    } else if (power < MAIN_MIN_DUTY) {
        new_power = MAIN_MIN_DUTY;
    }

    // Set the new pwm
    uint32_t pulsePeriod = SysCtlClockGet() / PWM_DIVIDER / PWM_MAIN_FREQUENCY;
    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, pulsePeriod);
    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, (pulsePeriod * new_power / 100));
    mainPower = new_power;
}

//
// Function to set the freq, duty cycle of the tail rotor
//
void setTailPower (int32_t power)
{
    int32_t new_power = power;

    // Check the power isn't out of range
    if (power > MAX_TAIL_DUTY) {
        new_power = MAX_TAIL_DUTY;
    } else if (power < TAIL_MIN_DUTY) {
        new_power = TAIL_MIN_DUTY;
    }
    // Set the new pwm
    uint32_t pulsePeriod = SysCtlClockGet() / PWM_DIVIDER / PWM_TAIL_FREQUENCY;
    PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, pulsePeriod);
    PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM, pulsePeriod * new_power / 100);
    tailPower = new_power;
}

//
// Get the main rotor's duty cycle
//
int32_t getMainPower(void)
{
    return mainPower;
}

//
// Get the tail rotor's duty cycle
//
int32_t getTailPower(void)
{
    return tailPower;
}

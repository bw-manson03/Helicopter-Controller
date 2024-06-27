//*****************************************************************************
//
// display.c - Controls the screen for outputing the relevant information
// of the heli  
//
// Author:  bma206, tki36
// Last modified:   21.5.2024
//
//*****************************************************************************

#include "OrbitOLED/OrbitOLEDInterface.h"
#include "display.h"
#include "altitude.h"
#include <stdint.h>
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
#include "OrbitOled/lib_OrbitOled/OrbitOled.h"
#include "yaw.h"
#include "pwm.h"



//
// Altiude as different value types
// for the display output use only!
//
int16_t altitudePercentage;
int16_t altitude;
int16_t baseAltitude;
uint32_t mainDuty;
uint32_t tailDuty;

// Current yaw
int16_t currentYaw = 0;

//
// intialise the Orbit OLED display
//
void initDisplay (void)
{
    OLEDInitialise ();
}

//
// Clears the display
//
void clearDisplay(void)
{
    OrbitOledClear();
}


//
// Displays the current altitude as a percentage of the maximum altitude using an ADC value
//
void displayPerVal(int16_t perValue, int16_t yawValue, uint32_t tailDuty, uint32_t mainDuty)
{
    char string[17];  // 16 characters across the display

    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf (string, sizeof(string), "Alt = %4d%%", perValue);
    OLEDStringDraw (string, 0, 0);

    usnprintf (string, sizeof(string), "YAW = %4d.%d", yawValue/10, abs(yawValue%10));
    OLEDStringDraw (string, 0, 1);

    usnprintf (string, sizeof(string), "Main duty = %3d%%", mainDuty);
    OLEDStringDraw (string, 0, 2);

    usnprintf (string, sizeof(string), "Tail duty = %3d%%", tailDuty);
    OLEDStringDraw (string, 0, 3);
}


//
// Updates/outputs the display
//
void updateDisplay(void)
{
    // Gets the values to display
    altitude = getAltitudeADC();
    currentYaw = getCurrentYaw();
    mainDuty = getMainPower();
    tailDuty = getTailPower();

    // Display altitude as a percentage
    getAltitudePercentage(altitude, &altitudePercentage);
    displayPerVal (altitudePercentage, currentYaw, tailDuty, mainDuty);
}

//*****************************************************************************
//
// altitude.c - altitude calculator for ADC
// Stores altitude and is used to set the target altitude
//
// Author:  bma206, tki36
// Last modified:   21.5.2024
//
//*****************************************************************************
// Based on 'lab 4 ADCdemo1.c' from 2024  
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "circBufT.h"
#include "altitude.h"
#include "switch.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

#define MAX_ALTITUDE 100
#define MIN_ALTITUDE 0

// Altitude range for 1V
static int32_t altitudeRange = 1240; //1190;
// Altitude circular buffer
static circBuf_t g_inBuffer;

int32_t baseAltitude;   // Base altitude
int32_t altitudePercentage;
int32_t targetAltitude = 0;

//
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
//
void ADCIntHandler()
{
    uint32_t ulValue;

    //
    // Get the single sample from ADC0.  ADC_BASE is defined in
    // inc/hw_memmap.h
    ADCSequenceDataGet(ADC_BASE, 3, &ulValue);
    //
    // Place it in the circular buffer (advancing write index)
    writeCircBuf (&g_inBuffer, ulValue);
    //
    // Clean up, clearing the interrupt
    ADCIntClear(ADC_BASE, 3);
}



void initADC(void)
{
    //
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
    ADCSequenceStepConfigure(ADC_BASE, 3, 0, ADC_HEIGHT_CHANNEL | ADC_CTL_IE |
                             ADC_CTL_END);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC_BASE, 3);

    //
    // Register the interrupt handler
    ADCIntRegister (ADC_BASE, 3, ADCIntHandler);

    //
    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC_BASE, 3);
}

//
// Initialises altitude
//
void initAltitude(void)
{
    initADC();
    initCircBuf(&g_inBuffer, BUF_SIZE);
}

//
// Background task: calculate the (approximate) mean of the values in the
// circular buffer and display it, together with the sample number.
//
int getAltitudeADC()
{
    uint16_t i;
    int32_t sum = 0;
    int32_t altitude = 0;
    // Reads the circ buffer to get the average
    for (i = 0; i < BUF_SIZE; i++)
        sum = sum + readCircBuf (&g_inBuffer);
    altitude = (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;
    return altitude;
}

//
// Calculates the percentage of the max altitude the helicopter is at
//
void getAltitudePercentage(int16_t altitude, int16_t* altPercentage)
{
    *altPercentage = (int16_t)(((baseAltitude - altitude) * 100) /  altitudeRange);
    altitudePercentage = (int32_t)(((baseAltitude - altitude) * 100) /  altitudeRange);
}


//
// Changes the target altitude by the increment amount, in percentage
//
void incrementAltitude(int16_t increment)
{
    // Inncrement target altitude
    targetAltitude += increment;
    // Check if new altitude is out of min/max range
    if (targetAltitude > MAX_ALTITUDE) {
        targetAltitude = MAX_ALTITUDE;
    } else if (targetAltitude < MIN_ALTITUDE) {
        targetAltitude = MIN_ALTITUDE;
    }
}

//
// Sets the base altitude
//
void setBaseAltitude(int32_t base)
{
    baseAltitude = base;
}

//
// Sets the target altitude
//
void setTargetAltitude(int32_t t_altitude)
{
    targetAltitude = t_altitude;
}

//
// returns the current base altitude
//
int32_t getBaseAltitude(void)
{
    return baseAltitude;
}


//
// returns the current target altitude
//
int32_t getTargetAltitude(void)
{
    return targetAltitude;
}

//
// Returns current altitude as a percentage
//
int32_t getAltitude(void)
{
    int16_t alt;
    int16_t altADC = getAltitudeADC();
    getAltitudePercentage(altADC, &alt);

    return alt;
}

//
// Gets the difference between target and current altitude
//
int32_t getAltitudeError(void)
{
    return targetAltitude - altitudePercentage;
}

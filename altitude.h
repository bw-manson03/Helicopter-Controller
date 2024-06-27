#ifndef ALTITUDE_H_
#define ALTITUDE_H_

//*****************************************************************************
//
// altitude.h - Module for calculating the altitude and 
// for setting the altitude
//
// Author:  bma206, tki36
// Last modified:   21.5.2024
//
//*****************************************************************************


#define BUF_SIZE 20                     // Altitude circ buffer size
#define ADC_HEIGHT_CHANNEL ADC_CTL_CH9  // Altitude input channel
#define ADC_BASE ADC0_BASE              // Altitude input channel base


#include <stdint.h>

//
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
//
void ADCIntHandler(void);

//
// Initialises altitude
//
void initAltitude(void);

//
// Background task: calculate the (approximate) mean of the values in the
// circular buffer and display it, together with the sample number.
//
int getAltitudeADC(void);

//
// Calculates the percentage of the max altitude the helicopter is at
//
void getAltitudePercentage(int16_t altitude, int16_t* altPercentage);


//
// Changes the target altitude by the increment amount, in percentage
//
void incrementAltitude(int16_t increment);

//
// Sets the base altitude of the helicopter
//
void setBaseAltitude(int32_t base);

//
// returns the current base altitude
//
int32_t getBaseAltitude(void);

//
// Gets the difference between target and current altitude
//
int32_t getAltitudeError(void);

//
// returns the current target altitude
//
int32_t getTargetAltitude(void);

//
// Sets the target altitude
//
void setTargetAltitude(int32_t t_altitude);

#endif /*ALTITUDE_H_*/

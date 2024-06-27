//*****************************************************************************
//
// serial.h - Header module for sending serial communication through the USB
//
// Author:  bma206, tki36
// Last modified:   21.5.2024
//
//*****************************************************************************


#ifndef SERIAL_H
#define SERIAL_H

//
// Initialises the serial communation
//
void initUART (void);

//
// Gets the info of the heli and sends it through the usb
//
void UARTSendData(void);

#endif // SERIAL_H

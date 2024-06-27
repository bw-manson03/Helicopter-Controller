//*****************************************************************
//
// display.h - header file for display function, with initialisation
// and updating/changing the display as public functions.
//
// Authors: bma206, tki36
// Date modified: 30.4.24
//
//*****************************************************************

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

//
// intialise the Orbit OLED display
//
void initDisplay (void);

//
// Updates the display with the latest info
//
void updateDisplay(void);

#endif /* DISPLAY_H_ */

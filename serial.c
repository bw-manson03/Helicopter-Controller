//*****************************************************************************
//
// serial.c - Module for sending serial communication through the USB
//
// Author:  bma206, tki36
// Last modified:   21.5.2024
//
//*****************************************************************************
// Based on week 4 lab uartDemo.c 
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "utils/ustdlib.h"
#include "stdio.h"
#include "stdlib.h"
#include "control.h"

#include "yaw.h"
#include "altitude.h"
#include "pwm.h"
#include "switch.h"



#define SYSTICK_RATE_HZ 100
#define SLOWTICK_RATE_HZ 4
#define MAX_STR_LEN 35
//---USB Serial comms: UART0, Rx:PA0 , Tx:PA1
#define BAUD_RATE 9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX


int16_t alt;



//
// Initialises the serial communation
// 
void initUART (void)
{
    // Enable GPIO port A which is used for UART0 pins (see Page 12, TivaTM4C123G Launchpad 
    // Evaluation Board_Users Guide.pdf). 
    SysCtlPeripheralEnable (SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOA);
    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART (GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk (UART0_BASE, SysCtlClockGet(), BAUD_RATE,
    UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | 
    UART_CONFIG_PAR_NONE);
    UARTFIFOEnable (UART0_BASE);
    UARTEnable (UART0_BASE);
}

//**********************************************************************
// Transmit a string via UART0
//**********************************************************************
void UARTSend (char *pucBuffer)
{
    // Loop while there are more characters to send.
    while(*pucBuffer)
    {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut(UART0_BASE, *pucBuffer);
        pucBuffer ++;
    }
}

//
// Gets the info of the heli and sends it through the usb
//
void UARTSendData(void)
{
    char string[MAX_STR_LEN + 1];

    int16_t altADC = getAltitudeADC();

    getAltitudePercentage(altADC, &alt);

    // Sends The Info through the serial
    usnprintf(string, sizeof(string), "yaw_d=%3d.%d |", getTargetYaw()/10, abs(getTargetYaw()%10));
    UARTSend(string);
    usnprintf(string, sizeof(string), "yaw=%3d.%d |", getCurrentYaw()/10, abs(getCurrentYaw()%10));
    UARTSend(string);
    usnprintf(string, sizeof(string), "alt_d=%3d |", getTargetAltitude());
    UARTSend(string);
    usnprintf(string, sizeof(string), "alt=%3d |", alt);
    UARTSend(string);
    // Gets the heli state and converts it to string
    switch(getHeliState()) {
        case LANDED:
            usnprintf(string, sizeof(string), "state=LANDED |");
            break;
        case FIND_YAW:
            usnprintf(string, sizeof(string), "state=FIND_YAW |");
            break;
        case FLYING:
            usnprintf(string, sizeof(string), "state=FLYING |");
            break;
        case RESET_YAW:
            usnprintf(string, sizeof(string), "state=RESET_YAW |");
            break;
        case LANDING:
            usnprintf(string, sizeof(string), "state=LANDING |");
            break;
    }
    UARTSend(string);

    usnprintf(string, sizeof(string), "tailPWM=%3d |", getTailPower());
    UARTSend(string);

    usnprintf(string, sizeof(string), "mainPWM=%3d |", getMainPower());
    UARTSend(string);

    usnprintf(string, sizeof(string), "yawER=%3d |", getYawError());
    UARTSend(string);

    usnprintf(string, sizeof(string), "yawDI=%3d |", (int32_t)getYI());
    UARTSend(string);




    usnprintf(string, sizeof(string), "\n");
    UARTSend(string);
}

/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        Fault LED header file.

   \details
        This file contains the function prototypes related to managing and
        displaying faults through the LED multiplexer.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2024/03/08 $  \n

 ******************************************************************************/

#ifndef _FAULT_H_
#define _FAULT_H_

//! Define macro for ATmega32U4 micro controller
#define __AVR_ATmega32U4__ 1

// Include AVR input/output definitions for low-level hardware control
#include <avr/io.h>

// Include utility functions for delays
#include <util/delay.h>

// Include standard integer type definitions
#include "stdint.h"

// Include motor header
#include "config.h"

// Function prototypes
void EnableOverCurrentLED(void);
void EnableMotorStoppedLED(void);
void EnableReverseRotationLED(void);
void EnableU3(void);
void EnableU2(void);
void EnableU1(void);
void EnableNoHallConnectionsLED(void);
void DisableFaultLEDs(void);
void SweepLEDsBlocking(void);
void faultSequentialStateMachine(volatile faultflags_t *faultFlags, volatile motorflags_t *motorFlags);

#endif /* _FAULT_H_ */

/* This file has been prepared for Doxygen automatic documentation generation. */
/*! \file ********************************************************************

   \brief
        Motor Control Tables.

   \details
        This file contains table definitions used for motor control, including block commutation
        masks, expected hall sensor sequences, and related settings.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2024/03/08 $  \n

******************************************************************************/

#ifndef _TABLES_H_
#define _TABLES_H_

//! Define macro for ATmega32U4 micro controller
#define __AVR_ATmega32U4__ 1

// Include main Arduino library for basic Arduino functions
#include <Arduino.h>

// Include standard integer type definitions
#include <stdint.h>

#ifdef __INTELLISENSE__
#define PROGMEM
#else
#include <avr/pgmspace.h>
#endif
#include "config.h"

/*! \brief Block Commutation Port Direction Masks for Forward Driving

    This array contains port and output compare override masks for block
    commutation when running in the forward direction. It defines how the timer
    controls the output compare (OC) pins and the output state of corresponding
    pins for each commutation step.

    The masks control the following:

    - The TCCR4E register (\ref OC_ENABLE_PORTB, \ref OC_ENABLE_PORTC, \ref
      OC_ENABLE_PORTD): This controls the output compare (OC) pins by enabling
      or disabling them for each commutation step. A '1' enables the
      corresponding OC pin for PWM output, while '0' disables it for normal port
      operation.

    - The PORTx registers (PORTB, PORTC, PORTD): These control the output state
      of the corresponding pins (\ref AL_PIN, \ref BL_PIN, \ref CL_PIN) for each
      commutation step. A '1' in the mask represents a pin set to a HIGH state,
      while '0' represents a pin set to a LOW state.


    \par Table:

    |  Hall | Phase A/PORTB | Phase B/PORTC | Phase C/PORTD |      TCCR4E       |
    |-------|---------------|---------------|---------------|-------------------|
    |  000  |      0        |      0        |      0        |        0          |
    |  001  |      1        |      0        |      0        | \ref OC_ENABLE_PORTD |
    |  010  |      0        |      1        |      0        | \ref OC_ENABLE_PORTB |
    |  011  |      0        |      0        |      1        | \ref OC_ENABLE_PORTC |
    |  100  |      1        |      0        |      0        | \ref OC_ENABLE_PORTC |
    |  101  |      0        |      0        |      1        | \ref OC_ENABLE_PORTB |
    |  110  |      0        |      1        |      0        | \ref OC_ENABLE_PORTD |
    |  111  |      1        |      0        |      0        |        0          |

    \note The array is stored in program memory instead of SRAM (denoted by the
    PROGMEM attribute). This is useful for storing large data arrays in micro
    controllers with limited RAM capacity.

*/
const uint8_t blockCommutationTableForward[32] PROGMEM =
    {
        0, 0, 0, 0,
        (1 << AL_PIN), (0 << BL_PIN), (0 << CL_PIN), OC_ENABLE_PORTD, // UL, WH // AL, CH // AL, CH
        (0 << AL_PIN), (1 << BL_PIN), (0 << CL_PIN), OC_ENABLE_PORTB, // UH, VL // AH, BL // CL, BH
        (0 << AL_PIN), (1 << BL_PIN), (0 << CL_PIN), OC_ENABLE_PORTD, // VL, WH // BL, CH // AL, BH
        (0 << AL_PIN), (0 << BL_PIN), (1 << CL_PIN), OC_ENABLE_PORTC, // VH, WL // BH, CL // BL, AH
        (1 << AL_PIN), (0 << BL_PIN), (0 << CL_PIN), OC_ENABLE_PORTC, // UL, VH // AL, BH // BL, CH
        (0 << AL_PIN), (0 << BL_PIN), (1 << CL_PIN), OC_ENABLE_PORTB, // UH, WL // AH, CL // CL, AH
        0, 0, 0, 0};

/*! \brief Block Commutation Port Direction Masks for Reverse Driving

    This array contains port and output compare override masks for block
    commutation when running in the reverse direction. It defines how the timer
    controls the output compare (OC) pins and the output state of corresponding
    pins for each commutation step.

    The masks control the following:

    - The TCCR4E register (\ref OC_ENABLE_PORTB, \ref OC_ENABLE_PORTC, \ref
      OC_ENABLE_PORTD): This controls the output compare (OC) pins by enabling
      or disabling them for each commutation step. A '1' enables the
      corresponding OC pin for PWM output, while '0' disables it for normal port
      operation.

    - The PORTx registers (PORTB, PORTC, PORTD): These control the output state
      of the corresponding pins (\ref AL_PIN, \ref BL_PIN, \ref CL_PIN) for each
      commutation step. A '1' in the mask represents a pin set to a HIGH state,
      while '0' represents a pin set to a LOW state.

    \par Table:

    |  Hall | Phase A/PORTB | Phase B/PORTC | Phase C/PORTD |      TCCR4E       |
    |-------|---------------|---------------|---------------|-------------------|
    |  000  |      0        |      0        |      0        |        0          |
    |  001  |      1        |      0        |      0        | \ref OC_ENABLE_PORTD |
    |  010  |      0        |      1        |      0        | \ref OC_ENABLE_PORTB |
    |  011  |      0        |      0        |      1        | \ref OC_ENABLE_PORTC |
    |  100  |      1        |      0        |      0        | \ref OC_ENABLE_PORTC |
    |  101  |      0        |      0        |      1        | \ref OC_ENABLE_PORTB |
    |  110  |      0        |      1        |      0        | \ref OC_ENABLE_PORTD |
    |  111  |      1        |      0        |      0        |        0          |

    \note The array is stored in program memory (PROGMEM) instead of SRAM. This
    is useful for storing large data arrays in micro controllers with limited
    RAM capacity.
*/
const uint8_t blockCommutationTableReverse[32] PROGMEM =
    {
        0, 0, 0, 0,
        (0 << AL_PIN), (0 << BL_PIN), (1 << CL_PIN), OC_ENABLE_PORTB, // UH, WL // AH, CL // CL, AH
        (1 << AL_PIN), (0 << BL_PIN), (0 << CL_PIN), OC_ENABLE_PORTC, // UL, VH // AL, BH // BL, CH
        (0 << AL_PIN), (0 << BL_PIN), (1 << CL_PIN), OC_ENABLE_PORTC, // VH, WL // BH, CL // BL, AH
        (0 << AL_PIN), (1 << BL_PIN), (0 << CL_PIN), OC_ENABLE_PORTD, // VL, WH // BL, CH // AL, BH
        (0 << AL_PIN), (1 << BL_PIN), (0 << CL_PIN), OC_ENABLE_PORTB, // UH, VL // AH, BL // CL, BH
        (1 << AL_PIN), (0 << BL_PIN), (0 << CL_PIN), OC_ENABLE_PORTD, // UL, WH // AL, CH // AL, CH
        0, 0, 0, 0};

/*! \brief Table of Expected Hall Sensor Values in Forward Direction

    This array represents the expected next hall sensor value when the motor is
    running in the forward direction. Each element in the array corresponds to
    the next anticipated hall sensor value, indexed by the current hall sensor
    value.

    For example, if the current hall sensor value is '2', the next expected hall
    sensor value in the forward direction is '6'.
*/
const uint8_t expectedHallSequenceForward[7] PROGMEM =
    {
        0xff, 3, 6, 2, 5, 1, 4};

/*! \brief Table of Expected Hall Sensor Values in Reverse Direction

    This array represents the expected next hall sensor value when the motor is
    running in the reverse direction. Each element in the array corresponds to
    the next anticipated hall sensor value, indexed by the current hall sensor
    value.

    For example, if the current hall sensor value is '2', the next expected hall
    sensor value in the reverse direction is '3'.
*/
const uint8_t expectedHallSequenceReverse[7] PROGMEM =
    {
        0xff, 5, 3, 1, 6, 4, 2};

#endif /* _TABLES_H_ */

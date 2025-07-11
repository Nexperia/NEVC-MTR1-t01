/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        Fault LED source file.

   \details
        This file contains all functions necessary for managing and displaying
        faults through the LED multiplexer.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2024/03/08 $  \n

 ******************************************************************************/

// Include fault header
#include "fault.h"

/**
   \brief Enables the over current fault LED

   \note As this feeds to a multiplexer, other LEDs are disabled.

   Sets: FAULT_BIT3 = 0 FAULT_BIT2 = 0 FAULT_BIT1 = 1
*/
void EnableOverCurrentLED(void)
{
  PORTB &= ~((1 << FAULT_PIN_3) | (1 << FAULT_PIN_2));
  PORTD |= (1 << FAULT_PIN_1);
}

/**
    \brief Enables the motor stopped fault LED

   \note As this feeds to a multiplexer, other LEDs are disabled.

   Sets: FAULT_BIT3 = 0 FAULT_BIT2 = 1 FAULT_BIT1 = 0

*/
void EnableMotorStoppedLED(void)
{
  PORTB &= ~(1 << FAULT_PIN_3);
  PORTB |= (1 << FAULT_PIN_2);
  PORTD &= ~(1 << FAULT_PIN_1);
}

/**
    \brief Enables the reverse rotation fault LED

   \note As this feeds to a multiplexer, other LEDs are disabled.

   Sets: FAULT_BIT3 = 0 FAULT_BIT2 = 1 FAULT_BIT1 = 1

*/
void EnableReverseRotationLED(void)
{
  PORTB &= ~(1 << FAULT_PIN_3);
  PORTB |= (1 << FAULT_PIN_2);
  PORTD |= (1 << FAULT_PIN_1);
}

/**
    \brief Enables the user function 3 LED

   \note As this feeds to a multiplexer, other LEDs are disabled.

   Sets: FAULT_BIT3 = 1 FAULT_BIT2 = 0 FAULT_BIT1 = 0

*/
void EnableU3(void)
{
  PORTB |= (1 << FAULT_PIN_3);
  PORTB &= ~(1 << FAULT_PIN_2);
  PORTD &= ~(1 << FAULT_PIN_1);
}

/**
    \brief Enables the user function 2 LED

   \note As this feeds to a multiplexer, other LEDs are disabled.

   Sets: FAULT_BIT3 = 1 FAULT_BIT2 = 0 FAULT_BIT1 = 1

*/
void EnableU2(void)
{
  PORTB |= (1 << FAULT_PIN_3);
  PORTB &= ~(1 << FAULT_PIN_2);
  PORTD |= (1 << FAULT_PIN_1);
}

/**
    \brief Enables the user function 1 LED

   \note As this feeds to a multiplexer, other LEDs are disabled.

   Sets: FAULT_BIT3 = 1 FAULT_BIT2 = 1 FAULT_BIT1 = 0

*/
void EnableU1(void)
{
  PORTB |= (1 << FAULT_PIN_3) | (1 << FAULT_PIN_2);
  PORTD &= ~(1 << FAULT_PIN_1);
}

/**
    \brief Enables the no hall connection fault LED

   \note As this feeds to a multiplexer, other LEDs are disabled.

   Sets: FAULT_BIT3 = 1 FAULT_BIT2 = 1 FAULT_BIT1 = 1

*/
void EnableNoHallConnectionsLED(void)
{
  PORTB |= (1 << FAULT_PIN_3) | (1 << FAULT_PIN_2);
  PORTD |= (1 << FAULT_PIN_1);
}

/**
    \brief Disables all LEDs

   Sets: FAULT_BIT3 = 0 FAULT_BIT2 = 0 FAULT_BIT1 = 0

*/
void DisableFaultLEDs(void)
{
  PORTB &= ~((1 << FAULT_PIN_3) | (1 << FAULT_PIN_2));
  PORTD &= ~(1 << FAULT_PIN_1);
}

/**
    \brief Sweeps through all LEDs individually with a delay

   \note This is a blocking function. Do not use while motor is operational.

*/
void SweepLEDsBlocking(void)
{
  EnableU3();
  _delay_ms(100);

  EnableU2();
  _delay_ms(100);

  EnableU1();
  _delay_ms(100);

  EnableNoHallConnectionsLED();
  _delay_ms(100);

  EnableOverCurrentLED();
  _delay_ms(100);

  EnableMotorStoppedLED();
  _delay_ms(100);

  EnableReverseRotationLED();
  _delay_ms(100);
}

/**
   \brief Sequential State Machine for Handling Fault Flags.

   This function implements a sequential state machine for handling fault flags
   and controlling corresponding LEDs or indicators. It sequentially checks the
   various fault flags and enables/disables LEDs or indicators based on the
   current fault flag conditions.

   \param faultFlags A pointer to the volatile faultflags_t structure containing
                    the fault flags to be checked.
    \param motorFlags A pointer
                    to the volatile motorflags_t structure containing motor
                    control flags (e.g., motor enable status).
*/
void faultSequentialStateMachine(volatile faultflags_t *faultFlags, volatile motorflags_t *motorFlags)
{
  static uint8_t state = 1;

  switch (state)
  {
  case 1:
    if (faultFlags->motorStopped == TRUE && motorFlags->enable == TRUE)
    {
      EnableMotorStoppedLED();
    }
    else
    {
      DisableFaultLEDs();
    }
    state = 2;
    break;
  case 2:
    if (faultFlags->reverseDirection == TRUE && faultFlags->motorStopped == FALSE)
    {
      EnableReverseRotationLED();
    }
    else
    {
      DisableFaultLEDs();
    }
    state = 3;
    break;
  case 3:
    if (faultFlags->noHallConnections == TRUE)
    {
      EnableNoHallConnectionsLED();
    }
    else
    {
      DisableFaultLEDs();
    }
    state = 4;
    break;
  case 4:
    if (faultFlags->overCurrent == TRUE)
    {
      EnableOverCurrentLED();
    }
    else
    {
      DisableFaultLEDs();
    }
    state = 5;
    break;
  case 5:
    if (faultFlags->userFlag1 == TRUE)
    {
      EnableU1();
    }
    else
    {
      DisableFaultLEDs();
    }
    state = 6;
    break;
  case 6:
    if (faultFlags->userFlag2 == TRUE)
    {
      EnableU2();
    }
    else
    {
      DisableFaultLEDs();
    }
    state = 7;
    break;
  case 7:
    if (faultFlags->userFlag3 == TRUE)
    {
      EnableU3();
    }
    else
    {
      DisableFaultLEDs();
    }
    state = 8;
    break;
  default:
    // No active fault flags; disable all LEDs or indicators.
    DisableFaultLEDs();
    state = 1;
  }
}

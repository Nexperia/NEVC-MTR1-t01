/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file ********************************************************************

   \brief
        Motor config header file.

   \details
        This file contains all defines, typedefs, and prototypes related to
        the motor control.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2024/03/08 $  \n

 ******************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

//! Define macro for ATmega32U4 micro controller
#define __AVR_ATmega32U4__ 1

// Include main Arduino library for basic Arduino functions
#include <Arduino.h>

// Include AVR input/output definitions for low-level hardware control
#include <avr/io.h>

// Define ISR macro for IntelliSense, else include AVR interrupt handling
// library
#ifdef __INTELLISENSE__
#define ISR(vector) void vector(void)
#else
#include <avr/interrupt.h>
#endif

// Define __LPM macro for IntelliSense, else include AVR program space utility
// library
#ifdef __INTELLISENSE__
#define __LPM(x) 0
#else
#include <avr/pgmspace.h>
#endif

// Include math definitions
#include <math.h>

// Include standard integer type definitions
#include <stdint.h>

/**
   \addtogroup ConfigDefines Motor Configuration Definitions

   \brief Collection of definitions that control system behavior.

   These are definitions that exist inside \ref config.h contains that control the
   behavior of the system. The configuration definitions are organized into
   different groups based on their purpose and modifiability.
*/

/*!
   \defgroup UserSettable User Settable Defines

   \brief These defines can be modified by the user.

   \ingroup ConfigDefines
   @{
*/

/*!
   \brief Number of poles in the motor.

   This macro defines the number of poles in the motor. The value is used to
   calculate the motor speed.

   The motor provided with the kit, 42BLS40-24-01, has 8 poles.

   \todo Specify the number of poles of the specific motor being used.
*/
#define MOTOR_POLES 8

/*!
   \brief Desired Switching Frequency for MOSFET Gate Signals

   This macro defines the desired switching frequency for the MOSFET gate
   signals. The resolution of the duty cycle is changes with the selected
   switching frequency. The best pre-scaler is chose to provide the best
   resolution.

  The absolute PWM Resolution is calculated using the following formula: \f[
   \text{Absolute PWM Resolution (%)} = \frac{100}{(F_{HST} / (TIM4_{FREQ}
   \times TIM4_{PRESCALER} \times 2)) - 1} \f]

  Where:
    - \ref F_HST : High-speed system clock frequency, configured to be 64 MHz.
    - \ref  TIM4_FREQ : The required gate switching frequency.
    - TIM4_PRESCALER : The pre-scaler value, which depends on the gate switching
      frequency as explained below.

  The TIM4 Pre-scaler is selected based on the gate switching frequency (\ref
  TIM4_FREQ):
    - For frequencies \f$\geq\f$ 31250 Hz, the pre-scaler is set to 1.
    - For frequencies < 31250 Hz and \f$\geq\f$ 15625 Hz, the pre-scaler is set
      to 2.
    - For frequencies < 15625 Hz and \f$\geq\f$ 7813 Hz, the pre-scaler is set
      to 4.

   \image html resolution_vs_frequency_dark.svg "Figure: Absolute PWM resolution across all supported gate switching frequencies."

   \warning The maximum recommended value for TIM4_FREQ is 100 kHz. Please
   review this value and uncomment the advisory warning if necessary. You may
   also need to change the values in the scpi.cpp file.

   \warning The minimum value for TIM4_FREQ is 7813 Hz as the higher pre-scalers
   needed to support this have not been implemented.

   \todo Specify the desired gate switching frequency.
*/
#define TIM4_FREQ 20000UL
#if TIM4_FREQ > 100000UL
#error "ADVISORY WARNING: TIM4_FREQ should not be set above 100 kHz. If you want to still continue, please uncomment and compile again."
#endif
#if TIM4_FREQ < 7183UL
#error "ADVISORY WARNING: TIM4_FREQ should not be set below 7183 Hz. If you want to still continue, you'll have to modify the code to allow for a higher pre-scaler value for Timer4."
#endif

/*!
   \brief Dead Time Specification

   This macro specifies the desired dead time between switching actions in
   nanoseconds (ns). The resolution of the dead time changes with the selected
   dead time. The best pre-scaler is chose to provide the best resolution.

   The following are the resolution for different dead time ranges:
   - For 0 to 234 ns, resolution is 16 ns.
   - For 235 to 468 ns, resolution is 31 ns.
   - For 469 to 937 ns, resolution is 63 ns.
   - For 938 to 1875 ns, resolution is 125 ns.

   \warning The minimum recommended dead time in the set up provided is 350 ns.
   Please review this value and uncomment the advisory warning if necessary.

   \warning The maximum allowed dead time is 1875 ns. Larger dead times are not
   supported by the timer peripheral of ATMEGA32u4.

   \todo Specify the desired dead time in nanoseconds.
*/
#define DEAD_TIME 350UL
#if DEAD_TIME < 350
#error "ADVISORY WARNING: DEAD_TIME should not be set below 350 ns. If you want to still continue, please uncomment and compile again."
#endif

/*!
   \brief Internal Pull-up Resistor Configuration for Hall Sensor Inputs

   This macro configures whether internal pull-up resistors should be enabled on
   hall sensor inputs.

   \todo Enable or disable internal pull-ups on hall sensor inputs by setting to
   \ref TRUE or \ref FALSE.

*/
#define HALL_PULLUP_ENABLE TRUE

/*!
   \brief Emulate Motor Spinning

   When enabled, this flag transforms Hall effect sensor inputs into
   hardware-generated outputs, initiating a motor sequencing process.

   \note This will not work if \ref SPEED_CONTROL_METHOD is set to \ref
   SPEED_CONTROL_CLOSED_LOOP.

   \warning Do not have hall sensors connected while this feature is set to \ref
   TRUE.

   \todo Enable or disable motor emulation by setting to \ref TRUE or \ref
   FALSE.

   \see TIM3_FREQ
*/
#define EMULATE_HALL FALSE

/*!
   \brief Desired Electrical Rotational Frequency for "Emulated" Motor

   This macro specifies the desired electrical rotational frequency for the
   "emulated" motor. The valid range is from 5.08 Hz to 167.67 KHz.

   \note As a general rule of thumb, you should keep the electrical rotational
   frequency switching lower than at least 1/10th of the electrical rotational
   frequency.

   \todo Specify the desired electrical rotational frequency in Hz.

   \see EMULATE_HALL
*/
#define TIM3_FREQ 200UL

/*!
   \brief Commutation Stopped Limit

   This macro defines the number of commutation 'ticks' that must pass without
   any hall changes before the motor is considered to be stopped.

   \todo Define how many 'ticks' before the motor is considered stopped.
*/
#define COMMUTATION_TICKS_STOPPED 6000

/*!
   \brief Turn Off Mode

   Set this macro to either \ref TURN_OFF_MODE_COAST <s>or \ref TURN_OFF_MODE_RAMP</s>
   to specify the desired turn mode.

   \todo Select the turn mode by assigning \ref TURN_OFF_MODE_COAST <s>or \ref
   TURN_OFF_MODE_RAMP</s>.

   \see TURN_OFF_MODE_RAMP, TURN_OFF_MODE_COAST
*/
#define TURN_OFF_MODE TURN_OFF_MODE_COAST

/*!
   \brief In-line Phase Current Gain for Current Measurement

   This macro defines the gain factor used in the current measurement circuit.
   The gain is a unit-less multiplier that amplifies the signal from the current
   sensor before it is read by the controller. It is used in the calculation to
   convert the sensor reading into an actual current value.

   The NEVB-MTR1-I56-1 comes with a single gain factor option for the in-line
   phase current measurement circuit - 20.

   \note Ensure that the gain value is correct.

   \see IPHASE_SENSE_RESISTOR
*/
#define IPHASE_GAIN 20

/*!
   \brief Hi-side Current (IBUS) Sense Resistor Value

   This macro specifies the resistance value of the current sense resistor in
   the system, measured in micro-ohms (μΩ). The value is used in conjunction
   with the current gain to calculate the actual current based on the voltage
   across the current sense resistor.

   The NEVB-MTR1-I56-1 comes with a in-line phase current sense resistors of
   value 2.5 mΩ. This is the default value.

   \note Ensure that the resistor value matches the actual hardware component
   used.

   \see IPHASE_GAIN
*/
#define IPHASE_SENSE_RESISTOR 2500

/*!
   \brief Hi-side Current (IBUS) Gain for Current Measurement

   This macro defines the gain factor used in the current measurement circuit.
   The gain is a unit-less multiplier that amplifies the signal from the current
   sensor before it is read by the controller. It is used in the calculation to
   convert the sensor reading into an actual current value.

   The NEVB-MTR1-I56-1 comes with two current op-amps with different gain factors:
   1. Gain factor of 50 - This is the default value.
   2. Gain factor of 20 - This is the alternative value.

   \image html current_sense_select.png width=70%

   \note Ensure that the gain value is correct.

   \see IBUS_SENSE_RESISTOR
*/
#define IBUS_GAIN 50

/*!
   \brief Hi-side Current (IBUS) Sense Resistor Value

   This macro specifies the resistance value of the current sense resistor in
   the system, measured in micro-ohms (μΩ). The value is used in conjunction
   with the current gain to calculate the actual current based on the voltage
   across the current sense resistor.

   The NEVB-MTR1-I56-1 comes with a current sense resistor of value 4 mΩ. This
   is the default value.

   \note Ensure that the resistor value matches the actual hardware component
   used.

   \see IBUS_GAIN
*/
#define IBUS_SENSE_RESISTOR 4000

/*!
   \brief Hi-side Current (IBUS) Warning Threshold (Register Value)

   This macro specifies the threshold value for current warning as a register
   value. When the current measured by the controller exceeds this threshold, it
   triggers the fault LED to be turned on.

   The range is 0-1023.

   This value is not scaled and represents the raw register value. To obtain the
   register value from the current in amperes, you can use the formula:

   \f[ \text{Register Value} = \frac{\text{CURRENT} \times
      \text{IBUS_SENSE_RESISTOR} \times \text{IBUS_GAIN}}{0.004888
      \times 1000000} \f]

   Where:
     - CURRENT : The current threshold in amperes.
     - 0.004888 : The conversion factor for a 10-bit ADC with a Vref of 5V.
     - \ref IBUS_GAIN : The gain of the current sense operational amplifier.
     - \ref IBUS_SENSE_RESISTOR : The value of the shunt resistor in
       micro-ohms (μΩ).

   The NEVB-MTR1-I56-1 comes with a default current gain factor of 50 and
   a current sense resistor of value 4 mΩ. This corresponds to approximately
   0.0244 amperes (A) per register value. The default value is 103 which
   corresponds to approximately 2.5 A.

   \todo Calculate and set the register value for the current warning threshold.

   \see IBUS_ERROR_THRESHOLD, IBUS_FAULT_ENABLE, IBUS_GAIN,
   IBUS_SENSE_RESISTOR
*/
#define IBUS_WARNING_THRESHOLD 103

/*!
   \brief Hi-side Current (IBUS) Error Threshold (Register Value)

   This macro specifies the threshold value for current error as a register
   value. When the current measured by the controller exceeds this threshold, it
   triggers the controller to take protective action, by disabling all PWM
   outputs, essentially allowing the motor to coast.

   The range is 0-1023.

   This value is not scaled and represents the raw register value. To obtain the
   register value from the current in amperes, you can use the formula:

   \f[ \text{Register Value} = \frac{\text{CURRENT} \times
      \text{IBUS_SENSE_RESISTOR} \times \text{IBUS_GAIN}}{0.004888
      \times 1000000} \f]

   Where:
     - CURRENT : The current threshold in amperes.
     - 0.004888 : The conversion factor for a 10-bit ADC with a Vref of 5V.
     - \ref IBUS_GAIN : The gain of the current sense operational amplifier.
     - \ref IBUS_SENSE_RESISTOR : The value of the shunt resistor in
       micro-ohms (μΩ).

   The NEVB-MTR1-I56-1 comes with a default current gain factor of 50 and
   a current sense resistor of value 4 mΩ. This corresponds to approximately
   0.0244 amperes (A) per register value. The default value is 614 which
   corresponds to approximately 15 A.

   \note Braking is not implemented yet for this fault, so the motor coasts when
   an error occurs.

   \todo Calculate and set the register value for the current error threshold.

   \see IBUS_WARNING_THRESHOLD, IBUS_FAULT_ENABLE, IBUS_GAIN,
   IBUS_SENSE_RESISTOR
*/
#define IBUS_ERROR_THRESHOLD 614

/*!
   \brief Hi-side Current (IBUS) Fault Enable

   This macro sets if any action is taken if \ref IBUS_ERROR_THRESHOLD is
   exceeded.

   \note This does not affect fault reporting for \ref
   IBUS_WARNING_THRESHOLD.

   \see IBUS_ERROR_THRESHOLD
*/
#define IBUS_FAULT_ENABLE FALSE

/*!
   \brief Speed Control Method

   Select the type of speed control by setting this macro to either \ref
   SPEED_CONTROL_OPEN_LOOP or \ref SPEED_CONTROL_CLOSED_LOOP.

   \todo Select the speed control method by assigning \ref
         SPEED_CONTROL_OPEN_LOOP or \ref SPEED_CONTROL_CLOSED_LOOP.

   \see SPEED_CONTROLLER_TIME_BASE, SPEED_CONTROLLER_MAX_DELTA,
        SPEED_CONTROLLER_MAX_SPEED, PID_K_P, PID_K_I, PID_K_D_ENABLE, PID_K_D
*/
#define SPEED_CONTROL_METHOD SPEED_CONTROL_OPEN_LOOP

/*!
   \brief Speed Controller Time Base

   This macro specifies the number of `ticks` between each iteration of the
   speed loop. One 'tick' is one PWM period. Adjust this value to set the speed
   control loop time base. Range is 1-255.

   \note Minimum overhead of atleast 1 us due to a blocking delay placed inside
   the \ref SpeedController function.

   \see SPEED_CONTROL_METHOD, SPEED_CONTROLLER_MAX_DELTA,
        SPEED_CONTROLLER_MAX_SPEED, PID_K_P, PID_K_I, PID_K_D_ENABLE, PID_K_D
*/
#define SPEED_CONTROLLER_TIME_BASE 200

/*!
   \brief Speed Controller Maximum Delta (Applicable for Open Loop Control)

   This macro specifies the maximum allowed change in speed reference by the
   speed controller after each iteration of the speed loop when using open-loop
   speed control. Adjust this value as needed. When a user changes the requested
   speed input, this parameter limits the maximum allowed change per
   SPEED_CONTROLLER_TIME_BASE.

   \note This parameter is applicable when \ref SPEED_CONTROL_METHOD is set to
   \ref SPEED_CONTROL_OPEN_LOOP.

   \todo Adjust the value to set the desired maximum change in speed reference
         for open-loop control according to your system's speed response
         requirements.

   \see SPEED_CONTROL_METHOD, SPEED_CONTROLLER_TIME_BASE,
        SPEED_CONTROLLER_MAX_SPEED, PID_K_P, PID_K_I, PID_K_D_ENABLE, PID_K_D
*/
#define SPEED_CONTROLLER_MAX_DELTA 1

/*!
   \brief Speed Controller Maximum Speed

   This macro specifies the maximum speed, to be used as a setpoint when the
   maximum speed reference value is input to the speed controller. This "speed"
   is given as rate of change of the hall effect sensors in Hz.

   This parameter is crucial for ensuring that the speed controller does not
   attempt to change the motor speed beyond a safe and controlled rate.

   \note This parameter is applicable when \ref SPEED_CONTROL_METHOD is set to
   \ref SPEED_CONTROL_CLOSED_LOOP.

   \todo Set to the maximum speed based on the system's safety and operational
   requirements.

   \see SPEED_CONTROL_METHOD, SPEED_CONTROLLER_TIME_BASE,
        SPEED_CONTROLLER_MAX_DELTA, PID_K_P, PID_K_I, PID_K_D_ENABLE, PID_K_D
*/
#define SPEED_CONTROLLER_MAX_SPEED 400

/*!
   \brief PID Controller Proportional Gain Constant (Only for Closed Loop)

   This macro specifies the proportional gain constant for the PID controller.
   This parameter accepts signed integer values in the int16_t range.

   \note This parameter is applicable when \ref SPEED_CONTROL_METHOD is set to
   \ref SPEED_CONTROL_CLOSED_LOOP.

   \todo Adjust the value as needed for the proportional gain in the PID
         controller when operating in closed-loop speed control mode based on
         your system's control requirements.

   \see SPEED_CONTROL_METHOD, SPEED_CONTROLLER_TIME_BASE,
        SPEED_CONTROLLER_MAX_DELTA, SPEED_CONTROLLER_MAX_SPEED, PID_K_I,
        PID_K_D_ENABLE, PID_K_D
*/
#define PID_K_P 100

/*!
   \brief PID Controller Integral Gain Constant (Only for Closed Loop)

   This macro specifies the integral gain constant for the PID controller. This
   parameter accepts signed integer values in the int16_t range.

   \note This parameter is applicable when \ref SPEED_CONTROL_METHOD is set to
   \ref SPEED_CONTROL_CLOSED_LOOP.

   \todo Adjust the value as needed for the integral gain in the PID controller
         when operating in closed-loop speed control mode based on your system's
         control requirements.

   \see SPEED_CONTROL_METHOD, SPEED_CONTROLLER_TIME_BASE,
        SPEED_CONTROLLER_MAX_DELTA, SPEED_CONTROLLER_MAX_SPEED, PID_K_P,
        PID_K_D_ENABLE, PID_K_D
*/
#define PID_K_I 10

/*!
   \brief PID Controller Derivative Control Enable Flag (Only for Closed Loop)

   Set this flag to \ref TRUE or \ref FALSE to enable or disable derivative
   control in the PID controller.

   \note This parameter is applicable when \ref SPEED_CONTROL_METHOD is set to
   \ref SPEED_CONTROL_CLOSED_LOOP.

   \todo Enable or disable derivative control by assigning \ref TRUE or \ref
         FALSE based on your system's control requirements when operating in
         closed-loop speed control mode.

   \see SPEED_CONTROL_METHOD, SPEED_CONTROLLER_TIME_BASE,
        SPEED_CONTROLLER_MAX_DELTA, SPEED_CONTROLLER_MAX_SPEED, PID_K_P,
        PID_K_I, PID_K_D
*/
#define PID_K_D_ENABLE TRUE

/*!
   \brief PID Controller Derivative Gain Constant (Only for Closed Loop)

   This macro specifies the derivative gain constant for the PID controller.
   This parameter accepts signed integer values in the int16_t range.

   \note This parameter is applicable when \ref SPEED_CONTROL_METHOD is set to
   \ref SPEED_CONTROL_CLOSED_LOOP.

   \todo Adjust the value as needed for the derivative gain in the PID
         controller when operating in closed-loop speed control mode based on
         your system's control requirements.

   \see SPEED_CONTROL_METHOD, SPEED_CONTROLLER_TIME_BASE,
        SPEED_CONTROLLER_MAX_DELTA, SPEED_CONTROLLER_MAX_SPEED, PID_K_P,
        PID_K_I, PID_K_D_ENABLE
*/
#define PID_K_D 0

/*!
   \brief Top resistor value in the VBUS voltage potential divider.

   This macro defines the value of the top resistor (RTOP) in the potential
   divider circuit used for measuring the VBUS voltage. It is specified in ohms
   (Ω).

   The value of RTOP is used to determine the scaling factor for the ADC
   conversion in the VBUS voltage measurement circuit.

   NEVB-MTR1-C-1 has a resistor divider with RTOP of 100 kΩ and RBOTTOM of 6.2
   kΩ.

   \note Ensure that the resistor value matches the actual hardware component
   used.

   \see VBUS_RBOTTOM, vbusVref
*/
#define VBUS_RTOP 100000

/*!
   \brief Bottom resistor value in the VBUS voltage potential divider.

   This macro defines the value of the bottom resistor (RBOTTOM) in the
   potential divider circuit used for measuring the VBUS voltage. It is
   specified in ohms (Ω).

   The value of RBOTTOM is used to determine the scaling factor for the ADC
   conversion in the VBUS voltage measurement circuit.

   NEVB-MTR1-C-1 has a resistor divider with RTOP of 100 kΩ and RBOTTOM of 6.2
   kΩ.

   \note Ensure that the resistor value matches the actual hardware component
   used.

   \see VBUS_RTOP, vbusVref
*/
#define VBUS_RBOTTOM 6200

/*!
   \brief Set the remote debug mode.

   When in remote debug mode, errors are immediately reported back on the serial
   port without having to query using the "SYSTem:ERRor[:NEXT]?" command.
*/
#define REMOTE_DEBUG_MODE FALSE

/** @} */

/*!
   \defgroup SystemFixed System Constant Defines

   \brief These defines should not be modified by the user.

   \ingroup ConfigDefines
   @{
*/

// Boolean values
//! FALSE constant value.
#define FALSE 0
//! TRUE constant value, defined to be compatible with comparisons.
#define TRUE (!FALSE)

//! High-speed system clock frequency.
#define F_HST 64000000UL

// PWM pin definitions
//! PIN where high side gate PWM for phase A is connected.
#define AH_PIN PB6
//! PIN where high side gate PWM for phase B is connected.
#define BH_PIN PC7
//! PIN where high side gate PWM for phase C is connected.
#define CH_PIN PD7
//! PIN where low side gate PWM for phase A is connected.
#define AL_PIN PB5
//! PIN where low side gate PWM for phase B is connected.
#define BL_PIN PC6
//! PIN where low side gate PWM for phase C is connected.
#define CL_PIN PD6

// PWM PORT patterns
//! Bit pattern of PWM pins placed on PORTB (Phase A).
#define PWM_PATTERN_PORTB ((1 << AH_PIN) | (1 << AL_PIN))
//! Bit pattern of PWM pins placed on PORTC (Phase B).
#define PWM_PATTERN_PORTC ((1 << BH_PIN) | (1 << BL_PIN))
//! Bit pattern of PWM pins placed on PORTD (Phase C).
#define PWM_PATTERN_PORTD ((1 << CH_PIN) | (1 << CL_PIN))
//! Bit pattern for Output Compare Enable Bits for PORTB (Phase A) placed on
//! TCCR1E.
#define OC_ENABLE_PORTB ((1 << OC4OE3) | (1 << OC4OE2))
//! Bit pattern for Output Compare Enable Bits for PORTC (Phase B) placed on
//! TCCR1E.
#define OC_ENABLE_PORTC ((1 << OC4OE1) | (1 << OC4OE0))
//! Bit pattern for Output Compare Enable Bits for PORTD (Phase C) placed on
//! TCCR1E.
#define OC_ENABLE_PORTD ((1 << OC4OE5) | (1 << OC4OE4))

// Direction macro definitions
//! Forward direction flag value.
#define DIRECTION_FORWARD 0
//! Reverse direction flag value.
#define DIRECTION_REVERSE 1
//! Unknown direction flag value.
#define DIRECTION_UNKNOWN 3

// Hall pins definitions
//! PIN register for Hall sensor input.
#define HALL_PIN PINB
//! Pin where H1 is connected.
#define H1_PIN PB1
//! Pin where H2 is connected.
#define H2_PIN PB2
//! Pin where H3 is connected.
#define H3_PIN PB3

// ADC ADMUX selections
//! Lower analog channel selection bits (MUX4:0) for analog speed reference
//! measurement.
#define ADC_MUX_L_SPEED ADC_MUX_L_ADC4
//! High analog channel selection bit (MUX5) for analog speed reference
//! measurement.
#define ADC_MUX_H_SPEED ADC_MUX_H_ADC4
//! Lower analog channel selection bits (MUX4:0) for motor current measurement.
#define ADC_MUX_L_IBUS ADC_MUX_L_ADC5
//! High analog channel selection bit (MUX5) for for motor current measurement.
#define ADC_MUX_H_IBUS ADC_MUX_H_ADC5
//! Lower analog channel selection bits (MUX4:0) for motor current measurement.
#define ADC_MUX_L_IPHASE_U ADC_MUX_L_ADC1
//! High analog channel selection bit (MUX5) for for motor current measurement.
#define ADC_MUX_H_IPHASE_U ADC_MUX_H_ADC1
//! Lower analog channel selection bits (MUX4:0) for motor current measurement.
#define ADC_MUX_L_IPHASE_V ADC_MUX_L_ADC7
//! High analog channel selection bit (MUX5) for for motor current measurement.
#define ADC_MUX_H_IPHASE_V ADC_MUX_H_ADC7
//! Lower analog channel selection bits (MUX4:0) for motor current measurement.
#define ADC_MUX_L_IPHASE_W ADC_MUX_L_ADC0
//! High analog channel selection bit (MUX5) for for motor current measurement.
#define ADC_MUX_H_IPHASE_W ADC_MUX_H_ADC0
//! Lower analog channel selection bits (MUX4:0) for motor vbusVref measurement.
#define ADC_MUX_L_VBUSVREF ADC_MUX_L_ADC6
//! High analog channel selection bit (MUX5) for for motor vbusVref measurement.
#define ADC_MUX_H_VBUSVREF ADC_MUX_H_ADC6

// ADC configurations
//! ADC clock pre-scaler used in this application.
#define ADC_PRESCALER ADC_PRESCALER_DIV_128
//! ADC voltage reference used in this application.
#define ADC_REFERENCE_VOLTAGE ADC_REFERENCE_VOLTAGE_VCC
//! ADC trigger used in this application.
#define ADC_TRIGGER ADC_TRIGGER_TIMER0_OVF

// Input pin definitions
//! Pin where direction command input is located.
#define DIRECTION_COMMAND_PIN PD2
//! Enable input pin.
#define ENABLE_PIN PD0
//! Remote input pin.
#define REMOTE_PIN PD3

// Speed Input Source definitions (only during remote mode)
//! Speed input source - Local or speed input pin.
#define SPEED_INPUT_SOURCE_LOCAL 0
//! Speed input source - Remote input.
#define SPEED_INPUT_SOURCE_REMOTE 1

// Fault pin definitions
//! Fault Pin 1.
#define FAULT_PIN_1 PD4
//! Fault Pin 2.
#define FAULT_PIN_2 PB4
//! Fault Pin 3.
#define FAULT_PIN_3 PB7

// Waveform macro definitions
//! Waveform constant for block commutation.
#define WAVEFORM_BLOCK_COMMUTATION 0
//! Waveform status flag used for coasting.
#define WAVEFORM_UNDEFINED 3

// Turn off mode macro definitions
//! TURN_OFF_MODE value for coasting (disabled drivers).
#define TURN_OFF_MODE_COAST 0
//! TURN_OFF_MODE value for ramping down (drivers ramp to zero speed reference).
#define TURN_OFF_MODE_RAMP 1

// Speed control macro definitions
//! Speed control selection for open loop control.
#define SPEED_CONTROL_OPEN_LOOP 0
//! Speed control selection for closed loop control.
#define SPEED_CONTROL_CLOSED_LOOP 1

/*!
   \brief Maximum Speed Reference Input

   This macro specifies the maximum speed reference input value that the speed
   controller should consider which corresponds to the highest possible reading
   from the ADC This is by default 8 bits as only the top 8 bits of the 10 bit
   ADC is being used. This corresponds to a value of 255.

   \note This parameter is applicable when \ref SPEED_CONTROL_METHOD is set to
         \ref SPEED_CONTROL_CLOSED_LOOP. Only change this if changes to the code
         are made to alter the ADC bits used.
*/
#define SPEED_CONTROLLER_MAX_INPUT 255

//! Macro to choose Timer4 pre-scaler.
#define CHOOSE_TIM4_PRESCALER(tim4Freq) ((tim4Freq) < 15625 ? 4 : ((tim4Freq) < 31250 ? 2 : 1))

/*!
   \brief Macro to choose Timer4 dead time pre-scaler based on the dead time.

   This macro determines the appropriate dead time pre-scaler for Timer4 based
   on the provided deadTime value. The deadTime is expected to be an integer
   value representing nanoseconds.

   The pre-scaler is chosen based on the following conditions:
     - For deadTime of 16 ns to 234 ns, pre-scaler 1 is chosen.
     - For deadTime of 235 ns to 468 ns, pre-scaler 2 is chosen.
     - For deadTime of 469 ns to 937 ns, pre-scaler 4 is chosen.
     - For deadTime of 938 ns to 1875 ns, pre-scaler 8 is chosen.

   \param deadTime The dead time in nanoseconds. \return The chosen pre-scaler
   value.
*/
#define CHOOSE_DT_PRESCALER(deadTime)             \
   ((deadTime) <= 234 ? 1 : (deadTime) <= 468 ? 2 \
                        : (deadTime) <= 937   ? 4 \
                        : (deadTime) <= 1875  ? 8 \
                                              : 0)

/**
   \def FORCE_INLINE

   \brief Macro for forcing inline expansion of functions.

   This macro ensures that a function is always inlined by the compiler,
   overriding the compiler's own decision process. When parsed by IntelliSense
   in VSCode, it is defined as a simple inline to avoid IntelliSense errors. For
   actual compilation, it includes the GCC-specific `always_inline` attribute to
   enforce inlining.
*/
#if defined(__INTELLISENSE__) || defined(DOXYGEN)
#define FORCE_INLINE inline
#else
#define FORCE_INLINE inline __attribute__((always_inline))
#endif

/**
   \def FAST_ACCESS(register_address)

   \brief Assign a specific memory address to a variable for fast access.

   This macro is used to specify the memory address of a register for fast
   access, directly instructing the compiler to place the variable at the given
   address. When parsed by IntelliSense in VSCode, the macro is defined as empty
   to avoid IntelliSense errors.
*/
#if defined(__INTELLISENSE__) || defined(DOXYGEN)
#define FAST_ACCESS(register_address)
#else
#define FAST_ACCESS(register_address) __attribute__((address(register_address)))
#endif

#ifdef __INTELLISENSE__
/**
   \def sei()

   \brief Macro to simulate the sei() function for IntelliSense for VSCode.

   This macro defines sei() as a no-operation (no-op) in the IntelliSense
   environment to prevent analysis errors and ensure compatibility with the
   IntelliSense parser.
*/
#define sei() ((void)0)

/**
   \def cli()

   \brief Macro to simulate the cli() function for IntelliSense for VSCode.

   This macro defines cli() as a no-operation (no-op) in the IntelliSense
   environment to prevent analysis errors and ensure compatibility with the
   IntelliSense parser.
*/
#define cli() ((void)0)
#endif

/**
   \defgroup PLLMacros PLL Post-scaler Macros

   \brief PLL post-scaler selection bits (PLLTM1:0) for High Speed Timer in
   PLLFRQ.
   @{
*/
//! PLL Post-scaler - off.
#define PLL_POSTSCALER_OFF ((0 << PLLTM1) | (0 << PLLTM0))
//! PLL Post-scaler - division factor 1.
#define PLL_POSTSCALER_DIV_1_0 ((0 << PLLTM1) | (1 << PLLTM0))
//! PLL Post-scaler - division factor 1.5.
#define PLL_POSTSCALER_DIV_1_5 ((1 << PLLTM1) | (0 << PLLTM0))
//! PLL Post-scaler - division factor 1.
#define PLL_POSTSCALER_DIV_2_0 ((1 << PLLTM1) | (1 << PLLTM0))
/** @} */

/**
   \defgroup DeadtimeMacros Deadtime Pre-scaler Macros

   \brief Deadtime generator pre-scaler selection bits (DTPS41:0) in TCCR4B.
   @{
*/
//! Deadtime generator pre-scaler - division factor 1.
#define DT_PRESCALER_DIV_1 ((0 << DTPS41) | (0 << DTPS40))
//! Deadtime generator pre-scaler - division factor 2.
#define DT_PRESCALER_DIV_2 ((0 << DTPS41) | (1 << DTPS40))
//! Deadtime generator pre-scaler - division factor 4.
#define DT_PRESCALER_DIV_4 ((1 << DTPS41) | (0 << DTPS40))
//! Deadtime generator pre-scaler - division factor 8.
#define DT_PRESCALER_DIV_8 ((1 << DTPS41) | (1 << DTPS40))
/** @} */

/**
   \defgroup TIM1ClockMacro Timer 1 (and 3) Clock Select Macros

   \brief Timer 1 (and 3) clock selection bits (CS12:0/CS12:0) for
   TCCR1B/TCCR3B.
   @{
*/
//! Timer1 clock - no clock source.
#define TIM1_CLOCK_OFF ((0 << CS12) | (0 << CS11) | (0 << CS10))
//! Timer1 clock - i/o clk with division factor 1.
#define TIM1_CLOCK_DIV_1 ((0 << CS12) | (0 << CS11) | (1 << CS10))
//! Timer1 clock - i/o clk with division factor 8.
#define TIM1_CLOCK_DIV_8 ((0 << CS12) | (1 << CS11) | (0 << CS10))
//! Timer1 clock - i/o clk with division factor 64.
#define TIM1_CLOCK_DIV_64 ((0 << CS12) | (1 << CS11) | (1 << CS10))
//! Timer1 clock - i/o clk with division factor 256.
#define TIM1_CLOCK_DIV_256 ((1 << CS12) | (0 << CS11) | (0 << CS10))
//! Timer1 clock - i/o clk with division factor 1024.
#define TIM1_CLOCK_DIV_1024 ((1 << CS12) | (0 << CS11) | (1 << CS10))
//! Timer1 clock - external clock source on Tn pin (falling edge triggered).
#define TIM1_CLOCK_EXT_FALLING ((1 << CS12) | (1 << CS11) | (0 << CS10))
//! Timer1 clock - external clock source on Tn pin (rising edge triggered).
#define TIM1_CLOCK_EXT_RISING ((1 << CS12) | (1 << CS11) | (1 << CS10))
/** @} */

/**
   \defgroup TIM4ClockMacro Timer 4 Clock Select Macros

   \brief Timer 4 clock selection bits (CS43:0) for TCCR4B.
   @{
*/
//! Timer4 pre-scaler - division factor OFF.
#define TIM4_PRESCALER_OFF ((0 << CS43) | (0 << CS42) | (0 << CS41) | (0 << CS40))
//! Timer4 pre-scaler - division factor 1.
#define TIM4_PRESCALER_DIV_1 ((0 << CS43) | (0 << CS42) | (0 << CS41) | (1 << CS40))
//! Timer4 pre-scaler - division factor 2.
#define TIM4_PRESCALER_DIV_2 ((0 << CS43) | (0 << CS42) | (1 << CS41) | (0 << CS40))
//! Timer4 pre-scaler - division factor 4.
#define TIM4_PRESCALER_DIV_4 ((0 << CS43) | (0 << CS42) | (1 << CS41) | (1 << CS40))
/** @} */

/**
   \defgroup ADCSelect ADC Multiplexer Select Macros

   \brief ADC multiplexer selection bits (MUX5 and MUX4:0) in ADCSRB and ADMUX
   respectively.
   @{
*/
//! Lower ADC channel selection bits (MUX4:0) mask.
#define ADC_MUX_L_BITS ((1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) mask.
#define ADC_MUX_H_BITS (1 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC0/PF0.
#define ADC_MUX_L_ADC0 ((0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC0/PF0.
#define ADC_MUX_H_ADC0 (0 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC1/PF1
#define ADC_MUX_L_ADC1 ((0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC1/PF1
#define ADC_MUX_H_ADC1 (0 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC4/PF4.
#define ADC_MUX_L_ADC4 ((0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (0 << MUX1) | (0 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC4/PF4.
#define ADC_MUX_H_ADC4 (0 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC5/PF5.
#define ADC_MUX_L_ADC5 ((0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (0 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC5/PF5.
#define ADC_MUX_H_ADC5 (0 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC6/PF6.
#define ADC_MUX_L_ADC6 ((0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC6/PF6.
#define ADC_MUX_H_ADC6 (0 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC7/PF7 .
#define ADC_MUX_L_ADC7 ((0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC7/PF7.
#define ADC_MUX_H_ADC7 (0 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC8/PD4.
#define ADC_MUX_L_ADC8 ((0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC8/PD4.
#define ADC_MUX_H_ADC8 (1 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC9/PD6.
#define ADC_MUX_L_ADC9 ((0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC9/PD6.
#define ADC_MUX_H_ADC9 (1 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC10/PD7.
#define ADC_MUX_L_ADC10 ((0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC10/PD7.
#define ADC_MUX_H_ADC10 (1 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC11/PB4.
#define ADC_MUX_L_ADC11 ((0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC11/PB4.
#define ADC_MUX_H_ADC11 (1 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC12/PB5.
#define ADC_MUX_L_ADC12 ((0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (0 << MUX1) | (0 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC12/PB5.
#define ADC_MUX_H_ADC12 (1 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - ADC13/PB6.
#define ADC_MUX_L_ADC13 ((0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (0 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) - ADC13/PB6.
#define ADC_MUX_H_ADC13 (1 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - Temperature Sensor.
#define ADC_MUX_L_TEMP_SENSOR ((0 << MUX4) | (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) - Temperature Sensor.
#define ADC_MUX_H_TEMP_SENSOR (1 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - 1.1V (Vbandgap).
#define ADC_MUX_L_1V1 ((1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0))
//! High ADC channel selection bit (MUX5) - 1.1V (Vbandgap).
#define ADC_MUX_H_1V1 (0 << MUX5)
//! Lower ADC channel selection bits (MUX4:0) - 0V (GND).
#define ADC_MUX_L_0V ((1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0))
//! High ADC channel selection bit (MUX5) - 0V (GND).
#define ADC_MUX_H_0V (0 << MUX5)
/** @} */

/**
   \defgroup ADCClock ADC Pre-scaler Macros

   \brief ADC pre-scaler selection bits (ADPS2:0) in ADCSRA.
   @{
*/
//! ADC pre-scaler - division factor 2.
#define ADC_PRESCALER_DIV_2 ((0 << ADPS2) | (0 << ADPS1) | (0 << ADPS0))
//! ADC pre-scaler - division factor 4.
#define ADC_PRESCALER_DIV_4 ((0 << ADPS2) | (1 << ADPS1) | (0 << ADPS0))
//! ADC pre-scaler - division factor 8.
#define ADC_PRESCALER_DIV_8 ((0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))
//! ADC pre-scaler - division factor 16.
#define ADC_PRESCALER_DIV_16 ((1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0))
//! ADC pre-scaler - division factor 32.
#define ADC_PRESCALER_DIV_32 ((1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0))
//! ADC pre-scaler - division factor 64.
#define ADC_PRESCALER_DIV_64 ((1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0))
//! ADC pre-scaler - division factor 128.
#define ADC_PRESCALER_DIV_128 ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))
/** @} */

/**
   \defgroup ADCReference ADC Reference Macros

   \brief ADC voltage reference selection bits (REFS1:0) in ADMUX.
   @{
*/
//! ADC voltage reference - AREF voltage.
#define ADC_REFERENCE_VOLTAGE_AREF ((0 << REFS1) | (0 << REFS0))
//! ADC voltage reference - VCC voltage.
#define ADC_REFERENCE_VOLTAGE_VCC ((0 << REFS1) | (1 << REFS0))
//! ADC voltage reference - internal voltage (2.56V).
#define ADC_REFERENCE_VOLTAGE_INTERNAL ((1 << REFS1) | (1 << REFS0))
/** @} */

/**
   \defgroup ADCTrigger ADC Auto Trigger Macros

   \brief ADC auto trigger source selection bits (ADTS3:0) in ADCSRB.
   @{
*/
//! ADC auto trigger source - ADC Free Running
#define ADC_TRIGGER_FREE ((0 << ADTS3) | (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0))
//! ADC auto trigger source - Analog Comparator
#define ADC_TRIGGER_ANALOG_COMP ((0 << ADTS3) | (0 << ADTS2) | (0 << ADTS1) | (1 << ADTS0))
//! ADC auto trigger source - External Interrupt Request 0
#define ADC_TRIGGER_INT0 ((0 << ADTS3) | (0 << ADTS2) | (1 << ADTS1) | (0 << ADTS0))
//! ADC auto trigger source - Timer/Counter0 Compare Match A
#define ADC_TRIGGER_TIMER0_COMPA ((0 << ADTS3) | (0 << ADTS2) | (1 << ADTS1) | (1 << ADTS0))
//! ADC auto trigger source - Timer/Counter0 Overflow
#define ADC_TRIGGER_TIMER0_OVF ((0 << ADTS3) | (1 << ADTS2) | (0 << ADTS1) | (0 << ADTS0))
//! ADC auto trigger source - Timer/Counter1 Compare Match B
#define ADC_TRIGGER_TIMER1_COMPB ((0 << ADTS3) | (1 << ADTS2) | (0 << ADTS1) | (1 << ADTS0))
//! ADC auto trigger source - Timer/Counter1 Overflow
#define ADC_TRIGGER_TIMER1_OVF ((0 << ADTS3) | (1 << ADTS2) | (1 << ADTS1) | (0 << ADTS0))
//! ADC auto trigger source - Timer/Counter1 Capture Event
#define ADC_TRIGGER_TIMER1_CAPT ((0 << ADTS3) | (1 << ADTS2) | (1 << ADTS1) | (1 << ADTS0))
//! ADC auto trigger source - Timer/Counter4 Overflow
#define ADC_TRIGGER_TIMER4_OVF ((1 << ADTS3) | (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0))
//! ADC auto trigger source - Timer/Counter4 Compare Match A
#define ADC_TRIGGER_TIMER4_COMPA ((1 << ADTS3) | (0 << ADTS2) | (0 << ADTS1) | (1 << ADTS0))
//! ADC auto trigger source - Timer/Counter4 Compare Match B
#define ADC_TRIGGER_TIMER4_COMPB ((1 << ADTS3) | (0 << ADTS2) | (1 << ADTS1) | (0 << ADTS0))
//! ADC auto trigger source - Timer/Counter4 Compare Match D
#define ADC_TRIGGER_TIMER4_COMPD ((1 << ADTS3) | (0 << ADTS2) | (1 << ADTS1) | (1 << ADTS0))
/** @} */

// Typedefs
/*! \brief Collection of all motor control flags.

    This struct contains all motor control flags used in this implementation.
*/
typedef struct motorflags
{
   //! Reserved bit(s).
   uint8_t reserved : 8;
   //! Should speed controller run?
   uint8_t speedControllerRun : 1;
   //! Is the remote enabled?
   uint8_t remote : 1;
   //! Is the motor enabled?
   uint8_t enable : 1;
   //! The actual direction of rotation.
   uint8_t actualDirection : 2;
   //! The desired direction of rotation.
   uint8_t desiredDirection : 1;
   //! The current waveform that should be produced.
   uint8_t driveWaveform : 2;
} motorflags_t;

/*! \brief Collection of all fault flags.

    This struct contains all fault flags used in this implementation.
*/
typedef struct faultflags
{
   //! Reserved bit(s).
   uint8_t reserved : 1;
   //! Is motor spinning in an unexpected direction?
   uint8_t reverseDirection : 1;
   //! Is motor stopped?
   uint8_t motorStopped : 1;
   //! Has it tripped the over current limit?
   uint8_t overCurrent : 1;
   //! Is there no hall connections?
   uint8_t noHallConnections : 1;
   //! Is user flag 1 set?
   uint8_t userFlag1 : 1;
   //! Is user flag 2 set?
   uint8_t userFlag2 : 1;
   //! Is user flag 3 set?
   uint8_t userFlag3 : 1;
} faultflags_t;

/*! \brief Collection of motor configurations.

    This struct contains some of the motor configurations. This may support
    remote implementation.
*/
typedef struct motorconfigs
{
   //! TIM4 or gate switching frequency.
   uint32_t tim4Freq : 17; // max value 131071
   //! Corresponding TIM4 top value for TIM4 frequency.
   uint16_t tim4Top : 10; // max value 1023
   //! Corresponding dead time for TIM4 output.
   uint16_t tim4DeadTime : 11; // max value 2047
   //! SpeedInput source select (only for remote mode).
   uint8_t speedInputSource : 1;
} motorconfigs_t;

/** @} */

/**
   \defgroup DerivedDefines Derived Defines

   \brief These defines are derived from the user settable defines.

   \ingroup ConfigDefines
   @{
*/

/*!
   \brief Timer 4 clock select bits based on pre-scaler value

   This macro generates the timer 4 clock select bits pattern based on the
   chosen pre-scaler value. It is used to configure the timer's clock division.
*/
#define TIM4_PRESCALER_DIV_PATTERN(tim4Prescaler)                                             \
   ((tim4Prescaler) == 1 ? TIM4_PRESCALER_DIV_1 : (tim4Prescaler) == 2 ? TIM4_PRESCALER_DIV_2 \
                                              : (tim4Prescaler) == 4   ? TIM4_PRESCALER_DIV_4 \
                                                                       : 0)

//! Calculate top value for Timer 4. Formula: TIM4_TOP = (\ref F_HST / (\ref
//! TIM4_FREQ * TIM4_PRESCALER * 2)).
#define TIM4_TOP(tim4Freq) (((F_HST / ((uint32_t)tim4Freq * CHOOSE_TIM4_PRESCALER(tim4Freq))) >> 1) - 1)

//! Maximum top value for Timer 4.
#define TIM4_TOP_MAX 0x03ff

/*!
   \brief Deadtime generator pre-scaler selection bits based on pre-scaler value

   This macro generates the deadtime generator pre-scaler selection bits pattern
   based on the chosen pre-scaler value.
*/
#define DT_PRESCALER_DIV_PATTERN(dtPrescaler)                                         \
   ((dtPrescaler) == 1 ? DT_PRESCALER_DIV_1 : (dtPrescaler) == 2 ? DT_PRESCALER_DIV_2 \
                                          : (dtPrescaler) == 4   ? DT_PRESCALER_DIV_4 \
                                          : (dtPrescaler) == 8   ? DT_PRESCALER_DIV_8 \
                                                                 : 0)

/*! \brief This value specifies half the dead time in number of clock cycles.
Divide by frequency to get duration.

*/
#define DEAD_TIME_HALF(deadTime) (((uint8_t)(ceil((double)deadTime * F_HST / ((double)CHOOSE_DT_PRESCALER(deadTime) * 1000000000)))))

/*! \brief Calculated top value for Timer 3.

    Formula: TIM3_TOP = (F_CPU / (\ref TIM3_FREQ * TIM3_PRESCALER * 6)) - 1,
    where TIM3_PRESCALER = 8 and 6 is the number of commutations in a complete
    cycle.
*/
#define TIM3_TOP (((F_CPU / TIM3_FREQ / 3) >> 4) - 1)

//! Maximum top value for Timer 3.
#define TIM3_TOP_MAX 0xffff
#if TIM3_TOP_ > TIM3_TOP_MAX
#error "Invalid TIM3_FREQ set"
#endif

#if ((EMULATE_HALL == TRUE) && (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP))
#error "Invalid combination of EMULATE_HALL and SPEED_CONTROL_METHOD"
#endif

/** @} */

/*!
  \mainpage

  This project is focused on the implementation of motor control, specifically
  designed for Brushless DC (BLDC) motors using Hall Effect sensors on the
  [Leonardo](https://docs.arduino.cc/hardware/leonardo) board or the
  [ATMEGA32u4](https://ww1.microchip.com/downloads/en/devicedoc/atmel-7766-8-bit-avr-atmega16u4-32u4_datasheet.pdf)
  micro controller, platforms widely recognized under the Arduino Leonardo
  nomenclature when operated through the Arduino IDE.

  This documentation documents the data structures, functions, variables,
  defines, enums, and typedefs in the code designed for the motor driver evaluation kit
  NEVB-MTR1-KIT1 which provides a full motor drive solution for
  3-phase BLDC motors. Support for brushless DC motors shall be implemented
  later. Specifically, the code is to work with NEVB-MTR1-C-1, accompanied
  with the 3-phase inverter module NEVB-MTR1-I56-1,
  which contains PCBs designed for the [Nexperia LFPAK 5x6 MOSFET
  family](https://www.nexperia.com/products/mosfets/family/LFPAK56-MOSFETS/).

  \image html evaluation_board.png "Figure: Motor driver evaluation kit NEVB-MTR1-KIT1."

  To fully understand the system's capabilities and limitations, and for
  detailed instructions on setting up the kit, please refer to the accompanying
  user manual.

  Please also refer to the User Manual and other materials, which can be found
  [here](https://l.ead.me/beShjo).

  \section contactinfo Contact Info

  For more information, visit [Nexperia.com](http://www.nexperia.com).

  Support page: https://www.nexperia.com/support
*/

/*! \page features Features

   The firmware provides an Arduino compatible, or specifically the ATMEGA32u4
   AVR micro controller, trapezoidal control of brushless DC (BLDC) motors using
   Hall Effect sensors. Most important it's all open-source.

   Specific features are discussed below:

   \section motor_configuration Motor Configuration
   - Configurable motor poles (\ref MOTOR_POLES).
   - Configurable switching frequency for MOSFET gate signals (\ref TIM4_FREQ).
   - Adjustable dead time between switching actions (\ref DEAD_TIME).
   - Option to enable or disable internal pull-up resistors on hall sensor
     inputs (\ref HALL_PULLUP_ENABLE).
   - Motor emulation capability by generating hall effect sensor inputs (\ref
     EMULATE_HALL).
   - Configurable electrical rotational frequency for emulated motor (\ref
     TIM3_FREQ).
   - Threshold for determining when the motor is considered stopped (\ref
     COMMUTATION_TICKS_STOPPED).
   - Selectable turn-off mode (coast or brake) (\ref TURN_OFF_MODE).

   \section drive_controls Drive Controls
   - Slow ramp up or down when turning on the motor or changing the speed
     reference input. (Not when turning it off as this depends on the turn-off
     mode.)
   - Set the direction of rotation of the motor using input buttons.
   - Enable/disable the motor using input buttons.
   - Easily set a duty cycle or speed based on choice of speed control.

   \section current_monitor Current Monitor
   - Adjustable current gain factor (\ref IBUS_GAIN).
   - Configuration of the resistance value of the current sense resistor (\ref
     IBUS_SENSE_RESISTOR).
   - Threshold values for current warning and error conditions (\ref
     IBUS_WARNING_THRESHOLD, \ref IBUS_ERROR_THRESHOLD).
   - Option to enable or disable action when the current error threshold is
     exceeded (\ref IBUS_FAULT_ENABLE).

   \section speed_control Speed Control
   - Selection between open-loop and closed-loop speed control (\ref
     SPEED_CONTROL_METHOD).
   - Adjustable parameters for PID controller in closed-loop control (\ref
     PID_K_P, \ref PID_K_I, \ref PID_K_D_ENABLE, \ref PID_K_D).
   - Maximum speed for closed-loop control (\ref SPEED_CONTROLLER_MAX_SPEED).
   - Parameters for both speed control (\ref SPEED_CONTROLLER_TIME_BASE, \ref
     SPEED_CONTROLLER_MAX_DELTA).

   \section scpi_implementation SCPI Implementation
   - Implementation of SCPI protocol for remote control and communication.
   - Definable SCPI input buffer length, error queue size, and command strings.
   - Implementation of SCPI commands for motor control, error handling, and
     system information.
   - SCPI commands for measuring motor speed, current, direction, and gate
     voltage.
   - Commands for querying motor enable state, direction, and configuration.

   \section error_handling_and_reporting Error Handling and Reporting
   - Flags to store motor state and error reporting (\ref motorFlags, \ref
     faultFlags).
   - Fault LEDs to indicate faults.
   - Detect if inverter board is attached or not.
   - SCPI command for retrieving errors from the error queue (
     SYSTem:ERRor[:NEXT]?).

   \section documentation_and_support Documentation and Support
   - Complete documentation of project.
   - Links to external SCPI parser library and support page.
   - User manual for board level documentation.
*/

/*! \page quickstart Quick Start

  The project code is set up to be used with the Arduino IDE. To learn more
  about the the Leonardo hardware and software tools, go to Arduino's [getting
  started
  page](https://docs.arduino.cc/learn/starting-guide/getting-started-arduino).

  \section quickstartinstall Uploading the Code

  1. Install Arduino IDE [v1](https://docs.arduino.cc/software/ide-v1) or
     [v2](https://docs.arduino.cc/software/ide-v2). Alternatively, you can also
     use [Microsoft Visual Studio Code](https://code.visualstudio.com/) with the
     [Arduino
     extension](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino)
     installed.

  2. Download and unzip a copy of the latest version of the code provided for
     this project.

  3. In this example, Arduino IDE v1 is used. Open main.ino with Arduino IDE.

  4. If this is the first time you are running the code with the motor provided
     in the kit, then you do not have to modify any parameters unless other
     parameters are preferred. To make changes to the the configuration for your
     particular motor, head to the \ref todo section to see the list of changes
     recommended.

  5. Select the appropriate board, in this case Leonardo, from Tools > Board,
     the correct port from Tools > Port, and ensure the programmer is set to
     AVRISP mkII under Tools > Programmer.

  6. Click on the "Upload" button to compile and upload the code to the Leonardo
     development board.

  \section quickstartrun Operating the Board

  \note Before powering on the system, it's essential to refer to the user
  manual for specific setup instructions related to the Nexperia's BLDC
  evaluation kit. The manual provides critical information on how to properly
  configure and prepare the system for operation. Also, refer to the \ref safety
  page.

  After ensuring that the setup is in accordance with the manual, power on the
  system. The motor control program will start, enabling the control and
  operation of the BLDC motor using the Hall Effect sensors.

  \section quickstartnext Next Steps

  \subsection quickstartchanges1 Making configuration changes

  Making simple configuration changes can be done by changing the \ref
  UserSettable in the motor header file, \ref config.h. These are also summarized
  in the \ref todo section.

  \subsection quickstartchanges2 Making code changes

  Making any further changes to the code will require understanding of the whole
  code base and it is recommended to read through the documentation completely.

  \subsection quickstartscpi Using SCPI

  The code supports basic implementation of SCPI allowing remote control by
  serial communication. To understand how to use this feature, please refer to
  the SCPI pages.
*/

/*! \page safety Safety Precautions

  - **Motor Handling**: Ensure that the motor is securely clamped down and
    cannot move unexpectedly during operation. This is crucial to prevent any
    accidents or damage to the system.

  - **Operating Ranges**: Verify that the motor's intended operating ranges
    (voltage, current, speed, etc.) are supported by your setup. Operating the
    motor outside its specified limits can lead to malfunction or damage.

  - **Power Isolation**: Implement appropriate measures to quickly and safely
    isolate the power to the system if needed. This might include emergency stop
    mechanisms or easily accessible power switches.

  - **General Safety**: Always take reasonable safety precautions when handling
    electronic components. This includes wearing protective gear as necessary,
    ensuring a tidy and organized workspace, and being cautious of potential
    electrical hazards.

  \image html warning1.png width=70%

  \image html warning2.png width=70%

  \image html warning3.png width=70%
*/

/*! \page todo To Do List

    This page lists all the macros that can be set by the user to quickly modify
    operating parameters such as switching frequencies and speed control
    methods. If this is the first time you are running the code with the motor
    provided in the kit, then you do not have to modify the parameters unless
    other parameters are preferred.

    Each entry is linked to the part of the code where it was mentioned.
*/

/*! \page license License

   \include LICENSE

   \note The code is dependent on [SCPI Parser Arduino
   Library](https://github.com/sfeister/scpi-parser-arduino) which is a port by
   Scott Feister to the Arduino platform of the [SCPI Parser Library
   v2](https://github.com/sfeister/scpi-parser-arduino) by Jan Breuer. This
   dependency is provided in the source code underneath the `main/src/`
   directory accompanied with its own license (BSD 2-Clause License). Please
   read through the `LICENSE` file in the dependency directory for more
   information. The library has also been modified to specifically support the
   needs of this project.
*/

#endif /* _MAIN_H_ */

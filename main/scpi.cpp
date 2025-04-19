/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI implementation source file.

   \details
        This file contains all function callbacks and type definitions necessary
        for SCPI implementation.

        This file depends on the base [SCPI parser library v2](https://github.com/j123b567/scpi-parser)
        (commit #[4e87990](https://github.com/j123b567/scpi-parser/tree/4e879901b51cbb43dab36dd83f95a23f1dbaa4c0))
        by Jan Breuer which was then ported by Scott Feister to the Arduino IDE,
        [SCPI Parser Arduino Library](https://github.com/sfeister/scpi-parser-arduino).

        Further modifications were made to the base library to allow for memory
        optimisation and support for the avr-gcc compiler.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2024/03/08 $  \n

 ******************************************************************************/

#include "scpi.h"

/**
   \brief Array or enumeration of possible motor directions.

   This defines the possible directions in which the motor can move, i.e.
   forward, reverse. It is used in SCPI commands to set or query the motor's
   current direction. Each direction is typically associated with a specific
   command or numeric value.
*/
const scpi_choice_def_t motorDirections[] = {
    {"FORWard", DIRECTION_FORWARD},
    {"REVErse", DIRECTION_REVERSE},
    SCPI_CHOICE_LIST_END /* termination of option list */
};

/**
   \brief Measures the motor speed.

   This function calculates and returns the motor speed in revolutions per
   minute (RPM).

   \param context The SCPI context \return SCPI result code indicating
   successful operation
*/
static scpi_result_t MeasureMotorSpeed(scpi_t *context)
{
  if (lastCommutationTicks == 0xffff)
  {
    SCPI_ResultDouble(context, 0);
  }
  else
  {
    SCPI_ResultDouble(context, ((motorConfigs.tim4Freq * 20) / (lastCommutationTicks * MOTOR_POLES)));
  }

  return SCPI_RES_OK;
}

/**
   \brief Measures and returns the motor's current.

   This function calculates the current being used by the motor and returns it
   in Amperes.

   \param context The SCPI context \return SCPI result code indicating success
   or failure
*/
static scpi_result_t MeasureMotorCurrent(scpi_t *context)
{
  SCPI_ResultDouble(context, ((double)current * 5 * 1000000) / ((double)1024 * CURRENT_GAIN * CURRENT_SENSE_RESISTOR));

  return SCPI_RES_OK;
}

/**
   \brief Measures and reports the current direction of the motor.

   This function checks the current direction of the motor and returns a textual
   representation ('FORWard', 'REVErse', or 'UNKNown') based on the motor's
   actual direction.

   \param context The SCPI context \return SCPI result code indicating success
   or failure
*/
static scpi_result_t MeasureMotorDirection(scpi_t *context)
{
  if (motorFlags.actualDirection == DIRECTION_UNKNOWN)
  {
    SCPI_ResultText(context, "UNKNown");
  }
  else
  {
    const char *name;

    SCPI_ChoiceToName(motorDirections, motorFlags.actualDirection, &name);

    SCPI_ResultText(context, name);
  }

  return SCPI_RES_OK;
}

/**
   \brief Measures and returns the gate voltage of the motor.

   This function calculates the gate voltage of the motor using the gate voltage
   reference value and returns it in Volts.

   \param context The SCPI context \return SCPI result code indicating success
   or failure
*/
static scpi_result_t MeasureGateVoltage(scpi_t *context)
{
  SCPI_ResultDouble(context, ((double)vbusVref * 5 * (VBUS_RTOP + VBUS_RBOTTOM)) / ((double)1024 * VBUS_RBOTTOM));

  return SCPI_RES_OK;
}

/**
   \brief Configures the motor's speed input source.

   This function reads a boolean parameter from the SCPI command and sets the
   motor's source corresponding to \ref SPEED_INPUT_SOURCE_LOCAL and \ref
   SPEED_INPUT_SOURCE_REMOTE.

   \param context The SCPI context \return SCPI result code indicating success
   (SCPI_RES_OK) or error (SCPI_RES_ERR)
*/
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
static scpi_result_t ConfigureMotorDutyCycleSource(scpi_t *context)
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
static scpi_result_t ConfigureMotorSpeedSource(scpi_t *context)
#endif
{
  scpi_bool_t param;

  /* read first parameter if present */
  if (!SCPI_ParamBool(context, &param, TRUE))
  {
    return SCPI_RES_ERR;
  }
  else
  {
    if (param == SPEED_INPUT_SOURCE_LOCAL)
    {
      motorConfigs.speedInputSource = SPEED_INPUT_SOURCE_LOCAL;
    }
    else
    {
      motorConfigs.speedInputSource = SPEED_INPUT_SOURCE_REMOTE;
      speedInput = 0;
    }
  }

  return SCPI_RES_OK;
}

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
/**
   \brief Configures the motor's speed input by changing the duty cycle.

   \param context The SCPI context \return SCPI result code indicating success
   (SCPI_RES_OK) or error (SCPI_RES_ERR)
*/
static scpi_result_t ConfigureMotorDutyCycle(scpi_t *context)
{
  double param;

  // Read first parameter if present
  if (!SCPI_ParamDouble(context, &param, TRUE))
  {
    return SCPI_RES_ERR;
  }
  else
  {
    // Check if within range
    if ((param < 0.0) | (param > 100.0))
    {
      return SCPI_RES_ERR;
    }

    speedInput = (param * SPEED_CONTROLLER_MAX_INPUT) / 100;
  }

  return SCPI_RES_OK;
}
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
/**
   \brief Configures the motor's speed input by changing the speed reference.

   \param context The SCPI context \return SCPI result code indicating success
   (SCPI_RES_OK) or error (SCPI_RES_ERR)
*/
static scpi_result_t ConfigureMotorSpeed(scpi_t *context)
{
  uint32_t param;

  // Read first parameter if present
  if (!SCPI_ParamUInt32(context, &param, TRUE))
  {
    return SCPI_RES_ERR;
  }
  else
  {
    // Check if within range
    if (param > ((((uint32_t)SPEED_CONTROLLER_MAX_SPEED * 15) << 3) / MOTOR_POLES))
    {
      return SCPI_RES_ERR;
    }

    speedInput = ((param * SPEED_CONTROLLER_MAX_INPUT * MOTOR_POLES) >> 3) / ((uint32_t)SPEED_CONTROLLER_MAX_SPEED * 15);
  }

  return SCPI_RES_OK;
}
#endif

/**
   \brief Measures and returns the gate PWM duty cycle.

   \param context The SCPI context \return SCPI result code indicating success
   or failure
*/
static scpi_result_t MeasureGateDutyCycle(scpi_t *context)
{
  if (motorFlags.enable == TRUE)
  {
    // Reading 16 bit register so disabling interrupts for atomic operation
    cli();
    uint16_t duty = 0xff & OCR4A;
    duty |= (0x03 & TC4H) << 8;
    sei();

    SCPI_ResultDouble(context, ((double)duty / (double)motorConfigs.tim4Top * 100));
  }
  else
  {
    SCPI_ResultDouble(context, 0);
  }

  return SCPI_RES_OK;
}

/**
   \brief Configures the motor's enable state.

   This function reads a boolean parameter from the SCPI command and sets the
   motor's enable state accordingly.

   In remote mode, the \ref ENABLE_PIN is configured as an output as opposed to
   an input. The motor is then enabled or disabled by setting or clearing the
   \ref ENABLE_PIN on the PORTD register, which triggers the relevant software
   interrupt as it would in normal operation.

   \param context The SCPI context \return SCPI result code indicating success
   (SCPI_RES_OK) or error (SCPI_RES_ERR)
*/
static scpi_result_t ConfigureMotorEnable(scpi_t *context)
{
  scpi_bool_t param;

  /* read first parameter if present */
  if (!SCPI_ParamBool(context, &param, TRUE))
  {
    return SCPI_RES_ERR;
  }
  else
  {
    if (param)
    {
      // Set the enable pin
      PORTD |= (1 << ENABLE_PIN);
    }
    else
    {
      // Clear the enable pin
      PORTD &= ~(1 << ENABLE_PIN);
    }
  }

  return SCPI_RES_OK;
}

/**
   \brief Retrieves the current motor enable state.

   This function queries the current enable state of the motor and returns it as
   a boolean value.

   \param context The SCPI context \return SCPI result code indicating success
   or failure
*/
static scpi_result_t GetConfigureMotorEnable(scpi_t *context)
{
  SCPI_ResultBool(context, (scpi_bool_t)motorFlags.enable);

  return SCPI_RES_OK;
}

/**
   \brief Sets the motor's direction based on the input parameter.

   This function reads a direction parameter ('FORWard' or 'REVErse') and sets
   the motor's direction accordingly by manipulating the \ref
   DIRECTION_COMMAND_PIN on PORTD.

   In remote mode, the \ref DIRECTION_COMMAND_PIN is configured as an output as
   opposed to an input. Setting or clearing the pin triggers the relevant
   software interrupt as it would in normal operation.

   \param context The SCPI context \return SCPI result code indicating success
   (SCPI_RES_OK) or error (SCPI_RES_ERR)
*/
static scpi_result_t ConfigureMotorDirection(scpi_t *context)
{
  int32_t param;

  /* read first parameter if present */
  if (!SCPI_ParamChoice(context, motorDirections, &param, TRUE))
  {
    return SCPI_RES_ERR;
  }
  else
  {
    if (param)
    {
      // Set the direction pin if param is 1
      PORTD |= (1 << DIRECTION_COMMAND_PIN);
    }
    else
    {
      // Clear the direction pin if param is 0
      PORTD &= ~(1 << DIRECTION_COMMAND_PIN);
    }
  }

  return SCPI_RES_OK;
}

/**
   \brief Retrieves the configured direction of the motor.

   This function queries the desired direction of the motor and returns a
   textual representation ('FORWard' or 'REVErse') based on the motor's desired
   direction setting.

   \param context The SCPI context \return SCPI result code indicating success
   or failure
*/
static scpi_result_t GetConfigureMotorDirection(scpi_t *context)
{
  const char *name;

  SCPI_ChoiceToName(motorDirections, motorFlags.desiredDirection, &name);

  SCPI_ResultText(context, name);

  return SCPI_RES_OK;
}

/**
   \brief Configures the motor's operating frequency.

   This function sets the motor's operating frequency based on the input
   parameter. It validates the frequency range, updates the motor configuration,
   and reinitializes timers after ensuring the motor is stopped.

   \param context The SCPI context \return SCPI result code indicating success
   (SCPI_RES_OK) or error (SCPI_RES_ERR)
*/
static scpi_result_t ConfigureMotorFrequency(scpi_t *context)
{
  // Clear the enable pin
  PORTD &= ~(1 << ENABLE_PIN);

  uint32_t param;

  // Read first parameter if present
  if (!SCPI_ParamUInt32(context, &param, TRUE))
  {
    return SCPI_RES_ERR;
  }
  else
  {
    // Check if within range
    if ((param < 7183) | (param > 100000))
    {
      return SCPI_RES_ERR;
    }

    // Reload the configs
    motorConfigs.tim4Freq = param;
    motorConfigs.tim4Top = (uint16_t)TIM4_TOP(motorConfigs.tim4Freq);

    // Wait until motor is stopped
    while (faultFlags.motorStopped == FALSE)
    {
      ;
    }

    // Re-init timers
    TimersInit();
  }

  return SCPI_RES_OK;
}

/**
   \brief Retrieves the configured motor frequency.

   This function queries the current frequency configuration of the motor and
   returns it as a double value.

   \param context The SCPI context \return SCPI result code indicating success
   or failure
*/
static scpi_result_t GetConfigureMotorFrequency(scpi_t *context)
{
  SCPI_ResultDouble(context, (double)motorConfigs.tim4Freq);

  return SCPI_RES_OK;
}

/**
   \brief Configures the motor's dead time.

   This function sets the motor's dead time based on the input parameter. It
   validates the dead time range, updates the motor configuration, and
   reinitializes timers after ensuring the motor is stopped.

   \param context The SCPI context \return SCPI result code indicating success
   (SCPI_RES_OK) or error (SCPI_RES_ERR)
*/
static scpi_result_t ConfigureMotorDeadTime(scpi_t *context)
{
  // Clear the enable pin
  PORTD &= ~(1 << ENABLE_PIN);

  uint32_t param;

  // Read first parameter if present
  if (!SCPI_ParamUInt32(context, &param, TRUE))
  {
    return SCPI_RES_ERR;
  }
  else
  {
    // Check if within range
    if ((param < 350) | (param > 1875))
    {
      return SCPI_RES_ERR;
    }

    // Reload the config
    motorConfigs.tim4DeadTime = param;

    // Wait until motor is stopped
    while (faultFlags.motorStopped == FALSE)
    {
      ;
    }

    // Re-init timers
    TimersInit();
  }

  return SCPI_RES_OK;
}

/**
   \brief Retrieves the configured motor dead time.

   This function queries the current dead time configuration of the motor and
   returns it as a double value.

   \param context The SCPI context \return SCPI result code indicating success
   or failure
*/
static scpi_result_t GetConfigureMotorDeadTime(scpi_t *context)
{
  SCPI_ResultDouble(context, (double)motorConfigs.tim4DeadTime);

  return SCPI_RES_OK;
}

/**
   \brief Array of SCPI commands.

   This array contains the definitions of all SCPI commands that the system
   recognizes. Each command is defined with its pattern, callback function, and
   associated context. The array is stored in program memory (PROGMEM) to
   optimize memory usage.
*/
const scpi_command_t scpi_commands[] PROGMEM = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    {"*CLS", SCPI_CoreCls, 0},
    {"*IDN?", SCPI_CoreIdnQ, 0},
    {"*RST", SCPI_CoreRst, 0},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {"SYSTem:ERRor[:NEXT]?", SCPI_SystemErrorNextQ, 0},
    {"SYSTem:ERRor:COUNt?", SCPI_SystemErrorCountQ, 0},
    {"SYSTem:VERSion?", SCPI_SystemVersionQ, 0},

    /* Motor */
    {"CONFigure:MOTOr:ENABle", ConfigureMotorEnable, 0},
    {"CONFigure:MOTOr:ENABle?", GetConfigureMotorEnable, 0},
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
    {"CONFigure:MOTOr:GATE:DUTYcycle:SOURce", ConfigureMotorDutyCycleSource, 0},
    {"CONFigure:MOTOr:GATE:DUTYcycle", ConfigureMotorDutyCycle, 0},
#else
    {"CONFigure:MOTOr:SPEED:SOURce", ConfigureMotorSpeedSource, 0},
    {"CONFigure:MOTOr:SPEED", ConfigureMotorSpeed, 0},
#endif
    {"CONFigure:MOTOr:GATE:FREQuency", ConfigureMotorFrequency, 0},
    {"CONFigure:MOTOr:GATE:FREQuency?", GetConfigureMotorFrequency, 0},
    {"CONFigure:MOTOr:GATE:DEADtime", ConfigureMotorDeadTime, 0},
    {"CONFigure:MOTOr:GATE:DEADtime?", GetConfigureMotorDeadTime, 0},
    {"CONFigure:MOTOr:DIREction", ConfigureMotorDirection, 0},
    {"CONFigure:MOTOr:DIREction?", GetConfigureMotorDirection, 0},
    {"MEASure:MOTOr:SPEEd?", MeasureMotorSpeed, 0},
    {"MEASure:MOTOr:CURRent?", MeasureMotorCurrent, 0},
    {"MEASure:MOTOr:DIREction?", MeasureMotorDirection, 0},
    {"MEASure:MOTOr:GATE:VOLTage?", MeasureGateVoltage, 0},
    {"MEASure:MOTOr:GATE:DUTYcycle?", MeasureGateDutyCycle, 0},

    SCPI_CMD_LIST_END};

/**
   \brief Writes data to the SCPI interface.

   This function is responsible for writing data to the Serial interface. It is
   used as a callback in the SCPI interface structure.

   \param context The SCPI context \param data Pointer to the data to be written
   \param len Length of the data to be written \return The number of bytes
   written
*/
size_t SCPI_Write(scpi_t *context, const char *data, size_t len)
{
  (void)context;
  Serial.write(data, len);
  return len;
}

/**
   \brief Flushes the Serial interface buffer.

   This function ensures that all pending Serial data is transmitted. It acts as
   a callback for the SCPI interface structure.

   \param context The SCPI context \return SCPI result code indicating
   successful operation
*/
scpi_result_t SCPI_Flush(scpi_t *context)
{
  (void)context;
  Serial.flush();
  return SCPI_RES_OK;
}

/**
   \brief Handles SCPI errors and outputs them to the Serial interface.

   This function processes and displays SCPI error messages. It is used as a
   callback in the SCPI interface structure for error handling.

   \param context The SCPI context \param err The error code \return Always
   returns 0
*/
int SCPI_Error(scpi_t *context, int_fast16_t err)
{
  (void)context;
#if (REMOTE_DEBUG_MODE == TRUE)
  Serial.print(err);
  Serial.print(", \"");
  Serial.print(SCPI_ErrorTranslate(err));
  Serial.println("\"");
#endif
  return 0;
}

/**
   \brief Handles control messages for the SCPI interface (dummy)

   This function processes control messages such as SRQ (Service Request) or
   other control signals. It outputs relevant information to the Serial
   interface. Used as a callback in the SCPI interface structure.

   \note Dummy function implemented here.

   \param context The SCPI context \param ctrl The control name/type \param val
   The control value \return SCPI result code indicating successful operation
*/
scpi_result_t SCPI_Control(scpi_t *context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val)
{
  (void)context;

  return SCPI_RES_OK;
}

/**
   \brief Resets the SCPI context.

   This function is called to reset the SCPI environment. It outputs a reset
   message to the Serial interface and is used as a callback in the SCPI
   interface structure.

   \param context The SCPI context \return SCPI result code indicating
   successful operation
*/
scpi_result_t SCPI_Reset(scpi_t *context)
{
  (void)context;

  // Clear the enable pin
  PORTD &= ~(1 << ENABLE_PIN);

  // Wait until motor is stopped
  while (faultFlags.motorStopped == FALSE)
  {
    ;
  }

  // Reset configurations
  ConfigsInit();

  // Re-init timers
  TimersInit();

  return SCPI_RES_OK;
}

/**
   \brief SCPI interface structure.

   This structure defines the callbacks for various SCPI interface operations
   like error handling, writing to an output, control functions, flushing the
   output, and resetting the interface.
*/
scpi_interface_t scpi_interface = {
    /*.error = */ SCPI_Error,     /**< Callback for error handling. */
    /*.write = */ SCPI_Write,     /**< Callback for writing to an output. */
    /*.control = */ SCPI_Control, /**< Callback for control functions. */
    /*.flush = */ SCPI_Flush,     /**< Callback for flushing the output. */
    /*.reset = */ SCPI_Reset,     /**< Callback for resetting the interface. */
};

/**
   \brief SCPI input buffer.

   Array for storing incoming SCPI commands. Its size is defined by \ref
   SCPI_INPUT_BUFFER_LENGTH.
*/
char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

/**
   \brief SCPI error queue data array.

   Array for storing SCPI errors. Its size is defined by \ref
   SCPI_ERROR_QUEUE_SIZE.
*/
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

/**
   \brief SCPI context structure.

   This structure holds the context for SCPI operations, encapsulating state and
   configuration information.
*/
scpi_t scpi_context;

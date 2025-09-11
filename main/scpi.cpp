/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI implementation source file.

   \details
        This file contains the command handlers for the SCPI parser to recognize and
        execute.

        Most of this code was originally written by Diego González Chávez as part of the
        [Vrekrer SCPI Parser](https://github.com/Vrekrer/Vrekrer_scpi_parser) project.

        It has been merged into the main codebase for customization and linking limitations
        of the Arduino IDE.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2025/04/21 $  \n

 ******************************************************************************/

#include "scpi.h"
#include "config.h"
#include "scpi_config.h"
#include "scpi_helper.h"

// Forward declarations
static void ScpiCoreIdnQ(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ScpiSystemErrorCountQ(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ScpiSystemErrorNextQ(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void GetMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ConfigureMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void GetConfigureMotorDutyCycleSource(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ConfigureMotorDutyCycle(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ConfigureMotorFrequency(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void GetConfigureMotorFrequency(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ConfigureMotorDirection(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void GetConfigureMotorDirection(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void MeasureMotorSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void MeasureMotorCurrentVBus(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void MeasureMotorCurrentPhaseU(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void MeasureMotorCurrentPhaseV(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void MeasureMotorCurrentPhaseW(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void MeasureMotorDirection(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void MeasureMotorVoltage(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void MeasureGateDutyCycle(SCPI_C commands, SCPI_P parameters, Stream &interface);
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
static void ConfigureMotorDutyCycleSource(SCPI_C commands, SCPI_P parameters, Stream &interface);
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
static void ConfigureMotorSpeedSource(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void GetMotorSpeedSource(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ConfigureMotorSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface);
#endif

// Instantiate the SCPI Parser
SCPI_Parser scpiParser;

/**
 * \brief Initializes the SCPI command parser and registers all supported commands.
 *
 * This function sets up the SCPI parser instance and registers the core IEEE
 * mandated commands, required SCPI commands, and the custom motor control
 * and measurement commands.
 */
void ScpiInit(void)
{
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    // "*CLS" and "*RST" are not supported
    scpiParser.RegisterCommand(F("*IDN?"), &ScpiCoreIdnQ);

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    // "SYSTem:VERSion?" and "SYSTem:ERRor:NEXT?" are not supported
    scpiParser.SetCommandTreeBase(F("SYSTem"));
    scpiParser.RegisterCommand(F(":ERRor?"), &ScpiSystemErrorNextQ);
    scpiParser.RegisterCommand(F(":ERRor:COUNt?"), &ScpiSystemErrorCountQ);

    /* Motor Configuration Commands */
    scpiParser.SetCommandTreeBase(F("CONFigure"));
    scpiParser.RegisterCommand(F(":ENABle"), &ConfigureMotorEnable);
    scpiParser.RegisterCommand(F(":ENABle?"), &GetMotorEnable);
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
    scpiParser.RegisterCommand(F(":DUTYcycle:SOURce"), &ConfigureMotorDutyCycleSource);
    scpiParser.RegisterCommand(F(":DUTYcycle:SOURce?"), &GetConfigureMotorDutyCycleSource);
    scpiParser.RegisterCommand(F(":DUTYcycle"), &ConfigureMotorDutyCycle);
#else
    scpiParser.RegisterCommand(F(":SPEED:SOURce"), &ConfigureMotorSpeedSource);
    scpiParser.RegisterCommand(F(":SPEED:SOURce?"), &GetMotorSpeedSource);
    scpiParser.RegisterCommand(F(":SPEED"), &ConfigureMotorSpeed);
#endif
    scpiParser.RegisterCommand(F(":FREQuency"), &ConfigureMotorFrequency);
    scpiParser.RegisterCommand(F(":FREQuency?"), &GetConfigureMotorFrequency);
    scpiParser.RegisterCommand(F(":DIREction"), &ConfigureMotorDirection);
    scpiParser.RegisterCommand(F(":DIREction?"), &GetConfigureMotorDirection);

    /* Motor Measurement Commands */
    scpiParser.SetCommandTreeBase(F("MEASure"));
    scpiParser.RegisterCommand(F(":SPEEd?"), &MeasureMotorSpeed);
    scpiParser.RegisterCommand(F(":CURRent:IBUS?"), &MeasureMotorCurrentVBus);
    scpiParser.RegisterCommand(F(":CURRent:IPHU?"), &MeasureMotorCurrentPhaseU);
    scpiParser.RegisterCommand(F(":CURRent:IPHV?"), &MeasureMotorCurrentPhaseV);
    scpiParser.RegisterCommand(F(":CURRent:IPHW?"), &MeasureMotorCurrentPhaseW);
    scpiParser.RegisterCommand(F(":VOLTage?"), &MeasureMotorVoltage);
    scpiParser.RegisterCommand(F(":DIREction?"), &MeasureMotorDirection);
    scpiParser.RegisterCommand(F(":DUTYcycle?"), &MeasureGateDutyCycle);
}

/**
 * \brief Processes incoming data from a serial interface for SCPI commands.
 *
 * This function takes a `Stream` object (like `Serial`) and processes any
 * received data, looking for complete SCPI commands terminated by a newline
 * character. It then passes the command to the SCPI parser for execution.
 *
 * \param interface The serial stream interface to read commands from.
 */
void ScpiInput(Stream &interface)
{
    scpiParser.ProcessInput(interface, SCPI_CMD_TERM);
}

/**
 * \brief Implements the `*IDN?` (Identification Query) command.
 *
 * This function responds to the `*IDN?` command by sending the manufacturer,
 * model, serial number (if defined), and firmware revision of the device.
 * The identification string is formatted as: `<Manufacturer>,<Model>,<SerialNumber>,<FirmwareRevision>`.
 *
 * \param commands The parsed SCPI commands (not used).
 * \param parameters The parsed SCPI parameters (not used).
 * \param interface The serial stream interface to write the response to.
 */
static void ScpiCoreIdnQ(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.print(F(SCPI_IDN_MANUFACTURER));
    interface.print(F(","));
    interface.print(F(SCPI_IDN_MODEL));
    interface.print(F(","));
    if (SCPI_IDN_DEFAULT_SERIAL != NULL && strlen(SCPI_IDN_DEFAULT_SERIAL) > 0)
    {
        interface.print(F(SCPI_IDN_DEFAULT_SERIAL));
    }
    interface.print(F(","));
    interface.println(F(SCPI_IDN_FIRMWARE_VERSION));
}

/**
 * \brief Implements the `SYSTem:ERRor:COUNt?` (System Error Count Query) command.
 *
 * This function returns the number of errors currently in the error queue.
 * In this simplified implementation, it returns 1 if there was a last error,
 * and 0 otherwise.
 *
 * \param commands The parsed SCPI commands (not used).
 * \param parameters The parsed SCPI parameters (not used).
 * \param interface The serial stream interface to write the response to.
 */
static void ScpiSystemErrorCountQ(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(scpiParser.last_error == ErrorCode::NoError ? 0 : 1);
}

/**
 * \brief Implements the `SYSTem:ERRor?` (System Error Next Query) command.
 *
 * This function returns the oldest error from the error queue and clears it.
 * In this implementation, it returns a textual description of the last recorded
 * error and then clears the `last_error` flag.
 *
 * \param commands The parsed SCPI commands (not used).
 * \param parameters The parsed SCPI parameters (not used).
 * \param interface The serial stream interface to write the response to.
 */
static void ScpiSystemErrorNextQ(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    switch (scpiParser.last_error)
    {
    case ErrorCode::BufferOverflow:
        interface.println(F("Buffer overflow error"));
        break;
    case ErrorCode::Timeout:
        interface.println(F("Communication timeout error"));
        break;
    case ErrorCode::UnknownCommand:
        interface.println(F("Unknown command received"));
        break;
    case ErrorCode::NoError:
        interface.println(F("No Error"));
        break;
    case ErrorCode::MissingOrInvalidParameter:
        interface.println(F("Missing or invalid parameter"));
        break;
    default:
        interface.println(F("Unknown error"));
    }
    scpiParser.last_error = ErrorCode::NoError;
}

/**
 * \brief Retrieves the current motor enable state.
 *
 * This function queries the current enable state of the motor and returns it as
 * a boolean value (1 for enabled, 0 for disabled).
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void GetMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(motorFlags.enable);
}

/**
 * \brief Configures the motor's enable state.
 *
 * This function reads a boolean parameter (0 or 1) from the SCPI command and sets the
 * motor's enable state accordingly.
 *
 * In remote mode, the `ENABLE_PIN` is configured as an output. The motor is then
 * enabled or disabled by setting or clearing the `ENABLE_PIN` on the `PORTD`
 * register, which triggers the relevant software interrupt as it would in normal
 * operation.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters containing the boolean enable value.
 * \param interface The serial interface (not used).
 */
static void ConfigureMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    bool param;
    if (!ScpiParamBool(parameters, param))
    {
        scpiParser.last_error = ErrorCode::MissingOrInvalidParameter;
        return;
    }
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

    scpiParser.last_error = ErrorCode::NoError;
}

/**
 * \brief Configures the motor's speed input source.
 *
 * This function reads a choice parameter ('LOCAL' or 'REMOTE') from the SCPI
 * command and sets the motor's speed input source accordingly, corresponding
 * to `SPEED_INPUT_SOURCE_LOCAL` and `SPEED_INPUT_SOURCE_REMOTE`.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters containing the input source choice.
 * \param interface The serial interface (not used).
 */
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
static void ConfigureMotorDutyCycleSource(SCPI_C commands, SCPI_P parameters, Stream &interface)
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
static void ConfigureMotorSpeedSource(SCPI_C commands, SCPI_P parameters, Stream &interface)
#endif
{
    uint8_t param;

    // read first parameter if present
    if (!ScpiParamChoice(parameters, inputSources, INPUT_SOURCE_OPTIONS, param))
    {
        scpiParser.last_error = ErrorCode::MissingOrInvalidParameter;
        return;
    }

    if (param == SPEED_INPUT_SOURCE_LOCAL)
    {
        motorConfigs.speedInputSource = SPEED_INPUT_SOURCE_LOCAL;
        return;
    }
    else
    {
        motorConfigs.speedInputSource = SPEED_INPUT_SOURCE_REMOTE;
        speedInput = 0;
        return;
    }
    scpiParser.last_error = ErrorCode::NoError;
}

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
/**
 * \brief Retrieves the configured motor's duty cycle source.
 *
 * This function queries the currently configured source for the motor's duty cycle
 * (either 'LOCAL' or 'REMOTE') and returns it as a string.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void GetConfigureMotorDutyCycleSource(SCPI_C commands, SCPI_P parameters, Stream &interface)
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
/**
 * \brief Retrieves the configured motor's speed source.
 *
 * This function queries the currently configured source for the motor's speed
 * (either 'LOCAL' or 'REMOTE') and returns it as a string.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void GetMotorSpeedSource(SCPI_C commands, SCPI_P parameters, Stream &interface)
#endif
{
    String name;
    ScpiChoiceToName(inputSources, INPUT_SOURCE_OPTIONS, motorConfigs.speedInputSource, name);
    interface.println(name);
}

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
/**
 * \brief Configures the motor's speed input by setting the duty cycle.
 *
 * This function reads a double parameter (0.0 to 100.0) from the SCPI command
 * and sets the motor's duty cycle accordingly.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters containing the duty cycle value.
 * \param interface The serial interface (not used).
 */
static void ConfigureMotorDutyCycle(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    double param;
    if (!ScpiParamDouble(parameters, param) || param < 0.0 || param > 100.0)
    {
        scpiParser.last_error = ErrorCode::MissingOrInvalidParameter;
        return;
    }
    speedInput = param;
    scpiParser.last_error = ErrorCode::NoError;
}
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
/**
 * \brief Configures the motor's speed input by setting the speed reference.
 *
 * This function reads a double parameter representing the target speed and
 * sets the internal speed reference accordingly. The input speed is validated
 * against the maximum allowed speed.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters containing the target speed value.
 * \param interface The serial interface (not used).
 */
static void ConfigureMotorSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    double param;
    if (!ScpiParamDouble(parameters, param) || param > ((((uint32_t)SPEED_CONTROLLER_MAX_SPEED * 15) << 3) / MOTOR_POLES))
    {
        scpiParser.last_error = ErrorCode::MissingOrInvalidParameter;
        return;
    }
    speedInput = ((param * SPEED_CONTROLLER_MAX_INPUT * MOTOR_POLES) >> 3) / ((uint32_t)SPEED_CONTROLLER_MAX_SPEED * 15);
    scpiParser.last_error = ErrorCode::NoError;
}
#endif

/**
 * \brief Configures the motor's operating frequency.
 *
 * This function sets the motor's operating frequency based on the input
 * parameter. It validates the frequency range (7183 to 100000 Hz), updates
 * the motor configuration, and reinitializes timers after ensuring the motor
 * is stopped.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters containing the frequency value in Hz.
 * \param interface The serial interface (not used).
 */
static void ConfigureMotorFrequency(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    // Clear the enable pin to stop the motor before changing frequency
    PORTD &= ~(1 << ENABLE_PIN);

    uint32_t param;

    // Read first parameter if present and within range
    if (!ScpiParamUInt32(parameters, param) || param < F_MOSFET_MIN || param > F_MOSFET_MAX)
    {
        scpiParser.last_error = ErrorCode::MissingOrInvalidParameter;
        return;
    }

    // Reload the configs
    motorConfigs.tim4Freq = param;
    motorConfigs.tim4Top = (uint16_t)TIM4_TOP(motorConfigs.tim4Freq);

    // Wait until motor is stopped
    while (faultFlags.motorStopped == FALSE)
    {
        ;
    }

    // Re-init timers with the new frequency
    TimersInit();
    scpiParser.last_error = ErrorCode::NoError;
}

/**
 * \brief Retrieves the configured motor frequency.
 *
 * This function queries the current frequency configuration of the motor and
 * returns it as a double value representing the frequency in Hertz.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void GetConfigureMotorFrequency(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(motorConfigs.tim4Freq);
}

/**
 * \brief Sets the motor's direction based on the input parameter.
 *
 * This function reads a direction parameter ('FORW' or 'REVE') and sets
 * the motor's direction accordingly by manipulating the `DIRECTION_COMMAND_PIN`
 * on `PORTD`.
 *
 * In remote mode, the `DIRECTION_COMMAND_PIN` is configured as an output.
 * Setting or clearing the pin triggers the relevant software interrupt as it
 * would in normal operation.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters containing the direction choice.
 * \param interface The serial interface (not used).
 */
static void ConfigureMotorDirection(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    uint8_t param;

    // read first parameter if present
    if (!ScpiParamChoice(parameters, motorDirections, MOTOR_DIRECTION_OPTIONS, param))
    {
        scpiParser.last_error = ErrorCode::MissingOrInvalidParameter;
        return;
    }

    if (param)
    {
        // Set the direction pin if param is 1 (DIRECTION_REVERSE)
        PORTD |= (1 << DIRECTION_COMMAND_PIN);
    }
    else
    {
        // Clear the direction pin if param is 0 (DIRECTION_FORWARD)
        PORTD &= ~(1 << DIRECTION_COMMAND_PIN);
    }
    scpiParser.last_error = ErrorCode::NoError;
}

/**
 * \brief Retrieves the configured direction of the motor.
 *
 * This function queries the desired direction of the motor and returns a
 * textual representation ('FORW' or 'REVE') based on the `motorFlags.desiredDirection`
 * setting.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void GetConfigureMotorDirection(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String name;
    ScpiChoiceToName(motorDirections, MOTOR_DIRECTION_OPTIONS, motorFlags.desiredDirection, name);
    interface.println(name);
}

/**
 * \brief Measures the motor speed.
 *
 * This function calculates and returns the motor speed in revolutions per
 * minute (RPM). It uses the time difference between the last two commutation
 * events and the configured motor frequency and pole count for the calculation.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void MeasureMotorSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (lastCommutationTicks == 0xffff)
    {
        interface.println(0.0);
    }
    else
    {
        interface.println(
            (motorConfigs.tim4Freq * 20.0) / (lastCommutationTicks * MOTOR_POLES));
    }
}

/**
 * \brief Measures and returns the motor's VBUS current.
 *
 * This function reads the ADC value for the VBUS current sense, converts it
 * to Amperes using the known gain and sense resistor value.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void MeasureMotorCurrentVBus(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(((double)ibus * 5.0 * 1000000.0) / ((double)1023.0 * IBUS_GAIN * IBUS_SENSE_RESISTOR));
}

/**
 * \brief Measures and returns the motor's phase U current.
 *
 * This function reads the ADC value for the phase U current sense, converts it
 * to Amperes using the known gain and sense resistor value. It accounts for
 * the ADC offset (typically around 511 for a centered reading).
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void MeasureMotorCurrentPhaseU(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(((double)(iphaseU - 511) * 5.0 * 1000000.0) / ((double)1023.0 * IPHASE_GAIN * IPHASE_SENSE_RESISTOR));
}

/**
 * \brief Measures and returns the motor's phase V current.
 *
 * This function reads the ADC value for the phase V current sense, converts it
 * to Amperes using the known gain and sense resistor value. It accounts for
 * the ADC offset (typically around 511 for a centered reading).
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void MeasureMotorCurrentPhaseV(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(((double)(iphaseV - 511) * 5.0 * 1000000.0) / ((double)1023.0 * IPHASE_GAIN * IPHASE_SENSE_RESISTOR));
}

/**
 * \brief Measures and returns the motor's phase W current.
 *
 * This function reads the ADC value for the phase W current sense, converts it
 * to Amperes using the known gain and sense resistor value. It accounts for
 * the ADC offset (typically around 511 for a centered reading).
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void MeasureMotorCurrentPhaseW(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(((double)(iphaseW - 511) * 5.0 * 1000000.0) / ((double)1023.0 * IPHASE_GAIN * IPHASE_SENSE_RESISTOR));
}

/**
 * \brief Measures and reports the current direction of the motor.
 *
 * This function checks the `motorFlags.actualDirection` and returns a textual
 * representation ('FORW', 'REVE', or 'UNKN') based on the motor's sensed
 * direction.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void MeasureMotorDirection(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (motorFlags.actualDirection == DIRECTION_UNKNOWN)
    {
        interface.println(F("UNKN"));
    }
    else
    {
        String name;
        ScpiChoiceToName(motorDirections, MOTOR_DIRECTION_OPTIONS, motorFlags.actualDirection, name);
        interface.println(name);
    }
}

/**
 * \brief Measures and returns the VBUS voltage of the motor.
 *
 * This function reads the ADC value for the VBUS voltage reference and converts
 * it to Volts using the voltage divider resistor values.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void MeasureMotorVoltage(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(((double)vbusVref * 5.0 * (VBUS_RTOP + VBUS_RBOTTOM)) / ((double)1023.0 * VBUS_RBOTTOM));
}

/**
 * \brief Measures and returns the gate PWM duty cycle as a percentage.
 *
 * This function reads the current PWM duty cycle from Timer4's compare register
 * (OCR4A) and calculates the percentage relative to the timer's top value.
 * Interrupts are briefly disabled to ensure atomic reading of the 16-bit duty
 * cycle value.
 *
 * \param commands The SCPI commands (not used).
 * \param parameters The SCPI parameters (not used).
 * \param interface The serial interface to write the response to.
 */
static void MeasureGateDutyCycle(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    if (motorFlags.enable == TRUE)
    {
        // Reading 16 bit register so disabling interrupts for atomic operation
        cli();
        uint16_t duty = 0xff & OCR4A;
        duty |= (0x03 & TC4H) << 8;
        sei();

        interface.println((double)duty / (double)motorConfigs.tim4Top * 100.0);
    }
    else
    {
        interface.println(0.0);
    }
}

/**
 * \brief Array defining the possible motor directions for SCPI commands.
 *
 * This array is used by the SCPI parser to interpret and represent the motor's
 * direction ('FORW' for forward, 'REVE' for reverse). Each entry in the array
 * associates a textual representation with a numerical value defined elsewhere
 * (e.g., `DIRECTION_FORWARD`, `DIRECTION_REVERSE`).
 */
const SCPI_choice_def_t motorDirections[MOTOR_DIRECTION_OPTIONS] = {
    {"FORW", "ard", DIRECTION_FORWARD},
    {"REVE", "rse", DIRECTION_REVERSE},
};

/**
 * \brief Array defining the possible input sources for speed/duty cycle control.
 *
 * This array is used by the SCPI parser to interpret and represent the source
 * of the motor's speed or duty cycle control ('LOCA' for local, 'REMO' for remote).
 * Each entry associates a textual representation with a numerical value
 * (e.g., `SPEED_INPUT_SOURCE_LOCAL`, `SPEED_INPUT_SOURCE_REMOTE`).
 */
const SCPI_choice_def_t inputSources[INPUT_SOURCE_OPTIONS] = {
    {"LOCA", "l", SPEED_INPUT_SOURCE_LOCAL},
    {"REMO", "te", SPEED_INPUT_SOURCE_REMOTE},
};

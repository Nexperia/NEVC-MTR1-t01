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

// Instantiate the SCPI Parser
SCPI_Parser scpiParser;

void ScpiInit(void)
{
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    // scpiParser.RegisterCommand(F("*CLS"), &SCPI_CoreCls);
    scpiParser.RegisterCommand(F("*IDN?"), &ScpiCoreIdnQ);
    // scpiParser.RegisterCommand(F("*RST"), &SCPI_CoreRst);
    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    scpiParser.RegisterCommand(F("SYSTem:ERRor?"), &ScpiSystemErrorNextQ);
    scpiParser.RegisterCommand(F("SYSTem:ERRor:COUNt?"), &ScpiSystemErrorCountQ);
    // scpiParser.RegisterCommand(F("SYSTem:VERSion?"), &SCPI_SystemVersionQ);
    //   /* Motor */
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:ENABle"), &ConfigureMotorEnable);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:ENABle?"), &GetMotorEnable);
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DUTYcycle:SOURce"), &ConfigureMotorDutyCycleSource);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DUTYcycle:SOURce?"), &GetConfigureMotorDutyCycleSource);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DUTYcycle"), &ConfigureMotorDutyCycle);
#else
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:SPEED:SOURce"), &ConfigureMotorSpeedSource);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:SPEED:SOURce?"), &GetMotorSpeedSource);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:SPEED"), &ConfigureMotorSpeed);
#endif
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:FREQuency"), &ConfigureMotorFrequency);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:FREQuency?"), &GetConfigureMotorFrequency);
    // scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DEADtime"), &ConfigureMotorDeadTime);
    // scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DEADtime?"), &GetConfigureMotorDeadTime);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:DIREction"), &ConfigureMotorDirection);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:DIREction?"), &GetConfigureMotorDirection);
    scpiParser.RegisterCommand(F("MEASure:MOTOr:SPEEd?"), &MeasureMotorSpeed);
    scpiParser.RegisterCommand(F("MEASure:MOTOr:VOLTage?"), &MeasureMotorVoltage);
    scpiParser.RegisterCommand(F("MEASure:MOTOr:CURRent?"), &MeasureMotorCurrent);
    scpiParser.RegisterCommand(F("MEASure:MOTOr:DIREction?"), &MeasureMotorDirection);
    scpiParser.RegisterCommand(F("MEASure:MOTOr:GATE:DUTYcycle?"), &MeasureGateDutyCycle);
}

void ScpiInput(Stream &interface)
{
    scpiParser.ProcessInput(interface, "\n");
}

static void ScpiCoreIdnQ(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.print(F(SCPI_IDN1));
    interface.print(F(","));
    interface.print(F(SCPI_IDN2));
    interface.print(F(","));
    if (SCPI_IDN3 != NULL && strlen(SCPI_IDN3) > 0)
    {
        interface.print(F(SCPI_IDN3));
        interface.print(F(","));
    }
    interface.println(F(SCPI_IDN4));
}

static void ScpiSystemErrorCountQ(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(scpiParser.last_error == ErrorCode::NoError ? 0 : 1);
}

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
    \brief Retrieves the current motor enable state.

    This function queries the current enable state of the motor and returns it as
    a boolean value.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void GetMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(motorFlags.enable);
}

/**
    \brief Configures the motor's enable state.

    This function reads a boolean parameter from the SCPI command and sets the
    motor's enable state accordingly.

    In remote mode, the \ref ENABLE_PIN is configured as an output as opposed to
    an input. The motor is then enabled or disabled by setting or clearing the
    \ref ENABLE_PIN on the PORTD register, which triggers the relevant software
    interrupt as it would in normal operation.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
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

    scpiParser.last_error = ErrorCode::MissingOrInvalidParameter;
}

/**
    \brief Configures the motor's speed input source.

    This function reads a boolean parameter from the SCPI command and sets the
    motor's source corresponding to \ref SPEED_INPUT_SOURCE_LOCAL and \ref
    SPEED_INPUT_SOURCE_REMOTE.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
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
}

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
/**
    \brief Configures the motor's speed input by changing the duty cycle.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void GetConfigureMotorDutyCycleSource(SCPI_C commands, SCPI_P parameters, Stream &interface)
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
static void GetMotorSpeedSource(SCPI_C commands, SCPI_P parameters, Stream &interface)
#endif
{
    String name;
    ScpiChoiceToName(inputSources, INPUT_SOURCE_OPTIONS, motorConfigs.speedInputSource, name);
    interface.println(name);
}

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
static void ConfigureMotorDutyCycle(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    double param;
    if (!ScpiParamDouble(parameters, param) || param < 0.0 || param > 100.0)
    {
        scpiParser.last_error = ErrorCode::MissingOrInvalidParameter;
        return;
    }
    speedInput = param;
}
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
/**
    \brief Configures the motor's speed input by changing the speed reference.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
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
}
#endif

/**
    \brief Configures the motor's operating frequency.

    This function sets the motor's operating frequency based on the input
    parameter. It validates the frequency range, updates the motor configuration,
    and reinitializes timers after ensuring the motor is stopped.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void ConfigureMotorFrequency(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    // Clear the enable pin
    PORTD &= ~(1 << ENABLE_PIN);

    uint32_t param;

    // Read first parameter if present and within range
    if (!ScpiParamUInt32(parameters, param) || param < 7183 || param > 100000)
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

    // Re-init timers
    TimersInit();
}

/**
    \brief Retrieves the configured motor frequency.

    This function queries the current frequency configuration of the motor and
    returns it as a double value.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void GetConfigureMotorFrequency(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(motorConfigs.tim4Freq);
}

/**
    \brief Sets the motor's direction based on the input parameter.

    This function reads a direction parameter ('FORWard' or 'REVErse') and sets
    the motor's direction accordingly by manipulating the \ref
    DIRECTION_COMMAND_PIN on PORTD.

    In remote mode, the \ref DIRECTION_COMMAND_PIN is configured as an output as
    opposed to an input. Setting or clearing the pin triggers the relevant
    software interrupt as it would in normal operation.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
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
        // Set the direction pin if param is 1
        PORTD |= (1 << DIRECTION_COMMAND_PIN);
    }
    else
    {
        // Clear the direction pin if param is 0
        PORTD &= ~(1 << DIRECTION_COMMAND_PIN);
    }
}

/**
    \brief Retrieves the configured direction of the motor.

    This function queries the desired direction of the motor and returns a
    textual representation ('FORWard' or 'REVErse') based on the motor's desired
    direction setting.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void GetConfigureMotorDirection(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String name;
    ScpiChoiceToName(motorDirections, MOTOR_DIRECTION_OPTIONS, motorFlags.desiredDirection, name);
    interface.println(name);
}

/**
    \brief Measures the motor speed.

    This function calculates and returns the motor speed in revolutions per
    minute (RPM).

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
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
            (motorConfigs.tim4Freq * 20) / (lastCommutationTicks * MOTOR_POLES));
    }
}

/**
    \brief Measures and returns the motor's current.

    This function calculates the current being used by the motor and returns it
    in Amperes.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void MeasureMotorCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(((double)current * 5 * 1000000) / ((double)1024 * CURRENT_GAIN * CURRENT_SENSE_RESISTOR));
}

/**
    \brief Measures and reports the current direction of the motor.

    This function checks the current direction of the motor and returns a textual
    representation ('FORWard', 'REVErse', or 'UNKNown') based on the motor's
    actual direction.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void MeasureMotorDirection(SCPI_C context, SCPI_P parameters, Stream &interface)
{
    if (motorFlags.actualDirection == DIRECTION_UNKNOWN)
    {
        interface.println("UNKNown");
    }
    else
    {
        String name;
        ScpiChoiceToName(motorDirections, MOTOR_DIRECTION_OPTIONS, motorFlags.actualDirection, name);
        interface.println(name);
    }
}

/**
    \brief Measures and returns the VBUS voltage of the motor.

    This function calculates the VBUS voltage of the motor using the VBUS voltage
    reference value and returns it in Volts.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void MeasureMotorVoltage(SCPI_C context, SCPI_P parameters, Stream &interface)
{
    interface.println(((double)vbusVref * 5 * (VBUS_RTOP + VBUS_RBOTTOM)) / ((double)1024 * VBUS_RBOTTOM));
}

/**
    \brief Measures and returns the gate PWM duty cycle.

    \param commands The SCPI commands.
    \param parameters The SCPI parameters.
    \param interface The serial interface.
*/
static void MeasureGateDutyCycle(SCPI_C context, SCPI_P parameters, Stream &interface)
{
    if (motorFlags.enable == TRUE)
    {
        // Reading 16 bit register so disabling interrupts for atomic operation
        cli();
        uint16_t duty = 0xff & OCR4A;
        duty |= (0x03 & TC4H) << 8;
        sei();

        interface.println((double)duty / (double)motorConfigs.tim4Top * 100);
    }
    else
    {
        interface.println(0.0);
    }
}

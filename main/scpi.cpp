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
    scpiParser.RegisterCommand(F("SYSTem:ERRor[:NEXT]?"), &ScpiSystemErrorNextQ);
    scpiParser.RegisterCommand(F("SYSTem:ERRor:COUNt?"), &ScpiSystemErrorCountQ);
    // scpiParser.RegisterCommand(F("SYSTem:VERSion?"), &SCPI_SystemVersionQ);
    //   /* Motor */
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:ENABle"), &ConfigureMotorEnable);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:ENABle?"), &GetConfigureMotorEnable);
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DUTYcycle:SOURce"), &ConfigureMotorDutyCycleSource);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DUTYcycle"), &ConfigureMotorDutyCycle);
#else
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:SPEED:SOURce"), &ConfigureMotorSpeedSource);
    scpiParser.RegisterCommand(F("CONFigure:MOTOr:SPEED"), &ConfigureMotorSpeed);
#endif
    //   scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:FREQuency"), &ConfigureMotorFrequency);
    //   scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:FREQuency?"), &GetConfigureMotorFrequency);
    //   scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DEADtime"), &ConfigureMotorDeadTime);
    //   scpiParser.RegisterCommand(F("CONFigure:MOTOr:GATE:DEADtime?"), &GetConfigureMotorDeadTime);
    //   scpiParser.RegisterCommand(F("CONFigure:MOTOr:DIREction"), &ConfigureMotorDirection);
    //   scpiParser.RegisterCommand(F("CONFigure:MOTOr:DIREction?"), &GetConfigureMotorDirection);
    //   scpiParser.RegisterCommand(F("MEASure:MOTOr:SPEEd?"), &MeasureMotorSpeed);
    //   scpiParser.RegisterCommand(F("MEASure:MOTOr:CURRent?"), &MeasureMotorCurrent);
    //   scpiParser.RegisterCommand(F("MEASure:MOTOr:DIREction?"), &MeasureMotorDirection);
    //   scpiParser.RegisterCommand(F("MEASure:MOTOr:GATE:VOLTage?"), &MeasureGateVoltage);
    //   scpiParser.RegisterCommand(F("MEASure:MOTOr:GATE:DUTYcycle?"), &MeasureGateDutyCycle);
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
    interface.println(scpiParser.last_error == SCPI_Parser::ErrorCode::NoError ? 0 : 1);
}

static void ScpiSystemErrorNextQ(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    switch (scpiParser.last_error)
    {
    case SCPI_Parser::ErrorCode::BufferOverflow:
        interface.println(F("Buffer overflow error"));
        break;
    case SCPI_Parser::ErrorCode::Timeout:
        interface.println(F("Communication timeout error"));
        break;
    case SCPI_Parser::ErrorCode::UnknownCommand:
        interface.println(F("Unknown command received"));
        break;
    case SCPI_Parser::ErrorCode::NoError:
        interface.println(F("No Error"));
        break;
        // case SCPI_Parser::ErrorCode::InvalidParameter:
        //   interface.println(F("Command not allowed"));
        //   break;
    }
    scpiParser.last_error = SCPI_Parser::ErrorCode::NoError;
}

static void GetConfigureMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    interface.println(motorFlags.enable);
}

static void ConfigureMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface)
{
    String first_parameter = String(parameters.First());
    first_parameter.toUpperCase();
    if ((first_parameter == "ON") || (first_parameter == "1"))
    {
        motorFlags.enable = TRUE;
    }
    else if ((first_parameter == "OFF") || (first_parameter == "0"))
    {
        motorFlags.enable = FALSE;
    }
}

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
static void ConfigureMotorDutyCycleSource(SCPI_C commands, SCPI_P parameters, Stream &interface)
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
static void ConfigureMotorSpeedSource(SCPI_C commands, SCPI_P parameters, Stream &interface)
#endif
{
    String first_parameter = String(parameters.First());
    first_parameter.toUpperCase();

    if (first_parameter == SPEED_INPUT_SOURCE_LOCAL)
    {
        motorConfigs.speedInputSource = SPEED_INPUT_SOURCE_LOCAL;
    }
    else
    {
        motorConfigs.speedInputSource = SPEED_INPUT_SOURCE_REMOTE;
        speedInput = 0;
    }
}

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
static void ConfigureMotorDutyCycle(SCPI_C commands, SCPI_P parameters, Stream &interface)
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
static void ConfigureMotorSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface)
#endif
{
    String first_parameter = String(parameters.First());
    first_parameter.toUpperCase();
    speedInput = first_parameter.toInt();
}
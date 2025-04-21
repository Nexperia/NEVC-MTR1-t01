/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI implementation header file.

   \details
        This file contains the command definitions for the SCPI parser to recognize and
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

#ifndef _SCPI_H_
#define _SCPI_H_

#include "scpi_config.h"
#include "scpi_helper.h"

#define MOTOR_DIRECTION_OPTIONS 2
extern const SCPI_choice_def_t motorDirections[MOTOR_DIRECTION_OPTIONS];
#define INPUT_SOURCE_OPTIONS 2
extern const SCPI_choice_def_t inputSources[INPUT_SOURCE_OPTIONS];

/** @cond DOXYGEN_IGNORE */
// External prototypes and (defined in main.cpp or another relevant file)
extern void TimersInit(void);
extern void ConfigsInit(void);
extern volatile motorflags_t motorFlags;
extern volatile motorconfigs_t motorConfigs;
extern volatile faultflags_t faultFlags;
extern volatile uint16_t lastCommutationTicks;
extern volatile uint16_t ibus;
extern volatile int16_t iphaseU;
extern volatile int16_t iphaseV;
extern volatile int16_t iphaseW;
extern volatile uint16_t vbusVref;
extern volatile uint8_t speedInput;
/** @endcond */

// SCPI Parser Instance
extern SCPI_Parser scpiParser;

// Function Prototypes
void ScpiInit(void);
void ScpiInput(Stream &interface);

// Static Function Prototypes for SCPI Commands
static void ScpiCoreIdnQ(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ScpiSystemErrorCountQ(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ScpiSystemErrorNextQ(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void GetMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ConfigureMotorEnable(SCPI_C commands, SCPI_P parameters, Stream &interface);

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_OPEN_LOOP)
static void ConfigureMotorDutyCycleSource(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void GetConfigureMotorDutyCycleSource(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ConfigureMotorDutyCycle(SCPI_C commands, SCPI_P parameters, Stream &interface);
#elif (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
// Assuming scpi_result_t is the return type expected by the parser for commands
// with context (though this example doesn't use it)
// If Vrekrer_scpi_parser.h defines SCPI_C/P differently for context-based commands,
// adjust accordingly. For now, keeping it consistent with other commands.
static void ConfigureMotorSpeedSource(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void GetMotorSpeedSource(SCPI_C commands, SCPI_P parameters, Stream &interface);
static void ConfigureMotorSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface);
#endif

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

#endif // _SCPI_H_
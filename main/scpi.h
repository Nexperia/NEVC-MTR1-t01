/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI implementation header file.

   \details
        This file contains all the defines for configurations and function prototypes
        related to the SCPI implementation.

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

#ifndef _SCPI_H_
#define _SCPI_H_

#include "scpi_config.h"
#include "scpi_util.h"

// External Variables (defined in main.cpp or another relevant file)
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

// External prototypes
//! Initializes and synchronizes Timers.
extern void TimersInit(void);
//! Initializes motorConfigs.
extern void ConfigsInit(void);

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
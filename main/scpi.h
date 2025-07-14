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

#include "scpi_helper.h"
#include "config.h"

/*! \brief Motor direction options array. */
#define MOTOR_DIRECTION_OPTIONS 2
/*! \brief Number of motor direction options. */
extern const SCPI_choice_def_t motorDirections[MOTOR_DIRECTION_OPTIONS];
/*! \brief Motor direction options array. */
#define INPUT_SOURCE_OPTIONS 2
/*! \brief Number of speed input source options. */
extern const SCPI_choice_def_t inputSources[INPUT_SOURCE_OPTIONS];
/*! \brief Speed input source options array. */

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

/*!  \page scpi SCPI

     \section Introduction

     \subsection scpi_intro_overview Overview

     This project includes an implementation of the Standard Commands for
     Programmable Instruments (SCPI) protocol. SCPI is a widely-accepted
     standard for syntax and commands, designed to provide a uniform method of
     controlling test and measurement devices. For a quick overview of SCPI,
     please refer to \ref scpi_quick_guide "SCPI Quick Guide".

     This allows remote operation of the motor via any programming language or
     software compatible with SCPI.

     Please note the SCPI implementation in this project is intended primarily
     for evaluation purposes only.

     \subsection scpi_intro_compliance Compliance with Standards

     While our SCPI implementation adheres to the core principles of the SCPI
     standard and relevant IEEE standards, it is important to note that it may
     not fully comply with the full required specification. The implementation
     focuses on the essential commands and functionalities necessary for
     evaluation. For detailed command information, see \ref scpi_commands.

     \subsection scpi_intro_usage Using the SCPI Implementation

     The SCPI commands available in this project are designed to be intuitive
     and easy to use, adhering to the conventional SCPI syntax. Users familiar
     with SCPI will find the command structure familiar and straightforward. For
     those new to SCPI, ample documentation is provided to guide through the
     basic commands and their usage, as detailed in the \ref scpi_quick_guide
     and \ref scpi_commands pages.

     \subsection scpi_intro_customization Customization and Expansion

     The modular nature of our SCPI implementation allows for customization and
     expansion. Users with specific needs can extend or modify the existing
     command set. This flexibility is further detailed in the sections \ref
     scpi_quick_guide "SCPI Quick Guide" and \ref scpi_commands.

     \section scpi_quick_guide Quick Guide

     This guide provides a quick overview of SCPI and its usage.

     \subsection scpi_quick_guide_structure Command Structure

     SCPI commands typically follow a hierarchical structure resembling a file
     path, starting with a root and followed by nodes separated by colons or
     semicolons. For example: `MEASure:VOLTage:DC?`.

     - \b Root: Usually an abbreviation of the instrument function (e.g.,
       `MEASure`).
     - \b Nodes: Subcategories or actions within the function (e.g.,
       `MOTOr:SPEEd`).

     \subsection scpi_quick_guide_types Command Types

     There are two primary types of SCPI commands:

     - \b Queries: Request data or status from the instrument (e.g., `*IDN?`).
     - \b Settings: Configure the instrument or execute actions (e.g.,
       `CONFigure:ENABle`).

     \subsection scpi_quick_guide_syntax Syntax and Conventions

     - \b Case Insensitivity: SCPI commands are not case-sensitive.
     - \b Abbreviations: Commands can be abbreviated to the shortest unique
       string.
     - \b Query Suffix: Queries end with a question mark (`?`).
     - \b Command Separation: Multiple commands can be concatenated using a
       semicolon (`;`).

     \subsection scpi_quick_guide_command_forms Short and Long Forms of Commands

     From the command set documentation, you can deduce both the short and long
     forms of SCPI commands by examining the case used in the command syntax.

     For instance, take the command `CONFigure:ENABle`. The way it's
     written provides clues:

     - \b Long Form: The long form of the command is exactly as it's presented:
       `CONFigure:ENABle`. It is the complete, full-length command.
     - \b Short Form: The short form capitalizes only the minimum letters
       required to uniquely identify the command. Thus, for
       `CONFigure:ENABle`, the short form would be `CONF:ENAB`.

     This approach to determining short and long forms applies to all SCPI
     commands. The long form is always the full command, while the short form
     uses the least number of initial capitalized letters. This feature of SCPI
     provides a balance between readability and ease of typing, especially for
     users who interact with the commands frequently.

     \subsection scpi_quick_guide_examples Example Usage

     These examples demonstrate that SCPI commands are not case-sensitive and
     can be abbreviated to the shortest unique string:

     - Query the device identification: `*IDN?`
     - Set the motor direction: `CONFigure:DIREction FORWard`
     - Measure voltage: `MEASure:VOLTage:DC?`
     - Enable motor:
          - Standard command: `CONFigure:ENABle ON`
          - Mixed case: `conFigure:enabLE ON`
          - Abbreviated command: `CONF:ENAB ON`
          - Mixed case with abbreviation: `Conf:Enab ON`

     SCPI's flexibility with command syntax makes it user-friendly and adaptable
     to various styles of interaction.

     \subsection scpi_quick_guide_communication Communication

     SCPI commands are sent to the instrument via a communication interface,
     such as GPIB, USB, LAN, or RS-232.

     In this case, we are using a serial communication (like RS-232) via USB,
     and certain settings need to be correctly configured:

     - \b Baud Rate: Typically set to `115200` bits per second.
     - \b Line Ending: Use Line Feed (`LF`) as the line ending character.
     - \b Data Bits: Set the number of data bits to `8`.
     - \b Parity: Set parity to `None`.
     - \b Stop Bits: Use `1` stop bit.
     - \b DTR (Data Terminal Ready): Should be checked or enabled.
     - \b RTS (Request to Send): Should be checked or enabled.

     \note These were the only verified working configurations. If other
     configurations are to be used, please test and verify.

     After configuring these settings, the instrument processes the SCPI command
     and returns a response, especially for queries. This ensures reliable
     communication between the controlling system and the instrument.

     \subsection scpi_quick_guide_starting Starting with SCPI

     To start using SCPI:

     1. Connect to the instrument using a suitable interface.
     2. Send SCPI commands using the interface's communication protocol.
     3. Read responses from the instrument for queries.
     4. Use appropriate software or libraries that support SCPI communication.

     \subsection scpi_quick_guide_opportunities Opportunities

     SCPI provides a standardized and intuitive way to communicate with and
     control test and measurement equipment. Its uniform syntax across different
     devices simplifies the process of instrument automation and data
     acquisition.

     LabView is a popularly used in communicating with test and measurement
     equipment, and an example of LabView VI in the picture below shows the
     capabilities opportunities that the SCPI implementation can provide.

     \image html scpi_example_application.png "Figure: Example LabView VI for remote monitoring with the motor driver kit."

     \note This guide is a quick overview. For detailed SCPI specifications and
     commands, refer to the SCPI standard documentation.

     \section scpi_commands Command Sets

     \subsection scpi_commands_overview Overview

     This page provides documentation for the SCPI (Standard Commands for
     Programmable Instruments) command sets implemented in this project. The
     commands are categorized based on their functions and compliance with SCPI
     and IEEE standards.

     \subsection ieee_mandated_commands IEEE Mandated Commands

     These commands are some of mandated by the SCPI standard (V1999.0 Section
     4.1.1) that must be implemented in any SCPI-compliant device.

     \note Not all mandatory commands are implemented.

     | Command | Description | Parameters | Return Value |
     |---------|-------------|------------|--------------|
     | `*IDN?` | Queries the identification string of the instrument. | None. | The identification string. |

     \subsection scpi_commands_required Required SCPI Commands

     These commands are some of the required commands as per SCPI standard
     (V1999.0 Section 4.2.1).

     \note Not all mandatory commands are implemented.

     | Command                   | Description                                 | Parameters | Return Value                                   |
     |---------------------------|---------------------------------------------|------------|------------------------------------------------|
     | `SYSTem:ERRor?`    | Retrieves the next error from the error queue. | None.     | The next error message or `0, "No error"` if none. |
     | `SYSTem:ERRor:COUNt?`     | Queries the count of errors in the error queue. | None.    | The number of errors in the queue.              |

     \subsection scpi_commands_motor Motor Control Commands

     Commands specific to motor control.

     | Command                                   | Description                                  | Parameters                                                      | Return Value                                                    |
     |-------------------------------------------|----------------------------------------------|-----------------------------------------------------------------|-----------------------------------------------------------------|
     | `CONFigure:ENABle`                  | Configures the motor enable state.           | Boolean (`ON` or `1` to enable, `OFF` or `0` to disable).       | None or error code and message if incorrect parameter.          |
     | `CONFigure:ENABle?`                 | Queries the motor enable state.              | None.                                                           | Boolean state of the motor (`1` if enabled or `0` if disabled). |
     | `CONFigure:FREQuency`          | Sets the gate drive frequency for the motor. | Frequency value in Hertz (Hz). Minimum `7183` Hz, maximum `100000` Hz. | None or error code and message if the frequency is out of range. |
     | `CONFigure:FREQuency?`         | Queries the gate drive frequency.            | None.                                                           | Current gate drive frequency in Hertz (Hz).                     |
     | `CONFigure:DIREction`               | Sets the motor direction.                    | Direction value (`FORWard` or `REVErse`).                      | None or error code and message if incorrect parameter.          |
     | `CONFigure:DIREction?`              | Queries the motor direction.                 | None.                                                           | The configured motor direction as a string (`FORWard` or `REVErse`). |
     | `MEASure:SPEEd?`                    | Measures the motor speed.                    | None.                                                           | Motor speed in revolutions per minute (RPM).                    |
     | `MEASure:CURRent?`                  | Measures the motor current.                  | None.                                                           | Motor current in Amperes (A).                                   |
     | `MEASure:DIREction?`                | Measures the motor direction.                | None.                                                           | The motor direction as a string (`FORWard`, `REVErse` or `UNKNown`). |
     | `MEASure:VOLTage?`             | Measures the gate voltage.                   | None.                                                           | Gate voltage in Volts (V).                                      |

     These commands are only available when \ref SPEED_CONTROL_METHOD is \ref
     SPEED_CONTROL_OPEN_LOOP.

     | Command                                   | Description                                  | Parameters                                                      | Return Value                                                    |
     |-------------------------------------------|----------------------------------------------|-----------------------------------------------------------------|-----------------------------------------------------------------|
     | `CONFigure:DUTYcycle:SOURce`   | Sets the duty cycle source for the motor.    | `0` for local (speed input pin) and `1` for remote.             | None or error code and message if incorrect parameter.          |
     | `CONFigure:DUTYcycle`          | Sets the duty cycle for the motor.           | Duty cycle in percentage (%). Minimum 0.0 %, maximum 100.0 %    | None or error code and message if incorrect parameter.          |

     These commands are only available when \ref SPEED_CONTROL_METHOD is \ref
     SPEED_CONTROL_CLOSED_LOOP.

     | Command                          | Description                             | Parameters                                                      | Return Value                                                    |
     |----------------------------------|-----------------------------------------|-----------------------------------------------------------------|-----------------------------------------------------------------|
     | `CONFigure:SPEEd:SOURce`   | Sets the speed source for the motor.    | `0` for local (speed input pin) and `1` for remote.             | None or error code and message if incorrect parameter.          |
     | `CONFigure:SPEEd`          | Sets the speed for the motor.           | Speed in revolutions per minute (RPM). Minimum 0 RPM, maximum \ref SPEED_CONTROLLER_MAX_SPEED | None or error code and message if incorrect parameter. |

     \subsection scpi_commands_conclusion Conclusion

     This document provides a comprehensive overview of the SCPI command sets
     implemented in the project. It covers various categories including IEEE
     mandated commands, required SCPI commands, status subsystem commands, and
     motor control commands. For uncertain or future commands, placeholders are
     provided.

     \warning Due to low memory, a smaller input buffer is used. This means it
     is preferred not to concatenate commands using a semicolon (`;`).
*/

#endif // _SCPI_H_
/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI parser configuration header file.

   \details
        This header file defines various configuration constants for the SCPI parser library.
        These constants control the sizes of internal buffers and arrays, the maximum
        number of supported tokens and commands, and the data type used for command hashing.
        Users can modify these definitions to adjust the parser's capabilities and memory
        usage according to their specific application requirements.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2025/04/21 $  \n

 ******************************************************************************/

#ifndef _SCPI_CONFIG_H_
#define _SCPI_CONFIG_H_

/*!
 * \file scpi_config.h
 * \brief Header file for SCPI parser configuration.
 *
 * \details
 * This header file defines various configuration constants for the SCPI parser library.
 * These constants control the sizes of internal buffers and arrays, the maximum
 * number of supported tokens and commands, and the data type used for command hashing.
 * Users can modify these definitions to adjust the parser's capabilities and memory
 * usage according to their specific application requirements.
 */

/*! \def SCPI_MAX_TOKENS
 * \brief Maximum number of valid SCPI tokens (keywords) that the parser can recognize.
 *
 * \details
 * This constant determines the size of the internal storage for unique command tokens
 * extracted from registered commands. Increasing this value allows for more complex
 * command structures with a larger vocabulary of keywords, but also increases memory usage.
 * Default value is 20.
 */
#define SCPI_MAX_TOKENS 20

/*! \def SCPI_MAX_COMMANDS
 * \brief Maximum number of distinct SCPI commands that can be registered with the parser.
 *
 * \details
 * This constant defines the capacity of the internal storage for registered command
 * hash codes and their associated callback functions. Increasing this value allows
 * the parser to handle a larger set of unique SCPI commands, but also increases memory usage.
 * Default value is 20.
 */
#define SCPI_MAX_COMMANDS 20

/*! \def SCPI_MAX_SPECIAL_COMMANDS
 * \brief Maximum number of special SCPI commands (without parameters) that can be registered.
 *
 * \details
 * This constant defines the capacity of the internal storage for registered special
 * command hash codes and their associated callback functions. Special commands are
 * typically used for actions or queries that do not require parameters. Setting this
 * value to 0 disables the support for special commands, which can save memory if not needed.
 * Default value is 0.
 */
#define SCPI_MAX_SPECIAL_COMMANDS 0

/*! \def SCPI_BUFFER_LENGTH
 * \brief Length of the buffer used to store incoming SCPI messages from the communication interface.
 *
 * \details
 * This constant determines the maximum size of a single SCPI message that the parser can
 * receive and process. Messages exceeding this length will be truncated, and a buffer
 * overflow error will be triggered. Adjusting this value should be based on the expected
 * length of the SCPI commands used by the instrument. Default value is 64 bytes.
 */
#define SCPI_BUFFER_LENGTH 64

/*! \def SCPI_ARRAY_SYZE
 * \brief Maximum branch size of the command tree and the maximum number of parameters that can be parsed from a command.
 *
 * \details
 * This constant serves two purposes: it limits the depth of the hierarchical command
 * tree that can be defined using the `SetCommandTreeBase` function, and it also
 * limits the maximum number of parameters that the parser will attempt to extract
 * from a received command. Default value is 6.
 */
#define SCPI_ARRAY_SYZE 6

/*! \def SCPI_HASH_TYPE
 * \brief Integer data type used for calculating and storing command hash codes.
 *
 * \details
 * This constant defines the underlying integer type used for the hash values generated
 * for registered SCPI commands. The size of this data type affects the range of possible
 * hash values and thus the likelihood of hash collisions. Common choices include `uint8_t`,
 * `uint16_t`, or `uint32_t`. Default value is `uint8_t`.
 */
#define SCPI_HASH_TYPE uint8_t

// SCPI Identification Definitions
/*! \def SCPI_IDN1
 * \brief Manufacturer identification string for the `*IDN?` command.
 *
 * \details
 * This constant defines the first field of the identification string returned by
 * the standard SCPI `*IDN?` command. Default value is "NEXPERIA".
 */
#define SCPI_IDN1 "NEXPERIA"

/*! \def SCPI_IDN2
 * \brief Model number identification string for the `*IDN?` command.
 *
 * \details
 * This constant defines the second field of the identification string returned by
 * the standard SCPI `*IDN?` command. Default value is "NEVB-MTR1-xx".
 */
#define SCPI_IDN2 "NEVB-MTR1-xx"

/*! \def SCPI_IDN3
 * \brief Revision level identification string for the `*IDN?` command (optional).
 *
 * \details
 * This constant defines the third field of the identification string returned by
 * the standard SCPI `*IDN?` command. It is often left empty if no specific
 * revision information is needed. Default value is "".
 */
#define SCPI_IDN3 ""

/*! \def SCPI_IDN4
 * \brief Firmware version identification string for the `*IDN?` command.
 *
 * \details
 * This constant defines the fourth field of the identification string returned by
 * the standard SCPI `*IDN?` command, typically representing the firmware version.
 * Default value is "NEVC-MTR1-t01-1.0.0".
 */
#define SCPI_IDN4 "NEVC-MTR1-t01-1.0.0"

#endif
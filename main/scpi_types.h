/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI types header file.

   \details
        This file contains the declaration of helper classes used for SCPI
        command parsing, including string array management and tokenization
        of command and parameter messages.

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

#ifndef _SCPI_TYPES_CODE_H
#define _SCPI_TYPES_CODE_H

#include <Arduino.h>
#include "scpi_config.h"

/*!
 * \class SCPI_String_Array
 * \brief Variable size string array class.
 *
 * This class represents a simple container for storing and managing a list of string tokens.
 *
 * Strings can be added using \c Append (which acts like a LIFO stack push),
 * and removed using \c Pop. The current size can be queried via \c Size.
 * Indexing is supported via \c operator[]. Methods \c First and \c Last return
 * the respective elements in the array.
 *
 * Overflow protection is built-in, and the array size is bounded by
 * the compile-time constant \c SCPI_ARRAY_SYZE.
 */
class SCPI_String_Array
{
public:
  char *operator[](const byte index) const;     ///< Read-only array indexing.
  void Append(char *value);                     ///< Append a new string to the array (LIFO push).
  char *Pop();                                  ///< Remove and return the last string (LIFO pop).
  char *First() const;                          ///< Get the first appended string.
  char *Last() const;                           ///< Get the last appended string.
  uint8_t Size() const;                         ///< Get the number of stored strings.
  bool overflow_error = false;                  ///< Flag set when exceeding \c storage_size.
  const uint8_t storage_size = SCPI_ARRAY_SYZE; ///< Max number of entries allowed.
protected:
  uint8_t size_ = 0;              ///< Current size of the array.
  char *values_[SCPI_ARRAY_SYZE]; ///< Internal storage for string pointers.
};

/*!
 * \class SCPI_Commands
 * \brief Stores parsed command tokens.
 *
 * Derived from \c SCPI_String_Array, this class is used to tokenize an SCPI
 * command message into parts separated by ':'.
 *
 * Remaining unparsed text is stored in \c not_processed_message.
 */
class SCPI_Commands : public SCPI_String_Array
{
public:
  SCPI_Commands();              ///< Default constructor.
  SCPI_Commands(char *message); ///< Parse command tokens from a message.
  char *not_processed_message;  ///< Remaining message text after token parsing.
};

/*!
 * \class SCPI_Parameters
 * \brief Stores parsed command parameters.
 *
 * Derived from \c SCPI_String_Array, this class splits a message using commas
 * to extract and store parameters following an SCPI command.
 *
 * Whitespace before each parameter is trimmed. The remaining unprocessed
 * string is stored in \c not_processed_message.
 */
class SCPI_Parameters : public SCPI_String_Array
{
public:
  SCPI_Parameters();              ///< Default constructor.
  SCPI_Parameters(char *message); ///< Parse parameters from a message.
  char *not_processed_message;    ///< Remaining message text after parsing.
};

/// \typedef SCPI_C
/// \brief Alias for \c SCPI_Commands.
using SCPI_C = SCPI_Commands;

/// \typedef SCPI_P
/// \brief Alias for \c SCPI_Parameters.
using SCPI_P = SCPI_Parameters;

/// \typedef scpi_hash_t
/// \brief Alias for SCPI integer hash type defined in \c SCPI_HASH_TYPE.
using scpi_hash_t = SCPI_HASH_TYPE;

/// \enum ErrorCode
/// \brief SCPI Error codes.
///
/// Represents possible error conditions that can occur during SCPI message
/// parsing or execution.
enum class ErrorCode
{
  /// No error occurred.
  NoError = 0,

  /// An unknown command was received that could not be matched to a known handler.
  UnknownCommand,

  /// A timeout occurred before receiving the expected termination characters.
  Timeout,

  /// The message buffer was exceeded during message reception.
  BufferOverflow,

  /// A required parameter was missing or an invalid parameter was provided.
  MissingOrInvalidParameter,
};

#endif
/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI types implementation source file.

   \details
        This file contains the implementation of helper classes used for SCPI
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

#include "scpi_types.h"

// ========================== SCPI_String_Array member functions ==========================

/*!
 * \brief Indexing operator.
 *
 * Provides array-like access to internal values.
 *
 * \param index Index of the element to retrieve.
 * \return Pointer to the string at the given index, or NULL if out of bounds.
 */
char *SCPI_String_Array::operator[](const uint8_t index) const
{
  if (index >= size_)
    return NULL; // Invalid index
  return values_[index];
}

/*!
 * \brief Append a new string to the array.
 *
 * Acts like a LIFO stack push. If the array is full, sets `overflow_error`.
 *
 * \param value Pointer to the null-terminated string to append.
 */
void SCPI_String_Array::Append(char *value)
{
  overflow_error = (size_ >= storage_size);
  if (overflow_error)
    return;
  values_[size_] = value;
  size_++;
}

/*!
 * \brief Pop the last string from the array.
 *
 * Acts like a LIFO stack pop. Removes and returns the last inserted string.
 *
 * \return Pointer to the last string, or NULL if the array is empty.
 */
char *SCPI_String_Array::Pop()
{
  if (size_ == 0)
    return NULL; // Empty array
  size_--;
  return values_[size_];
}

/*!
 * \brief Get the first element in the array.
 *
 * \return Pointer to the first string, or NULL if empty.
 */
char *SCPI_String_Array::First() const
{
  if (size_ == 0)
    return NULL;
  return values_[0];
}

/*!
 * \brief Get the last element in the array.
 *
 * \return Pointer to the last string, or NULL if empty.
 */
char *SCPI_String_Array::Last() const
{
  if (size_ == 0)
    return NULL;
  return values_[size_ - 1];
}

/*!
 * \brief Get the current number of strings in the array.
 *
 * \return Number of elements in the array.
 */
uint8_t SCPI_String_Array::Size() const
{
  return size_;
}

// ========================== SCPI_Commands member functions ==========================

/*!
 * \brief Default constructor.
 */
SCPI_Commands::SCPI_Commands() {}

/*!
 * \brief Construct and tokenize a SCPI command from a message.
 *
 * Splits the input string on ':' characters, storing resulting tokens in the array.
 * Terminates tokenization at the first whitespace (space or tab), storing the rest
 * in `not_processed_message`.
 *
 * \param message Null-terminated string containing the SCPI command message.
 */
SCPI_Commands::SCPI_Commands(char *message)
{
  char *token = message;
  // Trim leading spaces and tabs
  while (isspace(*token))
    token++;
  // Save parameters and multicommands for later
  not_processed_message = strpbrk(token, " \t");
  if (not_processed_message != NULL)
  {
    not_processed_message[0] = '\0';
    not_processed_message++;
  }
  // Split using ':'
  token = strtok(token, ":");
  while (token != NULL)
  {
    this->Append(token);
    token = strtok(NULL, ":");
  }
}

// ========================== SCPI_Parameters member functions ==========================

/*!
 * \brief Default constructor.
 */
SCPI_Parameters::SCPI_Parameters() {}

/*!
 * \brief Construct and tokenize parameters from a message.
 *
 * Splits the input string on ',' characters and trims leading whitespace for each parameter.
 * Stores each parameter as a separate element in the array.
 *
 * \param message Null-terminated string containing the SCPI parameter message.
 */
SCPI_Parameters::SCPI_Parameters(char *message)
{
  char *parameter = message;
  // Split using ','
  parameter = strtok(parameter, ",");
  while (parameter != NULL)
  {
    while (isspace(*parameter))
      parameter++;
    this->Append(parameter);
    parameter = strtok(NULL, ",");
  }
  // TODO add support for strings parameters (do not split parameters inside "")
}

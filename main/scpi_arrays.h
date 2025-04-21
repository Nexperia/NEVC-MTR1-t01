#ifndef _SCPI_ARRAYS_CODE_H
#define _SCPI_ARRAYS_CODE_H

#include <Arduino.h>
#include "scpi_config.h"

/*!
 Variable size string array class.

 The array must be filled using the \c Append method (acts as a LIFO stack Push). \n
 Values can be extracted (and removed) using the \c Pop function (LIFO stack Pop). \n
 Both \c Append and \c Pop modifies the \c Size of the array. \n
 Values can be read (without removing them) using the following methods: \n
  \li \c First() : Returns the first value appended to the array.
  \li \c Last()  : Returns the last value appended to the array.
  \li Indexing (e.g. \c my_array[1] to get the second value of the array).

 The max size of the array is defined by \c SCPI_ARRAY_SYZE (default 6).
*/
class SCPI_String_Array
{
public:
  char *operator[](const byte index) const;     // Add indexing capability
  void Append(char *value);                     // Append new string (LIFO stack Push)
  char *Pop();                                  // LIFO stack Pop
  char *First() const;                          // Returns the first element of the array
  char *Last() const;                           // Returns the last element of the array
  uint8_t Size() const;                         // Array size
  bool overflow_error = false;                  // Storage overflow error
  const uint8_t storage_size = SCPI_ARRAY_SYZE; // Max size of the array
protected:
  uint8_t size_ = 0;              // Internal array size
  char *values_[SCPI_ARRAY_SYZE]; // Storage of the strings
};

/*!
 String array class used to store the tokens of a command.
 @see SCPI_String_Array
*/
class SCPI_Commands : public SCPI_String_Array
{
public:
  // Dummy constructor.
  SCPI_Commands();
  // Constructor that extracts and tokenize a command from a message
  SCPI_Commands(char *message);
  /// Not processed part of the message after the constructor is called.
  char *not_processed_message;
};

/*!
 String array class used to store the parameters found after a command.
 @see SCPI_String_Array
*/
class SCPI_Parameters : public SCPI_String_Array
{
public:
  // Dummy constructor.
  SCPI_Parameters();
  // Constructor that extracts and splits parameters from a message
  SCPI_Parameters(char *message);
  /// Not processed part of the message after the constructor is called.
  char *not_processed_message;
};

/// Alias of SCPI_Commands.
using SCPI_C = SCPI_Commands;

/// Alias of SCPI_Parameters.
using SCPI_P = SCPI_Parameters;

/// Integer size used for hashes.
using scpi_hash_t = SCPI_HASH_TYPE;

#endif
#ifndef _SCPI_UTIL_H_
#define _SCPI_UTIL_H_

/// Customise Vrekrer_scpi_parser.h
#define SCPI_CUSTOM_ERROR_CODES
#define SCPI_MAX_TOKENS 20

/// SCPI Error codes.
enum class ErrorCode
{
    /// No error
    NoError = 0,
    /// Unknown command received.
    UnknownCommand,
    /// Timeout before receiving the termination chars.
    Timeout,
    /// Message buffer overflow.
    BufferOverflow,
    /// Missing or invalid parameter.
    MissingOrInvalidParameter,
};

#include "config.h"
#include "src/scpi_parser/src/Vrekrer_scpi_parser.h"

typedef struct _SCPI_choice_def_t
{
    String stem;
    String suffix;
    int8_t tag;
} SCPI_choice_def_t;
#define MOTOR_DIRECTION_OPTIONS 2
extern const SCPI_choice_def_t motorDirections[MOTOR_DIRECTION_OPTIONS];
#define INPUT_SOURCE_OPTIONS 2
extern const SCPI_choice_def_t inputSources[INPUT_SOURCE_OPTIONS];

uint8_t ScpiParamString(SCPI_P &parameters, String &param);
uint8_t ScpiParamUInt8(SCPI_P &parameters, uint8_t &param);
uint8_t ScpiParamUInt32(SCPI_P &parameters, uint32_t &param);
uint8_t ScpiParamDouble(SCPI_P &parameters, double &param);
uint8_t ScpiParamBool(SCPI_P &parameters, bool &param);
uint8_t ScpiParamInt8(SCPI_P &parameters, int8_t &param);
uint8_t ScpiParamChoice(SCPI_P &parameters, const SCPI_choice_def_t *options, size_t optionsSize, uint8_t &param);
uint8_t ScpiChoiceToName(const SCPI_choice_def_t *options, size_t optionsSize, int8_t value, String &name);
#endif
#include "scpi_util.h"

uint8_t ScpiParamString(SCPI_P &parameters, String &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    param = String(parameters.Pop());
    return TRUE;
}

uint8_t ScpiParamUInt8(SCPI_P &parameters, uint8_t &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    param = String(parameters.Pop()).toInt();
    return TRUE;
}

uint8_t ScpiParamUInt32(SCPI_P &parameters, uint32_t &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    param = String(parameters.Pop()).toInt();
    return TRUE;
}

uint8_t ScpiParamDouble(SCPI_P &parameters, double &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    param = String(parameters.Pop()).toDouble();
    return TRUE;
}

uint8_t ScpiParamBool(SCPI_P &parameters, bool &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    String rawParam = String(parameters.Pop());
    rawParam.toUpperCase();

    if (rawParam == "ON" || rawParam == "1")
        param = TRUE;
    else if (rawParam == "OFF" || rawParam == "0")
        param = FALSE;
    else
        return FALSE;
    return TRUE;
}

uint8_t ScpiParamChoice(SCPI_P &parameters, const SCPI_choice_def_t *options, size_t optionsSize, uint8_t &param)
{
    String paramStr;
    uint8_t result = ScpiParamString(parameters, paramStr);
    if (result)
    {
        // Check if the parsed string matches any of the valid choices
        for (size_t i = 0; i < optionsSize; i++)
        {
            if (paramStr.equalsIgnoreCase(options[i].stem) || paramStr.equalsIgnoreCase(options[i].stem + options[i].suffix))
            {
                param = options[i].tag;
                return TRUE;
            }
        }
    }
    return FALSE;
}

uint8_t ScpiChoiceToName(const SCPI_choice_def_t *options, size_t optionsSize, int8_t value, String &name)
{
    for (size_t i = 0; i < optionsSize; i++)
    {
        if (options[i].tag == value)
        {
            name = options[i].stem + options[i].suffix;
            return TRUE;
        }
    }
    return FALSE;
}

/**
   \brief Array or enumeration of possible motor directions.

   This defines the possible directions in which the motor can move, i.e.
   forward, reverse. It is used in SCPI commands to set or query the motor's
   current direction. Each direction is typically associated with a specific
   command or numeric value.
*/
const SCPI_choice_def_t motorDirections[MOTOR_DIRECTION_OPTIONS] = {
    {"FORW", "ard", DIRECTION_FORWARD},
    {"REVE", "rse", DIRECTION_REVERSE},
};

/**
   \brief Array or enumeration of possible input sources.

   This defines the possible input sources for parameters, i.e.
   local, remote.
*/
const SCPI_choice_def_t inputSources[INPUT_SOURCE_OPTIONS] = {
    {"LOCA", "l", SPEED_INPUT_SOURCE_LOCAL},
    {"REMO", "te", SPEED_INPUT_SOURCE_REMOTE},
};
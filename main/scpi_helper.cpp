/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI helper implementation file.

   \details
        This file implements a set of helper functions designed to simplify
        the process of extracting and converting parameters from the SCPI parser
        and mapping choice values to their string representations. These functions
        are intended to be used in the implementation of SCPI commands.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2025/04/21 $  \n

 ******************************************************************************/

#include "scpi_helper.h"

/**
 * \brief Extracts a string parameter from the SCPI parameter list.
 *
 * This function checks if there are any parameters available. If so, it pops
 * the last parameter from the list and converts it to a String.
 *
 * \param parameters A reference to the SCPI parameter list.
 * \param param A reference to a String variable where the extracted parameter will be stored.
 * \return 1 if a string parameter was successfully extracted, 0 otherwise (if no parameters are available).
 */
uint8_t ScpiParamString(SCPI_P &parameters, String &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    param = String(parameters.Pop());
    return TRUE;
}

/**
 * \brief Extracts an unsigned 8-bit integer parameter from the SCPI parameter list.
 *
 * This function checks if there are any parameters available. If so, it pops
 * the last parameter from the list and converts it to an unsigned 8-bit integer.
 *
 * \param parameters A reference to the SCPI parameter list.
 * \param param A reference to a uint8_t variable where the extracted parameter will be stored.
 * \return 1 if an unsigned 8-bit integer parameter was successfully extracted, 0 otherwise (if no parameters are available).
 */
uint8_t ScpiParamUInt8(SCPI_P &parameters, uint8_t &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    param = String(parameters.Pop()).toInt();
    return TRUE;
}

/**
 * \brief Extracts an unsigned 32-bit integer parameter from the SCPI parameter list.
 *
 * This function checks if there are any parameters available. If so, it pops
 * the last parameter from the list and converts it to an unsigned 32-bit integer.
 *
 * \param parameters A reference to the SCPI parameter list.
 * \param param A reference to a uint32_t variable where the extracted parameter will be stored.
 * \return 1 if an unsigned 32-bit integer parameter was successfully extracted, 0 otherwise (if no parameters are available).
 */
uint8_t ScpiParamUInt32(SCPI_P &parameters, uint32_t &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    param = String(parameters.Pop()).toInt();
    return TRUE;
}

/**
 * \brief Extracts a double-precision floating-point parameter from the SCPI parameter list.
 *
 * This function checks if there are any parameters available. If so, it pops
 * the last parameter from the list and converts it to a double.
 *
 * \param parameters A reference to the SCPI parameter list.
 * \param param A reference to a double variable where the extracted parameter will be stored.
 * \return 1 if a double-precision floating-point parameter was successfully extracted, 0 otherwise (if no parameters are available).
 */
uint8_t ScpiParamDouble(SCPI_P &parameters, double &param)
{
    if (parameters.Size() == 0)
        return FALSE;
    param = String(parameters.Pop()).toDouble();
    return TRUE;
}

/**
 * \brief Extracts a boolean parameter ('ON', '1', 'OFF', or '0') from the SCPI parameter list.
 *
 * This function checks if there are any parameters available. If so, it pops
 * the last parameter from the list, converts it to uppercase, and checks if it
 * matches "ON", "1" (for true) or "OFF", "0" (for false).
 *
 * \param parameters A reference to the SCPI parameter list.
 * \param param A reference to a bool variable where the extracted parameter will be stored.
 * \return 1 if a valid boolean parameter was successfully extracted, 0 otherwise (if no parameters are available or the parameter is invalid).
 */
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

/**
 * \brief Extracts a choice parameter from the SCPI parameter list and maps it to a numerical tag.
 *
 * This function first extracts a string parameter. If successful, it iterates
 * through the provided list of valid choices and compares the extracted string
 * (case-insensitively) with the stem or the full stem and suffix of each choice.
 * If a match is found, the corresponding numerical tag is stored in the output parameter.
 *
 * \param parameters A reference to the SCPI parameter list.
 * \param options A pointer to an array of SCPI_choice_def_t structures defining the valid choices.
 * \param optionsSize The number of elements in the \a options array.
 * \param param A reference to a uint8_t variable where the numerical tag of the matched choice will be stored.
 * \return 1 if a valid choice parameter was found and its tag was extracted, 0 otherwise (if no parameters are available or no match is found).
 */
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

/**
 * \brief Converts a numerical choice tag back to its string representation.
 *
 * This function iterates through the provided list of valid choices and compares
 * the input numerical tag with the tag of each choice. If a match is found,
 * the stem and suffix of the corresponding choice are concatenated and stored
 * in the output string.
 *
 * \param options A pointer to an array of SCPI_choice_def_t structures defining the valid choices.
 * \param optionsSize The number of elements in the \a options array.
 * \param value The numerical tag to convert to a string.
 * \param name A reference to a String variable where the string representation of the tag will be stored.
 * \return 1 if a matching choice was found and its name was stored in \a name, 0 otherwise (if no matching tag is found).
 */
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
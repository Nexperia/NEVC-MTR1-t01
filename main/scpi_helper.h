/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI helper header file.

   \details
        This header file declares a set of helper functions designed to simplify
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

#ifndef _SCPI_HELPER_H_
#define _SCPI_HELPER_H_

#include "config.h"
#include "scpi_parser.h"

/*! \struct _SCPI_choice_def_t
 * \brief Defines a structure for SCPI choice options.
 *
 * \details
 * This structure is used to define the possible choices for SCPI parameters
 * that accept a limited set of string values. It includes the stem and suffix
 * of the command keyword, as well as a numerical tag associated with the choice.
 */
typedef struct _SCPI_choice_def_t
{
    String stem;   /*!< Choice keyword stem. */
    String suffix; /*!< Choice keyword suffix. */
    int8_t tag;    /*!< Numerical tag. */
} SCPI_choice_def_t;

// Prototypes
uint8_t ScpiParamString(SCPI_P &parameters, String &param);
uint8_t ScpiParamUInt8(SCPI_P &parameters, uint8_t &param);
uint8_t ScpiParamUInt32(SCPI_P &parameters, uint32_t &param);
uint8_t ScpiParamDouble(SCPI_P &parameters, double &param);
uint8_t ScpiParamBool(SCPI_P &parameters, bool &param);
uint8_t ScpiParamInt8(SCPI_P &parameters, int8_t &param);
uint8_t ScpiParamChoice(SCPI_P &parameters, const SCPI_choice_def_t *options, size_t optionsSize, uint8_t &param);
uint8_t ScpiChoiceToName(const SCPI_choice_def_t *options, size_t optionsSize, int8_t value, String &name);

#endif // _SCPI_HELPER_H_
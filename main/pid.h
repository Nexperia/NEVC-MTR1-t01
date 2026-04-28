/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file ********************************************************************

   \brief
        PID controller header file.

   \details
        This file contains defines, typedefs and prototypes for the PID
        controller.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2024/03/08 $  \n

 ******************************************************************************/

#ifndef PID_H
#define PID_H

// Include standard integer type definitions
#include "stdint.h"

/*! \brief Scaling division factor for PID controller.

    Scaling division factor for the PID controller. The P, I and D gains will be
    divided by this number.
*/
#define SCALING_FACTOR 256

/*! \brief Flag indicating whether scaling factor is enabled for PID controller.

    The scaling factor can be adjusted by modifying the value of \ref
    SCALING_FACTOR.
*/
#define SCALING_FACTOR_ENABLED FALSE

/*! \brief PID Status

   Set points and data used by the PID control algorithm
*/
typedef struct pidData
{
     //! Last process value, used to find derivative of process value.
     int16_t lastProcessValue;
     //! Summation of errors, used for integrate calculations
     int32_t sumError;
     //! The Proportional tuning constant, given in x100
     int16_t P_Factor;
     //! The Integral tuning constant, given in x100
     int16_t I_Factor;
     //! The Derivative tuning constant, given in x100
     int16_t D_Factor;
     //! Maximum allowed error, avoid overflow
     int16_t maxError;
     //! Maximum allowed sum error, avoid overflow
     int32_t maxSumError;
#if (PID_K_D_ENABLE == TRUE)
     //! The D-term represents the rate of change of the error
     int16_t d_term;
#endif
     //! The P-term represents the immediate response to the current error
     int16_t p_term;
     //! The I-term accumulates the error over time to eliminate steady-state
     //! errors
     int32_t i_term;
} pidData_t;

//! Maximum value of integers
#define MAX_INT 32767

//! Maximum value of long integers
#define MAX_LONG 2147483647L

/*! \brief Maximum value of the I-term.

    This limits the maximum positive or negative value of the I-term, providing
    integral anti-windup. Prefer setting \ref PID_MAX_I_TERM in \ref config.h
    to override this value. The fallback here is the maximum safe value given
    the underlying 32-bit data types.
*/
#ifndef MAX_I_TERM
#define MAX_I_TERM (MAX_LONG - (2 * (int32_t)MAX_INT))
#endif

// Boolean values
//! FALSE constant.
#ifndef FALSE
#define FALSE 0
#endif
//! TRUE constant.
#ifndef TRUE
#define TRUE (!FALSE)
#endif

// Function prototypes
void PIDInit(int16_t p_factor, int16_t i_factor, int16_t d_factor, pidData_t *pid);
uint16_t PIDController(int16_t setPoint, int16_t processValue, pidData_t *pid_st);
void PIDResetIntegrator(pidData_t *pid_st);

#endif /* PID_H */

/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file ********************************************************************

   \brief
        PID controller source file.

   \details
        This file contains the implementation of the PID controller.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2024/03/08 $  \n

 ******************************************************************************/

// Include motor config (provides MAX_I_TERM via PID_MAX_I_TERM)
#include "config.h"
// Include PID header
#include "pid.h"

/*! \brief Initialisation of PID controller parameters.

    Initialise the variables used by the PID algorithm.

    \param p_factor  Proportional term. \param i_factor  Integral term. \param
    d_factor  Derivate term. \param pid  Struct with PID status.
*/
void PIDInit(int16_t p_factor, int16_t i_factor, int16_t d_factor, pidData_t *pid)
// Set up PID controller parameters
{
  // Start values for PID controller
  pid->sumError = 0;
  pid->lastProcessValue = 0;
  // Tuning constants for PID loop
  pid->P_Factor = p_factor;
  pid->I_Factor = i_factor;
  pid->D_Factor = d_factor;
  // Limits to avoid overflow
  pid->maxError = MAX_INT / pid->P_Factor;
  pid->maxSumError = MAX_I_TERM / pid->I_Factor;
}

/*! \brief PID control algorithm.

    Calculates output from set point, process value, and PID status.

    \param setPoint  Desired value. \param processValue  Measured value. \param
    pid_st  PID status struct. \return Calculated control output as a 16-bit
    unsigned integer.
*/
uint16_t PIDController(int16_t setPoint, int16_t processValue, pidData_t *pid_st)
{
  int32_t ret;
  int32_t temp;
  int16_t error;

  error = setPoint - processValue;

  // Calculate "P" term and limit error overflow
  if (error > pid_st->maxError)
  {
    pid_st->p_term = MAX_INT;
  }
  else if (error < -pid_st->maxError)
  {
    pid_st->p_term = -MAX_INT;
  }
  else
  {
    pid_st->p_term = pid_st->P_Factor * error / 1000;
  }

  // Calculate "I" term and limit integral runaway.
  // i_term is always derived from sumError so the clamp is continuous.
  temp = pid_st->sumError + error;
  if (temp > pid_st->maxSumError)
  {
    pid_st->sumError = pid_st->maxSumError;
  }
  else if (temp < -pid_st->maxSumError)
  {
    pid_st->sumError = -pid_st->maxSumError;
  }
  else
  {
    pid_st->sumError = temp;
  }
  pid_st->i_term = pid_st->I_Factor * pid_st->sumError / 1000;

#if (PID_K_D_ENABLE == TRUE)
  // Calculate "D" term
  pid_st->d_term = pid_st->D_Factor * (pid_st->lastProcessValue - processValue) / 1000;
#endif

  pid_st->lastProcessValue = processValue;

#if (PID_K_D_ENABLE == TRUE)
  ret = (pid_st->p_term + pid_st->i_term + pid_st->d_term);
#else
  ret = (pid_st->p_term + pid_st->i_term);
#endif

#if (SCALING_FACTOR_ENABLED == TRUE)
  ret = ret / SCALING_FACTOR;
#endif
  if (ret > MAX_INT)
  {
    ret = MAX_INT;
  }
  // Since the return type is uint16_t
  else if (ret < 0)
  {
    ret = 0;
  }

  return ((uint16_t)ret);
}

/*! \brief Resets the integrator in the PID regulator.

    Calling this function will reset the integrator in the PID regulator.

    \param pid_st  Pointer to the PID status struct for which the integrator
    will be reset.
*/
void PIDResetIntegrator(pidData_t *pid_st)
{
  pid_st->sumError = 0;
  pid_st->p_term = 0;
  pid_st->i_term = 0;
#if (PID_K_D_ENABLE == TRUE)
  pid_st->d_term = 0;
#endif
}

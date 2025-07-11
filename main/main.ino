/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        Motor control implementation.

   \details
        This file contains the full implementation of the motor control, except
        the PID-controller, filter, fault, SCPI and table implementations and
        definitions.

   \par User Manual:
        ANxxx: Trapezoidal Control of BLDC Motors Using Hall Effect Sensors

   \par Documentation
        For comprehensive code documentation, supported compilers, compiler
        settings and supported devices see readme.html

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2024/03/08 $  \n

 ******************************************************************************/

// Include motor control related headers
#include "config.h"
#include "tables.h"
#include "fault.h"
#include "filter.h"
#include "scpi.h"

// Include PID control algorithm if closed-loop speed control is enabled
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
#include "pid.h"
#endif

/*! \brief Motor control flags placed in I/O space for fast access.

    This variable contains all the flags used for motor control. It is placed in
    GPIOR0 register, which allows usage of several fast bit manipulation/branch
    instructions.
*/
volatile motorflags_t motorFlags FAST_ACCESS(0x3E);

/*! \brief Fault flags placed in I/O space for fast access.

    This variable contains all the flags used for faults. It is placed in GPIOR1
    register, which allows usage of several fast bit manipulation/branch
    instructions.
*/
volatile faultflags_t faultFlags FAST_ACCESS(0x4A);

/*! \brief Motor Configs.

    This variable contains some of the motor configurations.
*/
volatile motorconfigs_t motorConfigs;

/*! \brief The number of 'ticks' between two hall sensor changes (counter).

    This variable is used to count the number of 'ticks' between each hall
    sensor change. It is cleared when the hall sensor change occurs. One 'tick'
    is one PWM period.

  \note The speed of the motor is inversely proportional to this value, but this
  variable is also reset to 0 when a hall sensor change is detected, so cannot
  be used to infer speed of the motor.

  \see lastCommutationTicks

*/
volatile uint16_t commutationTicks = 0;
/*!
  \brief The number of 'ticks' between two hall sensor changes (store).

    This variable is used to store the number of 'ticks' between each hall
    sensor change. It is set to the value in \ref commutationTicks before it is
    cleared when the hall sensor change occurs.

    One 'tick' is one PWM period. The speed of the motor is inversely
    proportional to this value.

  \see commutationTicks
*/
volatile uint16_t lastCommutationTicks = 0xffff;

/*! \brief The most recent "speed" input measurement.

    This variable is set by the ADC from the speed input reference pin. The
    range is 0-255.
*/
volatile uint8_t speedInput = 0;

/*! \brief The most recent "speed" output from the speed controller.

    This variable controls the duty cycle of the generated PWM signals. The
    range is 0-255.
*/
volatile uint8_t speedOutput = 0;

/*!
   \brief Hi-side Current (IBUS) measurement (Register Value).

   The most recent current measurement is stored in this variable.

   The range is 0-1023.

   This value is not scaled and represents the raw ADC register value. To obtain
   the scaled current value in amperes, you can use the formula:

   \f[ \text{Current (A)} = \frac{\text{REGISTER_VALUE} \times 0.004888
     \times 1000000}{\text{IBUS_GAIN} \times \text{IBUS_SENSE_RESISTOR}}
     \f]

   Where:
     - REGISTER_VALUE : The raw ADC register value stored in this variable.
     - 0.004888 : The conversion factor for a 10-bit ADC with a Vref of 5V.
     - \ref IBUS_GAIN : The gain of the current sense operational amplifier.
     - \ref IBUS_SENSE_RESISTOR : The value of the shunt resistor in
       micro-ohms (μΩ).

   The NEVB-MTR1-I56-1 comes with a current op-amp with a gain factor of 50 and
   a current sense resistor of value 4 mΩ. This corresponds to approximately
   0.0244 amperes (A) per register value.

   \see IBUS_WARNING_THRESHOLD, IBUS_ERROR_THRESHOLD
*/
volatile uint16_t ibus = 0;

/*!
   \brief In-line Phase U current current measurement (Register Value).

   The most recent current measurement is stored in this variable.

   The range is 0-1023.

   This value is not scaled and represents the raw ADC register value. To obtain
   the scaled current value in amperes, you can use the formula:

   \f[ \text{Current (A)} = \frac{\text{REGISTER_VALUE} \times 0.004888
     \times 1000000}{\text{IBUS_GAIN} \times \text{IBUS_SENSE_RESISTOR}}
     \f]

   Where:
     - REGISTER_VALUE : The raw ADC register value stored in this variable.
     - 0.004888 : The conversion factor for a 10-bit ADC with a Vref of 5V.
     - \ref IPHASE_GAIN : The gain of the current sense operational amplifier.
     - \ref IPHASE_SENSE_RESISTOR : The value of the shunt resistor in
       micro-ohms (μΩ).

   The NEVB-MTR1-I56-1 comes with a current op-amp with a gain factor of 20 and
   a current sense resistor of value 2.5 mΩ. This corresponds to approximately
   TODO: amperes (A) per register value.

   \note It is not used for any significant purpose in this implementation, but
   the measurement is updated.
*/
volatile int16_t iphaseU = 0;
/*!
   \brief In-line Phase V current current measurement (Register Value).

   The most recent current measurement is stored in this variable.

   The range is 0-1023.

   This value is not scaled and represents the raw ADC register value. To obtain
   the scaled current value in amperes, you can use the formula:

   \f[ \text{Current (A)} = \frac{\text{REGISTER_VALUE} \times 0.004888
     \times 1000000}{\text{IBUS_GAIN} \times \text{IBUS_SENSE_RESISTOR}}
     \f]

   Where:
     - REGISTER_VALUE : The raw ADC register value stored in this variable.
     - 0.004888 : The conversion factor for a 10-bit ADC with a Vref of 5V.
     - \ref IPHASE_GAIN : The gain of the current sense operational amplifier.
     - \ref IPHASE_SENSE_RESISTOR : The value of the shunt resistor in
       micro-ohms (μΩ).

   The NEVB-MTR1-I56-1 comes with a current op-amp with a gain factor of 20 and
   a current sense resistor of value 2.5 mΩ. This corresponds to approximately
   TODO: amperes (A) per register value.

   \note It is not used for any significant purpose in this implementation, but
   the measurement is updated.
*/
volatile int16_t iphaseV = 0;

/*!
   \brief In-line Phase W current current measurement (Register Value).

   The most recent current measurement is stored in this variable.

   The range is 0-1023.

   This value is not scaled and represents the raw ADC register value. To obtain
   the scaled current value in amperes, you can use the formula:

   \f[ \text{Current (A)} = \frac{\text{REGISTER_VALUE} \times 0.004888
     \times 1000000}{\text{IBUS_GAIN} \times \text{IBUS_SENSE_RESISTOR}}
     \f]

   Where:
     - REGISTER_VALUE : The raw ADC register value stored in this variable.
     - 0.004888 : The conversion factor for a 10-bit ADC with a Vref of 5V.
     - \ref IPHASE_GAIN : The gain of the current sense operational amplifier.
     - \ref IPHASE_SENSE_RESISTOR : The value of the shunt resistor in
       micro-ohms (μΩ).

   The NEVB-MTR1-I56-1 comes with a current op-amp with a gain factor of 20 and
   a current sense resistor of value 2.5 mΩ. This corresponds to approximately
   TODO: amperes (A) per register value.

   \note It is not used for any significant purpose in this implementation, but
   the measurement is updated.
*/
volatile int16_t iphaseW = 0;

/*!
  \brief VBUS voltage measurement (Register Value)

  The most recent VBUS voltage measurement is stored in this variable.

  The range is 0-1023.

  This value is not scaled and represents the raw ADC register value. To obtain
  the scaled VBUS voltage in volts, you can use the formula:

  \f[ \text{Voltage (V)} = \frac{\text{REGISTER_VALUE} \times 0.004888 \times
     (\text{VBUS_RTOP} + \text{VBUS_RBOTTOM})}{\text{VBUS_RBOTTOM}} \f]

  Where:
    - REGISTER_VALUE : The raw ADC register value stored in this variable.
    - 0.004888 : The conversion factor for a 10-bit ADC with a Vref of 5V.
    - \ref VBUS_RTOP : The top resistor value in ohms (Ω) in the potential
      divider.
    - \ref VBUS_RBOTTOM : The bottom resistor value in ohms (Ω) in the potential
      divider.

  The NEVB-MTR1-C-1 has a resistor divider with RTOP of 100 kΩ and RBOTTOM of 6.2
  kΩ, so it corresponds to approximately 0.0837 volts (V) per register value.

  \note It is not used for any significant purpose in this implementation, but
  the measurement is updated.
*/
volatile uint16_t vbusVref = 0;

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
//! Struct used to hold PID controller parameters and variables.
pidData_t pidParameters;
#endif

/*! \brief Main initialization function

   The main initialization function initializes all subsystems needed for motor
   control.
*/
void setup(void)
{
  // Load motor configs
  ConfigsInit();

  // Initialize flags.
  FlagsInit();

  // Check if remote mode requested.
  RemoteUpdate();

  // Initialize peripherals.
  PortsInit(); // depends on motorFlags.remote
  ADCInit();   // include self-test + loop until board detected must be before TimersInit
  PLLInit();
  TimersInit();

#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
  PIDInit(PID_K_P, PID_K_I, PID_K_D, &pidParameters);
#endif

  if (motorFlags.remote == TRUE)
  {
    // Start serial interface with 115200 bauds.
    Serial.begin(115200);
    // while (!Serial); // wait for serial to finish initializing
    while (!Serial)
      ; // wait for serial to finish initializing

    // Initialise SCPI subsystem if remote mode.
    ScpiInit();
  }
  else
  {
    // Update direction before enable flag.
    DesiredDirectionUpdate();
    // Do not update enable flag until everything is ready.
    EnableUpdate();
  }

  // Set up pin change interrupts.
  PinChangeIntInit();

  // Enable Timer4 overflow event interrupt.
  TIMSK4 |= (1 << TOIE4);

  // Enable interrupts globally and let motor driver take over.
  sei();
}

/*! \brief Main Loop Function

    The main loop function is executed continuously in normal operation after
    the program has configured everything. It continually checks if the speed
    controller needs to be run.
*/
void loop()
{
  if (motorFlags.remote == TRUE)
  {
    ScpiInput(Serial);
  }
  if (motorFlags.speedControllerRun)
  {
    SpeedController();
    motorFlags.speedControllerRun = FALSE;
  }
}

/*! \brief Initializes motorFlags and faultFlags

    This function initializes both motorFlags and faultFlags to their default
    values.
*/
static void FlagsInit(void)
{
  // Initialize motorFlags with default values.
  motorFlags.speedControllerRun = FALSE;
  motorFlags.remote = FALSE;
  motorFlags.enable = FALSE;
  motorFlags.actualDirection = DIRECTION_UNKNOWN;
  motorFlags.desiredDirection = DIRECTION_FORWARD;
  motorFlags.driveWaveform = WAVEFORM_UNDEFINED;

  // Initialize faultFlags with default values. Set motorStopped to FALSE at
  // startup. This will make sure that the motor is not started if it is not
  // really stopped. If it is stopped, this variable will quickly be updated.
  faultFlags.motorStopped = FALSE;
  faultFlags.reverseDirection = FALSE;
  faultFlags.overCurrent = FALSE;
  faultFlags.noHallConnections = FALSE;
  faultFlags.userFlag1 = FALSE;
  faultFlags.userFlag2 = FALSE;
  faultFlags.userFlag3 = FALSE;
}

/*! \brief Initializes motorConfigs

    This function initializes motorConfigs to their default values.
*/
static void ConfigsInit(void)
{
  motorConfigs.tim4Freq = (uint32_t)TIM4_FREQ;
  motorConfigs.tim4Top = (uint16_t)TIM4_TOP(motorConfigs.tim4Freq);
  motorConfigs.tim4DeadTime = (uint16_t)DEAD_TIME;
  motorConfigs.speedInputSource = (uint8_t)SPEED_INPUT_SOURCE_LOCAL;
}

/*!
   \brief Initialize PLL (Phase-Locked Loop)

   This function configures and initializes the PLL for clock generation. It
   sets the PLL frequency control register and waits until the PLL is locked and
   stable before returning.
*/
static void PLLInit(void)
{
  // Configure PLL Frequency Control Register to output 48MHz for USB Module and
  // 64MHz for the High-Speed Clock Timer with a base speed of 96MHz for PLL
  // Output Frequency.

  PLLFRQ = (0 << PINMUX) | (1 << PLLUSB) | PLL_POSTSCALER_DIV_1_5 | (1 << PDIV3) | (0 << PDIV2) | (1 << PDIV1) | (0 << PDIV0);

  // Enable PLL.
  PLLCSR = (1 << PINDIV) | (1 << PLLE);

  // Wait until PLOCK bit is set, indicating PLL is locked and stable.
  while ((PLLCSR & (1 << PLOCK)) == 0)
  {
    // Wait for PLL lock
    ;
  }
}

/*!
   \brief Initialize I/O port directions and pull-up resistors

   This function initializes all I/O ports with the correct directions and
   enables pull-up resistors, if needed, for various pins and signals used in
   the motor control.
*/
static void PortsInit(void)
{
#if (EMULATE_HALL == TRUE)
  // Configure and set hall sensor pins for motor emulation
  PORTB &= ~((1 << H1_PIN) | (1 << H2_PIN) | (1 << H3_PIN));
  PORTB |= (0x07 & pgm_read_byte_near(&expectedHallSequenceForward[1]));
  // Set hall sensor pins as outputs.
  DDRB |= (1 << H1_PIN) | (1 << H2_PIN) | (1 << H3_PIN);
#endif

#if ((HALL_PULLUP_ENABLE == TRUE) && (EMULATE_HALL != TRUE))
  // Configure and set hall sensor pins as input with pull-ups enabled
  PORTB |= (1 << H1_PIN) | (1 << H2_PIN) | (1 << H3_PIN);
#endif

  // Configure and set pins FAULT_PIN_3, FAULT_PIN_2, and FAULT_PIN_1 as outputs
  PORTB &= ~((1 << FAULT_PIN_3) | (1 << FAULT_PIN_2));
  DDRB |= (1 << FAULT_PIN_3) | (1 << FAULT_PIN_2);
  PORTD &= ~(1 << FAULT_PIN_1);
  DDRD |= (1 << FAULT_PIN_1);

  // If remote mode, set enable and direction pin as output to allow software
  // triggered interrupts
  if (motorFlags.remote == TRUE)
  {
    PORTD &= ~((1 << ENABLE_PIN) | (1 << DIRECTION_COMMAND_PIN));
    DDRD |= (1 << ENABLE_PIN) | (1 << DIRECTION_COMMAND_PIN);
  }
}

/*! \brief Initializes and synchronizes Timers

    This function sets the correct pre-scaler and starts all required timers.

    Timer 1 is used to trigger the fault multiplexing on overflow. Timer 3 is
    used, if \ref EMULATE_HALL is set, to trigger the change in hall output on
    overflow. Timer 4 is used to generate PWM outputs for the gates and trigger
    the commutation tick counter and check if the motor is spinning on overflow.
    The overflow event interrupt for Timer 4 is set outside this function at the
    end of the main initialization function.

    Timer 0 is used by the Ardiono core to generate interrupts. For reference,
    it is set up in mode 3 (Fast PWM, TOP=0xFF) with a prescaler of 64. This means
    ovwerflow occurs ~977 times per second. The overflow interrupt is used to
    trigger the ADC conversion unless changed by the user.

    \see EMULATE_HALL, TIM3_FREQ, TimersSetModeBlockCommutation(),
    TimersSetModeBrake()
*/
void TimersInit(void)
{
  // Set Timer1 accordingly.
  TCCR1A = (1 << WGM11) | (0 << WGM10);
  TCCR1B = (0 << WGM13) | (1 << WGM12);
  TIMSK1 = (1 << TOIE1);

  // Start Timer1.
  TCCR1B |= TIM1_CLOCK_DIV_64;

#if (EMULATE_HALL == TRUE)
  // Set Timer3 accordingly.
  TCCR3A = (1 << WGM31) | (1 << WGM30);
  TCCR3B = (1 << WGM33) | (1 << WGM32);
  TIMSK3 = (1 << TOIE3);

  // Set top value of Timer/counter3.
  OCR3AH = (uint8_t)(TIM3_TOP >> 8);
  OCR3AL = (uint8_t)(0xff & TIM3_TOP);

  // Start Timer3.
  TCCR3B |= TIM1_CLOCK_DIV_8;
#endif
  // Set Timer4 in "PWM6 / Dual-slope" mode. Does not enable outputs yet.
  TCCR4A = (0 << COM4A1) | (1 << COM4A0) | (0 << COM4B1) | (1 << COM4B0) | (1 << PWM4A) | (1 << PWM4B);
  TCCR4B = DT_PRESCALER_DIV_PATTERN(CHOOSE_DT_PRESCALER(motorConfigs.tim4DeadTime));
  TCCR4C |= (0 << COM4D1) | (1 << COM4D0) | (1 << PWM4D);
  TCCR4E = (1 << ENHC4);

  // Set top value of Timer/counter4.
  TC4H = (uint8_t)(motorConfigs.tim4Top >> 8);
  OCR4C = (uint8_t)(0xff & motorConfigs.tim4Top);

  // Set the dead time.
  DT4 = (DEAD_TIME_HALF(motorConfigs.tim4DeadTime) << 4) | DEAD_TIME_HALF(motorConfigs.tim4DeadTime);

  // Start Timer4.
  TCCR4B |= TIM4_PRESCALER_DIV_PATTERN(CHOOSE_TIM4_PRESCALER(TIM4_FREQ));
}

/*! \brief Initialize pin change interrupts.

    This function initializes pin change interrupt on hall sensor input pins
    input, shutdown input and motor direction control input.
*/
static void PinChangeIntInit(void)
{
  // Initialize external interrupt on shutdown pin (INT0) and direction input
  // (INT2) pin.
  EICRA = (0 << ISC21) | (1 << ISC20) | (0 << ISC01) | (1 << ISC00);
  EIMSK = (1 << INT2) | (1 << INT0);

  // Initialize pin change interrupt on hall sensor inputs (PCINT1..3).
  PCMSK0 = (1 << PCINT3) | (1 << PCINT2) | (1 << PCINT1);

  // Enable pin change interrupt on ports with pin change signals
  PCICR = (1 << PCIE0);
}

/*! \brief Initializes the ADC

    This function initializes the ADC for speed reference, current  and gate
    voltage measurements.

    The function performs self-tests to ensure the ADC's accuracy:
    - It measures 0V to verify the ADC's ability to read close to zero.
    - It measures 1.1V to ensure the ADC's accuracy within a specified range.
    - It measures BREF and waits for any board to be connected.

    If any of the self-tests fail, a fatal error is indicated.

    After self-testing, the ADC is configured for speed reference measurements
    and set to trigger on \ref ADC_TRIGGER.

    \note \ref ADC_TRIGGER is set to \ref ADC_TRIGGER_TIMER0_OVF. This triggers
    the ADC at a rate of ~977 samples per second. The ADC prescaler is set to
    \ref ADC_PRESCALER_DIV_128 as this gives a 125kHz ADC clock. It is recommended
    to have the ADC clock between 50kHz and 200kHz to get maximum ADC resolution.

    \see ADC_TRIGGER
*/
static void ADCInit(void)
{
  // Select initial AD conversion channel [0V for self-test].
  ADMUX = (ADC_REFERENCE_VOLTAGE | (1 << ADLAR) | ADC_MUX_L_0V);
  ADCSRB = ADC_MUX_H_0V;
  _delay_ms(1);

  // Enable ADC
  ADCSRA = (1 << ADEN);

  // Start ADC single conversion and discard first measurement.
  uint16_t adc_reading = ADCSingleConversion();

  // Start ADC single conversion to measure 0V, this time it is correct.
  adc_reading = ADCSingleConversion();

  // Check if ADC can measure 0V within 10mV.
  if (adc_reading > 2)
  {
    FatalError((uint8_t)0b0000111);
  }

  // Select next AD conversion channel [1V1 for self-test].
  ADMUX = (ADC_REFERENCE_VOLTAGE | (1 << ADLAR) | ADC_MUX_L_1V1);
  ADCSRB = ADC_MUX_H_1V1;
  _delay_ms(1);

  // Start ADC single conversion to measure 1V1.
  adc_reading = ADCSingleConversion();

  // Check if ADC can measure 1.1V within 1% (1.09 to 1.1V).
  if ((adc_reading < 223) || (adc_reading > 227))
  {
    FatalError((uint8_t)0b0000111);
  }

  // Select next AD conversion channel [BREF].
  ADMUX = (ADC_REFERENCE_VOLTAGE | (1 << ADLAR) | ADC_MUX_L_IBUS);
  ADCSRB = ADC_MUX_H_IBUS;

  // Enable pull up resistor
  PORTF |= (1 << PF0);

  _delay_ms(1);
  SweepLEDsBlocking();

  // Start ADC single conversion to measure BREF.
  adc_reading = ADCSingleConversion();

  // Wait to check if any board is connected. Should be anything other than
  // 0x3ff if any board is connected assuming BREF of the board is not equal to
  // IOREF.
  while (adc_reading == 0x3ff)
  {
    SweepLEDsBlocking();

    // Start ADC single conversion to measure BREF.
    adc_reading = ADCSingleConversion();
  }

  // Disable pull up resistor
  PORTF &= ~(1 << PF0);

  // Re-initialize ADC mux channel select.
  ADMUX &= ~ADC_MUX_L_BITS;
  ADMUX |= ADC_MUX_L_SPEED;
  // Set trigger source to ADC_TRIGGER.
  ADCSRB = ADC_MUX_H_SPEED | ADC_TRIGGER;

  // Re-initialize ADC to work with interrupts.
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIF) | (1 << ADIE) | ADC_PRESCALER;
}

/*! \brief Perform a single ADC conversion

    This function initiates a single ADC conversion to measure a voltage. It
    waits for the conversion to complete and then reads the ADC result.

    \return The 16-bit ADC result representing the measured voltage.
*/
static uint16_t ADCSingleConversion(void)
{
  // Initialize ADC for one-time conversion.
  ADCSRA |= (1 << ADSC);

  // Wait for the conversion to complete.
  while (ADCSRA & (1 << ADSC))
  {
    // Wait until the conversion is finished.
  }

  // Read the ADC result and combine ADCH and ADCL into a 16-bit value.
  uint16_t value = (ADCL >> 6);
  value |= 0x3ff & (ADCH << 2);

  return value;
}

/*! \brief Check whether the enable pin is set and update flags accordingly.

    This function checks the state of the enable pin and updates the motorFlags
    and faultFlags accordingly. If the enable pin is not set, it can perform
    specific actions based on the \ref TURN_OFF_MODE configuration:
    - If \ref TURN_OFF_MODE is set to \ref TURN_OFF_MODE_COAST, it disables
      driver signals, allowing the motor to coast.
    - If \ref TURN_OFF_MODE is set to \ref TURN_OFF_MODE_BRAKE, it sets the
      motor in brake mode.

    \note The behavior of this function depends on the \ref TURN_OFF_MODE
    configuration.

    \see TURN_OFF_MODE, TimersSetModeBrake()
*/
static void EnableUpdate(void)
{
  if ((PIND & (1 << ENABLE_PIN)) != 0)
  {
    motorFlags.enable = TRUE;
  }
  else
  {
    motorFlags.enable = FALSE;

#if (TURN_OFF_MODE == TURN_OFF_MODE_COAST)
    // Disable driver signals to let the motor coast.
    motorFlags.driveWaveform = WAVEFORM_UNDEFINED;
    DisablePWMOutputs();
    ClearPWMPorts();
    faultFlags.motorStopped = FALSE;
#endif

#if (TURN_OFF_MODE == TURN_OFF_MODE_BRAKE)
    // Set the motor in brake mode.
    faultFlags.motorStopped = FALSE;
    TimersSetModeBrake();
#endif
  }
}

/*! \brief Check whether the remote pin is set and update flags accordingly.

    This function checks the state of the remote pin and updates the motorFlags.

    \note This is only called during setup so any further pin changes are not
    detected.
*/
static void RemoteUpdate(void)
{
  if ((PIND & (1 << REMOTE_PIN)) != 0)
  {
    motorFlags.remote = TRUE;
  }
  else
  {
    motorFlags.remote = FALSE;
  }
}

/*! \brief Speed regulator loop.

    This function is called periodically every \ref SPEED_CONTROLLER_TIME_BASE
    ticks. In this implementation, a simple PID controller loop is called, but
    this function could be replaced by any speed or other regulator.

    If the \ref SPEED_CONTROL_METHOD is set to \ref SPEED_CONTROL_CLOSED_LOOP, a
    PID controller is used to regulate the speed. The speed input is converted
    into an increment set point, and a PID controller computes the output value.
    The output is limited to a maximum value of 255.

    If the \ref SPEED_CONTROL_METHOD is not set to \ref
    SPEED_CONTROL_CLOSED_LOOP, a simple speed control mechanism is applied. If
    the motor is enabled, the function calculates the delta between the speed
    input and the current speed output. If the delta exceeds the maximum allowed
    change, it limits the change to \ref SPEED_CONTROLLER_MAX_DELTA. If the
    motor is disabled, the speed output is set to 0.

    \note The behavior of this function depends on the \ref SPEED_CONTROL_METHOD
    configuration.

    \see SPEED_CONTROL_METHOD, SPEED_CONTROLLER_TIME_BASE,
        SPEED_CONTROLLER_MAX_DELTA, SPEED_CONTROLLER_MAX_SPEED, PID_K_P,
        PID_K_I, PID_K_D_ENABLE, PID_K_D
*/
static void SpeedController(void)
{
  if (motorFlags.enable == TRUE)
  {
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
    // Calculate an increment set point from the analog speed input.
    int16_t incrementSetpoint = ((int32_t)speedInput * SPEED_CONTROLLER_MAX_SPEED) / SPEED_CONTROLLER_MAX_INPUT;

    // PID regulator with feed forward from speed input.
    uint16_t outputValue;

    outputValue = PIDController(incrementSetpoint, (motorConfigs.tim4Freq / (lastCommutationTicks * 3)) >> 1, &pidParameters);

    if (outputValue > 255)
    {
      outputValue = 255;
    }

    speedOutput = outputValue;

    // Without the delay PID does not reset when needed
    _delay_us(1);
#else
    // Calculate the delta in speedInput
    int16_t delta = speedInput - speedOutput;
    // If delta exceeds the maximum allowed change, limit it and update
    // speedOutput
    if (delta > SPEED_CONTROLLER_MAX_DELTA)
    {
      speedOutput += SPEED_CONTROLLER_MAX_DELTA;
    }
    else if (delta < -SPEED_CONTROLLER_MAX_DELTA)
    {
      speedOutput -= SPEED_CONTROLLER_MAX_DELTA;
    }
    else
    {
      speedOutput = speedInput;
    }
#endif
  }
  else
  {
    speedOutput = 0;
  }
}

/**
   \brief Handle a fatal error and enter a fault state.

   This function is called in response to a fatal error condition. It performs
   the following actions:
   - Detaches all interrupts to prevent further operation.
   - Disables PWM outputs.
   - Sets fault flags based on the provided error code.
   - Enters a loop, continuously executing the fault sequential state machine
     and delaying for 2 milliseconds between each iteration.

   \param code The error code indicating the nature of the fatal error. Each bit
               of the code corresponds to a specific fault condition: - Bit 0:
               User Flag 3 - Bit 1: User Flag 2 - Bit 2: User Flag 1 - Bit 3:
               Reverse Direction - Bit 4: Motor Stopped - Bit 5: Over Current -
               Bit 6: No Hall Connections
*/
static void FatalError(uint8_t code)
{
  // Detach all interrupts
  cli();

  // Disable outputs
  DisablePWMOutputs();

  // Set faultFlags based on the provided error code
  faultFlags.userFlag3 = (code & 0x01) != 0;
  faultFlags.userFlag2 = (code & 0x02) != 0;
  faultFlags.userFlag1 = (code & 0x04) != 0;
  faultFlags.reverseDirection = (code & 0x08) != 0;
  faultFlags.motorStopped = (code & 0x10) != 0;
  faultFlags.overCurrent = (code & 0x20) != 0;
  faultFlags.noHallConnections = (code & 0x40) != 0;

  // loop forever
  while (1)
  {
    faultSequentialStateMachine(&faultFlags, &motorFlags);
    _delay_ms(2);
  }
}

/*! \brief Set duty cycle for TIM4.

    This function sets the duty cycle for block commutation, where the duty
    range is 0-1023 (not in percentage). To convert to percentage, use: duty *
    100 / 1023.

    \param duty New duty cycle, range 0-1023.
*/
static FORCE_INLINE void SetDuty(const uint16_t duty)
{
  TC4H = duty >> 8;
  OCR4A = 0xFF & duty;
}

/*! \brief Configures timers for block commutation.

    This function is called every time the drive waveform is changed and block
    commutation is needed. PWM outputs are safely disabled while configuration
    registers are changed to avoid unintended driving or shoot-through.

    The function sets the PWM pins to input (High-Z) while changing modes and
    configures the timers. The output duty cycle is set to zero initially. It
    waits for the next PWM cycle to ensure that all outputs are updated, and
    then the motor drive waveform is set to block commutation. Finally, the PWM
    pins are changed back to output mode to allow PWM control.
*/
static FORCE_INLINE void TimersSetModeBlockCommutation(void)
{
  // Set PWM pins to input (High-Z) while changing modes.
  DisablePWMOutputs();

  // Sets up timers.
  TCCR4A = (0 << COM4A1) | (1 << COM4A0) | (0 << COM4B1) | (1 << COM4B0) | (1 << PWM4A) | (1 << PWM4B);
  TCCR4C |= (0 << COM4D1) | (1 << COM4D0) | (1 << PWM4D);
  TCCR4D = (1 << WGM41) | (1 << WGM40);

  // Set output duty cycle to zero for now.
  SetDuty(0);

  // Wait for the next PWM cycle to ensure that all outputs are updated.
  TimersWaitForNextPWMCycle();

  motorFlags.driveWaveform = WAVEFORM_BLOCK_COMMUTATION;

  // Change PWM pins to output again to allow PWM control.
  EnablePWMOutputs();
}

#if (TURN_OFF_MODE == TURN_OFF_MODE_BRAKE)
/*! \brief Configures timers for braking and starts braking.

    This function configures the timers for braking and starts braking. Please
    note that braking when turning can produce too much heat for the MOSFETs to
    handle. Use with care!

    When called, this function sets the PWM pins to input (High-Z) while
    changing modes. It then configures the timer and sets a 50% duty cycle for
    braking. Both the high side and the low side gates are set to output the PWM
    signal. It waits for the next PWM cycle to ensure that all outputs are
    updated, sets the motor drive waveform to braking, and changes the PWM pins
    back to output mode to allow PWM control.

    \note This function is conditional and depends on the \ref TURN_OFF_MODE
    configuration.

    \see TURN_OFF_MODE
*/
static FORCE_INLINE void TimersSetModeBrake(void)
{
  // Set PWM pins to input (High-Z) while changing modes.
  DisablePWMOutputs();

  // Sets up timers.
  TCCR4A = (0 << COM4A1) | (1 << COM4A0) | (0 << COM4B1) | (1 << COM4B0) | (1 << PWM4A) | (1 << PWM4B);
  TCCR4C |= (0 << COM4D1) | (1 << COM4D0) | (1 << PWM4D);
  TCCR4D = (1 << WGM41) | (1 << WGM40);

  // Set to 50% duty
  SetDuty((uint16_t)511);

  // PWM outputs on both high side and low side gates are turned enabled
  // (complementary)
  TCCR4E &= ~0b00111111;
  TCCR4E |= OC_ENABLE_PORTB | OC_ENABLE_PORTC | OC_ENABLE_PORTD;

  // Wait for the next PWM cycle to ensure that all outputs are updated.
  TimersWaitForNextPWMCycle();

  motorFlags.driveWaveform = WAVEFORM_BRAKING;

  EnablePWMOutputs();
}
#endif

/*! \brief Wait for the start of the next PWM cycle.

    This function waits for the beginning of the next PWM cycle to ensure smooth
    transitions between different PWM modes and avoid shoot-through conditions.
*/
static FORCE_INLINE void TimersWaitForNextPWMCycle(void)
{
  // Clear Timer1 Capture event flag.
  TIFR4 = (1 << TOV4);

  // Wait for new Timer1 Capture event flag.
  while (!(TIFR4 & (1 << TOV4)))
  {
  }
}

/*! \brief Retrieve the intended motor direction.

    This function provides the current intended direction for the motor.

    \note The direction input is not directly read at this point. Instead, a
    separate pin change interrupt is responsible for monitoring the input.

    \retval DIRECTION_FORWARD Forward direction is the intended direction.
    \retval DIRECTION_REVERSE Reverse direction is the intended direction.
*/
static FORCE_INLINE uint8_t GetDesiredDirection(void)
{
  return (uint8_t)motorFlags.desiredDirection;
}

/*! \brief Retrieve the current motor direction.

    This function returns the current operating direction of the motor.

    \note To accurately determine the direction, two consecutive hall sensor
    changes in the same direction are required.

    \return The current motor direction.

    \retval DIRECTION_FORWARD Motor is currently running forward. \retval
    DIRECTION_REVERSE Motor is currently running in reverse. \retval
    DIRECTION_UNKNOWN The current motor direction cannot be determined, which
    may indicate the motor is stopped, changing direction, or that the hall
    sensors are providing incorrect information.
*/
static FORCE_INLINE uint8_t GetActualDirection(void)
{
  return motorFlags.actualDirection;
}

/*! \brief Perform block commutation based on direction and hall sensor input

    This function performs block commutation according to the specified
    direction of rotation and the hall sensor input. Block commutation is used
    to control motor phases during operation.

    \param direction Direction of rotation (\ref DIRECTION_FORWARD or \ref
    DIRECTION_REVERSE). \param hall Hall sensor input value corresponding to the
    rotor position.
*/
static FORCE_INLINE void BlockCommutate(const uint8_t direction, uint8_t hall)
{
  const uint8_t *tableAddress;

  if (direction == DIRECTION_FORWARD)
  {
    tableAddress = blockCommutationTableForward;
  }
  else
  {
    tableAddress = blockCommutationTableReverse;
  }
  tableAddress += (hall * 4);

  ClearPWMPorts();
  TCCR4E &= ~0b00111111;

  DisablePWMOutputs();

  PORTB |= (uint8_t)pgm_read_byte_near(tableAddress++);
  PORTC |= (uint8_t)pgm_read_byte_near(tableAddress++);
  PORTD |= (uint8_t)pgm_read_byte_near(tableAddress++);
  TCCR4E |= (uint8_t)pgm_read_byte_near(tableAddress);

  EnablePWMOutputs();
}

/*! \brief Read the hall sensor inputs and decode the hall state.

    This function reads the hall sensor inputs and converts them into a number
    ranging from 1 to 6, where the hall sensors' states are represented as bits:
    H3|H2|H1.

    \return The decoded hall sensor value.

    \retval 0 Illegal hall state. Possible hardware error. \retval 1-6 Valid
    hall sensor values. \retval 7 Illegal hall state. Possible hardware error.
*/
static FORCE_INLINE uint8_t GetHall(void)
{
  uint8_t hall;

  hall = HALL_PIN & ((1 << H3_PIN) | (1 << H2_PIN) | (1 << H1_PIN));
  hall >>= H1_PIN;

  return hall;
}

/*! \brief Updates global desired direction flag.

    Running this function triggers a reading of the direction input pin. The
    desiredDirection flag is set accordingly.
*/
static FORCE_INLINE void DesiredDirectionUpdate(void)
{
  if ((PIND & (1 << DIRECTION_COMMAND_PIN)) != 0)
  {
    motorFlags.desiredDirection = DIRECTION_REVERSE;
  }
  else
  {
    motorFlags.desiredDirection = DIRECTION_FORWARD;
  }
}

/*! \brief Update the global actual direction flag based on the two latest hall
    values.

    Calling this function with the last two hall sensor values as parameters
    triggers an update of the global actualDirection flag.

    \param lastHall The previous hall sensor value. \param newHall The current
    hall sensor value.
*/
static FORCE_INLINE void ActualDirectionUpdate(uint8_t lastHall, const uint8_t newHall)
{
  // Ensure that lastHall is within the bounds of the table. If not, set it to
  // 0, which is also an illegal hall value but a legal table index.
  if (lastHall > 6)
  {
    lastHall = 0;
  }

  if (pgm_read_byte_near(&expectedHallSequenceForward[lastHall]) == newHall)
  {
    motorFlags.actualDirection = DIRECTION_FORWARD;
  }
  else if (pgm_read_byte_near(&expectedHallSequenceReverse[lastHall]) == newHall)
  {
    motorFlags.actualDirection = DIRECTION_REVERSE;
  }
  else
  {
    motorFlags.actualDirection = DIRECTION_UNKNOWN;
  }
}

/*! \brief Update the reverse rotation flag.

    This function compares the actual and desired direction flags to determine
    if the motor is running in the opposite direction of what is requested.
*/
static FORCE_INLINE void ReverseRotationSignalUpdate(void)
{
  if (GetActualDirection() == GetDesiredDirection())
  {
    faultFlags.reverseDirection = FALSE;
  }
  else
  {
    faultFlags.reverseDirection = TRUE;
  }
}

/*! \brief Enable PWM output pins.

    This function sets the port direction for all PWM pins as output, allowing
    PWM signals to be generated on these pins. It does not modify the PWM
    configuration itself.
*/
static FORCE_INLINE void EnablePWMOutputs(void)
{
  DDRB |= PWM_PATTERN_PORTB;
  DDRC |= PWM_PATTERN_PORTC;
  DDRD |= PWM_PATTERN_PORTD;
}

/*! \brief Disable PWM output pins.

    This function disables PWM outputs by setting the port direction for all PWM
    pins as inputs. This action overrides the PWM signals, effectively stopping
    the generation of PWM. The PWM configuration itself remains unchanged.
*/
static FORCE_INLINE void DisablePWMOutputs(void)
{
  DDRB &= ~PWM_PATTERN_PORTB;
  DDRC &= ~PWM_PATTERN_PORTC;
  DDRD &= ~PWM_PATTERN_PORTD;
}

/*! \brief Clear PWM output ports.

    This function clears the PWM output ports by setting the corresponding bits
    of the port registers to low. It effectively turns off the PWM signals on
    the specified ports while leaving the port directions unchanged.
*/
static FORCE_INLINE void ClearPWMPorts(void)
{
  PORTB &= ~PWM_PATTERN_PORTB;
  PORTC &= ~PWM_PATTERN_PORTC;
  PORTD &= ~PWM_PATTERN_PORTD;
}

/*! \brief Update the 'tick' counter and check for a stopped motor.

    This function should be called at every PWM timer overflow to update the
    'tick' counter. It increments the 'tick' counter until it reaches the
    maximum tick limit, indicating a potentially stopped or stalled motor. If
    the limit is reached, the global motor stopped flag is set.

    \note This function can be used to detect a stalled motor and take
    appropriate actions.

    \note If the motor is determined to be stopped, it may change the motor
    drive waveform and disable PWM outputs as necessary.

    \see COMMUTATION_TICKS_STOPPED, TURN_OFF_MODE
*/
static FORCE_INLINE void CommutationTicksUpdate(void)
{
  if (commutationTicks < COMMUTATION_TICKS_STOPPED)
  {
    commutationTicks++;
  }
  else
  {
    faultFlags.motorStopped = TRUE;
    lastCommutationTicks = 0xffff;
    uint8_t hall = GetHall();
    if ((hall == 0) || (hall == 0b111))
    {
      faultFlags.noHallConnections = TRUE;
    }
    if (motorFlags.driveWaveform != WAVEFORM_BLOCK_COMMUTATION && motorFlags.enable == TRUE)
    {
      speedOutput = 0;
#if (SPEED_CONTROL_METHOD == SPEED_CONTROL_CLOSED_LOOP)
      PIDResetIntegrator(&pidParameters);
#endif
      TimersSetModeBlockCommutation();
      BlockCommutate(GetDesiredDirection(), GetHall());
    }
#if (TURN_OFF_MODE == TURN_OFF_MODE_BRAKE)
    else if (motorFlags.enable == FALSE)
    {
      motorFlags.driveWaveform = WAVEFORM_UNDEFINED;
      DisablePWMOutputs();
      ClearPWMPorts();
    }
#endif
  }
}

/*! \brief Enable interrupt service routine.

    This interrupt service routine is called if the enable pin changes state.
*/
ISR(INT0_vect)
{
  EnableUpdate();
}

/*! \brief Hall sensor change interrupt service routine.

    This interrupt service routine is called every time any of the hall sensors
    change. The actual direction, the reverse rotation and no hall connections
    flags are updated.

    The motor stopped flag is also set to FALSE, since the motor is obviously
    not stopped when there is a hall change.
*/
ISR(PCINT0_vect)
{
  static uint8_t lastHall = 0xff;
  uint8_t hall;

  hall = GetHall();

  if (motorFlags.driveWaveform == WAVEFORM_BLOCK_COMMUTATION)
  {
    BlockCommutate(GetDesiredDirection(), hall);
  }

  // Update flags that depend on hall sensor value.
  ActualDirectionUpdate(lastHall, hall);
  ReverseRotationSignalUpdate();

  lastHall = hall;

  // Reset commutation timer.
  lastCommutationTicks = calculateEMA(commutationTicks, lastCommutationTicks, 2);
  commutationTicks = 0;

  // Since the hall sensors are changing, the motor can not be stopped.
  faultFlags.motorStopped = FALSE;
  faultFlags.noHallConnections = FALSE;
}

/*! \brief Direction input change interrupt service routine.

    This interrupt service routine is called every time the direction input pin
    changes state. The desired direction flag is updated accordingly. The motor
    control then goes into a state where it needs a stopped motor before any
    driving of the motor is performed.

    \note Depending on the \ref TURN_OFF_MODE configuration, it will either
    coast or brake.

    \see TURN_OFF_MODE
*/
ISR(INT2_vect)
{
  // Update desired direction flag.
  DesiredDirectionUpdate();
  if (motorFlags.desiredDirection == DIRECTION_FORWARD)
  {
  }
  else if (motorFlags.desiredDirection == DIRECTION_REVERSE)
  {
  };

#if (TURN_OFF_MODE == TURN_OFF_MODE_COAST)
  // Disable driver signals to let motor coast. The motor will automatically
  // start once it stopped.
  motorFlags.driveWaveform = WAVEFORM_UNDEFINED;
  DisablePWMOutputs();
  ClearPWMPorts();
  faultFlags.motorStopped = FALSE;
#endif

#if (TURN_OFF_MODE == TURN_OFF_MODE_BRAKE)
  // Set motor in brake mode. The motor will automatically start once it is
  // stopped.
  faultFlags.motorStopped = FALSE;
  TimersSetModeBrake(); // Automatically sets driveWaveform.
#endif
}

/*! \brief Timer4 Overflow Event Interrupt Service Routine.

   This interrupt service routine is trigger on Timer4 overflow. It manages the
   commutation ticks, which determines motor status. It also controls the
   execution of the speed regulation loop at constant intervals.

   \see TimersInit(), TIM4_FREQ
*/
ISR(TIMER4_OVF_vect)
{
  if (motorFlags.driveWaveform == WAVEFORM_BLOCK_COMMUTATION)
  {
    uint16_t dutyCycle = ((uint32_t)speedOutput * motorConfigs.tim4Top) >> 7;

    if (dutyCycle > (uint16_t)(motorConfigs.tim4Top << 1))
    {
      dutyCycle = (uint16_t)(motorConfigs.tim4Top << 1);
    }

    SetDuty(dutyCycle);
  }

  CommutationTicksUpdate();

  {
    // Run the speed regulation loop with constant intervals.
    static uint8_t speedRegTicks = 0;
    speedRegTicks++;
    if (speedRegTicks >= SPEED_CONTROLLER_TIME_BASE)
    {
      motorFlags.speedControllerRun = TRUE;
      speedRegTicks -= SPEED_CONTROLLER_TIME_BASE;
    }
  }
}

#if (EMULATE_HALL == TRUE)
/*!
   \brief Timer3 Overflow Interrupt Service Routine.

   This interrupt service routine is triggered when Timer3 overflows and \ref
   EMULATE_HALL is set \ref TRUE. It is responsible for emulating hall sensor
   behavior as if a the motor is running.

   The emulated hall sequence is determined by the desired direction and \ref
   expectedHallSequenceForward or \ref expectedHallSequenceReverse arrays
   respectively, which are indexed based on the actual hall sensor reading.

   \note This ISR is only active when hall emulation is enabled.

   \see EMULATE_HALL, TimersInit()
*/
ISR(TIMER3_OVF_vect)
{
  if (motorFlags.enable == TRUE)
  {
    uint8_t hall = GetHall();

    if (GetDesiredDirection() == DIRECTION_FORWARD)
    {
      PORTB = (PORTB & ~((1 << H1_PIN) | (1 << H2_PIN) | (1 << H3_PIN))) | ((0x07 & pgm_read_byte_near(&expectedHallSequenceForward[hall])) << H1_PIN);
    }
    else
    {
      PORTB = (PORTB & ~((1 << H1_PIN) | (1 << H2_PIN) | (1 << H3_PIN))) | ((0x07 & pgm_read_byte_near(&expectedHallSequenceReverse[hall])) << H1_PIN);
    }
  }
}
#endif

/*!
   \brief Timer1 Overflow Interrupt Service Routine

   This interrupt service routine (ISR) is triggered on Timer1 overflow. It
   calls the \ref faultSequentialStateMachine() function to handle motor fault
   reporting.

   \see faultSequentialStateMachine()
*/
ISR(TIMER1_OVF_vect)
{
  faultSequentialStateMachine(&faultFlags, &motorFlags);
}

/**
   \brief ADC Conversion Complete Interrupt Service Routine.

   This interrupt service routine is automatically executed every time an AD
   conversion is finished, and the converted result is available in the ADC data
   register.

   The switch/case construct ensures that the converted value is stored in the
   variable corresponding to the selected channel and changes the channel for
   the next ADC measurement.

   Additional ADC measurements can be added to the cycle by extending the
   switch/case construct.
*/
ISR(ADC_vect)
{
  switch ((ADMUX & ADC_MUX_L_BITS) | (ADCSRB & ADC_MUX_H_BITS))
  {
  case (ADC_MUX_H_SPEED | ADC_MUX_L_SPEED):
    // Handle ADC conversion result for speed measurement.
    if (motorConfigs.speedInputSource == SPEED_INPUT_SOURCE_LOCAL)
    {
      speedInput = ADCH;
    }
    ADMUX &= ~ADC_MUX_L_BITS;
    ADMUX |= ADC_MUX_L_IBUS;
    ADCSRB &= ~ADC_MUX_H_BITS;
    ADCSRB |= ADC_MUX_H_IBUS;
    break;
  case (ADC_MUX_H_IBUS | ADC_MUX_L_IBUS):
    // Handle ADC conversion result for current measurement.
    ibus = ADCL >> 6;
    ibus |= (ADCH << 2);
    ADMUX &= ~ADC_MUX_L_BITS;
    ADMUX |= ADC_MUX_L_IPHASE_U;
    ADCSRB &= ~ADC_MUX_H_BITS;
    ADCSRB |= ADC_MUX_H_IPHASE_U;

    // Check for over current conditions and set fault flags.
    if (ibus > IBUS_ERROR_THRESHOLD)
    {
      FatalError((uint8_t)0b0100111);
    }
    else if (ibus > IBUS_WARNING_THRESHOLD)
    {
      faultFlags.overCurrent = TRUE;
    }
    else
    {
      faultFlags.overCurrent = FALSE;
    }
    break;
  case (ADC_MUX_H_IPHASE_U | ADC_MUX_L_IPHASE_U):
    // Handle ADC conversion result for phase current measurement.
    iphaseU = ADCL >> 6;
    iphaseU |= (ADCH << 2);
    ADMUX &= ~ADC_MUX_L_BITS;
    ADMUX |= ADC_MUX_L_IPHASE_V;
    ADCSRB &= ~ADC_MUX_H_BITS;
    ADCSRB |= ADC_MUX_H_IPHASE_V;
    break;
  case (ADC_MUX_H_IPHASE_V | ADC_MUX_L_IPHASE_V):
    // Handle ADC conversion result for phase current measurement.
    iphaseV = ADCL >> 6;
    iphaseV |= (ADCH << 2);
    ADMUX &= ~ADC_MUX_L_BITS;
    ADMUX |= ADC_MUX_L_IPHASE_W;
    ADCSRB &= ~ADC_MUX_H_BITS;
    ADCSRB |= ADC_MUX_H_IPHASE_W;
    break;
  case (ADC_MUX_H_IPHASE_W | ADC_MUX_L_IPHASE_W):
    // Handle ADC conversion result for phase current measurement.
    iphaseW = ADCL >> 6;
    iphaseW |= (ADCH << 2);
    ADMUX &= ~ADC_MUX_L_BITS;
    ADMUX |= ADC_MUX_L_VBUSVREF;
    ADCSRB &= ~ADC_MUX_H_BITS;
    ADCSRB |= ADC_MUX_H_VBUSVREF;
    break;
  case (ADC_MUX_H_VBUSVREF | ADC_MUX_L_VBUSVREF):
    // Handle ADC conversion result for gate voltage reference measurement.
    vbusVref = ADCL >> 6;
    vbusVref |= (ADCH << 2);
    ADMUX &= ~ADC_MUX_L_BITS;
    ADMUX |= ADC_MUX_L_SPEED;
    ADCSRB &= ~ADC_MUX_H_BITS;
    ADCSRB |= ADC_MUX_H_SPEED;
    break;
  default:
    // This is probably an error and should be handled.
    FatalError((uint8_t)0b0000111);
    break;
  }

  // Clear Timer/Counter0 overflow flag.
  TIFR0 = (1 << TOV0);
}

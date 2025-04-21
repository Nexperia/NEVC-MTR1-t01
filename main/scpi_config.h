#ifndef _SCPI_CONFIG_H_
#define _SCPI_CONFIG_H_

/// Max number of valid tokens.
#define SCPI_MAX_TOKENS 20

/// Max number of registered commands.
#define SCPI_MAX_COMMANDS 20

/// Max number of registered special commands.
#define SCPI_MAX_SPECIAL_COMMANDS 0

/// Length of the message buffer.
#define SCPI_BUFFER_LENGTH 64

/// Max branch size of the command tree and max number of parameters.
#define SCPI_ARRAY_SYZE 6

/// Integer size used for hashes.
#define SCPI_HASH_TYPE uint8_t

// SCPI Identification Definitions
#define SCPI_IDN1 "NEXPERIA"
#define SCPI_IDN2 "NEVB-MTR1-xx"
#define SCPI_IDN3 ""
#define SCPI_IDN4 "NEVC-MTR1-t01-1.0.0"

#endif
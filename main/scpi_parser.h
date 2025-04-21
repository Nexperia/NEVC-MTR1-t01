#ifndef _SCPI_PARSER_H_
#define _SCPI_PARSER_H_

#include "scpi_config.h"
#include "scpi_arrays.h"

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

/// Void template used with SCPI_Parser::RegisterCommand.
using SCPI_caller_t = void (*)(SCPI_Commands, SCPI_Parameters, Stream &);
/// Void template used with SCPI_Parser::RegisterSpecialCommand.
using SCPI_special_caller_t = void (*)(SCPI_Commands, Stream &);

/*!
  Main class of the Vrekrer_SCPI_Parser library.
*/
class SCPI_Parser
{
public:
  // Constructor
  SCPI_Parser();
  // Change the TreeBase for the next RegisterCommand calls
  void SetCommandTreeBase(char *tree_base);
  // SetCommandTreeBase version with RAM string support
  void SetCommandTreeBase(const char *tree_base);
  // SetCommandTreeBase version with Flash strings (F() macro) support
  void SetCommandTreeBase(const __FlashStringHelper *tree_base);
  // Registers a new valid command and associate a procedure to it
  void RegisterCommand(char *command, SCPI_caller_t caller);
  // RegisterCommand version with RAM string support.
  void RegisterCommand(const char *command, SCPI_caller_t caller);
  // RegisterCommand version with Flash strings (F() macro) support
  void RegisterCommand(const __FlashStringHelper *command,
                       SCPI_caller_t caller);
  // Set the function to be used by the error handler.
  void SetErrorHandler(SCPI_caller_t caller);
  /// Variable that holds the last error code.
  ErrorCode last_error = ErrorCode::NoError;
  // Process a message and execute it a valid command is found
  void Execute(char *message, Stream &interface);
  // Gets a message from a Stream interface and execute it
  void ProcessInput(Stream &interface, const char *term_chars);
  // Gets a message from a Stream interface
  char *GetMessage(Stream &interface, const char *term_chars);
  // Prints registered tokens and command hashes to the serial interface
  void PrintDebugInfo(Stream &interface);
  /// Magic number used for hashing the commands
  scpi_hash_t hash_magic_number = 37;
  /// Magic offset used for hashing the commands
  scpi_hash_t hash_magic_offset = 7;
  // Timeout, in miliseconds, for GetMessage and ProcessInput.
  unsigned long timeout = 10;

#if SCPI_MAX_SPECIAL_COMMANDS
  // Registers a new valid special command and associate a procedure to it
  void RegisterSpecialCommand(char *command, SCPI_special_caller_t caller);
  // RegisterSpecialCommand version with RAM string support.
  void RegisterSpecialCommand(const char *command,
                              SCPI_special_caller_t caller);
  // RegisterSpecialCommand version with Flash strings (F() macro) support
  void RegisterSpecialCommand(const __FlashStringHelper *command,
                              SCPI_special_caller_t caller);
#endif

protected:
  // Length of the message buffer.
  const uint8_t buffer_length = SCPI_BUFFER_LENGTH;
  // Max number of valid tokens.
  const uint8_t max_tokens = SCPI_MAX_TOKENS;
  // Max number of registered commands.
  const uint8_t max_commands = SCPI_MAX_COMMANDS;
  // Internal errors container
  struct internal_errors
  {
    // Command storage overflow error
    bool command_overflow = false;
    // Token storage overflow error
    bool token_overflow = false;
    // Branch (SCPI_Commands) storage overflow error
    bool branch_overflow = false;
    // Special command storage overflow error
    bool special_command_overflow = false;
  } setup_errors;
  // Hash result for unknown commands
  const scpi_hash_t unknown_hash = 0;
  // Hash reserved for invalid commands
  const scpi_hash_t invalid_hash = 1;

  // Add a token to the tokens' storage
  void AddToken_(char *token);
  // Get a hash from a command
  scpi_hash_t GetCommandCode_(SCPI_Commands &commands);
  // Number of stored tokens
  uint8_t tokens_size_ = 0;
  // Storage for tokens
  char *tokens_[SCPI_MAX_TOKENS];
  // Number of registered commands
  uint8_t codes_size_ = 0;
  // Registered commands' hash storage
  scpi_hash_t valid_codes_[SCPI_MAX_COMMANDS];
  // Pointers to the functions to be called when a valid command is received
  SCPI_caller_t callers_[SCPI_MAX_COMMANDS + 1];
  // TreeBase branch's hash used when calculating hashes (0 for root)
  scpi_hash_t tree_code_ = 0;
  // TreeBase branch's length (0 for root)
  uint8_t tree_length_ = 0;
  // Message buffer.
  char msg_buffer_[SCPI_BUFFER_LENGTH];
  // Length of the readed message
  uint8_t message_length_ = 0;
  // Varible used for checking timeout errors
  unsigned long time_checker_;

#if SCPI_MAX_SPECIAL_COMMANDS
  // Max number of registered special commands.
  const uint8_t max_special_commands = SCPI_MAX_SPECIAL_COMMANDS;
  // Number of registered special commands
  uint8_t special_codes_size_ = 0;
  // Registered special commands' hash storage
  scpi_hash_t valid_special_codes_[SCPI_MAX_SPECIAL_COMMANDS];
  // Pointers to the functions to be called when a special command is received
  SCPI_special_caller_t special_callers_[SCPI_MAX_SPECIAL_COMMANDS];
#endif
};

#endif
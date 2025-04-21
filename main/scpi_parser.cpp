/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************

   \brief
        SCPI parser implementation file.

   \details
        This file implements the \ref SCPI_Parser class, which is the core component
        of the library, and was originally part of the
        [Vrekrer SCPI Parser](https://github.com/Vrekrer/Vrekrer_scpi_parser) project.

        It has been merged into this code base for customization and linking limitations
        of the Arduino IDE.

        This class provides the functionality to parse and execute commands adhering to
        the Standard Commands for Programmable Instruments (SCPI) syntax. It includes
        methods for registering SCPI commands with associated callback functions, setting
        a command tree base for hierarchical command organization, processing incoming
        SCPI messages from a serial stream, and handling errors. The class also defines
        internal structures and variables used for command storage, tokenization, and
        hashing to efficiently match received commands with their registered handlers.

   \author
        Nexperia: http://www.nexperia.com

   \par Support Page
        For additional support, visit: https://www.nexperia.com/support

   $Author: Aanas Sayed $
   $Date: 2025/04/21 $  \n

 ******************************************************************************/

#include "scpi_parser.h"

// Do nothing function
void DefaultErrorHandler(SCPI_C c, SCPI_P p, Stream &interface) {}

/**
 * \brief SCPI_Parser class constructor.
 *
 * \details
 * Initializes the SCPI_Parser object. It sets the default error handler
 * to a no-operation function.
 *
 * \code
 * SCPI_Parser my_instrument;
 * \endcode
 */
SCPI_Parser::SCPI_Parser()
{
  callers_[max_commands] = &DefaultErrorHandler;
}

/**
 * \brief Adds a token to the internal tokens' storage.
 *
 * \details
 * This private method adds a given token (keyword) to the internal storage
 * used for command parsing. It prevents adding duplicate tokens and checks
 * for potential overflow of the token storage. Query symbols ('?') at the
 * end of the token are removed before storing.
 *
 * \param token A pointer to a null-terminated character array representing the token to add.
 */
void SCPI_Parser::AddToken_(char *token)
{
  if (tokens_size_ >= max_tokens)
  {
    setup_errors.token_overflow = true;
    return;
  }
  size_t token_size = strlen(token);
  // Remove query symbols
  if (token[token_size - 1] == '?')
    token_size--;
  for (uint8_t i = 0; i < tokens_size_; i++)
    // Check if the token is already added
    if ((strncmp(token, tokens_[i], token_size) == 0) and (token_size == strlen(tokens_[i])))
      return;
  char *stored_token = new char[token_size + 1];
  strncpy(stored_token, token, token_size);
  stored_token[token_size] = '\0';
  tokens_[tokens_size_] = stored_token;
  tokens_size_++;
}

/**
 * \brief Gets a hash code for a valid SCPI command.
 *
 * \details
 * This private method calculates a hash code for a given SCPI command
 * represented by a \ref SCPI_Commands object. The hash is computed based
 * on the sequence of keywords in the command and the currently set
 * command tree base. If any of the keywords in the command are not
 * registered as known tokens, the function returns `unknown_hash`.
 *
 * \param commands A reference to the \ref SCPI_Commands object representing the command keywords.
 * \return The calculated hash code for the command, or `unknown_hash` if unregistered tokens are found.
 *
 * \see SetCommandTreeBase
 */
scpi_hash_t SCPI_Parser::GetCommandCode_(SCPI_Commands &commands)
{
  if (tree_code_ == invalid_hash)
    return invalid_hash;
  scpi_hash_t code;
  code = (tree_code_ == 0) ? hash_magic_offset : tree_code_;
  if (commands.Size() == 0)
    return unknown_hash;
  // Loop all keywords in the command
  for (uint8_t i = 0; i < commands.Size(); i++)
  {
    // Get keywords's length
    size_t header_length = strlen(commands[i]);
    // For the last keyword remove the query symbol if needed
    bool is_query = false;
    if (i == commands.Size() - 1)
    {
      is_query = (commands[i][header_length - 1] == '?');
      if (is_query)
        header_length--;
    }

    // Loop over all the known tokens
    for (uint8_t j = 0; j < tokens_size_; j++)
    {
      // Get the token's short and long lengths
      size_t short_length = 0;
      while (isupper(tokens_[j][short_length]))
        short_length++;
      size_t long_length = strlen(tokens_[j]);

      // If the token allows numeric suffixes
      // remove the trailing digits from the keyword
      if ((tokens_[j][long_length - 1] == '#') and (commands[i][header_length - 1] != '#'))
      {
        long_length--;
        while (isdigit(commands[i][header_length - 1]))
          header_length--;
      }

      // Test if the keyword match with the token
      // otherwise test next token
      if (header_length == short_length)
      {
        for (uint8_t k = 0; k < short_length; k++)
          if (not(toupper(commands[i][k]) == tokens_[j][k]))
            goto no_header_token_match;
      }
      else if (header_length == long_length)
      {
        for (uint8_t k = 0; k < long_length; k++)
          if (not(toupper(commands[i][k]) == toupper(tokens_[j][k])))
            goto no_header_token_match;
      }
      else
      {
        goto no_header_token_match;
      }

      // Apply the hashing step using token number j
      // hash(i) = hash(i - 1) * hash_magic_number + j
      code *= hash_magic_number;
      code += j;
      break;

    no_header_token_match:;
      // If the keyword does not match any token return unknown_hash
      if (j == (tokens_size_ - 1))
        return unknown_hash;
    }
    // If last keyword is a query, add a hashing step
    if (is_query)
    {
      code *= hash_magic_number;
      code -= 1;
    }
  }
  return code;
}

/**
 * \brief Changes the base of the command tree for subsequent RegisterCommand calls.
 *
 * \details
 * This method sets a new tree base for organizing SCPI commands hierarchically.
 * The tree base is a sequence of command keywords separated by colons (':').
 * Subsequent calls to \ref RegisterCommand will have their command hashes
 * calculated relative to this tree base. An empty string `""` resets the
 * tree base to the root.
 *
 * \param tree_base A pointer to a null-terminated character array (RAM string) representing the new tree base.
 * Example: `"SYSTem:COMMunication"`.
 *
 * \see RegisterCommand
 */
void SCPI_Parser::SetCommandTreeBase(char *tree_base)
{
  SCPI_Commands tree_tokens(tree_base);
  if (tree_tokens.Size() == 0)
  {
    tree_code_ = 0;
    tree_length_ = 0;
    return;
  }
  for (uint8_t i = 0; i < tree_tokens.Size(); i++)
    AddToken_(tree_tokens[i]);
  tree_code_ = 0;
  tree_code_ = this->GetCommandCode_(tree_tokens);
  tree_length_ = tree_tokens.Size();
  if (tree_tokens.overflow_error)
  {
    setup_errors.branch_overflow = true;
    tree_code_ = invalid_hash;
  }
}

/**
 * \brief SetCommandTreeBase version with RAM string support.
 *
 * \details
 * This overload of \ref SetCommandTreeBase accepts a constant character pointer
 * for the tree base string, which resides in RAM.
 *
 * \param tree_base A constant pointer to a null-terminated character array (RAM string) representing the new tree base.
 * Example: `"SYSTem:LED"`.
 *
 * \see SetCommandTreeBase(char *tree_base)
 */
void SCPI_Parser::SetCommandTreeBase(const char *tree_base)
{
  strcpy(msg_buffer_, tree_base);
  this->SetCommandTreeBase(msg_buffer_);
}

/**
 * \brief SetCommandTreeBase version with Flash strings (F() macro) support.
 *
 * \details
 * This overload of \ref SetCommandTreeBase accepts a Flash string helper object,
 * allowing the tree base string to reside in program memory (Flash), saving RAM.
 *
 * \param tree_base A Flash string helper object (created using the F() macro) representing the new tree base.
 * Example: `F("SYSTem:LED")`.
 *
 * \see SetCommandTreeBase(char *tree_base)
 */
void SCPI_Parser::SetCommandTreeBase(const __FlashStringHelper *tree_base)
{
  strcpy_P(msg_buffer_, (const char *)tree_base);
  this->SetCommandTreeBase(msg_buffer_);
}

/**
 * \brief Registers a new valid SCPI command and associates a callback function with it.
 *
 * \details
 * This method registers a new SCPI command that the parser will recognize. When
 * a matching command is received and parsed, the provided callback function
 * will be executed. The command string can include a query symbol ('?') for
 * query commands. The command is tokenized, and each token is added to the
 * internal token storage if it's not already present. The command's hash
 * is calculated based on the current tree base.
 *
 * \param command A pointer to a null-terminated character array (RAM string) representing the command.
 * Example: `"*IDN?"`, `"MEASure:VOLTage?"`, `"CONFigure:VOLTage 10.5"`.
 * \param caller A function pointer to the callback function (\ref SCPI_caller_t)
 * to be executed when the command is received.
 */
void SCPI_Parser::RegisterCommand(char *command, SCPI_caller_t caller)
{
  if (codes_size_ >= max_commands)
  {
    setup_errors.command_overflow = true;
    return;
  }
  SCPI_Commands command_tokens(command);
  for (uint8_t i = 0; i < command_tokens.Size(); i++)
    this->AddToken_(command_tokens[i]);
  scpi_hash_t code = this->GetCommandCode_(command_tokens);

  // Check for errors
  if (code == unknown_hash)
    code = invalid_hash;
  bool overflow_error = command_tokens.overflow_error;
  overflow_error |= (tree_length_ + command_tokens.Size()) > command_tokens.storage_size;
  setup_errors.command_overflow |= overflow_error;
  if (overflow_error)
    code = invalid_hash;

  valid_codes_[codes_size_] = code;
  callers_[codes_size_] = caller;
  codes_size_++;
}

/**
 * \brief RegisterCommand version with RAM string support.
 *
 * \details
 * This overload of \ref RegisterCommand accepts a constant character pointer
 * for the command string, which resides in RAM.
 *
 * \param command A constant pointer to a null-terminated character array (RAM string) representing the command.
 * Example: `"*IDN?"`.
 * \param caller A function pointer to the callback function (\ref SCPI_caller_t).
 *
 * \see RegisterCommand(char *command, SCPI_caller_t caller)
 */
void SCPI_Parser::RegisterCommand(const char *command, SCPI_caller_t caller)
{
  strcpy(msg_buffer_, command);
  this->RegisterCommand(msg_buffer_, caller);
}

/**
 * \brief RegisterCommand version with Flash strings (F() macro) support.
 *
 * \details
 * This overload of \ref RegisterCommand accepts a Flash string helper object,
 * allowing the command string to reside in program memory (Flash), saving RAM.
 *
 * \param command A Flash string helper object (created using the F() macro) representing the command.
 * Example: `F("*IDN?")`.
 * \param caller A function pointer to the callback function (\ref SCPI_caller_t).
 *
 * \see RegisterCommand(char *command, SCPI_caller_t caller)
 */
void SCPI_Parser::RegisterCommand(const __FlashStringHelper *command,
                                  SCPI_caller_t caller)
{
  strcpy_P(msg_buffer_, (const char *)command);
  this->RegisterCommand(msg_buffer_, caller);
}

/**
 * \brief Sets the function to be used as the error handler.
 *
 * \details
 * This method allows the user to specify a custom error handling function
 * that will be called when the parser encounters an error, such as an
 * unknown command or a buffer overflow. The error handler function
 * receives the parsed commands, parameters (if available), and the
 * communication interface as arguments.
 *
 * \param caller A function pointer to the error handler callback function (\ref SCPI_caller_t).
 * Example: `&myErrorHandler`.
 */
void SCPI_Parser::SetErrorHandler(SCPI_caller_t caller)
{
  callers_[max_commands] = caller;
}

/**
 * \brief Processes an incoming SCPI message and executes the associated command if found.
 *
 * \details
 * This method takes a SCPI message as input, parses it into commands and
 * parameters, and then attempts to find a registered command that matches.
 * If a match is found, the associated callback function is executed, with
 * the parsed commands, parameters, and the communication interface passed as arguments.
 * The message can contain multiple commands separated by semicolons (';').
 *
 * \param message A pointer to a null-terminated character array containing the SCPI message to process.
 * Example: `"*IDN?; MEASure:VOLTage?"`.
 * \param interface A reference to a Stream object (e.g., Serial) used for communication.
 *
 * \see GetMessage
 */
void SCPI_Parser::Execute(char *message, Stream &interface)
{
  while (message != NULL)
  {
    // Save multicomands for later
    char *multicomands = strpbrk(message, ";");
    if (multicomands != NULL)
    {
      multicomands[0] = '\0';
      multicomands++;
    }

    tree_code_ = 0;
    SCPI_Commands commands(message);
    message = multicomands;
    SCPI_Parameters parameters(commands.not_processed_message);
    scpi_hash_t code = this->GetCommandCode_(commands);
    if (code == unknown_hash)
    {
      // Call ErrorHandler UnknownCommand
      last_error = ErrorCode::UnknownCommand;
      (*callers_[max_commands])(commands, parameters, interface);
      continue;
    }
    for (uint8_t i = 0; i < codes_size_; i++)
      if (valid_codes_[i] == code)
      {
        (*callers_[i])(commands, parameters, interface);
        break;
      }
  }
}

/**
 * \brief Reads a message from a Stream interface and executes it.
 *
 * \details
 * This method continuously reads data from the provided Stream interface
 * until a termination character (or sequence of characters) is detected.
 * Once a complete message is received, it is passed to the \ref Execute
 * method for parsing and execution.
 *
 * \param interface A reference to a Stream object (e.g., Serial) to read the message from.
 * \param term_chars A constant pointer to a null-terminated character array containing the termination characters for a message (e.g., "\n", "\r\n").
 *
 * \see GetMessage
 * \see Execute
 */
void SCPI_Parser::ProcessInput(Stream &interface, const char *term_chars)
{
  char *message = this->GetMessage(interface, term_chars);
  if (message != NULL)
  {
    this->Execute(message, interface);
  }
}

/**
 * \brief Reads a message from a Stream interface until termination characters are found.
 *
 * \details
 * This method reads characters from the provided Stream interface and stores
 * them in an internalinternal buffer until one of the specified termination character sequences is encountered. It also handles communication timeouts and buffer overflows, calling the error handler if either occurs.
 *
 * \param interface A reference to a Stream object (e.g., Serial) to read the message from.
 * \param term_chars A constant pointer to a null-terminated character array containing the termination characters for a message (e.g., "\r\n").
 * \return A pointer to the internal message buffer containing the received message (without the termination characters), or `NULL` if no complete message is received due to timeout or no incoming data.
 */
char *SCPI_Parser::GetMessage(Stream &interface, const char *term_chars)
{
  while (interface.available())
  {
    // Read the new char
    msg_buffer_[message_length_] = interface.read();
    ++message_length_;
    time_checker_ = millis();

    if (message_length_ >= buffer_length)
    {
      // Call ErrorHandler due BufferOverflow
      last_error = ErrorCode::BufferOverflow;
      (*callers_[max_commands])(SCPI_C(), SCPI_P(), interface);
      message_length_ = 0;
      return NULL;
    }

#if SCPI_MAX_SPECIAL_COMMANDS
    // For the first space only.
    if (strcspn(msg_buffer_, " ") == message_length_ - 1)
    {
      msg_buffer_[message_length_ - 1] = '\0';
      tree_code_ = 0;
      SCPI_Commands commands(msg_buffer_);
      scpi_hash_t code = this->GetCommandCode_(commands);
      for (uint8_t i = 0; i < special_codes_size_; i++)
        if (valid_special_codes_[i] == code)
        {
          (*special_callers_[i])(commands, interface);
          message_length_ = 0;
          return msg_buffer_;
        }
      // restore original message.
      msg_buffer_[message_length_ - 1] = ' ';
      for (uint8_t i = 0; i < commands.Size() - 1; i++)
        commands[i][strlen(commands[i])] = ':';
      commands.not_processed_message--;
      commands.not_processed_message[0] = ' ';
    }
#endif

    // Test for termination chars (end of the message)
    msg_buffer_[message_length_] = '\0';
    if (strstr(msg_buffer_, term_chars) != NULL)
    {
      // Return the received message
      msg_buffer_[message_length_ - strlen(term_chars)] = '\0';
      message_length_ = 0;
      return msg_buffer_;
    }
  }
  // No more chars available yet

  // Return NULL if no message is incoming
  if (message_length_ == 0)
    return NULL;

  // Check for communication timeout
  if ((millis() - time_checker_) > timeout)
  {
    // Call ErrorHandler due Timeout
    last_error = ErrorCode::Timeout;
    (*callers_[max_commands])(SCPI_C(), SCPI_P(), interface);
    message_length_ = 0;
    return NULL;
  }

  // No errors, be sure to return NULL
  return NULL;
}

/**
 * \brief Prints debug information about the SCPI parser configuration and status to a Stream interface.
 *
 * \details
 * This method prints various internal details of the SCPI parser, including
 * the maximum sizes of internal buffers and arrays, the currently registered
 * tokens and commands with their hash codes and associated handler function
 * addresses, and any setup errors that might have occurred (e.g., buffer
 * overflows). This information is useful for debugging and understanding
 * the parser's internal state.
 *
 * \param interface A reference to a Stream object (e.g., Serial) to print the debug information to.
 */
void SCPI_Parser::PrintDebugInfo(Stream &interface)
{
  interface.println(F("*** DEBUG INFO ***\n"));
  interface.print(F("Max command tree branches: "));
  interface.print(SCPI_ARRAY_SYZE);
  interface.println(F(" (SCPI_ARRAY_SYZE)"));
  if (setup_errors.branch_overflow)
    interface.println(F(" **ERROR** Max branch size exceeded."));
  interface.print(F("Max number of parameters: "));
  interface.print(SCPI_ARRAY_SYZE);
  interface.println(F(" (SCPI_ARRAY_SYZE)"));
  interface.print(F("Message buffer size: "));
  interface.print(buffer_length);
  interface.println(F(" (SCPI_BUFFER_LENGTH)\n"));

  interface.print(F("TOKENS : "));
  interface.print(tokens_size_);
  interface.print(F(" / "));
  interface.print(max_tokens);
  interface.println(F(" (SCPI_MAX_TOKENS)"));
  if (setup_errors.token_overflow)
    interface.println(F(" **ERROR** Max tokens exceeded."));
  for (uint8_t i = 0; i < tokens_size_; i++)
  {
    interface.print(F("  "));
    interface.print(i + 1);
    interface.print(F(":\t"));
    interface.println(String(tokens_[i]));
    interface.flush();
  }
  interface.println();

  bool hash_crash = false;
  bool unknown_error = false;
  bool invalid_error = false;
  interface.print(F("VALID CODES : "));
  interface.print(codes_size_);
  interface.print(F(" / "));
  interface.print(max_commands);
  interface.println(F(" (SCPI_MAX_COMMANDS)"));
  if (setup_errors.command_overflow)
    interface.println(F(" **ERROR** Max commands exceeded."));
  interface.println(F("  #\tHash\t\tHandler"));
  for (uint8_t i = 0; i < codes_size_; i++)
  {
    interface.print(F("  "));
    interface.print(i + 1);
    interface.print(F(":\t"));
    interface.print(valid_codes_[i], HEX);
    if (valid_codes_[i] == unknown_hash)
    {
      interface.print(F("!*"));
      unknown_error = true;
    }
    else if (valid_codes_[i] == invalid_hash)
    {
      interface.print(F("!%"));
      invalid_error = true;
    }
    else
      for (uint8_t j = 0; j < i; j++)
        if (valid_codes_[i] == valid_codes_[j])
        {
          interface.print("!!");
          hash_crash = true;
          break;
        }
    interface.print(F("\t\t0x"));
    interface.print(long(callers_[i]), HEX);
    interface.println();
    interface.flush();
  }
  if (unknown_error)
    interface.println(F(" **ERROR** Tried to register ukwnonk tokens. (!*)"));
  if (invalid_error)
    interface.println(F(" **ERROR** Tried to register invalid commands. (!%)"));
  if (hash_crash)
    interface.println(F(" **ERROR** Hash crashes found. (!!)"));

#if SCPI_MAX_SPECIAL_COMMANDS
  hash_crash = false;
  unknown_error = false;
  invalid_error = false;
  interface.println();
  interface.print(F("VALID SPECIAL CODES : "));
  interface.print(special_codes_size_);
  interface.print(F(" / "));
  interface.print(max_special_commands);
  interface.println(F(" (SCPI_MAX_SPECIAL_COMMANDS)"));
  if (setup_errors.special_command_overflow)
    interface.println(F(" **ERROR** Max special commands exceeded."));
  interface.println(F("  #\tHash\t\tHandler"));
  for (uint8_t i = 0; i < special_codes_size_; i++)
  {
    interface.print(F("  "));
    interface.print(i + 1);
    interface.print(F(":\t"));
    interface.print(valid_special_codes_[i], HEX);
    if (valid_special_codes_[i] == unknown_hash)
    {
      interface.print(F("!*"));
      unknown_error = true;
    }
    else if (valid_special_codes_[i] == invalid_hash)
    {
      interface.print(F("!%"));
      invalid_error = true;
    }
    else
      for (uint8_t j = 0; j < i; j++)
        if (valid_special_codes_[i] == valid_special_codes_[j])
        {
          interface.print("!!");
          hash_crash = true;
          break;
        }
    interface.print(F("\t\t0x"));
    interface.print(long(special_callers_[i]), HEX);
    interface.println();
    interface.flush();
  }
  if (unknown_error)
    interface.println(F(" **ERROR** Tried to register ukwnonk tokens. (!*)"));
  if (invalid_error)
    interface.println(F(" **ERROR** Tried to register invalid commands. (!%)"));
  if (hash_crash)
    interface.println(F(" **ERROR** Hash crashes found. (!!)"));
#endif

  interface.println(F("\nHASH Configuration:"));
  interface.print(F("  Hash size: "));
  interface.print((uint8_t)(sizeof(scpi_hash_t) * 8));
  interface.println(F("bits (SCPI_HASH_TYPE)"));
  interface.print(F("  Hash magic number: "));
  interface.println(hash_magic_number);
  interface.print(F("  Hash magic offset: "));
  interface.println(hash_magic_offset);
  interface.println(F("\n*******************\n"));
}

#if SCPI_MAX_SPECIAL_COMMANDS

/**
 * \brief Registers a new valid special SCPI command (without parameters) and associates a callback function with it.
 *
 * \details
 * This method registers a special SCPI command that does not expect any parameters.
 * When a matching special command is received, the provided callback function
 * will be executed. The command string can include a query symbol ('?').
 * The command is tokenized, and each token is added to the internal token
 * storage if it's not already present. The command's hash is calculated
 * based on the current tree base. Special commands are typically used for
 * actions or queries that don't require additional arguments. They are
 * checked for specifically at the beginning of the input buffer.
 *
 * \param command A pointer to a null-terminated character array (RAM string) representing the special command.
 * Example: `"RESET"`, `"GET:STATUS?"`.
 * \param caller A function pointer to the callback function (\ref SCPI_special_caller_t)
 * to be executed when the special command is received.
 */
void SCPI_Parser::RegisterSpecialCommand(char *command,
                                         SCPI_special_caller_t caller)
{
  if (special_codes_size_ >= max_special_commands)
  {
    setup_errors.special_command_overflow = true;
    return;
  }
  SCPI_Commands command_tokens(command);
  for (uint8_t i = 0; i < command_tokens.Size(); i++)
    this->AddToken_(command_tokens[i]);
  scpi_hash_t code = this->GetCommandCode_(command_tokens);

  // Check for errors
  if (code == unknown_hash)
    code = invalid_hash;
  bool overflow_error = command_tokens.overflow_error;
  overflow_error |= (tree_length_ + command_tokens.Size()) > command_tokens.storage_size;
  setup_errors.branch_overflow |= overflow_error;
  if (overflow_error)
    code = invalid_hash;

  valid_special_codes_[special_codes_size_] = code;
  special_callers_[special_codes_size_] = caller;
  special_codes_size_++;
}

/**
 * \brief RegisterSpecialCommand version with RAM string support.
 *
 * \details
 * This overload of \ref RegisterSpecialCommand accepts a constant character
 * pointer for the special command string, which resides in RAM.
 *
 * \param command A constant pointer to a null-terminated character array (RAM string) representing the special command.
 * Example: `"GET:DATA"`.
 * \param caller A function pointer to the callback function (\ref SCPI_special_caller_t).
 *
 * \see RegisterSpecialCommand(char *command, SCPI_special_caller_t caller)
 */
void SCPI_Parser::RegisterSpecialCommand(const char *command,
                                         SCPI_special_caller_t caller)
{
  strcpy(msg_buffer_, command);
  this->RegisterSpecialCommand(msg_buffer_, caller);
}

/**
 * \brief RegisterSpecialCommand version with Flash strings (F() macro) support.
 *
 * \details
 * This overload of \ref RegisterSpecialCommand accepts a Flash string helper
 * object, allowing the special command string to reside in program memory
 * (Flash), saving RAM.
 *
 * \param command A Flash string helper object (created using the F() macro) representing the special command.
 * Example: `F("GET:DATA")`.
 * \param caller A function pointer to the callback function (\ref SCPI_special_caller_t).
 *
 * \see RegisterSpecialCommand(char *command, SCPI_special_caller_t caller)
 */
void SCPI_Parser::RegisterSpecialCommand(const __FlashStringHelper *command,
                                         SCPI_special_caller_t caller)
{
  strcpy_P(msg_buffer_, (const char *)command);
  this->RegisterSpecialCommand(msg_buffer_, caller);
}

#endif
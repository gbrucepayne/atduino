# V.25 AT Command Parser for Arduino Framework

This was developed due to seeming lack of a generalized AT command processing
library in the Arduino ecosystem. Several other implementations exist for more
specific purposes but miss certain functions such as non-verbose (`V0`) mode,
echo on/off, or unsolicited result codes.

It also provides various text processing utilities with options for `String` or
`c_str` char arrays to accommodate different board/processor constraints.

## Client

The client functionality is used to talk to a modem (or anything similar that
supports AT commands).

Allows `HardwareSerial` (*recommended*) or `SoftwareSerial` as a `Stream`
to interface to a modem via UART.

Allows for processing of command/response or receipt of unsolicited result code
(URC) emitted by the modem. Also includes an optional CRC validation supported
by some modems.

### Command/Response

This is the main mode of intended use. The logic flow is as follows:

1. AT commmand, with optional timeout, is submitted by a function call
`sendAtCommand()` which:
    * If a prior command is pending (TBD thread-safe) returns `false`;
    * Clears the last error code;
    * Clears the receive buffer;
    * (Optional) calculates and applies CRC to the command;
    * Applies the command line termination character (default `\r`);
    * Sends the command on serial and waits for all data to be sent;
    * Sets the pending command state;
    * Calls an internal response parsing function and returns `true`
    if successful;
    * If no timeout is specified, the default is 1 second
    (`AT_TIMEOUT_DEFAULT_MS`).

2. Response parsing indicates success or failure code accessible via
`lastErrorCode()` and delivered via the callback if registered, and successful
responses are stored in a *get* buffer for retrieval:
    * Transitioning through states `ECHO`, `INFO`, (*optional*) `CRC` to either
    `RESULT_OK` or `RESULT_ERROR`;
    * If timeout is exceeded, parsing stops and indicates `AT_TIMEOUT` failure;
    * (Optional) validation of checksum, failure indicates `AT_CRC_RX_ERROR`;
    * Other modem error codes received will be indicated transparently;
    * Successful parsing will place the response in the *get* buffer;
    * Sets the last error code or `AT_OK` if successful;
    * Clears the pending command state.

3. Retrieval of successful response is done using `getResponse()` or
`sgetResponse()` with an optional `prefix` that can be removed.
All other leading/trailing whitespace is removed, and multi-line responses are
separated by a single line feed (`\n`). Retrieval clears the *get* buffer.

### Unsolicited Result Codes (URC)

Some modems emit unsolicited codes. In these cases it is recommended that the
application checks/retrieves any URC(s) prior to submitting any AT command.

`checkUrc()` simply checks if any serial data is waiting when no AT command is
pending, and if present parses until both command line termination and response
formatting character have been received or timeout (default 1 second
`URC_DEFAULT_TIMEOUT`).
URC data is placed in the *get* buffer and retrieved in the same way as a
commmand response.

### CRC support

Currently a CCITT-16-CRC option is supported for commands and responses. The
enable/disable command may be configured using `AT_CRC_ENABLE` (default `"CRC"`).

## Server (Work in Progress)

The server concept is to act as a modem/proxy replying to a microcontroller.

You register custom commands using `addCommand` with a data structure that
includes the command `name` and optional callback functions for `read`, `run`,
`test` and `write` operations.

`Verbose` and `Echo` features are supported using the standard `V` and `E`
commands defined in the spec.

`CRC` is an optional extended command to support 16-bit checksum validation of
requests and responses that can be useful in noisy environments.

### Feature considerations

* Repeating a command line using `A/` or `a/` is not supported;
* No special consideration is given for numeric or string constants, those are
left to custom handling functions;
* Concatenation of basic commands deviates from the standard and expects a
semicolon separator;

### Acknowledgements

The server idea is based somewhat on the
[ATCommands](https://github.com/yourapiexpert/ATCommands)
library which had some shortcomings for my cases including GPL, and
[cAT](https://github.com/marcinbor85/cAT) but reframed for C++.
Many thanks to those developers for some great ideas!
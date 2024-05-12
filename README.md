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
to interface to a modem via UART (or digital pins).

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
    * Calls an internal response parsing function and returns an `at_error_t`
    code, with 0 (`AT_OK`) indicating success;
    * If no timeout is specified, the default is 1 second
    (`AT_TIMEOUT_DEFAULT_MS`).

2. Response parsing:
    * Transitions through states `_ECHO`, `_RESPONSE`, (*optional*) `_CRC`
    to either `_OK` or `_ERROR`;
    * If timeout is exceeded, parsing stops and indicates `AT_ERR_TIMEOUT`;
    * (Optional) validation of checksum, failure indicates `AT_ERR_CMD_CRC`;
    * Other modem error codes received will be indicated transparently;
    * Successful parsing will place the response into a buffer for retrieval;
    * Sets the last error code or `AT_OK` (0) if successful;
    * Clears the pending command state.

3. Retrieval of successful response is done using `getResponse()` or
`sgetResponse()` with an optional `prefix` to remove.
All other leading/trailing whitespace is removed, and multi-line responses are
separated by a single line feed (`\n`). Retrieval clears the *get* buffer.

4. A virtual function `lastErrorCode()` is intended to be defined for modems
that support this concept (e.g. query `S80?` on Orbcomm satellite modem).

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
enable/disable command may be configured using `+CRC=<1|0>`.
(`%CRC=<1|0>` also works)

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
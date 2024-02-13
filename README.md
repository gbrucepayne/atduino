# V.25 AT Command Parser for Arduino Framework

This was developed due to seeming lack of a generalized AT command processing
library in the Arduino ecosystem. Several other implementations exist for more
specific purposes but miss certain functions such as non-verbose (`V0`) mode,
echo on/off, or unsolicited result codes.

Allows `HardwareSerial` (*recommended*) or `SoftwareSerial` as a `Stream`
to interface to a modem via UART.

Allows for processing of command/response or receipt of unsolicited result code
(URC) emitted by the modem. Also includes an optional CRC validation supported
by some modems.

Implements various text processing utilities with options for `String` or
`c_str` char arrays to accommodate different board/processor constraints.

## Command/Response

This is the main mode of intended use. The logic flow is as follows:

1. (*Optional*) register a callback for response completion which will receive
a `at_error_t` integer code indicating success (`AT_OK` = 0) or error (> 0).
If a callback is not specified, the state of response ready may be polled using
`responseReady()`.

1. AT commmand, with optional timeout, is submitted by a function call
`sendAtCommand()` which:
    * If a prior command is pending (TBD thread-safe) returns `AT_BUSY`;
    * Clears the last error code;
    * Clears the receive buffer, putting any residual data in an orphan buffer;
    * (Optional) calculates and applies CRC to the command;
    * Applies the command line termination character (default `\r`);
    * Sends the command on serial and waits for all data to be sent;
    * Sets the pending command state;
    * Starts the response parsing state transitions to listening mode;
    * If no timeout is specified, the default is 5 seconds
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
`sgetResponse()` with an optional `prefix` (pointer) that can be removed.
All other leading/trailing whitespace is removed, and multi-line responses are
separated by a single line feed (`\n`). Retrieval clears the *get* buffer.

## Unsolicited Result Codes (URC)

Some modems emit unsolicited codes. In these cases it is recommended that the
application checks/retrieves any URC(s) prior to submitting any AT command.

`checkUrc()` simply checks if any serial data is waiting when no AT command is
pending, and if present parses until both command line termination and response
formatting character have been received or timeout (default 1 second
`URC_DEFAULT_TIMEOUT`).
URC data is placed in the *get* buffer and retrieved in the same way as a
commmand response.
If a callback is registered, it will be passed `AT_URC` code to prompt retrieval.

## CRC support

Currently a CCITT-16-CRC option is supported for commands and responses. The
enable/disable command may be configured using `AT_CRC_ENABLE` (default `"CRC"`).
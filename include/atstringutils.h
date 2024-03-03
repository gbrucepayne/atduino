/**
 * @file atstringutils.h
 * @author G.Bruce-Payne (geoff.bruce-payne@nimarsat.com)
 * @brief String-like char manipulation utilities for memory-constrained devices
 * also compatible with Arduino String
 * @version 0.1
 * @date 2023-09-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef AT_STRING_UTILS_H
#define AT_STRING_UTILS_H

#include <Arduino.h>
#include "atdebug.h"

namespace at {

/**
 * @brief Print a printable character or known substitutions or the int value.
 * Known substitiions: <cr>, <lf>. Unprintable values appear as `[ <int> ]`.
 * 
 * @param c The character to attempt to print
 * @param print Flag set to debug print
 * @return false If the character is unprintable and not a known substitution
 */
bool printableChar(const char c, bool print = false);

/**
 * @brief Print a (c)string using `printableChar` for debug.
 * 
 * @param str The (c)string to debug print
 */
void debugPrint(const char* str);
void debugPrint(const String& str);

/**
 * @brief Get a string using `printableChar` substitutions for debug.
 * 
 * @param str The original string or character
 * @param start Optional slice start (default 0 = start of string)
 * @param end Optional slice end (default/0 = end of string)
 * @return The debug-formatted string
 */
String debugString(const char* str, size_t start = 0, size_t end = 0);
String debugString(const String& str, size_t start = 0, size_t end = 0);
String debugString(const char c);

/**
 * @brief Append a string to another string
 * 
 * @param target The target (buffer) to append to
 * @param substr The substring to append
 * @param buffer_size The maximum size of the target buffer
*/
bool append(char* target, const char* substr, size_t buffer_size);
bool append(String& target, const String& substr);

/**
 * @brief Check if a string includes a substring
 * 
 * @param str The string to search
 * @param substr The substring or character to find
 * @return true if substring is found in string
 */
bool includes(const char* str, const char* substr);
bool includes(const char* str, const char c);
bool includes(const String& str, const String& substr);
bool includes(const String& str, const char c);

/**
 * @brief Get the index of a character or substring within a string
 * 
 * @param str The string to search
 * @param substr The substring to search for
 * @return int The index within the string or -1 if not found
 */
int indexOf(const char* str, const char* substr);
int indexOf(const char* str, const char c);

/**
 * @brief Get the number of instances of substring in a string
 * 
 * @param str The string to search
 * @param substr The substring to search for
 * @return int The count of substring instances in string
 */
int instancesOf(const char* str, const char* substr);
int instancesOf(const char* str, const char c);
int instancesOf(const String& str, const String& substr);

/**
 * @brief Determine if string starts with substring
 * 
 * @param str The string to search
 * @param substr The substring to find
 * @return true If str starts with substr
 */
bool startsWith(const char* str, const char* substr, bool end = false);
bool startsWith(const String& str, const String& substr);

/**
 * @brief Determine if string ends with substring
 * 
 * @param str The string to search
 * @param substr The substring to find
 * @return true If str ends with substr
 */
bool endsWith(const char* str, const char* substr);
bool endsWith(const String& str, const String& substr);

/**
 * @brief Get a substring from a string
 * 
 * @param substr The destination substring
 * @param str The string to extract from
 * @param start The string index to start extracting from
 * @param end The string index to stop extracting from (default 0 = to the end)
 * @return true if successful
 */
bool substring(char* substr, const char* str, size_t start, size_t end = 0);
bool substring(String& substr, const String& str, size_t start, size_t end = 0);

/**
 * @brief Remove characters from a string after an index up to n characters
 * 
 * @param str The original string to be overwritten
 * @param index The starting index to remove from
 * @param count (Optional) The number of characters to remove (default 0 = all)
 * @return true if successful
 */
bool remove(char* str, size_t index, size_t count = 0);
bool remove(String& str, size_t index, size_t count = 0);

/**
 * @brief Replace a substring with another within a string
 * 
 * @param str The original string to be overwritten
 * @param old_substr The substring to be replaced
 * @param new_substr The replacement substring
 * @param buffer_size The size of the (c)string buffer
 */
void replace(char* str, const char* old_substr, const char* new_substr,
             size_t buffer_size, size_t max_count = 0);
void replace(String& str, const String& old_substr, const String& new_substr);

/**
 * @brief Remove leading and trailing whitespace from a string
 * 
 * @param str The string to trim
 * @param buffer_length The size of the (c)string buffer
 */
void trim(char* str, size_t buffer_length);
void trim(String& str);

/**
 * @brief Get the next parameter from a response string to the next separator
 * 
 * @param at_param The buffer to store the result in
 * @param response Pointer to the response (or offset) char array
 * @param buffersize The parameter buffer size
 * @param sep The separator (default `,` based on V.25 standard)
 * @returns -1 if unable to extract the parameter
 * @returns The offset of the next parameter relative to response pointer
*/
long getNextParameter(char* at_param, const char* response,
                      size_t buffersize, const char sep = ',');

/**
 * @brief Convert an unsigned integer to ASCII string
 * 
 * @param n The number to convert
 * @param result The destination (c)string
 * @param result_size The c-string buffer size
 */
void uintToChar(uint32_t n, char* result, size_t result_size);

/**
 * @brief Check if a string is valid hexadecimal characters
 * 
 * @param candidate The candidate (c)string
*/
bool isHex(const char* candidate);

/**
 * @brief Convert an integer value to a hex string
 * 
 * @param hex_string The target string
 * @param value The integer value
 * @param width The hexadecimal string width
 */
void intToHex(char* hex_string, int value, uint8_t width, size_t buffer_size);
void intToHex(String& hex_string, int value, uint8_t width);

/**
 * @brief Get the integer value of a hexadecimal string
 * 
 * @param hex_string The hexadecimal string
 * @return unsigned long 
 */
uint32_t hexToInt(const char* hex_string);
uint32_t hexToInt(const String& hex_string);

/**
 * @brief Encode a buffer to a Base64 string.
 * The output string must be sized appropriately e.g. `base64StringLength` + 1.
 * 
 * @param b64_str The output string (length >= base64StringLength + 1)
 * @param buffer The data buffer to encode
 * @param buffer_len The length of the data buffer to encode
 */
void base64Encode(char* b64_str, const uint8_t* buffer, size_t buffer_len);
void base64Encode(String& b64_str, const uint8_t* buffer, size_t buffer_len);

/**
 * @brief Decode a Base64 string to a char buffer
 * 
 * @param buffer The target buffer
 * @param b64_str The string to decode
 */
void base64Decode(char* buffer, const char* b64_str);

/**
 * @brief Get the buffer size required for a Base64 decode
 * 
 * @param b64_str The Base64 string
 * @return size_t 
 */
size_t base64BufferLength(const char* b64_str);

/**
 * @brief Get the required string length for an decoded Base64 buffer
 * 
 * @param buffersize 
 * @return size_t 
 */
size_t base64StringLength(size_t buffersize);

}   // namespace at

#endif   // AT_STRING_UTILS_H
/**
 * @file devatstringutils.cpp
 * @author G.Bruce-Payne (geoff.bruce-payne@nimarsat.com)
 * @brief String-like char manipulation utilities for memory-constrained devices
 * @version 0.1
 * @date 2023-09-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "atstringutils.h"

namespace at {

bool printableChar(const char c, bool print) {
  bool printable = true;
#ifndef ARDEBUG_DISABLED
  char to_print[8] = "";
  if (c == 8) {
    snprintf(to_print, 8, "<bs>");
  } else if (c == 10) {
    snprintf(to_print, 8, "<lf>");
  } else if (c == 13) {
    snprintf(to_print, 8, "<cr>");
  } else if (c < 32 || c > 125) {
    printable = false;
    snprintf(to_print, 8, "[%d]", c);
  } else {
    snprintf(to_print, 8, "%c", c);
  }
  if (print)
    ardprintf(to_print);
#endif
  return printable;
}

void debugPrint(const char* str) {
  size_t str_len = strlen(str);
  for (size_t i = 0; i < str_len; i++) {
    printableChar(str[i]);
  }
}

void debugPrint(const String& str) {
  size_t str_len = str.length();
  for (size_t i = 0; i < str_len; i++) {
    printableChar(str[i]);
  }
}

String debugString(const String &str, size_t start, size_t end) {
  String debug_string = "";
#ifndef ARDEBUG_DISABLED
  size_t str_len = str.length();
  if (start > str_len)
    start = 0;
  if (end > 0 && end > start)
    str_len = end;
  for (size_t i = start; i < str_len; i++) {
    char c = str[i];
    if (c == '\b') {
      debug_string += "<bs>";
    } else if (c == '\r') {
      debug_string += "<cr>";
    } else if (c == '\n') {
      debug_string += "<lf>";
    } else if (c < 32 || c > 125) {
      debug_string += "[" + String((int)c) + "]";
    // } else if (c == '%') {   // escape for printf
    //   debug_string += "%" + c;
    } else {
      debug_string += c;
    }
  }
#endif
  return debug_string;
}

String debugString(const char* str, size_t start, size_t end) {
  return debugString(String(str), start, end);
}

String debugString(const char c) {
  return debugString(String(c));
}

bool append(char* target, const char* substr, size_t buffer_size) {
  size_t app_idx = strlen(target);
  size_t adder = strlen(substr);
  if ((app_idx + adder) >= (buffer_size - 1)) return false;
  strncpy(target + app_idx, substr, buffer_size);
  target[app_idx + adder + 1] = '\0';
  return true;
}

bool append(String& target, const String& substr) {
  target = target + substr;
  return true;
}

bool includes(const char *str, const char *substr) {
  // AR_LOGV("Assessing %s for %s", debugString(str), debugString(substr).c_str());
  return strstr(str, substr) != nullptr;
}

bool includes(const char *str, const char c) {
  size_t str_len = strlen(str);
  for (size_t i = 0; i < str_len; i++) {
    if (str[i] == c)
      return true;
  }
  return false;
}

bool includes(const String &str, const String &substr) {
  // size_t s_buf_len = str.length() + 1;
  // char s[s_buf_len];
  // char ss[s_buf_len];
  // str.toCharArray(s, s_buf_len);
  // substr.toCharArray(ss, s_buf_len);
  return includes(str.c_str(), substr.c_str());
}

bool includes(const String &str, const char c) {
  return includes(str.c_str(), c);
}

int indexOf(const char *str, const char *substr) {
  size_t s_len = strlen(str);
  size_t ss_len = strlen(substr);
  size_t char_matches = 0;
  int index = -1;
  if (ss_len > 0) {
    for (size_t i = 0, j = 0; i < s_len; i++) {
      if (str[i] == substr[j]) {
        if (j == 0)
          index = i;
        char_matches++;
        j++;
        if (char_matches == ss_len) {
          break;
        }
      } else if (char_matches > 0) {
        char_matches = 0;
        i--;
        j = 0;
      }
    }
  }
  if (char_matches != ss_len)
    index = -1;
  return index;
}

int indexOf(const char *str, const char c) {
  char subcstr[2];
  subcstr[0] = c;
  subcstr[1] = '\0';
  return indexOf(str, subcstr);
}

int instancesOf(const char *str, const char *substr) {
  // AR_LOGV("Searching %s for %s", debugString(str).c_str(), debugString(substr).c_str());
  int instances = 0;
  size_t s_len = strlen(str);
  size_t ss_len = strlen(substr);
  size_t char_matches = 0;
  if (ss_len > 0) {
    for (size_t i = 0, j = 0; i < s_len; i++) {
      // AR_LOGV("Assessing %s vs %s", debugString(str[i]).c_str(), debugString(substr[j]).c_str());
      if (str[i] == substr[j]) {
        // AR_LOGV("Found match for %s", debugString(substr[j]).c_str());
        char_matches++;
        j++;
        if (char_matches == ss_len) {
          instances += 1;
          char_matches = 0;
          j = 0;
        }
      } else if (char_matches > 0) {
        // AR_LOGV("Mismatch for %s vs %s", debugString(substr[j]).c_str(), debugString(str[i]).c_str());
        char_matches = 0;
        i--;
        j = 0;
      }
    }
  }
  return instances;
}

int instancesOf(const char* str, const char c) {
  char substr[2] = "";
  substr[0] = c;
  return instancesOf(str, (const char*)substr);
}

int instancesOf(const String &str, const String &substr) {
  // size_t s_buf_len = str.length() + 1;
  // size_t ss_buf_len = substr.length() + 1;
  // char s[s_buf_len];
  // char ss[ss_buf_len];
  // str.toCharArray(s, s_buf_len);
  // substr.toCharArray(ss, ss_buf_len);
  return instancesOf(str.c_str(), substr.c_str());
}

bool startsWith(const char *str, const char *substr, bool end) {
  size_t s_len = strlen(str);
  size_t ss_len = strlen(substr);
  size_t char_matches = 0;
  int offset = end ? s_len - ss_len : 0;
  if (ss_len > 0) {
    if (s_len >= ss_len) {
      for (size_t i = offset, j = 0; i < s_len; i++, j++) {
        if (str[i] == substr[j]) {
          char_matches++;
          if (char_matches == ss_len)
            break;
        }
      }
    }
  }
  return char_matches == ss_len;
}

bool startsWith(const char* str, const char c, bool end) {
  char substr[2] = "";
  substr[0] = c;
  return startsWith(str, substr, end);
}

bool startsWith(const String &str, const String &substr) {
  return str.startsWith(substr);
}

bool endsWith(const char *str, const char *substr) {
  return startsWith(str, substr, true);
}

bool endsWith(const String &str, const String &substr) {
  return str.endsWith(substr);
}

bool substring(char *substr, const char *str, size_t start, size_t end) {
  if (start >= strlen(str)) {
    AR_LOGE("Invalid argument: start must be less than the string length");
    return false;
  }
  const char *original = str;
  if (end == 0)
    end = strlen(str);
  size_t j = 0;
  for (size_t i = start; i < end; i++) {
    substr[j++] = original[i];
  }
  substr[j] = '\0';
  return true;
}

bool substring(String &substr, const String &str, size_t start, size_t end) {
  if (start >= str.length())
    return false;
  if (end > 0) {
    substr = str.substring(start, end);
  } else {
    substr = str.substring(start, end);
  }
  return true;
}

bool remove(char *str, size_t index, size_t count) {
  size_t buffer_size = strlen(str) + 1;
  if (index > buffer_size - 1) {
    AR_LOGE("Invalid parameter: index exceeds string length");
    return false;
  }
  if (count == 0) {
    str[index] = '\0';
  } else {
    int moved_len = strlen(str) - count;
    if (moved_len <= 0) {
      AR_LOGE("Invalid parameter: index + count exceeds string length");
      return false;
    }
    char* p_start = str + index;
    char* p_end = str + index + count;
    memmove(p_start, p_end, moved_len);
    str[buffer_size] = '\0';
  }
  return true;
}

bool remove(String &str, size_t index, size_t count) {
  str.remove(index, count);
  return true;
}

bool replace(char *str, const char *old_substr, const char *new_substr,
             size_t buffer_size, size_t max_count) {
  if (strcmp(old_substr, new_substr) == 0)
    return true;
  int replacements = instancesOf((const char*)str, old_substr);
  AR_LOGV("Found %d instances of %s to replace with %s", replacements, 
      debugString(old_substr).c_str(), debugString(new_substr).c_str());
  if (replacements > 0) {
    size_t new_len = strlen(str) + (replacements *
                     (strlen(new_substr) - strlen(old_substr)));
    if (new_len >= buffer_size - 1) {
      AR_LOGE("Buffer too small for replacement string");
      return false;
    }
    int replacement_count = 0;
    char *p_old = str;
    std::vector<char> tmp = {};
    size_t offset = strlen(new_substr);
    while (includes((const char*)p_old, old_substr)) {
      replacement_count++;
      size_t idx = indexOf(p_old, old_substr);
      AR_LOGV("Found %s in %s at index %d", debugString(old_substr).c_str(), 
          debugString(p_old).c_str(), idx);
      for (size_t i = 0; i < idx; i++) {
        // AR_LOGV("Adding %s to vector", debugString(*p_old).c_str());
        tmp.push_back(*p_old++);
      }
      for (size_t i = 0; i < offset; i++) {
        // AR_LOGV("Adding %s to vector", debugString(new_substr[i]).c_str());
        tmp.push_back(new_substr[i]);
      }
      p_old += strlen(old_substr);
      if (max_count > 0 && replacement_count >= max_count)
        break;
    }
    if (tmp.size() < new_len) {
      size_t remaining_chars = strlen(p_old);
      for (size_t i = 0; i < remaining_chars; i++) {
        // AR_LOGV("Adding %s to vector", debugString(*p_old).c_str());
        tmp.push_back(*p_old++);
      }
    }
    strncpy(str, tmp.data(), buffer_size);
    if (strlen(tmp.data()) >= buffer_size) {
      AR_LOGW("Resulting string exceeds buffer size - truncating");
      str[buffer_size - 1] = 0;
    }
    AR_LOGV("Replaced %d - result: %s",
        replacement_count, debugString(str).c_str());
  }
  return true;
}

bool replace(String &str, const String &old_substr, const String &new_substr) {
  str.replace(old_substr, new_substr);
  return true;
}

/**
 * @brief Check if the target character is whitespace (`\n`, `\r` or space)
 * 
 * @param c The character to check
 * @return true If the character is whitespace
 */
static bool isWhitespace(const char c) {
  return (c == '\r' || c == '\n' || c == ' ');
}

void trim(char *str, size_t buffer_size) {
  size_t replaced = 0;
  size_t olen = strlen(str);
  if (olen > 0) {
    for (size_t i = 0; i < olen; i++) {
      if (!isWhitespace(str[i])) break;
      replaced++;
    }
    memmove(str, str + replaced, olen - replaced + 1);
    size_t end = strlen(str) - 1;
    for (size_t i = end; i >= 0; i--) {
      if (!isWhitespace(str[i])) break;
      replaced++;
      str[i] = '\0';
    }
    AR_LOGV("Trimmed %d: %s", replaced, debugString(str).c_str());
  }
}

// TODO: possible buffer problem for large strings
void trim(String &str) {
  str.trim();
}

long getNextParameter(char* at_param, const char* response,
                      size_t buffer_size, const char sep) {
  if (strlen(response) == 0)
    return -1;
  int extra_param_count = instancesOf(response, sep);
  size_t param_end = extra_param_count > 0 ? indexOf(response, sep) : 0;
  if (indexOf(response, sep) == 0) {
    at_param[0] = '\0';
  } else {
    substring(at_param, response, 0, param_end);
  }
  return (strlen(at_param) + (extra_param_count > 0 ? 1 : 0));
}

void uintToChar(uint32_t n, char *result, size_t result_size) {
  uint32_t m = n;
  uint32_t digit = 0;
  while (m > 0) {
    digit++;
    m /= 10;
  }
  memset(result, 0, result_size);
  char arr[digit + 1];
  uint32_t index = 0;
  if (n > 0) {
    while (n > 0) {
      index++;
      arr[index] = static_cast<char>(n % 10 + 48);
      n /= 10;
    }
    for (int i = 0; i < index; i++) {
      result[i] = arr[index - i];
    }
  } else {
    result[0] = static_cast<char>(48);
  }
}

// ---------------- HEX / BASE64 / BINARY CONVERSIONS -----------------------

static const char* HEX_CHARSET = "0123456789ABCDEF";

bool isNumber(const char* candidate) {
  size_t str_len = strlen(candidate);
  for (size_t i = 0; i < str_len; i++) {
    if (!std::isdigit(candidate[i]) &&
        candidate[i] != '.' &&
        candidate[i] != '-' &&
        candidate[i] != ' ')
      return false;
  }
  return true;
}

bool isBinary(const char* candidate) {
  size_t str_len = strlen(candidate);
  for (size_t i = 0; i < str_len; i++) {
    if (candidate[i] != '1' &&
        candidate[i] != '0' &&
        candidate[i] != ' ')
      return false;
  }
  return true;
}

uint32_t binToInt(const char* bin_str) {
  uint32_t value = 0;
  if (isBinary(bin_str)) {
    size_t len = strlen(bin_str);
    for (int i = 0; i < len; i++) {
      value *= 2;
      if (bin_str[i] == '1') value++;
    }
  } else {
    AR_LOGE("Invalid binary string %s", bin_str);
  }
  return value;
}

bool isHex(const char* candidate) {
  if ((strlen(candidate) % 2) != 0)
    return false;
  size_t len = strlen(candidate);
  for (size_t i = 0; i < len; i++) {
    if (!std::isxdigit(static_cast<unsigned char>(candidate[i])))
      return false;
  }
  return true;
}

void intToHex(char* hex_string, int value, uint8_t width, size_t buffer_size) {
  memset(hex_string, 0, buffer_size);
  for (size_t i=0, j=(width-1)*4; i < width; ++i, j-=4) {
    hex_string[i] = HEX_CHARSET[(value >> j) & 0xF];
  }
}

void intToHex(String& hex_string, int value, uint8_t width) {
  hex_string = "";
  for (size_t i=0, j=(width-1)*4; i < width; ++i, j-=4) {
    hex_string += HEX_CHARSET[(value >> j) & 0xF];
  }
}

uint32_t hexToInt(const char* hex_value) {
  return std::stoul(hex_value, nullptr, 16);
}

uint32_t hexToInt(const String& hex_value) {
  return std::stoul(hex_value.c_str(), nullptr, 16);
}

static const char* B64_CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz0123456789+/";

void base64Encode(char* b64_str, const uint8_t* buffer, size_t buffer_size) {
  int i = 0;
  int j = 0;
  uint8_t char_array_3[3];
  uint8_t char_array_4[4];
  size_t out_idx = 0;
  while (buffer_size--) {
    char_array_3[i++] = *(buffer++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      for(i = 0; (i <4) ; i++) {
        b64_str[out_idx++] = B64_CHARSET[char_array_4[i]];
      }
      i = 0;
    }
  }
  if (i) {
    for(j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;
    for (j = 0; (j < i + 1); j++) {
      b64_str[out_idx++] = B64_CHARSET[char_array_4[j]];
    }
    while((i++ < 3)) {
      b64_str[out_idx++] = '=';
    }
  }
  b64_str[out_idx] = '\0';
}

void base64Encode(String& b64_str, const uint8_t* buffer, size_t buffer_size) {
  int i = 0;
  int j = 0;
  uint8_t char_array_3[3];
  uint8_t char_array_4[4];
  // size_t out_idx = 0;
  while (buffer_size--) {
    char_array_3[i++] = *(buffer++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      for(i = 0; (i <4) ; i++) {
        b64_str += B64_CHARSET[char_array_4[i]];
      }
      i = 0;
    }
  }
  if (i) {
    for(j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;
    for (j = 0; (j < i + 1); j++) {
      b64_str += B64_CHARSET[char_array_4[j]];
    }
    while((i++ < 3)) {
      b64_str += '=';
    }
  }
  AR_LOGV("Result: %s", b64_str);
}

static inline bool isBase64(const char c) {
  return (isalnum(c) || (c == '+' || c == '/'));
}

static size_t lookupB64(const char c) {
  const char *candidate = B64_CHARSET;
  size_t idx = 0;
  while (*candidate) {
    if (*candidate++ != c) {
      idx++;
    } else {
      return idx;
    }
  }
  return strlen(B64_CHARSET);
}

void base64Decode(char* buffer, const char* b64_str) {
  int in_len = strlen(b64_str);
  int i = 0;
  int j = 0;
  int in_ = 0;
  char char_array_4[4];
  char char_array_3[3];
  size_t output_idx = 0;
  while (in_len-- && b64_str[in_] != '=' && isBase64(b64_str[in_])) {
    char_array_4[i++] = b64_str[in_];
    in_++;
    if (i == 4) {
      for (i = 0; i < 4; i++) {
        char_array_4[i] = lookupB64(char_array_4[i]);
      }
      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
      for (i = 0; (i < 3); i++) {
          buffer[output_idx++] = char_array_3[i];
      }
      i = 0;
    }
  }
  if (i) {
    for (j = i; j <4; j++) {
      char_array_4[j] = 0;
    }
    for (j = 0; j <4; j++) {
      char_array_4[j] = lookupB64(char_array_4[j]);
    }
    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
    for (j = 0; (j < i - 1); j++) {
      buffer[output_idx++] = char_array_3[j];
    }
  }
}

size_t base64BufferLength(const char* b64_str) {
  size_t str_len = strlen(b64_str);
  size_t buf_len = str_len / 4 * 3;
  for (size_t r_idx = 1; r_idx <= 2; r_idx++) {
    if (b64_str[str_len - r_idx] == '=')
      buf_len--;
  }
  return buf_len;
}

size_t base64StringLength(size_t buffer_size) {
  return ((4 * buffer_size / 3) + 3) & ~3;
}

}   // namespace at
#include <atstringutils.h>
#include <unity.h>

void test_printableChar_ascii() {
  char c = 'A';
  TEST_ASSERT_TRUE(at::printableChar(c));
}

void test_printableChar_null() {
  char c = 0;
  TEST_ASSERT_FALSE(at::printableChar(c));
}

void test_printableChar_crlf() {
  char c = '\r';
  TEST_ASSERT_TRUE(at::printableChar(c));
  c = '\n';
  TEST_ASSERT_TRUE(at::printableChar(c));
}

void test_includes_str() {
  String test_str = "test string";
  String test_true = "test";
  String test_false = "strings";
  TEST_ASSERT_TRUE(at::includes(test_str, test_true));
  TEST_ASSERT_FALSE(at::includes(test_str, test_false));
}

void test_includes_cstr() {
  const char* test_str = "test string";
  const char* test_true = "test";
  const char* test_false = "strings";
  TEST_ASSERT_TRUE(at::includes(test_str, test_true));
  TEST_ASSERT_FALSE(at::includes(test_str, test_false));
}

void test_indexOf_cstr() {
  char test_str[] = "test string";
  const char pattern[] = "e";
  int expected = 1;
  TEST_ASSERT_EQUAL(expected, at::indexOf(test_str, pattern));
  const char pattern2[] = "E";
  expected = -1;
  TEST_ASSERT_EQUAL(expected, at::indexOf(test_str, pattern2));
  char c = 'e';
  expected = 1;
  TEST_ASSERT_EQUAL(expected, at::indexOf(test_str, c));
}

void test_startsWith_cstr() {
  const char* test_str = "test string";
  const char* test_true = "test ";
  const char* test_false = "string";
  TEST_ASSERT_TRUE(at::startsWith(test_str, test_true));
  TEST_ASSERT_FALSE(at::startsWith(test_str, test_false));
}

void test_endsWith_cstr() {
  const char* test_str = "test string";
  const char* test_true = " string";
  const char* test_false = "t";
  TEST_ASSERT_TRUE(at::endsWith(test_str, test_true));
  TEST_ASSERT_FALSE(at::endsWith(test_str, test_false));
}

void test_substring_cstr() {
  char target[32];
  char original[] = "test string";
  uint8_t start = 0;
  uint8_t end = 4;
  char expected[] = "test";
  at::substring(target, original, start, end);
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, target, strlen(expected));
  #endif
}

void test_substring_cstr_to_end() {
  char target[32];
  char original[] = "test string";
  uint8_t start = 5;
  char expected[] = "string";
  at::substring(target, original, start);
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, target, strlen(expected));
  #endif
}

void test_replace_cstr() {
  const int buffersize = 64;
  // char cstr[buffersize] = "test\r\nstring\r\none";
  // char to_replace[] = "\r\n";
  // char replace_with[] = " ";
  // char expected[] = "test string one";
  char cstr[buffersize] = "\r\n+GSN: 00000000SKYEE3D\r\n\r\nOK\r\n";
  char to_replace[] = "\r\nOK\r\n";
  char replace_with[] = "";
  char expected[] = "\r\n+GSN: 00000000SKYEE3D\r\n";
  at::replace(cstr, to_replace, replace_with, buffersize);
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, cstr, strlen(expected));
  #else
  LOG_INFO("\r\n  Expected:", at::debugString(expected), "\r\n  Got:", at::debugString(cstr));
  #endif
}

void test_remove_cstr() {
  char test_cstr[] = "test string";
  char expected[] = "test";
  at::remove(test_cstr, 4);
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, test_cstr, 4);
  #endif
}

void test_trim_cstr() {
  char test_cstr[32] = "\r\ntest string \r\n";
  char expected[] = "test string";
  at::trim(test_cstr, 32);
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, test_cstr, 4);
  #endif
}

void test_intToHex_cstr() {
  int val = 255;
  char result[4+1];
  at::intToHex(result, val, 4, 4+1);
  char expected[] = "00FF";
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, result, 4);
  #endif
}

void test_hexToInt_cstr() {
  char test_cstr[] = "00FF";
  int expected = 255;
  TEST_ASSERT_EQUAL(expected, at::hexToInt(test_cstr));
  char test_cstr2[] = "86C5";
  int expected2 = 34501;
  TEST_ASSERT_EQUAL(expected2, at::hexToInt(test_cstr2));
}

void test_base64Encode() {
  unsigned char data[] = { 1, 2, 3, 4 };
  size_t bufferlen = sizeof(data) / sizeof(data[0]);
  size_t b64_len = at::base64StringLength(bufferlen) + 1;
  char result[b64_len];
  at::base64Encode(result, data, bufferlen);
  char expected[] = "AQIDBA==";
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, result, strlen(expected));
  #endif
}

void test_base64BufferLength() {
  char b64_cstr[] = "AQIDBA==";
  TEST_ASSERT_EQUAL(4, at::base64BufferLength(b64_cstr));
}

void test_base64Decode() {
  char b64_cstr[] = "AQIDBA==";
  char result[4];
  at::base64Decode(result, b64_cstr);
  char expected[] = { 1, 2, 3, 4 };
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, result, 4);
  #endif
}

void test_getNextParameter() {
  char test_response[] = "param1,parameter2";
  char* p_resp = test_response;
  char result[64];
  long offset = at::getNextParameter(result, p_resp, 64);
  TEST_ASSERT_EQUAL_STRING("param1", result);
  TEST_ASSERT_EQUAL(7, offset);
  p_resp += offset;
  offset = at::getNextParameter(result, p_resp, 64);
  TEST_ASSERT_EQUAL_STRING("parameter2", result);
  TEST_ASSERT_EQUAL(10, offset);
  p_resp += offset;
  offset = at::getNextParameter(result, p_resp, 64);
  TEST_ASSERT_EQUAL(-1, offset);
  strncpy(test_response, "0,0", 10);
  p_resp = test_response;
  offset = at::getNextParameter(result, p_resp, 64);
  TEST_ASSERT_EQUAL_STRING("0", result);
  TEST_ASSERT_EQUAL(2, offset);
  p_resp += offset;
  offset = at::getNextParameter(result, p_resp, 64);
  TEST_ASSERT_EQUAL_STRING("0", result);
  TEST_ASSERT_EQUAL(-1, offset);
}

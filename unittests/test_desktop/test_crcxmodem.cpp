#include <crcxmodem.h>
#include <unity.h>

void test_applyCrc_cstr() {
  char cstr[32] = "AT%CRC=0";
  at::applyCrc(cstr, 32);
  char expected[] = "AT%CRC=0*BBEB";
  #if defined TEST_ASSERT_EQUAL_CHAR_ARRAY
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, cstr, strlen(expected));
  #endif
}

void test_validateCrc_cstr() {
  char at_response[] = "\r\nOK\r\n*86C5\r\n";
  TEST_ASSERT_TRUE(at::validateCrc(at_response));
}

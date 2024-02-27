#include <unity.h>
#include "../unittests/test_desktop/test_atstringutils.cpp"
#include "../unittests/test_desktop/test_crcxmodem.cpp"

int main(int argc, char** argv) {
  UNITY_BEGIN();

  /* atstringutils */
  RUN_TEST(test_printableChar_ascii);
  RUN_TEST(test_printableChar_null);
  RUN_TEST(test_printableChar_crlf);
  RUN_TEST(test_includes_str);
  RUN_TEST(test_includes_cstr);
  RUN_TEST(test_startsWith_cstr);
  RUN_TEST(test_endsWith_cstr);
  RUN_TEST(test_replace_cstr);
  RUN_TEST(test_remove_cstr);
  RUN_TEST(test_substring_cstr);
  RUN_TEST(test_substring_cstr_to_end);
  RUN_TEST(test_trim_cstr);
  RUN_TEST(test_intToHex_cstr);
  RUN_TEST(test_hexToInt_cstr);
  RUN_TEST(test_base64Encode);
  RUN_TEST(test_base64BufferLength);
  RUN_TEST(test_base64Decode);
  RUN_TEST(test_indexOf_cstr);
  RUN_TEST(test_getNextParameter);

  /* crcxmodem */
  RUN_TEST(test_applyCrc_cstr);
  RUN_TEST(test_validateCrc_cstr);
  
  UNITY_END();
  return 0;
}
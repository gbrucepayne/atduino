#ifndef AT_DEBUG_H
#define AT_DEBUG_H

#ifdef ARDUINO
  #include <DebugLog.h>
#else
  #define PRINT(...)
  #define PRINTLN(...)
  #define LOG_ERROR(...)
  #define LOG_WARN(...)
  #define LOG_INFO(...)
  #define LOG_DEBUG(...)
  #define LOG_TRACE(...)
  #define LOG_GET_LEVEL()
#endif

#endif   // AT_DEBUG_H

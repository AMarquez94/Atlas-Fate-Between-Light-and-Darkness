#ifndef INC_UTILS_UTILS_H_
#define INC_UTILS_UTILS_H_

// -----------------------------------------
#include "murmur3/murmur3.h"
uint32_t getID(const char* txt);
uint32_t getID(const void* buff, size_t nbytes);

// -----------------------------------------

//#ifdef _DEBUG
void dbg(const char* fmt, ...);
//#else
//#define dbg(...)
//#endif

bool fatal(const char* fmt, ...);

bool isPressed(int key);

json loadJson(const std::string& filename);

#endif


#ifndef INC_UTILS_UTILS_H_
#define INC_UTILS_UTILS_H_

// -----------------------------------------
#include "murmur3/murmur3.h"
#include "PxPhysicsAPI.h"
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
bool isPressedWithOrWithoutFocus(int key);

bool fileExists(const std::string& afilename);
json loadJson(const std::string& filename);
bool fileExists(const std::string& afilename);

template <typename T>
T Clamp(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

float urand(float loVal, float hiVal);

void ToUpperCase(std::string&);

int Strnicmp(const char* str1, const char* str2, int n);

physx::PxVec3 ToPxVec3(VEC3);
VEC3 ToVec3(physx::PxVec3);
physx::PxQuat ToPxQuat(QUAT);
QUAT ToQuat(physx::PxQuat);

void getFileNameFromPath(std::string&);

#endif


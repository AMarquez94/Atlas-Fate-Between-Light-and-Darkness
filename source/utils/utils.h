#ifndef INC_UTILS_UTILS_H_
#define INC_UTILS_UTILS_H_

// -----------------------------------------
#include "murmur3/murmur3.h"
#define E_ROUND(f) (f * 100) / 100

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

void ToUpperCase(std::string&);
float urand(float loVal, float hiVal);
int Strnicmp(const char* str1, const char* str2, int n);
const char *convert(const std::string & s);
std::string stringify(VEC2 pos);
std::string stringify(VEC3 pos);
std::string stringify(VEC4 pos);

float mapInRange(float result_from, float result_to, float current_from, float current_to, float value);

bool isPointInRectangle(VEC2 point, VEC2 rectangle_size, VEC2 rectangle_position);

VEC2 getMouseInRange(float min_x, float max_x, float min_y, float max_y);
//float lerpAngle(float a, float b, float t);

#endif


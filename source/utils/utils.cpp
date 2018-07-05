#include "mcv_platform.h"
#include "windows/app.h"
using namespace physx;

void dbg(const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  char dest[1024 * 16];
  _vsnprintf(dest, sizeof(dest), format, argptr);
  va_end(argptr);
  ::OutputDebugString(dest);
}

bool fatal(const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  char dest[1024 * 16];
  _vsnprintf(dest, sizeof(dest), format, argptr);
  va_end(argptr);
  ::OutputDebugString(dest);
  MessageBox(nullptr, dest, "Error!", MB_OK);
  return false;
}

bool isPressed(int key) {
  return CApp::get().hasFocus() && (::GetAsyncKeyState(key) & 0x8000) == 0x8000;
}

bool isPressedWithOrWithoutFocus(int key) {
	return (::GetAsyncKeyState(key) & 0x8000) == 0x8000;
}

// --------------------------------------------------------
// Quick and dirty..
bool fileExists(const std::string& afilename) {
	FILE * f = fopen(afilename.c_str(), "rb");
	if (!f)
		return false;
	fclose(f);
	return true;
	
}

// --------------------------------------------------------
json loadJson(const std::string& filename) {

	json j;

	while (true) {

		std::ifstream ifs(filename.c_str());
		if (!ifs.is_open()) {
			fatal("Failed to open json file %s\n", filename.c_str());
			return json();
		}


#ifdef NDEBUG

		j = json::parse(ifs, nullptr, false);
		if (j.is_discarded()) {
			fatal("Failed to parse json file %s\n", filename.c_str());
			continue;
		}

#else

		try
		{
			// parsing input with a syntax error
			j = json::parse(ifs);
		}
		catch (json::parse_error& e)
		{
			// output exception information
			fatal("Failed to parse json file %s\n%s\nAt offset: %d"
				, filename.c_str(), e.what(), e.byte);
			continue;
		}


#endif
		// The json is correct, we can leave the while loop
		break;
	}

	return j;
}

// generate a hash from the input buffer
uint32_t getID(const void* buff, size_t nbytes) {
	uint32_t seed = 0;
	uint32_t out_value = 0;
	MurmurHash3_x86_32(buff, (uint32_t)nbytes, seed, &out_value);
	assert(out_value != 0);
	return out_value;
}

uint32_t getID(const char* txt) {
	return getID(txt, strlen(txt));
}

int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
{
	int i, j, c = 0;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((verty[i]>testy) != (verty[j]>testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
			c = !c;
	}
	return c;
}

float urand(float loVal, float hiVal)
{
	return loVal + (float(rand()) / RAND_MAX)*(hiVal - loVal);
}

void ToUpperCase(std::string& string) {
	for (int i = 0; i < string.size(); i++) {
		string.at(i) = toupper(string.at(i));
	}
}

int Strnicmp(const char * str1, const char * str2, int n)
{
	int d = 0; 
	while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { 
		str1++; str2++; n--; 
	} 
	return d;
}

const char *convert(const std::string & s)
{
    return s.c_str();
}

std::string stringify(VEC2 pos) {

    return std::to_string(E_ROUND(pos.x)) + " " +
    std::to_string(E_ROUND(pos.y));
}


std::string stringify(VEC3 pos) {

    return std::to_string(E_ROUND(pos.x)) + " " +
    std::to_string(E_ROUND(pos.y)) + " " +
    std::to_string(E_ROUND(pos.z));
}

std::string stringify(VEC4 pos) {

    return std::to_string(E_ROUND(pos.x)) + " " +
    std::to_string(E_ROUND(pos.y)) + " " +
    std::to_string(E_ROUND(pos.z)) + " " + 
    std::to_string(E_ROUND(pos.w));
}

PxVec3 ToPxVec3(VEC3 vector) {
	PxVec3 result = PxVec3(vector.x, vector.y, vector.z);
	return result;
}

VEC3 ToVec3(PxVec3 vector) {
	VEC3 result = VEC3(vector.x, vector.y, vector.z);
	return result;
}

PxQuat ToPxQuat(QUAT quaternion) {
	physx::PxQuat q = PxQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
	return q;
}

QUAT ToPxQuat(PxQuat quaternion) {
	QUAT q = QUAT(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
	return q;
}


void getFileNameFromPath(std::string& nameWithPath) {
	size_t startPos = nameWithPath.find_last_of("/\\") + 1;
	size_t endPos = nameWithPath.find_last_of(".");
	nameWithPath = nameWithPath.substr(startPos, endPos - startPos);
}
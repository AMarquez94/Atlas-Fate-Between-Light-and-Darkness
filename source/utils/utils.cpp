#include "mcv_platform.h"

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
  return(::GetAsyncKeyState(key) & 0x8000) == 0x8000;
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

		// The json is correct, we can leave the while loop
		break;

#endif
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
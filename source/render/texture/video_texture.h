#include "mcv_platform.h"

extern "C" {
#include "h264bsd/h264bsd_decoder.h"
}

class CVideoTexture : public CTexture {

  storage_t *decoder = NULL;
  u32 status = H264BSD_RDY;

public:
  bool open(const char* filename);
  void close();
  void update();
};


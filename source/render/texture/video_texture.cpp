#include "mcv_platform.h"
#include "video_texture.h"
#include "utils/data_provider.h"


bool CVideoTexture::open( const char* filename ) {

  CFileDataProvider dp(filename);
  if (!dp.isValid())
    return false;

  decoder = h264bsdAlloc();
  status = h264bsdInit(decoder, 0);
  if (status > 0) return false;

  return true;
}

void CVideoTexture::update() {

  u8* byteStrm = fileData;
  status = h264bsdDecode(decoder, byteStrm, len, 0, &bytesRead);

}



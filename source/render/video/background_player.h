#pragma once

class CVideoTexture;

void addVideoTextureToBackgroundPlayer(CVideoTexture* vt);
void deleteVideoTextureForBackgroundPlayer(CVideoTexture* vt);
void resetVideoFromBackgroundPlayer(CVideoTexture* vt);
void uploadAllVideoTexturesReady();



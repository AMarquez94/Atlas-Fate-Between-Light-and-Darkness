#include "mcv_platform.h"
#include "background_player.h"
#include "utils/semaphore.h"
#include "video_texture.h"

static std::thread* decoder_thread = nullptr;
static Semaphore    sem_can_decode(1);
static Semaphore    sem_can_upload;
static std::vector< CVideoTexture* > playing_video_textures;

static void decodeVideoTextures() {

    CTimer tm;
    float desired_fps = 30.f;

    while (true) {

        for (auto t : playing_video_textures)
            t->update(0.f);   // 0.f is not correct either
        sem_can_upload.notify();

        float elapsed = tm.elapsedAndReset();
        float remaining_time = (1.0f / desired_fps) - elapsed;
        if (remaining_time > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(2 * (int)(remaining_time * 1000)));
    }
}

void addVideoTextureToBackgroundPlayer(CVideoTexture* vt) {

    playing_video_textures.push_back(vt);
    // With the first video texture, we start the thread
    if (decoder_thread == nullptr)
        decoder_thread = new std::thread(decodeVideoTextures);
}

// Called from the main thread...
void uploadAllVideoTexturesReady() {

    PROFILE_FUNCTION("uploadAllVideoTexturesReady");
    // We never wait...
    if (sem_can_upload.try_wait()) {
        for (auto t : playing_video_textures) {
            if (t->isFrameReadyToUpload())
                t->uploadToVRAM();
        }
        sem_can_decode.notify();
    }
}
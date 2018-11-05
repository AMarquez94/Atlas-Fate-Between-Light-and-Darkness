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
    CTimer tm2;
    float desired_frame_fps = 1.0f / 25.0f;
    dbg("total fps %f ", desired_frame_fps);

    while (true) {

        //std::this_thread::sleep_for(std::chrono::milliseconds(30));
        tm.elapsedAndReset();
        sem_can_decode.wait();
        //tm2.reset();
        for (auto t : playing_video_textures)
            t->update(0.f);   // 0.f is not correct either
        //float utime = tm2.elapsedAndReset();
        //dbg("total time update video %f\n", utime);
        sem_can_upload.notify();

        float elapsed = tm.elapsedAndReset();
        float remaining_time = (desired_frame_fps - elapsed);
        //dbg("total remaining time %f\n", remaining_time);

        if (remaining_time > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(remaining_time * 1000)));
    }
}

void resetPlayingVideos() {
    for (auto t : playing_video_textures)
        t->setToBeReset(true);
}

void addVideoTextureToBackgroundPlayer(CVideoTexture* vt) {

    playing_video_textures.push_back(vt);
    // With the first video texture, we start the thread
    if (decoder_thread == nullptr)
        decoder_thread = new std::thread(decodeVideoTextures);
}

void deleteVideoTextureForBackgroundPlayer(CVideoTexture * vt)
{
    sem_can_upload.wait();
    int i = 0;
    bool found = false;
    for (i = 0; i < playing_video_textures.size(); i++) {
        found = playing_video_textures[i] == vt;
        playing_video_textures.erase(playing_video_textures.begin() + i);
    }
    sem_can_upload.notify();
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
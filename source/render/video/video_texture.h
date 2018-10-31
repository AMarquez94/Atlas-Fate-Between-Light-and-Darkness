#include "mcv_platform.h"

extern "C" {
#include "h264bsd/h264bsd_decoder.h"
}

class CVideoTexture : public CTexture {

    storage_t *decoder = nullptr;
    std::vector<char> video_data;
    std::vector< u8 > streaming_data;
    u32 len = 0;
    u32 status = H264BSD_RDY;
    u8* byteStrm = nullptr;
    u32 num_frame = 0;
    u32* picData = nullptr;
    bool finished = false;
    bool frame_ready = false;
    CTimer chrono;
    bool to_be_reset = false;

    void close();
    bool initDecoder();
    bool decodeNextFrame();
    bool hasFinished() const {
        return finished;
    }

    static std::vector< CVideoTexture* > playing_video_textures;

public:

    bool active = false;

    ~CVideoTexture();

    bool create(const std::string& name) override;
    void destroy() override;
    void update(float dt);
    bool isFrameReadyToUpload() const;
    bool uploadToVRAM();

    void resetVideo();
    void setActive(bool state);
    void setToBeReset(bool toBeReset);
};


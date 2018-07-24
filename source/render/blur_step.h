#ifndef INC_RENDER_BLUR_STEP_H_
#define INC_RENDER_BLUR_STEP_H_

class CRenderToTexture;
class CRenderTechnique;
class CRenderMesh;
class CTexture;

struct CBlurStep {

    CRenderToTexture* rt_half_y = nullptr;
    CRenderToTexture* rt_output = nullptr;    // half x & y

    int   xres = 0, yres = 0;
    const CRenderTechnique* tech = nullptr;
    const CRenderMesh*      mesh = nullptr;

    // ---------------------
    bool create(const char* name, int in_xres, int in_yres);
    void applyBlur(float dx, float dy);
    CTexture* apply(CTexture* input, float global_distance, VEC4 distances, VEC4 weights);
    CTexture* applyHalf(CTexture* input, float global_distance, VEC4 distances, VEC4 weights);
};

#endif

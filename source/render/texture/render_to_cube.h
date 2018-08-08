#ifndef INC_RENDER_TO_CUBE_H_
#define INC_RENDER_TO_CUBE_H_

#include "texture.h"
#include "camera/camera.h"

class CRenderToCube : public CTexture {
    int                         res;
    DXGI_FORMAT                 color_fmt;
    DXGI_FORMAT                 depth_fmt;
    static const int nsides = 6;

    CCamera                     camera;
    ID3D11RenderTargetView*     render_target_views[nsides];
    ID3D11DepthStencilView*     depth_stencil_view = nullptr;
    CTexture*                   ztexture = nullptr;

    friend class CDeferredRender;

    bool createColorBuffer();
    bool createDepthBuffer();

public:

    CRenderToCube();
    bool create(const char* name, int res, DXGI_FORMAT color_fmt, DXGI_FORMAT depth_fmt);
    void destroy();
    void activateFace(int face, CCamera* camera);
    void activateViewport();
    void clearColorBuffer(int face, const FLOAT ColorRGBA[4]);
    void clearDepthBuffer();
    void getCamera(int face, CCamera* camera_to_use) const;

    void setPosition(VEC3 new_pos);
    int  getNSides() const { return nsides; }

    CTexture* getZTexture() { return ztexture; }
    bool isValid() const { return render_target_views[0] != nullptr; }
    void debugInMenu() override;
};

#endif

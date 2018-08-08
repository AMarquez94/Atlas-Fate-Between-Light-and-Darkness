#include "mcv_platform.h"
#include "render_to_cube.h"
#include "render/render_utils.h"

// http://richardssoftware.net/Home/Post/26

CRenderToCube::CRenderToCube()
    : res(0)
    , color_fmt(DXGI_FORMAT_UNKNOWN)
    , depth_fmt(DXGI_FORMAT_UNKNOWN)
    , depth_stencil_view(nullptr)
    , ztexture(nullptr)
{
    for (int i = 0; i < nsides; ++i)
        render_target_views[i] = nullptr;
}

// ------------------------------------------------
bool CRenderToCube::create(
    const char* aname
    , int ares
    , DXGI_FORMAT acolor_fmt
    , DXGI_FORMAT adepth_fmt
) {

    setNameAndClass(aname, getResourceClassOf<CTexture>());

    // Save the params
    res = ares;
    color_fmt = acolor_fmt;
    depth_fmt = adepth_fmt;
    xres = yres = res;

    // Use the name for the debug name
    if (color_fmt != DXGI_FORMAT_UNKNOWN)
        if (!createColorBuffer())
            return false;

    // Create ZBuffer
    if (depth_fmt != DXGI_FORMAT_UNKNOWN) {
        if (!createDepthBuffer())
            return false;
    }
    else
        depth_stencil_view = nullptr;

    Resources.registerResource(this);

    camera.setPerspective(deg2rad(90.f), 1.0f, 1000);
    camera.setViewport(0, 0, res, res);

    return true;
}

// ------------------------------------------------
void CRenderToCube::debugInMenu() {

    ImGui::Text("Cube %dx%d", res, res);
}

// ------------------------------------------------
bool CRenderToCube::createColorBuffer() {

    // Create a color surface
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = res;
    desc.Height = res;
    desc.MipLevels = 1;
    desc.ArraySize = nsides;
    desc.Format = color_fmt;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    ID3D11Texture2D* texture2d = nullptr;
    HRESULT hr = Render.device->CreateTexture2D(&desc, NULL, &texture2d);
    if (FAILED(hr))
        return false;
    texture = texture2d;

    // Create 6 render target views
    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
    rtv_desc.Format = color_fmt;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    rtv_desc.Texture2DArray.FirstArraySlice = 0;
    rtv_desc.Texture2DArray.ArraySize = 1;
    rtv_desc.Texture2DArray.MipSlice = 0;
    for (int i = 0; i < nsides; ++i) {
        rtv_desc.Texture2DArray.FirstArraySlice = i;
        hr = Render.device->CreateRenderTargetView(texture, &rtv_desc, &render_target_views[i]);
        if (FAILED(hr))
            return false;
        char side_name[64];
        sprintf(side_name, "%s.%d", getName().c_str(), i);
        setDXName(render_target_views[i], side_name);
    }

    // Create a resource view so we can use it in the shaders as input
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    memset(&SRVDesc, 0, sizeof(SRVDesc));
    SRVDesc.Format = desc.Format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    SRVDesc.TextureCube.MipLevels = desc.MipLevels;
    SRVDesc.TextureCube.MostDetailedMip = 0;
    hr = Render.device->CreateShaderResourceView(texture, &SRVDesc, &shader_resource_view);
    if (FAILED(hr))
        return false;
    setDXName(shader_resource_view, getName().c_str());

    return true;
}

// ------------------------------------------------
void CRenderToCube::destroy() {

    for (int i = 0; i < nsides; ++i)
        SAFE_RELEASE(render_target_views[i]);
    SAFE_RELEASE(depth_stencil_view);
    CTexture::destroy();
}

// ------------------------------------------------
bool CRenderToCube::createDepthBuffer() {

    auto device = Render.device;
    //assert(ztexture.resource_view == NULL);
    assert(depth_fmt != DXGI_FORMAT_UNKNOWN);
    assert(depth_fmt == DXGI_FORMAT_R32_TYPELESS
        || depth_fmt == DXGI_FORMAT_R24G8_TYPELESS
        || depth_fmt == DXGI_FORMAT_R16_TYPELESS
        || depth_fmt == DXGI_FORMAT_D24_UNORM_S8_UINT
        || depth_fmt == DXGI_FORMAT_R8_TYPELESS);

    D3D11_TEXTURE2D_DESC          depthBufferDesc;

    // Init depth and stencil buffer
    // Initialize the description of the depth buffer.
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // Set up the description of the depth buffer.
    depthBufferDesc.Width = res;
    depthBufferDesc.Height = res;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = depth_fmt;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    depthBufferDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

    DXGI_FORMAT texturefmt = DXGI_FORMAT_R32_TYPELESS;
    DXGI_FORMAT SRVfmt = DXGI_FORMAT_R32_FLOAT;       // Stencil format
    DXGI_FORMAT DSVfmt = DXGI_FORMAT_D32_FLOAT;       // Depth format

    switch (depth_fmt) {
    case DXGI_FORMAT_R32_TYPELESS:
        SRVfmt = DXGI_FORMAT_R32_FLOAT;
        DSVfmt = DXGI_FORMAT_D32_FLOAT;
        break;
    case DXGI_FORMAT_R24G8_TYPELESS:
        SRVfmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        DSVfmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
    case DXGI_FORMAT_R16_TYPELESS:
        SRVfmt = DXGI_FORMAT_R16_UNORM;
        DSVfmt = DXGI_FORMAT_D16_UNORM;
        break;
    case DXGI_FORMAT_R8_TYPELESS:
        SRVfmt = DXGI_FORMAT_R8_UNORM;
        DSVfmt = DXGI_FORMAT_R8_UNORM;
        break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        SRVfmt = depthBufferDesc.Format;
        DSVfmt = depthBufferDesc.Format;
        break;
    }

    // Create the texture for the de  pth buffer using the filled out description.
    ID3D11Texture2D* ztexture2d;
    HRESULT hr = device->CreateTexture2D(&depthBufferDesc, NULL, &ztexture2d);
    if (FAILED(hr))
        return false;

    // Initialize the depth stencil view.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DSVfmt;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    hr = device->CreateDepthStencilView(ztexture2d, &depthStencilViewDesc, &depth_stencil_view);
    if (FAILED(hr))
        return false;
    setDXName(depth_stencil_view, "ZTextureDSV");

    // Setup the description of the shader resource view.
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = SRVfmt;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = depthBufferDesc.MipLevels;

    // Create the shader resource view.
    ID3D11ShaderResourceView* depth_resource_view = nullptr;
    hr = device->CreateShaderResourceView(ztexture2d, &shaderResourceViewDesc, &depth_resource_view);
    if (FAILED(hr))
        return false;

    ztexture = new CTexture();
    ztexture->setDXParams(res, res, ztexture2d, depth_resource_view);
    ztexture->setNameAndClass(("Z" + getName()).c_str(), getResourceClassOf<CTexture>());
    //  texture_manager.registerNew(ztexture->getName(), ztexture);
    const char* zname = ztexture->getName().c_str();
    setDXName(depth_resource_view, zname);

    SAFE_RELEASE(ztexture2d);
    return true;
}

void CRenderToCube::setPosition(VEC3 new_pos) {

    camera.lookAt(new_pos, new_pos + VEC3(0, 0, 1), VEC3(0, 1, 0));
}

void CRenderToCube::getCamera(int face, CCamera* camera_to_use) const {

    static VEC3 fronts[nsides] = {
      VEC3(+1, 0, 0),
      VEC3(-1, 0, 0),
      VEC3(0, +1, 0),
      VEC3(0, -1, 0),
      VEC3(0, 0, +1),
      VEC3(0, 0, -1),
    };
    static VEC3 ups[nsides]{
      VEC3(0, 1, 0),
      VEC3(0, 1, 0),
      VEC3(0, 0, -1),
      VEC3(0, 0, 1),
      VEC3(0, 1, 0),
      VEC3(0, 1, 0),
    };

    *camera_to_use = camera;
    camera_to_use->lookAt(camera.getPosition()
        , camera.getPosition() + fronts[face]
        , ups[face]
    );
}

void CRenderToCube::activateFace(int face, CCamera* camera_to_use) {

    assert(camera_to_use);
    Render.ctx->OMSetRenderTargets(1, &render_target_views[face], depth_stencil_view);
    activateViewport();
    getCamera(face, camera_to_use);
}

void CRenderToCube::activateViewport() {
    // activate the associated viewport

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)res;
    vp.Height = (FLOAT)res;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    Render.ctx->RSSetViewports(1, &vp);
}

void CRenderToCube::clearColorBuffer(int face, const FLOAT ColorRGBA[4]) {

    assert(render_target_views[face]);
    Render.ctx->ClearRenderTargetView(render_target_views[face], ColorRGBA);
}

void CRenderToCube::clearDepthBuffer() {

    assert(depth_stencil_view);
    Render.ctx->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
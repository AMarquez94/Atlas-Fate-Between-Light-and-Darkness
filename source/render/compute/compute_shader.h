#ifndef INC_RENDER_PIXEL_SHADER_H_
#define INC_RENDER_PIXEL_SHADER_H_

#include <d3d11shader.h>

// -----------------------------------------
class CComputeShader {

    ID3D11ComputeShader*     cs = nullptr;

public:
    std::vector<D3D11_SHADER_INPUT_BIND_DESC> bound_resources;

    bool create(
        const std::string& filename
        , const std::string& fn_entry_name
        , const std::string& profile
    );
    void destroy();
    void activate() const;
    void deactivate() const;
};

#endif

#include "mcv_platform.h"
#include "compute_shader.h"
#include "render/shaders/shader_compiler.h"
#include <d3dcompiler.h>
#include <d3dcompiler.inl>    // D3D11Reflect

FORCEINLINE HRESULT
D3D11Reflect(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData,
    _In_ SIZE_T SrcDataSize,
    _Out_ ID3D11ShaderReflection** ppReflector)
{
    return D3DReflect(pSrcData, SrcDataSize,
        IID_ID3D11ShaderReflection, (void**)ppReflector);
}

// --------------------------------------------------------------------
bool CComputeShader::create(
    const std::string& filename
    , const std::string& fn_entry_name
    , const std::string& profile
) {

    ID3DBlob* pBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(filename.c_str(), fn_entry_name.c_str(), profile.c_str(), &pBlob);
    if (FAILED(hr))
        return false;

    // Create the compute shader
    hr = Render.device->CreateComputeShader(
        pBlob->GetBufferPointer()
        , pBlob->GetBufferSize()
        , nullptr
        , &cs);
    if (FAILED(hr))
    {
        pBlob->Release();
        return false;
    }

    setDXName(cs, (filename + ":" + fn_entry_name).c_str());

    // Query bound resources
    ID3D11ShaderReflection* reflection = NULL;

    D3D11Reflect(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &reflection);
    D3D11_SHADER_DESC desc;
    memset(&desc, 0x00, sizeof(desc));
    reflection->GetDesc(&desc);

    bound_resources.clear();
    for (unsigned int i = 0; i < desc.BoundResources; ++i) {
        D3D11_SHADER_INPUT_BIND_DESC desc;
        HRESULT hr = reflection->GetResourceBindingDesc(i, &desc);
        if (FAILED(hr))
            continue;
        bound_resources.push_back(desc);
    }

    pBlob->Release();

    return true;
}

void CComputeShader::destroy() {
    SAFE_RELEASE(cs);
}

void CComputeShader::activate() const {
    Render.ctx->CSSetShader(cs, nullptr, 0);
}

void CComputeShader::deactivate() const {
    static const int max_slots = 4;
    ID3D11UnorderedAccessView *uavs_null[max_slots] = { nullptr, nullptr, nullptr, nullptr };
    ID3D11ShaderResourceView*  srvs_null[max_slots] = { nullptr, nullptr, nullptr, nullptr };
    // Null all cs params
    Render.ctx->CSSetUnorderedAccessViews(0, max_slots, uavs_null, nullptr);
    Render.ctx->CSSetShaderResources(0, max_slots, srvs_null);
    Render.ctx->CSSetShader(nullptr, nullptr, 0);
}

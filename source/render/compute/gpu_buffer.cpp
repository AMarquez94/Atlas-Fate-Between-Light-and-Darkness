#include "mcv_platform.h"
#include "gpu_buffer.h"

// -------------------------------------------------------------------------------------
void CGPUBuffer::destroy() {
    SAFE_RELEASE(buffer);
    SAFE_RELEASE(srv);
    SAFE_RELEASE(uav);
    bytes_per_elem = 0;
    num_elems = 0;
}

// -------------------------------------------------------------------------------------
bool CGPUBuffer::create(uint32_t new_bytes_per_elem, uint32_t new_num_elems) {

    // Save input params
    bytes_per_elem = new_bytes_per_elem;
    num_elems = (uint32_t)new_num_elems;

    uint32_t total_bytes = bytes_per_elem * num_elems;

    assert(!buffer && !srv);

    // Set flags
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = (UINT)total_bytes;
    bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS
        | D3D11_BIND_SHADER_RESOURCE
        ;

    bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bd.StructureByteStride = bytes_per_elem;

    // Create the buffer in the GPU
    D3D11_SUBRESOURCE_DATA* sd_addr = nullptr;
    HRESULT hr = Render.device->CreateBuffer(&bd, sd_addr, &buffer);
    if (FAILED(hr))
        return false;

    // This is to be able to use it in a shader
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    ZeroMemory(&srv_desc, sizeof(srv_desc));
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srv_desc.BufferEx.FirstElement = 0;
    srv_desc.Format = DXGI_FORMAT_UNKNOWN;
    srv_desc.BufferEx.NumElements = num_elems;
    if (FAILED(Render.device->CreateShaderResourceView(buffer, &srv_desc, &srv)))
        return false;

    // This is to be able to use it as a unordered access buffer
    D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
    ZeroMemory(&uav_desc, sizeof(uav_desc));
    uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uav_desc.Buffer.FirstElement = 0;
    // Format must be must be DXGI_FORMAT_UNKNOWN, when creating 
    // a View of a Structured Buffer
    uav_desc.Format = DXGI_FORMAT_UNKNOWN;
    uav_desc.Buffer.NumElements = num_elems;
    if (FAILED(Render.device->CreateUnorderedAccessView(buffer, &uav_desc, &uav)))
        return false;

    cpu_data.resize(total_bytes);

    return true;
}

// -------------------------------------------------------------------------------------
bool CGPUBuffer::copyGPUtoCPU() const {

    // We need a staging buffer to read the contents of the gpu buffer.
    // Create a new buffer with the same format as the buffer of the gpu buffer, but with the staging & cpu read flags.
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    buffer->GetDesc(&desc);
    UINT byteSize = desc.ByteWidth;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    ID3D11Buffer* cpu_buffer = nullptr;
    if (!FAILED(Render.device->CreateBuffer(&desc, nullptr, &cpu_buffer))) {
        // Copy from the gpu buffer to the cpu buffer.
        Render.ctx->CopyResource(cpu_buffer, buffer);

        assert(cpu_buffer);

        // Request access to read it.
        D3D11_MAPPED_SUBRESOURCE mr;
        if (Render.ctx->Map(cpu_buffer, 0, D3D11_MAP_READ, 0, &mr) == S_OK) {
            memcpy((char*)cpu_data.data(), mr.pData, desc.ByteWidth);
            Render.ctx->Unmap(cpu_buffer, 0);
            return true;
        }

        cpu_buffer->Release();
    }

    return false;
}


// -------------------------------------------------------------------------------------
bool CGPUBuffer::copyCPUtoGPU() const {
    Render.ctx->UpdateSubresource(buffer, 0, nullptr, cpu_data.data(), 0, 0);
    return true;
}
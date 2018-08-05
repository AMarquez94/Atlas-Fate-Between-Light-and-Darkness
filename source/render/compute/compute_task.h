#pragma once

class CComputeShader;
class CGPUBuffer;

// -------------------------------------------------------
class CComputeTask {
    std::string                name;
    CComputeShader*            cs = nullptr;
    std::vector< const CTexture*   > textures;
    std::vector< const CGPUBuffer* > buffers;
public:
    int                        sizes[3] = { 1,1,1 };
    int                        groups[3] = { 4,4,1 };
    void run();
    void bind(const char*, const CGPUBuffer* buf);
    const std::string& getName() const {
        return name;
    }
    bool create(const json& j);
    void destroy();
    void debugInMenu();
};


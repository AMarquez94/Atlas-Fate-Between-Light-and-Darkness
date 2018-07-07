#include "mcv_platform.h"
#include "compute_task.h"
#include "compute_shader.h"
#include "gpu_buffer.h"


// -------------------------------------------------------------------------------------
void CComputeTask::bind(const char* bind_name, const CGPUBuffer* buf) {
    int n = 0;
    for (auto &br : cs->bound_resources) {
        if (strcmp(br.Name, bind_name) == 0) {
            if (n >= buffers.size())
                buffers.resize(n + 1);
            buffers[n] = buf;
            return;
        }
        if (br.Type == D3D_SIT_STRUCTURED || br.Type == D3D_SIT_UAV_RWSTRUCTURED || br.Type == D3D_SIT_CBUFFER)
            ++n;
    }
}

// -------------------------------------------------------------------------------------
void CComputeTask::run() {
    assert(cs);
    // Wrap it inside a debug group
    CTraceScoped gpu_scope(name.c_str());
    cs->activate();
    static bool dump = false;
    if (dump) dbg("Binding of %d resources for cs %s\n", cs->bound_resources.size(), getName().c_str());

    int next_texture = 0;
    int next_buffer = 0;
    int idx = 0;
    for (auto &br : cs->bound_resources) {
        // Textures as ro
        if (br.Type == D3D_SIT_TEXTURE) {
            assert(next_texture < textures.size());
            assert(br.BindCount == 1);
            if (dump) dbg("  Using ro texture %s for slot %s at point %d\n", textures[next_texture]->getName().c_str(), br.Name, br.BindPoint);
            ID3D11ShaderResourceView* srv = (ID3D11ShaderResourceView *)textures[next_texture]->getShaderResourceView();
            Render.ctx->CSSetShaderResources(br.BindPoint, 1, &srv);
            ++next_texture;
        }
        // Textures as rw
        else if (br.Type == D3D_SIT_UAV_RWTYPED) {
            if (dump) dbg("  Using rw texture %s for slot %s at point %d\n", textures[next_texture]->getName().c_str(), br.Name, br.BindPoint);
            assert(br.BindCount == 1);
            assert(next_texture < textures.size());
            assert(textures[next_texture]->uav || fatal("[%d/%d] For bounded resource at slot %d and name %s, can't bind the texture %s as requires the 'compute_output' flag\n", idx, cs->bound_resources.size(), br.BindPoint, br.Name, textures[next_texture]->getName().c_str()));
            Render.ctx->CSSetUnorderedAccessViews(br.BindPoint, 1, &textures[next_texture]->uav, nullptr);
            ++next_texture;
        }
        // Buffers as ro
        else if (br.Type == D3D_SIT_STRUCTURED) {
            if (dump) dbg("  Using ro buffer %s for slot %s at point %d\n", buffers[next_buffer]->name.c_str(), br.Name, br.BindPoint);
            assert(br.BindCount == 1);
            assert(next_buffer < buffers.size());
            assert(buffers[next_buffer]->srv);
            Render.ctx->CSSetShaderResources(br.BindPoint, 1, (ID3D11ShaderResourceView **)&buffers[next_buffer]->srv);
            ++next_buffer;
        }
        // Buffers as rw
        else if (br.Type == D3D_SIT_UAV_RWSTRUCTURED) {
            if (dump) dbg("  Using rw buffer %s for slot %s at point %d\n", buffers[next_buffer]->name.c_str(), br.Name, br.BindPoint);
            assert(br.BindCount == 1);
            assert(next_buffer < buffers.size());
            assert(buffers[next_buffer]->uav);
            Render.ctx->CSSetUnorderedAccessViews(br.BindPoint, 1, &buffers[next_buffer]->uav, nullptr);
            ++next_buffer;
        }
        // Buffers as cte buffer
        else if (br.Type == D3D_SIT_CBUFFER) {
            if (dump) dbg("  Using cbuffer %s for slot %s at point %d\n", buffers[next_buffer]->name.c_str(), br.Name, br.BindPoint);
            assert(br.BindCount == 1);
            assert(next_buffer < buffers.size());
            assert(buffers[next_buffer]->buffer);
            Render.ctx->CSSetConstantBuffers(br.BindPoint, 1, &buffers[next_buffer]->buffer);
            ++next_buffer;
        }
        else {
            fatal("Unsupported bounded buffer. %d\n", br.Type);
        }
        ++idx;
    }

    int thread_group_counts[3] = { groups[0], groups[1], groups[2] };
    int w = sizes[0];
    int h = sizes[1];
    int thread_groups[3] = { w / thread_group_counts[0], h / thread_group_counts[1], 1 };
    Render.ctx->Dispatch(thread_groups[0], thread_groups[1], thread_groups[2]);

    cs->deactivate();
}

void CComputeTask::debugInMenu() {
    ImGui::DragInt3("Sizes", sizes);
    ImGui::DragInt3("Groups", groups);
}

bool CComputeTask::create(const json& j) {
    std::string cs_profile = j.value("profile", "cs_5_0");
    std::string cs_entry_point = j.value("cs_entry_point", "");
    std::string src = j.value("cs_file", "data/shaders/compute.fx");
    std::string cs_name = j.value("name", "");
    assert(!cs_name.empty());

    // Read textures
    typedef std::vector< std::string > VStrings;
    VStrings texture_names = j.value<VStrings>("textures", {});
    for (auto& it : texture_names) {
        auto t = Resources.get(it)->as<CTexture>();
        textures.push_back(t);
    }

    sizes[0] = j.value("sx", 16);
    sizes[1] = j.value("sy", 16);
    sizes[2] = j.value("sz", 1);
    groups[0] = j.value("gx", 32);
    groups[1] = j.value("gy", 32);
    groups[2] = j.value("gz", 1);

    cs = new CComputeShader();
    return cs->create(src.c_str(), cs_entry_point.c_str(), cs_profile.c_str());
}

void CComputeTask::destroy() {
    if (cs) {
        cs->destroy();
        delete cs;
        cs = nullptr;
    }
}

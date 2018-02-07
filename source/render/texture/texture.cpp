#include "mcv_platform.h"
#include "texture.h"
#include "DDSTextureLoader.h"

// ----------------------------------------------
class CTexturesResourceClass : public CResourceClass {
public:
  CTexturesResourceClass() {
    class_name = "Textures";
    extension = ".dds";
  }
  IResource* create(const std::string& name) const override {
    dbg("Creating texture %s\n", name.c_str());
    CTexture* res = new CTexture();
    bool is_ok = res->create(name);
    assert(is_ok);
    return res;
  }
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CTexture>() {
  static CTexturesResourceClass the_textures_resource_class;
  return &the_textures_resource_class;
}

// ----------------------------------------------------------
bool CTexture::create(const std::string& name) {

  wchar_t wFilename[MAX_PATH];
  mbstowcs(wFilename, name.c_str(), name.length()+1 );

  HRESULT hr = DirectX::CreateDDSTextureFromFile(
    Render.device,
    wFilename,
    &texture,
    &shader_resource_view
  );
  if( FAILED( hr ) )
    return false;
  return true;
}

void CTexture::activate(int slot) const {
  assert(texture && shader_resource_view);

  // Starting at slot 0, update just 1 slot with my shader resource view
  // In the future I could update more than one slot with multiple resource views
  // in a single call.
  Render.ctx->PSSetShaderResources(slot, 1, &shader_resource_view);
}

void CTexture::destroy() {
  SAFE_RELEASE(texture);
  SAFE_RELEASE(shader_resource_view);
}

void CTexture::debugInMenu() {
  ImGui::Text("Resolution is ..");
  ImGui::Image(shader_resource_view, ImVec2(128, 128));
}

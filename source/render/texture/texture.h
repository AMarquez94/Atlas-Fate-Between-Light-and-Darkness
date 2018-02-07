#pragma once

#include "resources/resource.h"

// ----------------------------------------------
class CTexture : public IResource {
  
  // DX
  ID3D11Resource*           texture = nullptr;
  ID3D11ShaderResourceView* shader_resource_view = nullptr; 

public:
  void activate(int slot) const;
  bool create(const std::string& name);
  void debugInMenu() override;
  void destroy() override;
};


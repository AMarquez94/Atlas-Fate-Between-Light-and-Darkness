#pragma once

class CRenderTechnique {
public:
  CVertexShader *vs = nullptr;
  CPixelShader *ps = nullptr;
  void activate() {
    vs->activate();
    ps->activate();
  }
};


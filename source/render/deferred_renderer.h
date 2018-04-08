#ifndef INC_RENDER_DEFERRED_RENDERER_H_
#define INC_RENDER_DEFERRED_RENDERER_H_

#include "render/texture/render_to_texture.h"

class CDeferredRenderer {

public:
  
  CRenderToTexture* rt_normals = nullptr;
  CRenderToTexture* rt_albedos = nullptr;;
  CRenderToTexture* rt_depth = nullptr;;
  CRenderToTexture* rt_acc_light = nullptr;;

  void renderGBuffer();
  void renderAccLight();
  void renderAmbientPass();
  void renderDirectionalLights();
  void renderPointLights();

public:

  bool create( int xres, int yres );
  void render( CRenderToTexture* rt_destination );

};


#endif


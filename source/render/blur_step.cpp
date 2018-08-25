#include "mcv_platform.h"
#include "blur_step.h"
#include "texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render_objects.h"

// ---------------------
bool CBlurStep::create(const char* name, int in_xres, int in_yres) {

  // Get input resolution
  xres = in_xres;
  yres = in_yres;

  rt_half_y = new CRenderToTexture();
  std::string sname = std::string(name) + "_y";
  bool is_ok = rt_half_y->createRT(sname.c_str(), xres, yres / 2, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
  assert(is_ok);

  sname = std::string(name) + "_xy";
  rt_output = new CRenderToTexture();
  is_ok = rt_output->createRT(sname.c_str(), xres / 2, yres / 2, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
  assert(is_ok);

  tech = Resources.get("blur.tech")->as<CRenderTechnique>();
  mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();

  return is_ok;
}

// --------------------
// dx/dy are texel units. (1,0) means one texel to the right
void CBlurStep::applyBlur(float dx, float dy) {

  // Normalize UV's
  cb_blur.blur_step.x = (float)dx / (float)xres;
  cb_blur.blur_step.y = (float)dy / (float)yres;
  
  cb_blur.activate();
  cb_blur.updateGPU();

  tech->activate();
  mesh->activateAndRender();
}

// ---------------------
CTexture* CBlurStep::apply(
  CTexture* input, 
  float global_distance, 
  VEC4 distances, 
  VEC4 weights
) {

  // Sum( Weights ) = 1 to not loose energy. +2 is to account for left and right taps
  float normalization_factor =
      1 * weights.x
    + 2 * weights.y
    + 2 * weights.z
    + 2 * weights.w
    ;
  cb_blur.blur_w.x = weights.x / normalization_factor;
  cb_blur.blur_w.y = weights.y / normalization_factor;
  cb_blur.blur_w.z = weights.z / normalization_factor;
  cb_blur.blur_w.w = weights.w / normalization_factor;
  cb_blur.blur_d.x = distances.x;
  cb_blur.blur_d.y = distances.y;
  cb_blur.blur_d.z = distances.z;
  cb_blur.blur_d.w = distances.w;  // Not used

  rt_half_y->activateRT();
  input->activate(TS_ALBEDO);
  applyBlur(0, global_distance);

  rt_output->activateRT();
  rt_half_y->activate(TS_ALBEDO);
  applyBlur(global_distance, 0);

  return rt_output;
}

CTexture* CBlurStep::applyHalf(
    CTexture* input,
    float global_distance,
    VEC4 distances,
    VEC4 weights
) {

    // Sum( Weights ) = 1 to not loose energy. +2 is to account for left and right taps
    float normalization_factor =
        1 * weights.x
        + 2 * weights.y
        + 2 * weights.z
        + 2 * weights.w
        ;
    cb_blur.blur_w.x = weights.x / normalization_factor;
    cb_blur.blur_w.y = weights.y / normalization_factor;
    cb_blur.blur_w.z = weights.z / normalization_factor;
    cb_blur.blur_w.w = weights.w / normalization_factor;
    cb_blur.blur_d.x = distances.x;
    cb_blur.blur_d.y = distances.y;
    cb_blur.blur_d.z = distances.z;
    cb_blur.blur_d.w = distances.w;  // Not used

    rt_output->activateRT();
    input->activate(TS_ALBEDO);
    applyBlur(global_distance, 0);

    return rt_output;
}

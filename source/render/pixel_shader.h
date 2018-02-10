#ifndef INC_RENDER_PIXEL_SHADER_H_
#define INC_RENDER_PIXEL_SHADER_H_

class CPixelShader {

  ID3D11PixelShader*   ps = nullptr;

public:
  bool create(const std::string& filename, const std::string& fn_entry_name);
  void destroy();
  void activate() const;

};


#endif


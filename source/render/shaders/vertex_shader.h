#ifndef INC_RENDER_VERTEX_SHADER_H_
#define INC_RENDER_VERTEX_SHADER_H_

class CVertexShader {

  ID3D11VertexShader*  vs = nullptr;
  ID3D11InputLayout*   vertex_layout = nullptr;
  const CVertexDecl*   vtx_declaration;

public:
	~CVertexShader() { destroy(); }
  bool create(const std::string& filename, const std::string& fn_entry_name, const std::string& vtx_decl_name);
  void destroy();
  void activate() const;
  const CVertexDecl* getVertexDecl() const { return vtx_declaration; }

};

#endif


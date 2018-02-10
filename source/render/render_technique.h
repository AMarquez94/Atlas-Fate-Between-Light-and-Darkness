#pragma once

class CRenderTechnique: public IResource {
public:
	CVertexShader vs;
	CPixelShader ps;


	void activate() const;
	bool create(const std::string& name, json& j);
	void debugInMenu() override;
	void destroy() override;
};


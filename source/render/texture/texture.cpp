#include "mcv_platform.h"
#include "texture.h"
#include "DDSTextureLoader.h"
#include "render/video/video_texture.h"

// ----------------------------------------------
class CTexturesResourceClass : public CResourceClass {
public:
	CTexturesResourceClass() {
		class_name = "Textures";
		extensions = { ".dds", ".h264" };
	}
	IResource* create(const std::string& name) const override {
		dbg("Creating texture %s\n", name.c_str());
        CTexture* res;
        
        std::string ext = name.substr(name.length() - 5);
        if (ext == ".h264")
            res = new CVideoTexture();
        else
            res = new CTexture();

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

	//wchar_t wFilename[MAX_PATH];
	//mbstowcs(wFilename, name.c_str(), name.length() + 1);

    const std::vector<char> &file_data = EngineFiles.loadResourceFile(name);

    HRESULT hr = DirectX::CreateDDSTextureFromMemory(
        Render.device,
        (const uint8_t*)file_data.data(),
        file_data.size(),
        &texture,
        &shader_resource_view);

	if (FAILED(hr))
		return false;

	// Name both objects in DX
	setDXName(texture, name.c_str());
	setDXName(shader_resource_view, name.c_str());

    EngineFiles.addPendingResourceFile(name, false);

	// Update the resolution from the resource
	// ...


	return true;
}

void CTexture::onFileChanged(const std::string& filename) {
	if (filename != getName())
		return;
	destroy();
	if (!create(filename)) {
		// reLoad has failed!!
	}
}

// Set to the DX driver that we don't want any texture in this slot
void CTexture::setNullTexture(int slot) {
	ID3D11ShaderResourceView* null_srv = nullptr;
	Render.ctx->PSSetShaderResources(slot, 1, &null_srv);
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
    ID3D11Texture2D * t = (ID3D11Texture2D*)texture;
    D3D11_TEXTURE2D_DESC desc;
    t->GetDesc(&desc);
    ImGui::Text("Resolution is %dx%d", desc.Width, desc.Height);
	ImGui::Image(shader_resource_view, ImVec2(128, 128));
}

// ------------------------------------------------
bool CTexture::create(
	int nxres
	, int nyres
	, DXGI_FORMAT nformat
	, TCreateOptions options
)
{

	xres = nxres;
	yres = nyres;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = xres;
	desc.Height = yres;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = nformat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	if (options == CREATE_DYNAMIC) {
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (options == CREATE_RENDER_TARGET) {
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
    else if (options == CREATE_COMPUTE_OUTPUT) {
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }
	else {
		assert(options == CREATE_STATIC);
	}

	ID3D11Texture2D* tex2d = nullptr;
	HRESULT hr = Render.device->CreateTexture2D(&desc, nullptr, &tex2d);
	if (FAILED(hr))
		return false;
	texture = tex2d;
	setDXName(texture, getName().c_str());

    if (options == CREATE_COMPUTE_OUTPUT) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
        ZeroMemory(&uav_desc, sizeof(uav_desc));
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = 0;
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        //uav_desc.Buffer.NumElements = num_elems;
        hr = Render.device->CreateUnorderedAccessView(texture, &uav_desc, &uav);
        if (FAILED(hr))
            return false;
    }

	// -----------------------------------------
	// Create a resource view so we can use the data in a shader
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	ZeroMemory(&srv_desc, sizeof(srv_desc));
	srv_desc.Format = nformat;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = desc.MipLevels;
	hr = Render.device->CreateShaderResourceView(texture, &srv_desc, &shader_resource_view);
	if (FAILED(hr))
		return false;
	// Name both objects in DX
	setDXName(texture, name.c_str());
	setDXName(shader_resource_view, name.c_str());

	setNameAndClass(getName(), getResourceClassOf<CTexture>());

    // Autoregister render targets only
    if (options == TCreateOptions::CREATE_RENDER_TARGET)
	    Resources.registerResource(this);

	return true;
}

void CTexture::setDXParams(int new_xres, int new_yres, ID3D11Resource* new_texture, ID3D11ShaderResourceView* new_srv) {
	xres = new_xres;
	yres = new_yres;
	shader_resource_view = new_srv;
	new_srv->AddRef();
    if (new_texture) {
        texture = new_texture;
        new_texture->AddRef();
    }
}

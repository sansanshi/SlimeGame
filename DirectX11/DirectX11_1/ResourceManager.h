#pragma once
#include<D3D11.h>
#include<xnamath.h>
#include<map>
#include<memory>
#include<string>
class ResourceManager
{
public:
	~ResourceManager();
	static ResourceManager& Instance()
	{
		static ResourceManager instance;
		return instance;
	}
	std::shared_ptr<ID3D11ShaderResourceView*> LoadSRV(std::string registerName, std::string path);
	void RegisterVS(std::string registerName, ID3D11VertexShader*);
	void RegisterPS(std::string registerName, ID3D11PixelShader*);
	std::shared_ptr<ID3D11ShaderResourceView*> GetTexture(std::string name);
	std::shared_ptr<ID3D11VertexShader*> GetVertexShader(std::string name);
	std::shared_ptr<ID3D11PixelShader*> GetPixelShader(std::string name);

private:

	ResourceManager();
	ResourceManager(HWND);
	ResourceManager(const ResourceManager&);
	std::map<std::string, std::shared_ptr<ID3D11ShaderResourceView*>> _srvMap;
	std::map<std::string, std::shared_ptr<ID3D11VertexShader*>> _vertexShaderMap;
	std::map<std::string, std::shared_ptr<ID3D11PixelShader*>> _pixelShaderMap;

	HWND _hwnd;
};


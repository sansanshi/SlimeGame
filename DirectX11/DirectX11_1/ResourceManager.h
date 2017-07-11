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
	void LoadVS(
		std::string registerName,
		LPCSTR srcFileName, LPCSTR functionName, LPCSTR shaderModel,
		std::weak_ptr<ID3D11VertexShader*>& shader_out,
		D3D11_INPUT_ELEMENT_DESC* inputElementDescs, int descCnt,
		std::weak_ptr<ID3D11InputLayout*>& layout_out
	);
	void LoadPS(
		std::string registerName,
		LPCSTR srcFile, LPCSTR functionName, LPCSTR shaderModel,
		std::weak_ptr<ID3D11PixelShader*>& shader_out
	);

	std::shared_ptr<ID3D11ShaderResourceView*> LoadSRV(std::string registerName, std::string path);
	void RegisterVS(std::string registerName, ID3D11VertexShader*);
	void RegisterPS(std::string registerName, ID3D11PixelShader*);
	bool IsRegisterdVS(std::string registerName);
	bool IsRegisterdPS(std::string registerName);
	std::shared_ptr<ID3D11VertexShader*> VertexShader(std::string registerName);
	std::shared_ptr<ID3D11PixelShader*> PixelShader(std::string registerName);
	std::shared_ptr<ID3D11InputLayout*> InputLayout(std::string registerName);

private:

	ResourceManager();
	ResourceManager(HWND);
	ResourceManager(const ResourceManager&);
	std::map<std::string, std::shared_ptr<ID3D11ShaderResourceView*>> _srvMap;
	std::map<std::string, std::shared_ptr<ID3D11VertexShader*>> _vertexShaderMap;
	std::map<std::string, std::shared_ptr<ID3D11PixelShader*>> _pixelShaderMap;
	std::map<std::string, std::shared_ptr<ID3D11InputLayout*>> _inputLayoutMap;

	HWND _hwnd;
};


#include "ResourceManager.h"
#include<D3DX11.h>
#include"DeviceDx11.h"

ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager()
{
}

ResourceManager::ResourceManager(HWND hwnd):_hwnd(hwnd)
{

}
void
ResourceManager::LoadVS(
	std::string registerName,
	LPCSTR srcFileName, LPCSTR functionName, LPCSTR shaderModel,
	std::weak_ptr<ID3D11VertexShader*>& shader_out,
	D3D11_INPUT_ELEMENT_DESC* inputElementDescs, int descCnt,
	std::weak_ptr<ID3D11InputLayout*>& layout_out)
{
	if (_vertexShaderMap.find(registerName) != _vertexShaderMap.end())
	{
		shader_out = _vertexShaderMap[registerName];
		layout_out = _inputLayoutMap[registerName];
		return;
	}
	else
	{
		DeviceDx11& dev = DeviceDx11::Instance();
		HRESULT result;
		ID3DBlob* compiledVS = nullptr;
		ID3DBlob* shaderError = nullptr;

		D3DX11CompileFromFile(
			srcFileName,
			nullptr,
			nullptr,
			functionName,
			shaderModel,
			0,
			0,
			nullptr,
			&compiledVS,
			&shaderError,
			&result);

		char* msg;
		if (shaderError != nullptr)
		{
			msg = static_cast<char*>(shaderError->GetBufferPointer());// (char*)shaderError->GetBufferPointer();
			int j = 0;
		}
		ID3D11VertexShader* tempVS = nullptr;
		ID3D11InputLayout* tempInput = nullptr;
		if (FAILED(dev.Device()->CreateVertexShader(compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), nullptr, &tempVS)))
		{
			std::string path = srcFileName;
			path += "/";
			path += functionName;
			std::string msg = path + " が生成できません";
			MessageBox(_hwnd, msg.c_str(), nullptr, MB_OK);
			exit(0);
		}
		if (FAILED(dev.Device()->CreateInputLayout(inputElementDescs, descCnt, compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &tempInput)))
		{
			std::string path = srcFileName;
			path += "/";
			path += functionName;
			std::string msg = path + " のインプットレイアウトが生成できません";
			MessageBox(_hwnd, msg.c_str(), nullptr, MB_OK);
			exit(0);
		}

		_vertexShaderMap[registerName] = std::make_shared<ID3D11VertexShader*>(tempVS);
		_inputLayoutMap[registerName] = std::make_shared<ID3D11InputLayout*>(tempInput);


		compiledVS->Release();
	}
	shader_out = _vertexShaderMap[registerName];
	layout_out = _inputLayoutMap[registerName];
	return;
}
void
ResourceManager::LoadPS(
	std::string registerName,
	LPCSTR srcFileName, LPCSTR functionName, LPCSTR shaderModel,
	std::weak_ptr<ID3D11PixelShader*>& shader_out)
{
	if (_pixelShaderMap.find(registerName) != _pixelShaderMap.end())
	{
		shader_out = _pixelShaderMap[registerName];
		return;
	}
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result;
	ID3D11PixelShader* shader = nullptr;
	ID3DBlob* compiledPS = nullptr;
	ID3DBlob* shaderError = nullptr;

	D3DX11CompileFromFile(
		srcFileName,
		nullptr,
		nullptr,
		functionName,
		shaderModel,
		0,
		0,
		nullptr,
		&compiledPS,
		&shaderError,
		&result);

	char* msg;
	if (shaderError != nullptr)
	{
		msg = (char*)shaderError->GetBufferPointer();
		int j = 0;
	}

	ID3D11PixelShader* tempPS = nullptr;
	if (FAILED(dev.Device()->CreatePixelShader(compiledPS->GetBufferPointer(), compiledPS->GetBufferSize(), nullptr, &tempPS)))
	{
		std::string path = srcFileName;
		path += "/";
		path += functionName;
		std::string msg = path + " が生成できません";
		MessageBox(_hwnd, msg.c_str(), nullptr, MB_OK);
		exit(0);
	}
	_pixelShaderMap[registerName] = std::make_shared<ID3D11PixelShader*>(tempPS);

	shader_out = _pixelShaderMap[registerName];

	compiledPS->Release();

	return;
}


std::shared_ptr<ID3D11ShaderResourceView*>
ResourceManager::LoadSRV(std::string registerName, std::string filename)
{
	if (_srvMap.find(registerName)!=_srvMap.end())
	{
		return _srvMap[registerName];
	}
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;

	std::string path = "texture/" + filename;

	ID3D11ShaderResourceView* temp;
	if (FAILED(D3DX11CreateShaderResourceViewFromFile(dev.Device(), path.c_str(), nullptr, nullptr,
		&temp, &result)))
	{
		std::string msg = path + " がロードできません";
		MessageBox(_hwnd, msg.c_str(), nullptr, MB_OK);
		exit(0);
	}
	_srvMap[registerName] = std::make_shared<ID3D11ShaderResourceView*>(temp);
	return _srvMap[registerName];
	
}
void
ResourceManager::RegisterVS(std::string registerName, ID3D11VertexShader* vs)
{
	_vertexShaderMap[registerName] = std::make_shared<ID3D11VertexShader*>(vs);
	return;
}
void
ResourceManager::RegisterPS(std::string registerName, ID3D11PixelShader* ps)
{
	_pixelShaderMap[registerName] = std::make_shared<ID3D11PixelShader*>(ps);
	return;
}

bool
ResourceManager::IsRegisterdVS(std::string registerName)
{
	return _vertexShaderMap.find(registerName) != _vertexShaderMap.end();
	
}
bool
ResourceManager::IsRegisterdPS(std::string registerName)
{
	return _pixelShaderMap.find(registerName) != _pixelShaderMap.end();
}
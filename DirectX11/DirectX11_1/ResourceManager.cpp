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

std::shared_ptr<ID3D11ShaderResourceView*>
ResourceManager::LoadSRV(std::string registerName, std::string path)
{
	if (_srvMap.find(registerName)!=_srvMap.end())
	{
		return _srvMap[registerName];
	}
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;


	ID3D11ShaderResourceView* temp;
	if (FAILED(D3DX11CreateShaderResourceViewFromFile(dev.Device(), path.c_str(), nullptr, nullptr,
		&temp, &result)))
	{
		std::string msg = path + " Ç™ÉçÅ[ÉhÇ≈Ç´Ç‹ÇπÇÒ";
		MessageBox(_hwnd, msg.c_str(), nullptr, MB_OK);
		exit(0);
	}
	_srvMap[registerName] = std::make_shared<ID3D11ShaderResourceView*>(temp);
	return _srvMap[registerName];
	
}
void
ResourceManager::RegisterVS(std::string registerName, ID3D11VertexShader* vs)
{
	if (_vertexShaderMap[registerName] != nullptr)
	{
		return;
	}
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;

	_vertexShaderMap[registerName] = std::make_shared<ID3D11VertexShader*>(vs);
	return;
}
void
ResourceManager::RegisterPS(std::string registerName, ID3D11PixelShader* ps)
{
	if (_pixelShaderMap[registerName] != nullptr)
	{
		return;
	}
	DeviceDx11& dev = DeviceDx11::Instance();
	HRESULT result = S_OK;

	_pixelShaderMap[registerName] = std::make_shared<ID3D11PixelShader*>(ps);
	return;
}
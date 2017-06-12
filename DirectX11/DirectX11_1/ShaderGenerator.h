#pragma once
#include<D3D11.h>
#include<D3DX11.h>
#include"DeviceDx11.h"
#include<map>
#include<string>

class ShaderGenerator{
private:
	ShaderGenerator();
	ShaderGenerator(const ShaderGenerator&);

	std::map<std::string, ID3D11VertexShader*> vsMap;
	std::map<std::string, ID3D11PixelShader*> psMap;

public:
	~ShaderGenerator();
	static HRESULT CreateVertexShader(LPCSTR srcFile, LPCSTR functionName, LPCSTR shaderModel, ID3D11VertexShader*& shader_out, D3D11_INPUT_ELEMENT_DESC* inputElementDescs, int descCnt, ID3D11InputLayout*& layout_out)
	{
		DeviceDx11& dev = DeviceDx11::Instance();
		HRESULT result;
		ID3DBlob* compiledVS = nullptr;
		ID3DBlob* shaderError = nullptr;

		D3DX11CompileFromFile(
			srcFile,
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

		result = dev.Device()->CreateVertexShader(compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), nullptr, &shader_out);
		result = dev.Device()->CreateInputLayout(inputElementDescs, descCnt, compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &layout_out);
		int f = sizeof(*inputElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC);

		compiledVS->Release();

		return result;

	}
	static HRESULT CreatePixelShader(LPCSTR srcFile, LPCSTR functionName, LPCSTR shaderModel, ID3D11PixelShader*& shader_out)
	{
		DeviceDx11& dev = DeviceDx11::Instance();
		HRESULT result;
		ID3D11PixelShader* shader = nullptr;
		ID3DBlob* compiledPS = nullptr;
		ID3DBlob* shaderError = nullptr;

		D3DX11CompileFromFile(
			srcFile,
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

		result = dev.Device()->CreatePixelShader(compiledPS->GetBufferPointer(), compiledPS->GetBufferSize(), nullptr, &shader_out);

		compiledPS->Release();

		return result;
	}
	static HRESULT CreateHullShader(LPCSTR srcFile, LPCSTR functionName, LPCSTR shaderModel, ID3D11HullShader*& shader_out)
	{
		DeviceDx11& dev = DeviceDx11::Instance();
		HRESULT result;
		ID3D11PixelShader* shader = nullptr;
		ID3DBlob* compiledHS = nullptr;
		ID3DBlob* shaderError = nullptr;

		D3DX11CompileFromFile(
			srcFile,
			nullptr,
			nullptr,
			functionName,
			shaderModel,
			0,
			0,
			nullptr,
			&compiledHS,
			&shaderError,
			&result);

		char* msg;
		if (shaderError != nullptr)
		{
			msg = (char*)shaderError->GetBufferPointer();
			int j = 0;
		}

		result = dev.Device()->CreateHullShader(compiledHS->GetBufferPointer(), compiledHS->GetBufferSize(), nullptr, &shader_out);

		compiledHS->Release();

		return result;
	}
	static HRESULT CreateDomainShader(LPCSTR srcFile, LPCSTR functionName, LPCSTR shaderModel, ID3D11DomainShader*& shader_out)
	{
		DeviceDx11& dev = DeviceDx11::Instance();
		HRESULT result;
		ID3D11DomainShader* shader = nullptr;
		ID3DBlob* compiledDS = nullptr;
		ID3DBlob* shaderError = nullptr;

		D3DX11CompileFromFile(
			srcFile,
			nullptr,
			nullptr,
			functionName,
			shaderModel,
			0,
			0,
			nullptr,
			&compiledDS,
			&shaderError,
			&result);

		char* msg;
		if (shaderError != nullptr)
		{
			msg = (char*)shaderError->GetBufferPointer();
			int j = 0;
		}

		result = dev.Device()->CreateDomainShader(compiledDS->GetBufferPointer(), compiledDS->GetBufferSize(), nullptr, &shader_out);

		compiledDS->Release();

		return result;
	}

};
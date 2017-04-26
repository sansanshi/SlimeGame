#include "DeviceDx11.h"
#include<D3D11.h>
#include"Define.h"

DeviceDx11::DeviceDx11()
{
}


DeviceDx11::~DeviceDx11()
{
}

HRESULT
DeviceDx11::Init(HWND hwnd)
{
	_hwnd = hwnd;
	//デバイスとスワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd = {};//構造体初期化
	//ここからスワップチェイン
	sd.BufferCount = 1;

	sd.BufferDesc.Width = (unsigned int)WINDOW_WIDTH;
	sd.BufferDesc.Height = (unsigned int)WINDOW_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_R16G16B16A16_FLOAT;//

	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = _hwnd;//g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL pFeatureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* pFeatureLevel = nullptr;
	//gpuに命令とか投げるときはdevicecontext
	HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&pFeatureLevels,
		1,
		D3D11_SDK_VERSION,
		&sd,
		&_swapchain,
		&_device,
		pFeatureLevel,
		&_context
		);

	return result;

	
}

IDXGISwapChain*
DeviceDx11::SwapChain(){
	return _swapchain;
}
ID3D11Device*
DeviceDx11::Device(){
	return _device;
}
ID3D11DeviceContext*
DeviceDx11::Context(){
	return _context;
}
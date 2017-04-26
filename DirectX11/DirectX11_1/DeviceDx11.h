#pragma once
#include<Windows.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;


class DeviceDx11
{
private:
	HWND _hwnd;
	IDXGISwapChain* _swapchain;
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;


	DeviceDx11();
	DeviceDx11(const DeviceDx11&);
public:
	static DeviceDx11& Instance(){
		static DeviceDx11 instance;
		return instance;
	}


	HRESULT Init(HWND);
	IDXGISwapChain* SwapChain();//_swapchain‚ð•Ô‚·
	ID3D11Device* Device();//_device‚ð•Ô‚·
	ID3D11DeviceContext* Context();//_context‚ð•Ô‚·

	~DeviceDx11();
};


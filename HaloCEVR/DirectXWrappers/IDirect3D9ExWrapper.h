#pragma once
#include <d3d9.h>
class IDirect3D9ExWrapper : public IDirect3D9Ex
{
public:
	IDirect3D9ExWrapper(IDirect3D9Ex* RealInstance) { Real = RealInstance; }

	virtual __declspec(nothrow) HRESULT __stdcall CreateDevice(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface);

	// Inherited via IDirect3D9Ex
	virtual __declspec(nothrow)HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObj) override;
	virtual __declspec(nothrow)ULONG __stdcall AddRef(void) override;
	virtual __declspec(nothrow)ULONG __stdcall Release(void) override;
	virtual __declspec(nothrow)HRESULT __stdcall RegisterSoftwareDevice(void* pInitializeFunction) override;
	virtual __declspec(nothrow)UINT __stdcall GetAdapterCount(void) override;
	virtual __declspec(nothrow)HRESULT __stdcall GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) override;
	virtual __declspec(nothrow)UINT __stdcall GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) override;
	virtual __declspec(nothrow)HRESULT __stdcall EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) override;
	virtual __declspec(nothrow)HRESULT __stdcall GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) override;
	virtual __declspec(nothrow)HRESULT __stdcall CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) override;
	virtual __declspec(nothrow)HRESULT __stdcall CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) override;
	virtual __declspec(nothrow)HRESULT __stdcall CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) override;
	virtual __declspec(nothrow)HRESULT __stdcall CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) override;
	virtual __declspec(nothrow)HRESULT __stdcall CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) override;
	virtual __declspec(nothrow)HRESULT __stdcall GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) override;
	virtual __declspec(nothrow)HMONITOR __stdcall GetAdapterMonitor(UINT Adapter) override;
	virtual __declspec(nothrow)UINT __stdcall GetAdapterModeCountEx(UINT Adapter, const D3DDISPLAYMODEFILTER* pFilter) override;
	virtual __declspec(nothrow)HRESULT __stdcall EnumAdapterModesEx(UINT Adapter, const D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) override;
	virtual __declspec(nothrow)HRESULT __stdcall GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) override;
	virtual __declspec(nothrow)HRESULT __stdcall CreateDeviceEx(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) override;
	virtual __declspec(nothrow)HRESULT __stdcall GetAdapterLUID(UINT Adapter, LUID* pLUID) override;

protected:
	IDirect3D9Ex* Real;
};


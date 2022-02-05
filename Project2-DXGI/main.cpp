#include <stdio.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <wincodec.h>
#include <DirectXTex.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
using namespace DirectX;

int main(void) {
	CoInitialize(NULL);
	HRESULT hr = 0;

	D3D_FEATURE_LEVEL level_11[1] = {
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL result;

	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_Context = nullptr;
	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED, level_11, 1, D3D11_SDK_VERSION, &m_Device, &result, &m_Context);
	if (!m_Device) {
		return 1;
	}
	printf("ID3D11Device D3D11CreateDevice = %X\n", hr);
	printf("D3D_FEATURE_LEVEL result = %X\n", result);

	IDXGIDevice* DxgiDevice = nullptr;
	hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
	printf("IDXGIDevice QueryInterface = %X\n",hr);

	

	IDXGIAdapter* DxgiAdapter = nullptr;
	hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
	DxgiDevice->Release();
	DxgiDevice = nullptr;
	printf("IDXGIAdapter GetParent = %X\n", hr);

	IDXGIOutput* DxgiOutput = nullptr;
	hr = DxgiAdapter->EnumOutputs(NULL, &DxgiOutput);
	DxgiAdapter->Release();
	DxgiAdapter = nullptr;
	printf("IDXGIOutput EnumOutputs = %X\n", hr);

	/*DXGI_OUTPUT_DESC m_OutputDesc;
	DxgiOutput->GetDesc(&m_OutputDesc);*/

	IDXGIOutput1* DxgiOutput1 = nullptr;
	hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), reinterpret_cast<void**>(&DxgiOutput1));
	DxgiOutput->Release();
	DxgiOutput = nullptr;
	printf("IDXGIOutput1 QueryInterface = %X\n", hr);

	IDXGIOutputDuplication* m_DeskDupl;
	hr = DxgiOutput1->DuplicateOutput(m_Device, &m_DeskDupl);
	DxgiOutput1->Release();
	DxgiOutput1 = nullptr;
	printf("IDXGIOutput1 DuplicateOutput = %X\n", hr);

	Sleep(500);
	hr = m_DeskDupl->ReleaseFrame();
	printf("ID3D11Texture2D ReleaseFrame = %X\n", hr);

	IDXGIResource* DesktopResource = nullptr;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	hr = m_DeskDupl->AcquireNextFrame(500, &FrameInfo, &DesktopResource);
	printf("IDXGIResource AcquireNextFrame = %X\n", hr);

	ID3D11Texture2D* m_AcquiredDesktopImage = nullptr;
	hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_AcquiredDesktopImage));
	DesktopResource->Release();
	DesktopResource = nullptr;
	printf("ID3D11Texture2D QueryInterface = %X\n", hr);
	if (!m_AcquiredDesktopImage) {
		printf("no2");
		return 2;
	}

	D3D11_TEXTURE2D_DESC pDesc;
	m_AcquiredDesktopImage->GetDesc(&pDesc);
	printf("width = %d  height = %d  DXGI_FORMAT = %d\n", pDesc.Width, pDesc.Height,pDesc.Format);
	printf("DXGI_FORMAT_B8G8R8A8_UNORM = 87\n");
	printf("is = %d\n", pDesc.Usage);

	ID3D11Texture2D* copyDesktop = nullptr;
	pDesc.Usage = D3D11_USAGE_STAGING;
	pDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	pDesc.BindFlags = 0;
	pDesc.MiscFlags = 0;
	pDesc.MipLevels = 1;
	pDesc.ArraySize = 1;
	pDesc.SampleDesc.Count = 1;

	hr = m_Device->CreateTexture2D(&pDesc, NULL, &copyDesktop);
	printf("ID3D11Texture2D CreateTexture2D = %X\n", hr);
	if (!copyDesktop) {
		printf("no3");
		return 3;
	}

	m_Context->CopyResource(copyDesktop, m_AcquiredDesktopImage);
	

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = m_Context->Map(copyDesktop, 0, D3D11_MAP_READ, 0, &mappedResource);
	printf("D3D11_MAPPED_SUBRESOURCE Map = %X\n", hr);

	Image img;
	img.width = (size_t)pDesc.Width;
	img.height = (size_t)pDesc.Height;
	img.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	img.rowPitch = (size_t)mappedResource.RowPitch;
	img.slicePitch = (size_t)mappedResource.DepthPitch;
	img.pixels = (uint8_t *)mappedResource.pData;
	hr = SaveToWICFile(img, WIC_FLAGS_FORCE_SRGB, GetWICCodec(WIC_CODEC_PNG), L"NEW_IMAGE.PNG", &GUID_WICPixelFormat24bppBGR);
	printf("Image SaveToWICFile = %X\n", hr);
	//E_NOINTERFACE
	printf("RowPitch = %d  DepthPitch = %d\n", mappedResource.RowPitch, mappedResource.DepthPitch);
	
	//getchar();
	CoUninitialize();
}

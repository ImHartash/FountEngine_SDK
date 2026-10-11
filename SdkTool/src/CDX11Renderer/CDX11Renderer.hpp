#pragma once
#include <d3d11.h>
#include <QImage>
#include <vector>
#include <cstdint>

struct PreviewVertex_t {
	float px, py, pz;
	float nx, ny, nz;
	float tu, tv;

	PreviewVertex_t()
		: px(0.f), py(0.f), pz(0.f), nx(0.f), ny(0.f), nz(0.f), tu(0.f), tv(0.f) { }
	PreviewVertex_t(float px, float py, float pz, float nx, float ny, float nz, float tu, float tv)
		: px(px), py(py), pz(pz), nx(nx), ny(ny), nz(nz), tu(tu), tv(tv) { }
};

class CDX11Renderer {
public:
	CDX11Renderer() = default;
	~CDX11Renderer();

	bool Initialize();
	QImage RenderImage(const std::vector<PreviewVertex_t>& vecVertices,
		const std::vector<uint32_t>& vecIndices, int nWidth = 320, int nHeight = 320);

private:
	bool LoadShaders();
	bool CreateOffscreenTarget(int nWidth, int nHeight);
	bool SetupGeometry(const std::vector<PreviewVertex_t>& vecVertices,
		const std::vector<uint32_t>& vecIndices);

	QImage ReadBack(int nWidth, int nHeight);

	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	ID3D11Buffer* m_pConstantBuffer = nullptr;
	ID3D11Texture2D* m_pRenderTarget = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	ID3D11Texture2D* m_pDepth = nullptr;
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
	ID3D11Texture2D* m_pStaging = nullptr;
	int m_nWidth = 0;
	int m_nHeight = 0;
};
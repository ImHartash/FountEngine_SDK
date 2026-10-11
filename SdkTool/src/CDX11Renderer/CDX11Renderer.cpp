/*
This is simpliest realization of render on DX11. 
In future, when engine will be more stable, I maybe will recode this.

ImHartash
*/

#include "CDX11Renderer.hpp"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <QDebug>
#define RELEASE(x) if (x) x->Release();
#define R_DBG qDebug() << "[RENDERER] "

static const char* g_pVertexShaderCode = R"(
cbuffer cb : register(b0) {
    float4x4 gWorld;
    float4x4 gWorldIT;
    float4x4 gWVP;
    float4 gLightDir;
    float4 gCamPos;
    float4 gColor;
    float  gShininess;
};
struct VSIn  { float3 p : POSITION; float3 n : NORMAL; float2 uv : TEXCOORD; };
struct VSOut { float4 pos : SV_POSITION; float3 nw : NORMAL; float3 wp : POSITION; };
VSOut main(VSIn i) {
    VSOut o;
    o.wp  = mul(float4(i.p, 1.0f), gWorld).xyz;
    o.nw  = mul(i.n, (float3x3)gWorldIT);
    o.pos = mul(float4(i.p, 1.0f), gWVP);
    return o;
}
)";

static const char* g_pPixelShaderCode = R"(
cbuffer cb : register(b0) {
    float4x4 gWorld;
    float4x4 gWorldIT;
    float4x4 gWVP;
    float4 gLightDir;
    float4 gCamPos;
    float4 gColor;
    float  gShininess;
};
struct VSOut { float4 pos : SV_POSITION; float3 nw : NORMAL; float3 wp : POSITION; };
float4 main(VSOut i) : SV_TARGET {
    float3 n = normalize(i.nw);
    float3 L = normalize(gLightDir.xyz);
    float3 V = normalize(gCamPos.xyz - i.wp);
    float  diff = saturate(dot(n, L));
    float3 H = normalize(L + V);
    float  spec = pow(saturate(dot(n, H)), gShininess);
    float3 col = gColor.rgb * (0.15 + 0.85 * diff) + float3(1,1,1) * 0.55 * spec;
    return float4(col, 1.0);
}
)";

struct PreviewBuffer_t {
    DirectX::XMFLOAT4X4 mtWorld;
    DirectX::XMFLOAT4X4 mtWorldIT;
    DirectX::XMFLOAT4X4 mtWVP;
    DirectX::XMFLOAT4   vecLightDir;
    DirectX::XMFLOAT4   vecCamPos;
    DirectX::XMFLOAT4   vecColor;
    float  fShininess;
    float  fPad[3];
};

CDX11Renderer::~CDX11Renderer() {
    RELEASE(m_pStaging);
    RELEASE(m_pDepthStencilView);
    RELEASE(m_pDepth);
    RELEASE(m_pRenderTargetView);
    RELEASE(m_pRenderTarget);
    RELEASE(m_pConstantBuffer);
    RELEASE(m_pIndexBuffer);
    RELEASE(m_pVertexBuffer);
    RELEASE(m_pInputLayout);
    RELEASE(m_pPixelShader);
    RELEASE(m_pVertexShader);
    RELEASE(m_pDeviceContext);
    RELEASE(m_pDevice);
}

bool CDX11Renderer::Initialize() {
    if (m_pDevice) return true;

    HRESULT hResult = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
        D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
    if (FAILED(hResult)) return false;

    return this->LoadShaders();
}

QImage CDX11Renderer::RenderImage(const std::vector<PreviewVertex_t>& vecVertices, const std::vector<uint32_t>& vecIndices, int nWidth, int nHeight) {
    if (!Initialize()) {
        R_DBG << "Failed to initialize renderer.";
        return QImage();
    }

    if (m_nWidth != nWidth || m_nHeight != nHeight)
        if (!CreateOffscreenTarget(nWidth, nHeight)) {
            R_DBG << "Failed to create offscreen target.";
            return QImage();
        }

    if (!SetupGeometry(vecVertices, vecIndices)) {
        R_DBG << "Failed to setup geometry buffers.";
        return QImage();
    }

    // Bounding sphere
    DirectX::XMVECTOR vecMin = DirectX::XMVectorSet(1e9f, 1e9f, 1e9f, 0);
    DirectX::XMVECTOR vecMax = DirectX::XMVectorSet(-1e9f, -1e9f, -1e9f, 0);

    for (const auto& vertex : vecVertices) {
        DirectX::XMVECTOR vecVPosition = DirectX::XMVectorSet(vertex.px, vertex.py, vertex.pz, 0);
        vecMin = DirectX::XMVectorMin(vecMin, vecVPosition);
        vecMax = DirectX::XMVectorMax(vecMax, vecVPosition);
    }

    DirectX::XMVECTOR vecCenter = DirectX::XMVectorScale(DirectX::XMVectorAdd(vecMin, vecMax), .5f);
    DirectX::XMVECTOR vecSize = DirectX::XMVectorSubtract(vecMax, vecMin);

    float flRadius = DirectX::XMVectorGetX(DirectX::XMVector3Length(vecSize)) * 0.5f;
    if (flRadius < 0.0001f) flRadius = 1.f;

    R_DBG << "center:" << DirectX::XMVectorGetX(vecCenter) << DirectX::XMVectorGetY(vecCenter) << DirectX::XMVectorGetZ(vecCenter)
        << " radius:" << flRadius;


    // Matrices
    float flFit = 1.f / flRadius;
    DirectX::XMMATRIX mtWorld = 
        DirectX::XMMatrixTranslation(-DirectX::XMVectorGetX(vecCenter),
        -DirectX::XMVectorGetY(vecCenter), -DirectX::XMVectorGetZ(vecCenter)) *
        DirectX::XMMatrixScaling(flFit, flFit, flFit) *
        DirectX::XMMatrixRotationY(-0.6f);

    DirectX::XMVECTOR vecCamera = DirectX::XMVectorSet(0.f, .35f, 2.6f, 0.f);
    DirectX::XMMATRIX mtView = DirectX::XMMatrixLookAtLH(vecCamera, DirectX::XMVectorSet(0, 0, 0, 0), DirectX::XMVectorSet(0, 1.f, 0, 0));
    float flAspectRatio = static_cast<float>(nWidth) / static_cast<float>(nHeight);
    DirectX::XMMATRIX mtProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.f), flAspectRatio, 0.01f, 100.f);
    
    DirectX::XMMATRIX mtWorldViewProjection = mtWorld * mtView * mtProjection;
    DirectX::XMMATRIX mtWorldInverseTranspose = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, mtWorld));

    PreviewBuffer_t Buffer{};
    DirectX::XMStoreFloat4x4(&Buffer.mtWorld, DirectX::XMMatrixTranspose(mtWorld));
    DirectX::XMStoreFloat4x4(&Buffer.mtWorldIT, DirectX::XMMatrixTranspose(mtWorldInverseTranspose));
    DirectX::XMStoreFloat4x4(&Buffer.mtWVP, DirectX::XMMatrixTranspose(mtWorldViewProjection));
    DirectX::XMStoreFloat4(&Buffer.vecLightDir, DirectX::XMVector3Normalize(
        DirectX::XMVectorSet(0.5f, 0.8f, 0.6f, 0.f)));
    DirectX::XMStoreFloat4(&Buffer.vecCamPos, vecCamera);
    DirectX::XMStoreFloat4(&Buffer.vecColor, DirectX::XMVectorSet(0.6f, 0.6f, 0.6f, 1.f));
    Buffer.fShininess = 32.f;

    m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
    float clear[4] = { 0.13f, 0.13f, 0.16f, 1.f };
    m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, clear);
    m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.f, 0);

    D3D11_VIEWPORT VP{ 0, 0, static_cast<float>(nWidth), static_cast<float>(nHeight), 0.f, 1.f };
    m_pDeviceContext->RSSetViewports(1, &VP);

    UINT nStride = sizeof(PreviewVertex_t), nOffset = 0;
    m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &nStride, &nOffset);
    m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_pDeviceContext->IASetInputLayout(m_pInputLayout);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
    m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    m_pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &Buffer, 0, 0);

    R_DBG << "Drawing scene...";
    m_pDeviceContext->DrawIndexed(static_cast<UINT>(vecIndices.size()), 0, 0);
    return ReadBack(nWidth, nHeight);
}

bool CDX11Renderer::LoadShaders() {
    HRESULT hResult;
    ID3DBlob* pVertexShaderBlob = nullptr;
    ID3DBlob* pPixelShaderBlob = nullptr;

    hResult = D3DCompile(
        g_pVertexShaderCode,
        strlen(g_pVertexShaderCode),
        nullptr, nullptr, nullptr,
        "main", "vs_5_0", 0, 0,
        &pVertexShaderBlob, nullptr
    );
    if (FAILED(hResult)) {
        RELEASE(pVertexShaderBlob);
        RELEASE(pPixelShaderBlob);
        return false;
    }

    hResult = D3DCompile(
        g_pPixelShaderCode,
        strlen(g_pPixelShaderCode),
        nullptr, nullptr, nullptr,
        "main", "ps_5_0", 0, 0,
        &pPixelShaderBlob, nullptr
    );
    if (FAILED(hResult)) {
        RELEASE(pVertexShaderBlob);
        RELEASE(pPixelShaderBlob);
        return false;
    }

    hResult = m_pDevice->CreateVertexShader(
        pVertexShaderBlob->GetBufferPointer(),
        pVertexShaderBlob->GetBufferSize(),
        nullptr, &m_pVertexShader);
    if (FAILED(hResult)) {
        RELEASE(pVertexShaderBlob);
        RELEASE(pPixelShaderBlob);
        return false;
    }

    hResult = m_pDevice->CreatePixelShader(
        pPixelShaderBlob->GetBufferPointer(),
        pPixelShaderBlob->GetBufferSize(),
        nullptr, &m_pPixelShader);
    if (FAILED(hResult)) {
        RELEASE(pVertexShaderBlob);
        RELEASE(pPixelShaderBlob);
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC VertexDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hResult = m_pDevice->CreateInputLayout(
        VertexDesc, 3, pVertexShaderBlob->GetBufferPointer(), 
        pVertexShaderBlob->GetBufferSize(), &m_pInputLayout);
    if (FAILED(hResult)) {
        RELEASE(pVertexShaderBlob);
        RELEASE(pPixelShaderBlob);
        return false;
    }

    RELEASE(pVertexShaderBlob);
    RELEASE(pPixelShaderBlob);

    D3D11_BUFFER_DESC BufferDesc{};
    BufferDesc.ByteWidth = sizeof(PreviewBuffer_t);
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hResult = m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pConstantBuffer);

    return SUCCEEDED(hResult);
}

bool CDX11Renderer::CreateOffscreenTarget(int nWidth, int nHeight) {
    RELEASE(m_pRenderTarget);
    RELEASE(m_pRenderTargetView);
    RELEASE(m_pDepth);
    RELEASE(m_pDepthStencilView);
    RELEASE(m_pStaging);

    D3D11_TEXTURE2D_DESC RenderTargetDesc{};
    RenderTargetDesc.Width = nWidth;
    RenderTargetDesc.Height = nHeight;
    RenderTargetDesc.MipLevels = 1;
    RenderTargetDesc.ArraySize = 1;
    RenderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    RenderTargetDesc.SampleDesc.Count = 1;
    RenderTargetDesc.Usage = D3D11_USAGE_DEFAULT;
    RenderTargetDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (FAILED(m_pDevice->CreateTexture2D(&RenderTargetDesc, nullptr, &m_pRenderTarget))) return false;
    if (FAILED(m_pDevice->CreateRenderTargetView(m_pRenderTarget, nullptr, &m_pRenderTargetView))) return false;

    D3D11_TEXTURE2D_DESC DepthDesc{};
    DepthDesc.Width = nWidth;
    DepthDesc.Height = nHeight;
    DepthDesc.MipLevels = 1;
    DepthDesc.ArraySize = 1;
    DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.SampleDesc.Count = 1;
    DepthDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (FAILED(m_pDevice->CreateTexture2D(&DepthDesc, nullptr, &m_pDepth))) return false;
    if (FAILED(m_pDevice->CreateDepthStencilView(m_pDepth, nullptr, &m_pDepthStencilView))) return false;

    D3D11_TEXTURE2D_DESC StagingDesc{};
    StagingDesc.Width = nWidth;
    StagingDesc.Height = nHeight;
    StagingDesc.MipLevels = 1;
    StagingDesc.ArraySize = 1;
    StagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    StagingDesc.SampleDesc.Count = 1;
    StagingDesc.Usage = D3D11_USAGE_STAGING;
    StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(m_pDevice->CreateTexture2D(&StagingDesc, nullptr, &m_pStaging))) return false;

    m_nWidth = nWidth;
    m_nHeight = nHeight;
    return true;
}

bool CDX11Renderer::SetupGeometry(const std::vector<PreviewVertex_t>& vecVertices, const std::vector<uint32_t>& vecIndices) {
    if (vecVertices.empty() || vecIndices.empty()) return false;

    RELEASE(m_pVertexBuffer);
    RELEASE(m_pIndexBuffer);

    D3D11_BUFFER_DESC VertexBufferDesc{};
    VertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(PreviewVertex_t) * vecVertices.size());
    VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA VertexBufferSubresourceData{};
    VertexBufferSubresourceData.pSysMem = vecVertices.data();
    if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexBufferSubresourceData, &m_pVertexBuffer))) return false;

    D3D11_BUFFER_DESC IndexBufferDesc{};
    IndexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * vecIndices.size());
    IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA IndexBufferSubresourceData{};
    IndexBufferSubresourceData.pSysMem = vecIndices.data();
    if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexBufferSubresourceData, &m_pIndexBuffer))) return false;

    return true;
}

QImage CDX11Renderer::ReadBack(int nWidth, int nHeight) {
    m_pDeviceContext->CopyResource(m_pStaging, m_pRenderTarget);
    D3D11_MAPPED_SUBRESOURCE Mapped{};
    if (FAILED(m_pDeviceContext->Map(m_pStaging, 0, D3D11_MAP_READ, 0, &Mapped)))
        return QImage();

    QImage Image(nWidth, nHeight, QImage::Format_RGBA8888);
    const uchar* pSource = static_cast<const uchar*>(Mapped.pData);
    for (int i = 0; i < nHeight; ++i)
        memcpy(Image.scanLine(i), pSource + i * Mapped.RowPitch, nWidth * 4);

    m_pDeviceContext->Unmap(m_pStaging, 0);
    return Image;
}

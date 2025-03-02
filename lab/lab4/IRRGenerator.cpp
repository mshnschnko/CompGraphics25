#include "IRRGenerator.h"
#include "D3DInclude.h"
#include "renderer.h"

HRESULT IRRGenerator::Init(ID3D11Device* device, ID3D11DeviceContext* context) {
  // Init constants
  g_irradienceTextureSize = 64;

  g_mMatrises[0] = XMMatrixRotationY(XM_PIDIV2);  // +X
  g_mMatrises[1] = XMMatrixRotationY(-XM_PIDIV2); // -X

  g_mMatrises[2] = XMMatrixRotationX(-XM_PIDIV2); // +Y
  g_mMatrises[3] = XMMatrixRotationX(XM_PIDIV2);  // -Y

  g_mMatrises[4] = XMMatrixIdentity();            // +Z
  g_mMatrises[5] = XMMatrixRotationY(XM_PI);      // -Z

  mViews[0] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
  );	// +X
  mViews[1] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
  );	// -X
  mViews[2] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }
  );	// +Y
  mViews[3] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }
  );	// -Y
  mViews[4] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
  );	// +Z
  mViews[5] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
  );	// -Z

  const float nearp = 0.5f;
  const float farp = 1.5f;
  const float fov = XM_PIDIV2;
  const float width = nearp / tanf(fov / 2.0f);
  const float height = width;
  mProjection = DirectX::XMMatrixPerspectiveLH(2 * width, 2 * height, nearp, farp);

  // Compile shaders
  ID3D10Blob* vertexShaderBuffer = nullptr;
  ID3D10Blob* pixelShaderBuffer = nullptr;
  int flags = 0;
#ifdef _DEBUG
  flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  HRESULT hr = D3DReadFileToBlob(L"HDRToCubeMap_VS.cso", &vertexShaderBuffer);
  if (FAILED(hr))
  {
      MessageBox(nullptr, L"The HDRToCubeMap_VS.cso file not found.", L"Error", MB_OK);
      return hr;
  }

  hr = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader);
  if (FAILED(hr))
    return hr;

  hr = D3DReadFileToBlob(L"CMToIRRMGenerator_PS.cso", &pixelShaderBuffer);
  if (FAILED(hr))
  {
      MessageBox(nullptr,
          L"The CMToIRRMGenerator_PS.cso file not found.", L"Error", MB_OK);
      return hr;
  }

  hr = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader);
  if (FAILED(hr))
    return hr;

  // Set constant buffers
  D3D11_BUFFER_DESC descCB = { 0 };
  descCB.Usage = D3D11_USAGE_DEFAULT;
  descCB.ByteWidth = sizeof(ConstantBuffer);
  descCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descCB.CPUAccessFlags = 0;
  descCB.MiscFlags = 0;
  descCB.StructureByteStride = 0;

  ConstantBuffer cb;
  memset(&cb, 0, sizeof(cb));

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = &cb;
  hr = device->CreateBuffer(&descCB, &data, &g_pConstantBuffer);
  if (FAILED(hr))
    return hr;

  // Init sampler
  D3D11_SAMPLER_DESC descSmplr = {};

  descSmplr.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  descSmplr.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  descSmplr.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  descSmplr.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  descSmplr.MinLOD = 0;
  descSmplr.MaxLOD = D3D11_FLOAT32_MAX;
  descSmplr.MipLODBias = 0.0f;

  hr = device->CreateSamplerState(&descSmplr, &g_pSamplerState);
  if (FAILED(hr))
    return hr;

  // cleate render target texture
  D3D11_TEXTURE2D_DESC hdrtd = {};
  hdrtd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  hdrtd.Width = g_irradienceTextureSize;
  hdrtd.Height = g_irradienceTextureSize;
  hdrtd.BindFlags = D3D11_BIND_RENDER_TARGET;
  hdrtd.Usage = D3D11_USAGE_DEFAULT;
  hdrtd.CPUAccessFlags = 0;
  hdrtd.MiscFlags = 0;
  hdrtd.MipLevels = 1;
  hdrtd.ArraySize = 1;
  hdrtd.SampleDesc.Count = 1;
  hdrtd.SampleDesc.Quality = 0;

  hr = device->CreateTexture2D(&hdrtd, nullptr, &g_pIRRTexture);
  if (FAILED(hr))
    return hr;

  hr = device->CreateRenderTargetView(g_pIRRTexture, nullptr, &g_pIRRTextureRTV);
  if (FAILED(hr))
    return hr;

  // Create irradience cube map texture
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  desc.Width = g_irradienceTextureSize;
  desc.Height = g_irradienceTextureSize;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
  desc.MipLevels = 1;
  desc.ArraySize = 6;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;

  hr = device->CreateTexture2D(&desc, nullptr, &g_pIRRMap);
  return hr;
}

void IRRGenerator::SetViewPort(ID3D11DeviceContext* context, UINT width, UINT hight)
{
  D3D11_VIEWPORT viewport = {};
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;
  viewport.Width = (float)width;
  viewport.Height = (float)hight;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;

  D3D11_RECT rect = {};
  rect.left = 0;
  rect.top = 0;
  rect.right = width;
  rect.bottom = hight;

  context->RSSetViewports(1, &viewport);
  context->RSSetScissorRects(1, &rect);
}

HRESULT IRRGenerator::GenerateIrradienceMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* cmSRV) {
  context->ClearState();
  context->OMSetRenderTargets(1, &g_pIRRTextureRTV, nullptr);
  Renderer::GetInstance().EnableDepth(false);

  // set view port & scissors rect
  SetViewPort(context, g_irradienceTextureSize, g_irradienceTextureSize);

  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->PSSetShader(g_pPixelShader, nullptr, 0);
  context->PSSetShaderResources(0, 1, &cmSRV);
  context->PSSetSamplers(0, 1, &g_pSamplerState);

  float clearColor[4] = { 0.9f, 0.3f, 0.1f, 1.0f };
  ConstantBuffer cb = {};

  for (UINT i = 0; i < 6; ++i)
  {
    context->ClearRenderTargetView(g_pIRRTextureRTV, clearColor);
    XMStoreFloat4x4(&cb.projectionMatrix, XMMatrixTranspose(g_mMatrises[i]));
    XMStoreFloat4x4(&cb.viewProjectionMatrix, XMMatrixTranspose(mViews[i] * mProjection));

    context->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    context->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    context->Draw(4, 0);
    context->CopySubresourceRegion(g_pIRRMap, i, 0, 0, 0, g_pIRRTexture, 0, nullptr);
  }

  // Create subresource
  HRESULT hr = device->CreateShaderResourceView(g_pIRRMap, nullptr, &g_pIRRMapSRV);
  Renderer::GetInstance().EnableDepth(true);
  return hr;
}

void IRRGenerator::Release() {
  if (g_pIRRMapSRV) g_pIRRMapSRV->Release();
  if (g_pIRRMap) g_pIRRMap->Release();
  if (g_pConstantBuffer) g_pConstantBuffer->Release();
  if (g_pPixelShader) g_pPixelShader->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pSamplerState) g_pSamplerState->Release();
  if (g_pIRRTextureRTV) g_pIRRTextureRTV->Release();
  if (g_pIRRTexture) g_pIRRTexture->Release();
}

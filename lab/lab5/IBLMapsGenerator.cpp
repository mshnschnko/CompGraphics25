#include "IBLMapsGenerator.h"
#include "D3DInclude.h"
#include "renderer.h"

HRESULT IBLMapsGenerator::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
  // Setting this flag improves the shader debugging experience, but still allows 
  // the shaders to be optimized and to run exactly the way they will run in 
  // the release configuration of this program.
  dwShaderFlags |= D3DCOMPILE_DEBUG;

  // Disable optimizations to further improve shader debugging
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
  D3DInclude includeObj;

  ID3DBlob* pErrorBlob = nullptr;
  hr = D3DCompileFromFile(szFileName, nullptr, &includeObj, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

  if (FAILED(hr))
  {
    if (pErrorBlob)
    {
      OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
    }
    return hr;
  }

  if (pErrorBlob)
    pErrorBlob->Release();

  return S_OK;
}

void IBLMapsGenerator::InitMatricies() {
  
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
}

HRESULT IBLMapsGenerator::Init(ID3D11Device* device, ID3D11DeviceContext* context) {
  // Compile shaders
  ID3D10Blob* vertexShaderBuffer = nullptr;
  ID3D10Blob* iirPixelShaderBuffer = nullptr;
  ID3D10Blob* prefilPixelMShaderBuffer = nullptr;
  ID3D10Blob* BRDFPixelMShaderBuffer = nullptr;
  int flags = 0;
#ifdef _DEBUG
  flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  HRESULT hr = CompileShaderFromFile(L"IBLMapsGenerator_vs.hlsl", "main", "vs_5_0", &vertexShaderBuffer);
  if (FAILED(hr))
    return hr;

  hr = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader);
  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(L"CMToIRRMGenerator_PS.hlsl", "main", "ps_5_0", &iirPixelShaderBuffer);
  if (FAILED(hr))
    return hr;

  hr = device->CreatePixelShader(iirPixelShaderBuffer->GetBufferPointer(), iirPixelShaderBuffer->GetBufferSize(), NULL, &g_pIrrCMPixelShader);
  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(L"CMToPrefilMGenerator_PS.hlsl", "main", "ps_5_0", &prefilPixelMShaderBuffer);
  if (FAILED(hr))
    return hr;

  hr = device->CreatePixelShader(prefilPixelMShaderBuffer->GetBufferPointer(), prefilPixelMShaderBuffer->GetBufferSize(), NULL, &g_pPrefilPixelShader);
  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(L"BRDFGenerator_PS.hlsl", "main", "ps_5_0", &BRDFPixelMShaderBuffer);
  if (FAILED(hr))
    return hr;

  hr = device->CreatePixelShader(BRDFPixelMShaderBuffer->GetBufferPointer(), BRDFPixelMShaderBuffer->GetBufferSize(), NULL, &g_pBRDFPixelShader);
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

  D3D11_BUFFER_DESC descPCB = { 0 };
  descPCB.Usage = D3D11_USAGE_DEFAULT;
  descPCB.ByteWidth = sizeof(PrefilConstantBuffer);
  descPCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descPCB.CPUAccessFlags = 0;
  descPCB.MiscFlags = 0;
  descPCB.StructureByteStride = 0;

  PrefilConstantBuffer pcb;
  memset(&pcb, 0, sizeof(pcb));

  D3D11_SUBRESOURCE_DATA prefilData;
  prefilData.pSysMem = &pcb;
  hr = device->CreateBuffer(&descPCB, &prefilData, &g_pPrefilConstantBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC descICB = { 0 };
  descICB.Usage = D3D11_USAGE_DEFAULT;
  descICB.ByteWidth = sizeof(IRRConstantBuffer);
  descICB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descICB.CPUAccessFlags = 0;
  descICB.MiscFlags = 0;
  descICB.StructureByteStride = 0;

  PrefilConstantBuffer icb;
  memset(&icb, 0, sizeof(icb));

  D3D11_SUBRESOURCE_DATA irrData;
  irrData.pSysMem = &icb;
  hr = device->CreateBuffer(&descICB, &irrData, &g_pIRRConstantBuffer);
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

  // cleate render target texture for IRR
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
  if (FAILED(hr))
    return hr;

  // cleate render target texture for prefiltered color
  D3D11_TEXTURE2D_DESC prefilHdrtd = {};
  prefilHdrtd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  prefilHdrtd.Width = g_prefilTextureSize;
  prefilHdrtd.Height = g_prefilTextureSize;
  prefilHdrtd.BindFlags = D3D11_BIND_RENDER_TARGET;
  prefilHdrtd.Usage = D3D11_USAGE_DEFAULT;
  prefilHdrtd.CPUAccessFlags = 0;
  prefilHdrtd.MiscFlags = 0;
  prefilHdrtd.MipLevels = 1;
  prefilHdrtd.ArraySize = 1;
  prefilHdrtd.SampleDesc.Count = 1;
  prefilHdrtd.SampleDesc.Quality = 0;

  hr = device->CreateTexture2D(&prefilHdrtd, nullptr, &g_pPrefilTexture);
  if (FAILED(hr))
    return hr;

  hr = device->CreateRenderTargetView(g_pPrefilTexture, nullptr, &g_pPrefilTextureRTV);
  if (FAILED(hr))
    return hr;

  // Create prefiled color map
  D3D11_TEXTURE2D_DESC pcmDesc = {};
  pcmDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  pcmDesc.Width = g_prefilTextureSize;
  pcmDesc.Height = g_prefilTextureSize;
  pcmDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  pcmDesc.Usage = D3D11_USAGE_DEFAULT;
  pcmDesc.CPUAccessFlags = 0;
  pcmDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
  pcmDesc.MipLevels = g_prefilMipMapLevels;
  pcmDesc.ArraySize = 6;
  pcmDesc.SampleDesc.Count = 1;
  pcmDesc.SampleDesc.Quality = 0;

  hr = device->CreateTexture2D(&pcmDesc, nullptr, &g_pPrefilMap);
  if (FAILED(hr))
    return hr;

  // Create render target texture for BRDF
  D3D11_TEXTURE2D_DESC BRDFd = {};
  BRDFd.Format = DXGI_FORMAT_R32G32_FLOAT;
  BRDFd.Width = g_BRDFTextureSize;
  BRDFd.Height = g_BRDFTextureSize;
  BRDFd.BindFlags = D3D11_BIND_RENDER_TARGET;
  BRDFd.Usage = D3D11_USAGE_DEFAULT;
  BRDFd.CPUAccessFlags = 0;
  BRDFd.MiscFlags = 0;
  BRDFd.MipLevels = 1;
  BRDFd.ArraySize = 1;
  BRDFd.SampleDesc.Count = 1;
  BRDFd.SampleDesc.Quality = 0;

  hr = device->CreateTexture2D(&BRDFd, nullptr, &g_pBRDFTexture);
  if (FAILED(hr))
    return hr;

  hr = device->CreateRenderTargetView(g_pBRDFTexture, nullptr, &g_pBRDFTextureRTV);
  if (FAILED(hr))
    return hr;
  
  // Create prefiled color map
  D3D11_TEXTURE2D_DESC brdfDesc = {};
  brdfDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
  brdfDesc.Width = g_BRDFTextureSize;
  brdfDesc.Height = g_BRDFTextureSize;
  brdfDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  brdfDesc.Usage = D3D11_USAGE_DEFAULT;
  brdfDesc.CPUAccessFlags = 0;
  brdfDesc.MiscFlags = 0;
  brdfDesc.MipLevels = 1;
  brdfDesc.ArraySize = 1;
  brdfDesc.SampleDesc.Count = 1;
  brdfDesc.SampleDesc.Quality = 0;

  hr = device->CreateTexture2D(&brdfDesc, nullptr, &g_pBRDFMap);
  return hr;
}

void IBLMapsGenerator::SetViewPort(ID3D11DeviceContext* context, UINT width, UINT hight)
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

HRESULT IBLMapsGenerator::GenerateMaps(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* cmSRV) {
  beginEvent(L"irradince cm generating");
  auto hr = GenerateIrranienceMap(device, context, cmSRV);
  endEvent();
  if (FAILED(hr))
    return hr;

  beginEvent(L"prefiltered color generating");
  hr = GeneratePrefilteredMap(device, context, cmSRV);
  endEvent();
  if (FAILED(hr))
    return hr;

  beginEvent(L"counting BRDF");
  hr = GenerateBRDF(device, context);
  endEvent();
  return hr;
}

HRESULT IBLMapsGenerator::GenerateIrranienceMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* cmSRV) {
  context->ClearState();
  context->OMSetRenderTargets(1, &g_pIRRTextureRTV, nullptr);
  Renderer::GetInstance().EnableDepth(false);

  // set view port & scissors rect
  SetViewPort(context, g_irradienceTextureSize, g_irradienceTextureSize);

  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->PSSetShader(g_pIrrCMPixelShader, nullptr, 0);
  context->PSSetShaderResources(0, 1, &cmSRV);
  context->PSSetSamplers(0, 1, &g_pSamplerState);

  float clearColor[4] = { 0.9f, 0.3f, 0.1f, 1.0f };
  ConstantBuffer cb = {};
  IRRConstantBuffer icb = {};
  icb.param.x = N1;
  icb.param.y = N2;

  for (UINT i = 0; i < 6; ++i)
  {
    context->ClearRenderTargetView(g_pIRRTextureRTV, clearColor);
    XMStoreFloat4x4(&cb.projectionMatrix, XMMatrixTranspose(g_mMatrises[i]));
    XMStoreFloat4x4(&cb.viewProjectionMatrix, XMMatrixTranspose(mViews[i] * mProjection));

    context->UpdateSubresource(g_pIRRConstantBuffer, 0, nullptr, &icb, 0, 0);
    context->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    context->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    context->PSSetConstantBuffers(0, 1, &g_pIRRConstantBuffer);
    context->Draw(4, 0);
    context->CopySubresourceRegion(g_pIRRMap, i, 0, 0, 0, g_pIRRTexture, 0, nullptr);
  }

  // Create subresource
  HRESULT hr = device->CreateShaderResourceView(g_pIRRMap, nullptr, &g_pIRRMapSRV);
  Renderer::GetInstance().EnableDepth(true);
  return hr;
}

HRESULT IBLMapsGenerator::GenerateBRDF(ID3D11Device* device, ID3D11DeviceContext* context) {
  context->ClearState();
  context->OMSetRenderTargets(1, &g_pBRDFTextureRTV, nullptr);
  Renderer::GetInstance().EnableDepth(false);

  // set view port & scissors rect
  SetViewPort(context, g_BRDFTextureSize, g_BRDFTextureSize);

  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->PSSetShader(g_pBRDFPixelShader, nullptr, 0);
  
  float clearColor[4] = { 0.9f, 0.3f, 0.1f, 1.0f };
  ConstantBuffer cb = {};

  context->ClearRenderTargetView(g_pBRDFTextureRTV, clearColor);
  XMStoreFloat4x4(&cb.projectionMatrix, XMMatrixTranspose(g_mMatrises[4]));
  XMStoreFloat4x4(&cb.viewProjectionMatrix, XMMatrixTranspose(mViews[4] * mProjection));

  context->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
  context->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
  context->Draw(4, 0);
  context->CopySubresourceRegion(g_pBRDFMap, 0, 0, 0, 0, g_pBRDFTexture, 0, nullptr);
  
  // Create subresource
  HRESULT hr = device->CreateShaderResourceView(g_pBRDFMap, nullptr, &g_pBRDFMapSRV);
  Renderer::GetInstance().EnableDepth(true);
  return hr;
}

HRESULT IBLMapsGenerator::GeneratePrefilteredMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* cmSRV) {
  context->ClearState();
  context->OMSetRenderTargets(1, &g_pPrefilTextureRTV, nullptr);
  Renderer::GetInstance().EnableDepth(false);

  // set view port & scissors rect
  SetViewPort(context, g_prefilTextureSize, g_prefilTextureSize);

  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->PSSetShader(g_pPrefilPixelShader, nullptr, 0);
  context->PSSetShaderResources(0, 1, &cmSRV);
  context->PSSetSamplers(0, 1, &g_pSamplerState);

  float clearColor[4] = { 0.9f, 0.3f, 0.1f, 1.0f };
  ConstantBuffer cb = {};
  PrefilConstantBuffer pcb = {};
  
  D3D11_BOX mipMapCube = {};
  mipMapCube.left = mipMapCube.top = mipMapCube.front = 0;
  mipMapCube.back = 1;
  
  for (UINT i = 0; i < 6; ++i)
  {
    XMStoreFloat4x4(&cb.projectionMatrix, XMMatrixTranspose(g_mMatrises[i]));
    XMStoreFloat4x4(&cb.viewProjectionMatrix, XMMatrixTranspose(mViews[i] * mProjection));
    context->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    UINT currentPrefilTextSize = g_prefilTextureSize;
    for (UINT j = 0; j < g_prefilMipMapLevels; j++) {
      context->ClearRenderTargetView(g_pPrefilTextureRTV, clearColor);

      pcb.roughness.x = j / (g_prefilMipMapLevels - 1);
      context->UpdateSubresource(g_pPrefilConstantBuffer, 0, nullptr, &pcb, 0, 0);
      context->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
      context->PSSetConstantBuffers(0, 1, &g_pPrefilConstantBuffer);

      mipMapCube.right = currentPrefilTextSize;
      mipMapCube.bottom = currentPrefilTextSize;


      SetViewPort(context, currentPrefilTextSize, currentPrefilTextSize);
      context->Draw(4, 0);
      context->CopySubresourceRegion(g_pPrefilMap, D3D11CalcSubresource(j, i, g_prefilMipMapLevels), 0, 0, 0, g_pPrefilTexture, 0, &mipMapCube);
      
      currentPrefilTextSize = (UINT)(currentPrefilTextSize / 2);
    }
  }

  // Create subresource
  HRESULT hr = device->CreateShaderResourceView(g_pPrefilMap, nullptr, &g_pPrefilMapSRV);
  Renderer::GetInstance().EnableDepth(true);
  return hr;
}

void IBLMapsGenerator::Release() {
  if (g_pBRDFMapSRV) g_pBRDFMapSRV->Release();
  if (g_pBRDFMap) g_pBRDFMap->Release();

  if (g_pBRDFTextureRTV) g_pBRDFTextureRTV->Release();
  if (g_pBRDFTexture) g_pBRDFTexture->Release();

  if (g_pPrefilMapSRV) g_pPrefilMapSRV->Release();
  if (g_pPrefilMap) g_pPrefilMap->Release();
  if (g_pPrefilTextureRTV) g_pPrefilTextureRTV->Release();
  if (g_pPrefilTexture) g_pPrefilTexture->Release();

  if (g_pIRRMapSRV) g_pIRRMapSRV->Release();
  if (g_pIRRMap) g_pIRRMap->Release();
  if (g_pIRRTextureRTV) g_pIRRTextureRTV->Release();
  if (g_pIRRTexture) g_pIRRTexture->Release();

  if (g_pPrefilConstantBuffer) g_pPrefilConstantBuffer->Release();
  if (g_pConstantBuffer) g_pConstantBuffer->Release();
  if (g_pIRRConstantBuffer) g_pIRRConstantBuffer->Release();
  if (g_pIrrCMPixelShader) g_pIrrCMPixelShader->Release();
  if (g_pBRDFPixelShader) g_pBRDFPixelShader->Release();
  if (g_pPrefilPixelShader) g_pPrefilPixelShader->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pSamplerState) g_pSamplerState->Release();
}

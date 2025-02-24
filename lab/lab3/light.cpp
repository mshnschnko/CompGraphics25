#include "light.h"

HRESULT Light::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  // Create index array
  static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  D3D11_BUFFER_DESC descVert = {};
  descVert.ByteWidth = sizeof(SphereVertex) * numSphereVertices;
  descVert.Usage = D3D11_USAGE_IMMUTABLE;
  descVert.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  descVert.CPUAccessFlags = 0;
  descVert.MiscFlags = 0;
  descVert.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA dataVert;
  ZeroMemory(&dataVert, sizeof(dataVert));
  dataVert.pSysMem = &vertices[0];
  HRESULT hr = device->CreateBuffer(&descVert, &dataVert, &g_pVertexBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC descInd = {};
  ZeroMemory(&descInd, sizeof(descInd));

  descInd.ByteWidth = sizeof(UINT) * numSphereFaces * 3;
  descInd.Usage = D3D11_USAGE_IMMUTABLE;
  descInd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  descInd.CPUAccessFlags = 0;
  descInd.MiscFlags = 0;
  descInd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA dataInd;
  dataInd.pSysMem = &indices[0];

  hr = device->CreateBuffer(&descInd, &dataInd, &g_pIndexBuffer);
  if (FAILED(hr))
    return hr;

  // Compile shaders
  ID3D10Blob* vertexShaderBuffer = nullptr;
  ID3D10Blob* pixelShaderBuffer = nullptr;
  int flags = 0;
#ifdef _DEBUG
  flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  hr = D3DCompileFromFile(L"no_lightning_VS.hlsl", NULL, NULL, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
  if (FAILED(hr))
    return hr;

  hr = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader);
  if (FAILED(hr))
    return hr;

  hr = D3DCompileFromFile(L"no_lightning_PS.hlsl", NULL, NULL, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
  if (FAILED(hr))
    return hr;

  hr = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader);
  if (FAILED(hr))
    return hr;

  int numElements = sizeof(InputDesc) / sizeof(InputDesc[0]);
  hr = device->CreateInputLayout(InputDesc, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &g_pVertexLayout);
  if (FAILED(hr))
    return hr;

  // Set constant buffers
  D3D11_BUFFER_DESC descWM = {};
  descWM.ByteWidth = sizeof(WorldMatrixBuffer);
  descWM.Usage = D3D11_USAGE_DEFAULT;
  descWM.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descWM.CPUAccessFlags = 0;
  descWM.MiscFlags = 0;
  descWM.StructureByteStride = 0;

  WorldMatrixBuffer worldMatrixBuffer;
  worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = &worldMatrixBuffer;
  data.SysMemPitch = sizeof(worldMatrixBuffer);
  data.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&descWM, &data, &g_pWorldMatrixBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC descSM = {};
  descSM.ByteWidth = sizeof(SceneMatrixBuffer);
  descSM.Usage = D3D11_USAGE_DYNAMIC;
  descSM.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descSM.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  descSM.MiscFlags = 0;
  descSM.StructureByteStride = 0;

  hr = device->CreateBuffer(&descSM, nullptr, &g_pSceneMatrixBuffer);
  if (FAILED(hr))
    return hr;

  // Set rastrizer state
  D3D11_RASTERIZER_DESC descRast = {};
  descRast.AntialiasedLineEnable = false;
  descRast.FillMode = D3D11_FILL_SOLID;
  descRast.CullMode = D3D11_CULL_NONE;
  descRast.DepthBias = 0;
  descRast.DepthBiasClamp = 0.0f;
  descRast.FrontCounterClockwise = false;
  descRast.DepthClipEnable = true;
  descRast.ScissorEnable = false;
  descRast.MultisampleEnable = false;
  descRast.SlopeScaledDepthBias = 0.0f;

  hr = device->CreateRasterizerState(&descRast, &g_pRasterizerState);
  if (FAILED(hr))
    return hr;

  return hr;
}

void Light::Release() {
  if (g_pRasterizerState) g_pRasterizerState->Release();
  if (g_pWorldMatrixBuffer) g_pWorldMatrixBuffer->Release();
  if (g_pSceneMatrixBuffer) g_pSceneMatrixBuffer->Release();
  if (g_pIndexBuffer) g_pIndexBuffer->Release();
  if (g_pVertexBuffer) g_pVertexBuffer->Release();
  if (g_pVertexLayout) g_pVertexLayout->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pPixelShader) g_pPixelShader->Release();
}

HRESULT Light::Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
  // Update world matrix
  WorldMatrixBuffer worldMatrixBuffer;

  worldMatrixBuffer.worldMatrix = XMMatrixScaling(radius, radius, radius) * XMMatrixTranslation(pos.x, pos.y, pos.z);
  worldMatrixBuffer.color = color;

  context->UpdateSubresource(g_pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);

  // Update Scene matrix
  D3D11_MAPPED_SUBRESOURCE subresource;
  HRESULT hr = context->Map(g_pSceneMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
  if (FAILED(hr))
    return hr;

  SceneMatrixBuffer& sceneBuffer = *reinterpret_cast<SceneMatrixBuffer*>(subresource.pData);
  sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
  context->Unmap(g_pSceneMatrixBuffer, 0);

  return S_OK;
}

void Light::RenderSphere(ID3D11DeviceContext* context) {
  context->RSSetState(g_pRasterizerState);

  context->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

  ID3D11Buffer* vertexBuffers[] = { g_pVertexBuffer };
  UINT strides[] = { sizeof(SphereVertex) };
  UINT offsets[] = { 0 };

  context->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
  context->IASetInputLayout(g_pVertexLayout);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer);
  context->VSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
  context->PSSetShader(g_pPixelShader, nullptr, 0);
  context->PSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer);

  context->DrawIndexed(numSphereFaces * 3, 0, 0);
}

void Light::ProvideInput(const Input& input) {
  // Presets: 0 - off, 1-9 = 2^key - 1
  std::vector<UINT> keys_codes = { DIKEYBOARD_0,
    DIKEYBOARD_1, DIKEYBOARD_2, DIKEYBOARD_3,
    DIKEYBOARD_4, DIKEYBOARD_5, DIKEYBOARD_6,
    DIKEYBOARD_7, DIKEYBOARD_8, DIKEYBOARD_9 };

  for (int i = 0; i < keys_codes.size(); i++)
    if (input.IsKeyPressed(keys_codes[i]))
      color.w = (float)(std::pow(2, i) - 1.f);
      
  // Smooth light changing
  auto status = input.IsPlusMinusPressed();
  auto newWval = color.w + status * 1.f;
  newWval = max(min(newWval, MAX_I), MIN_I);
  color.w = newWval;

  if (input.IsKeyPressed(DIK_L))
  {
    showSource = !showSource;
  }
}

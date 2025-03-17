#include "box.h"

HRESULT Box::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  // Compile the vertex shader
  ID3DBlob* pVSBlob = nullptr;
  HRESULT hr = D3DReadFileToBlob(L"box_VS.cso", &pVSBlob);
  if (FAILED(hr))
  {
      MessageBox(nullptr, L"The box_VS.cso file not found.", L"Error", MB_OK);
      return hr;
  }

  // Create the vertex shader
  hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
  if (FAILED(hr))
  {
    pVSBlob->Release();
    return hr;
  }

  // Define the input layout
  D3D11_INPUT_ELEMENT_DESC layout[] =
  {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
  };
  UINT numElements = ARRAYSIZE(layout);

  // Create the input layout
  hr = device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
    pVSBlob->GetBufferSize(), &g_pVertexLayout);
  pVSBlob->Release();
  if (FAILED(hr))
    return hr;

  // Set the input layout
  context->IASetInputLayout(g_pVertexLayout);

  // Compile the pixel shader
  ID3DBlob* pPSBlob = nullptr;

  hr = D3DReadFileToBlob(L"box_PS.cso", &pPSBlob);
  if (FAILED(hr))
  {
      MessageBox(nullptr,
          L"The box_PS.cso file not found.", L"Error", MB_OK);
      return hr;
  }

  // Create the pixel shader
  hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
  pPSBlob->Release();
  if (FAILED(hr))
    return hr;

  // Create vertex buffer
  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_IMMUTABLE;
  bd.ByteWidth = sizeof(BoxVertex) * vertices.size();
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = &vertices[0];
  /*InitData.SysMemPitch = sizeof(vertices);
  InitData.SysMemSlicePitch = 0;*/

  hr = device->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
  if (FAILED(hr))
    return hr;

  // Create index buffer
  D3D11_BUFFER_DESC bd1;
  ZeroMemory(&bd1, sizeof(bd1));
  bd1.Usage = D3D11_USAGE_IMMUTABLE;
  bd1.ByteWidth = sizeof(unsigned short) * indices.size();
  bd1.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd1.CPUAccessFlags = 0;
  bd1.MiscFlags = 0;
  bd1.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData1;
  ZeroMemory(&InitData1, sizeof(InitData1));
  InitData1.pSysMem = &indices[0];
  //InitData1.SysMemPitch = sizeof(indices);
  //InitData1.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&bd1, &InitData1, &g_pIndexBuffer);
  if (FAILED(hr))
    return hr;

  // Set constant buffers
  D3D11_BUFFER_DESC descWMB = {};
  descWMB.ByteWidth = sizeof(WorldMatrixBuffer);
  descWMB.Usage = D3D11_USAGE_DEFAULT;
  descWMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descWMB.CPUAccessFlags = 0;
  descWMB.MiscFlags = 0;
  descWMB.StructureByteStride = 0;

  WorldMatrixBuffer worldMatrixBuffer;
  worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = &worldMatrixBuffer;
  data.SysMemPitch = sizeof(worldMatrixBuffer);
  data.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&descWMB, &data, &g_pWorldMatrixBuffer);
  if (FAILED(hr))
    return hr;
  
  D3D11_BUFFER_DESC descSMB = {};
  descSMB.ByteWidth = sizeof(LightableSceneMatrixBuffer);
  descSMB.Usage = D3D11_USAGE_DYNAMIC;
  descSMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descSMB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  descSMB.MiscFlags = 0;
  descSMB.StructureByteStride = 0;

  hr = device->CreateBuffer(&descSMB, nullptr, &g_pSceneMatrixBuffer);
  if (FAILED(hr))
    return hr;

  // Set rastrizer state
  D3D11_RASTERIZER_DESC descRastr = {};
  descRastr.FillMode = D3D11_FILL_SOLID;
  descRastr.CullMode = D3D11_CULL_BACK;
  descRastr.FrontCounterClockwise = false;
  descRastr.DepthBias = 0;
  descRastr.SlopeScaledDepthBias = 0.0f;
  descRastr.DepthBiasClamp = 0.0f;
  descRastr.DepthClipEnable = true;
  descRastr.ScissorEnable = false;
  descRastr.MultisampleEnable = false;
  descRastr.AntialiasedLineEnable = false;

  hr = device->CreateRasterizerState(&descRastr, &g_pRasterizerState);
  if (FAILED(hr))
    return hr;

  return S_OK;
}


void Box::Release() {
  if (g_pRasterizerState) g_pRasterizerState->Release();

  if (g_pWorldMatrixBuffer) g_pWorldMatrixBuffer->Release();

  if (g_pSceneMatrixBuffer) g_pSceneMatrixBuffer->Release();
  if (g_pIndexBuffer) g_pIndexBuffer->Release();
  if (g_pVertexBuffer) g_pVertexBuffer->Release();
  if (g_pVertexLayout) g_pVertexLayout->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pPixelShader) g_pPixelShader->Release();
}

void Box::Render(ID3D11DeviceContext* context) {
  context->RSSetState(g_pRasterizerState);

  context->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
  
  ID3D11Buffer* vertexBuffers[] = { g_pVertexBuffer };
  UINT strides[] = { sizeof(BoxVertex) };
  UINT offsets[] = { 0 };
  context->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
  context->IASetInputLayout(g_pVertexLayout);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  
  context->VSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
  context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer);

  context->PSSetShader(g_pPixelShader, nullptr, 0);
  
  context->PSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
  context->PSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer);
  
  context->DrawIndexed(36, 0, 0);
}


HRESULT Box::Update(ID3D11DeviceContext* context, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, XMVECTOR& cameraPos, std::vector<Light>& lights) {
  // Update world matrix angle of first cube
  WorldMatrixBuffer worldMatrixBuffer;
  worldMatrixBuffer.worldMatrix = worldMatrix;
  worldMatrixBuffer.color = XMFLOAT4(boxMaterial.shine, 0.0f, 0.0f, 0.0f); // its shine of boxes
  context->UpdateSubresource(g_pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);

  // Get the view matrix
  D3D11_MAPPED_SUBRESOURCE subresource;
  HRESULT hr = context->Map(g_pSceneMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
  if (FAILED(hr))
    return FAILED(hr);

  LightableSceneMatrixBuffer& sceneBuffer = *reinterpret_cast<LightableSceneMatrixBuffer*>(subresource.pData);
  sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
  sceneBuffer.cameraPos = XMFLOAT4(XMVectorGetX(cameraPos), XMVectorGetY(cameraPos), XMVectorGetZ(cameraPos), 1.0f);
  sceneBuffer.ambientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
  sceneBuffer.lightCount = XMINT4((int32_t)lights.size(), 0, 0, 0);
  for (int i = 0; i < lights.size(); i++) {
    sceneBuffer.lightPos[i] = lights[i].GetLightPosition();
    sceneBuffer.lightColor[i] = lights[i].GetLightColor();
  }

  context->Unmap(g_pSceneMatrixBuffer, 0);

  return S_OK;
}

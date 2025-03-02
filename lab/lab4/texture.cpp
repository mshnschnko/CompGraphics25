#include <string>
#include "texture.h"

using namespace DirectX;

HRESULT Texture::CreateTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename) {
  std::wstring fn = filename;

  if (fn.find(std::wstring(L".dds")) != std::wstring::npos)
    return CreateDDSTextureFromFile(device, filename, nullptr, &g_pTextureView);
  else if (fn.find(std::wstring(L".hdr")) != std::wstring::npos)
    return CreateHDRTextureFromFile(device, filename);
  else
    return E_FAIL;
}

HRESULT Texture::CreateTextureFromFileEx(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename) {
  std::wstring fn = filename;

  if (fn.find(std::wstring(L".dds")) != std::wstring::npos)
    return CreateDDSTextureFromFileEx(device, deviceContext, filename,
      0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE,
      false, nullptr, &g_pTextureView);
  else if (fn.find(std::wstring(L".hdr")) != std::wstring::npos)
    return CreateHDRTextureFromFile(device, filename);
  else
    return E_FAIL;
}

HRESULT Texture::CreateHDRTextureFromFile(ID3D11Device* device, const wchar_t* filename) {
  // read file
  int h = 0, w = 0, c = 0;
  char *tmpPath = new char[300];
  sprintf(tmpPath, "%ws", filename);
  auto imgData = stbi_loadf(tmpPath, &w, &h, &c, 4);
  delete[] tmpPath;

  if (c == 0)
    return E_FAIL;

  // create texture
  D3D11_TEXTURE2D_DESC txtDesc;
  txtDesc.Width = w;
  txtDesc.Height = h;

  txtDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  txtDesc.Usage = D3D11_USAGE_DEFAULT;
  txtDesc.CPUAccessFlags = 0;
  txtDesc.MiscFlags = 0;
  txtDesc.MipLevels = 1;
  txtDesc.ArraySize = 1;
  txtDesc.SampleDesc.Count = 1;
  txtDesc.SampleDesc.Quality = 0;

  D3D11_SUBRESOURCE_DATA hdrtdata = {};
  hdrtdata.pSysMem = imgData;
  hdrtdata.SysMemPitch = 4u * w * sizeof(float);
  hdrtdata.SysMemSlicePitch = 0;

  // create texture
  HRESULT hr = device->CreateTexture2D(&txtDesc, &hdrtdata, &g_pTexture);
  if (FAILED(hr))
    return hr;

  // create shader resource view
  hr = device->CreateShaderResourceView(g_pTexture, nullptr, &g_pTextureView);
  return hr;
}

HRESULT Texture::Init(
  ID3D11Device* device, 
  ID3D11DeviceContext* deviceContext, 
  const wchar_t* filename
) {
  Release();
  return CreateTextureFromFile(device, deviceContext, filename);
}

HRESULT Texture::InitEx(
  ID3D11Device* device, 
  ID3D11DeviceContext* deviceContext, 
  const wchar_t* filename
) {
  Release();
  return CreateTextureFromFileEx(device, deviceContext, filename);
}

HRESULT Texture::InitArray(
  ID3D11Device* device,
  ID3D11DeviceContext* deviceContext,
  const std::vector<const wchar_t*> &filenames
) {
  Release();

  HRESULT hr = S_OK;
  auto textureCount = (UINT)filenames.size();

  std::vector<ID3D11Texture2D*> textures(textureCount);

  // Load textures from DDS files.
  for (UINT i = 0; i < textureCount; ++i) {
    hr = DirectX::CreateDDSTextureFromFile(device, filenames[i], (ID3D11Resource**)(&textures[i]), nullptr);
    if (FAILED(hr)) {
      return hr;
    }
  }

  D3D11_TEXTURE2D_DESC textureDesc;
  textures[0]->GetDesc(&textureDesc); // each element in the texture array has the same format and dimensions

  D3D11_TEXTURE2D_DESC arrayDesc;
  arrayDesc.Width = textureDesc.Width;
  arrayDesc.Height = textureDesc.Height;
  arrayDesc.MipLevels = textureDesc.MipLevels;
  arrayDesc.ArraySize = textureCount;
  arrayDesc.Format = textureDesc.Format;
  arrayDesc.SampleDesc.Count = 1;
  arrayDesc.SampleDesc.Quality = 0;
  arrayDesc.Usage = D3D11_USAGE_DEFAULT;
  arrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  arrayDesc.CPUAccessFlags = 0;
  arrayDesc.MiscFlags = 0;

  ID3D11Texture2D* textureArray = nullptr;
  hr = device->CreateTexture2D(&arrayDesc, 0, &textureArray);
  if (FAILED(hr))
    return hr;

  for (UINT texElement = 0; texElement < textureCount; ++texElement)
    for (UINT mipLevel = 0; mipLevel < textureDesc.MipLevels; ++mipLevel) {
      const int sourceSubresource = D3D11CalcSubresource(mipLevel, 0, textureDesc.MipLevels);
      const int destSubresource = D3D11CalcSubresource(mipLevel, texElement, textureDesc.MipLevels);
      deviceContext->CopySubresourceRegion(textureArray, destSubresource, 0, 0, 0, textures[texElement], sourceSubresource, nullptr);
    }

  D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
  viewDesc.Format = arrayDesc.Format;
  viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
  viewDesc.Texture2DArray.MostDetailedMip = 0;
  viewDesc.Texture2DArray.MipLevels = arrayDesc.MipLevels;
  viewDesc.Texture2DArray.FirstArraySlice = 0;
  viewDesc.Texture2DArray.ArraySize = textureCount;

  hr = device->CreateShaderResourceView(textureArray, &viewDesc, &g_pTextureView);
  if (FAILED(hr)) {
    return hr;
  }
  
  textureArray->Release();
  for (UINT i = 0; i < textureCount; ++i) {
    textures[i]->Release();
  }

  return hr;
}

ID3D11ShaderResourceView* Texture::GetTexture() {
  return g_pTextureView; 
};

void Texture::Release() {
  if (g_pTexture) {
    g_pTexture->Release();
    g_pTexture = nullptr;
  }
  if (g_pTextureView) {
    g_pTextureView->Release();
    g_pTextureView = nullptr;
  }
}

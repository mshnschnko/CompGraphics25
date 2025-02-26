#include "texture.h"

using namespace DirectX;

HRESULT Texture::Init(
  ID3D11Device* device, 
  ID3D11DeviceContext* deviceContext, 
  const wchar_t* filename
) {
  Release();
  return CreateDDSTextureFromFile(device, filename, nullptr, &g_pTextureView);
}

HRESULT Texture::InitEx(
  ID3D11Device* device, 
  ID3D11DeviceContext* deviceContext, 
  const wchar_t* filename
) {
  Release();
  return CreateDDSTextureFromFileEx(device, deviceContext, filename,
    0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE,
    false, nullptr, &g_pTextureView);
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
  }
  if (FAILED(hr)) {
    return hr;
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
  if (g_pTextureView) {
    g_pTextureView->Release();
    g_pTextureView = nullptr;
  }
}

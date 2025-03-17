// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.


#include "gltf_model.h"

HRESULT Model::LoadGLTFModelMetadata() {
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, m_gltffile.c_str());
  
  if (!err.empty() || !ret) {
    return E_FAIL;
  }

  return S_OK;
}

HRESULT Model::InitTexturesFromlMetadata(ID3D11Device* device) {
  g_pTextures = std::vector<ID3D11Texture2D*>(model.textures.size(), nullptr);
  g_pTexturesSRV = std::vector<ID3D11ShaderResourceView*>(model.textures.size(), nullptr);

  HRESULT hr = S_OK;
  for (int i = 0; i < model.images.size(); i++) {
    hr = InitTexture(device, i);
    if (FAILED(hr))
      break;
    hr = device->CreateShaderResourceView(g_pTextures[i], nullptr, &(g_pTexturesSRV[i]));
    if (FAILED(hr))
      break;
  }

  return S_OK;
}

HRESULT Model::InitTexture(ID3D11Device* device, size_t imgId) {
  DXGI_FORMAT format;
  size_t bites;
  if ((model.images[imgId].pixel_type == TINYGLTF_COMPONENT_TYPE_FLOAT) && (model.images[imgId].component = 4))
    format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  else if ((model.images[imgId].pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) && (model.images[imgId].component = 4))
    format = DXGI_FORMAT_R8G8B8A8_UNORM;
  else
    return E_FAIL;

  D3D11_TEXTURE2D_DESC txtDesc;
  txtDesc.Width = model.images[imgId].width;
  txtDesc.Height = model.images[imgId].height;

  txtDesc.Format = format;
  txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  txtDesc.Usage = D3D11_USAGE_DEFAULT;
  txtDesc.CPUAccessFlags = 0;
  txtDesc.MiscFlags = 0;
  txtDesc.MipLevels = 1;
  txtDesc.ArraySize = 1;
  txtDesc.SampleDesc.Count = 1;
  txtDesc.SampleDesc.Quality = 0;

  D3D11_SUBRESOURCE_DATA hdrtdata = {};
  hdrtdata.pSysMem = (void *)(&(model.images[imgId].image[0]));
  hdrtdata.SysMemPitch = model.images[imgId].component * model.images[imgId].width * (int)(model.images[imgId].bits / 8);
  hdrtdata.SysMemSlicePitch = 0;

  // create texture
  HRESULT hr = device->CreateTexture2D(&txtDesc, &hdrtdata, &(g_pTextures[imgId]));
  return hr;
}

HRESULT Model::InitSamplersFromlMetadata(ID3D11Device* device) {
  g_pSamplers = std::vector<ID3D11SamplerState*>(model.samplers.size(), nullptr);
  
  HRESULT hr = S_OK;
  for (int i = 0; i < model.samplers.size(); i++) {
    hr = InitSampler(device, i);
    if (FAILED(hr))
      break;
  }

  return S_OK;
}


HRESULT Model::InitSampler(ID3D11Device* device, size_t smplrId) {
  D3D11_TEXTURE_ADDRESS_MODE addressModeS, addressModeT;
  if (model.samplers[smplrId].wrapS == TINYGLTF_TEXTURE_WRAP_REPEAT)
    addressModeS = D3D11_TEXTURE_ADDRESS_WRAP;
  else if (model.samplers[smplrId].wrapS == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE)
    addressModeS = D3D11_TEXTURE_ADDRESS_CLAMP;
  else if (model.samplers[smplrId].wrapS == TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT)
    addressModeS = D3D11_TEXTURE_ADDRESS_MIRROR;
  else
    return E_FAIL;

  if (model.samplers[smplrId].wrapT == TINYGLTF_TEXTURE_WRAP_REPEAT)
    addressModeT = D3D11_TEXTURE_ADDRESS_WRAP;
  else if (model.samplers[smplrId].wrapT == TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE)
    addressModeT = D3D11_TEXTURE_ADDRESS_CLAMP;
  else if (model.samplers[smplrId].wrapT == TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT)
    addressModeT = D3D11_TEXTURE_ADDRESS_MIRROR;
  else
    return E_FAIL;
  
  D3D11_FILTER filter;
  if (model.samplers[smplrId].minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR)
    filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  else
    return E_FAIL;

  // Init samplers
  D3D11_SAMPLER_DESC descSmplr = {};

  descSmplr.Filter = filter;
  descSmplr.AddressU = addressModeS;
  descSmplr.AddressV = addressModeT;
  descSmplr.AddressW = addressModeT;
  descSmplr.MinLOD = 0;
  descSmplr.MaxLOD = D3D11_FLOAT32_MAX;
  descSmplr.MipLODBias = 0.0f;

  HRESULT hr = device->CreateSamplerState(&descSmplr, &(g_pSamplers[smplrId]));
  return hr;
}

void Model::InitMaterialsFromMetadata() {
  gltfTextures = std::vector<GLTFTexture>(model.textures.size(), { -1, -1});
  for (int i = 0; i < model.textures.size(); i++)
    gltfTextures[i] = { model.textures[i].source, model.textures[i].sampler };

  gltfMaterials = std::vector<GLTFMaterial>(model.materials.size(), {-1, -1, -1});
  for (int i = 0; i < model.materials.size(); i++)
    gltfMaterials[i] = { model.materials[i].pbrMetallicRoughness.baseColorTexture.index, 
                         model.materials[i].pbrMetallicRoughness.metallicRoughnessTexture.index,
                         model.materials[i].normalTexture.index};

  meshMaterislIdx = std::vector<int>(model.meshes.size(), -1);
  for (int i = 0; i < model.meshes.size(); i++)
    meshMaterislIdx[i] = model.meshes[i].primitives[0].material;
}

HRESULT Model::InitBuffersFromFile(ID3D11Device* device, FILE* binFile) {
  g_pVertexBuffers = std::vector<ID3D11Buffer*>(model.meshes.size(), nullptr);
  g_pIndexBuffers = std::vector<ID3D11Buffer*>(model.meshes.size(), nullptr);
  indeciesLenghts = std::vector<size_t>(model.meshes.size(), 0);

  HRESULT hr = S_OK;
  for (int i = 0; i < model.meshes.size(); i++) {
    hr = LoadMesh(device, binFile, i);
    if (FAILED(hr))
      break;
  }

  return hr;
}


HRESULT Model::LoadMesh(ID3D11Device* device, FILE* binFile, size_t meshId) {
  // Load vertecies data
  int posAccInd     = model.meshes[meshId].primitives[0].attributes.at("POSITION");
  int normAccInd    = model.meshes[meshId].primitives[0].attributes.at("NORMAL");
  int tangentAccInd = model.meshes[meshId].primitives[0].attributes.at("TANGENT");
  int texUVAccInd   = model.meshes[meshId].primitives[0].attributes.at("TEXCOORD_0");

  std::vector<XMFLOAT3> posVec = std::vector<XMFLOAT3>(0, XMFLOAT3(0, 0, 0));
  std::vector<XMFLOAT3> normVec = std::vector<XMFLOAT3>(0, XMFLOAT3(0, 0, 0));
  std::vector<XMFLOAT3> tanVec = std::vector<XMFLOAT3>(0, XMFLOAT3(0, 0, 0));
  std::vector<XMFLOAT2> texVec = std::vector<XMFLOAT2>(0, XMFLOAT2(0, 0));

  HRESULT hr = LoadSpecificTypeArray(model.accessors[posAccInd], binFile, posVec);
  if (FAILED(hr))
    return hr;

  hr = LoadSpecificTypeArray(model.accessors[normAccInd], binFile, normVec);
  if (FAILED(hr))
    return hr;

  hr = LoadSpecificTypeArray(model.accessors[tangentAccInd], binFile, tanVec);
  if (FAILED(hr))
    return hr;

  hr = LoadSpecificTypeArray(model.accessors[texUVAccInd], binFile, texVec);
  if (FAILED(hr))
    return hr;

  // Concatenate them into one array of Vertex
  std::vector<Vertex> verticies;
  hr = GenerateVerticiesArray(posVec, normVec, tanVec, texVec, verticies);
  if (FAILED(hr))
    return hr;

  // Init vertex buffer
  hr = InitVerticiesBuffer(device, verticies, meshId);
  if (FAILED(hr))
    return hr;

  // Load indexes array
  size_t indAccInd = model.meshes[meshId].primitives[0].indices;
  if (model.accessors[indAccInd].componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
    std::vector<unsigned int> indicies;
    hr = GetDataFromFile(model.accessors[indAccInd], binFile, indicies);
    if (FAILED(hr))
      return hr;
    hr = InitIndeciesBuffer(device, indicies, meshId);
    if (FAILED(hr))
      return hr;
    indeciesLenghts[meshId] = indicies.size();
  }
  else if (model.accessors[indAccInd].componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
    std::vector<unsigned short> indicies;
    hr = GetDataFromFile(model.accessors[indAccInd], binFile, indicies);
    if (FAILED(hr))
      return hr;
    hr = InitIndeciesBuffer(device, indicies, meshId);
    if (FAILED(hr))
      return hr;
    indeciesLenghts[meshId] = indicies.size();
  }
  else if (model.accessors[indAccInd].componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
    std::vector<short> indicies;
    hr = GetDataFromFile(model.accessors[indAccInd], binFile, indicies);
    if (FAILED(hr))
      return hr;
    hr = InitIndeciesBuffer(device, indicies, meshId);
    if (FAILED(hr))
      return hr;
    indeciesLenghts[meshId] = indicies.size();
  }
  else
    return E_FAIL;

  return S_OK;
}

template<typename VertexType>
HRESULT Model::LoadSpecificTypeArray(const tinygltf::Accessor& accessor, FILE* binFile, std::vector<VertexType>& verticiesToLoad) {
  HRESULT hr = S_OK;
  if (accessor.type == TINYGLTF_TYPE_SCALAR) {
    std::vector<float> dataToLoad;
    hr = GetDataFromFile(accessor, binFile, dataToLoad);
    if (FAILED(hr))
      return hr;

    verticiesToLoad = std::vector<VertexType>(dataToLoad.size());
    for (int i = 0; i < dataToLoad.size(); i++)
      std::memcpy(&(verticiesToLoad[i]), &(dataToLoad[i]), min(sizeof(float), sizeof(VertexType)));
  }
  else if (accessor.type == TINYGLTF_TYPE_VEC2) {
    std::vector<XMFLOAT2> dataToLoad;
    hr = GetDataFromFile(accessor, binFile, dataToLoad);
    if (FAILED(hr))
      return hr;

    verticiesToLoad = std::vector<VertexType>(dataToLoad.size());
    float minX = 100, minY = 100;
    for (int i = 0; i < dataToLoad.size(); i++) {
      std::memcpy(&(verticiesToLoad[i]), &(dataToLoad[i]), min(sizeof(XMFLOAT2), sizeof(VertexType)));
      minX = minX > dataToLoad[i].x ? dataToLoad[i].x : minX;
      minY = minY > dataToLoad[i].y ? dataToLoad[i].y : minY;
    }
    float huynya = 1000;
    minX = huynya;
  }
  else if (accessor.type == TINYGLTF_TYPE_VEC3) {
    std::vector<XMFLOAT3> dataToLoad;
    hr = GetDataFromFile(accessor, binFile, dataToLoad);
    if (FAILED(hr))
      return hr;

    verticiesToLoad = std::vector<VertexType>(dataToLoad.size());
    for (int i = 0; i < dataToLoad.size(); i++)
      std::memcpy(&(verticiesToLoad[i]), &(dataToLoad[i]), min(sizeof(XMFLOAT3), sizeof(VertexType)));
  }
  else if (accessor.type == TINYGLTF_TYPE_VEC4) {
    std::vector<XMFLOAT4> dataToLoad;
    hr = GetDataFromFile(accessor, binFile, dataToLoad);
    if (FAILED(hr))
      return hr;

    verticiesToLoad = std::vector<VertexType>(dataToLoad.size());
    for (int i = 0; i < dataToLoad.size(); i++)
      std::memcpy(&(verticiesToLoad[i]), &(dataToLoad[i]), min(sizeof(XMFLOAT4), sizeof(VertexType)));
  }
  else
    return E_FAIL; // UNKNOWN DATA TYPE

  return hr;
}

template<typename DataType>
HRESULT Model::GetDataFromFile(const tinygltf::Accessor& accessor, FILE* binFile, std::vector<DataType>& dataToLoad) {
  size_t buffViewInd = accessor.bufferView;
  size_t count = accessor.count;
  size_t accByteOffset = accessor.byteOffset;
  size_t type = accessor.type;

  size_t byteOffset = model.bufferViews[buffViewInd].byteOffset;

  size_t offset = byteOffset + accByteOffset;
  size_t totalCount = count;

  fseek(binFile, offset, SEEK_SET);

  dataToLoad = std::vector<DataType>(totalCount);
  auto readed = fread(&(dataToLoad[0]), sizeof(DataType), totalCount, binFile);
  if (readed != totalCount)
    return E_FAIL;
  return S_OK;
}


HRESULT Model::GenerateVerticiesArray(std::vector<XMFLOAT3>& posVec, std::vector<XMFLOAT3>& normVec, std::vector<XMFLOAT3>& tangentVec, std::vector<XMFLOAT2>& texUVVec, std::vector<Vertex>& verticiesRes) {
  size_t vecSize = posVec.size();
  if (vecSize == 0)
    return E_FAIL;

  verticiesRes = std::vector<Vertex>(vecSize);
  for (int i = 0; i < vecSize; i++) {
    verticiesRes[i].pos = posVec[i];
    verticiesRes[i].norm = normVec.size() > i ? normVec[i] : XMFLOAT3(0, 0, 0);
    verticiesRes[i].tangent = tangentVec.size() > i ? tangentVec[i] : XMFLOAT3(0, 0, 0);
    verticiesRes[i].texUV = texUVVec.size() > i ? texUVVec[i] : XMFLOAT2(0, 0);

    verticiesRes[i].pos.x *= -1, verticiesRes[i].norm.x *= -1, verticiesRes[i].tangent.x *= -1;
  }
  return S_OK;
}


template<typename IndexType>
HRESULT Model::InitIndeciesBuffer(ID3D11Device* device, std::vector<IndexType>& indicies, size_t bufferIndex) {
  D3D11_BUFFER_DESC descInd = {};
  ZeroMemory(&descInd, sizeof(descInd));

  descInd.ByteWidth = sizeof(IndexType) * indicies.size();
  descInd.Usage = D3D11_USAGE_IMMUTABLE;
  descInd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  descInd.CPUAccessFlags = 0;
  descInd.MiscFlags = 0;
  descInd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA dataInd;
  dataInd.pSysMem = &indicies[0];

  HRESULT hr = device->CreateBuffer(&descInd, &dataInd, &(g_pIndexBuffers[bufferIndex]));
  return hr;
}

HRESULT Model::InitVerticiesBuffer(ID3D11Device* device, std::vector<Vertex>& verticies, size_t bufferIndex) {
  D3D11_BUFFER_DESC desc = {};
  desc.ByteWidth = sizeof(Vertex) * verticies.size();
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA data;
  ZeroMemory(&data, sizeof(data));
  data.pSysMem = &verticies[0];
  HRESULT hr = device->CreateBuffer(&desc, &data, &(g_pVertexBuffers[bufferIndex]));
  return hr;
}


HRESULT Model::InitShadersPipeline(ID3D11Device* device) {
  // Create index array
  static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  // Compile shaders
  ID3D10Blob* vertexShaderBuffer = nullptr;
  ID3D10Blob* pixelShaderBuffer = nullptr;

  HRESULT hr = CompileShaderFromFile(L"pbrLightable_VS.hlsl", "main", "vs_5_0", &vertexShaderBuffer);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &g_pVertexShader);
  if (FAILED(hr))
  {
    vertexShaderBuffer->Release();
    return hr;
  }

  // Compile the pixel shader
  hr = CompileShaderFromFile(L"pbrLightable_PS.hlsl", "main", "ps_5_0", &pixelShaderBuffer);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &g_pPixelShader);
  pixelShaderBuffer->Release();
  if (FAILED(hr))
    return hr;

  int numElements = sizeof(InputDesc) / sizeof(InputDesc[0]);
  hr = device->CreateInputLayout(InputDesc, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &g_pVertexLayout);
  if (FAILED(hr))
    return hr;

  vertexShaderBuffer->Release();
}

HRESULT Model::InitConstantBuffersFromlMetadata(ID3D11Device* device) {
  // Init all ConstantBuffers as XMMatrixIdentity(); matricies
  meshesWM = std::vector<WorldMatrixBuffer>(model.meshes.size(), { XMMatrixIdentity(), 
                                                                   XMFLOAT4(PBRParams.roughness, PBRParams.metalness, PBRParams.dielectricF0, 0.0f),
                                                                   XMFLOAT4(PBRParams.albedo.x, PBRParams.albedo.y, PBRParams.albedo.z, 0.0f) });
  g_pWMBuffers = std::vector<ID3D11Buffer*>(model.meshes.size(), nullptr);

  // Go throw all nodes and save transformation
  for (auto& nodeId : model.scenes[model.defaultScene].nodes) {
    XMMATRIX startedMatr = XMMatrixIdentity();
    CountMatrixTransformation(nodeId, startedMatr);
  }

  // Set constant buffers
  HRESULT hr = S_OK;
  for (int i = 0; i < model.meshes.size(); i++) {
    D3D11_BUFFER_DESC descWM = {};
    descWM.ByteWidth = sizeof(WorldMatrixBuffer);
    descWM.Usage = D3D11_USAGE_DEFAULT;
    descWM.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    descWM.CPUAccessFlags = 0;
    descWM.MiscFlags = 0;
    descWM.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &(meshesWM[i]);
    data.SysMemPitch = sizeof(meshesWM[i]);
    data.SysMemSlicePitch = 0;

    hr = device->CreateBuffer(&descWM, &data, &(g_pWMBuffers[i]));
    if (FAILED(hr))
      return hr;
  }

  D3D11_BUFFER_DESC descSM = {};
  descSM.ByteWidth = sizeof(SceneMatrixBuffer);
  descSM.Usage = D3D11_USAGE_DYNAMIC;
  descSM.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descSM.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  descSM.MiscFlags = 0;
  descSM.StructureByteStride = 0;

  hr = device->CreateBuffer(&descSM, nullptr, &g_pSMBuffer);
  if (FAILED(hr))
    return hr;

  return S_OK;
}

void Model::CountMatrixTransformation(int nodeId, const XMMATRIX& parentTransformation) {
  XMMATRIX currentTransformation = parentTransformation;
  if (model.nodes[nodeId].matrix.size() == 16)
    currentTransformation *= XMMATRIX(model.nodes[nodeId].matrix[0],  model.nodes[nodeId].matrix[1],  model.nodes[nodeId].matrix[2],  model.nodes[nodeId].matrix[3], 
                                      model.nodes[nodeId].matrix[4],  model.nodes[nodeId].matrix[5],  model.nodes[nodeId].matrix[6],  model.nodes[nodeId].matrix[7], 
                                      model.nodes[nodeId].matrix[8],  model.nodes[nodeId].matrix[9],  model.nodes[nodeId].matrix[10], model.nodes[nodeId].matrix[11],
                                      model.nodes[nodeId].matrix[12], model.nodes[nodeId].matrix[13], model.nodes[nodeId].matrix[14], model.nodes[nodeId].matrix[15]);

  if (model.nodes[nodeId].translation.size() == 3)
    currentTransformation *= XMMatrixTranslation(model.nodes[nodeId].translation[0], model.nodes[nodeId].translation[1], model.nodes[nodeId].translation[2]);

  /*if (model.nodes[nodeId].rotation.size() == 4) {
    XMFLOAT4 quatFloat = { model.nodes[nodeId].rotation[0], model.nodes[nodeId].rotation[1], model.nodes[nodeId].rotation[2], model.nodes[nodeId].rotation[3] };
    XMVECTOR quat = XMLoadFloat4(&quatFloat);
    currentTransformation *= XMMatrixRotationQuaternion(quat);
  }*/

  if (model.nodes[nodeId].scale.size() == 3)
    currentTransformation *= XMMatrixScaling(model.nodes[nodeId].scale[0], model.nodes[nodeId].scale[1], model.nodes[nodeId].scale[2]);

  if (model.nodes[nodeId].mesh != -1)
    meshesWM[model.nodes[nodeId].mesh].worldMatrix = currentTransformation;

  for (auto& childID : model.nodes[nodeId].children)
    CountMatrixTransformation(childID, currentTransformation);
}

HRESULT Model::InitDX11Vars(ID3D11Device* device) {
  // Init samplers
  D3D11_SAMPLER_DESC envSmplr = {};

  envSmplr.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  envSmplr.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  envSmplr.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  envSmplr.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  envSmplr.MinLOD = 0;
  envSmplr.MaxLOD = D3D11_FLOAT32_MAX;
  envSmplr.MipLODBias = 0.0f;

  HRESULT hr = device->CreateSamplerState(&envSmplr, &g_pEnvSamplerState);
  if (FAILED(hr))
    return hr;

  D3D11_SAMPLER_DESC descBRDFSmplr = {};

  descBRDFSmplr.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  descBRDFSmplr.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  descBRDFSmplr.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  descBRDFSmplr.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  descBRDFSmplr.MinLOD = 0;
  descBRDFSmplr.MaxLOD = D3D11_FLOAT32_MAX;
  descBRDFSmplr.MipLODBias = 0.0f;

  hr = device->CreateSamplerState(&descBRDFSmplr, &g_pBRDFSamplerState);
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
}

HRESULT Model::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  // Load metadata about whole scene(s)
  HRESULT hr = LoadGLTFModelMetadata();
  if (FAILED(hr))
    return hr;

  // Init textures from file
  hr = InitTexturesFromlMetadata(device);
  if (FAILED(hr))
    return hr;

  // Init samplers from file
  hr = InitSamplersFromlMetadata(device);
  if (FAILED(hr))
    return hr;

  InitMaterialsFromMetadata();

  // Init buffers with transforms for meshs
  hr = InitConstantBuffersFromlMetadata(device);
  if (FAILED(hr))
    return hr;

  // Open filestream with scene data
  FILE *binData;
  if ((binData = fopen(m_binfile.c_str(), "rb")) == NULL)
    return E_FAIL;

  // Init buffers from file
  hr = InitBuffersFromFile(device, binData);
  fclose(binData);
  if (FAILED(hr))
    return hr;

  // Init shaders' pipeline
  hr = InitShadersPipeline(device);
  if (FAILED(hr))
    return hr;

  hr = InitDX11Vars(device);
  if (FAILED(hr))
    return hr;

  return S_OK;
}

void Model::Release() {
  for (auto& buffer : g_pVertexBuffers)
    if (buffer) buffer->Release();

  for (auto& buffer : g_pIndexBuffers)
    if (buffer) buffer->Release();

  for (auto& buffer : g_pWMBuffers)
    if (buffer) buffer->Release();

  for (auto& textureSRV : g_pTexturesSRV)
    if (textureSRV) textureSRV->Release();

  for (auto& texture : g_pTextures)
    if (texture) texture->Release();

  for (auto& smplr : g_pSamplers)
    if (smplr) smplr->Release();

  if (g_pSMBuffer) g_pSMBuffer->Release();
  if (g_pPixelShader) g_pPixelShader->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pVertexLayout) g_pVertexLayout->Release();

  if (g_pRasterizerState) g_pRasterizerState->Release();
  if (g_pEnvSamplerState) g_pEnvSamplerState->Release();
  if (g_pBRDFSamplerState) g_pBRDFSamplerState->Release();
}

void Model::Render(ID3D11DeviceContext* context) {
  for (int i = 0; i < model.meshes.size(); i++) {
    beginEvent((std::wstring(L"Drawing mesh #") + std::to_wstring(i + 1)).c_str());

    context->RSSetState(g_pRasterizerState);

    context->IASetIndexBuffer(g_pIndexBuffers[i], DXGI_FORMAT_R32_UINT, 0);

    ID3D11Buffer* vertexBuffers[] = { g_pVertexBuffers[i] };
    UINT strides[] = { sizeof(Vertex) };
    UINT offsets[] = { 0 };

    context->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    context->IASetInputLayout(g_pVertexLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // TODO : написать метод по выбору исходя из формата меша
    
    context->VSSetShader(g_pVertexShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &g_pWMBuffers[i]);
    context->VSSetConstantBuffers(1, 1, &g_pSMBuffer);

    context->PSSetShader(g_pPixelShader, nullptr, 0);
    context->PSSetConstantBuffers(0, 1, &g_pWMBuffers[i]);
    context->PSSetConstantBuffers(1, 1, &g_pSMBuffer);

    // TODO: set textures
    if (meshMaterislIdx[i] != -1) {
      auto material = gltfMaterials[meshMaterislIdx[i]];
      if (material.metalnessTexId != -1) {
        context->PSSetShaderResources(0, 1, &g_pTexturesSRV[gltfTextures[material.metalnessTexId].texId]);
        context->PSSetSamplers(0, 1, &g_pSamplers[gltfTextures[material.metalnessTexId].samplerId]);
      }
      if (material.normalTexId != -1) {
        context->PSSetShaderResources(1, 1, &g_pTexturesSRV[gltfTextures[material.normalTexId].texId]);
        context->PSSetSamplers(1, 1, &g_pSamplers[gltfTextures[material.normalTexId].samplerId]);
      }
      if (material.diffTexId != -1) {
        context->PSSetShaderResources(2, 1, &g_pTexturesSRV[gltfTextures[material.diffTexId].texId]);
        context->PSSetSamplers(2, 1, &g_pSamplers[gltfTextures[material.diffTexId].samplerId]);
      }
    }

    // set env params
    context->PSSetShaderResources(3, 1, &maps.pIRRMapSRV);
    context->PSSetShaderResources(4, 1, &maps.pPrefilMapSRV);
    context->PSSetShaderResources(5, 1, &maps.pBRDFMapSRV);
    context->PSSetSamplers(3, 1, &g_pEnvSamplerState);
    context->PSSetSamplers(4, 1, &g_pBRDFSamplerState);
    
    
    context->DrawIndexed(indeciesLenghts[i], 0, 0);

    endEvent();
  }
}

HRESULT Model::Update(ID3D11DeviceContext* context, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, XMVECTOR& cameraPos, const std::vector<Light>& lights, PBRRichMaterial pbrMaterial, ViewMode viewMode) {
  // Update world matrix angle of first cube
  WorldMatrixBuffer worldMatrixBuffer;
  for (int i = 0; i < model.meshes.size(); i++) {
    worldMatrixBuffer.worldMatrix = meshesWM[i].worldMatrix;
    worldMatrixBuffer.pbrParams = XMFLOAT4(pbrMaterial.roughness, pbrMaterial.metalness, pbrMaterial.dielectricF0, 0.0);// pbrMaterial;
    worldMatrixBuffer.albedo = XMFLOAT4(pbrMaterial.albedo.x, pbrMaterial.albedo.y, pbrMaterial.albedo.z, 0.0);
    
    context->UpdateSubresource(g_pWMBuffers[i], 0, nullptr, &worldMatrixBuffer, 0, 0);
  }
  
  // Get the view matrix
  D3D11_MAPPED_SUBRESOURCE subresource;
  HRESULT hr = context->Map(g_pSMBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
  if (FAILED(hr))
    return FAILED(hr);

  SceneMatrixBuffer& sceneBuffer = *reinterpret_cast<SceneMatrixBuffer*>(subresource.pData);
  sceneBuffer.viewMode = XMFLOAT4(viewMode.modelViewMode, viewMode.isPlainNormal, viewMode.isPlainMetalRough, viewMode.isPlainColor);
  sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
  sceneBuffer.cameraPos = XMFLOAT4(XMVectorGetX(cameraPos), XMVectorGetY(cameraPos), XMVectorGetZ(cameraPos), 1.0f);
  sceneBuffer.lightCount = XMINT4((int32_t)lights.size(), 0, 0, 0);
  for (int i = 0; i < lights.size(); i++) {
    sceneBuffer.lightPos[i] = lights[i].GetLightPosition();
    sceneBuffer.lightColor[i] = lights[i].GetLightColor();
  }

  context->Unmap(g_pSMBuffer, 0);

  return S_OK;
}


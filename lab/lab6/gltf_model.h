#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "rendered.h"
#include "materials.h"
#include "common.h"
#include "light.h"
#include "skybox.h"
#include "../libs/tiny_gltf.h"

#define MAX_LIGHT_SOURCES 10



class Model : public Rendered {
public:
  Model() = default;

  Model(const std::string& gltffile, 
    const std::string& binfile, 
    Skybox& sb, 
    PBRRichMaterial richPBRParams) {
    m_gltffile = gltffile;
    m_binfile = binfile;

    m_modelpath = gltffile.substr(0, gltffile.find_last_of("/\\"));
    maps = sb.GetMaps();
    PBRParams = richPBRParams;
  };

  void SetIBLMaps(const IBLMaps& _maps) {
    maps = _maps;
  };

  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);
  void Release();
  void Render(ID3D11DeviceContext* context);
  HRESULT Update(ID3D11DeviceContext* context, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, XMVECTOR& cameraPos, const std::vector<Light>& lights, PBRRichMaterial pbrMaterial, ViewMode viewMode);

private:
  struct Vertex
  {
    XMFLOAT3 pos;          // positional coords
    XMFLOAT3 norm;         // normal vec
    XMFLOAT3 tangent;      // tangent vec
    XMFLOAT2 texUV;        // texture coord
  };

  // loading metadata method
  HRESULT LoadGLTFModelMetadata();

  // methods to init textures and samplers
  HRESULT InitTexturesFromlMetadata(ID3D11Device* device);
  HRESULT InitTexture(ID3D11Device* device, size_t imgId);
  HRESULT InitSamplersFromlMetadata(ID3D11Device* device);
  HRESULT InitSampler(ID3D11Device* device, size_t smplrId);

  // Methods to init materials structures
  struct GLTFTexture {
    int texId = -1;
    int samplerId = -1;
  };
  struct GLTFMaterial {
    int diffTexId = -1;
    int metalnessTexId = -1;
    int normalTexId = -1;
  };
  void  InitMaterialsFromMetadata();

  // methods to init mesh buffers
  HRESULT InitBuffersFromFile(ID3D11Device* device, FILE* binFile);
  HRESULT LoadMesh(ID3D11Device* device, FILE* binFile, size_t meshId);
  // - methods to load *.GLTF type buffers and convert them to ours
  template<typename ArrayType>
  HRESULT LoadSpecificTypeArray(const tinygltf::Accessor& accessor, FILE* binFile, std::vector<ArrayType>& verticiesToLoad);
  template<typename DataType>
  HRESULT GetDataFromFile(const tinygltf::Accessor& accessor, FILE* binFile, std::vector<DataType>& dataToLoad);
  // - methods to init buffers
  HRESULT GenerateVerticiesArray(std::vector<XMFLOAT3>& posVec, std::vector<XMFLOAT3>& normVec, std::vector<XMFLOAT3>& tangentVec, std::vector<XMFLOAT2>& texUVVec, std::vector<Vertex>& verticiesRes);
  template<typename IndexType>
  HRESULT InitIndeciesBuffer(ID3D11Device* device, std::vector<IndexType>& indicies, size_t bufferIndex);
  HRESULT InitVerticiesBuffer(ID3D11Device* device, std::vector<Vertex>& verticies, size_t bufferIndex);

  // methods to init constant buffers for verticies transforms of meshes
  struct SceneMatrixBuffer {
    XMMATRIX viewProjectionMatrix;
    XMFLOAT4 cameraPos;
    XMINT4 lightCount;
    XMFLOAT4 lightPos[MAX_LIGHT_SOURCES];
    XMFLOAT4 lightColor[MAX_LIGHT_SOURCES];
    XMFLOAT4 viewMode;
  };

  struct WorldMatrixBuffer {
    XMMATRIX worldMatrix;
    XMFLOAT4 pbrParams;
    XMFLOAT4 albedo;
  };
  HRESULT InitConstantBuffersFromlMetadata(ID3D11Device* device);
  void CountMatrixTransformation(int nodeId, const XMMATRIX& parentTransformation);

  // methods to init shaders
  HRESULT InitShadersPipeline(ID3D11Device* device);

  // Method to init other Dx11 stuff
  HRESULT InitDX11Vars(ID3D11Device* device);

  // paths and filenames
  std::string m_gltffile;
  std::string m_binfile;
  std::string m_modelpath;
  tinygltf::Model model;

  // var for outer resources (no need to release them)
  IBLMaps maps;
  PBRRichMaterial PBRParams;

  // dx11 vars for shaders
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;
  ID3D11InputLayout* g_pVertexLayout = nullptr;

  // dx11 vars for buffers
  ID3D11Buffer* g_pSMBuffer = nullptr;
  std::vector<WorldMatrixBuffer> meshesWM = std::vector<WorldMatrixBuffer>(0);
  std::vector<ID3D11Buffer*> g_pWMBuffers = std::vector<ID3D11Buffer*>(0, nullptr);
  std::vector<ID3D11Buffer*> g_pVertexBuffers = std::vector<ID3D11Buffer*>(0, nullptr);
  std::vector<ID3D11Buffer*> g_pIndexBuffers = std::vector<ID3D11Buffer*>(0, nullptr);
  std::vector<size_t> indeciesLenghts = std::vector<size_t>(0, 0);
  std::vector<size_t> materialsIdxs = std::vector<size_t>(0, 0);

  // Vars for managment fast access to textures
  std::vector<GLTFTexture> gltfTextures = std::vector<GLTFTexture>(0);
  std::vector<GLTFMaterial> gltfMaterials = std::vector<GLTFMaterial>(0);
  std::vector<int> meshMaterislIdx = std::vector<int>(0);

  // dx11 vars for textures and samplers
  std::vector<ID3D11Texture2D*> g_pTextures = std::vector<ID3D11Texture2D*>(0, nullptr);
  std::vector<ID3D11SamplerState*> g_pSamplers = std::vector<ID3D11SamplerState*>(0, nullptr);
  std::vector<ID3D11ShaderResourceView*> g_pTexturesSRV = std::vector<ID3D11ShaderResourceView*>(0, nullptr);

  ID3D11RasterizerState* g_pRasterizerState = nullptr;
  ID3D11SamplerState* g_pEnvSamplerState = nullptr;
  ID3D11SamplerState* g_pBRDFSamplerState = nullptr;
};

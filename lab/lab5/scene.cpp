#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  int square_size = 1;
  HRESULT hr = S_OK;

  //sb = Skybox(L"skybox.dds", 30, 30);
  sb = Skybox(L"wildflower_field_4k.hdr", 30, 30);
  hr = sb.Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;
  maps = sb.GetMaps();
  pbrMaterial.metalness = 0.1f;
  pbrMaterial.roughness = 0.1f;
  pbrMaterial.albedo = XMFLOAT3(0.25f, 0.25f, 0.25f);

  spheres.resize(square_size * square_size);
  for (int x = 0; x < square_size; x++)
    for (int y = 0; y < square_size; y++) {
      spheres[x * square_size + y] = Sphere(1.5f, XMFLOAT3(0, 0.0f + 1.5f * x, 0.0f + 1.5f * y), sb,
        XMFLOAT3(0.1f * x, 0.1f * y, 0.025f), 0, 0, 50, 50);
      hr = spheres[x * square_size + y].Init(device, context, screenWidth, screenHeight);
      if (FAILED(hr))
        return hr;

      spheres[x * square_size + y].SetIBLMaps(maps);
    }

  // Init lights
  lights.reserve(1);
  lights.push_back(Light(XMFLOAT4(1.f, 0.0f, 0.0f, 0.3f), 2.0f, 2.0f, 2.0f));
  
  hr = lights[0].Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

#ifdef _DEBUG
  hr = context->QueryInterface(__uuidof(pAnnotation), reinterpret_cast<void**>(&pAnnotation));
  if (FAILED(hr))
    return hr;
#endif

  return hr;
}

void Scene::ProvideInput(const Input& input) {
  for (auto& light : lights)
    light.ProvideInput(input);

  for (auto& sphere : spheres)
    sphere.ProvideInput(input);
}

void Scene::Release() {
#ifdef _DEBUG
  if (pAnnotation) pAnnotation->Release();
#endif
  sb.Release();

  for (auto& sphere : spheres)
    sphere.Release();
  
  for (auto& light : lights)
    light.Release();
}

void Scene::Render(ID3D11DeviceContext* context) {
  sb.Render(context);

#ifdef _DEBUG
  pAnnotation->BeginEvent((LPCWSTR)L"Draw Spheres");
  std::string indexBufferName = "Indexes buffer", vertexBufferName = "Vertexes buffer";
#endif
  for (auto& sphere : spheres)
    sphere.Render(context);
#ifdef _DEBUG
  pAnnotation->EndEvent();
#endif

  for (auto& light : lights)
    light.Render(context);
}

bool Scene::Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
  sb.Update(context, viewMatrix, projectionMatrix, XMFLOAT3(XMVectorGetX(cameraPos), XMVectorGetY(cameraPos), XMVectorGetZ(cameraPos)));

  for (auto& light : lights) {
    light.Update(context, viewMatrix, projectionMatrix, cameraPos);
    if (isOff)
      light.GetLightColorRef()->w = 0.0f;
    else
      light.GetLightColorRef()->w = intensity;
  }
  
  for (auto& sphere : spheres)
    sphere.Update(context, viewMatrix, projectionMatrix, cameraPos, lights, pbrMaterial, pbrMode, iblMode);

  return true;
}

void Scene::Resize(int screenWidth, int screenHeight) {
  sb.Resize(screenWidth, screenHeight);
};

void Scene::RenderGUI() {
  /*
  static bool show = true;
  ImGui::ShowDemoWindow(&show);
  */

  // Generate window
  ImGui::Begin("Scene params");

  // Enum pbr mode
  ImGui::Text("PBR params");
  ImGui::RadioButton("Full", reinterpret_cast<int*>(&pbrMode), static_cast<int>(PBRMode::allPBR));
  ImGui::RadioButton("Normal distribution", reinterpret_cast<int*>(&pbrMode), static_cast<int>(PBRMode::normal));
  ImGui::RadioButton("Geometry", reinterpret_cast<int*>(&pbrMode), static_cast<int>(PBRMode::geom));
  ImGui::RadioButton("Fresnel", reinterpret_cast<int*>(&pbrMode), static_cast<int>(PBRMode::fresnel));

  ImGui::Text("Switching diffuse/specular components");
  ImGui::RadioButton("Full IBL", reinterpret_cast<int*>(&iblMode), static_cast<int>(IBLMode::full));
  ImGui::RadioButton("Diffuse only", reinterpret_cast<int*>(&iblMode), static_cast<int>(IBLMode::diffuse));
  ImGui::RadioButton("Specular only", reinterpret_cast<int*>(&iblMode), static_cast<int>(IBLMode::specular));
  ImGui::RadioButton("No IBL", reinterpret_cast<int*>(&iblMode), static_cast<int>(IBLMode::nothing));
  
  // PBR Materials params
  ImGui::Text("Sphere materials params");
  ImGui::ColorEdit3("Albedo", &((&pbrMaterial.albedo)->x));
  ImGui::SliderFloat("Roughness", &pbrMaterial.roughness, 0, 1);
  ImGui::SliderFloat("Metalness ", &pbrMaterial.metalness, 0, 1);

  ImGui::Text("Lights params");
  ImGui::Checkbox("Off light", &isOff);
  for (int i = 0; i < lights.size(); i++) {
    ImGui::Text((std::string("Light-") + std::to_string(i + 1)).c_str());
    ImGui::ColorEdit3("Color", &lights[i].GetLightColorRef()->x);
    ImGui::SliderFloat("Intensity", &intensity, 0, 10.0f);
    ImGui::SliderFloat("Pos-X", &lights[i].GetLightPositionRef()->x, -100.f, 100.f);
    ImGui::SliderFloat("Pos-Y", &lights[i].GetLightPositionRef()->y, -100.f, 100.f);
    ImGui::SliderFloat("Pos-Z", &lights[i].GetLightPositionRef()->z, -100.f, 100.f);
  }

  ImGui::End();
}

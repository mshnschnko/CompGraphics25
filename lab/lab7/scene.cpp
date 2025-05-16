#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  HRESULT hr = S_OK;

  //sb = Skybox(L"skybox.dds", 30, 30);
  sb = Skybox(L"wildflower_field_4k.hdr", 30, 30);
  hr = sb.Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;
  maps = sb.GetMaps();

  // Init model
  pbrMaterial = PBRRichMaterial(0.2, 0.3, 0.04, XMFLOAT3(1, 1, 1));
  model = Model("model/scene.gltf", "model/scene.bin", sb, pbrMaterial);
  hr = model.Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  // Init lights
  lights.reserve(1);
  lights.push_back(Light(XMFLOAT4(1.f, 0.0f, 0.0f, 0.3f), 2.0f, 2.0f, 2.0f));
  
  hr = lights[0].Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  return hr;
}

void Scene::ProvideInput(const Input& input) {
  for (auto& light : lights)
    light.ProvideInput(input);
}

void Scene::Release() {
  sb.Release();

  model.Release();
  
  for (auto& light : lights)
    light.Release();
}

void Scene::Render(ID3D11DeviceContext* context) {
  sb.Render(context);

  beginEvent(L"Drawing model");
  model.Render(context);
  endEvent();

  for (auto& light : lights)
    light.Render(context);
}

bool Scene::Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
  sb.Update(context, viewMatrix, projectionMatrix, XMFLOAT3(XMVectorGetX(cameraPos), XMVectorGetY(cameraPos), XMVectorGetZ(cameraPos)));

  model.Update(context, viewMatrix, projectionMatrix, cameraPos, lights, pbrMaterial, viewMode);

  for (auto& light : lights) {
    light.Update(context, viewMatrix, projectionMatrix, cameraPos);
    if (isOff)
      light.GetLightColorRef()->w = 0.0f;
    else
      light.GetLightColorRef()->w = intensity;
  }
  
  return true;
}

void Scene::Resize(int screenWidth, int screenHeight) {
  sb.Resize(screenWidth, screenHeight);
};

void Scene::RenderGUI() {
  // Generate window
  ImGui::Begin("Scene params");

  // PBR Materials params
  ImGui::Text("Sphere materials params");
  ImGui::SliderFloat("Roughness", &pbrMaterial.roughness, 0, 1);
  ImGui::SliderFloat("Metalness ", &pbrMaterial.metalness, 0, 1);
  ImGui::SliderFloat("DielectricF0 ", &pbrMaterial.dielectricF0, 0, 0.1);
  ImGui::ColorEdit3("Albedo", &((&pbrMaterial.albedo)->x));

  ImGui::Text("View mode controlling");
  ImGui::RadioButton("Full model", reinterpret_cast<int*>(&viewMode.modelViewMode), static_cast<int>(ModelViewMode::all));
  ImGui::RadioButton("Normals", reinterpret_cast<int*>(&viewMode.modelViewMode), static_cast<int>(ModelViewMode::normal));
  ImGui::RadioButton("Roughness/Metalness", reinterpret_cast<int*>(&viewMode.modelViewMode), static_cast<int>(ModelViewMode::rm));
  ImGui::RadioButton("Diffuse color", reinterpret_cast<int*>(&viewMode.modelViewMode), static_cast<int>(ModelViewMode::texture));
  ImGui::Text("Switching plain/texture pbr params"); 
  ImGui::Checkbox("Is plain normal", &(viewMode.isPlainNormal));
  ImGui::Checkbox("Is plain metal/rough", &(viewMode.isPlainMetalRough));
  ImGui::Checkbox("Is plain color", &(viewMode.isPlainColor));
  
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

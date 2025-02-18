#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
    cube = Cube({ 0.0f });
    cube.Scale(3.0f);
    cube.Replace(0.0f, 2.0f, 0.0f);

    HRESULT hr = cube.Init(device, context, screenWidth, screenHeight);
    if (FAILED(hr))
        return hr;

    lights.reserve(3);
    lights.push_back(Light(XMFLOAT4(1.f, 0.f, 0.0f, 1.0f), 1.0f, 10U, 10U, 0.f, 5.5f, 0.8f));
    lights.push_back(Light(XMFLOAT4(0.f, 1.f, 0.0f, 1.0f), 1.0f, 10U, 10U, 0.8f, 5.5f, 0.f));
    lights.push_back(Light(XMFLOAT4(0.f, 0.f, 1.0f, 1.0f), 1.0f, 10U, 10U, 0.0f, 5.5f, -0.8f));

    hr = lights[0].Init(device, context, screenWidth, screenHeight);
    if (FAILED(hr))
        return hr;

    hr = lights[1].Init(device, context, screenWidth, screenHeight);
    if (FAILED(hr))
        return hr;

    hr = lights[2].Init(device, context, screenWidth, screenHeight);
    if (FAILED(hr))
        return hr;

#ifdef _DEBUG
    hr = context->QueryInterface(__uuidof(pAnnotation), reinterpret_cast<void**>(&pAnnotation));
    if (FAILED(hr))
        return hr;
#endif

    return hr;
}

void Scene::ProvideInput(const InputHandler& input) {
    for (auto& light : lights)
        light.ProvideInput(input);
}

void Scene::Release() {
#ifdef _DEBUG
    if (pAnnotation) pAnnotation->Release();
#endif

    cube.Release();

    for (auto& light : lights)
        light.Release();
}

void Scene::Render(ID3D11DeviceContext* context) {

#ifdef _DEBUG
    pAnnotation->BeginEvent((LPCWSTR)L"Draw Cube");
    std::string indexBufferName = "Indexes buffer", vertexBufferName = "Vertexes buffer";
#endif
    cube.Render(context);
#ifdef _DEBUG
    pAnnotation->EndEvent();
#endif
}

bool Scene::UpdateCubes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
    XMMATRIX worldMatrix = XMMatrixIdentity();
    cube.Update(context, worldMatrix, viewMatrix, projectionMatrix, cameraPos, lights);
    return true;
}


HRESULT Scene::Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
    UpdateCubes(context, viewMatrix, projectionMatrix, cameraPos);

    for (auto& light : lights)
        light.Update(context, viewMatrix, projectionMatrix, cameraPos);
    return S_OK;
}

void Scene::Resize(int screenWidth, int screenHeight) {};
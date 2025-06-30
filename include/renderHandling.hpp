#pragma once

#include <cmath>
#include <random>
#include <optional>
#include "model.hpp"
#include "Transform.hpp"
#include "multifloat.hpp"
#include "camera.hpp"
#include "Scene.hpp"
#include "SceneSettings.hpp"
#include "modelHandling.hpp"
#include "Light.hpp"


void RenderScene(Scene& scene, const Camera& camera, const std::vector<Light>& lights, SceneSettings& settings, std::uint8_t* pixelBuffer, float* zbuffer);

void RenderModel(Model& model, float3 position, float3 rotation, const Camera& camera, const std::vector<Light>& lights, SceneSettings& settings, std::uint8_t* pixelBuffer, float* zbuffer);

float3 WorldToScreen(float3 a, const Camera& camera);
float3 ScreenToWorld(float3 b, const Camera& camera);

float2 PixelToScreenSpace(float2 pixelCoords, const Camera& camera);
float3 ScreenToPixelSpace(float3 screenCoords, const Camera& camera);

float PerspBarycentricInterp(float3 baryCoords, float w1, float w2, float w3);

std::optional<float3> GetBarycentricCoords(float2 point, float2 v1, float2 v2, float2 v3);

float EdgeFunction(float2 point, float2 v1, float2 v2);

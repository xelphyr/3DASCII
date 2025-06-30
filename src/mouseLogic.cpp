#include "ui/mouseLogic.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <algorithm>

inline float FastSmoothFalloff(float x, float minBound, float maxBound);

void RunLogic(Camera &camera, SceneSettings& settings)
{
    if (!ImGui::IsAnyItemHovered() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
    {
        ImGuiIO& io = ImGui::GetIO();
        Transform& transform = camera.GetModifiableTransform();
        float scroll = io.MouseWheel;
        ImVec2 mouseDrag = io.MouseDelta;
        if (scroll != 0.f)
        {
            if (io.KeyMods == ImGuiMod_Shift) transform.SetPos(transform.GetPos() + transform.GetForwardVector()*scroll*settings.scrollVel/3);
            else if (io.KeyMods == ImGuiMod_Ctrl) camera.SetFOV(camera.GetFOV() + scroll*FastSmoothFalloff(camera.GetFOV() + scroll*settings.FOVChangeSpeed, 0.f, 180.f));
            else transform.SetPos(transform.GetPos() + transform.GetForwardVector()*scroll*settings.scrollVel);
        }
        if (mouseDrag[0] != 0)
        {
            //Middle mouse down
            if (io.MouseDown[2]) transform.SetPos(transform.GetPos() + transform.GetRightVector()*-mouseDrag[0]*settings.panVel/3);
            //Right mouse down
            else if (io.MouseDown[1]) transform.SetRot(transform.GetRot() + float3(0, mouseDrag[0]*settings.rotVel, 0));
        }
        if (mouseDrag[1] != 0)
        {
            //Middle mouse down
            if (io.MouseDown[2]) transform.SetPos(transform.GetPos() + transform.GetUpVector()*mouseDrag[1]*settings.panVel/3);
            //Right mouse down
            else if (io.MouseDown[1]) transform.SetRot(transform.GetRot() + float3(-mouseDrag[1]*settings.rotVel, 0, 0));
        }

        //WASDQE movements

        if (ImGui::IsKeyDown(ImGuiKey_W))
        {
            transform.SetPos(transform.GetPos() + transform.GetForwardVector()*settings.motionVel*io.DeltaTime);
        }
        if (ImGui::IsKeyDown(ImGuiKey_S))
        {
            transform.SetPos(transform.GetPos() - transform.GetForwardVector()*settings.motionVel*io.DeltaTime);
        }
        if (ImGui::IsKeyDown(ImGuiKey_D))
        {
            transform.SetPos(transform.GetPos() + transform.GetRightVector()*settings.motionVel*io.DeltaTime);
        }
        if (ImGui::IsKeyDown(ImGuiKey_A))
        {
            transform.SetPos(transform.GetPos() - transform.GetRightVector()*settings.motionVel*io.DeltaTime);
        }
        if (ImGui::IsKeyDown(ImGuiKey_Q))
        {
            transform.SetPos(transform.GetPos() + transform.GetUpVector()*settings.motionVel*io.DeltaTime);
        }
        if (ImGui::IsKeyDown(ImGuiKey_E))
        {
            transform.SetPos(transform.GetPos() - transform.GetUpVector()*settings.motionVel*io.DeltaTime);
        }
    }
}

inline float FastSmoothFalloff(float x, float minBound, float maxBound)
{
    float mid = 0.5f * (minBound + maxBound);
    float range = 0.5f * (maxBound - minBound);
    float t = std::abs(x - mid) / range;
    t = std::min(t, 1.0f);            // Clamp to [0,1]
    return (1.0f - t) * (1.0f - t);   // Quadratic falloff
}

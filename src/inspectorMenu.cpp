#include "ui/inspectorMenu.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <cmath>
#include <iostream>

void InspectorMenu::Draw(Scene& scene, SceneObject*& selectedObject, Light*& selectedLight, Camera*& selectedCamera)
{

    int count = (selectedObject != nullptr)+(selectedLight != nullptr)+(selectedCamera != nullptr);
    ImGui::Begin("Inspector", &m_visible);
    if (count == 0)
    {
        ImGui::Text("No object selected.");
    }
    else if (count > 1)
    {
        std::cout << "ERROR: TWO ITEMS ARE ASSIGNED AT ONCE\n";
    }
    /* Object */
    else if (selectedObject)
    {
        ImGui::Text("Name: %s", selectedObject->GetName().c_str());
        ImGui::Text("ID: %s", selectedObject->GetID().c_str());

        ImGui::Separator();

        // Display Transform
        ImGui::Text("Transform:");
        Transform &transform = selectedObject->GetTransform();
        float3 position = transform.GetPos();
        float3 rotation = transform.GetRot();
        ImGui::DragFloat3("Position", &position.x, 0.05f);
        ImGui::DragFloat3("Rotation", &rotation.x, 0.1f, 0.0f, 2.f*3.14, "%.2f rad", ImGuiSliderFlags_WrapAround);

        // Update the transform with new values
        transform.SetPos(position);
        transform.SetRot(rotation);      

        ImGui::Separator();

        // Display Model Details
        Model &model = selectedObject->GetModel();
        ImGui::Text("Model:");
        ImGui::Text("Vertices: %zu", model.GetLocalVerts().size());
        ImGui::Text("Faces: %zu", model.GetFaceIndices().size() / 3);
        
        float3 color = model.GetColor();
        ImGui::ColorEdit3("Color ", &color.r);   
        model.SetColor(color);  

        //Display deletion button
        if (ImGui::Button("Delete"))
        {
            if (scene.RemoveObjectFromScene(selectedObject->GetID()))
            {
                selectedObject = nullptr;
                selectedLight = nullptr;
                selectedCamera = nullptr;
            }
        }
    }

    /* Light */
    else if (selectedLight)
    {
        ImGui::Text("Name: %s", selectedLight->GetName().c_str());
        ImGui::Text("ID: %s", selectedLight->GetID().c_str());

        // Position
        ImGui::Text("Transform:");
        float3 position = selectedLight->GetPosition();
        ImGui::DragFloat3("Position", &position.x, 0.05f);
        selectedLight->SetPosition(position);

        //Intensity
        ImGui::Text("Intensity:");
        float intensity = selectedLight->GetIntensity();
        ImGui::DragFloat("Intensity", &intensity, 0.05f);
        selectedLight->SetIntensity(intensity);
        float3 color = selectedLight->GetColor();
        if (ImGui::ColorEdit3("Color ", &color.r))
        {
            selectedLight->SetColor(color);  
        }
    }

    /* Camera */
    else if (selectedCamera)
    {
        ImGui::Text("Camera");

        // Display Transform
        ImGui::Text("Transform:");
        Transform &transform = selectedCamera->GetModifiableTransform();
        float3 position = transform.GetPos();
        float3 rotation = transform.GetRot();
        float FOV = selectedCamera->GetFOV();
        float focalLength = selectedCamera -> GetFocalLength();
        ImGui::DragFloat3("Position", &position.x, 0.05f);
        ImGui::DragFloat3("Rotation", &rotation.x, 0.1f, 0.0f, 2.f*3.14, "%.2f rad", ImGuiSliderFlags_WrapAround);
        ImGui::DragFloat("FOV", &FOV, 0.f, 180.f);
        ImGui::DragFloat("Focal Length", &focalLength);

        // Update the transform with new values
        transform.SetPos(position);
        transform.SetRot(rotation);     
        selectedCamera->SetFOV(FOV);
        selectedCamera->SetFocalLength(focalLength);
    }
    else
    {
        std::cout<<"ERROR: TWO ITEMS ARE ASSIGNED AT ONCE";
    }

    ImGui::End();
}

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include "ImGuiFileDialog.h"

#include "modelHandling.hpp"
#include "renderHandling.hpp"
#include "camera.hpp"
#include "SceneSettings.hpp"

#include "ui.hpp"

#include "textBrightnessArray.hpp"

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

constexpr int wWidth = 150;
constexpr int wHeight = 110;
constexpr int scaleFactor = 8;

SDL_FRect dstRect = {
    0.0f, 0.0f,
    static_cast<float>(wWidth * scaleFactor),
    static_cast<float>(wHeight * scaleFactor)
};

float FOV = 30.f;
float focalLength = 1.f;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

float zbuffer[wHeight*wWidth];
std::uint8_t pixelBuffer[wHeight*wWidth*4]; //Main Pixel Buffer
std::uint8_t asciiPixelBuffer[wHeight*scaleFactor*wWidth*scaleFactor*4]; // Optional Pixel Buffer for ASCIIxxx mode

Camera camera(float3(), float3(), 30.f, 1.f, wWidth, wHeight);
std::vector<Light> lights;
Scene scene;
SceneSettings settings;

std::vector<float> RGBtoHSV(float3 RGB);
float3 HSVtoRGB(std::vector<float> HSV);
void ExportCheck();

int shouldExport = 0;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{

    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("3DASCII", wWidth*scaleFactor, wHeight*scaleFactor, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    camera = Camera(float3(), float3(), FOV, focalLength, wWidth, wHeight);    
    settings.lightingMode = LightingMode::FLAT;
    settings.lightIntensityCoeff = 100.f;
    settings.falloffCoeff = 1.f;

    scene.GetData(lights, camera, settings);

    texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, wWidth, wHeight);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST); 

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io; // Avoid unused variable warning
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    if (event->type == SDL_EVENT_USER)
    {
        //ASCII Export
        if(event->user.code == 1001)
        {
            shouldExport = 1;
        }
        if(event->user.code == 1002)
        {
            shouldExport = 2;
        }
    }
    ImGui_ImplSDL3_ProcessEvent(event);
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */


    SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);

    std::fill(pixelBuffer, pixelBuffer + (wWidth * wHeight * 4), 0);
    std::fill(asciiPixelBuffer, asciiPixelBuffer + (wWidth * scaleFactor * wHeight * scaleFactor * 4), 0);
    std::fill(zbuffer, zbuffer + (wWidth * wHeight), FLT_MAX);

    
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
        //UI
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

                objImporter(filePath.c_str(), scene); 
            }

            ImGuiFileDialog::Instance()->Close();
        }
        
        ImGuiID id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode, nullptr);
        ImGuiDockNode* node = ImGui::DockBuilderGetCentralNode(id);

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

        DisplayUI(scene, lights, camera, settings);

        //Rendering
        RenderScene(scene, camera, lights, settings, pixelBuffer, zbuffer);
        if (settings.renderMode == RenderMode::NORMAL)
        {
            float texH;
            SDL_GetTextureSize(texture, nullptr, &texH);
            if(wHeight*scaleFactor == static_cast<int>(texH))
            {
                texture = SDL_CreateTexture(renderer,
                    SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, wWidth, wHeight);
                SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST); 
            }
            void* pixels = nullptr;
            int pitch = 0;

            SDL_UpdateTexture(texture, nullptr, pixelBuffer, wWidth * 4);
            SDL_RenderTexture(renderer, texture, nullptr, &dstRect);
        }
        else if (settings.renderMode == RenderMode::ASCIIBASIC)
        {
            float texH;
            SDL_GetTextureSize(texture, nullptr, &texH);
            if(wHeight == static_cast<int>(texH))
            {
                texture = SDL_CreateTexture(renderer,
                    SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, wWidth*scaleFactor, wHeight*scaleFactor);
                SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST); 
            }

            const float2& pixelDimensions = camera.GetPixelDimensions();
            for (int y = 0; y<pixelDimensions.y*scaleFactor; y++)
            {
                int y_scaledown = y/scaleFactor;
                for (int x = 0; x<pixelDimensions.x*scaleFactor; x++)
                {
                    int x_scaledown = x/scaleFactor; 
                    int idx_scaledown = 4*(y_scaledown * wWidth + x_scaledown);

                    if (pixelBuffer[idx_scaledown + 3] == 0.f) continue; //Alpha = 0 means pixel has not been written in, skip

                    std::vector<float> hsv = RGBtoHSV(float3(
                        pixelBuffer[idx_scaledown + 0],
                        pixelBuffer[idx_scaledown + 1],
                        pixelBuffer[idx_scaledown + 2]
                    ));
                    int brightnessIndex = std::clamp(static_cast<int>(hsv[2]*31.9999f), 0, 31); //To turn it from 0-1 float to 0-31 index ints (ik theres a magic number stfu)
                    hsv[2] = 1;
                    float3 colorToUse = HSVtoRGB(hsv);
                    if (textBrightnessArray[brightnessIndex][y%scaleFactor] & (1<<(x%scaleFactor)))
                    {
                        int idx = 4*(y*wWidth*scaleFactor + x);
                        asciiPixelBuffer[idx + 0] = static_cast<uint8_t>(std::clamp(std::round(colorToUse.r), 0.0f, 255.0f));
                        asciiPixelBuffer[idx + 1] = static_cast<uint8_t>(std::clamp(std::round(colorToUse.g), 0.0f, 255.0f));
                        asciiPixelBuffer[idx + 2] = static_cast<uint8_t>(std::clamp(std::round(colorToUse.b), 0.0f, 255.0f));
                        asciiPixelBuffer[idx + 3] = static_cast<std::uint8_t>(255.f);  //Opaque
                    }
                }
            }
            void* pixels = nullptr;
            int pitch = 0;

            SDL_UpdateTexture(texture, nullptr, asciiPixelBuffer, wWidth * scaleFactor * 4);
            SDL_RenderTexture(renderer, texture, nullptr, &dstRect);
        }
        

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(renderer);
    
    //EXPORT
    ExportCheck();


    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    scene.SaveData(lights, camera, settings);
}

//---------------------------------------
void ExportCheck()
{
    if (shouldExport != 0)
    {
        std::ofstream file("export.txt", std::ios::binary);
        file << "\xEF\xBB\xBF";
        float2 dimensions = camera.GetPixelDimensions();
        for (int y = 0; y<dimensions.y; y++)
        {
            for(int x = 0; x<dimensions.x; x++)
            {
                int idx = 4*(y * dimensions.x + x);

                if (pixelBuffer[idx + 3] == 0.f) 
                {file<<" "; continue;}

                std::vector<float> hsv = RGBtoHSV(float3(
                    pixelBuffer[idx + 0],
                    pixelBuffer[idx + 1],
                    pixelBuffer[idx + 2]
                ));
                int brightnessIndex = std::clamp(static_cast<int>(hsv[2]*31.9999f), 0, 31);
                hsv[2] = 1;
                float3 colorToUse = HSVtoRGB(hsv);
                if (shouldExport == 1)
                {
                    float3 colorToUse = HSVtoRGB(hsv);
                    file<<"\x1b[38;2;"
                    <<static_cast<int>(colorToUse.r)<<";"
                    <<static_cast<int>(colorToUse.g)<<";"
                    <<static_cast<int>(colorToUse.b)<<"m"
                    <<textBrightnessArrayCharacters[brightnessIndex]
                    <<"\x1b[0m";
                }
                if (shouldExport == 2)
                {
                    file<<textBrightnessArrayCharacters[brightnessIndex];
                }
            }
            file<<"\n";
        }
        shouldExport = 0;
        SDL_Log("Export Done!");
    }
}

//---------------------------------------

std::vector<float> RGBtoHSV(float3 RGB)
{
    std::vector<float> HSV = {0.f,0.f,0.f};
    RGB = RGB/255.f;
    float Cmax = std::max({RGB.r, RGB.g, RGB.b});
    float Cmin = std::min({RGB.r, RGB.g, RGB.b});
    float delta = Cmax - Cmin;

    //Hue
    if (delta < 1e-6f)
    {
        HSV[0] = 0.f;
    }
    else if (Cmax == RGB.g)
    {
        HSV[0] = (((RGB.b - RGB.r)/delta) + 2.f);
    }
    else if (Cmax == RGB.b)
    {
        HSV[0] = (((RGB.r - RGB.g)/delta) + 4.f);
    }
    else if (Cmax == RGB.r)
    {
        HSV[0] = (((RGB.g - RGB.b)/delta) + 0.f);
    }
    HSV[0] = std::fmod(HSV[0]/6.f + 1.f, 1.f)*360.f; 
    
    //Saturation
    if (Cmax == 0.f)
    {
        HSV[1] = 0.f;
    }
    else
    {
        HSV[1] = delta/Cmax;
    }

    //Value (Brightness)

    HSV[2] = Cmax;

    return HSV;
}

float3 HSVtoRGB(std::vector<float> HSV)
{
    float H = std::fmod(HSV[0]+360.f, 360.f);
    float S = HSV[1];
    float V = HSV[2];

    float C = S * V;
    float X = C * (1.0f- std::abs(std::fmod(H/60.0f, 2.0f) - 1.0f));
    float m = V - C;

    float3 RGB;

    //The long and hard process of R', G', B'

    if      (0.0f   <= H && H < 60.0f ) {RGB = float3(C, X, 0);}
    else if (60.0f  <= H && H < 120.0f) {RGB = float3(X, C, 0);}
    else if (120.0f <= H && H < 180.0f) {RGB = float3(0, C, X);}
    else if (180.0f <= H && H < 240.0f) {RGB = float3(0, X, C);}
    else if (240.0f <= H && H < 300.0f) {RGB = float3(X, 0, C);}
    else if (300.0f <= H && H < 360.0f) {RGB = float3(C, 0, X);}
    else RGB = float3(0.1f,0.1f,0.1f);

    RGB = (RGB+float3(m,m,m))*255;
    RGB.r = std::clamp(std::round(RGB.r), 0.0f, 255.0f);
    RGB.g = std::clamp(std::round(RGB.g), 0.0f, 255.0f);
    RGB.b = std::clamp(std::round(RGB.b), 0.0f, 255.0f);
    

    return RGB;
}

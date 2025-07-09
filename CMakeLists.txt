cmake_minimum_required(VERSION 3.22)
project(3DASCII VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

include(FetchContent)

# SDL3
FetchContent_Declare(SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG main  # SDL3 is on the main branch
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SDL3)

# ImGui (docking optional)
FetchContent_Declare(ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking  # or use v1.91.1 if you don't want docking
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(ImGui)

# Your executable
add_executable(3DASCII
  src/main.cpp
  src/model.cpp 
  src/multifloat.cpp 
  src/image.cpp 
  src/renderHandling.cpp 
  src/camera.cpp
  src/Light.cpp

  src/ui.cpp
  src/inspectorMenu.cpp
  src/assetBrowser.cpp
  src/sceneViewer.cpp
  src/toolbar.cpp
  src/sceneSettingsMenu.cpp
  src/mouseLogic.cpp

  src/ImGuiFileDialog.cpp
)

list(APPEND IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp  # optional

    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

target_sources(3DASCII PRIVATE
    ${IMGUI_SRC}
)

# Includes
target_include_directories(3DASCII PRIVATE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${sdl3_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(3DASCII PRIVATE SDL_STATIC=1)

# Link libraries
find_package(OpenGL REQUIRED)
target_link_libraries(3DASCII PRIVATE
  OpenGL::GL
  SDL3::SDL3  # or SDL3-shared if you prefer dynamic linking
)

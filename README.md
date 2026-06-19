# Optikos

A lightweight cross-platform C++ UI framework.

![demo](https://github.com/IlanVinograd/Optikos/blob/main/res/images/example1.gif?raw=true)

---

Optikos provides a simple API for building desktop UIs in C++ — buttons, sliders, text inputs, scrollable containers, and image rendering out of the box.

![example](https://github.com/IlanVinograd/Optikos/blob/main/res/images/example2.png?raw=true)

[example1.cpp](https://github.com/IlanVinograd/Optikos/blob/main/examples/example1.cpp)

## Example
```cpp
#include "optikos.hpp"

int main()
{
    Optikos::Optikos app("Hello", 800, 480);

    auto btn = std::make_unique<Optikos::Button>(120, 30, Optikos::Vec2(20, 20), "Click me");
    btn->setEvent([]() { std::cout << "Clicked!\n"; });
    app.addWidget(1, std::move(btn));

    while (!app.should_close())
    {
        app.begin();
        app.end();
    }
}
```

---

## Backends

|          | GLFW | SDL |
|----------|------|-----|
| Windows  | ✅   | ❌  |
| Linux    | ✅   | ❌  |
| macOS    | ✅   | ❌  |

|          | OpenGL | Vulkan | DirectX |
|----------|--------|--------|---------|
| Renderer | ✅     | ❌     | ❌      |

---

## Getting Started

Add Optikos to your project via CMake `FetchContent`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(TestApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(OPTIKOS_RENDERER "OPENGL" CACHE STRING "" FORCE)
set(OPTIKOS_RENDERER "VULKAN" CACHE STRING "" FORCE)

set(OPTIKOS_PLATFORM "GLFW"   CACHE STRING "" FORCE)
set(OPTIKOS_INPUT    "GLFW"   CACHE STRING "" FORCE)

find_package(glfw3 CONFIG REQUIRED)

if(OPTIKOS_RENDERER STREQUAL "VULKAN")
    find_package(Vulkan REQUIRED MODULE)
    find_package(unofficial-shaderc CONFIG REQUIRED)
else()
    find_package(glad CONFIG REQUIRED)
endif()

include(FetchContent)
FetchContent_Declare(
    Optikos
    GIT_REPOSITORY https://github.com/IlanVinograd/Optikos
    GIT_TAG main
)
FetchContent_MakeAvailable(Optikos)

add_executable(TestApp main.cpp)

target_link_libraries(TestApp PRIVATE 
    Optikos::Optikos
    glfw
)

if(OPTIKOS_RENDERER STREQUAL "VULKAN")
    target_link_libraries(TestApp PRIVATE 
        Vulkan::Vulkan
        unofficial::shaderc::shaderc
    )
else()
    target_link_libraries(TestApp PRIVATE 
        glad::glad
    )
endif()

target_compile_definitions(TestApp PRIVATE
    OPTIKOS_RES_DIR="${optikos_SOURCE_DIR}/res/"
)

option(ENABLE_GPU_PROFILING "Enable GPU profiling" OFF)
if(ENABLE_GPU_PROFILING)
    target_compile_definitions(TestApp PRIVATE ENABLE_GPU_PROFILING)
    target_compile_definitions(Optikos PRIVATE ENABLE_GPU_PROFILING)
    message(STATUS "GPU Profiling: ON")
endif()

optikos_copy_resources(TestApp)
```

---

```cmake
cmake_minimum_required(VERSION 3.20)
project(TestApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(OPTIKOS_RENDERER "OPENGL" CACHE STRING "" FORCE)
 set(OPTIKOS_RENDERER "WEBGPU" CACHE STRING "" FORCE)
# set(OPTIKOS_RENDERER "VULKAN" CACHE STRING "" FORCE)

set(OPTIKOS_PLATFORM "GLFW"   CACHE STRING "" FORCE)
set(OPTIKOS_INPUT    "GLFW"   CACHE STRING "" FORCE)

find_package(glfw3 CONFIG REQUIRED)

if(OPTIKOS_RENDERER STREQUAL "VULKAN")
    find_package(Vulkan REQUIRED MODULE)
    find_package(unofficial-shaderc CONFIG REQUIRED)
else()
    find_package(glad CONFIG REQUIRED)
endif()

#include(FetchContent)
#FetchContent_Declare(
#    Optikos
#    GIT_REPOSITORY https://github.com/IlanVinograd/Optikos
#    GIT_TAG main
#)
#FetchContent_MakeAvailable(Optikos)

add_subdirectory("C:/Users/ilanv/Optikos" optikos-build)

add_executable(TestApp main.cpp)

target_link_libraries(TestApp PRIVATE 
    Optikos::Optikos
    glfw
)

if(OPTIKOS_RENDERER STREQUAL "VULKAN")
    target_link_libraries(TestApp PRIVATE 
        Vulkan::Vulkan
        unofficial::shaderc::shaderc
    )
else()
    target_link_libraries(TestApp PRIVATE 
        glad::glad
    )
endif()

target_compile_definitions(TestApp PRIVATE
    OPTIKOS_RES_DIR="${optikos_SOURCE_DIR}/res/"
)

target_compile_definitions(TestApp PRIVATE
    OPTIKOS_RES_DIR="C:/Users/ilanv/Optikos/res/"
)

option(ENABLE_GPU_PROFILING "Enable GPU profiling" OFF)
if(ENABLE_GPU_PROFILING)
    target_compile_definitions(TestApp PRIVATE ENABLE_GPU_PROFILING)
    target_compile_definitions(Optikos PRIVATE ENABLE_GPU_PROFILING)
    message(STATUS "GPU Profiling: ON")
endif()

optikos_copy_resources(TestApp)

if(WIN32 AND VCPKG_TOOLCHAIN)
    set(VCPKG_BIN_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
    set(VCPKG_DEBUG_BIN_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")

    add_custom_command(
        TARGET TestApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<IF:$<CONFIG:Debug>,${VCPKG_DEBUG_BIN_DIR}/dxcompiler.dll,${VCPKG_BIN_DIR}/dxcompiler.dll>"
        "$<IF:$<CONFIG:Debug>,${VCPKG_DEBUG_BIN_DIR}/dxil.dll,${VCPKG_BIN_DIR}/dxil.dll>"
        $<TARGET_FILE_DIR:TestApp>
        VERBATIM
    )
endif()
```

---

## Build (Windows)

```powershell
cmake --preset windows-msvc
```

```powershell
cmake --preset windows-msvc && cmake --build build && .\build\Debug\Optikos.exe
```

With GPU profiling:

```powershell
cmake --preset windows-msvc -D ENABLE_GPU_PROFILING=ON && cmake --build build && .\build\Debug\Optikos.exe
```

```powershell
rmdir /s /q build


 -DENABLE_GPU_PROFILING=ON
cmake --build build
.\build\Debug\TestApp.exe
```

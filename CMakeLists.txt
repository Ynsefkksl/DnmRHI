cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE
        "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

project("DnmRHI")

find_package(Vulkan REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)

option(VULKAN_BACKEND "enable vulkan backend" ON)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("d3d12 backend not available on linux")
    set(D3D12_BACKEND OFF)
else()
    option(D3D12_BACKEND "enable d3d12 backend" ON)
endif()

file(GLOB_RECURSE Sources CONFIGURE_DEPENDS
    Src/*.cpp
)

if(VULKAN_BACKEND)
    file(GLOB_RECURSE VulkanSources CONFIGURE_DEPENDS 
        Src/Platform/Vulkan/*.cpp)
    list(APPEND Sources ${VulkanSources})
endif()

if(D3D12_BACKEND)
    file(GLOB_RECURSE D3d12Sources CONFIGURE_DEPENDS
        Src/Platform/D3D12/*.cpp
    )
    list(APPEND Sources ${D3d12Sources})
endif()

add_library(DnmRHI ${Sources})

target_include_directories(DnmRHI PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    $ENV{VULKAN_SDK}/include
)

target_link_libraries(DnmRHI PRIVATE
    Vulkan::Headers
    slang::slang
    xxHash::xxhash
    $<$<PLATFORM_ID:Linux>:xkbcommon-x11>
    $<$<PLATFORM_ID:Linux>:xkbcommon>
    $<$<PLATFORM_ID:Linux>:xcb>
    $<$<PLATFORM_ID:Linux>:xcb-xkb>
)

target_compile_definitions(DnmRHI PRIVATE
    $<$<PLATFORM_ID:Windows>:DnmRHI_Platform_Win32>
    $<$<PLATFORM_ID:Linux>:DnmRHI_Platform_Linux>
    
    "DnmRHI_Force_Inline=$<IF:$<CXX_COMPILER_ID:GNU,Clang,AppleClang,IntelLLVM>,__attribute__((always_inline))inline,$<IF:$<CXX_COMPILER_ID:MSVC>,__forceinline,inline>>"
    "DnmRHI_Restrict=$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang,IntelLLVM,MSVC>:__restrict>"
    "DnmRHI_Func_Signature=$<IF:$<CXX_COMPILER_ID:GNU,Clang,AppleClang,IntelLLVM>,__PRETTY_FUNCTION__,$<IF:$<CXX_COMPILER_ID:MSVC>,__FUNCSIG__,__func__>>"

    $<$<PLATFORM_ID:Windows>:NOMINMAX>
    $<$<PLATFORM_ID:Windows>:UNICODE>
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>

    $<$<CONFIG:Debug>:MODE_DEBUG>
        
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VMA_VULKAN_VERSION=1004000
)
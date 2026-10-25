#!/usr/bin/env bash
set -e

python3 "$VULKAN_SDK/share/vulkan/registry/gen_profiles_solution.py" \
  --registry "$VULKAN_SDK/share/vulkan/registry/vk.xml" \
  --input ./ \
  --output-library-inc ./Include/DnmRHI/Platform/Vulkan/ \
  --output-library-src ./Src/Platform/Vulkan/

sed -i.bak 's|<vulkan/vulkan_profiles.h>|<DnmRHI/Platform/Vulkan/vulkan_profiles.h>|g' ./Src/Platform/Vulkan/vulkan_profiles.cpp
rm -f ./Src/Platform/Vulkan/vulkan_profiles.cpp.bak
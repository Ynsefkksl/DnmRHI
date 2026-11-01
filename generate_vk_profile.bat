@echo off

python "%VULKAN_SDK%\share\vulkan\registry\gen_profiles_solution.py" ^
  --registry "%VULKAN_SDK%\share\vulkan\registry\vk.xml" ^
  --input ./ ^
  --output-library-inc ./External/vulkan/ ^
  --output-library-src ./External/vulkan/

if errorlevel 1 exit /b 1

@REM  powershell -NoProfile -Command "$f=(Resolve-Path '.\Src\Platform\Vulkan\vulkan_profiles.cpp').Path; [IO.File]::WriteAllText($f, [IO.File]::ReadAllText($f).Replace('<vulkan/vulkan_profiles.h>','<DnmRHI/Platform/Vulkan/vulkan_profiles.h>'))"
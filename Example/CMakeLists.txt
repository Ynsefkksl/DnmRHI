cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("Examples")

macro(add_dnm_example)
    get_filename_component(_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" _name "${_name}")

    add_executable(${_name} main.cpp)
    target_link_libraries(${_name} PRIVATE DnmRHI::DnmRHI)
endmacro()

file(GLOB dir RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(sub_dirs ${dir})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${sub_dirs}
       AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${sub_dirs}/CMakeLists.txt)
        add_subdirectory(${sub_dirs})
    endif()
endforeach()

cmake_minimum_required(VERSION 3.25)

project(Engine)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB engineHeaderFiles CONFIGURE_DEPENDS
     "Headers/engine/ai/*.h"
     "Headers/engine/data/*.h"
     "Headers/engine/graphics/*.h"
     "Headers/engine/input/*.h"
     "Headers/engine/modeling/*.h"
     "Headers/engine/physics/*.h"
     "Headers/engine/scripts/*.h"
     "Headers/engine/*.h"
)

file(GLOB engineSourceFiles CONFIGURE_DEPENDS
    "Source/*.c"
    "Source/scripts/*.c"
)

add_library(Engine STATIC ${engineSourceFiles} ${engineHeaderFiles})

set_property(TARGET Engine PROPERTY COMPILE_WARNING_AS_ERROR ON)
set_target_properties(Engine PROPERTIES COMPILE_FLAGS "/std:c11 /ZI /Od")
target_compile_definitions(Engine PRIVATE BUILD_STATIC_LIB)

target_include_directories(Engine PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/external/stb_image"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include"
    "${cglm_SOURCE_DIR}/include"
    "../Core/Headers"
    "Headers"
)

target_link_libraries(Engine PRIVATE
    Core
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/lib/Release/x64/glew32.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib-vc2022/glfw3.lib"
    opengl32.lib
)

add_dependencies(Engine Core)

set_target_properties(Engine PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/bin/Release/x64/glew32.dll" DESTINATION ${CMAKE_BINARY_DIR})
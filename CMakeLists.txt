cmake_minimum_required(VERSION 3.25) # or whatever is suitable/available for you

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Core)

#set (CMAKE_C_FLAGS "/std:c11")

link_directories(external)

file(GLOB coreHeaderFiles CONFIGURE_DEPENDS
     "Core/Headers/core/math/*.h"
     "Core/Headers/core/*.h"
)

file(GLOB coreSourceFiles CONFIGURE_DEPENDS
    "Core/Source/*.c"
)

file(GLOB_RECURSE assets CONFIGURE_DEPENDS
    "assets/*.*"
)

add_library(Core ${coreSourceFiles} ${coreHeaderFiles})

set_property(TARGET Core PROPERTY COMPILE_WARNING_AS_ERROR ON)

set_target_properties(Core PROPERTIES COMPILE_FLAGS "/std:c11 /ZI /Od")

include(FetchContent)

FetchContent_Populate(
    cglm                             # Recommendation: Stick close to the original name.
    GIT_REPOSITORY https://github.com/recp/cglm.git
    SOURCE_DIR     ../external/cglm              # (Relative) path within in the build directory.
)

add_subdirectory(${cglm_SOURCE_DIR} cglm/build)

add_dependencies(Core cglm)

target_include_directories(Core PUBLIC 
    "external/stb_image"
    "external/cglm/include"
    "Core/Headers"
    "Core"
) 

# copy glew to build dir so we can debug code from there
file(COPY "external/glew/bin/Release/x64/glew32.dll" DESTINATION .)

# copy assets to build dir so we dont constantly have failed to find file issues
file(COPY assets DESTINATION ./)

target_link_libraries(Core PRIVATE
    ../external/glew/lib/Release/x64/glew32
    ../external/glfw/lib-vc2022/glfw3
    opengl32.lib
    Synchronization
)

add_subdirectory(Generics)

add_dependencies(Generics Core)

target_link_libraries(Generics Core)

project(Engine)

file(GLOB engineHeaderFiles CONFIGURE_DEPENDS
     "Engine/Headers/engine/ai/*.h"
     "Engine/Headers/engine/data/*.h"
     "Engine/Headers/engine/graphics/*.h"
     "Engine/Headers/engine/input/*.h"
     "Engine/Headers/engine/modeling/*.h"
     "Engine/Headers/engine/physics/*.h"
     "Engine/Headers/engine/scripts/*.h"
     "Engine/Headers/engine/*.h"
)

file(GLOB engineSourceFiles CONFIGURE_DEPENDS
    "Engine/Source/*.c"
    "Engine/Source/scripts/*.c"
)

add_executable(Engine ${engineSourceFiles} ${engineHeaderFiles})

set_property(TARGET Engine PROPERTY COMPILE_WARNING_AS_ERROR ON)

# compile to preprocessor
# /P /EP /C
set_target_properties(Engine PROPERTIES COMPILE_FLAGS "/std:c11 /ZI /Od")

target_link_options(Engine PUBLIC "/DEBUG")

target_include_directories(Engine PUBLIC 
    "external/glfw/include"
    "external/cglm/include"
    "external/glew/include"
    "external/stb_image"
    "Engine/Headers"
    "Engine"
) 

target_link_libraries(Engine Core)

add_dependencies(Engine Core)

set_target_properties( Engine Core Generics
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
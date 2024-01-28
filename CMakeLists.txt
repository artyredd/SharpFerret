cmake_minimum_required(VERSION 3.25) # or whatever is suitable/available for you
project(Singine)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set (CMAKE_C_FLAGS "/std:c11")

link_directories(external)

file(GLOB headerFiles CONFIGURE_DEPENDS
     "Core/Headers/ai/*.h"
     "Core/Headers/data/*.h"
     "Core/Headers/graphics/*.h"
     "Core/Headers/helpers/*.h"
     "Core/Headers/input/*.h"
     "Core/Headers/math/*.h"
     "Core/Headers/modeling/*.h"
     "Core/Headers/physics/*.h"
     "Core/Headers/scripts/*.h"
     "Core/Headers/singine/*.h"
     "Core/Tests/*.h"
)

file(GLOB sourceFiles CONFIGURE_DEPENDS
    "Core/Tests/*.c"
    "Core/Source/*.c"
    "Core/Source/scripts/*.c"
)

add_executable(Singine ${sourceFiles} ${headerFiles})

target_include_directories(Singine PUBLIC 
    "external/glfw/include"
    "external/cglm/include"
    "external/glew/include"
    "external/stb_image"
    "Core/Headers"
    "Core"
) 

include(FetchContent)

FetchContent_Populate(
    cglm                             # Recommendation: Stick close to the original name.
    GIT_REPOSITORY https://github.com/recp/cglm.git
    SOURCE_DIR     external/cglm              # (Relative) path within in the build directory.
)

# And now you can already add and use it, like it's a part/target of your own project!
add_subdirectory(${cglm_SOURCE_DIR} cglm/build)
    # IMPORTANT: Lowercase name! See below for more on this...


add_dependencies(Singine cglm)

target_link_libraries(Singine PRIVATE
    external/glew/lib/Release/x64/glew32
    external/glfw/lib-vc2022/glfw3
    opengl32.lib
)
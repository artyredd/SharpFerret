cmake_minimum_required(VERSION 3.25)

project(Core)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB coreHeaderFiles CONFIGURE_DEPENDS
     "Headers/core/math/*.h"
     "Headers/core/*.h"
)

file(GLOB coreSourceFiles CONFIGURE_DEPENDS
    "Source/*.c"
)

add_library(Core ${coreSourceFiles} ${coreHeaderFiles})

set_property(TARGET Core PROPERTY COMPILE_WARNING_AS_ERROR ON)
set_target_properties(Core PROPERTIES COMPILE_FLAGS "/std:c11 /ZI /Od")

include(FetchContent)

FetchContent_Declare(
    cglm
    GIT_REPOSITORY https://github.com/recp/cglm.git
)
FetchContent_MakeAvailable(cglm)

target_include_directories(Core PRIVATE 
    "${cglm_SOURCE_DIR}/include"
    "external/stb_image"
    "Headers"
)

target_link_libraries(Core cglm)
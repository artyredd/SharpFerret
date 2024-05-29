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
    GIT_TAG v0.8.5
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/cglm
)
FetchContent_MakeAvailable(cglm)

target_include_directories(Core PRIVATE 
    "${cglm_SOURCE_DIR}/include"
    "Headers"
)

file(COPY "${CMAKE_SOURCE_DIR}/external/glew/bin/Release/x64/glew32.dll" DESTINATION ${CMAKE_BINARY_DIR})
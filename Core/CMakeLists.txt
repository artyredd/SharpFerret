cmake_minimum_required(VERSION 3.25)

project(Core C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB coreHeaderFiles CONFIGURE_DEPENDS
     "Headers/core/math/*.h"
     "Headers/core/*.h"
)

file(GLOB coreSourceFiles CONFIGURE_DEPENDS
    "Source/*.c"
)

add_library(Core STATIC ${coreSourceFiles} ${coreHeaderFiles})

set_property(TARGET Core PROPERTY COMPILE_WARNING_AS_ERROR ON)

if (MSVC)
    target_compile_options(Core PRIVATE /std:c11 /ZI /Od /experimental:c11atomics)
endif()

target_compile_definitions(Core PRIVATE BUILD_STATIC_LIB)

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

target_link_libraries(Core 
    cglm
    Synchronization
)
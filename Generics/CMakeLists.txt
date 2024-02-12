cmake_minimum_required(VERSION 3.25) # or whatever is suitable/available for you

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Generics)

# copy glew to build dir so we can debug code from there
#file(COPY "../Core/Headers/core/array.h" DESTINATION Source/.)
#file(COPY "../Core/Headers/core/csharp.h" DESTINATION Source/.)
#file(COPY "../Core/Headers/core/memory.h" DESTINATION Source/.)
#file(COPY "../Core/Headers/core/macros.h" DESTINATION Source/.)
#file(COPY "../Core/Source/array.c" DESTINATION Source/.)
#file(COPY "../Core/Source/memory.c" DESTINATION Source/.)


file(GLOB headers CONFIGURE_DEPENDS
     "Source/*.h"
)

file(GLOB source CONFIGURE_DEPENDS
    "Source/*.c"
)

add_executable(Generics ${source} ${headers})

set_property(TARGET Generics PROPERTY COMPILE_WARNING_AS_ERROR ON)

set_target_properties(Generics PROPERTIES COMPILE_FLAGS "/std:c11 /ZI /Od")

target_include_directories(Core PUBLIC 
    "../Core/Headers"
) 

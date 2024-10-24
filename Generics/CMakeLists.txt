cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Generics)

# Copy assets to build directory
file(GLOB assets CONFIGURE_DEPENDS "assets/*")
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/assets)

# Glob headers and source files
file(GLOB headers CONFIGURE_DEPENDS "Source/*.h")
file(GLOB source CONFIGURE_DEPENDS "Source/*.c")

# Create a library for Generics instead of an executable
add_executable(Generics ${source} ${headers})

set_property(TARGET Generics PROPERTY COMPILE_WARNING_AS_ERROR ON)
set_target_properties(Generics PROPERTIES COMPILE_FLAGS "/std:c11 /ZI /Od /experimental:c11atomics")

# Include directories
target_include_directories(Generics PRIVATE "../Core/Headers")

# Link against Core library
target_link_libraries(Generics PRIVATE Core)

# Set output directories
set_target_properties(Generics PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Repos/Singine/external/cglm"
  "C:/Repos/Singine/cglm-build"
  "C:/Repos/Singine/cglm-subbuild/cglm-populate-prefix"
  "C:/Repos/Singine/cglm-subbuild/cglm-populate-prefix/tmp"
  "C:/Repos/Singine/cglm-subbuild/cglm-populate-prefix/src/cglm-populate-stamp"
  "C:/Repos/Singine/cglm-subbuild/cglm-populate-prefix/src"
  "C:/Repos/Singine/cglm-subbuild/cglm-populate-prefix/src/cglm-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Repos/Singine/cglm-subbuild/cglm-populate-prefix/src/cglm-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Repos/Singine/cglm-subbuild/cglm-populate-prefix/src/cglm-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

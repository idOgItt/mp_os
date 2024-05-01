# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/json-src"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/json-build"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/json-subbuild/json-populate-prefix"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/json-subbuild/json-populate-prefix/tmp"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/json-subbuild/json-populate-prefix/src"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp/${subDir}")
endforeach()

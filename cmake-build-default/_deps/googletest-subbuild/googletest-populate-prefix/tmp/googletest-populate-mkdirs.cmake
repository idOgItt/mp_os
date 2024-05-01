# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/googletest-src"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/googletest-build"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/googletest-subbuild/googletest-populate-prefix"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/googletest-subbuild/googletest-populate-prefix/tmp"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/googletest-subbuild/googletest-populate-prefix/src"
  "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/cmake-build-default/_deps/googletest-subbuild/googletest-populate-prefix/src/googletest-populate-stamp/${subDir}")
endforeach()

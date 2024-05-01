# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os

# Include any dependencies generated for this target.
include allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/compiler_depend.make

# Include the progress variables for this target.
include allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/progress.make

# Include the compile flags for this target's objects.
include allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/flags.make

allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o: allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/flags.make
allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o: allocator/allocator_buddies_system/src/allocator_buddies_system.cpp
allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o: allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o"
	cd /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o -MF CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o.d -o CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o -c /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system/src/allocator_buddies_system.cpp

allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.i"
	cd /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system/src/allocator_buddies_system.cpp > CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.i

allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.s"
	cd /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system/src/allocator_buddies_system.cpp -o CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.s

# Object files for target mp_os_allctr_allctr_bdds_sstm
mp_os_allctr_allctr_bdds_sstm_OBJECTS = \
"CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o"

# External object files for target mp_os_allctr_allctr_bdds_sstm
mp_os_allctr_allctr_bdds_sstm_EXTERNAL_OBJECTS =

allocator/allocator_buddies_system/libmp_os_allctr_allctr_bdds_sstm.a: allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/src/allocator_buddies_system.cpp.o
allocator/allocator_buddies_system/libmp_os_allctr_allctr_bdds_sstm.a: allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/build.make
allocator/allocator_buddies_system/libmp_os_allctr_allctr_bdds_sstm.a: allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libmp_os_allctr_allctr_bdds_sstm.a"
	cd /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system && $(CMAKE_COMMAND) -P CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/cmake_clean_target.cmake
	cd /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/build: allocator/allocator_buddies_system/libmp_os_allctr_allctr_bdds_sstm.a
.PHONY : allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/build

allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/clean:
	cd /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system && $(CMAKE_COMMAND) -P CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/cmake_clean.cmake
.PHONY : allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/clean

allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/depend:
	cd /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system /home/stardust/Work/University/MAI_Math_Practicum_2/mp_os/allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : allocator/allocator_buddies_system/CMakeFiles/mp_os_allctr_allctr_bdds_sstm.dir/depend


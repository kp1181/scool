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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.22.2/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.22.2/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build

# Include any dependencies generated for this target.
include tsp/CMakeFiles/tsp_mpi.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tsp/CMakeFiles/tsp_mpi.dir/compiler_depend.make

# Include the progress variables for this target.
include tsp/CMakeFiles/tsp_mpi.dir/progress.make

# Include the compile flags for this target's objects.
include tsp/CMakeFiles/tsp_mpi.dir/flags.make

tsp/CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o: tsp/CMakeFiles/tsp_mpi.dir/flags.make
tsp/CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o: ../tsp/tsp_mpi.cpp
tsp/CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o: tsp/CMakeFiles/tsp_mpi.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tsp/CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o"
	cd /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/tsp && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tsp/CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o -MF CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o.d -o CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o -c /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/tsp/tsp_mpi.cpp

tsp/CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.i"
	cd /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/tsp && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/tsp/tsp_mpi.cpp > CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.i

tsp/CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.s"
	cd /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/tsp && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/tsp/tsp_mpi.cpp -o CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.s

# Object files for target tsp_mpi
tsp_mpi_OBJECTS = \
"CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o"

# External object files for target tsp_mpi
tsp_mpi_EXTERNAL_OBJECTS =

tsp/tsp_mpi: tsp/CMakeFiles/tsp_mpi.dir/tsp_mpi.cpp.o
tsp/tsp_mpi: tsp/CMakeFiles/tsp_mpi.dir/build.make
tsp/tsp_mpi: /opt/homebrew/lib/libomp.dylib
tsp/tsp_mpi: /opt/homebrew/Cellar/open-mpi/4.1.1_2/lib/libmpi.dylib
tsp/tsp_mpi: tsp/CMakeFiles/tsp_mpi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable tsp_mpi"
	cd /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/tsp && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tsp_mpi.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tsp/CMakeFiles/tsp_mpi.dir/build: tsp/tsp_mpi
.PHONY : tsp/CMakeFiles/tsp_mpi.dir/build

tsp/CMakeFiles/tsp_mpi.dir/clean:
	cd /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/tsp && $(CMAKE_COMMAND) -P CMakeFiles/tsp_mpi.dir/cmake_clean.cmake
.PHONY : tsp/CMakeFiles/tsp_mpi.dir/clean

tsp/CMakeFiles/tsp_mpi.dir/depend:
	cd /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/tsp /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/tsp /Users/krishnaprasadporandla/Downloads/scool-bnsl/examples/build/tsp/CMakeFiles/tsp_mpi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tsp/CMakeFiles/tsp_mpi.dir/depend


# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/berat/Desktop/clion-2020.1.1/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/berat/Desktop/clion-2020.1.1/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/berat/CLionProjects/myAlgo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/berat/CLionProjects/myAlgo/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/fastlog.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/fastlog.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/fastlog.dir/flags.make

CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.o: CMakeFiles/fastlog.dir/flags.make
CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.o: ../fastlog_filebuffer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/berat/CLionProjects/myAlgo/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.o -c /home/berat/CLionProjects/myAlgo/fastlog_filebuffer.cpp

CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/berat/CLionProjects/myAlgo/fastlog_filebuffer.cpp > CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.i

CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/berat/CLionProjects/myAlgo/fastlog_filebuffer.cpp -o CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.s

# Object files for target fastlog
fastlog_OBJECTS = \
"CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.o"

# External object files for target fastlog
fastlog_EXTERNAL_OBJECTS =

libfastlog.so: CMakeFiles/fastlog.dir/fastlog_filebuffer.cpp.o
libfastlog.so: CMakeFiles/fastlog.dir/build.make
libfastlog.so: CMakeFiles/fastlog.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/berat/CLionProjects/myAlgo/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libfastlog.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/fastlog.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/fastlog.dir/build: libfastlog.so

.PHONY : CMakeFiles/fastlog.dir/build

CMakeFiles/fastlog.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/fastlog.dir/cmake_clean.cmake
.PHONY : CMakeFiles/fastlog.dir/clean

CMakeFiles/fastlog.dir/depend:
	cd /home/berat/CLionProjects/myAlgo/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/berat/CLionProjects/myAlgo /home/berat/CLionProjects/myAlgo /home/berat/CLionProjects/myAlgo/cmake-build-debug /home/berat/CLionProjects/myAlgo/cmake-build-debug /home/berat/CLionProjects/myAlgo/cmake-build-debug/CMakeFiles/fastlog.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/fastlog.dir/depend

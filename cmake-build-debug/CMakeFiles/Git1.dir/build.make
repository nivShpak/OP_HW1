# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

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
CMAKE_COMMAND = /opt/clion-2019.3.5/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /opt/clion-2019.3.5/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/student/Desktop/Git1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/student/Desktop/Git1/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Git1.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Git1.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Git1.dir/flags.make

CMakeFiles/Git1.dir/smash.cpp.o: CMakeFiles/Git1.dir/flags.make
CMakeFiles/Git1.dir/smash.cpp.o: ../smash.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/student/Desktop/Git1/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Git1.dir/smash.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Git1.dir/smash.cpp.o -c /home/student/Desktop/Git1/smash.cpp

CMakeFiles/Git1.dir/smash.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Git1.dir/smash.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/student/Desktop/Git1/smash.cpp > CMakeFiles/Git1.dir/smash.cpp.i

CMakeFiles/Git1.dir/smash.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Git1.dir/smash.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/student/Desktop/Git1/smash.cpp -o CMakeFiles/Git1.dir/smash.cpp.s

CMakeFiles/Git1.dir/Commands.cpp.o: CMakeFiles/Git1.dir/flags.make
CMakeFiles/Git1.dir/Commands.cpp.o: ../Commands.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/student/Desktop/Git1/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/Git1.dir/Commands.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Git1.dir/Commands.cpp.o -c /home/student/Desktop/Git1/Commands.cpp

CMakeFiles/Git1.dir/Commands.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Git1.dir/Commands.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/student/Desktop/Git1/Commands.cpp > CMakeFiles/Git1.dir/Commands.cpp.i

CMakeFiles/Git1.dir/Commands.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Git1.dir/Commands.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/student/Desktop/Git1/Commands.cpp -o CMakeFiles/Git1.dir/Commands.cpp.s

CMakeFiles/Git1.dir/signals.cpp.o: CMakeFiles/Git1.dir/flags.make
CMakeFiles/Git1.dir/signals.cpp.o: ../signals.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/student/Desktop/Git1/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/Git1.dir/signals.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Git1.dir/signals.cpp.o -c /home/student/Desktop/Git1/signals.cpp

CMakeFiles/Git1.dir/signals.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Git1.dir/signals.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/student/Desktop/Git1/signals.cpp > CMakeFiles/Git1.dir/signals.cpp.i

CMakeFiles/Git1.dir/signals.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Git1.dir/signals.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/student/Desktop/Git1/signals.cpp -o CMakeFiles/Git1.dir/signals.cpp.s

# Object files for target Git1
Git1_OBJECTS = \
"CMakeFiles/Git1.dir/smash.cpp.o" \
"CMakeFiles/Git1.dir/Commands.cpp.o" \
"CMakeFiles/Git1.dir/signals.cpp.o"

# External object files for target Git1
Git1_EXTERNAL_OBJECTS =

Git1: CMakeFiles/Git1.dir/smash.cpp.o
Git1: CMakeFiles/Git1.dir/Commands.cpp.o
Git1: CMakeFiles/Git1.dir/signals.cpp.o
Git1: CMakeFiles/Git1.dir/build.make
Git1: CMakeFiles/Git1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/student/Desktop/Git1/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable Git1"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Git1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Git1.dir/build: Git1

.PHONY : CMakeFiles/Git1.dir/build

CMakeFiles/Git1.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Git1.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Git1.dir/clean

CMakeFiles/Git1.dir/depend:
	cd /home/student/Desktop/Git1/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/student/Desktop/Git1 /home/student/Desktop/Git1 /home/student/Desktop/Git1/cmake-build-debug /home/student/Desktop/Git1/cmake-build-debug /home/student/Desktop/Git1/cmake-build-debug/CMakeFiles/Git1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Git1.dir/depend

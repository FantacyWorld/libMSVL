# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/geekwar/libMSVL

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/geekwar/libMSVL/build

# Include any dependencies generated for this target.
include CMakeFiles/mytest.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/mytest.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/mytest.dir/flags.make

CMakeFiles/mytest.dir/mytest.c.o: CMakeFiles/mytest.dir/flags.make
CMakeFiles/mytest.dir/mytest.c.o: ../mytest.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/geekwar/libMSVL/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/mytest.dir/mytest.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mytest.dir/mytest.c.o   -c /home/geekwar/libMSVL/mytest.c

CMakeFiles/mytest.dir/mytest.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mytest.dir/mytest.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/geekwar/libMSVL/mytest.c > CMakeFiles/mytest.dir/mytest.c.i

CMakeFiles/mytest.dir/mytest.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mytest.dir/mytest.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/geekwar/libMSVL/mytest.c -o CMakeFiles/mytest.dir/mytest.c.s

CMakeFiles/mytest.dir/mytest.c.o.requires:
.PHONY : CMakeFiles/mytest.dir/mytest.c.o.requires

CMakeFiles/mytest.dir/mytest.c.o.provides: CMakeFiles/mytest.dir/mytest.c.o.requires
	$(MAKE) -f CMakeFiles/mytest.dir/build.make CMakeFiles/mytest.dir/mytest.c.o.provides.build
.PHONY : CMakeFiles/mytest.dir/mytest.c.o.provides

CMakeFiles/mytest.dir/mytest.c.o.provides.build: CMakeFiles/mytest.dir/mytest.c.o

# Object files for target mytest
mytest_OBJECTS = \
"CMakeFiles/mytest.dir/mytest.c.o"

# External object files for target mytest
mytest_EXTERNAL_OBJECTS =

mytest: CMakeFiles/mytest.dir/mytest.c.o
mytest: CMakeFiles/mytest.dir/build.make
mytest: libdyn_array.so
mytest: CMakeFiles/mytest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable mytest"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mytest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/mytest.dir/build: mytest
.PHONY : CMakeFiles/mytest.dir/build

CMakeFiles/mytest.dir/requires: CMakeFiles/mytest.dir/mytest.c.o.requires
.PHONY : CMakeFiles/mytest.dir/requires

CMakeFiles/mytest.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mytest.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mytest.dir/clean

CMakeFiles/mytest.dir/depend:
	cd /home/geekwar/libMSVL/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/geekwar/libMSVL /home/geekwar/libMSVL /home/geekwar/libMSVL/build /home/geekwar/libMSVL/build /home/geekwar/libMSVL/build/CMakeFiles/mytest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mytest.dir/depend


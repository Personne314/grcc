# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

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
CMAKE_SOURCE_DIR = /home/louis/Documents/Programmation/C++/grcc

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/louis/Documents/Programmation/C++/grcc/build/release

# Utility rule file for copy_resources.

# Include any custom commands dependencies for this target.
include CMakeFiles/copy_resources.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/copy_resources.dir/progress.make

CMakeFiles/copy_resources:
	rsync -a --update --exclude=".gitkeep" --exclude="release/" --exclude="debug/" /home/louis/Documents/Programmation/C++/grcc/resources/ /home/louis/Documents/Programmation/C++/grcc/build/release
	rsync -a --update --exclude=".gitkeep" /home/louis/Documents/Programmation/C++/grcc/resources/release/ /home/louis/Documents/Programmation/C++/grcc/build/release

copy_resources: CMakeFiles/copy_resources
copy_resources: CMakeFiles/copy_resources.dir/build.make
.PHONY : copy_resources

# Rule to build all files generated by this target.
CMakeFiles/copy_resources.dir/build: copy_resources
.PHONY : CMakeFiles/copy_resources.dir/build

CMakeFiles/copy_resources.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/copy_resources.dir/cmake_clean.cmake
.PHONY : CMakeFiles/copy_resources.dir/clean

CMakeFiles/copy_resources.dir/depend:
	cd /home/louis/Documents/Programmation/C++/grcc/build/release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/louis/Documents/Programmation/C++/grcc /home/louis/Documents/Programmation/C++/grcc /home/louis/Documents/Programmation/C++/grcc/build/release /home/louis/Documents/Programmation/C++/grcc/build/release /home/louis/Documents/Programmation/C++/grcc/build/release/CMakeFiles/copy_resources.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/copy_resources.dir/depend


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
CMAKE_BINARY_DIR = /home/louis/Documents/Programmation/C++/grcc/build/debug

# Include any dependencies generated for this target.
include CMakeFiles/test_regex.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test_regex.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test_regex.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_regex.dir/flags.make

CMakeFiles/test_regex.dir/test/test_regex.cpp.o: CMakeFiles/test_regex.dir/flags.make
CMakeFiles/test_regex.dir/test/test_regex.cpp.o: /home/louis/Documents/Programmation/C++/grcc/test/test_regex.cpp
CMakeFiles/test_regex.dir/test/test_regex.cpp.o: CMakeFiles/test_regex.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/louis/Documents/Programmation/C++/grcc/build/debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_regex.dir/test/test_regex.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_regex.dir/test/test_regex.cpp.o -MF CMakeFiles/test_regex.dir/test/test_regex.cpp.o.d -o CMakeFiles/test_regex.dir/test/test_regex.cpp.o -c /home/louis/Documents/Programmation/C++/grcc/test/test_regex.cpp

CMakeFiles/test_regex.dir/test/test_regex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test_regex.dir/test/test_regex.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louis/Documents/Programmation/C++/grcc/test/test_regex.cpp > CMakeFiles/test_regex.dir/test/test_regex.cpp.i

CMakeFiles/test_regex.dir/test/test_regex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test_regex.dir/test/test_regex.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louis/Documents/Programmation/C++/grcc/test/test_regex.cpp -o CMakeFiles/test_regex.dir/test/test_regex.cpp.s

CMakeFiles/test_regex.dir/src/grammar.cpp.o: CMakeFiles/test_regex.dir/flags.make
CMakeFiles/test_regex.dir/src/grammar.cpp.o: /home/louis/Documents/Programmation/C++/grcc/src/grammar.cpp
CMakeFiles/test_regex.dir/src/grammar.cpp.o: CMakeFiles/test_regex.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/louis/Documents/Programmation/C++/grcc/build/debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/test_regex.dir/src/grammar.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_regex.dir/src/grammar.cpp.o -MF CMakeFiles/test_regex.dir/src/grammar.cpp.o.d -o CMakeFiles/test_regex.dir/src/grammar.cpp.o -c /home/louis/Documents/Programmation/C++/grcc/src/grammar.cpp

CMakeFiles/test_regex.dir/src/grammar.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test_regex.dir/src/grammar.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louis/Documents/Programmation/C++/grcc/src/grammar.cpp > CMakeFiles/test_regex.dir/src/grammar.cpp.i

CMakeFiles/test_regex.dir/src/grammar.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test_regex.dir/src/grammar.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louis/Documents/Programmation/C++/grcc/src/grammar.cpp -o CMakeFiles/test_regex.dir/src/grammar.cpp.s

CMakeFiles/test_regex.dir/src/regex.cpp.o: CMakeFiles/test_regex.dir/flags.make
CMakeFiles/test_regex.dir/src/regex.cpp.o: /home/louis/Documents/Programmation/C++/grcc/src/regex.cpp
CMakeFiles/test_regex.dir/src/regex.cpp.o: CMakeFiles/test_regex.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/louis/Documents/Programmation/C++/grcc/build/debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/test_regex.dir/src/regex.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_regex.dir/src/regex.cpp.o -MF CMakeFiles/test_regex.dir/src/regex.cpp.o.d -o CMakeFiles/test_regex.dir/src/regex.cpp.o -c /home/louis/Documents/Programmation/C++/grcc/src/regex.cpp

CMakeFiles/test_regex.dir/src/regex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test_regex.dir/src/regex.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louis/Documents/Programmation/C++/grcc/src/regex.cpp > CMakeFiles/test_regex.dir/src/regex.cpp.i

CMakeFiles/test_regex.dir/src/regex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test_regex.dir/src/regex.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louis/Documents/Programmation/C++/grcc/src/regex.cpp -o CMakeFiles/test_regex.dir/src/regex.cpp.s

# Object files for target test_regex
test_regex_OBJECTS = \
"CMakeFiles/test_regex.dir/test/test_regex.cpp.o" \
"CMakeFiles/test_regex.dir/src/grammar.cpp.o" \
"CMakeFiles/test_regex.dir/src/regex.cpp.o"

# External object files for target test_regex
test_regex_EXTERNAL_OBJECTS =

test_regex: CMakeFiles/test_regex.dir/test/test_regex.cpp.o
test_regex: CMakeFiles/test_regex.dir/src/grammar.cpp.o
test_regex: CMakeFiles/test_regex.dir/src/regex.cpp.o
test_regex: CMakeFiles/test_regex.dir/build.make
test_regex: CMakeFiles/test_regex.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/louis/Documents/Programmation/C++/grcc/build/debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable test_regex"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_regex.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_regex.dir/build: test_regex
.PHONY : CMakeFiles/test_regex.dir/build

CMakeFiles/test_regex.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_regex.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_regex.dir/clean

CMakeFiles/test_regex.dir/depend:
	cd /home/louis/Documents/Programmation/C++/grcc/build/debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/louis/Documents/Programmation/C++/grcc /home/louis/Documents/Programmation/C++/grcc /home/louis/Documents/Programmation/C++/grcc/build/debug /home/louis/Documents/Programmation/C++/grcc/build/debug /home/louis/Documents/Programmation/C++/grcc/build/debug/CMakeFiles/test_regex.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/test_regex.dir/depend


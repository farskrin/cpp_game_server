# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

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
CMAKE_COMMAND = /home/anton/tools/clion/clion-2023.2.2/bin/cmake/linux/x64/bin/cmake

# The command to remove a file.
RM = /home/anton/tools/clion/clion-2023.2.2/bin/cmake/linux/x64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/anton/work/BackEnd_curs/Practice/cpp-backend/sprint4/problems/leave_game/solution

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/anton/work/BackEnd_curs/Practice/cpp-backend/sprint4/problems/leave_game/solution/build

# Utility rule file for ContinuousMemCheck.

# Include any custom commands dependencies for this target.
include CMakeFiles/ContinuousMemCheck.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ContinuousMemCheck.dir/progress.make

CMakeFiles/ContinuousMemCheck:
	/home/anton/tools/clion/clion-2023.2.2/bin/cmake/linux/x64/bin/ctest -D ContinuousMemCheck

ContinuousMemCheck: CMakeFiles/ContinuousMemCheck
ContinuousMemCheck: CMakeFiles/ContinuousMemCheck.dir/build.make
.PHONY : ContinuousMemCheck

# Rule to build all files generated by this target.
CMakeFiles/ContinuousMemCheck.dir/build: ContinuousMemCheck
.PHONY : CMakeFiles/ContinuousMemCheck.dir/build

CMakeFiles/ContinuousMemCheck.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ContinuousMemCheck.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ContinuousMemCheck.dir/clean

CMakeFiles/ContinuousMemCheck.dir/depend:
	cd /home/anton/work/BackEnd_curs/Practice/cpp-backend/sprint4/problems/leave_game/solution/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/anton/work/BackEnd_curs/Practice/cpp-backend/sprint4/problems/leave_game/solution /home/anton/work/BackEnd_curs/Practice/cpp-backend/sprint4/problems/leave_game/solution /home/anton/work/BackEnd_curs/Practice/cpp-backend/sprint4/problems/leave_game/solution/build /home/anton/work/BackEnd_curs/Practice/cpp-backend/sprint4/problems/leave_game/solution/build /home/anton/work/BackEnd_curs/Practice/cpp-backend/sprint4/problems/leave_game/solution/build/CMakeFiles/ContinuousMemCheck.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ContinuousMemCheck.dir/depend


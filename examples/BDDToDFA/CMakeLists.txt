# CMakeLists.txt for BDDToDFA
# Time Complexity: O(N) for file collection, O(1) per target and configuration

# Collect all source files in this directory
# Time Complexity: O(N), where N is the number of .cc and .cpp files
file(GLOB BDD_TO_DFA_SOURCES "*.cc" "*.cpp")

# Create a static library from these sources
# Time Complexity: O(1) per file added to the target
add_library(BDDToDFA STATIC ${BDD_TO_DFA_SOURCES})

# Set compile options
# Time Complexity: O(1)
if (MSVC)
  target_compile_options(BDDToDFA PRIVATE /W4 /WX) # O(1)
else()
  target_compile_options(BDDToDFA PRIVATE 
    -Wall           # O(1)
    -Wextra         # O(1)
    -Wshadow        # O(1)
    -Wunused        # O(1)
    -Wno-c++11-extensions # O(1)
    -Wno-unused-parameter # O(1)
  )
endif()

# Include BraveDD headers
# Time Complexity: O(1)
target_include_directories(BDDToDFA PUBLIC
  ${PROJECT_SOURCE_DIR}/src
)

# Set CMake policy to allow quoted test names with spaces
# Time Complexity: O(1)
cmake_policy(SET CMP0110 NEW)
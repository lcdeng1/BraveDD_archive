# Add source files
file(GLOB_RECURSE SRC_FILES "*.cc" "*.cpp")
file(GLOB_RECURSE HEADER_FILES "*.h")

# Add compile options
if (MSVC)
  add_compile_options(/W4 /WX)
else()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0)
    add_compile_options(-Wall -Wextra -Wshadow -Wunused -Wno-c++11-extensions -Wno-unused-parameter)
  endif()
endif()

# Create a library target
add_library(BraveDD ${SRC_FILES})

# Find LZMA 
find_package(LibLZMA REQUIRED)   

# Find Boost 
find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)

# Link LZMA 
target_link_libraries(BraveDD
    PUBLIC
    LibLZMA::LibLZMA    
)

# Link ghc 
target_link_libraries(BraveDD
    PUBLIC
    Boost::filesystem
)

#Link 

# Add include directories
target_include_directories(BraveDD
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)
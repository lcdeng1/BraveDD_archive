cmake_minimum_required(VERSION 3.19)

# Project name and version
project(BraveDD VERSION 0.1.1)

# Define metadata variables
set(PROJECT_URL "https://github.com/asminer/brave_dd")
set(PROJECT_HOMEPAGE_URL "https://github.com/asminer/brave_dd")
set(PROJECT_COPYRIGHT "Copyright (C) 2024, Lichuan Deng, Andrew S. Miner, Gianfranco Ciardo.")
set(PROJECT_LICENSE "Apache License, version 2.0")
set(PROJECT_DATE "Oct 22, 2025")

# Set output directories for different target types
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # .a or .lib files
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # .so or .dylib files

# Configure a header file to pass the metadata to the source code
configure_file(${PROJECT_SOURCE_DIR}/src/info.h.in ${PROJECT_SOURCE_DIR}/src/info.h)

# Add compile options
if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wshadow -Wunused -std=c++11)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0)
    add_compile_options(-Wno-deprecated-copy-with-user-provided-copy -Wno-return-type)
  else()
    add_compile_options(-Wno-unused-parameter)
  endif()
endif()

enable_testing()

# Add subdirectories in build 
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)
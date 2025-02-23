#
# Test source files
#
file(GLOB TEST_SOURCES tests/*.cc tests/*.cpp)

# Add compile options
if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wshadow -Wunused -Wno-c++11-extensions -Wno-unused-parameter)
endif()

# Ensure `test_parser` is properly defined
add_executable(test_parser ${CMAKE_CURRENT_SOURCE_DIR}/test_parser.cc)
target_link_libraries(test_parser PUBLIC BraveDD)
add_test(NAME test_parser COMMAND test_parser)

# Loop for other tests
foreach(file ${TEST_SOURCES})
  get_filename_component(TEST_EXE ${file} NAME_WE)
  if(NOT TEST_EXE STREQUAL "test_parser")  # Avoid duplicate definition
    add_executable(${TEST_EXE} ${file})
    target_link_libraries(${TEST_EXE} PUBLIC BraveDD)
    target_include_directories(${TEST_EXE} PUBLIC
      ${PROJECT_SOURCE_DIR}/src
    )
    add_test(NAME "${TEST_EXE}" COMMAND "${TEST_EXE}")
  endif()
endforeach()

#
# Allow quoted test names with spaces
#
cmake_policy(SET CMP0110 NEW)

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

# Add include directories
target_include_directories(BraveDD
    PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

# Copy all headers to a central "include" folder inside build/
# set(INCLUDE_DEST "${CMAKE_BINARY_DIR}/include")
foreach(HEADER ${HEADER_FILES})
    # Compute relative path (strip "src/" prefix)
    file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/src" "${HEADER}")
    # Compute destination path under includes/
    set(DEST "${CMAKE_BINARY_DIR}/includes/${REL_PATH}")
    # Make sure destination directory exists
    get_filename_component(DEST_DIR "${DEST}" DIRECTORY)
    add_custom_command(
        TARGET BraveDD PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${HEADER}" "${DEST}"
        # COMMENT "Copying header ${REL_PATH}"
    )
endforeach()
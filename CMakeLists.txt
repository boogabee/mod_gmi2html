cmake_minimum_required(VERSION 3.10)
project(mod_gmi2html C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find Apache2
find_package(Apache2 REQUIRED)
find_package(APR REQUIRED)

# Source files
set(SOURCES
    src/mod_gmi2html.c
    src/gemini_parser.c
)

# Create shared library
add_library(mod_gmi2html SHARED ${SOURCES})

# Include directories
target_include_directories(mod_gmi2html PRIVATE
    ${APACHE2_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(mod_gmi2html
    ${APACHE2_LIBRARIES}
    ${APR_LIBRARIES}
)

# Set output directory
set_target_properties(mod_gmi2html PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Installation target (optional)
install(TARGETS mod_gmi2html
    LIBRARY DESTINATION ${APACHE2_MODULES_DIR}
)

# Print build information
message(STATUS "Apache2 Include: ${APACHE2_INCLUDE_DIR}")
message(STATUS "Apache2 Modules: ${APACHE2_MODULES_DIR}")
message(STATUS "APR Include: ${APR_INCLUDE_DIR}")

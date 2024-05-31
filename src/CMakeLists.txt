cmake_minimum_required(VERSION 3.22)

project(libmozok VERSION 0.0.0 DESCRIPTION "quest engine for games")

# Setup the install directory prefix.
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../install)

# ------------------ Compiler Flags ------------------ #

add_library(libmozok_compiler_flags INTERFACE)

target_compile_features(libmozok_compiler_flags INTERFACE cxx_std_11)
target_compile_features(libmozok_compiler_flags INTERFACE cxx_noexcept)

# enable strict warning policy
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(libmozok_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)
#target_compile_options(libmozok_compiler_flags INTERFACE 
#  "$<${gcc_like_cxx}:$<$<CONFIG:Release>:-Ofast -DNDEBUG -march=native>>"
#)

option(OUTPUT_HASH_COLLISIONS_INFO "If set ON enables the output on hash collisions.")


# -------------- Add Subdirectories -------------- #

set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")
enable_testing() 
add_subdirectory(libmozok)


# ------------------ Doxygen ------------------ #

option(BUILD_DOXYGEN_DOCS "Generate documentation with Doxygen.")
if (BUILD_DOXYGEN_DOCS)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # Setup input and output paths.
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # Configure file request.
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started...")

        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
    message("Doxygen not found. Doxygen needs to be installed in order to "
            "generate the Doxygen documentation.")
    endif (DOXYGEN_FOUND)
endif(BUILD_DOXYGEN_DOCS)
#unset(BUILD_DOXYGEN_DOCS CACHE) 

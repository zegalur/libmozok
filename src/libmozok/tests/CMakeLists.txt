# Add test utils library.
add_library(libmozok_test_utils)
target_include_directories(libmozok_test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libmozok_test_utils PRIVATE libmozok_compiler_flags)
target_link_libraries(libmozok_test_utils PRIVATE libmozok)
target_sources(libmozok_test_utils PRIVATE utils/utils.cpp)
target_sources(libmozok_test_utils PUBLIC utils/utils.hpp)
set_property(TARGET libmozok_test_utils PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

enable_testing()
add_subdirectory(puzzles)
add_subdirectory(quests)
# ---------------- Add Library ---------------- #

add_library(libmozok STATIC)
set_target_properties(libmozok PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(libmozok PROPERTIES STATIC_LIBRARY_OPTIONS "/NODEFAULTLIB:msvcrt")
set_property(TARGET libmozok PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET libmozok PROPERTY POSITION_INDEPENDENT_CODE ON)

find_package (Threads)
target_link_libraries(libmozok PRIVATE Threads::Threads)

target_include_directories(libmozok PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libmozok PRIVATE libmozok_compiler_flags)

if(OUTPUT_HASH_COLLISIONS_INFO)
    add_compile_definitions(MOZOK_OUTPUT_HASH_COLLISIONS_INFO)
endif()

# -------------- Private Sources -------------- #

target_sources(libmozok PRIVATE libmozok/private_types.hpp)
target_sources(libmozok PRIVATE libmozok/result.cpp)
target_sources(libmozok PRIVATE libmozok/message_processor.cpp)
target_sources(libmozok PRIVATE libmozok/server.cpp)

target_sources(libmozok PRIVATE libmozok/parser.hpp)
target_sources(libmozok PRIVATE libmozok/parser.cpp)

target_sources(libmozok PRIVATE libmozok/error_utils.hpp)
target_sources(libmozok PRIVATE libmozok/error_utils.cpp)

target_sources(libmozok PRIVATE libmozok/world.hpp)
target_sources(libmozok PRIVATE libmozok/world.cpp)

target_sources(libmozok PRIVATE libmozok/project.hpp)
target_sources(libmozok PRIVATE libmozok/project.cpp)

target_sources(libmozok PRIVATE libmozok/script.hpp)
target_sources(libmozok PRIVATE libmozok/script.cpp)

target_sources(libmozok PRIVATE libmozok/type.hpp)
target_sources(libmozok PRIVATE libmozok/type.cpp)

target_sources(libmozok PRIVATE libmozok/object.hpp)
target_sources(libmozok PRIVATE libmozok/object.cpp)

target_sources(libmozok PRIVATE libmozok/relation.hpp)
target_sources(libmozok PRIVATE libmozok/relation.cpp)

target_sources(libmozok PRIVATE libmozok/relation_list.hpp)
target_sources(libmozok PRIVATE libmozok/relation_list.cpp)

target_sources(libmozok PRIVATE libmozok/action.hpp)
target_sources(libmozok PRIVATE libmozok/action.cpp)

target_sources(libmozok PRIVATE libmozok/quest.hpp)
target_sources(libmozok PRIVATE libmozok/quest.cpp)

target_sources(libmozok PRIVATE libmozok/statement.hpp)
target_sources(libmozok PRIVATE libmozok/statement.cpp)

target_sources(libmozok PRIVATE libmozok/state.hpp)
target_sources(libmozok PRIVATE libmozok/state.cpp)

target_sources(libmozok PRIVATE libmozok/quest_plan.hpp)
target_sources(libmozok PRIVATE libmozok/quest_plan.cpp)

target_sources(libmozok PRIVATE libmozok/quest_planner.hpp)
target_sources(libmozok PRIVATE libmozok/quest_planner.cpp)

target_sources(libmozok PRIVATE libmozok/message_queue.hpp)
target_sources(libmozok PRIVATE libmozok/message_queue.cpp)

target_sources(libmozok PRIVATE libmozok/quest_manager.hpp)
target_sources(libmozok PRIVATE libmozok/quest_manager.cpp)

# --------------- Public Headers -------------- #

target_sources(libmozok PUBLIC libmozok/mozok.hpp)
target_sources(libmozok PUBLIC libmozok/public_types.hpp)
target_sources(libmozok PUBLIC libmozok/result.hpp)
target_sources(libmozok PUBLIC libmozok/message_processor.hpp)
target_sources(libmozok PUBLIC libmozok/server.hpp)
target_sources(libmozok PUBLIC libmozok/filesystem.hpp)


# --------------- Installation ---------------- #

# Install rule for public headers.
# (No `FILE_SET` in order to be compatible with CMake 3.22)
install(FILES libmozok/mozok.hpp 
              libmozok/public_types.hpp 
              libmozok/result.hpp
              libmozok/message_processor.hpp
              libmozok/server.hpp
              libmozok/filesystem.hpp
        DESTINATION include/libmozok)

# Install rule for library itself.
install(TARGETS libmozok
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install rule for the pkg-config file.
#configure_file(libmozok.pc.in ${CMAKE_BINARY_DIR}/libmozok.pc @ONLY)
#install(FILES ${CMAKE_BINARY_DIR}/libmozok.pc 
#    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)


# ---------------- Add Testing ---------------- #

add_subdirectory(tests)

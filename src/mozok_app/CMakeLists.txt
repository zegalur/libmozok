# ---------------- Application -------------- #

add_executable(mozok)

set_target_properties(libmozok PROPERTIES VERSION ${PROJECT_VERSION})
set_property(TARGET mozok PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(mozok PRIVATE libmozok PRIVATE libmozok_compiler_flags)
target_include_directories(mozok PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


# ----------------- Sources ----------------- #

target_sources(mozok PRIVATE app/main.cpp)
target_sources(mozok PRIVATE app/strings.hpp)

target_sources(mozok PRIVATE app/app.hpp)
target_sources(mozok PRIVATE app/app.cpp)


# --------------- Installation -------------- #

install(TARGETS mozok DESTINATION bin)


# ------------------ Tests ------------------ #

add_subdirectory(tests)

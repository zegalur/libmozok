
add_executable(quest_solver main.cpp)

target_link_libraries(quest_solver PRIVATE libmozok PRIVATE libmozok_compiler_flags)
target_link_libraries(quest_solver PRIVATE libmozok_test_utils)

set_property(TARGET quest_solver PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

function(solve_quest quest init)
    add_test(NAME quest_${quest}_${init}
        COMMAND quest_solver ${quest} ${init}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(quest_${quest}_${init}
        PROPERTIES PASS_REGULAR_EXPRESSION "MOZOK_OK")
endfunction()

solve_quest(save_princess Init)
solve_quest(make_sword NoSQ)
solve_quest(make_sword WithSQ)
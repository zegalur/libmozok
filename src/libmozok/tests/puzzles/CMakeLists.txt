
add_executable(puzzle_solver main.cpp)

target_link_libraries(puzzle_solver PRIVATE libmozok PRIVATE libmozok_compiler_flags)
target_link_libraries(puzzle_solver PRIVATE libmozok_test_utils)

set_property(TARGET puzzle_solver PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

function(solve_puzzle puzzle init_action result)
    configure_file(${puzzle}.quest ${puzzle}.quest COPYONLY)
    add_test(NAME puzzle_${puzzle}_${init_action}
        COMMAND puzzle_solver ${puzzle} ${init_action}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(puzzle_${puzzle}_${init_action}
        PROPERTIES PASS_REGULAR_EXPRESSION "${result}")
endfunction()

solve_puzzle(wolf_goat_cabbage Init MOZOK_OK)
solve_puzzle(hanoi_towers Init MOZOK_OK)
solve_puzzle(game_of_fifteen Init_Easy MOZOK_OK)
#solve_puzzle(game_of_fifteen Init_Medium MOZOK_OK)
#solve_puzzle(game_of_fifteen Init_Hard MOZOK_OK)
#solve_puzzle(game_of_fifteen Init_Hardest_1 MOZOK_OK)
#solve_puzzle(game_of_fifteen Init_Impossible MOZOK_QUEST_STATUS_UNREACHABLE)
solve_puzzle(push_blocks Init_Reachable MOZOK_OK)
solve_puzzle(push_blocks Init_Unreachable MOZOK_QUEST_STATUS_UNREACHABLE)

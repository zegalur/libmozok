
add_executable(puzzle_solver main.cpp)

target_link_libraries(puzzle_solver PRIVATE libmozok PRIVATE libmozok_compiler_flags)
target_link_libraries(puzzle_solver PRIVATE libmozok_test_utils)

set_property(TARGET puzzle_solver PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

function(solve_puzzle puzzle)
    configure_file(${puzzle}.quest ${puzzle}.quest COPYONLY)
    add_test(NAME puzzle_${puzzle} 
        COMMAND puzzle_solver ${puzzle}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(puzzle_${puzzle} 
        PROPERTIES PASS_REGULAR_EXPRESSION "MOZOK_OK")
endfunction()

solve_puzzle(wolf_goat_cabbage)
solve_puzzle(hanoi_towers)
solve_puzzle(game_of_fifteen)
solve_puzzle(push_blocks)

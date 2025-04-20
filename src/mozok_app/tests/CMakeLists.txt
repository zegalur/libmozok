
# Copy quest files as configure files.
configure_file(tutorial.qsf tutorial.qsf COPYONLY)
configure_file(tutorial_debug.qsf tutorial_debug.qsf COPYONLY)
configure_file(tutorial_main.quest tutorial_main.quest COPYONLY)
configure_file(tutorial_controls.quest tutorial_controls.quest COPYONLY)
configure_file(tutorial_fighting.quest tutorial_fighting.quest COPYONLY)
configure_file(tutorial_key.quest tutorial_key.quest COPYONLY)
configure_file(tutorial_puzzle.quest tutorial_puzzle.quest COPYONLY)
configure_file(tutorial_utils.quest tutorial_utils.quest COPYONLY)


function(analyze_quest quest qsf options)
    add_test(NAME try_${qsf}
        COMMAND mozok ${quest}.qsf -P MOZOK_OK ${options}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(try_${qsf}
        PROPERTIES FAIL_REGULAR_EXPRESSION "[eE]rror|ERROR")
endfunction()


analyze_quest(tutorial tutorial "")

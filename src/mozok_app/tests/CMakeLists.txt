
function(analyze_quest quest qsf options)
    configure_file(${quest}.quest ${quest}.quest COPYONLY)
    add_test(NAME try_${qsf}
        COMMAND mozok ${quest}.qsf -P MOZOK_OK ${options}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(try_${qsf}
        PROPERTIES PASS_REGULAR_EXPRESSION "MOZOK_OK")
endfunction()


analyze_quest(tutorial tutorial "")

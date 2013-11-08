
set (tests
  tAsdmStMan
  makeAsdmIndex
)

foreach (test ${tests})
    casa_add_executable (${test} ${test}.cc)
    target_link_libraries (${test} casa_tables)
    add_test (${test} ${CMAKE_SOURCE_DIR}/cmake/cmake_assay ./${test})
endforeach (test)

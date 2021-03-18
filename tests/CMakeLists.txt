include_directories(${PROJECT_SOURCE_DIR}/src)

add_custom_target(tests)

function(custom_add_test)
  cmake_parse_arguments(TEST "" "NAME" "LIBRARIES" ${ARGN})
  if(NOT TEST_NAME)
    message(FATAL_ERROR "Required argument NAME missing in call to custom_add_test")
  endif()

  set(BUILD_${TEST_NAME} TRUE CACHE BOOL "Set true to build test ${TEST_NAME}/false to disable building this test")

  if(BUILD_${TEST_NAME})
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} ${TEST_LIBRARIES})
    add_test(${TEST_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
    add_dependencies(tests ${TEST_NAME})
  endif()
endfunction()

add_subdirectory(unit_tests)

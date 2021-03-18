set(TEST_LIBRARIES scheduler)

custom_add_test(NAME SimulatorTests LIBRARIES ${TEST_LIBRARIES})

file(GLOB SIMULATOR_TEST_DATA "input*.json" "output*.json")

foreach(TEST_DATA_FILE ${SIMULATOR_TEST_DATA})
  configure_file(${TEST_DATA_FILE} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()

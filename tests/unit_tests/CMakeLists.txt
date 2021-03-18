set(TEST_LIBRARIES scheduler)
custom_add_test(NAME OperationTest LIBRARIES ${TEST_LIBRARIES})
custom_add_test(NAME TimeHorizonTest LIBRARIES ${TEST_LIBRARIES})

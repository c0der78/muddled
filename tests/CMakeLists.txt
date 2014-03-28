
FILE(GLOB ENGINE_TEST_SOURCES muddyengine/*.c)
add_executable(test_muddyengine ${ENGINE_TEST_SOURCES})
target_link_libraries(test_muddyengine muddyengine check lua sqlite3)
add_test(test_muddyengine ${CMAKE_CURRENT_BINARY_DIR}/test_muddyengine)
set(SOURCE_FILES
  check_account.c
check_area.c
check_character.c
check_color.c
check_db.c
check_engine.c
check_explored.c
check_flags.c
check_hashmap.c
check_object.c
check_string.c)

add_executable(muddle_test ${SOURCE_FILES})

add_dependencies(muddle_test muddle)

target_include_directories(muddle_test PUBLIC ../src)

target_link_libraries(muddle_test muddle check pthread rt subunit)

enable_testing()

add_test(NAME muddle_test COMMAND muddle_test)


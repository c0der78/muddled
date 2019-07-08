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

find_library_ext(CHECK LIB check HEADER check.h REQUIRED HINTS /usr/local)
find_library_ext(RT LIB rt)
find_library_ext(SUBUNIT LIB subunit HINTS /usr/local)

add_executable(muddle_test ${SOURCE_FILES})

add_dependencies(muddle_test muddle)

message(STATUS "CHECK_LIB = ${CHECK_LIB} CHECK_INCLUDE_DIR = ${CHECK_INCLUDE_DIR}")

target_include_directories(muddle_test PUBLIC ../src PUBLIC ${CHECK_INCLUDE_DIR} PUBLIC ${RT_INCLUDE_DIR} PUBLIC ${SUBUNIT_INCLUDE_DIR})

target_link_libraries(muddle_test muddle ${CHECK_LIB} pthread ${RT_LIB} ${SUBUNIT_LIB})

enable_testing()

add_test(NAME muddle_test COMMAND muddle_test)


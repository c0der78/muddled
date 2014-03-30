
FILE(GLOB SERVER_SOURCES muddyplains/*.c)

add_executable(muddyplains main.c ${SERVER_SOURCES})

add_subdirectory(muddyengine)

include_directories(.)

target_link_libraries(muddyplains muddyengine sqlite3 lua websockets)

install(TARGETS muddyplains DESTINATION bin)
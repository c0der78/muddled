set(HEADER_FILES
  client.h
command.h
importer.h
olc.h
server.h
telnet.h
update.h
websocket.h)

set(SOURCE_FILES
client.c
command_admin.c
command.c
command_communication.c
command_info.c
command_settings.c
importer.c
main.c
olc_area.c
olc.c
olc_engine.c
olc_exit.c
olc_help.c
olc_npc.c
olc_object.c
olc_player.c
olc_room.c
olc_skill.c
olc_social.c
server.c
telnet.c
update.c
websocket.c)

find_library_ext(WEBSOCKETS LIB websockets HEADER libwebsockets.h HINTS /usr/local)

add_executable(muddled ${SOURCE_FILES})

target_include_directories(muddled PUBLIC .. PUBLIC ${WEBSOCKETS_INCLUDE_DIR})

target_link_libraries(muddled muddle ${WEBSOCKETS_LIB})

add_dependencies(muddled muddle)

add_definitions("-DROOT_DIR=\"${CMAKE_INSTALL_PREFIX}\"" "-DMUDDLED_VERSION=\"${PROJECT_VERSION}\"")

install(TARGETS muddled RUNTIME DESTINATION sbin)


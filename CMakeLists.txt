
cmake_minimum_required(VERSION 3.10)

set( CMAKE_C_STANDARD 11 )

project(muddled VERSION 0.5.0)

function(find_library_ext)
  set(options REQUIRED)
  set(singleOpts LIB HEADER)
  set(multiOpts LIB_ARGS HEADER_ARGS HINTS)

  cmake_parse_arguments(FIND_EXT "${options}" "${singleOpts}" "${multiOpts}" ${ARGN})

  if (NOT DEFINED FIND_EXT_LIB OR NOT DEFINED ARGV0)
    message(FATAL_ERROR "Invalid argument")
  endif()

  set(LIBRARY_VAR "${ARGV0}_LIB")
  set(HEADER_VAR "${ARGV0}_INCLUDE_DIR")

  set(FIND_EXT_LIB_VAR "${ARGV0}_LIB_VAR")
  set(FIND_EXT_HEADER_VAR "${ARGV0}_HEADER_VAR")

  set(${LIBARY_VAR} "" PARENT_SCOPE)
  set(${HEADER_VAR} "" PARENT_SCOPE)

  set(LIBRARY_HINTS)
  set(HEADER_HINTS)

  foreach (hint ${FIND_EXT_HINTS})
    list(APPEND ${LIBRARY_HINTS} "${hint}/lib")
    list(APPEND ${HEADER_HINTS} "${hints}/include")
  endforeach(hint)

  find_library(${FIND_EXT_LIB_VAR} NAME ${FIND_EXT_LIB} HINTS ${LIBRARY_HINTS} ${FIND_EXT_LIB_ARGS})

  if (${FIND_EXT_LIB_VAR})

    set(${LIBRARY_VAR} ${${FIND_EXT_LIB_VAR}} PARENT_SCOPE)

    message(STATUS "Found ${ARGV0} libary: ${${FIND_EXT_LIB_VAR}}")

  else()

    if (${FIND_EXT_REQUIRED})
      message(FATAL_ERROR "Could not find library ${FIND_EXT_LIB}")
    endif()

  endif()


  if (DEFINED FIND_EXT_HEADER)

    find_path(${FIND_EXT_HEADER_VAR} NAME ${FIND_EXT_HEADER} HINTS ${HEADER_HINTS} ${FIND_EXT_HEADER_ARGS})

    if (${FIND_EXT_HEADER_VAR})

      set(${HEADER_VAR} ${${FIND_EXT_HEADER_VAR}} PARENT_SCOPE)

    else()

      if (${FIND_EXT_REQUIRED})
        message(FATAL_ERROR "Could not find header ${FIND_EXT_HEADER}")
      endif()

    endif()
  endif()

endfunction()

add_definitions("-pedantic -Wall -Werror")

add_subdirectory(src)
add_subdirectory(tests)

add_subdirectory(web)
add_subdirectory(resources)
add_subdirectory(scripts)

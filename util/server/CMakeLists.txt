
add_executable (server server.c)

include_directories (${GLIB2_INCLUDE_DIRS})

target_link_libraries (server ${GLIB2_LIBRARIES})

set_target_properties(server PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${exe_dir}"
  OUTPUT_NAME "server"
  COMPILE_DEFINITIONS "_POSIX_SOURCE")


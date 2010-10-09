
add_executable (client client.c encode.c)

include_directories (${GLIB2_INCLUDE_DIRS})

target_link_libraries (client ${GLIB2_LIBRARIES})

set_target_properties(client PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${exe_dir}"
  OUTPUT_NAME "client"
  COMPILE_DEFINITIONS "_POSIX_SOURCE")


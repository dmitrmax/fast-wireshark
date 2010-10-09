
subdirs (rwcompare)
if (UNIX)
  subdirs (client server)
endif ()

set_directory_properties (PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY bin
  INCLUDE_DIRECTORIES "")

set (exe_dir "${CMAKE_CURRENT_SOURCE_DIR}/../bin")


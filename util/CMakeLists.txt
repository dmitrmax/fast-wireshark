
subdirs (rwcompare)
if (UNIX)
  subdirs (client server)
endif ()

set_directory_properties (PROPERTIES
  INCLUDE_DIRECTORIES "")


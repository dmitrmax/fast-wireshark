
add_executable (rwcompare pdmlParser.c rwcompare.c xmlPlanWriter.c)

include_directories (${GLIB2_INCLUDE_DIRS})
include_directories (${LIBXML2_INCLUDE_DIR}) 

target_link_libraries (rwcompare ${LIBXML2_STATIC_LIBRARIES})
target_link_libraries (rwcompare ${GLIB2_STATIC_LIBRARIES})
target_link_libraries (rwcompare ${ZLIB_STATIC_LIBRARIES})
target_link_libraries (rwcompare ${MATH_STATIC_LIBRARIES})

set_target_properties(rwcompare PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${exe_dir}"
  OUTPUT_NAME "rwcompare")


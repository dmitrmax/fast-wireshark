
add_executable (rwcompare pdmlParser.c run-exe.c rwcompare.c xmlPlanWriter.c
  ${debug_include_dir}/debug.c)

include_directories (${GLIB2_INCLUDE_DIRS})
include_directories (${LIBXML2_INCLUDE_DIR}) 
include_directories (${debug_include_dir}) 

target_link_libraries (rwcompare ${LIBXML2_LIBRARIES})
target_link_libraries (rwcompare ${GLIB2_LIBRARIES})

set_target_properties(rwcompare PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${exe_dir}"
  OUTPUT_NAME "rwcompare")


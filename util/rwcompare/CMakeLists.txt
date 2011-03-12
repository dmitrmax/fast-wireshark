
set (sources pdmlParser.c run-exe.c rwcompare.c xmlPlanWriter.c
  ${debug_include_dir}/debug.c)

setup_dbgn_defines (${sources})

add_executable (rwcompare ${sources})

find_package(GLIB2 COMPONENTS gthread-2.0)

include_directories (${GLIB2_INCLUDE_DIRS})
include_directories (${LIBXML2_INCLUDE_DIR}) 
include_directories (${debug_include_dir}) 

target_link_libraries (rwcompare ${LIBXML2_LIBRARIES})
target_link_libraries (rwcompare ${GLIB2_LIBRARIES})
target_link_libraries (rwcompare ${GTHREAD2_LIBRARIES})

set_target_properties(rwcompare PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${exe_dir}"
  OUTPUT_NAME "rwcompare")


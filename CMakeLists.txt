
# Just a guess, can probably go older.
cmake_minimum_required (VERSION 2.8)

project (FastWireshark)
set (plugin_name fast)
set (wireshark_src ../wireshark)

#set (CMAKE_BUILD_TYPE RELEASE)

if (UNIX)
  set (cflags "-ansi -pedantic -Wall -fPIC")
  set (install_dir "$ENV{HOME}/.wireshark/plugins")
else ()
  set (install_dir "$ENV{HOMEPATH}/Application Data/Wireshark/plugins")
  ### These defines and cflags copied from wireshark's config.nmake.
  list (APPEND plugin_defines "MSVC_VARIANT=MSVC2008")
  list (APPEND plugin_defines "MSC_VER_REQUIRED=1500")
  list (APPEND plugin_defines "_CRT_SECURE_NO_DEPRECATE")
  list (APPEND plugin_defines "_CRT_NONSTDC_NO_DEPRECATE")
  list (APPEND plugin_defines "WIN32_LEAN_AND_MEAN")
  list (APPEND plugin_defines "_NEED_VAR_IMPORT")
  set (cflags "/Zi /W3 /MP")
  set (lflags "/NOLOGO /INCREMENTAL:no /DYNAMICBASE /MANIFEST:no")

  set (wireshark_libs ${wireshark_src}/epan/libwireshark)
  set (pfx "C:/dev") # Hardcoded >:(
  list (APPEND CMAKE_PREFIX_PATH "${pfx}")
  list (APPEND CMAKE_PREFIX_PATH "${pfx}/lib/glib-2.0")
  set (pfx "")
endif ()

# We ALWAYS must have Wireshark's config.h.
list (APPEND plugin_defines "HAVE_CONFIG_H")

set (plugin_srcs
  debug.c decode.c dissect.c packet-fast.c
  parse-template.c setup.c template.c)

list (APPEND CMAKE_MODULE_PATH ${wireshark_src}/cmake/modules)

find_package (LibXml2 REQUIRED)
#find_package (ZLIB REQUIRED)
find_package (GLIB2 REQUIRED)

include_directories (${wireshark_src}) 
include_directories (${GLIB2_INCLUDE_DIRS})
include_directories (${LIBXML2_INCLUDE_DIR}) 

add_library (plugin SHARED ${plugin_srcs})
target_link_libraries (plugin ${LIBXML2_LIBRARIES})
target_link_libraries (plugin ${wireshark_libs})
target_link_libraries (plugin ${GLIB2_LIBRARIES})
#target_link_libraries (plugin ${ZLIB_LIBRARIES})

set_target_properties(plugin PROPERTIES
  OUTPUT_NAME "${plugin_name}"
  PREFIX ""
  COMPILE_DEFINITIONS "${plugin_defines}"
  COMPILE_FLAGS "${cflags}"
  LINK_FLAGS "${lflags}")

install (TARGETS plugin DESTINATION ${install_dir})

set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "FAST Protocol for Wireshark")
set (CPACK_PACKAGE_VENDOR "Some Students")
include (CPack)


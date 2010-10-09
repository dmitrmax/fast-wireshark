
# Just a guess, can probably go older.
cmake_minimum_required (VERSION 2.8)

subdirs (util)

project (FastWireshark)

set (plugin_srcs
  debug.c decode.c dissect.c packet-fast.c
  parse-template.c template.c)

set (plugin_name fast)
set (wireshark_src "${CMAKE_CURRENT_SOURCE_DIR}/../wireshark")

if (UNIX)
  set (install_dir "$ENV{HOME}/.wireshark/plugins")
  if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE DEBUG)
  endif ()
  set (CMAKE_C_FLAGS_DEBUG "-ansi -pedantic -Wall")
else ()
  set (install_dir "$ENV{HOMEPATH}/Application Data/Wireshark/plugins")
  # Always build as release mode. Wireshark was built the same way.
  if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE RELEASE)
  endif ()
  ### These defines and cflags copied from wireshark's config.nmake.
  list (APPEND plugin_defines "MSVC_VARIANT=MSVC2008")
  list (APPEND plugin_defines "MSC_VER_REQUIRED=1500")
  list (APPEND plugin_defines "_CRT_SECURE_NO_DEPRECATE")
  list (APPEND plugin_defines "_CRT_NONSTDC_NO_DEPRECATE")
  list (APPEND plugin_defines "WIN32_LEAN_AND_MEAN")
  list (APPEND plugin_defines "_NEED_VAR_IMPORT")
  set (plugin_cflags "/Zi /W3 /MP")
  set (plugin_lflags "/NOLOGO /INCREMENTAL:no /DYNAMICBASE /MANIFEST:no")

  set (wireshark_libs ${wireshark_src}/epan/libwireshark)
  set (pfx "C:/dev") # Hardcoded >:(
  list (APPEND CMAKE_PREFIX_PATH "${pfx}")
  list (APPEND CMAKE_PREFIX_PATH "${pfx}/lib/glib-2.0")
  set (pfx "")
endif ()

# We ALWAYS must have Wireshark's config.h.
list (APPEND plugin_defines "HAVE_CONFIG_H")

list (APPEND CMAKE_MODULE_PATH ${wireshark_src}/cmake/modules)

macro (staticlibify destlibs)
  foreach (lib ${ARGN})
    string (REGEX REPLACE
      "\\${CMAKE_SHARED_LIBRARY_SUFFIX}$"
      "${CMAKE_STATIC_LIBRARY_SUFFIX}"
      staticlib "${lib}")
    list (APPEND ${destlibs} ${staticlib})
    #message ("${staticlib}")
  endforeach ()
endmacro ()

find_package (LibXml2 REQUIRED)
find_package (GLIB2 REQUIRED)
find_package (ZLIB REQUIRED)
find_library (MATH_LIBRARIES m)

staticlibify (LIBXML2_STATIC_LIBRARIES ${LIBXML2_LIBRARIES})
staticlibify (GLIB2_STATIC_LIBRARIES ${GLIB2_LIBRARIES})
staticlibify (ZLIB_STATIC_LIBRARIES ${ZLIB_LIBRARIES})
staticlibify (MATH_STATIC_LIBRARIES ${MATH_LIBRARIES})

include_directories (${wireshark_src}) 
include_directories (${GLIB2_INCLUDE_DIRS})
include_directories (${LIBXML2_INCLUDE_DIR}) 

add_library (plugin SHARED ${plugin_srcs})
target_link_libraries (plugin ${wireshark_libs})
target_link_libraries (plugin ${LIBXML2_STATIC_LIBRARIES})
target_link_libraries (plugin ${GLIB2_STATIC_LIBRARIES})

set_target_properties(plugin PROPERTIES
  OUTPUT_NAME "${plugin_name}"
  PREFIX ""
  COMPILE_DEFINITIONS "${plugin_defines}"
  LINK_FLAGS "${plugin_lflags}")

install (TARGETS plugin DESTINATION ${install_dir})

set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "FAST Protocol for Wireshark")
set (CPACK_PACKAGE_VENDOR "Some Students")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "4")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_PACKAGE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

set (CPACK_NSIS_URL_INFO_ABOUT "http://code.google.com/p/fast-wireshark")

include (CPack)


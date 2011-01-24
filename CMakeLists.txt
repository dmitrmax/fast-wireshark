
# Just a guess, can probably go older.
cmake_minimum_required (VERSION 2.8)

subdirs (util)

project (FastWireshark)

set (plugin_srcs
  basic-dissect.c basic-field.c
  debug.c debug-tree.c decode.c dictionaries.c dissect.c
  packet-fast.c parse-template.c template.c)

set (plugin_name fast)
set (wireshark_src "${CMAKE_CURRENT_SOURCE_DIR}/../wireshark")
set (exe_dir "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set (debug_include_dir "${CMAKE_CURRENT_SOURCE_DIR}")

if (UNIX)
  set (install_dir "$ENV{HOME}/.wireshark/plugins")
  if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE DEBUG)
  else ()
    set (install_dir "${CMAKE_INSTALL_PREFIX}/share/wireshark/plugins")
  endif ()
  set (CMAKE_C_FLAGS "-Wall")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ansi -pedantic")
else ()
  if (NOT install_dir)
    #set (install_dir "$ENV{HOMEPATH}/Application Data/Wireshark/plugins")
    set (install_dir "/Documents and Settings/All Users/Application Data/Wireshark/plugins")
  endif ()
  # Always build as release mode. Wireshark was built the same way.
  set (CMAKE_BUILD_TYPE RELEASE)
  ### These defines and cflags copied from wireshark's config.nmake.
  list (APPEND plugin_defines "MSVC_VARIANT=MSVC2008")
  list (APPEND plugin_defines "MSC_VER_REQUIRED=1500")
  list (APPEND plugin_defines "_CRT_SECURE_NO_DEPRECATE")
  list (APPEND plugin_defines "_CRT_NONSTDC_NO_DEPRECATE")
  list (APPEND plugin_defines "WIN32_LEAN_AND_MEAN")
  list (APPEND plugin_defines "_NEED_VAR_IMPORT")
  set (CMAKE_C_FLAGS "/Zi /W3 /MP")
  set (plugin_lflags "/NOLOGO /INCREMENTAL:no /DYNAMICBASE /MANIFEST:no")
endif ()

# We ALWAYS must have Wireshark's config.h.
list (APPEND plugin_defines "HAVE_CONFIG_H")

list (APPEND CMAKE_MODULE_PATH ${wireshark_src}/cmake/modules)

# Usage:
#   setup_dbgn_defines (${my_sources})
macro (setup_dbgn_defines)
  foreach (src_file ${ARGN})
    set_property(SOURCE ${src_file}
      APPEND PROPERTY COMPILE_DEFINITIONS DBGN_FILENAME="${src_file}")
  endforeach ()
endmacro ()

setup_dbgn_defines (${plugin_srcs})

# Useage:
#   staticlibify (LIBXML2_STATIC_LIBRARIES ${LIBXML2_LIBRARIES})
macro (staticlibify destlibs)
  foreach (lib ${ARGN})
    message ("${lib}")
    string (REGEX REPLACE
      "\\${CMAKE_SHARED_LIBRARY_SUFFIX}$"
      "${CMAKE_STATIC_LIBRARY_SUFFIX}"
      staticlib "${lib}")
    list (APPEND ${destlibs} ${staticlib})
    message ("${staticlib}")
  endforeach ()
endmacro ()

if (UNIX)
  set (wireshark_libs "")
  find_package (LibXml2 REQUIRED)
else ()
  set (pfx "C:/dev") # Hardcoded >:(
  list (APPEND CMAKE_PREFIX_PATH "${pfx}")
  list (APPEND CMAKE_PREFIX_PATH "${pfx}/lib/glib-2.0")
  set (wireshark_libs "${wireshark_src}/epan/libwireshark.lib")
  find_package (LibXml2 REQUIRED)
  set (LIBXML2_LIBRARIES "")
  list (APPEND LIBXML2_LIBRARIES "${pfx}/lib/libxml2_a.lib")
  list (APPEND LIBXML2_LIBRARIES "${pfx}/lib/iconv_a.lib")
  set (pfx "")
endif ()

find_package (GLIB2 REQUIRED)


include_directories (${wireshark_src}) 
include_directories (${GLIB2_INCLUDE_DIRS})
include_directories (${LIBXML2_INCLUDE_DIR}) 


add_library (plugin SHARED ${plugin_srcs})
target_link_libraries (plugin ${wireshark_libs})
target_link_libraries (plugin ${LIBXML2_LIBRARIES})
target_link_libraries (plugin ${GLIB2_LIBRARIES})

set_target_properties(plugin PROPERTIES
  OUTPUT_NAME "${plugin_name}"
  PREFIX ""
  COMPILE_DEFINITIONS "${plugin_defines}"
  LINK_FLAGS "${plugin_lflags}")

set (CMAKE_INSTALL_PREFIX "${install_dir}" )
install (TARGETS plugin DESTINATION "." COMPONENT plugin)

# Override destination with CMAKE_INSTALL_PREFIX.
set (CPACK_SET_DESTDIR true)

set (CPACK_COMPONENTS_ALL plugin)

set (CPACK_PACKAGE_NAME "fastshark")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "FAST Protocol for Wireshark")
set (CPACK_PACKAGE_VENDOR "Some Students")
set (CPACK_PACKAGE_CONTACT "http://code.google.com/p/fast-wireshark")
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "4")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

set (CPACK_NSIS_URL_INFO_ABOUT "http://code.google.com/p/fast-wireshark")

include (CPack)


enable_testing ()

set (test_dir "test")
set (tshark_exe "tshark")
set (rwcompare_exe "${exe_dir}/rwcompare")
set (plan_runner "${test_dir}/OpenFastPlanRunner.jar")
set (port_num "5000")

# Add an rwcompare test.
macro (rwcompare test_name)
  add_test ("${test_name}"
    "${rwcompare_exe}"
    "port" "${port_num}" "tshark" "${tshark_exe}"
    "runner" "${plan_runner}"
    "tmpl" "${test_dir}/templates.xml"
    ${ARGN})
endmacro ()

rwcompare (IntegerFields send "${test_dir}/plans/intPlan.xml")
rwcompare (DecimalFields send "${test_dir}/plans/decimal.xml")
rwcompare (StringFields  send "${test_dir}/plans/stringPlan.xml")
rwcompare (GroupSequence send "${test_dir}/plans/groupSequencePlan.xml")
rwcompare (OptionalFields send "${test_dir}/plans/optional.xml")
rwcompare (UInt32CopyDictionary send "${test_dir}/plans/uintDictionaryPlan.xml")
rwcompare (ByteVectorFields send "${test_dir}/plans/byteVectorPlan.xml")
rwcompare (CopyEmptyFields send "${test_dir}/plans/copyEmpty.xml")
rwcompare (OptionalNestedFields send "${test_dir}/plans/optionalNested.xml")
rwcompare (Copy send "${test_dir}/plans/copy.xml")
rwcompare (Default send "${test_dir}/plans/default.xml")
rwcompare (Delta send "${test_dir}/plans/delta.xml")
rwcompare (Increment send "${test_dir}/plans/increment.xml")
rwcompare (Value send "${test_dir}/plans/value.xml")


rwcompare (MultiMessage send "${test_dir}/byteplans/multimessage.xml"
  expect "${test_dir}/plans/multimessage.xml" bypass)
rwcompare (TooManyIntBits send "${test_dir}/byteplans/toomanyintbits.xml"
  expect "${test_dir}/plans/toomanyintbits.xml")
rwcompare (TooMantIntBytes send "${test_dir}/byteplans/toomanyintbytes.xml"
  expect "${test_dir}/plans/toomanyintbytes.xml")
rwcompare (ByteVectorDelta send "${test_dir}/byteplans/bv_delta.xml"
  expect "${test_dir}/plans/bv_delta.xml" bypass "tmpl" "${test_dir}/bv_templates.xml")

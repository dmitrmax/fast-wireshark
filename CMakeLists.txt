# CMakeLists.txt
#
# This file is part of FAST Wireshark.
#
# FAST Wireshark is free software: you can redistribute it and/or modify
# it under the terms of the Lesser GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# FAST Wireshark is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# Lesser GNU General Public License for more details.
#
# You should have received a copy of the Lesser GNU General Public License
# along with FAST Wireshark.  If not, see
# <http://www.gnu.org/licenses/lgpl.txt>.
#

include(WiresharkPlugin)

# Plugin name and version info (major minor micro extra)
set_module_info(fast 1 1 0 0)

set(DISSECTOR_SRC
  address-utils.c
  basic-dissect.c
  basic-field.c
  debug.c
  debug-tree.c
  decode.c
  dictionaries.c
  dissect.c
  error_log.c
  packet-fast.c
  parse-template.c
  template.c
)

set(PLUGIN_FILES
	plugin.c
	${DISSECTOR_SRC}
)

set(CLEAN_FILES
	${PLUGIN_FILES}
)

set_source_files_properties(
	${CLEAN_FILES}
	PROPERTIES
	COMPILE_FLAGS "${WERROR_COMMON_FLAGS}"
)

find_package( LibXml2 REQUIRED )

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LIBXML2_INCLUDE_DIR})

register_dissector_files(plugin.c
	plugin
	${DISSECTOR_SRC}
)

add_plugin_library(fast)

target_link_libraries(fast ${LIBXML2_LIBRARIES})

install(TARGETS fast
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CPACK_PACKAGE_NAME}/plugins/${CPACK_PACKAGE_VERSION} NAMELINK_SKIP
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CPACK_PACKAGE_NAME}/plugins/${CPACK_PACKAGE_VERSION}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${CPACK_PACKAGE_NAME}/plugins/${CPACK_PACKAGE_VERSION}
)

file(GLOB DISSECTOR_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
CHECKAPI(
	NAME
	  fast
	SOURCES
	  ${DISSECTOR_SRC}
	  ${DISSECTOR_HEADERS}
)

#
# Editor modelines  -  http://www.wireshark.org/tools/modelines.html
#
# Local variables:
# c-basic-offset: 8
# tab-width: 8
# indent-tabs-mode: t
# End:
#
# vi: set shiftwidth=8 tabstop=8 noexpandtab:
# :indentSize=8:tabSize=8:noTabs=false:
#

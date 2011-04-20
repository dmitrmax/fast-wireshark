/*
 * This file is part of FAST Wireshark.
 *
 * FAST Wireshark is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * FAST Wireshark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 * 
 * You should have received a copy of the Lesser GNU General Public License
 * along with FAST Wireshark.  If not, see 
 * <http://www.gnu.org/licenses/lgpl.txt>.
 */
#ifndef RWCOMPARE_H_
#define RWCOMPARE_H_

#include <glib.h>
#include <libxml/parser.h>

int ArgParseBailOut(const char* arg, const char* reason);
gboolean equiv_plan_files (const char* plan_filename,
                           const char* expect_filename);
gboolean equiv_plan_xml (xmlNodePtr cnode,
                         xmlNodePtr enode);
xmlNodePtr next_xml_node (xmlNodePtr node);
gboolean ignore_xml_node (xmlNodePtr xmlnode);

#endif


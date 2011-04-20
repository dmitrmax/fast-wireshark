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

#ifndef PARSETEMPLATE_H_INCLUDED_
#define PARSETEMPLATE_H_INCLUDED_

#include <glib.h>

/*! \brief  Convert an XML file into an internal representation of
*          the templates.
* \param filename  Name of the XML file to parse.
* \return  An internal tree of FieldTypes.
*/
GNode* parse_templates_xml (const char* filename);

#endif


/*** Parse Template.c Created and last maintained by Eric Warsop.
	This file utilizes LibXML2 and internal project code to read in a template XML file
	and output a template or multiple templates contained within. 

	You'll not a distinct lack of 
	else-if where a lot of ifs are repeated. This may be bad practice, but there is no significant risk in msot cases 
***/

/* Needed includes from both project internals, and libxml2 */
#include "fast.h"
#include "template.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

/* here are our two functions*/
/* parse_xml is the funciton called from packet-fast.c It finds the verious templates and has them parsed. */
void parse_xml(const char *);
/* parse_template is passed a node to a template and tries to read the things inside of it. */
void parse_template(xmlDocPtr, xmlNodePtr, struct template_type*);
/* assign_Value is a helper function made to hide some of the confusing amounts of cross-referencing in determining what to do in which cases. */
void assign_Value(xmlChar *, struct template_field_type *, xmlChar *);

/* Code for parse_xml. Comments below */
void parse_xml(const char* template){
	
	/* 	Our main vairables. the xmlDocPtr doc is a pointer to the document, 
		the xmlNode pointer cur is the current node we are at. We traverse the document like a tree. */
	xmlDocPtr doc;
	xmlNodePtr cur;

	/* read in the document and parse it in one line of code */
	doc = xmlParseFile(template);
	
	/* Obligatory error checking. note that doc == NULL is bad state of code */
	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return;
	}

	/* Get the where the document starts */
	cur = xmlDocGetRootElement(doc);
	
	/* More error checking, this in the case of an empty document. */
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return;
	}
	/* just some friendly output for testing purposes, this should be removed when code is working right */	
	/*printf("The root node is %s.\n", (const char *) cur->name); *** We can comment this out. *** */

	/* This is in the case that our file maintains one template. If there is one template we do not go in. If there is, we go into the templates section and work */
	if (cur->name != (const xmlChar *)"template")
		cur = cur->xmlChildrenNode;

	/* Begin main parsing loop. Get the first child of the root, then loop from there. Any "template" nodes imply we have a template to parse, so we send it on over to parse_template! */
	
	while (cur != NULL) {
/*		printf("Node: %s.\n", cur->name); *** Random debugging output. We can comment out unless we want it to identify what nodes we are hitting. *** */
		/* If we have hit a template, pull our relevant information and read it's contents */
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"template"))) {
			xmlChar * name;
			xmlChar * id;
			struct template_type * template;
			name = xmlGetProp(cur,(const xmlChar*) "name");
			id = xmlGetProp(cur,(const xmlChar*) "id");
			printf("Name: %s. \nID: %s \n", name, id);
			/* once we have needed info, Create a template structure, then fill it in with parse_template() */
			create_template((const char *)name, atoi((const char*)id), &template);
			parse_template(doc, cur->xmlChildrenNode, template);
			/* the xmlGetProp() function does allocate memory, so we have to free it with xmlFree() in order to not leak */
			xmlFree(name);
			xmlFree(id);
		}
		/* check the next top level node, loop. */
		cur = cur->next;
	}
	/* Clean up and finish the function */
	xmlFreeDoc(doc);
	return;


}

/* our lovely Parse-template function. This function builds our template internal data structure, but is currently in need of fixing */
/* TODO: Added functionality for: Byte Vectors, Decimals, Groups, Sequences. Dictionary values can be looked at but are not used. */
void parse_template(xmlDocPtr doc, xmlNodePtr cur, struct template_type* template) {

	/* convience vairable for storing values. Namely needed values. */
	xmlChar *name;
	xmlChar *value;
	xmlChar *id;
	xmlChar *presence;


	/* this means we have nothing to do, me thinks  *** UPDATE: clean this out when code is finished. This is old debugging code *** */
/*	if (cur == NULL){
		printf("Well that was a bust, the node is NULL!\n");
	}*/

		/* Time to start on with sorting throguh the nodes. */ 
		while (cur != NULL) {
			/* Find the element of the current node and move into more looping */		
			
			/* get needed variables set up. 
			/* create is the field added in case we have a valid node */
			struct template_field_type create;			
			/* fop is short of Field Operator, in this exception. it exists do that code runs simpler. */ 
			guint fop;

			/* this is needed information found within the tag, as attributes */
			name = xmlGetProp(cur,(const xmlChar*) "name");
			id = xmlGetProp(cur,(const xmlChar*) "id");

			/* set up the field from the current node */
			presence = xmlGetProp(cur,(const xmlChar*) "presence");
			create.name=(char *)cur->name;
			if ((!xmlStrcmp(presence, (const xmlChar *)"optional"))) {	
				create.mandatory=0;
			}
			else if ((!xmlStrcmp(presence, (const xmlChar *)"mandatory"))) {
				create.mandatory=1;
			}
			else {
				create.mandatory=1;
			}
			
			/* check for fields in subset notes */
			if (cur->xmlChildrenNode != NULL){
				/* check to see if there is a value. If there is, call assign_Value() to figure that out and see where we put it in the field. */
				value = xmlGetProp(cur->xmlChildrenNode->next,(const xmlChar*) "value");
				if (value != NULL){
					assign_Value(name, &create, value);
				}
				/* Check for field operators. These are tags withing the Field, so they fit in the first level down, except in the case of decimals. Decimals are special. */
				if ((!xmlStrcmp(cur->xmlChildrenNode->next->name, (const xmlChar *)"default"))) {
					fop=FIELD_OP_DEFAULT;					
				}
				if ((!xmlStrcmp(cur->xmlChildrenNode->next->name, (const xmlChar *)"copy"))) {
					fop=FIELD_OP_COPY;
				}
				if ((!xmlStrcmp(cur->xmlChildrenNode->next->name, (const xmlChar *)"constant"))) {
					fop=FIELD_OP_CONST;
				}
				if ((!xmlStrcmp(cur->xmlChildrenNode->next->name, (const xmlChar *)"increment"))) {
					fop=FIELD_OP_CONST;
				}
				if ((!xmlStrcmp(cur->xmlChildrenNode->next->name, (const xmlChar *)"constant"))) {
					fop=FIELD_OP_CONST;
				}
				if ((!xmlStrcmp(cur->xmlChildrenNode->next->name, (const xmlChar *)"increment"))) {
					fop=FIELD_OP_INCR;
				}
				if ((!xmlStrcmp(cur->xmlChildrenNode->next->name, (const xmlChar *)"delta"))) {
					fop=FIELD_OP_DELTA;
				}
				if ((!xmlStrcmp(cur->xmlChildrenNode->next->name, (const xmlChar *)"tail"))) {
					fop=FIELD_OP_TAIL;
				}
				/* Avoiding memory leaks */
				xmlFree(value);			
/*				printf("SubNodeName: %s \n", cur->xmlChildrenNode->next->name); *** more debugging output that is less than nessesary *** */
			}

			/* Determeine Type and create */
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"int32"))) {
				create.type=FIELD_TYPE_INT32|fop;
				create_field(template, &create, NULL);
			}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"uInt32"))) {
				create.type=FIELD_TYPE_UINT32|fop;
				create_field(template, &create, NULL);
			}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"int64"))) {
				create.type=FIELD_TYPE_INT64|fop;
				create_field(template, &create, NULL);
			}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"uInt64"))) {
				create.type=FIELD_TYPE_UINT64|fop;
				create_field(template, &create, NULL);
			}
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"string"))) {
				create.type=FIELD_TYPE_ASCII|fop;
				create_field(template, &create, NULL);
			}
/*			printf("Propname: %s \nnode name: %s \nPropPresence: = %s \nPropid: %s \n", name, cur->name, presence, id); *** some detailed debugging output also outdated. *** */

			/* Quick, Free the memory, don't let it get lost and leak all over! */
			xmlFree(name);
			xmlFree(presence);
			xmlFree(id);
			/* Check the next node and loop. Standard Procedure */
			cur = cur->next;
		}
	/* all done here, nothing to see, move along now */
	return;
}

/* This is one set of checks, but we needed different work for default and other stuff. 
	In thoery, this is sorting where the value goes based on first it's operator, then scond it's data type 
	So, it is really long and boring code set over here for readability. Just you you check for the seperate cases of each 
	Operator vs each data type, I just have compressed it into two if statements with a nested set of if statements.

	NOTICE: this will have to be re-written or have to have a sister method made for Decmials. possibly groups and sequences, too. */
void assign_Value(xmlChar * name, struct template_field_type * create, xmlChar *value){	
	
	if ((!xmlStrcmp(name, (const xmlChar *)"default"))) {
		if ((!xmlStrcmp(name, (const xmlChar *)"int32"))) {
			(*create).cfg_value.i32=(gint32)atoi((const char *)value);
		}
		if ((!xmlStrcmp(name, (const xmlChar *)"uInt32"))) {
			(*create).cfg_value.u32=(gint32)atoi((const char *)value);
		}
		if ((!xmlStrcmp(name, (const xmlChar *)"int64"))) {
			(*create).cfg_value.i64=(gint32)atoi((const char *)value);
		}
		if ((!xmlStrcmp(name, (const xmlChar *)"uInt64"))) {
			(*create).cfg_value.u64=(gint32)atoi((const char *)value);
		}
		if ((!xmlStrcmp(name, (const xmlChar *)"string"))) {
			(*create).cfg_value.str.p=(guint8 *) value;
		}
	}
	if 	((!xmlStrcmp(name, (const xmlChar *)"copy")) 
		|| (!xmlStrcmp(name, (const xmlChar *)"constant"))
		|| (!xmlStrcmp(name, (const xmlChar *)"increment"))
		|| (!xmlStrcmp(name, (const xmlChar *)"constant")) 
		|| (!xmlStrcmp(name, (const xmlChar *)"delta")) 
		|| (!xmlStrcmp(name, (const xmlChar *)"tail"))) {
			if ((!xmlStrcmp(name, (const xmlChar *)"int32"))) {
				(*create).value.i32=(gint32)atoi((const char *)value);
			}
			if ((!xmlStrcmp(name, (const xmlChar *)"uInt32"))) {
				(*create).value.u32=(gint32)atoi((const char *)value);
			}
			if ((!xmlStrcmp(name, (const xmlChar *)"int64"))) {
				(*create).value.i64=(gint32)atoi((const char *)value);
			}
			if ((!xmlStrcmp(name, (const xmlChar *)"uInt64"))) {
				(*create).value.u64=(gint32)atoi((const char *)value);
			}
			if ((!xmlStrcmp(name, (const xmlChar *)"string"))) {
				(*create).value.str.p=(guint8 *) value;
			}					
		}	
}

#include "fast.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


void parse_xml(const char *);
void parse_template(xmlDocPtr, xmlNodePtr);


void parse_xml(const char* template){

	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(template);
	
	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return;
	}

	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return;
	}
	
	printf("The root node is %s.\n", (const char *) cur->name);

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
/*		printf("Node: %s.\n", cur->name);*/
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"template"))) {
			parse_template(doc, cur->xmlChildrenNode);
		}

		cur = cur->next;
	}
	
	xmlFreeDoc(doc);
	return;


}

void parse_template(xmlDocPtr doc, xmlNodePtr cur) {

	xmlChar *key;
/*	puts("Parsing...kinda.\n");*/
	cur = cur->xmlChildrenNode;
	if (cur == NULL)
/*		printf("Well that was a bust, the node is NULL!\n");*/
	while (cur != NULL) {
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		printf("keyword: %s\n, node name: %s", key, cur->name);
		xmlFree(key);
		cur = cur->next;
		puts("PARSE PARSE PARSE \n");
	}
    return;
}



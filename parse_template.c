#include "fast.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

/* here are our two functions*/
/*****
	parse_xml is the funciton called form packet-fast.c It finds the verious templates and has them parsed
*****/
void parse_xml(const char *);
/*****
	parse_template is passed a node to a template and tries to read the things inside of it. 
	****** currently only finds null. by output it seems to be an issue with parse_xml not parsing the file right
*****/
void parse_template(xmlDocPtr, xmlNodePtr);

/*****
	Code for parse_xml. Comments below
*****/
void parse_xml(const char* template){
	
	/*****
		Our main vairables. the xmlDocPtr doc is a pointer to the document, 
		the xmlNode pointer cur is the current node we are at. We traverse the document like a tree
	*****/
	xmlDocPtr doc;
	xmlNodePtr cur;

	/*****
	read in the document and parse it in one line of code
	*****/
	doc = xmlParseFile(template);
	
	/*****
	Obligatory error checking. note that doc == NULL is an error code
	*****/
	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return;
	}

	/*****
	Get the where the document starts
	*****/
	cur = xmlDocGetRootElement(doc);
	
	/*****
	More error checking, this in the case of an empty document.
	*****/
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return;
	}
	/* just some friendly output for testing purposes, this should be removed when code is working right */	
	printf("The root node is %s.\n", (const char *) cur->name);

	/*****
	Begin main parins loop. get the first child of the root, then loop from there. Any "template" nodes imply we have a 		template to parse, so we send it on over to parse_template!
	*****/
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
/*		printf("Node: %s.\n", cur->name);*/
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"template"))) {
			parse_template(doc, cur->xmlChildrenNode);
		}

		cur = cur->next;
	}
	/*****
	Clean up and finish the function
	*****/
	xmlFreeDoc(doc);
	return;


}
/*****
our lovely Parse-template function. This function will eventually build our templates we use, but for now it just tries to traverse them
***** WARNING - Fuinction does not work. possibly an issue in parse_xml, possibly an issue in how we are trying to do things and
	we are doing them wrong. *****
*****/
void parse_template(xmlDocPtr doc, xmlNodePtr cur) {

	/*****
	convience vairable for storing values
	*****/
	xmlChar *key;
/*	puts("Parsing...kinda.\n");*/
	/*****
	move along to the child, then parse
	*****/
	cur = cur->xmlChildrenNode;
	/*****
	this means we have nothing to do, me thinks
	*****/
/*	if (cur == NULL){
		printf("Well that was a bust, the node is NULL!\n");
	}*/
		while (cur != NULL) {
			/*****
			Find the element of the current node and move into more looping
			*****/		
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			printf("keyword: %s\n, node name: %s", key, cur->name);
			xmlFree(key);
			cur = cur->next;
	/*		puts("PARSE PARSE PARSE \n"); - Silly debugging statement */
		}
	/*****
	all done here, nothing to see, move along now
	*****/
	return;
}



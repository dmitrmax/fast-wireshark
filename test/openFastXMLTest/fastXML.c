#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


#define false 0
#define true 1

/* Attributes of fields.  
   Ex. In <string charset="ascii" id="2" name="a" />
   charset="ascii" is an attribute */
struct att {
	char *name;   /* name of attribute ex. (presense, charset ... ect) */
	char *value;  /* value of attribute */
	struct att * next; /* Pointer to next attribute */
};

/* operators associated with fields */
struct operator {
	char *type; /* type of operator (delta, copy ... ect) */
	char *value;  /* value of operator (may be null, meaning no value given) */
	struct operator * next; /* Pointer to next operator */
};

/* Fields within a template */
struct field {
		char *type;  /* Type of field ex. (uint32, string ... ect ) */
		int id;      /* id of field */
		char *name;  /* Name of the field */
		struct att * attsHead; /* Pointer to head of attribute list */
		struct att * attsTail; /* Pointer to tail of attribute list */
		struct operator * operatorsHead; /* Pointer to head of operator list */
		struct operator * operatorsTail; /* Pointer to tail of operator list */
		struct field * next; /* Pointer to next field */
};

struct template {
	int id;  /* id of template */
	char *name;  /* Name of the template */
	struct field * fieldsHead; /* Pointer to head of field list */
	struct field * fieldsTail; /* Pointer to tail of field list */
	struct template * next;  /* Pointer to next template */
};

/* Pointers to head and tail of template list */
struct template * templatesHead;
struct template * templatesTail;

/* Create a new template and add it to the list of templates */
struct template * newTemplate(){

	struct template * t = malloc(sizeof(struct template));
	t->fieldsHead=NULL;
	t->next=NULL;

	if(templatesHead==NULL){
		templatesHead = t;
		templatesTail = t;
	} else {
		templatesTail->next = t;
		templatesTail = t;
	}

	return t;
}

/* Create a new field and add it to the given template */
struct field * newField(struct template *t){

	struct field * f = malloc(sizeof(struct field));
	f->attsHead = NULL;
	f->attsTail = NULL;
	f->operatorsHead = NULL;
	f->operatorsTail = NULL;
	f->next = NULL;

	if(t->fieldsHead==NULL){
		t->fieldsHead = f;
		t->fieldsTail = f;
	} else {
		t->fieldsTail->next = f;
		t->fieldsTail = f;
	}

	return f;
}

/* Create a new operator and add it to the given field */
struct operator * newOperator(struct field *f){
	
	struct operator * o = malloc(sizeof(struct operator));
	o->next = NULL;
	o->type = NULL;
	o->value = NULL;

	if(f->operatorsHead==NULL){
		f->operatorsHead = o;
		f->operatorsTail = o;
	} else {
		f->operatorsTail->next = o;
		f->operatorsTail = o;
	}

	return o;
}

/* Create a new attribute and add it to the given field */
struct att * newAttribute(struct field *f){

	struct att * a = malloc(sizeof(struct att));
	a->next = NULL;

	if(f->attsHead==NULL){
		f->attsHead = a;
		f->attsTail = a;
	} else {
		f->attsTail->next = a;
		f->attsTail = a;
	}

	return a;
}

	

/* Print a field to the console for debuging and testing */
void printField (struct field *f){

	printf(" type: %s  id:%d  name: %s\n", f->type, f->id, f->name);
  if(f->attsHead!=NULL){
  	printf("  Attributes:\n");
		struct att * a = f->attsHead;
		while(a!=NULL){
			printf("   name: %s  value: %s\n",a->name, a->value);
			a = a->next;
		}
	}
	if(f->operatorsHead!=NULL){
		printf("  Operators:\n");
		struct operator * o = f->operatorsHead;
		while(o!=NULL){
			printf("   type: %s  value: %s\n", o->type, o->value);
			o = o->next;
		}
	}

}


/* Print a template to the console for debuging and testing */
void printTemplate(struct template *t){

	printf("Template:\n");
	printf(" id: %d  name: %s\n", t->id, t->name);
	if(t->fieldsHead!=NULL){
		printf(" Fields:\n");
		struct field * f = t->fieldsHead;
		while(f!=NULL){
			printField(f);
			f = f->next;
		}
	}

}

/* Print all templates to the console for debuging and testing */
void printTemplates(){

	if(templatesHead==NULL){
		printf("No templates\n");
	} else {
		struct template * t = templatesHead;
		while(t!=NULL){
			printTemplate(t);
			t = t->next;
		}
	}
}


/* Parse the operator section of the XML document */
int parseOperators( xmlDocPtr doc, xmlNodePtr cur, struct field *f){

	/* printf("Parseing Operators ...\n"); */
  xmlChar *prop;

	/* loop through field and find operators */
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"text"))) {
			/* Ignore text between fields (for now at least)*/
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"comment"))) {
			/* Ignore comments */
		} else {

    	/* printf("found %s operator\n", cur->name); */
			if(!validOperator(f->type, cur->name)){
						fprintf(stderr,"Invalid operator (%s) for field %s\n", cur->name, f->type);
						return false;
			}
			
			/* create a new operator and add it to the field */
			struct operator * o = newOperator(f);

			/* copy type string */
			char * n = malloc(strlen(cur->name) * sizeof(char));
			strcpy(n,cur->name);
			o->type = n;
			
			/* get int value of operator if given */
			prop = xmlGetProp( cur, "value");
			if(prop!=NULL){
				o->value = prop;
			} else {
				o->value = NULL;			
			}

		}
    cur = cur->next;
	}

	/* printf("Done Parseing Operators\n"); */
	return true;
}

/* Parse a template section of the XML document */
int parseTemplate (xmlDocPtr doc, xmlNodePtr cur) {

  /* printf("Parseing template ...\n"); */
	xmlChar *key;
  xmlChar *prop;

	/* create a new template to begin filling it in */
	struct template * t = newTemplate();
	t->name = xmlGetProp( cur, "name");

	/* get int value of id */
	prop = xmlGetProp( cur, "id");
	if(prop!=NULL){
		t->id = atoi(prop);
	} else { 
		t->id=-1;
	}

	/* loop through template and find fields */
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"text"))) {
			/* Ignore text between fields (for now at least)*/
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"comment"))) {
			/* Ignore comments */
		} else {

    	/* printf("found %s field\n", cur->name); */

			/* check if this is a valid field */
			if(!validField(cur->name)){
				fprintf(stderr,"Invalid field %s in template %s\n", cur->name, t->name);
				return false;
			}
			
			/* create a new field and add it to the template */
			struct field * f = newField(t);

			/* copy type string */
			char * n = malloc(strlen(cur->name) * sizeof(char));
			strcpy(n,cur->name);
			f->type = n;
			
			/* get int value of id */
			prop = xmlGetProp( cur, "id");
			if(prop!=NULL){
				f->id = atoi(prop);
			} else { f->id=-1; }

			/* get name */
			f->name = xmlGetProp( cur, "name");

			/* Get all field attributes */
			xmlAttrPtr attr;
			for(attr = cur->properties; NULL != attr; attr = attr->next){
				/* the name of the attribute is in attr->name */
				if((!xmlStrcmp(attr->name, (const xmlChar *)"name"))){
					/* do nothing with specical attribute */
				} else if((!xmlStrcmp(attr->name, (const xmlChar *)"id"))){
					/* do nothing with specical attribute */
				} else {
    			
					prop = xmlGetProp( cur, attr->name );
					/* printf("found attribute %s = %s\n", attr->name, prop); */
					if(!validAttribute(f->type, prop)){
						fprintf(stderr,"Invalid attribute (%s) for field %s\n", prop, f->type);
						return false;
					}
					/* create attribute and add it to the field */
					struct att * a = newAttribute(f);
					/* copy attribute name string */
					char * n = malloc(strlen(attr->name) * sizeof(char));
					strcpy(n,attr->name);
					a->name = n;

					a->value = prop;
				}    			
			}

			/* find operators for field and check if parse failed*/
			if(!parseOperators(doc, cur, f)){
				return false;
			}

			/* Get text between field.  Ex. <uint32 name="test">text</uint32> then key="text"
    	key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    	printf("keyword: %s\n", key);
    	xmlFree(key);*/
    
		}
    cur = cur->next;
	}

	/* printf("Done Parseing template\n"); */

    return true;
}

int parseDoc(char *docname) {

	xmlDocPtr doc; /* pointer to XML document */
	xmlNodePtr cur; /* pointer to current node within document */

	doc = xmlParseFile(docname); /* attempt to parse xml file and get pointer to parsed document */
	
	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return false;
	}
	
	cur = xmlDocGetRootElement(doc);  /* start at the root of the XML document */
	
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return false;
	}
	
	/* check if root is of type "templates" */
	if (xmlStrcmp(cur->name, (const xmlChar *) "templates")) {
		fprintf(stderr,"document of the wrong type, root node != templates\n");
		xmlFreeDoc(doc);
		return false;
	}
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {

		if ((!xmlStrcmp(cur->name, (const xmlChar *)"text"))){
			/* Ignore text between templates */
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"comment"))){
			/* Ignore comments */
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *)"template"))){
			/* parse template, and check if parse failed */
			if(!parseTemplate(doc, cur)){
				xmlFreeDoc(doc);
				return false;
			}
		} else {
			fprintf(stderr,"Warning: Unkown templates child: %s\n", cur->name);
			fprintf(stderr,"Continuing to parse templates file ...\n");
		}
		 
		cur = cur->next;
	}
	
	xmlFreeDoc(doc);
	return true;
}

int main(int argc, char **argv) {


	/* debug DataStructure
	struct template * t = newTemplate();
	t->id = 1337;
	t->name = "testTemplate";

	struct field * f = newField(t);
	f->type = "test";
	f->id = 1;
	f->name = "testField";

	struct att * a = newAttribute(f);
	a->name = "a";
	a->value = "hello";
	struct att * b = newAttribute(f);
	b->name = "b";
	b->value = "world";
	struct att * c = newAttribute(f);
	c->name = "c";
	c->value = "in structs!!";

	struct field * f2 = newField(t);
	f2->type = "test2";
	f2->id = 2;
	f2->name = "testField2";

	struct att * a2 = newAttribute(f2);
	a2->name = "a2";
	a2->value = "hello2";
	struct att * b2 = newAttribute(f2);
	b2->name = "b2";
	b2->value = "world2";
	
	
	printTemplate(t); */
	

	
	char *docname;
		
	if (argc <= 1) {
		printf("Usage: %s docname\n", argv[0]);
    return(1);
	}

	int sucess;
	docname = argv[1];
	sucess = parseDoc(docname);
	
	if(sucess){
		printTemplates();
	} else {
		fprintf(stderr,"Failed to parse %s\n", docname);
    return 0;
	}

	return (0);
}

int validField(const char * field){
	
	if(strcmp(field, "int32")==0){
		return true;
	} else if(strcmp (field, "string")==0){
		return true;
	} else if(strcmp (field, "byteVector")==0){
		return true;
	} else if(strcmp (field, "decimal")==0){
		return true;
	} else if(strcmp (field, "uInt32")==0){
		return true;
	} else if(strcmp (field, "int64")==0){
		return true;
	} else if(strcmp (field, "uInt64")==0){
		return true;
	} else {
		return false;
	}

}

int validOperator(const char * field, const char * type){

	return true;
}

int validAttribute(const char * field, const char * name){

	return true;
}




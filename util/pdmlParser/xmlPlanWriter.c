
#include "xmlPlanWriter.h"


/* Setup the xmlwriter to generate the plan file */
int initXMLWriter(char *output){

  int rc;
  xmlChar *tmp;

  /* Create a new XmlWriter for docname, with no compression. */
  writer = xmlNewTextWriterFilename(output, 0);
  if (writer == NULL) {
    fprintf(stderr, "Error creating the xml writer\n");
    return false;
  }

  rc = xmlTextWriterSetIndent(writer, 1);
  if(rc<0){
    fprintf(stderr, "Error cannot enable indentation\n");
    return false;
  }

    /* Start the document with the xml default for the version,
     * encoding ISO 8859-1 and the default for the standalone
     * declaration. */
  rc = xmlTextWriterStartDocument(writer, NULL, OUTPUT_ENCODING, NULL);
  if (rc < 0) {
    fprintf(stderr, "Error xmlTextWriterStartDocument\n");
    return false;
  }

  /* Add "Plan" as root element */
  rc = xmlTextWriterStartElement(writer, BAD_CAST "Plan");
  if (rc < 0) {
    fprintf(stderr, "Error making Plan element\n");
    return false;
  }

  return true;
}

/* Create a message element in the plan file
  set templateID = tid */
int writeMessage(const xmlChar* tid){
  
  int rc;

  /* Start an element named "Message" as child of Plan. */
  rc = xmlTextWriterStartElement(writer, BAD_CAST "Message");
  if (rc < 0) {
    fprintf(stderr, "Error making Message element\n");
    return false;
  }

  /* Add an attribute with name "templateID" and value tid to Message. */
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "templateID", BAD_CAST tid);
  if (rc < 0) {
    fprintf(stderr, "Error adding message attribute\n");
    return false;
  }

  return true;
}

/* Close a message element in the plan file */
int closeMessage(){
  
  int rc;

  /* Close Message element. */
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) {
    fprintf(stderr, "Error at closing message\n");
    return false;
  }

  return true;
}

/* Write a group element in the plan file */
int writeGroup(){
  
  int rc;

  /* Start an element named "Message" as child of Plan. */
  rc = xmlTextWriterStartElement(writer, BAD_CAST "group");
  if (rc < 0) {
    fprintf(stderr, "Error making group element\n");
    return false;
  }

  return true;
}

/* Close a group element in the plan file */
int closeGroup(){
  
  int rc;

  /* Close Message element. */
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) {
    fprintf(stderr, "Error closing group\n");
    return false;
  }

  return true;
}

/* Write a sequence element in the plan file */
int writeSequence(){
  
  int rc;

  /* Start an element named "Message" as child of Plan. */
  rc = xmlTextWriterStartElement(writer, BAD_CAST "sequence");
  if (rc < 0) {
    fprintf(stderr, "Error making sequence element\n");
    return false;
  }

  return true;
}

/* Close a sequence element in the plan file */
int closeSequence(){
  
  int rc;

  /* Close Message element. */
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) {
    fprintf(stderr, "Error closing sequence\n");
    return false;
  }

  return true;
}

/* Write a field to the plan file
  element will be named type with
  value attribute = value */ 
int writeField(const xmlChar* type, const xmlChar* value){

  int rc;

  /* Start an element named "Message" as child of Plan. */
  rc = xmlTextWriterStartElement(writer, BAD_CAST type);
  if (rc < 0) {
    fprintf(stderr, "Error writing field %s\n", type);
    return false;
  }

  /* Add an attribute with name "templateID" and value tid to Message. */
  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "value", BAD_CAST value);
  if (rc < 0) {
    fprintf(stderr, "Error writing field attribute for field %s\n", type);
    return false;
  }

  /* Close field element. */
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) {
    fprintf(stderr, "Error at closing field %s\n", type);
    return false;
  }

  return true;
}

/* Close the plan file and write it to the output file */
void closeAndWriteXMLOutput(){

  int rc;

  rc = xmlTextWriterEndDocument(writer);
  if (rc < 0) {
    printf("testXmlwriterFilename: Error at xmlTextWriterEndDocument\n");
    return;
  }

  xmlFreeTextWriter(writer);

}

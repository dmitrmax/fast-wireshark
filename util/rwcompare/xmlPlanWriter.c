
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "xmlPlanWriter.h"


/* Setup the xmlwriter to generate the plan file */
gboolean initXMLWriter(const char *output){

  int rc;

  /* Create a new XmlWriter for docname, with no compression. */
  writer = xmlNewTextWriterFilename(output, 0);
  if (writer == NULL) {
    BAILOUT(FALSE, "Error creating the xml writer.");
  }

  rc = xmlTextWriterSetIndent(writer, 1);
  if (rc<0) {
    BAILOUT(FALSE, "Error cannot enable indentation.");
  }

    /* Start the document with the xml default for the version,
     * encoding ISO 8859-1 and the default for the standalone
     * declaration. */
  rc = xmlTextWriterStartDocument(writer, NULL, OUTPUT_ENCODING, NULL);
  if (rc < 0) {
    BAILOUT(FALSE, "Error xmlTextWriterStartDocument.");
  }

  /* Add "Plan" as root element */
  rc = xmlTextWriterStartElement(writer, BAD_CAST "Plan");
  if (rc < 0) {
    BAILOUT(FALSE, "Error making Plan element\n");
  }

  return TRUE;
}

/*! \brief  Write a nested type.
 * \param value  NULL if this is an empty field.
 */
gboolean writeNestedType(const xmlChar* type, const xmlChar* value)
{
  int rc;

  rc = xmlTextWriterStartElement(writer, BAD_CAST type);
  if (rc < 0) {
    DBG1("Error creating %s.", (char*) type);
    return FALSE;
  }

  if (value) {
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "value", BAD_CAST value);
    if (rc < 0) {
      BAILOUT(FALSE, "Error adding value attribute.");
    }
  }

  return TRUE;
}

/*! \brief  Close a nested type (message, group, sequence).
 */
gboolean closeNestedType(xmlChar* type)
{
  int rc;

  /* Close Message element. */
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) {
    DBG1("Error closing %s.", (char*)type);
    return FALSE;
  }

  return TRUE;
}

/* Write a field to the plan file
  element will be named type with
  value attribute = value */ 
gboolean writeField(const xmlChar* type, const xmlChar* value){

  int rc;

  /* Start an element named "Message" as child of Plan. */
  rc = xmlTextWriterStartElement(writer, BAD_CAST type);
  if (rc < 0) {
    DBG1("Error writing field %s.", type);
    return FALSE;
  }

  if (value) {
    /* Add an attribute with name "templateID" and value tid to Message. */
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "value", BAD_CAST value);
    if (rc < 0) {
      DBG1("Error writing field attribute for field %s.", type);
      return FALSE;
    }
  }

  /* Close field element. */
  rc = xmlTextWriterEndElement(writer);
  if (rc < 0) {
    DBG1("Error at closing field %s.", type);
    return FALSE;
  }

  return TRUE;
}

/* Close the plan file and write it to the output file */
void closeAndWriteXMLOutput()
{

  int rc;

  rc = xmlTextWriterEndDocument(writer);
  if (rc < 0) {
    DBG1("Err, end document returned %d", rc);
  }

  if (writer) {
    xmlFreeTextWriter(writer);
  }
  writer = 0;
}


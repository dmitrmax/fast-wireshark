
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdmlParser.h"
#include "run-exe.h"

#include "rwcompare.h"

/*! \brief  Print usage and reason for failure.
 * \param arg  The argument that failed.
 * \param reason  A more detailed reason why that argument caused failure.
 * \return  A suitable non-zero exit code.
 */
int ArgParseBailOut(const char* arg, const char* reason)
{
  FILE* out;
  out = stderr;

  fputs ("Usage:\n", out);
  fputs ("  rwcompare [p <port>]\n", out);
  fputs ("            [tshark <TShark executable>]\n", out);
  fputs ("            [tmpl <template file>]\n", out);
  fputs ("            [pcap <pcap file>]\n", out);
  fputs ("            [expect <plan file>]\n", out);
  fputs ("            [pdml <pdml output file>]\n", out);
  fputs ("            [plan <plan output file>]\n", out);

  if (reason) {
    if (arg) {
      fprintf (out, "Error: arg(%s)  %s\n", arg, reason);
    }
    else {
      fprintf (out, "Error:  %s\n", reason);
    }
  }
  return EXIT_FAILURE;
}


/*! \brief  Run a regression test.
 */
int main (const int argc, const char* const* argv)
{
  const char* template_filename = 0;
  const char* pcap_filename = 0;
  const char* expect_filename = 0;
  char* pdml_filename = 0;
  char* plan_filename = 0;
  const char* tshark_exe = "tshark";
  int port = 5000;
  gboolean givenp_pdml = FALSE;
  gboolean givenp_plan = FALSE;
  gboolean goodp = TRUE;
  int argi;

  if (argc == 1) {
    return ArgParseBailOut(0,0);
  }

  /* Loop thru arguments to set internal data. */
  for (argi = 1; argi < argc; ++argi) {
    const char* arg = argv[argi];
    if (argc == argi+1) {
      return ArgParseBailOut(arg, "Trailing flag without a value.");
    }
    else if (!strcmp("p", arg)) {
      port = atoi (argv[++argi]);
    }
    else if (!strcmp("tshark", arg)) {
      tshark_exe = argv[++argi];
    }
    else if (!strcmp("tmpl", arg)) {
      template_filename = argv[++argi];
    }
    else if (!strcmp("pcap", arg)) {
      pcap_filename = argv[++argi];
    }
    else if (!strcmp("expect", arg)) {
      expect_filename = argv[++argi];
    }
    else if (!strcmp("pdml", arg)) {
      pdml_filename = g_strdup (argv[++argi]);
      givenp_pdml = TRUE;
    }
    else if (!strcmp("plan", arg)) {
      plan_filename = g_strdup (argv[++argi]);
      givenp_plan = TRUE;
    }
    else {
      return ArgParseBailOut(arg, "Unknown argument.");
    }
  }


  /* Run TShark. */
  if (goodp && template_filename && pcap_filename) {
    if (!givenp_pdml) {
      pdml_filename = g_strdup_printf ("%s-pdml.xml", pcap_filename);
    }
    goodp = run_tshark (tshark_exe,
                        pcap_filename,
                        template_filename,
                        port,
                        pdml_filename);
  }

  /* Generate a plan file. */
  if (goodp && pdml_filename) {
    if (!givenp_plan) {
      plan_filename = g_strdup_printf ("%s-plan.xml", pcap_filename);
    }
    
    goodp = generatePlanFromPDML(pdml_filename, plan_filename);
  }

  /* Compare the two plan files. */
  if (goodp && plan_filename && expect_filename) {
    xmlLineNumbersDefault(1);
    goodp = equiv_plan_files (plan_filename, expect_filename);
  }

  /* Remove intermediary files if not given. */
  if (pdml_filename && !givenp_pdml)  remove (pdml_filename);
  if (plan_filename && !givenp_plan)  remove (plan_filename);

  /* Free some heap-allocated strings. */
  if (pdml_filename)  g_free (pdml_filename);
  if (plan_filename)  g_free (plan_filename);

  if (!goodp) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


/*! \brief  Compares two plan files, only caring about important information.
 * \return  TRUE iff the files are equivalent with respect to FAST data.
 */
gboolean equiv_plan_files (const char* plan_filename,
                           const char* expect_filename)
{
  gboolean equivp;
  xmlDocPtr comput_doc;
  xmlDocPtr expect_doc;
  xmlNodePtr cnode = 0;
  xmlNodePtr enode = 0;

  comput_doc = xmlParseFile(plan_filename);
  expect_doc = xmlParseFile(expect_filename);

  if (comput_doc) {
    cnode = xmlDocGetRootElement(comput_doc);
  }
  if (expect_doc) {
    enode = xmlDocGetRootElement(expect_doc);
  }

  equivp = equiv_plan_xml (cnode, enode);

  if (comput_doc)  xmlFreeDoc(comput_doc);
  if (expect_doc)  xmlFreeDoc(expect_doc);

  if (equivp) {
    fprintf (stderr, "Plan matches expected.\n");
  }
  else {
    fprintf (stderr, "Plan does not match expected.\n");
  }
  return equivp;
}


/*! \brief  Compare two XML trees representing Plan Files.
 *
 * Also print some info about where the difference is.
 *
 * \return  TRUE iff equivalent.
 */
gboolean equiv_plan_xml (xmlNodePtr cnode,
                         xmlNodePtr enode)
{
  gboolean equivp = TRUE;
  cnode = next_xml_node (cnode);
  enode = next_xml_node (enode);
  while (cnode && enode) {
    xmlChar* cprop;
    xmlChar* eprop;

    /* Assure field names are the same. */
    equivp = !xmlStrcasecmp(cnode->name, enode->name);
    if (!equivp) {
      fprintf (stderr,
               "Field mismatch on line %ld (plan) and line %ld (expect).\n",
               xmlGetLineNo (cnode),
               xmlGetLineNo (enode));
      return FALSE;
    }

    /* Assure equivalent values. */
    cprop = xmlGetProp (cnode, (xmlChar*)"value");
    eprop = xmlGetProp (enode, (xmlChar*)"value");

    if (cprop || eprop) {
      if (cprop && eprop) {
        equivp = !xmlStrcmp (cprop, eprop);
      }
      if (cprop)  xmlFree (cprop);
      if (eprop)  xmlFree (eprop);
      /* Bail out if no good. */
      if (!equivp) {
        fprintf (stderr,
                 "Value mismatch on line %ld (plan) and line %ld (expect).\n",
                 xmlGetLineNo (cnode),
                 xmlGetLineNo (enode));
        return FALSE;
      }
    }

    /* Assure equivalent children. */
    equivp = equiv_plan_xml (cnode->xmlChildrenNode, enode->xmlChildrenNode);
    if (!equivp)  return FALSE;

    cnode = next_xml_node (cnode->next);
    enode = next_xml_node (enode->next);
  }

  /* Neither must have any more valid nodes. */
  if (cnode) {
    fprintf (stderr, "More plan nodes (line %ld) than expected.\n",
             xmlGetLineNo (cnode));
    return FALSE;
  }
  if (enode) {
    fprintf (stderr, "Fewer plan nodes than expected (line %ld).\n",
             xmlGetLineNo (enode));
    return FALSE;
  }
  return TRUE;
}


/*! \brief  Skip to the next interesting node (including te current one).
 * \return  Next relevant XML node or NULL if none exists.
 */
xmlNodePtr next_xml_node (xmlNodePtr node)
{
  for (;  node;  node = node->next) {
    if (xmlStrcasecmp(node->name, (xmlChar*) "text") &&
        xmlStrcasecmp(node->name, (xmlChar*) "comment")) {
      return node;
    }
  }
  return 0;
}


/*! \brief  Check if this XML node should be ignored.
 *
 * Ignored nodes include comments and text between XML tags.
 *
 * \param xmlnode  Current XML node.
 * \return  TRUE iff the node is something we ignore.
 */
gboolean ignore_xml_node (xmlNodePtr xmlnode)
{
  return (0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "text") ||
          0 == xmlStrcasecmp(xmlnode->name, (xmlChar*) "comment"));
}


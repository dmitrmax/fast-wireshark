
#include "rwcompare.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  fputs ("  rwcompare plans <plan file> <plan file>\n", out);

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
  int port = 1337;
  gboolean givenp_pdml = FALSE;
  gboolean givenp_plan = FALSE;
  gboolean goodp = TRUE;
  int argi;

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
    else if (argi == 1 && !strcmp("plans", arg)) {
      if (3+ argi != argc) {
        return ArgParseBailOut(arg, "Require exactly two plan files.");
      }
      plan_filename = g_strdup (argv[++argi]);
      expect_filename = g_strdup (argv[++argi]);
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
    /* TODO */
    goodp = FALSE;
  }

  /* Compare the two plan files. */
  if (goodp && plan_filename && expect_filename) {
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


/*! \brief  Do nothing. */
static void do_nothing (gpointer user_data) { (void) user_data; }


/*! \brief  Run a program.
 * \param argc  Number of arguments (including executable name).
 * \param argv  Hold executable name and arguments.
 * \param output  Return value. Will hold program's standard output.
 *                If NULL, stdout is piped to /dev/null.
 * \return  TRUE iff everything went well.
 */
gboolean run_gather (guint argc, const char* const* argv, char** output)
{
  char** argv_mutable;
  guint i;
  gboolean successp = TRUE;
  GSpawnFlags spawn_flags = G_SPAWN_SEARCH_PATH | G_SPAWN_STDERR_TO_DEV_NULL;

  if (!output) {
    spawn_flags |= G_SPAWN_STDOUT_TO_DEV_NULL;
  }

  /* Create a mutable copy of argv. */
  argv_mutable = g_malloc ((1+ argc) * sizeof (char*));
  for (i = 0; i < argc; ++i) {
    argv_mutable[i] = g_strdup (argv[i]);
  }
  argv_mutable[argc] = 0;

  successp = g_spawn_sync (0, /* Inherit working directory. */
                           argv_mutable,
                           0, /* Inherit environment. */
                           spawn_flags,
                           &do_nothing,
                           0, /* No data passed to do_nothing() */
                           output,
                           0, /* Standard error is going to /dev/null. */
                           0, /* Don't care about error codes. */
                           0); /* Don't care about the error. */

  /* Free the mutable copy of argv. */
  for (i = 0; i < argc; ++i) {
    g_free (argv_mutable[i]);
  }
  g_free (argv_mutable);

  return successp;
}


/*! \brief  Run TShark.
 * \param tshark_exe  Name of the TShark executable with optional path.
 * \param pcap_filename  Captured traffic for TShark to read.
 * \param template_filename  Template file used to generate the traffic.
 * \param part  Port number used the plugin.
 * \param output_filename  File to write PDML output.
 * \return  TRUE iff everything went well.
 */
gboolean run_tshark (const char* tshark_exe,
                     const char* pcap_filename,
                     const char* template_filename,
                     int port,
                     const char* output_filename)
{
  const char* argv[10];
  const char* proto_abbr = "fast";
  char* template_option;
  char* port_option;
  gboolean successp;
  char* output = 0;

  template_option =
    g_strdup_printf ("%s.template:%s", proto_abbr, template_filename);
  port_option =
    g_strdup_printf ("%s.port:%d",     proto_abbr, port);

  {
    unsigned argi = 0;
    argv[argi++] = tshark_exe;
    argv[argi++] = "-o";
    argv[argi++] = template_option;
    argv[argi++] = "-o";
    argv[argi++] = port_option;
    argv[argi++] = "-T";
    argv[argi++] = "pdml";
    argv[argi++] = "-r";
    argv[argi++] = pcap_filename;

    successp = run_gather (argi, argv, &output);
  }

  g_free (template_option);
  g_free (port_option);

  /* Write the TShark output to a file. */
  if (successp)
  {
    FILE* outf;
    outf = fopen (output_filename, "w+");
    if (outf) {
      fputs (output, outf);
      fclose (outf);
    }
    else {
      successp = FALSE;
    }
  }

  return successp;
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

  return equivp;
}


/*! \brief  Compare two XML trees representing Plan Files.
 * \return  TRUE iff equivalent.
 */
gboolean equiv_plan_xml (xmlNodePtr cnode,
                         xmlNodePtr enode)
{
  cnode = next_xml_node (cnode);
  enode = next_xml_node (enode);
  while (cnode && enode) {
    gboolean equivp = TRUE;
    xmlChar* cprop;
    xmlChar* eprop;

    /* Assure field names are the same. */
    equivp = !xmlStrcasecmp(cnode->name, enode->name);
    if (!equivp)  return FALSE;

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
      if (!equivp)  return FALSE;
    }

    /* Assure equivalent children. */
    equivp = equiv_plan_xml (cnode->xmlChildrenNode, enode->xmlChildrenNode);
    if (!equivp)  return FALSE;

    cnode = next_xml_node (cnode->next);
    enode = next_xml_node (enode->next);
  }

  /* Neither must have any more valid nodes. */
  if (cnode || enode) {
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


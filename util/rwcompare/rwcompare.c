
#include "rwcompare.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libxml/parser.h>

int ArgParseBailOut(const char* arg, const char* reason)
{
  fprintf (stderr, "Usage:\n");
  fprintf (stderr, "  rwcompare [-p <port>] [--tshark <TShark executable>]\n");
  fprintf (stderr, "            <templates file> <plan file> <pcap file>\n");
  if (reason) {
    if (arg) {
      fprintf (stderr, "Error: arg(%s)  %s\n", arg, reason);
    }
    else {
      fprintf (stderr, "Error:  %s\n", reason);
    }
  }
  return EXIT_FAILURE;
}

int main (const int argc, const char* const* argv)
{
  const char* templates_filename = 0;
  const char* plan_filename = 0;
  const char* pcap_filename = 0;
  const char* tshark_exe = "tshark";
  int port = 1337;
  int argi;
  gboolean havep_more_flags = TRUE;

  for (argi = 1; argi < argc; ++argi) {
    const char* arg = argv[argi];
    if (arg[0] != '-' || !havep_more_flags) {
      if (!templates_filename)  templates_filename = argv[argi];
      else if (!plan_filename)  plan_filename      = argv[argi];
      else if (!pcap_filename)  pcap_filename      = argv[argi];
      else return ArgParseBailOut(arg, "Too many filenames.");
    }
    else if (!strcmp("--", arg)) {
      havep_more_flags = FALSE;
    }
    else if (argc == argi+1) {
      return ArgParseBailOut(arg, "Trailing flag without a value.");
    }
    else if (!strcmp("-p", arg)) {
      port = atoi (argv[++argi]);
    }
    else if (!strcmp("--tshark", arg)) {
      tshark_exe = argv[++argi];
    }
    else {
      return ArgParseBailOut(arg, "Unknown argument.");
    }
  }

  if (!(templates_filename && plan_filename && pcap_filename)) {
    return ArgParseBailOut(0, "Need more filenames.");
  }

  /* xmlDocPtr doc; */
  /* xmlNodePtr node; */

  /* printf ("%s\n", filename); */

  /* doc = xmlParseFile(filename); */

  return EXIT_SUCCESS;
}



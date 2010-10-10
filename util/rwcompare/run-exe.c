
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "run-exe.h"

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



#ifndef RUN_EXE_H_
#define RUN_EXE_H_

#include <glib.h>

gboolean run_gather (guint argc, const char* const* argv, char** output);
gboolean run_tshark (const char* tshark_exe,
                     const char* pcap_filename,
                     const char* template_filename,
                     int port,
                     const char* output_filename);

#endif


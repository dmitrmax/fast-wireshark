
#ifndef RWCOMPARE_H_
#define RWCOMPARE_H_

#include <glib.h>

int ArgParseBailOut(const char* arg, const char* reason);
gboolean run_gather (guint argc, const char* const* argv, char** output);
gboolean run_tshark (const char* tshark_exe,
                     const char* pcap_filename,
                     const char* template_filename,
                     int port,
                     const char* output_filename);

#endif


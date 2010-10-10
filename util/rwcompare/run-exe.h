
#ifndef RUN_EXE_H_
#define RUN_EXE_H_

#include <glib.h>

gboolean run_gather (guint argc, const char* const* argv, char** output_ptr);
gboolean run_tshark (const char* tshark_exe,
                     const char* pcap_filename,
                     const char* template_filename,
                     int port,
                     const char* output_filename,
                     unsigned duration);
GThread* spawn_tshark (const char* tshark_exe,
                       const char* pcap_filename,
                       const char* template_filename,
                       int port,
                       const char* output_filename,
                       unsigned duration);
gboolean run_plan (const char* plan_runner_jar,
                   const char* template_filename,
                   const char* expect_filename,
                   int port);

#endif


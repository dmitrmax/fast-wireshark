
#ifndef RWCOMPARE_H_
#define RWCOMPARE_H_

#include <glib.h>
#include <libxml/parser.h>

int ArgParseBailOut(const char* arg, const char* reason);
gboolean run_gather (guint argc, const char* const* argv, char** output);
gboolean run_tshark (const char* tshark_exe,
                     const char* pcap_filename,
                     const char* template_filename,
                     int port,
                     const char* output_filename);
gboolean equiv_plan_files (const char* plan_filename,
                           const char* expect_filename);
gboolean equiv_plan_xml (xmlNodePtr cnode,
                         xmlNodePtr enode);
xmlNodePtr next_xml_node (xmlNodePtr node);
gboolean ignore_xml_node (xmlNodePtr xmlnode);

#endif


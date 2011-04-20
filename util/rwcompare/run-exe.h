/*
 * This file is part of FAST Wireshark.
 *
 * FAST Wireshark is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * FAST Wireshark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 * 
 * You should have received a copy of the Lesser GNU General Public License
 * along with FAST Wireshark.  If not, see 
 * <http://www.gnu.org/licenses/lgpl.txt>.
 */
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
                   const char* pcap_filename,
                   int port);

#endif


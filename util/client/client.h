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
#ifndef CLIENT_H_
#define CLIENT_H_

#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <getopt.h>

#include "encode.h"

typedef struct fast_message_struct
{
    char* host;
    char* service;
    struct addrinfo crit;

    GByteArray* pmap;
    gboolean tidp;
    guint32 tid;
    GByteArray* msg;

    GByteArray* encoded;
}   fast_message_type,
    * fast_message_t;

void encode_fast (fast_message_t pto_fmsg);
void send_fast (fast_message_t fmsg);
void add_pmap (fast_message_t fmsg, guint8 b);
int prog_getopt ();

void show_usage ();

#ifdef CLIENT_C_
enum optkey
{  
    optkey_first = 0,
    optkey_help,
    optkey_host, optkey_port,
    optkey_gen_c,
    optkey_tid, optkey_notid,
    optkey_req, optkey_noreq,
    optkey_uint32, optkey_int32,
    optkey_uint64, optkey_int64,
    optkey_ascii,
    optkey_bytevec,
    optkey_nop,
    optkey_hex, optkey_bit
};
static const struct option long_options[] =
{    {"help"  , 0, 0, 0 }
    ,{"host"  , 1, 0, 0 }
    ,{"port"  , 1, 0, 0 }
    ,{"gen-c" , 0, 0, 0 }
    ,{"tid"   , 1, 0, 0 }
    ,{"notid" , 0, 0, 0 }
    ,{"req"   , 0, 0, 0 }
    ,{"noreq" , 0, 0, 0 }
    ,{"uint32", 1, 0, 0 }
    ,{"int32" , 1, 0, 0 }
    ,{"uint64", 1, 0, 0 }
    ,{"int64" , 1, 0, 0 }
    ,{"ascii" , 1, 0, 0 }
    ,{"bytevec",1, 0, 0 }
    ,{"nop"   , 0, 0, 0 }
    ,{"hex"   , 1, 0, 0 }
    ,{"bit"   , 1, 0, 0 }
    ,{0,0,0,0}
};

int prog_getopt (int argc, char* const argv[])
{
    int indOpt;
    int opt = getopt_long (argc, argv, "h:p:",
                           long_options, &indOpt);

    if (0 > opt)  return 0;
    switch (opt)
    {
        case  0 : return indOpt + optkey_first +1;
        case 'h': return optkey_host;
        case 'p': return optkey_port;
    }
    assert (0);
    return 0;
}

void show_usage ()
{
    fputs ("\
Usage: ./client [flags]\n\n\
 Options\n\
    --help      Show this message\n\
    -p <port>     Connect to port /port/, default is 1337\n\
    -h <host>     Connect to /host/, default is localhost\n\
", stderr);
    fputs ("\
    --tid <n>     Set template id to /n/, default is 1\n\
    --notid     No template id for this message\n\
", stderr);
    fputs ("\n\
 Fields\n\
    --req | --noreq   Following fields are mandatory (default) or not\n\
                      i.e. they won't appear in the presence map\n\
    --uint32 <n>       Encode a unsigned 32-bit integer /n/\n\
    --int32 <n>       Encode a signed 32-bit integer /n/\n\
    (--uint64 | --int64) n\n\
    --ascii <str>    Encode /str/ as an ascii string\n\
    --bytevec <hexstring>\n\
    --nop        Put a zero in the presence map (even if --req specified)\n\
", stderr);
    fputs ("\n\
 Faux Fields\n\
    --hex <hexstring>  Directly encode a field, specified with a hex string\n\
                       (length is a multiple of 2, spaces are ignored)\n\
    --bit <bitstring>   Directly encode a field, specified by a bit string\n\
                       (length is a multiple of 8, spaces are ignored)\n\
", stderr);
}
#endif

#endif



#ifndef CLIENT_H_
#define CLIENT_H_

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

void show_usage ();

#endif


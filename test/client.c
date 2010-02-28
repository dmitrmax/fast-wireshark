
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <getopt.h>

#include <glib.h>

void encode_uint32 (guint32 x, int* offset, guint8* buf)
{
    size_t maxc = 9;
    int i = maxc;

    buf += *offset;

    do
    {
        --i;
        buf[i] = x & 0x7f;
        x >>= 7;
    } while (0 != x);

    buf[maxc -1] |= 0x80;

    memmove (buf, &buf[i], maxc - i);
    *offset += (maxc - i);
}

void encode_int32 (gint32 x, int* offset, guint8* buf)
{
    size_t maxc = 9;
    int i = maxc;

    buf += *offset;

    while (1)
    {
        --i;
        buf[i] = x & 0x7f;
        x >>= 6;
        if (x == 0 || ~x == 0)  break;
        x >>= 1;
    }

    buf[maxc -1] |= 0x80;

    memmove (buf, &buf[i], maxc - i);
    *offset += (maxc - i);
}

void send_bytes (int sock, size_t init_len, guint8* init_msg)
{
    static guint8*  msg = 0;
    static size_t len = 0;

    ssize_t bytecOut;

        /* Sneaky variable initialization */
    if (! sock)
    {
        len = init_len;
        msg = init_msg;
        return;
    }

    fprintf (stderr, "Woot connected to a socket!\n");

    bytecOut = send (sock, msg, len, 0);
    if (0 > bytecOut)
    {
        perror ("send() failed");
    }
}

void
    spawn_senders
(const struct addrinfo* crit,
 const char* host,
 const char* service,
 void (* sender_fn) (int))
{
    struct addrinfo* list;
    struct addrinfo* addr;

    {
        int rtn;
        rtn = getaddrinfo (host, service, crit, &list);
        if (rtn != 0)
        {
            fprintf (stderr, "getaddrinfo() failed: %s\n",
                     gai_strerror (rtn));
            return;
        }
    }

    for (addr = list; addr; addr = addr->ai_next)
    {
        int sock;
        sock = socket (addr->ai_family,
                       addr->ai_socktype,
                       addr->ai_protocol);
        if (0 > sock)
        {
            perror ("socket() failed");
            continue;
        }

        if (0 > connect (sock, addr->ai_addr, addr->ai_addrlen))
        {
            perror ("connect() failed");
            close (sock);
            continue;
        }

        sender_fn (sock);
        close (sock);
    }

    freeaddrinfo (list);
}

void show_usage ()
{
    fputs ("\
Usage: ./client [options]\n\
    -p port\n\
    -h host\n\
    --help\n\
\n\
 Fields\n\
    --uint32 n\n\
    --int32 n\n\
", stderr);
}


int main (int argc, char** argv)
{

    guint8 msg[BUFSIZ];
    char* host = "localhost";
    char* service = "1337";
    struct addrinfo crit;
    int off = 0;

    if (argc == 1)
    {
        show_usage ();
        exit (1);
    }

    encode_uint32 (0x40, &off, msg);
    encode_uint32 (1, &off, msg);

    while (1)
    {
        enum { optkey_first = 256, optkey_help, optkey_uint32, optkey_int32 };
        const struct option long_options[] =
        {    {"help"  , 0, 0, 0 }
            ,{"uint32", 1, 0, 0 }
            ,{"int32" , 1, 0, 0 }
            ,{0,0,0,0}
        };
        int indOpt;
        int opt = getopt_long
            (argc, argv, "p:h:",
             long_options, &indOpt);

        if (0 > opt)  break;
        if (0 == opt)  opt = indOpt + optkey_first +1;

        switch (opt)
        {
            case 'h' :
                host = optarg;
                break;
            case 'p' :
                service = optarg;
                break;
            case optkey_help :
                show_usage ();
                exit (0);
                break;
            case optkey_uint32 :
                encode_uint32 ((guint32) g_ascii_strtoull (optarg, 0, 10),
                               &off, msg);
                break;
            case optkey_int32 :
                encode_int32 ((gint32) g_ascii_strtoll (optarg, 0, 10),
                              &off, msg);
                break;
        }
    }

    memset (&crit, 0, sizeof (struct addrinfo));

        /* crit.  ai_family = AF_INET6; */
    crit.  ai_family = AF_UNSPEC;
    crit.ai_socktype = SOCK_DGRAM;
    crit.ai_protocol = IPPROTO_UDP;

        /* Initialize message */
    send_bytes (0, off, msg);

    spawn_senders (&crit, host, service,
                   (void (*) (int)) send_bytes);

    exit (0);
}



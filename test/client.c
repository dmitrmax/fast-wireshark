
#include "client.h"

    /* Encode and destroy */
void encode_fast (fast_message_t fmsg)
{
        /* Encode the message */
    fmsg->encoded = g_byte_array_new ();

    encode_pmap (fmsg->pmap, &fmsg->encoded);
    g_byte_array_free (fmsg->pmap, 1);

    encode_uint32 (fmsg->tid, fmsg->encoded);

    fmsg->encoded =
        g_byte_array_append (fmsg->encoded,
                             fmsg->msg->data,
                             fmsg->msg->len);
    g_byte_array_free (fmsg->msg, 1);
}

    /* Send and destroy encoded fast message */
void send_fast (fast_message_t fmsg)
{
    struct addrinfo* list;
    struct addrinfo* addr;

    {
        int rtn;
        rtn = getaddrinfo (fmsg->host, fmsg->service, &fmsg->crit, &list);
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
        ssize_t bytecOut;
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

        fprintf (stderr, "Woot connected to a socket!\n");

        bytecOut = send (sock,
                         fmsg->encoded->data,
                         fmsg->encoded->len,
                         0);
        if (0 > bytecOut)
        {
            perror ("send() failed");
        }
        close (sock);
    }

    freeaddrinfo (list);
    g_byte_array_free (fmsg->encoded, 1);
}

void add_pmap (fast_message_t fmsg, guint8 b)
{
    fmsg->pmap = g_byte_array_append (fmsg->pmap, &b, 1);
}

void show_usage ()
{
    fputs ("\
Usage: ./client [options]\n\
    -p port\n\
    --tid n\
    Set template id to /n/, default is 1\n\
    -h host\n\
    --help\n\
\n\
 Fields\n\
    --help\
    See this message\n\
    --uint32 n\
    Encode a unsigned 32-bit integer /n/\n\
    --int32 n\
    Encode a signed 32-bit integer /n/\n\
    --nop\
    Put a zero in the presence map\n\
", stderr);
}

int main (int argc, char** argv)
{
    fast_message_type fmsg;

        /* Die fast if no arguments */
    if (argc == 1)
    {
        show_usage ();
        exit (1);
    }

    fmsg.host = "localhost";
    fmsg.service = "1337";
    fmsg.pmap = g_byte_array_new ();
    fmsg.tid = 1;
    fmsg.msg = g_byte_array_new ();

        /* Set default socket values */
    memset (&fmsg.crit, 0, sizeof (struct addrinfo));
        /* fmsg->crit.  ai_family = AF_INET6; */
    fmsg.crit.  ai_family = AF_UNSPEC;
    fmsg.crit.ai_socktype = SOCK_DGRAM;
    fmsg.crit.ai_protocol = IPPROTO_UDP;

    while (1)
    {
        enum
        {   optkey_first = 256,
            optkey_help, optkey_tid,
            optkey_uint32, optkey_int32, optkey_nop
        };
        const struct option long_options[] =
        {    {"help"  , 0, 0, 0 }
            ,{"tid"   , 1, 0, 0 }
            ,{"uint32", 1, 0, 0 }
            ,{"int32" , 1, 0, 0 }
            ,{"nop"   , 0, 0, 0 }
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
                fmsg.host = optarg;
                break;
            case 'p' :
                fmsg.service = optarg;
                break;
            case optkey_help :
                show_usage ();
                exit (0);
                break;
            case optkey_tid :
                fmsg.tid = (guint32) g_ascii_strtoull (optarg, 0, 10);
                break;
            case optkey_uint32 :
                add_pmap (&fmsg, 1);
                encode_uint32 ((guint32) g_ascii_strtoull (optarg, 0, 10),
                               fmsg.msg);
                break;
            case optkey_int32 :
                add_pmap (&fmsg, 1);
                encode_int32 ((gint32) g_ascii_strtoll (optarg, 0, 10),
                              fmsg.msg);
                break;
            case optkey_nop :
                add_pmap (&fmsg, 0);
                break;
        }
    }

    encode_fast (&fmsg);
    send_fast (&fmsg);

    exit (0);
}



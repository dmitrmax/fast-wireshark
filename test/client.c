
#define CLIENT_C_
#include "client.h"
#undef CLIENT_C_

    /* Encode and destroy */
void encode_fast (fast_message_t fmsg)
{
        /* Set tidp's placeholder */
    fmsg->pmap->data[0] = fmsg->tidp;

        /* Encode the message */
    fmsg->encoded = g_byte_array_new ();

    encode_pmap (fmsg->pmap, &fmsg->encoded);
    g_byte_array_free (fmsg->pmap, 1);

    if (fmsg->tidp)
        encode_uint32 (fmsg->tid, &fmsg->encoded);

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

int main (int argc, char** argv)
{
    gboolean requiredp = 1;
    gboolean genp_c = 0; /* True if generating setup.c */
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
    fmsg.tidp = 1;
    fmsg.tid = 1;
    fmsg.msg = g_byte_array_new ();

    add_pmap (&fmsg, fmsg.tidp);

        /* Set default socket values */
    memset (&fmsg.crit, 0, sizeof (struct addrinfo));
        /* fmsg->crit.  ai_family = AF_INET6; */
    fmsg.crit.  ai_family = AF_UNSPEC;
    fmsg.crit.ai_socktype = SOCK_DGRAM;
    fmsg.crit.ai_protocol = IPPROTO_UDP;

    while (1)
    {
        int opt = prog_getopt (argc, argv);
        if (!opt) break;

        switch (opt)
        {
            case optkey_help :
                show_usage ();
                exit (0);
                break;
            case optkey_host :
                fmsg.host = optarg;
                break;
            case optkey_port :
                fmsg.service = optarg;
                break;
            case optkey_gen_c :
                genp_c = 1;
                break;

            case optkey_tid :
                fmsg.tid = (guint32) g_ascii_strtoull (optarg, 0, 10);
                break;
            case optkey_notid :
                fmsg.tidp = 0;
                break;

            case optkey_req :
                requiredp = 1;
                break;
            case optkey_noreq :
                requiredp = 0;
                break;

            case optkey_uint32 :
                if (! requiredp)  add_pmap (&fmsg, 1);
                encode_uint32 ((guint32) g_ascii_strtoull (optarg, 0, 10),
                               &fmsg.msg);
                break;
            case optkey_int32 :
                if (! requiredp)  add_pmap (&fmsg, 1);
                encode_int32 ((gint32) g_ascii_strtoll (optarg, 0, 10),
                              &fmsg.msg);
                break;
            case optkey_uint64 :
                if (! requiredp)  add_pmap (&fmsg, 1);
                encode_uint64 ((guint64) g_ascii_strtoull (optarg, 0, 10),
                               &fmsg.msg);
                break;
            case optkey_int64 :
                if (! requiredp)  add_pmap (&fmsg, 1);
                encode_uint64 ((gint64) g_ascii_strtoll (optarg, 0, 10),
                               &fmsg.msg);
                break;
            case optkey_ascii :
                if (! requiredp)  add_pmap (&fmsg, 1);
                encode_ascii ((guint8*) optarg, &fmsg.msg);
                break;
            case optkey_bytevec :
                if (! requiredp)  add_pmap (&fmsg, 1);
                encode_bytevec ((guint8*) optarg, &fmsg.msg);
                break;

            case optkey_nop :
                add_pmap (&fmsg, 0);
                break;
            case optkey_hex :
                if (! requiredp)  add_pmap (&fmsg, 1);
                encode_hex ((guint8*) optarg, &fmsg.msg);
                break;
            case optkey_bit :
                if (! requiredp)  add_pmap (&fmsg, 1);
                encode_bit ((guint8*) optarg, &fmsg.msg);
                break;
        }
    }

    encode_fast (&fmsg);
    send_fast (&fmsg);

    exit (0);
}



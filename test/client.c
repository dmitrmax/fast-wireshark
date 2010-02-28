
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
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

int main (int argc, char** argv)
{
    guint8 msg[BUFSIZ];
    char* host;
    char* service;
    struct addrinfo crit;

    if (1 >= argc)
    {
        fputs ("./client port [host]\n", stderr);
        exit (EXIT_FAILURE);
    }
    service = argv[1];

    if (3 <= argc)  host = argv[2];
    else            host = "localhost";

    memset (&crit, 0, sizeof (struct addrinfo));

        /* crit.  ai_family = AF_INET6; */
    crit.  ai_family = AF_UNSPEC;
    crit.ai_socktype = SOCK_DGRAM;
    crit.ai_protocol = IPPROTO_UDP;

    {
        int off = 0;
        encode_uint32 (4, &off, msg);
        encode_uint32 (1, &off, msg);
        encode_uint32 (3, &off, msg);
        encode_uint32 (2, &off, msg);

            /* Initialize message */
        send_bytes (0, off, msg);
    }

    spawn_senders (&crit, host, service,
                   (void (*) (int)) send_bytes);

    exit (0);
}




#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <glib.h>

void send_string (int sock, char* init_strOut)
{
    static char*  strOut = 0;
    static size_t lenOut = 0;

    ssize_t bytecOut;

        /* Sneaky variable initialization */
    if (! sock)
    {
        strOut = init_strOut;
        lenOut = strlen (strOut);
        return;
    }

    fprintf (stderr, "Woot connected to a socket!\n");


    bytecOut = send (sock, strOut, lenOut, 0);
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
    char* message;
    char* host;
    char* service;
    struct addrinfo crit;

    if (3 > argc)
    {
        fputs ("./client message port [host]\n", stderr);
        exit (EXIT_FAILURE);
    }
    message = argv[1];
    service = argv[2];

    if (4 <= argc)  host = argv[3];
    else            host = "localhost";

    memset (&crit, 0, sizeof (struct addrinfo));

        /* crit.  ai_family = AF_INET6; */
    crit.  ai_family = AF_UNSPEC;
    crit.ai_socktype = SOCK_DGRAM;
    crit.ai_protocol = IPPROTO_UDP;

        /* Initialize message */
        /* send_string (0, message); */

    {
        guint8 msg[3];
        msg[0] = 0x80 | 0x1;
        msg[1] = 0x80 | 0x1;
        msg[2] = 0x80 | 0x3;
        msg[3] = 0x80 | 0x2;
        msg[4] = 0;

            /* Initialize message */
        send_string (0, (char*)msg);
    }

    spawn_senders (&crit, host, service,
                   (void (*) (int)) send_string);

    exit (0);
}



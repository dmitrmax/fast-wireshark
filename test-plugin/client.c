
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int udp_connect (const char* host,
                 const char* service)
{
    struct addrinfo crit;

    memset (&crit, 0, sizeof (struct addrinfo));

    crit.  ai_family = AF_UNSPEC;
    crit.ai_socktype = SOCK_DGRAM;
    crit.ai_protocol = IPPROTO_UDP;

    {
        int rtn;
        struct addrinfo* addr;
        struct addrinfo* list = 0;
        int sock = -1;

        rtn = getaddrinfo (host, service, &crit, &list);

        if (rtn)
        {
            perror ("No addr info");
            exit (EXIT_FAILURE);
        }

        for (addr = list; addr; addr = addr->ai_next)
        {
            sock = socket (addr->ai_family,
                           addr->ai_socktype,
                           addr->ai_protocol);
            if (sock < 0)
                continue;

            if (0 == connect (sock, addr->ai_addr, addr->ai_addrlen))
                break; /* connection succeeded */


            close (sock); /* connection failed, try next >:( */
            sock = -1;
        }


        freeaddrinfo (list);
        return sock;
    }
}

int main (int argc, char** argv)
{
    char* strOut;
    int sock;

    if (argc != 4)
    {
        fputs ("Usage:  client MESSAGE SERVER PORT\n", stderr);
        exit (EXIT_FAILURE);
    }

    strOut = argv[1];
    sock = udp_connect (argv[2], argv[3]);

    if (sock < 0)
    {
        perror ("Socket open failed");
        exit (EXIT_FAILURE);
    }

    {
        ssize_t bytecIn = 0;
        ssize_t bytecOut;
        size_t lenOut = strlen (strOut);

        printf ("Sending: %s\n", strOut);
        bytecOut = send (sock, strOut, lenOut, 0);
        assert (bytecOut >= 0);

        {
            char bufIn[BUFSIZ];
            bytecIn = recv (sock, bufIn, BUFSIZ - 1, 0);

            if (bytecIn < 0)
            {
                perror ("recv() failed");
                exit (EXIT_FAILURE);
            }

            bufIn[bytecIn] = '\0';
            printf ("Received: %s\n", bufIn);
        }

    }
    exit (EXIT_SUCCESS);
}



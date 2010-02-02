
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main (int argc, char** argv)
{
    char* service;
    struct addrinfo crit;
    struct addrinfo* serv;
    int rtn;
    int sock;

    if (argc != 3)
    {
        fputs ("usage: server MESSAGE PORT\n", stderr);
        exit (EXIT_FAILURE);
    }

    service = argv[2];

    memset (&crit, 0, sizeof (struct addrinfo));
    crit.  ai_family = AF_UNSPEC;
    crit.   ai_flags = AI_PASSIVE;
    crit.ai_socktype = SOCK_DGRAM;
    crit.ai_protocol = IPPROTO_UDP;


    rtn = getaddrinfo (NULL, service, &crit, &serv);
    if (rtn != 0)
    {
        perror ("getaddrinfo() failed");
        exit (EXIT_FAILURE);
    }

    sock = socket (serv->ai_family,
                   serv->ai_socktype,
                   serv->ai_protocol);

    if (sock < 0)
    {
        perror ("socket() failed");
        exit (EXIT_FAILURE);
    }

    if (0 > bind (sock, serv->ai_addr, serv->ai_addrlen))
    {
        perror ("bind() failed");
        exit (EXIT_FAILURE);
    }

    freeaddrinfo (serv);

    {
        struct sockaddr_storage client;
        socklen_t sizeClient = sizeof (struct sockaddr_storage);
        char buf[BUFSIZ];
        ssize_t bytecOut;
        ssize_t bytecIn;

        bytecIn = recvfrom (sock, buf, BUFSIZ-1, 0,
                             (struct sockaddr*) &client, &sizeClient);
        if (bytecIn < 0)
        {
            perror ("recvfrom() failed");
            exit (EXIT_FAILURE);
        }
        buf[bytecIn] = 0;
        printf ("Received: %s\nSending: %s\n",
                buf, argv[1]);

        bytecOut = sendto (sock, argv[1], strlen (argv[1]), 0,
                           (struct sockaddr*) &client,
                           sizeClient);

        if (bytecOut < 0)
        {
            perror ("sendto() failed");
            exit (EXIT_FAILURE);
        }
    }

    exit (EXIT_SUCCESS);
}



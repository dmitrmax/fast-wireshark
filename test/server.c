
#include "server.h"

void chain_func (int s, pid_t p)
{
    static int   sock = 0;
    static pid_t  pid = 0;

    if (s)
    {
        sock = s;
        pid  = p;
    }
    else
    {
            /* Clean up */
        if (sock)  close (sock);
            /* Kill down the chain */
        if (pid)   kill (pid, SIGTERM);
            /* Exit gracefully */
        exit (0);
    }
}

void listener_die (int sig)
{
    chain_func (0, 0);
}

char* hex_byte (guint n, guint8* buf)
{
    int i, j;
    guint8 x;
    char* res;
    res = malloc ((2*n +1) * sizeof (char));
    for (i = 0; i < n; ++i)  for (j = 1; j >= 0; --j)
    {
        x = (buf[i] >> (4*j)) & 0xf;
        if (x < 10)
            res[2*i+1-j] = '0' + x;
        else
            res[2*i+1-j] = 'A' + x - 10;
    }
    res[2*n] = 0;
    return res;
}

void receive_string (int sock)
{
    struct sockaddr_storage client;
    socklen_t sizeClient = sizeof (struct sockaddr_storage);
    guint8 buf[BUFSIZ];
        /* ssize_t bytecOut; */
    ssize_t bytecIn;

    fprintf (stderr, "Woot bound a socket!\n");
    while (1)
    {
        bytecIn = recvfrom (sock, buf, BUFSIZ-1, 0,
                            (struct sockaddr*) &client, &sizeClient);
        if (bytecIn < 0)
        {
            perror ("recvfrom() failed");
                /* I guess the best action here is to not keep trying */
            pause ();
        }

            /* buf[bytecIn] = 0; */
            /* fprintf (stderr, "Received: %s\n", buf); */
        {
            char* str = hex_byte (bytecIn, buf);
            fprintf (stderr, "Received: %s\n", str);
            free (str);
        }
    }
}

int
    spawn_listeners
(const struct addrinfo* crit,
 const char* host,
 const char* service,
 void (* listener_fn) (int))
{
    int spawnc = 0;
    struct addrinfo* list;
    struct addrinfo* addr;

    {
        int rtn;
        rtn = getaddrinfo (host, service, crit, &list);
        if (rtn != 0)
        {
            fprintf (stderr, "getaddrinfo() failed: %s\n",
                     gai_strerror (rtn));
            return 0;
        }
    }

    for (addr = list; addr; addr = addr->ai_next)
    {
        int sock;
        int pid;
        sock = socket (addr->ai_family,
                       addr->ai_socktype,
                       addr->ai_protocol);
        if (0 > sock)
        {
            perror ("socket() failed");
            continue;
        }

        if (0 > bind (sock, addr->ai_addr, addr->ai_addrlen))
        {
            perror ("bind() failed");
            close (sock);
            continue;
        }

        pid = fork ();
        switch (pid)
        {
            case -1:
                perror ("fork() failed");
                break;
            case  0:
                    /* Loop forever */
                listener_fn (sock);
                break;
            default:
                    /* We are parent! */
                    /* Set to kill newly forked process */
                    /* That process is set to kill previous one */
                chain_func (sock, pid);
                ++ spawnc;
                break; /* <-- Habit */
        }
        close (sock);
    }

    freeaddrinfo (list);
    return spawnc;
}

int main (int argc, char** argv)
{
    char* service;
    char* host;
    struct addrinfo crit;

    if (2 > argc)
    {
        fputs ("./server port [host]\n", stderr);
        exit (1);
    }

    service = argv[1];

    if (3 <= argc)  host = argv[2];
    else            host = "localhost";


        /* This is used to recursively kill forked processes */
    signal (SIGTERM, listener_die);
        /* ... and sometimes I use C-c */
    signal (SIGINT, listener_die);

    memset (&crit, 0, sizeof (struct addrinfo));
        /* crit.  ai_family = AF_INET6; */
    crit.  ai_family = AF_UNSPEC;
    crit.   ai_flags = AI_PASSIVE;
    crit.ai_socktype = SOCK_DGRAM;
    crit.ai_protocol = IPPROTO_UDP;

    {
        int spawnc;
        spawnc = spawn_listeners (&crit, host, service, receive_string);
        
        if (! spawnc)
        {
            fputs ("Not able to bind to any ports!\n", stderr);
            fclose (stdout);
            exit (1);
        }

        fputs ("should be printing spawn message\n", stderr);

            /* Alert parent script of readiness */
        printf ("%d listeners spawned\n", spawnc);
        fflush (stdout);
        fputs ("should be done printing spawn message\n", stderr);
    }

        /* Block until test script signals our termination */
    pause ();

    fputs ("Should not have gotten to end of main()\n", stderr);
    exit (1);
}


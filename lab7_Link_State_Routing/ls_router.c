#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

// defines
#define N           4
#define INFINITE    1000
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))

// types
typedef struct routers
{
    char    name[50];
    char    ip[50];
    int     port;

} ROUTERS;

// global variables
ROUTERS routers[N];
int     costs[N][N];
int     distances[N];
int     myid, nodes;
int     sock;
struct sockaddr_in addr;
struct sockaddr_in otheraddr;
socklen_t addr_size;
pthread_mutex_t lock;

// print costs
void print_costs (void)
{
    int i, j;

    pthread_mutex_lock (&lock);
    printf ("Current cost table at router %d:\n", myid);
    for (i = 0; i < nodes; i++)
    {
        for (j = 0; j < nodes; j++)
            printf ("%4d ", costs[i][j]);
        printf ("\n");
    }
    printf ("\n");
    pthread_mutex_unlock (&lock);
}

// receive info
void * receive_info (void *arg)
{
    int packet[3];
    int n;
    int src, neigh, newcost;

    while (1)
    {
        addr_size = sizeof (otheraddr);
        n = recvfrom (sock, packet, sizeof (packet), 0,
                      (struct sockaddr *)&otheraddr, &addr_size);
        if (n < 0)
        {
            perror ("recvfrom");
            continue;
        }
        if (n != sizeof (packet))
        {
            // malformed packet, ignore
            continue;
        }

        src     = ntohl (packet[0]);
        neigh   = ntohl (packet[1]);
        newcost = ntohl (packet[2]);

        if (src < 0 || src >= N || neigh < 0 || neigh >= N)
            continue;

        pthread_mutex_lock (&lock);
        costs[src][neigh]  = newcost;
        costs[neigh][src]  = newcost;
        pthread_mutex_unlock (&lock);

        printf ("Received update from router %d about link (%d,%d) new cost %d\n",
                src, src, neigh, newcost);
        print_costs ();
    }

    return NULL;
}

// run_link_state
void * run_link_state (void *arg)
{
    int taken[N];
    int min, spot;
    int i, j;
    int r;

    while (1)
    {
        /* sleep for a random number of seconds between 10 and 20 */
        r = (rand () % 11) + 10;
        sleep (r);

        /* initialization */
        for (i = 0; i < N; i++)
        {
            taken[i] = 0;
            pthread_mutex_lock (&lock);
            distances[i] = costs[myid][i];
            pthread_mutex_unlock (&lock);
        }
        taken[myid] = 1;
        distances[myid] = 0;

        /* Dijkstra's algorithm */
        for (i = 1; i < N; i++)
        {
            /* find closest node not yet taken */
            min = INFINITE;
            spot = -1;
            for (j = 0; j < N; j++)
            {
                if (!taken[j] && distances[j] < min)
                {
                    min = distances[j];
                    spot = j;
                }
            }

            if (spot == -1)
                break;  // remaining nodes are unreachable

            taken[spot] = 1;

            /* recalculate distances using the newly taken node */
            for (j = 0; j < N; j++)
            {
                if (taken[j] == 0)
                {
                    int newdist;
                    pthread_mutex_lock (&lock);
                    newdist = distances[spot] + costs[spot][j];
                    pthread_mutex_unlock (&lock);

                    distances[j] = MIN (distances[j], newdist);
                }
            }
        }

        printf ("New least-cost distances from router %d:\n", myid);
        for (i = 0; i < N; i++)
            printf ("%d ", distances[i]);
        printf ("\n");
    }
}

// main()
int main (int argc, char *argv[])
{
    FILE    *fp;
    int     i, j;
    pthread_t   thr1, thr2;
    int     id, cost;
    int     packet[3];

    // Get from the command line, id, routers, cost table
    if (argc != 5) {
        printf ("Usage: %s <id> <num_routers> <routers_file> <cost_table_file>\n", argv[0]);
        exit (0);
    }

    myid = atoi (argv[1]);
    nodes = atoi (argv[2]);

    if (myid >= N)
    {
        printf ("wrong id\n");
        return 1;
    }

    if (nodes != N)
    {
        printf ("wrong number of nodes\n");
        return 1;
    }

    // get info on routers
    if ((fp = fopen (argv[3], "r")) == NULL)
    {
        printf ("can't open %s\n", argv[3]);
        return 1;
    }

    for (i = 0; i < nodes; i++)
        fscanf (fp, "%s%s%d", routers[i].name, routers[i].ip, &routers[i].port);

    fclose (fp);

    // get costs
    if ((fp = fopen (argv[4], "r")) == NULL)
    {
        printf ("can't open %s\n", argv[4]);
        return 1;
    }

    for (i = 0; i < nodes; i++)
    {
        for (j = 0; j < nodes; j++)
        {
            fscanf (fp, "%d", &costs[i][j]);
        }
    }

    fclose (fp);

    // init address
    addr.sin_family = AF_INET;
    addr.sin_port = htons ((short)routers[myid].port);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    memset ((char *)addr.sin_zero, '\0', sizeof (addr.sin_zero));
    addr_size = sizeof (addr);

    // create socket
    if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf ("socket error\n");
        return 1;
    }

    // bind
    if (bind (sock, (struct sockaddr *)&addr, sizeof (addr)) != 0)
    {
        printf ("bind error\n");
        return 1;
    }

    srand ((unsigned int)(time (NULL)) + myid);

    // create threads
    pthread_mutex_init (&lock, NULL);
    pthread_create (&thr1, NULL, receive_info, NULL);
    pthread_create (&thr2, NULL, run_link_state, NULL);

    // read changes from the keyboard (Thread 2 behavior in main)
    for (i = 0; i < 2; i++)
    {
        sleep (10);    // wait 10 seconds between changes
        printf ("any changes? (neighbor_id new_cost): ");
        fflush (stdout);

        if (scanf ("%d%d", &id, &cost) != 2)
        {
            printf ("input error\n");
            break;
        }

        if (id >= N  ||  id == myid)
        {
            printf ("wrong id\n");
            break;
        }

        pthread_mutex_lock (&lock);
        costs[myid][id] = cost;
        costs[id][myid] = cost;
        pthread_mutex_unlock (&lock);
        print_costs ();

        packet[0] = htonl (myid);
        packet[1] = htonl (id);
        packet[2] = htonl (cost);
        otheraddr.sin_family = AF_INET;
        addr_size = sizeof (otheraddr);

        for (j = 0; j < N; j++)
        {
            if (j != myid)
            {
                otheraddr.sin_port = htons ((short)routers[j].port);
                inet_pton (AF_INET, routers[j].ip, &otheraddr.sin_addr.s_addr);
                sendto (sock, packet, sizeof (packet), 0,
                        (struct sockaddr *)&otheraddr, addr_size);
            }
        }
        printf ("sent\n");
    }

    // finish 30 seconds after executing the changes
    sleep (30);

    return 0;
}


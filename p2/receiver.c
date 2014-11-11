#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd;
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char *hostname, *filename;

    float p_loss, p_corrupt;

    char buffer[256];
    
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s hostname port filename [loss ratio] [corrupt ratio]\n", argv[0]);
        exit(0);
    }

    switch (argc)
    {
        case 6:
            p_corrupt = atof(argv[5]);
        case 5:
            p_loss = atof(argv[4]);
        default:
            filename = argv[3];
            portno = atoi(argv[2]);
            hostname = argv[1];
            break;
    }

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "Error: no such host\n");
        exit(0);
    }

    // sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Using UDP!

    // if (sockfd < 0)
    // {
    //     error("Error opening socket");
    // }

    // bzero((char *)&serv_addr, sizeof(serv_addr));
    // serv_addr.sin_family = AF_INET;
    // bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    // serv_addr.sin_port = htons(portno);

    /*
     * 1. Make connection built on reliable data transfer algorithm
     * 2. Using connection, send data
     * 3. Receive data
     * 4. Print data
     */


}
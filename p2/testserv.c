/*
    Simple udp server
    Silver Moon (m00n.silv3r@gmail.com)
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
 
#define BUFLEN 1000  //Max length of buffer
 
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(int argc, char *argv[])
{
    struct sockaddr_in serv_si, cli_si;
     
    int portno, sockfd, i, slen = sizeof(cli_si) , recv_len;
    char buf[BUFLEN];
     
    portno = atoi(argv[1]);
    
    //create a UDP socket
    if ((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
            exit(1);
    }
     
    // zero out the structure
    memset((char *) &serv_si, 0, sizeof(serv_si));
     
    serv_si.sin_family = AF_INET;
    serv_si.sin_port = htons(portno);
    serv_si.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(sockfd , (struct sockaddr*)&serv_si, sizeof(serv_si) ) == -1)
    {
            exit(1);
    }
     
    //keep listening for data
    while(1)
    {
        printf("Waiting for data...");
        fflush(stdout);
         
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &cli_si, &slen)) == -1)
        {
            //exit(1);
        }
        
        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(cli_si.sin_addr), ntohs(cli_si.sin_port));
        
        int seg_num, ack_num;
        memcpy(&seg_num, buf, sizeof(seg_num));
        memcpy(&ack_num, buf+sizeof(seg_num), sizeof(ack_num));
        printf("%d", ack_num);
        //printf("File requested: %s\n" , buf);
        
        //now reply the client with the same data
        if (sendto(sockfd, buf, recv_len, 0, (struct sockaddr*) &cli_si, slen) == -1)
        {
            //exit(1);
        }
    }
 
    close(sockfd);
    return 0;
}
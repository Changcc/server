/*
UPD GBN N=4
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include <unistd.h> // for close
#include<signal.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include "packet.h"
 
#define N 4 //window size
#define T 4 //timeout seconds
 
char timeout = 'f';
void die(char *s, int socket)
{
    perror(s);
    close(socket);
    exit(1);
}
double chance()
{
    return (double)rand() / (double)RAND_MAX;
}
void handler(int signo)
{
	//alert when timed out
    alarm(4);
    timeout = 't';
}
int main(int argc, char *argv[])
{
	double p_loss, p_corrupt;
	int ppid = getpid();
    p_loss = 0.0;
    p_corrupt = 0.0;
    
    struct sockaddr_in serv_si, cli_si;
	struct packet *rcvpkt;
	rcvpkt = make_packet();
    FILE *file;
     
    int portno, sockfd, i, recv_len;
    int slen = sizeof(cli_si);
     
    portno = atoi(argv[1]);
    
    //create a UDP socket
    if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
            die("socketfd error", sockfd);
    }
     
    // zero out the structure
    memset((char *) &serv_si, 0, sizeof(serv_si));
     
    serv_si.sin_family = AF_INET;
    serv_si.sin_port = htons(portno);
    serv_si.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(sockfd , (struct sockaddr*)&serv_si, sizeof(serv_si) ) == -1)
    {
            die("error in bind\n", sockfd);
    }
    
    printf("Waiting for syn...\n");
    if ((recv_len = recvfrom(sockfd, rcvpkt, sizeof(struct packet), 0, (struct sockaddr *) &cli_si, (socklen_t*)&slen)) == -1)
	{
        die("error receive syn", sockfd);
	}
    
    if (check_syn(rcvpkt) != 0) {
    	//received syn flag, start connection, send packets
    	file = fopen(rcvpkt->data, "r");
    }
    
    int base = 1;
    int nextseq = 1; //next seq count
    int timer, rcvr; //timer, rcvr pid
    struct packet pkt[N];
    char refuse_data = 'f'; //f for false t for true 
    signal(SIGALRM, handler); //sets timeout handler
    rcvr = fork();
    if (rcvr == 0) { //in charge of receiving ack
    	while (refuse_data == 'f') {
    		struct packet *rcvpkt;
    		rcvpkt = make_packet();
    		if ((recv_len = recvfrom(sockfd, rcvpkt, sizeof(struct packet), 0, (struct sockaddr *) &cli_si, (socklen_t*)&slen)) == -1)
			{
		        die("Receive error:\n", sockfd);
			}
			else if (chance() < p_corrupt)
			{
				printf("Packet from sender CORRUPT\n");
			}
			else {
				printf("Packet Ack: %d\n", rcvpkt->seq_num);
				base = rcvpkt->seq_num + DATA_SIZE;
			}
    	}
    }
    else { //is parent process
		while (1) { //in charge of sending packets
			if (timeout == 't') {
				int n_char;
				int resend_base = base;
				while (resend_base < nextseq) {
					n_char = sendto(sockfd, &pkt[resend_base], sizeof(pkt[resend_base]), 0, (struct sockaddr*)&cli_si, slen);
					if (n_char < 0)
					{
						die("Error sending packet", sockfd);
					}
					resend_base++;
				}
				timeout = 'f';
			}
		
			if (nextseq < base + N) {
				//while within window, keep sending packet
				char buf[500];
				memset(buf, 0, 500);
				if (fread(buf, sizeof(char),  sizeof(buf), file) == 0) {
					struct packet *finpkt = make_packet();
					set_fin(finpkt);
					int n_char;
					n_char = sendto(sockfd, finpkt, sizeof(struct packet), 0, (struct sockaddr*)&cli_si, slen);
					if (n_char < 0) {
						die("Error sending packet", sockfd);
					}
					else {
						close(sockfd);
						exit(0);
					}
				}
				struct packet *nextpkt = make_packet();
				set_data(nextpkt, buf);
				nextpkt->seq_num = nextseq;
				pkt[nextseq] = *nextpkt;
				int n_char;
				n_char = sendto(sockfd, &pkt[nextseq], sizeof(pkt[nextseq]), 0, (struct sockaddr*)&cli_si, slen);
				if (n_char < 0) {
					die("Error sending packet", sockfd);
				}
				else {
					printf("Data sent: %s\n", pkt[nextseq].data);
				}
				if (nextseq == base) {
					//start timer
					alarm(4);
				}
				nextseq++;
			}
			else {
				refuse_data = 't';
			}
		}
    }
    close(sockfd);
    return 0;
}
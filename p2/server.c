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
int base;
int killp;
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

void updatebasehdlr(int signo)
{
	//updates base number when receive ack
	base++;
}

void killparent(int signo)
{
	killp = 1;
}
int main(int argc, char *argv[])
{
	double p_loss, p_corrupt;
	int ppid = getpid();
    p_loss = 0.0;
    p_corrupt = 0.0;
    killp = 0;
    
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
    	//received syn flag, start connection, send packets;
    	file = fopen(rcvpkt->data, "rb");
    	printf("Packet ACK: %d\n", rcvpkt->seq_num);
    }
    
    base = 1;
    int nextseq = 1; //next seq count
    int pktindex;
    int timer, rcvr; //timer, rcvr pid
    struct packet pkt[N];
//     char refuse_data = 'f'; //f for false t for true 
    signal(SIGALRM, handler); //sets timeout handler
    signal(SIGUSR1, updatebasehdlr); //sets base update handler
    signal(SIGUSR2, killparent); //sets base update handler
    rcvr = fork();
    if (rcvr == 0) { //in charge of receiving ack
    	int lastack = 0;
    	while (1) {
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
			else if (check_fin(rcvpkt)) {
				printf("Goodbye...\n");
				close(sockfd);
				kill(ppid, SIGUSR2);
				exit(0);
			}
			else if (rcvpkt->seq_num == lastack) {
				//received syn flag, start connection, send packets;
			}
			else {
				printf("Packet Ack: %d\n", rcvpkt->seq_num);
				lastack = rcvpkt->seq_num;
				kill(ppid, SIGUSR1);
//				base = rcvpkt->seq_num + 1;
			}
    	}
    }
    else { //is parent process
		while (1) { //in charge of sending packets
			if (killp == 1) {
				fclose(file);
				close(sockfd);
				exit(0);
			}
			if (timeout == 't') {
				int n_char;
				int resend_base = base;
				while (resend_base < nextseq) {
					pktindex = (resend_base - 1) % 4;
					n_char = sendto(sockfd, &pkt[pktindex], sizeof(pkt[pktindex]), 0, (struct sockaddr*)&cli_si, slen);
					if (n_char < 0)
					{
						die("Error sending packet during timeout", sockfd);
					}
					printf("NextSeq: %d\n", resend_base);
					resend_base++;
				}
				timeout = 'f';
				alarm(4);
			}
		
			if (nextseq < base + N) {
				//while within window, keep sending packet
				char buf[500];
				memset(buf, 0, 500);
				int readlength;
				if ((readlength = fread(buf, sizeof(char),  sizeof(buf), file)) == 0) {
					struct packet *finpkt = make_packet();
					set_fin(finpkt);
					int n_char;
					finpkt->seq_num = nextseq;
					n_char = sendto(sockfd, finpkt, sizeof(struct packet), 0, (struct sockaddr*)&cli_si, slen);
					if (n_char < 0) {
						die("Error sending packet during fin", sockfd);
					}
					else {
						printf("Fin packet sent...\n");
						alarm(4);
					}
				}
				else {
					struct packet *nextpkt = make_packet();
					set_data(nextpkt, buf, readlength);
					nextpkt->seq_num = nextseq;
					pktindex = (nextseq-1)%4;
					pkt[pktindex] = *nextpkt;
					int n_char;
					printf("NextSeq: %d\n", nextseq);
					n_char = sendto(sockfd, &pkt[pktindex], sizeof(pkt[pktindex]), 0, (struct sockaddr*)&cli_si, slen);
					if (n_char < 0) {
						die("Error sending packet during data", sockfd);
					}
					else {
						//printf("Data sent: %s\n", pkt[pktindex].data);
					}
					if (nextseq == base) {
						//start timer
						alarm(4);
					}
					nextseq++;
				}
			}
			else {
				//refuse_data = 't';
			}
		}
    }
    close(sockfd);
    return 0;
}
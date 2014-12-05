/*
UPD GBN N=4
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include <unistd.h> // for close
#include<arpa/inet.h>
#include<sys/socket.h>
#include "packet.c"
 
#define N 4 //window size
 
void die(char *s)
{
    perror(s);
    exit(1);
}
double chance()
{
    return (double)rand() / (double)RAND_MAX;
}
 
int main(int argc, char *argv[])
{
	double p_loss, p_corrupt;
    p_loss = 0.0;
    p_corrupt = 0.0;
    
    struct sockaddr_in serv_si, cli_si;
	struct packet *rcvpkt;
    FILE *file;
     
    int portno, sockfd, i, recv_len;
    int slen = sizeof(cli_si);
     
    portno = atoi(argv[1]);
    
    //create a UDP socket
    if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
            die("socketfd error");
    }
     
    // zero out the structure
    memset((char *) &serv_si, 0, sizeof(serv_si));
     
    serv_si.sin_family = AF_INET;
    serv_si.sin_port = htons(portno);
    serv_si.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(sockfd , (struct sockaddr*)&serv_si, sizeof(serv_si) ) == -1)
    {
            die("error in bind\n");
    }
    
    printf("Waiting for syn...\n");
    if ((recv_len = recvfrom(sockfd, rcvpkt, sizeof(rcvpkt), 0, (struct sockaddr *) &cli_si, (socklen_t*)&slen)) == -1)
	{
        die("error receive syn");
	}

    printf("%d\n", recv_len);
    printf("%s\n", rcvpkt->data);
    printf("%d\n", check_syn(rcvpkt));
    
    if (check_syn(rcvpkt) != 0) {
    	//received syn flag, start connection, send packets
    	file = fopen(rcvpkt->data, "r");
    }
    
    int base = 0;
    int ack; //last seq ack'd
    int nextseq = 0; //next seq count
    int timer, rcvr; //timer, rcvr pid
    struct packet pkt[N];
    char refuse_data = 'f'; //f for false t for true 
    char timeout = 'f';
    
    timer = fork();
    if (timer == 0) { //in charge of timer
    	//signals timeout
    	//receives signal to start timer
    	
    }
    
    rcvr = fork();
    if (rcvr == 0) { //in charge of receiving ack
    	while (refuse_data == 'f') {
    		struct packet *rcvpkt;
    		if ((recv_len = recvfrom(sockfd, rcvpkt, sizeof(rcvpkt), 0, (struct sockaddr *) &cli_si, (socklen_t*)&slen)) == -1)
			{
		        printf("Packet from sender LOST\n");
			}
			else if (chance() < p_corrupt)
			{
				printf("Packet from sender CORRUPT\n");
			}
			else {
				base = rcvpkt->ack_num + DATA_SIZE;
			}
    	}
    }
    
    while (1) { //in charge of sending packets
    	if (timeout == 't') {
    		int n_char;
    		int resend_base = base;
    		while (resend_base < nextseq) {
				n_char = sendto(sockfd, &pkt[resend_base], sizeof(&pkt[resend_base]), 0, (struct sockaddr*)&cli_si, slen);
				if (n_char < 0)
				{
					die("Error sending packet");
				}
				resend_base++;
			}
    	}
    	
		if (nextseq < base + N) {
			//while within window, keep sending packet
			char buf[DATA_SIZE];
			fread(buf, sizeof(char),  sizeof(buf), file);
			struct packet *nextpkt = make_packet();
			set_data(nextpkt, buf);
			
    		int n_char;
			n_char = sendto(sockfd, &pkt[nextseq], sizeof(&pkt[nextseq]), 0, (struct sockaddr*)&cli_si, slen);
			if (n_char < 0) {
				die("Error sending packet");
			}
			if (nextseq == base) {
				//start timer
			}
			nextseq++;
		}
		else {
			refuse_data = 't';
		}
    }
    
    close(sockfd);
    return 0;
}
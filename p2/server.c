/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/

/*	Header:
	16 bit: source			16 bit: destination
	32 bit: Seq 
	32 bit: Ack
	1 bit: Ack flag 	1 bit: Syn		1 bit: Fin 			29 bit of junk (0)
	16 bit: checksum										16 bit of junk (0)
	Options: nothing for now
	
*/

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */

/*	Server Specific
	must construct packet:
		header + 1k ...	
	Create Application Header + fgets 1k byte
	Timer
	Estimate Timeout set to constant: 1 sec
	Compute checksum
*/

#define SERVER_STRING "Server: cs118webserver\r\n"

struct header
{
	short source, destination, chksum;
	int seq, ack;
	bool a, s, f;
};

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void dostuff(int); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     struct sigaction sa;          // for signal SIGCHLD

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     listen(sockfd,5);
     
     clilen = sizeof(cli_addr);
     
     /****** Kill Zombie Processes ******/
     sa.sa_handler = sigchld_handler; // reap all dead processes
     sigemptyset(&sa.sa_mask);
     sa.sa_flags = SA_RESTART;
     if (sigaction(SIGCHLD, &sa, NULL) == -1) {
         perror("sigaction");
         exit(1);
     }
     /*********************************/
     
     while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         
         if (newsockfd < 0) 
             error("ERROR on accept");
         
         pid = fork(); //create a new process
         if (pid < 0)
             error("ERROR on fork");
         
         if (pid == 0)  { // fork() returns a value of 0 to the child process
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else //returns the process ID of the child process to the parent
             close(newsockfd); // parent doesn't need this 
     } /* end of while */
     return 0; /* we never get here */
}


/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   int n;
   char buffer[256];
   char *tok;
      
   /*
   Simply reads the buffer, write it out to the console. Then breaks the buffer into tokens
   and find the path requested.
   */
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) error("ERROR reading from socket");
   printf("%s\n", buffer);
   tok = strtok(buffer, " ");
   tok = strtok(NULL, " ");
   printf("token: %s\n",tok);
   FILE *file = NULL;
   
   /*
   If no file is given, then return nothing. This simplifies the server so that we don't have to handle the case where no file path is provided, and the server assumes to seek out the index file. 
   This server requires the path to be specified or the default behavior is to return nothing.
   
   If we were to include the search for the index file, then we would need to seek out index files of all types of extensions, which might also require other checking, like PHP and ASP files.
   */
   if (strcmp(tok, "/") != 0) {
   
     file = fopen(tok+1, "r");
     if (file != NULL) {
     
     /*
     If a file is requested, then we look for that file. If that file exists, then
     create the header and send the content of the file.
     Clearly, because file extension is not checked, we assume that the file is a text/html file.
     */
     char buf[1024];
     strcpy(buf, "HTTP/1.1 200 OK\r\n");
     send(sock, buf, strlen(buf), 0);
	 strcpy(buf, SERVER_STRING);
	 send(sock, buf, strlen(buf), 0);
	 // sprintf(buf, "Content-Type: text/html\r\n");
// 	 send(sock, buf, strlen(buf), 0);
	 strcpy(buf, "\r\n");
	 send(sock, buf, strlen(buf), 0);
	 
	 fgets(buf, sizeof(buf), file);
	 do
	 {
	  send(sock, buf, strlen(buf), 0);
	  fgets(buf, sizeof(buf), file);
	  //printf("%s\n", buf);
	 } while (!feof(file));
	 
	 fclose(file);
	 }
   }
}

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <mraa/aio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

sig_atomic_t volatile run_flag = 1;
int id = 404606017;

static int PORT_NUMBER = 16000;
static int TLS_PORT_NUMBER = 17000;
static int BUF_SIZE = 1024;
char *hostname = "r01.cs.ucla.edu";

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main (int argc, char **arv)
{
	int sockfd, n;
	struct sockaddr_in server_address;
	struct hostent* server;
	char buf[BUF_SIZE];
	
	//creating the socket
	if(sockfd = socket(AF_INET, SOCK_STREAM, 0))
	{
		fprintf(stderr, "Error: Socket creation failed\n");
		fflush(stderr);
		return 1;
	}
	
	server = gethostbyname(hostname);
	struct in_addr* serverIP = server -> h_addr_list[0];
	
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT_NUMBER);
	server_address.sin_addr = *serverIP;
	//connect to server
	if(connect(sockfd, &server_address, sizeof(server_address)) < 0)
	{
		fprintf(stderr, "Error in connecting to server\n");
		fflush(stderr);
		return 1;
	}
	char* ID_to_send = "404606017";
	char response[1024];
	char message_buf[1024];
	
	if(write(sockfd, ID_to_send, strlen(ID_to_send)) < 0)
	{
		fprintf(stderr, "Error writing to socket\n");
	}
	
	read(sockfd, response, 1024);
	fprintf(stderr, "Response from server: %s", response);
	close(sockfd);
}

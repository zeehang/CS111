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

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main (int argc, char **arv)
{
	int sockfd, n;
	struct sockaddr_in server_address;
	struct hostent *server;
	char *hostname = "r01.cs.ucla.edu";

}

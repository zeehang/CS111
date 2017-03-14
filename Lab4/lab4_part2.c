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

static unsigned int PORT_NUMBER = 16000;
static unsigned int TLS_PORT_NUMBER = 17000;
static int BUF_SIZE = 1024;
char *hostname = "r01.cs.ucla.edu";
pthread_mutex_t mut;
volatile int temp_scale = 0; //0 is F, 1 is Celsius
volatile int period = 3;
int sockfd, n;
FILE* file;

void error(char *msg)
{
	perror(msg);
	exit(0);
}

float temp_equation(int initial)
{
	int B = 4275;
	float R = 1023.0/((float) initial) - 1.0;
	R = 100000.0*R;
	float temperature = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;
	if(temp_scale == 0)
	{
		temperature = temperature * 9/5 + 32;
	}
	return temperature;
}

void* temperature_sense()
{
	fprintf(stderr, "Entered temperature_sense\n");
	mraa_aio_context temp_sensor;
	temp_sensor = mraa_aio_init(3);
	//mraa_gpio_dir(temp_sensor, MRAA_GPIO_IN);
	int value;

	//fprintf(stderr,"hello");
	char* ID_to_send = "404606017";
	if(write(sockfd, ID_to_send, strlen(ID_to_send)) < 0)
	{
		fprintf(stderr, "Error writing to socket\n");
	}
	
	while(1)
	{
		if(run_flag)
		{
			value = mraa_aio_read(temp_sensor);
			time_t curr_time = time(NULL);
			struct tm* format_time;
			format_time = localtime(&curr_time);
			char time_str[10];
			memset(time_str, 0, 10);
			strftime(time_str, 9, "%H:%M:%S", format_time);
			float converted = temp_equation(value);
			pthread_mutex_lock(&mut);
			fprintf(file,"%s %.1f\n", time_str, converted);
			pthread_mutex_unlock(&mut);
			char buffer[1024];
			sprintf(buffer, "%d TEMP=%.1f", id, converted);
			if(write(sockfd, buffer, strlen(buffer)) < 0)
			{
				fprintf(stderr, "Error writing to socket\n");
				fflush(stderr);
			}
			fflush(file);
			sleep(period);
		}
	}
	mraa_aio_close(temp_sensor);
}

void* read_server_data()
{
	fprintf(stderr, "Entered read_server_data\n");
	while(1)
	{
		char response[1024];
		bzero(response, 1024);

		if(read(sockfd, response, 1024) < 0)
		{
			fprintf(stderr, "Error reading from socket\n");	
			fflush(stderr);
		}
		if(strcmp(response, "START") == 0)
		{
			run_flag = 1;
			pthread_mutex_lock(&mut);
			fprintf(file, "%s\n", response);
			fflush(file);
			pthread_mutex_unlock(&mut);
		}
		else if(strcmp(response, "STOP") == 0)
		{
			run_flag = 0;
			pthread_mutex_lock(&mut);
			fprintf(file, "%s\n", response);
			fflush(file);
			pthread_mutex_unlock(&mut);
		}
		else if(strcmp(response, "OFF") == 0)
		{
			pthread_mutex_lock(&mut);
			fprintf(file, "%s\n", response);
			fflush(file);
			pthread_mutex_unlock(&mut);
			close(sockfd);
			fclose(file);
			exit(0);
		}
		else if(response[0] == 'S')
		{
			// char scale_check[5];
			// bzero(scale_check, 5);
			// strncpy(scale_check, response, 5);
			// if(scale_check == "SCALE")
			// {
				if(response[6] == 'F')
				{
					temp_scale = 0;
					pthread_mutex_lock(&mut);
					fprintf(file, "%s\n", response);
					fflush(file);
					pthread_mutex_unlock(&mut);
				}
				else if(response[6] == 'C')
				{
					temp_scale = 1;
					pthread_mutex_lock(&mut);
					fprintf(file, "%s\n", response);
					fflush(file);
					pthread_mutex_unlock(&mut);
				}
				else
				{
					pthread_mutex_lock(&mut);
					fprintf(file, "%s I\n", response);
					fflush(file);
					pthread_mutex_unlock(&mut);
				}
			// }
			// else
			// {
			// 	pthread_mutex_lock(&mut);
			// 	fprintf(file, "%s I\n", response);
			// 	fflush(file);
			// 	pthread_mutex_unlock(&mut);
			// }
		}
		else if(response[0] == 'P')
		{
			// char period_check[6];
			// bzero(period_check, 6);
			// strncpy(period_check, response, 6);
			// period_check[6] = 0;
			// if(period_check == "PERIOD")
			// {
				char* number = malloc(sizeof(char)*4);
				number = response+7;
				int number_conv = atoi(number);
				if(number_conv < 1 || number_conv > 3600)
				{
					pthread_mutex_lock(&mut);
					fprintf(file, "%s I\n", response);
					fflush(file);
					pthread_mutex_unlock(&mut);
				}
				else
				{
					pthread_mutex_lock(&mut);
					fprintf(file, "%s\n", response);
					fflush(file);
					pthread_mutex_unlock(&mut);
					period = number_conv;
					fprintf(stderr, "Period set to: %d\n", period);
				}
			// }
			// else
			// {
			// 	pthread_mutex_lock(&mut);
			// 	fprintf(file, "%s I\n", response);
			// 	fflush(file);
			// 	pthread_mutex_unlock(&mut);
			// }
		}
		else if(response[0] == 'D')
		{
			pthread_mutex_lock(&mut);
			fprintf(file, "%s\n", response);
			fflush(file);
			pthread_mutex_unlock(&mut);
		}
		fprintf(stderr, "%s\n", response);
		fflush(stderr);
	}
}

int main (int argc, char **arv)
{
	struct sockaddr_in server_address;
	struct hostent* server;
	char buf[BUF_SIZE];
	
	//creating the socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		fprintf(stderr, "Error: Socket creation failed\n");
		fflush(stderr);
		return 1;
	}
	
	server = gethostbyname(hostname);
	if(server == NULL)
	{
		fprintf(stderr, "ERROR: host not found\n");
	}
	//struct in_addr* serverIP = server -> h_addr_list[0];
	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char *) server->h_addr,(char *)&server_address.sin_addr.s_addr, server->h_length);
	server_address.sin_port = htons(PORT_NUMBER);

	//server_address.sin_addr = server -> h_addr;
	//server_address.sin_addr = *serverIP;
	//connect to server
	if(connect(sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		fprintf(stderr, "Error in connecting to server\n");
		fflush(stderr);
	}

	file = fopen("log_part2.txt", "w");
	pthread_t threadId[2];
	//start the first thread 
	pthread_create(&(threadId[0]), NULL, temperature_sense, NULL);
	//start the second thread 
	pthread_create(&(threadId[1]), NULL, read_server_data, NULL);

	pthread_join(threadId[0], NULL);
	pthread_join(threadId[1], NULL);
	close(sockfd);
	fclose(file);
}

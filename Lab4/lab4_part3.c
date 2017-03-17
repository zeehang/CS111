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
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/pem.h>
#include "display.h"

SSL *ssl;
int sockfd;
sig_atomic_t volatile run_flag = 1;
int id = 404606017;
pthread_mutex_t mut;
volatile int temp_scale = 0; //0 is F, 1 is Celsius
volatile int period = 3;
volatile int display = 1; //1 is on, 0 is off
FILE* file;

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
	temp_sensor = mraa_aio_init(0);
	//mraa_gpio_dir(temp_sensor, MRAA_GPIO_IN);
	int value;

	//fprintf(stderr,"hello");
	char* ID_to_send = "404606017";
	if(SSL_write(ssl, ID_to_send, strlen(ID_to_send)) < 0)
	{
		fprintf(stderr, "Error writing ID to ssl\n");
	}
	pthread_mutex_lock(&mut);
	fprintf(file,"%s\n", ID_to_send);
	pthread_mutex_unlock(&mut);
	
	while(1)
	{
		if(display)
		{
			display_on();
			set_RGB(255,255,255);
		}
		else
		{
			send_command(LCD_CLEARDISPLAY);
			set_RGB(0,0,0);
			display_off();
		}
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
			if(display)
			{
				char disp_buffer[1024];
				bzero(disp_buffer, 1024);
				char temp;
				if(temp_scale == 0)
					temp = 'F';
				else
					temp = 'C';
				sprintf(disp_buffer, "%.1f%c", converted, temp);
				print_temperature(disp_buffer);
			}
			if(SSL_write(ssl, buffer, strlen(buffer)) < 0)
			{
				fprintf(stderr, "Error writing to ssl\n");
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

		if(SSL_read(ssl, response, 1024) < 0)
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
            SSL_free(ssl);
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
			if(response[5] == 'Y')
			{
				display = 1;
				pthread_mutex_lock(&mut);
				fprintf(file, "%s\n", response);
				fflush(file);
				pthread_mutex_unlock(&mut);
			}
			else if(response[5] == 'N')
			{
				display = 0;
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
		}
		fprintf(stderr, "%s\n", response);
		fflush(stderr);
	}
}

int main()
{
    char hostname[] = "r01.cs.ucla.edu";
    int port_number = 17000;

    const SSL_METHOD *method;
    SSL_CTX *ctx;

    struct hostent *server;
    struct sockaddr_in server_address;

    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    SSL_library_init();

    method = SSLv23_client_method();

    ctx = SSL_CTX_new(method);

     SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

     server = gethostbyname(hostname);

     ssl = SSL_new(ctx);

     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     server_address.sin_family = AF_INET;
     server_address.sin_port=htons(port_number);
     server_address.sin_addr.s_addr = *(long*)(server -> h_addr);

     if(connect(sockfd, (struct sockaddr*) &server_address, sizeof(struct sockaddr))<0)
        fprintf(stderr, "Error connecting sockfd\n");
    else
        fprintf(stderr, "sockfd connect successfully\n");

    SSL_set_fd(ssl, sockfd);
    if(SSL_connect(ssl) < 0)
        fprintf(stderr, "Error connecting SSL_connect\n");
    else
        fprintf(stderr, "Success in connecting SSL_connect\n");
    file = fopen("lab4_3.log", "w");
    pthread_t threadId[2];
	//initialize the display
	initialize_connection();
	//start the first thread 
	pthread_create(&(threadId[0]), NULL, temperature_sense, NULL);
	//start the second thread 
	pthread_create(&(threadId[1]), NULL, read_server_data, NULL);

	pthread_join(threadId[0], NULL);
	pthread_join(threadId[1], NULL);
    SSL_free(ssl);
    close(sockfd);
	fclose(file);
    return 0;
}
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <mraa/aio.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

sig_atomic_t volatile run_flag = 1;

int ID = 404606017;

float temp_equation(int initial)
{
	int B = 4275;
	float R = 1023.0/((float) initial) - 1.0;
	R = 100000.0*R;
	float temperature = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;

	return temperature;
}

int main()
{
	mraa_aio_context temp_sensor;
	temp_sensor = mraa_aio_init(0);
	//mraa_gpio_dir(temp_sensor, MRAA_GPIO_IN);
	int value;
	FILE* file;
	//fprintf(stderr,"hello");
	file = fopen("lab4_1.log", "w");
	while(1)
	{
		value = mraa_aio_read(temp_sensor);
		time_t curr_time = time(NULL);
		struct tm* format_time;
		format_time = localtime(&curr_time);
		char time_str[10];
		memset(time_str, 0, 10);
		strftime(time_str, 9, "%H:%M:%S", format_time);
		float converted = temp_equation(value);
		converted = converted * 9/5 + 32;
		fprintf(file,"%s %.1f\n", time_str, converted);
		fflush(file);
		fprintf(stdout, "%s %.1f\n", time_str, converted);
		fflush(stdout);
		sleep(1);
	}
	fclose(file);
	mraa_aio_close(temp_sensor);

}

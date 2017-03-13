#include <math.h>

const int B=4275;
const int R0 = 100000;
const int pinTempSensor = A3;

void setup()
{
	Serial.begin(9600);
}

void loop()
{
	int a = analogRead(pinTempSensor);
	float R = 1023.0/((float)a)-1.0;
	R = 100000.0*R;

	float temperature = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;

	Serial.print("temperature = ");
	Serial.println(temperature);

	delay(100);
}

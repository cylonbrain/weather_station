#include <VirtualWire.h>

#include <Wire.h>
#include <VirtualWire.h>
#include <Adafruit_BMP085.h>
#include <dht.h>
#include<stdlib.h>


dht DHT;
Adafruit_BMP085 bmp;

#define DHT22_PIN 6
#define AMBIENT_LIGHT_PIN A1
#define MOISTURE_PIN A2
#define TRANSMITTER_PIN A6
#define SAMPLES 2
#define OUTPUT_STRING_LENGTH 64

int sensorValue = 0;
float temperature1[SAMPLES];
float temperature2[SAMPLES];
int32_t pressure1[SAMPLES];
int32_t pressure2[SAMPLES];
float altitude[SAMPLES];
float humidity[SAMPLES];
int illumination[SAMPLES];
unsigned int moisture[SAMPLES];

int current = 0;
String output_string;

void setup()
{
  memset(temperature1,0,sizeof(temperature1));
  memset(temperature2,0,sizeof(temperature2));
  memset(pressure1,0,sizeof(pressure1));
  memset(pressure2,0,sizeof(pressure2));
  memset(altitude,0,sizeof(altitude));
  memset(humidity,0,sizeof(humidity));
  memset(illumination,0,sizeof(illumination));
  memset(moisture,0,sizeof(moisture));
  
  vw_set_tx_pin(TRANSMITTER_PIN);
  vw_setup(2000);
  Serial.begin(9600);
  Serial.println("WEATHER STATION");
  Serial.print("DHT LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }
  Serial.println("Humidity (%),Temperature DHT22(C),Temperature BMP085(C),Ambient Light, Pressure(hPa), Pressure MSL(hPa), Altitude(m), Moisture");

}

void loop()
{

  // READ DATA
  int chk = DHT.read22(DHT22_PIN);          
  if(chk == DHTLIB_OK){
    humidity[current] = DHT.humidity;
    temperature1[current] = DHT.temperature;
  }
  temperature2[current] = bmp.readTemperature();
  illumination[current] = map(analogRead(AMBIENT_LIGHT_PIN), 0, 400, 255, 0);
  pressure1[current]    = bmp.readPressure();
  pressure2[current]    = bmp.readSealevelPressure(64.5 /* messured on 06.12.2014 */ );
  altitude[current]    = bmp.readAltitude();
  moisture[current] = map(analogRead(MOISTURE_PIN), 0, 1024, 0, 99);
  
  // DISPLAY DATA
  if(current >= (SAMPLES-1)) {
    float med_hum  = 0;
    float med_temp1 = 0;
    float med_temp2 = 0;
    float med_ilum = 0;
    float med_pressure1 = 0;
    float med_pressure2 = 0;
    float med_altitude = 0;
    float med_moisture=0;

    for(int i=0; i < SAMPLES; i++){
      med_hum  += humidity[i];
      med_temp1 += temperature1[i];
      med_temp2 += temperature2[i];
      med_ilum += illumination[i];
      med_pressure1 += pressure1[i];
      med_pressure2 += pressure2[i];
      med_altitude  += altitude[i];
      med_moisture  += moisture[i];
    }

    med_hum  /= SAMPLES;
    med_temp1 /= SAMPLES;
    med_temp2 /= SAMPLES;
    med_ilum /= SAMPLES;
    med_pressure1 /= SAMPLES;
    med_pressure2 /= SAMPLES;
    med_altitude /= SAMPLES;
    med_moisture /= SAMPLES;

    output_string = "$";

    char tmp[10];
    dtostrf(med_hum,1,2,tmp);
    output_string.concat(tmp);
    output_string += ";";
    dtostrf(med_temp1,1,2,tmp);
    output_string.concat(tmp);
    output_string += ";";
    dtostrf(med_temp2,1,2,tmp);
    output_string.concat(tmp);
    output_string += ";";
    dtostrf(med_ilum,1,2,tmp);
    output_string.concat(tmp);
    output_string += ";";
    dtostrf(med_pressure1/100,1,2,tmp);
    output_string.concat(tmp);
    output_string += ";";
    dtostrf(med_pressure2/100,1,2,tmp);
    output_string.concat(tmp);
    output_string += ";";
    dtostrf(med_altitude,1,2,tmp);
    output_string.concat(tmp);
    output_string += ";";
    dtostrf(med_moisture,1,1,tmp);
    output_string.concat(tmp);
    output_string += "#";

    current = 0;
  }
  

  Serial.println(output_string);
#ifdef TRANSMIT
  digitalWrite(A10, HIGH);
  digitalWrite(13, HIGH);
  
  int rem_length = output_string.length();
  while(rem_length > 0){
    int to_send = 0;
    if(rem_length > VW_MAX_PAYLOAD){
      to_send = VW_MAX_PAYLOAD;
    } else {
      to_send = rem_length;
    }
    vw_send((uint8_t *)output_string.c_str()+(output_string.length()-rem_length), to_send);
    vw_wait_tx(); // Wait until the whole message is gone
    rem_length -= to_send;
    delay(100);
  }
  digitalWrite(13, LOW);
  digitalWrite(A10, LOW);
#endif

  delay(1000);
  current++;
}
//
// END OF FILE
//

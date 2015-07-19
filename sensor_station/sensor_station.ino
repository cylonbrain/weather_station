#include <Wire.h>
#include <VirtualWire.h>
#include <Adafruit_BMP085.h>
#include <dht.h>
#include <stdlib.h>
#include <movingAvg.h>

dht DHT;
Adafruit_BMP085 bmp;

#define DHT22_PIN 6
#define AMBIENT_LIGHT_PIN A1
#define MOISTURE_PIN A2
#define TRANSMITTER_PIN A6
#define SAMPLES 2
#define OUTPUT_STRING_LENGTH 64

movingAvg avg_temperature1;
movingAvg avg_temperature2;
movingAvg avg_pressure1;
movingAvg avg_pressure2;
movingAvg avg_altitude;
movingAvg avg_humidity;
movingAvg avg_illumination;
movingAvg avg_moisture;

float temperature1;
float temperature2;
float pressure1;
float pressure2;
float altitude;
float humidity;
float illumination;
float moisture;


int current = 0;
String output_string;

void setup()
{

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
    humidity = DHT.humidity;
    temperature1 = DHT.temperature;
  }
  temperature2 = bmp.readTemperature();
  illumination = map(analogRead(AMBIENT_LIGHT_PIN), 0, 400, 255, 0);
  pressure1    = bmp.readPressure();
  pressure2    = bmp.readSealevelPressure(64.5 /* messured on 06.12.2014 */ );
  altitude     = bmp.readAltitude();
  moisture     = map(analogRead(MOISTURE_PIN), 0, 1024, 0, 99);

  
  output_string = "$";

  char tmp[10];
  float avg = avg_humidity.reading(humidity);
  dtostrf(avg,1,2,tmp);
  output_string.concat(tmp);
  output_string += ";";
  
  avg = avg_temperature1.reading(temperature1);
  dtostrf(avg,1,2,tmp);
  output_string.concat(tmp);
  output_string += ";";
  
  avg = avg_temperature2.reading(temperature2);
  dtostrf(avg,1,2,tmp);
  output_string.concat(tmp);
  output_string += ";";
  
  avg = avg_illumination.reading(illumination);
  dtostrf(avg,1,2,tmp);
  output_string.concat(tmp);
  output_string += ";";

  avg = avg_pressure1.reading(pressure1);
  dtostrf(avg/100,1,2,tmp);
  output_string.concat(tmp);
  output_string += ";";
  
  avg = avg_pressure2.reading(pressure2);
  dtostrf(avg/100,1,2,tmp);
  output_string.concat(tmp);
  output_string += ";";

  avg = avg_altitude.reading(altitude);
  dtostrf(avg,1,2,tmp);
  output_string.concat(tmp);
  output_string += ";";

  avg = avg_moisture.reading(moisture);
  dtostrf(avg,1,1,tmp);
  output_string.concat(tmp);
  output_string += "#";
  
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

  delay(100);
  current++;
}
//
// END OF FILE
//

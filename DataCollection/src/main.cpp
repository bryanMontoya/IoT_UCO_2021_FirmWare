#include <ESP8266WiFi.h>
#include <Arduino.h>
#include "HX711.h"
#include <math.h>
#include "mqtt/MQTTConnector.h"
#include "rest/Rest.h"
#include "Credentials.h"

HX711 balanza;
const int DOUT = 16;
const int CLK = 5;
double Peso;

double sensarPeso(){
  double Peso;    
  delay(3000);
  Serial.print("Peso: ");
  Peso = balanza.get_units(20);
  Peso = Peso*(1); //*(-1) en caso de poner la balanza al revés y estar recibiendo valores negativos.
  Serial.print(Peso);
  Serial.println(" gramos.");  
  return Peso;
}

void WiFiBegin(const char* ssid, const char* pass)
{
  WiFi.begin(ssid, pass);
  Serial.printf("Waiting for AP connection ...\n");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.printf(".");
  }
  IPAddress ip = WiFi.localIP();
  Serial.printf("IP : %d.%d.%d.%d\n", ip[0],ip[1],ip[2],ip[3]);
}

void setup() 
{
  Serial.begin(115200);  
  WiFiBegin(STA_SSID, STA_PASS);
  MQTTBegin();  
  balanza.begin(DOUT, CLK);
  Serial.print("Lectura del valor del ADC:  ");
  Serial.println(balanza.read());
  Serial.println("No ponga ningun  objeto sobre la balanza");
  Serial.println("Destarando...");
  Serial.println("...");  
  balanza.set_scale(406.3);  // Establecer la escala.
  balanza.tare(20);       
  Serial.println("Listo para pesar");  
}

void loop() 
{      
  MQTTLoop(); 
  //Llamada a api.
  String respuestaApi = apiRest("http://worldtimeapi.org/api/timezone/America/Bogota");  
  Peso = lround(sensarPeso()); //Aproximar peso al valor más cercano.    
  //Pubica el peso sensado en tópico MQTT.  
  String output = "{ \"item\" : \"diamantes\",\"weight\" : " + String( Peso ) + "," + " \"date\" : " + respuestaApi.substring(0, 10) + "," + "\"Hour\" : " + respuestaApi.substring(11, 16) + "}";
  MQTTPublish(TOPIC, (char *)output.c_str());  
}
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include "HX711.h"
#include <math.h>

#include "mqtt/MQTTConnector.h"
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
  Peso = Peso*(-1);
  Serial.print(Peso);
  Serial.println(" gramos.");  
  return Peso;
}

void calibracion(){  
  //Solo es necesario ejecutar cuando se debe realizar la calibración de la balanza.
    balanza.begin(DOUT, CLK);
    Serial.print("Lectura del valor del ADC:");
    Serial.println(balanza.read());
    Serial.println("No ponga ningún objeto sobre la balanza.");
    Serial.println("Destarando...");
    balanza.set_scale(); //La escala por defecto es 1
    balanza.tare(20);  //El peso actual es considerado Tara.
    Serial.println("Coloque un peso conocido:");

  while(1){
    Serial.print("Valor de lectura: ");
    Serial.println(balanza.get_value(10),0);
    delay(100);
  }  
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
  // Establecer la escala.
  balanza.set_scale(406.3); 
  balanza.tare(20);       //El peso actual es considerado Tara.  
  Serial.println("Listo para pesar");  
}

void loop() 
{  
  MQTTLoop(); 
  //Pubica el peso sensado en tópico MQTT.  
  Peso = lround(sensarPeso()); //Aproximar al más cercano.
  String output = "{ \"item\" : \"diamantes\", \"weight\": " + String( Peso ) + "}";  
  MQTTPublish(TOPIC, (char *)output.c_str());  
}

#include <WiFi.h>
#include <WebServer.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <NTPClient.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
using namespace std;
// Раскомментируйте одну из строк ниже в зависимости от того, какой датчик вы используете!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
/* Установите здесь свои SSID и пароль */
const char* ssid = "ilovedora";  // Enter SSID here
const char* password = "bossofthisgym";  //Enter Password here
WebServer server(80);
// DHT Sensor
uint8_t DHTPin = 25; 
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);     
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);           
float Temperature;
float Humidity;
int calculateE( int t );
int greatestCommonDivisor( int e, int t );
int calculateD( int e, int t );
int encrypt( int i, int e, int n );
int decrypt(int i, int d, int n );
String encoder(string msg);

String encoder(string msg){
    int p = 17, q = 19, n, t, e, d;
    
    int encryptedText[100];
    memset(encryptedText, 0, sizeof(encryptedText));
    
    int decryptedText[100];
    memset(decryptedText, 0, sizeof(decryptedText));
    
    n = p * q;
    
    t = ( p - 1 ) * ( q - 1 );
    
    e = calculateE( t );
    
    d = calculateD( e, t );
    
    //(n,e) is a public key
    //(n,d) is a private key
    
    
    // encryption
    
    for (int i = 0; i < msg.length(); i++)
    {
        encryptedText[i] = encrypt( msg[i], e, n);
    }
    
    
    
    String result = "";
    
    for (int i = 0; i < msg.length(); i++) {
        result+= String(encryptedText[i]) + " ";
    }
    
    return result;
}

int calculateE( int t )
{   
    int e;
    
    for ( e = 2; e < t; e++ )
    {
        if (greatestCommonDivisor( e, t ) == 1 )
        {
            return e;
        }
    }
    
    return -1;
}

int greatestCommonDivisor( int e, int t )
{
    while ( e > 0 )
    {
        int temp;
        
        temp = e;
        e = t % e;
        t = temp;
    }
    
    return t;
}

int calculateD( int e, int t)
{    
    int d;
    int k = 1;
    
    while ( 1 )
    {
        k = k + t;
        
        if ( k % e == 0)
        {
            d = (k / e);
            return d;
        }
    }
    
}


int encrypt(int i, int e, int n )
{
    int current, result;
    
    current = i - 97;
    result = 1;
    
    for ( int j = 0; j < e; j++ )
    {
        result = result * current;
        result = result % n;
    }
    
    return result;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(DHTPin, INPUT);
  dht.begin();              
  Serial.println("Connecting to ");
  Serial.println(ssid);
  //connect to your local wi-fi network
  WiFi.begin(ssid, password);
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);
  
  server.begin();
  Serial.println("HTTP server started");
  timeClient.begin();
  timeClient.setTimeOffset(10800);
}
void loop() {
  handle_OnConnect();
  delay(20000);

}
void handle_OnConnect() {
 Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  String formattedDate = timeClient.getFormattedDate();
  String ans = "";
  for (int i = 11; i < 16; ++i){
    ans += formattedDate[i];
  }
  SendHTML(Temperature, Humidity, ans); 
}

String SendHTML(float Temperaturestat,float Humiditystat, String DateStat){
    WiFiClient client;
    HTTPClient http;
    http.begin(client, "http://192.168.43.24:5000/upload");
    String httpRequestData = "{'type': 'temperature', 'value':'" + encoder(to_string(Temperaturestat)) + "', 'time':'" + DateStat + "', 'device':'0'}";
    int httpResponseCode = http.POST(httpRequestData);
    http.end();
    http.begin(client, "http://192.168.43.24:5000/upload");
    httpRequestData = "{'type': 'humidity', 'value':'" + encoder(to_string(Humiditystat)) + "', 'time':'" + DateStat + "', 'device':'0'}";
    httpResponseCode = http.POST(httpRequestData);
    http.end();
}
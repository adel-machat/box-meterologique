  #include "DHT.h"
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


#define PI 3.14
#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;
struct Aimant {
  const uint8_t PIN;
  unsigned long tempsDebut;
  unsigned long tempsFin;
  bool etat;
  bool etatPrecedent;
};

unsigned long duration = 0;

long lastMsg = 0;
int value = 0;

#include <Arduino.h>
#include <WiFi.h>


#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define API_KEY "AIzaSyA0FYxU3z1BsFNyrEHZwvflt6S9xQrqMro"
#define DATABASE_URL "https://connect-fe748-default-rtdb.firebaseio.com/" 
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

#define wifi_ssid "Orange-7959"
#define wifi_password "YBgnGtMadn8"

#include <PubSubClient.h>
#define mqtt_server "192.168.1.139"
WiFiClient espClient;
PubSubClient client(espClient);





Aimant anemo = {34, 0, 0, 0, 0};
float V,ve=0;

void IRAM_ATTR isr(){
  anemo.etatPrecedent = anemo.etat;
  anemo.etat = digitalRead(anemo.PIN);
  if (anemo.etat && !anemo.etatPrecedent) {
    anemo.tempsDebut = anemo.tempsFin;
    anemo.tempsFin = millis();
  }
}
void setup() {
   pinMode(DHTPIN, INPUT);
  dht.begin();

  Serial.begin(9600);
  pinMode(anemo.PIN, INPUT_PULLUP);   // interrupteur Reed à la pin 34
  attachInterrupt(anemo.PIN, isr, CHANGE);
    Wire.begin();
  lightMeter.begin();

   setup_wifi();
 
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

   client.setServer(mqtt_server, 1883);
   client.setCallback(callback);
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}


 void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266client")) {
      Serial.println("MQTT connected");
      client.subscribe("esp8266/dht11");
      Serial.println("Topic Subscribed");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // wait 5sec and retry
    }
  }
}

void loop() {
  int delai;
    char strbuf[50];
   char strbuf_2[50];
    char strbuf_3[50];
   char strbuf_4[50];
   
  if (!client.connected()) {
    reconnect();
  }

 
  if (anemo.etat && !anemo.etatPrecedent){
    
    Serial.println(anemo.tempsDebut);
    if (anemo.tempsDebut > 0){ 
      delai = anemo.tempsFin - anemo.tempsDebut;  // durée du tour qui vient de se terminer
      Serial.print("Periode :" );
      Serial.print(delai);
      V=3.6*1000*2*PI*0.1/delai;
      Serial.print("  miliseconsecondes,  Vitesse :"); 
      ve=(V*1000)/3600;   
      Serial.print(ve);
      Serial.println(" m/s");
   
     char vent[8];
    dtostrf(ve, 1, 2, vent);
    sprintf(strbuf, "{\"vent\": %s}", vent);
    Serial.println(strbuf);
    client.publish("esp32/vent", strbuf);

    
  if (Firebase.ready() && signupOK ) {
    
     Firebase.RTDB.setFloat(&fbdo, "/BOX2/vent",ve);

    } 
  }
    

 
    float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

     char lum[8];
    dtostrf(lux, 1, 2, lum);
    sprintf(strbuf_2, "{\"lum\": %s}", lum);
    Serial.println(strbuf_2);
    client.publish("esp32/lum", strbuf_2);

    
  if (Firebase.ready() && signupOK ) {
    
    Firebase.RTDB.setFloat(&fbdo, "/BOX2/lum",lux);
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();
    
     char hum[8];
    dtostrf(h, 1, 2, hum);
    sprintf(strbuf_3, "{\"hum\": %s}", hum);
    client.publish("esp32/hum", strbuf_3);
      
    char temp[8];
    dtostrf(t, 1, 2, temp);
    sprintf(strbuf_4, "{\"temp\": %s}", temp);
    client.publish("esp32/temp", strbuf_4);
  
  if (Firebase.ready() && signupOK ) {
    
    Firebase.RTDB.setFloat(&fbdo, " /BOX2/hum",h);
    Firebase.RTDB.setFloat(&fbdo, " /BOX2/temp", t);

    }
    
 
     

if((millis()- duration)>= 1000){ 
      duration = millis();
             
             Firebase.RTDB.getString(&fbdo,"esp/minutes");
             int seconds =0  ;
             int minutes =fbdo.stringData().toInt();
             minutes ++ ;
            Firebase.RTDB.setString(&fbdo, "/BOX2/minutes",(String) minutes);
            
    }


  

  }
}

  
  
  

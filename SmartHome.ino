
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

#define Relay1            D6
#define Relay2            D2
#define DHTPIN            D7
#define buzzer            D0
//analog pins multiplexing
#define S0                D3
#define S1                D4
#define analogpin         A0
#define WLAN_SSID       "yashram"
#define WLAN_PASS       "yashas winnie"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                  
#define AIO_USERNAME    "yashram"
#define AIO_KEY         "rdffasafrthb67ulivdaszxscvwin"
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish CO2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/co2");
Adafruit_MQTT_Publish Sound = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/sound");
Adafruit_MQTT_Publish Motion = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/motion");
Adafruit_MQTT_Publish Light = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/light");
Adafruit_MQTT_Subscribe Light1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay1");
Adafruit_MQTT_Subscribe Fan1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/relay2");

#define DHTTYPE           DHT11  
DHT dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
void MQTT_connect();
Servo myservo;
void setup() {
  Serial.begin(115200);

  delay(10);

  pinMode(buzzer, OUTPUT);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(A0, INPUT);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  dht.begin();
  mqtt.subscribe(&Light1);
  mqtt.subscribe(&Fan1);
 
}

uint32_t x = 0;

void loop() {
  
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &Light1) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light1.lastread);
      int Light1_State = atoi((char *)Light1.lastread);
      digitalWrite(Relay1, Light1_State);
      
    }
   
    if (subscription == &Fan1) {
      Serial.print(F("Got: "));
      Serial.println((char *)Fan1.lastread);
      int Fan1_State = atoi((char *)Fan1.lastread);
      digitalWrite(Relay3, Fan1_State);
    }
  
  }

  
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  Serial.print("Motion "); Serial.println(analogRead(analogpin));
  Serial.print("...");
  int Value = analogRead(analogpin);
  if(Value>400)
  Value=1;
  else
  Value=0;
  if (! Motion.publish(Value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  Serial.print("C02 "); Serial.println(analogRead(analogpin));
  Serial.print("...");
  Value = analogRead(analogpin);
  if (! CO2.publish(Value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  digitalWrite(S0, LOW);
  digitalWrite(S1, HIGH);
  Serial.print("Sound "); Serial.println(analogRead(analogpin));
  Serial.print("...");
  int raw_sound = analogRead(analogpin);
  Value = map(raw_sound,0,1024,0,100);
  if (! Sound.publish(Value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
  Serial.print("Light "); Serial.println(analogRead(analogpin));
  Serial.print("...");
  int raw_light = analogRead(analogpin);
  Value = map(raw_light,1024,0,0,100);
  if (! Light.publish(Value)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  

 
  if (isnan(h) || isnan(t)) {
    Serial.println("DHT Failed");
    return;
  }
  if (! Humidity.publish(h)) {
    Serial.println(F("Humidity Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  if (! Temperature.publish(t)) {
    Serial.println(F("Temperature Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("trying to connecting to MQTT... ");

  uint8_t retries = 3;
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
  while ((ret = mqtt.connect()) != 0) { 
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected");
  digitalWrite(buzzer, HIGH);
  delay(2000);
  digitalWrite(buzzer, LOW);
}

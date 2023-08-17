#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

//DHT11
#include "DHT.h"
#define DHTTYPE DHT11
#define DHTPIN D3
DHT dht(DHTPIN, DHTTYPE);


#include "ESP8266WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/********* WiFi Access Point ***********/

#define WLAN_SSID       "YJSANTY"         // User ID
#define WLAN_PASS       "Github"          // Pass

/********* Adafruit.io Setup ***********/

#define AIO_SERVER      "io.adafruit.com" // Web Side
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME     "ajit_natarajan"
#define AIO_KEY          "aio_ehoU39vpcylb22qNSTtS6Td4Nv7S"

/**** Global State (you don't need to change this!) ******/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/********** Feeds *************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish mqtt_humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish mqtt_temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish mqtt_ldr = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ldr");
Adafruit_MQTT_Publish mqtt_soil = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soil");
Adafruit_MQTT_Publish mqtt_ir = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ir");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/button");

/********* Message and Call Setup *********/

#define rxPin 2
#define txPin 3
SoftwareSerial sim800L(rxPin,txPin); 

String buff;

/******** Other Declarations ************/

void MQTT_connect();
#define LEDPIN D4
const int ADC_pin = A0;
const int IR = 7;

/********* Sketch Code ************/

struct {
  float temp = 0.00;
  float humi = 0.00;
  int   ldr  = 0;
  int   soil = 0;
  float   ir = 0.00;
} data;


void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

uint32_t x = 0;
int testVariable = 0;
void mqtt_code() {
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      if (0 == strcmp((char *)onoffbutton.lastread, "0")) {
        digitalWrite(LEDPIN, HIGH);
      }
      if (0 == strcmp((char *)onoffbutton.lastread, "1")) {
        digitalWrite(LEDPIN, LOW);
      }
    }
  }

  //Send Serial
  if (1) {
    for (int i = 0; i < 13; i++) Serial.print("-"); Serial.print("Data");
    for (int i = 0; i < 13; i++) Serial.print("-"); Serial.println();
    Serial.print("Humidity:"); Serial.println(data.humi);
    Serial.print("Temperature:"); Serial.println(data.temp);
    Serial.print("LDR:"); Serial.println(data.ldr);
    Serial.print("Soil:"); Serial.println(data.soil);
    Serial.print("IR_DIS:"); Serial.println(data.ir);
    for (int i = 0; i < 30; i++) Serial.print("-"); Serial.println();
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IR");
  lcd.print(data.ir);
  lcd.print("%");
  lcd.setCursor(8, 0);
  lcd.print("T");
  lcd.print(data.temp);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("L:");
  lcd.print(data.ldr);
  lcd.setCursor(8, 1);
  lcd.print("S:");
  lcd.print(data.soil); lcd.print("%");


  mqtt_humidity.publish(data.humi);    delay(1000);
  mqtt_temperature.publish(data.temp); delay(1000);
  mqtt_ldr.publish(data.ldr);          delay(1000);
  mqtt_soil.publish(data.soil);        delay(1000);
  mqtt_soil.publish(data.ir);          delay(1000);
}

#define WLAN_SSID       "alpha" // User ID
#define WLAN_PASS       "12345678"          // Pass


void read_dht() {
  data.temp = dht.readTemperature();
  data.humi = dht.readHumidity();
}

void read_ldr() {
  data.ldr = digitalRead(D5);
}

void read_soil() {

  data.soil = map(analogRead(A0), 0, 1023, 100, 0);
}

void read_IR(){
  int IRState = digitalRead(IR);
  Serial.println(IRState);
}


void send_sms(){
  sim800L.print("AT+CMGS=\"+917871926105\"\r");
  waitForResponse();
  
  sim800L.print("Alert! This is from your IoT device  publishing a emergency message ,kindly check the cloud dashboard about for accesing of the information and take neccesary actions ");
  sim800L.write(0x1A);
  waitForResponse();
}


void make_call(){
  sim800L.println("ATD+917871926105;");
  waitForResponse();
}

void waitForResponse(){
  delay(1000);
  while(sim800L.available()){
    Serial.println(sim800L.readString());
  }
  sim800L.read();
}

void message_call(){
    while(sim800L.available()){
    buff = sim800L.readString();
    Serial.println(buff);
  }
  while(Serial.available())  {
    // buff = Serial.readString();
    // buff.trim();
    if(data.ir == 10.00)
      send_sms();
    else if(data.ir == 0.00)
      make_call();
    else
      sim800L.println(buff);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println(); Serial.println();
  lcd.begin(16,2);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Program Start");
  delay(2000);
  Serial.println("Program Started");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  Serial.print("Connecting to ");

  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  mqtt.subscribe(&onoffbutton);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected");
  pinMode(LEDPIN, OUTPUT); digitalWrite(LEDPIN, HIGH);

  dht.begin();
  pinMode(D5, INPUT);
  pinMode(A0, INPUT);
  pinMode(IR, INPUT);

  
  
  //Begin serial communication with Arduino and SIM800L
  sim800L.begin(115200);

  Serial.println("Initializing Message connection...");
  lcd.setCursor(0, 0);
  lcd.print("Initializing Message connection...");
  sim800L.println("AT");
  waitForResponse();

  sim800L.println("ATE1");
  waitForResponse();

  sim800L.println("AT+CMGF=1");
  waitForResponse();

  sim800L.println("AT+CNMI=1,2,0,0,0");
  waitForResponse();
  lcd.setCursor(0, 0);
  lcd.print("Messaging");
  lcd.setCursor(0, 1);
  lcd.print("Connected");

}

void loop() {
  mqtt_code();
  read_dht();
  read_soil();
  read_ldr();
  read_IR();
  message_call();
  //delay(20000);
}
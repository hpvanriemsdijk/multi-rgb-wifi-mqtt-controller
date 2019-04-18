/*
 * RGB controller, supporting:  
 *  OTA
 *  Two adressable ledstrips
 *  Chainable P9813 controller
 *
 * Controllable with MQTT over wifi
 * following Homeassistant MQTT Light JSON protocol
 * Using the great fastled libary
 * Written for the Wemos D1 mini 
 * 
 */
#include <ArduinoJson.h>  // For JSON decoding
#include <ESP8266WiFi.h>  // For ESP8266
#include <PubSubClient.h> // For MQTT
#include <ESP8266mDNS.h>  // For OTA
#include <WiFiUdp.h>      // For OTA
#include <ArduinoOTA.h>   // For OTA
#include <FastLED.h>      // RGB stuff

//Pin assesment
#define DATA_PIN_1   D4
#define CLK_PIN_1    D3
#define DATA_PIN_2   D8
#define CLK_PIN_2    D7
#define DATA_PIN_3   D5
#define CLK_PIN_3    D6

// Let strip config 
// Digital strips
#define LED_TYPE_1    APA102
#define COLOR_ORDER_1 BGR
#define NUM_LEDS_1    10          //10 in test, 83 in production
CRGB leds_left[NUM_LEDS_1];
CRGB leds_right[NUM_LEDS_1];

// Let strip config 
// P9813 pixels (analog stripts)
#define LED_TYPE_2    P9813
#define COLOR_ORDER_2 GRB
#define NUM_LEDS_2    6
CRGB leds_analog[NUM_LEDS_2];

// Let strip config 
// Other
#define FRAMES_PER_SECOND  120

// Holders for received values
CRGB color = {255, 0, 0};         // Color 
uint8_t brightness = 130;         // 0-255
boolean state = false;            

/* 
 *  WiFi
 *  credentials.h has to be in your libary folder. its content is:
 *  #define mySSID "yourSSID"
 *  #define myMqttServer "your MQTT server IP"
 *  #define myMqttUID "Your MQTT user name "
 *  #define myMqttPass "your MQTT Password"
 *  #define myMqttTopic "Base topic"
 *  If you do not want this file, hard-code your credentials in this sketch
 */
#include "credentials.h";
const char* ssid = mySSID;
const char* password = myPASSWORD;
WiFiClient espClient;

//MQTT
PubSubClient client(espClient);
const char* mqtt_server = myMqttServer;
const char* mqtt_uid = myMqttUID;
const char* mqtt_pass = myMqttPass;
const String mqtt_base_topic = myMqttTopic;
const String stat_topic = "stat/" + mqtt_base_topic;
const String cmnd_topic = "cmnd/" + mqtt_base_topic;
String clientId;

//OTA
WiFiServer TelnetServer(8266);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message)) {
    return;
  }
  
  sendState();
}

bool processJson(char* message) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.println("JSON error");
    return false;
  }
  
  JsonObject root = doc.as<JsonObject>();

  if(root.containsKey("color")){
    color.r = constrain(root["color"]["r"],0,255);
    color.g = constrain(root["color"]["g"],0,255);
    color.b = constrain(root["color"]["b"],0,255);
  }

  if(root.containsKey("brightness")){
    brightness = constrain(doc["brightness"],0,255);
    FastLED.setBrightness(brightness);
  }

  if(root.containsKey("state")){
    if(doc["state"] == "ON"){ state = true; }
    if(doc["state"] == "OFF"){ state = false; }
  }
  
  return true;
}

void sendState() {
  String msg;
  StaticJsonDocument<256> doc;

  if(state){ 
    doc["state"] = "ON";
  }else{
    doc["state"] = "OFF";
  }
  
  doc["brightness"] = brightness;

  JsonObject docColor = doc.createNestedObject("color");
  docColor["r"] = color.r;
  docColor["g"] = color.g;
  docColor["b"] = color.b;

  serializeJson(doc, msg);
    
  client.publish((stat_topic).c_str(),(msg).c_str(), false);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_uid, mqtt_pass,(stat_topic).c_str(), 0, true, "OFF")) {
      Serial.println("Connected with id: " + clientId );
      client.subscribe((cmnd_topic+"/#").c_str());
      Serial.println("subscribed to " + cmnd_topic+"/#" );
      client.publish((stat_topic).c_str(), "ON", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\r\nBooting...");
  
  setup_wifi();

  Serial.print("Configuring OTA device...");
  TelnetServer.begin();   //Necesary to make Arduino Software autodetect OTA device  
  ArduinoOTA.onStart([]() {Serial.println("OTA starting...");});
  ArduinoOTA.onEnd([]() {Serial.println("OTA update finished!");Serial.println("Rebooting...");});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {Serial.printf("OTA in progress: %u%%\r\n", (progress / (total / 100)));});  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OK");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("");

  FastLED.addLeds<LED_TYPE_1,DATA_PIN_1,CLK_PIN_1,COLOR_ORDER_1>(leds_left, NUM_LEDS_1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE_1,DATA_PIN_2,CLK_PIN_2,COLOR_ORDER_1>(leds_right, NUM_LEDS_1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE_2,DATA_PIN_3,CLK_PIN_3,COLOR_ORDER_2>(leds_analog, NUM_LEDS_2).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
}

void loop() {
  //OTA
  ArduinoOTA.handle();

  //MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Led
  if(!state){ //Off
    FastLED.clear();
  }else{ //On
    fill_solid(leds_left, NUM_LEDS_1, CRGB(color)); 
    fill_solid(leds_right, NUM_LEDS_1, CRGB(color)); 
    fill_solid(leds_analog, NUM_LEDS_2, CRGB(color));     
  }

  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

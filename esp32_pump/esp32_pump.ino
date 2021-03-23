#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "CN - Cisco";
const char* password = "stifstof1";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Update these with values suitable for your network.

const char* mqtt_server = "159.89.3.14";
#define mqtt_port 1883
//#define MQTT_USER "eapcfltj"
//#define MQTT_PASSWORD "3EjMIy89qzVn"
#define MQTT_SERIAL_PUBLISH_CH "waterack/1"
#define MQTT_SERIAL_RECEIVER_CH "watercmd/1"

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) { //,MQTT_USER,MQTT_PASSWORD)
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish(MQTT_SERIAL_PUBLISH_CH, "Reconnected...");
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendWithReconnect(const char* topic, const char* msg) {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish(topic, msg);
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();

    // handle message
    char payload_string[length + 1];
    int result;
    
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    result = atoi(payload_string);
    
    int quantity = result;
    pump_cycle(quantity * 1000, payload_string);
}


// constants won't change. Used here to set a pin number:
const int ledPin =  32;// the number of the LED pin

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 3000;           // interval at which to blink (milliseconds)

void pump_cycle(long intval, const char* payload_string) {
  if (intval > 0 && intval < 4000) {
   
//    unsigned long currentMillis = millis();
//  
//    while (currentMillis - previousMillis >= interval) {
//      Serial.println(ledState);
//      // save the last time you blinked the LED
//      previousMillis = currentMillis;
//  
//      // if the LED is off turn it on and vice-versa:
//      if (ledState == LOW) {
//        ledState = HIGH;
//      } else {
//        ledState = LOW;
//      }
//  
//      // set the LED with the ledState of the variable:
//      digitalWrite(ledPin, ledState);
//      currentMillis = millis();
//    }

    ledState = HIGH;
    digitalWrite(ledPin, ledState);
    delay(intval);
    ledState = LOW;
    digitalWrite(ledPin, ledState);

    String str =  String("WE DID IT!!!") + " (" + String(payload_string) + ")";
    client.publish(MQTT_SERIAL_PUBLISH_CH, str.c_str());
    
  } else {
    String str =  String("ERROR: WE DIDNT DO IT!!!") + " (" + String(payload_string) + ")" + "Limit was out of bounds";
    client.publish(MQTT_SERIAL_PUBLISH_CH, str.c_str());
  }
}


void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

void loop()
{
  client.loop();
  delay(2000);
}

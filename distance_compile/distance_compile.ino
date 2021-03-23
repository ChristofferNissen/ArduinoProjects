/* HC-SR04 ultrasonic distance sensor with NewPing library example code. More info: www.makerguides.com */
// Include the library:
#include <NewPing.h>

// Define pins and max distance:
#define trigPin  32
#define echoPin  33
#define MAX_DISTANCE 350 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar(trigPin, echoPin, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
float duration, distance;


#include <SPI.h>
#include <WiFi.h>
const char* ssid = "CN - Cisco";
const char* password = "stifstof1";


#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Update these with values suitable for your network.

const char* mqtt_server = "159.89.3.14";
#define mqtt_port 1883
#define MQTT_USER "eapcfltj"
#define MQTT_PASSWORD "3EjMIy89qzVn"
#define MQTT_SERIAL_PUBLISH_CH "/icircuit/ESP32/serialdata/tx"
#define MQTT_SERIAL_RECEIVER_CH "Resovoir-Water-Bottom-Distance"

String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
  }
}
 
void scanNetworks() {
  int numberOfNetworks = WiFi.scanNetworks();
  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);
 
  for (int i = 0; i < numberOfNetworks; i++) {
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
 
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
 
    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));
 
    Serial.print("Encryption type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("-----------------------");
 
  }
}
 
void connectToNetwork() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
  }
  Serial.println("Connected to network");
  Serial.println(ssid);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffdd), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish("/icircuit/presence/ESP32/", "hello world");
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
}

void publishSerialData(const char *ch, char *serialData){
//  if (!client.connected()) {
//  reconnect();
//  }
  client.publish(ch, serialData);
}

void convertValuesAndPublish(String topic_str, float value){
  char data[32];
  sprintf(data, "%f", value);
  publishSerialData(topic_str.c_str(), data);
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for 

  // setup_wifi();
  scanNetworks();
  connectToNetwork();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

}

void loop()
{
  client.loop();

//  digitalWrite(trigPin, LOW);
  delay(100); // Wait 100ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
//  
//  duration = sonar.ping();
  int iterations = 100;
  duration = sonar.ping_median(iterations);
  distance = (duration / 2.0) * 0.0343;
  
  Serial.print("Distance = ");
  Serial.print(distance); // Distance will be 0 when out of set max range.
  Serial.println(" cm");

  convertValuesAndPublish("Tray-Water-Bottom-Distance", distance);

  delay(10000);                                    //recomended polling frequency 8sec..30sec
}

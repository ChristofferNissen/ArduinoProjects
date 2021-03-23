#include <SPI.h>
#include <WiFi.h>
const char* ssid = "CN - Cisco";
const char* password = "stifstof1";

#include <Wire.h>
#include <AHT10.h>
#include <driver/adc.h>


AHT10             myAHT10(AHT10_ADDRESS_0X38);

#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Update these with values suitable for your network.

const char* mqtt_server = "159.89.3.14";
#define mqtt_port 1883
#define MQTT_USER "eapcfltj"
#define MQTT_PASSWORD "3EjMIy89qzVn"
#define MQTT_SERIAL_PUBLISH_CH "/icircuit/ESP32/serialdata/tx"
#define MQTT_SERIAL_RECEIVER_CH "/icircuit/ESP32/serialdata/rx"

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
  if (!client.connected()) {
    reconnect();
  }
  client.publish(ch, serialData);
}

void convertValuesAndPublish(String topic_str, float value){
  char data[20];
  sprintf(data, "%f", value);
//  char topic[30];
//  sprintf(topic, "%s", topic_str);
//  Serial.println(data);
//  Serial.println(topic_str.c_str());
  publishSerialData(topic_str.c_str(), data);
}

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
 
}



void setup()
{
  Serial.begin(115200);

  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  pinMode(34,INPUT);
  pinMode(35,INPUT);

  /* AHT10 connection check */
  while (myAHT10.begin() != true)
  {
    Serial.print(F("AHT10 error"));                   //(F()) saves string to flash & keeps dynamic memory free
    delay(5000);
  }

  Wire.setClock(400000);                           //experimental I2C speed! 400KHz, default 100KHz

//  scanNetworks();
//  connectToNetwork();
//  Serial.println(WiFi.localIP());
  
}

void loop()
{
  client.loop();

  float temp = myAHT10.readTemperature();             //read 6 bytes over I2C
  if (temp != AHT10_ERROR)
  {
    Serial.println(String("") + "Temperature " + temp + "°C");
  }
  else
  {
    Serial.println(F("i2c error  "));
  } 
  convertValuesAndPublish("AHT10-Temp-C", temp);
  delay(1000); 
  
  /* prints dynamic humidity data */
  float humidity = myAHT10.readHumidity();                //read 6 bytes over I2C
  if (humidity != AHT10_ERROR)
  {
    Serial.println(String("") + "Humidity " + humidity + "%");
  }
  else
  {
    Serial.println(F("i2c error  "));
  }
  convertValuesAndPublish("AHT10-Humidity-Percentage", humidity);
  delay(1000); 
  
  float sensorVoltage; 
  float sensorValue;
 
  //  sensorValue = analogRead(35);
  adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_0);
  sensorValue = adc1_get_raw(ADC1_CHANNEL_7);
  sensorVoltage = sensorValue/1024*5.0;
  Serial.print("GUVA S12D sensor reading = ");
  Serial.print(sensorValue);
  Serial.println("");
  Serial.print("GUVA S12D sensor voltage = ");
  Serial.print(sensorVoltage);
  Serial.println(" V");
  convertValuesAndPublish("GUVA-UV", sensorValue);
  convertValuesAndPublish("GUVA-UV-V", sensorVoltage);
  delay(1000); 
  
  //  sensorValue = analogRead(34);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
  sensorValue = adc1_get_raw(ADC1_CHANNEL_6);
  sensorVoltage = sensorValue/1024*5.0;
  Serial.print("TEMPT6000 sensor reading = ");
  Serial.print(sensorValue);
  Serial.println("");
  Serial.print("TEMPT6000 sensor voltage = ");
  Serial.print(sensorVoltage);
  Serial.println(" V");
  convertValuesAndPublish("TEMPT6000-Light", sensorValue);
  convertValuesAndPublish("TEMPT6000-Light-V", sensorVoltage);
  delay(1000); 

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4,ADC_ATTEN_DB_0);
  sensorValue = adc1_get_raw(ADC1_CHANNEL_4);
  sensorVoltage = sensorValue/1024*5.0;
  Serial.print("TEMPT6000 sensor 2 reading = ");
  Serial.print(sensorValue);
  Serial.println("");
  Serial.print("TEMPT6000 sensor 2 voltage = ");
  Serial.print(sensorVoltage);
  Serial.println(" V");
  convertValuesAndPublish("TEMPT6000-Light-2", sensorValue);
  convertValuesAndPublish("TEMPT6000-Light-2-V", sensorVoltage);

  delay(10000);                                    //recomended polling frequency 8sec..30sec
}

#include <driver/adc.h>

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(34,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
  float sensorValue = adc1_get_raw(ADC1_CHANNEL_6);
  Serial.println(sensorValue);
  delay(1000);
  

}

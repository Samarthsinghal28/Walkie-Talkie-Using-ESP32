#include <driver/i2s.h>
#include "DFrobot_MSM261.h"
#include <esp_now.h>
#include <WiFi.h>
#include <MusicDefinitions.h>
#include <XT_DAC_Audio.h>


#define BUTTON_PIN 21
#define SAMPLE_RATE     (44100)
#define I2S_SCK_IO      (20)
#define I2S_WS_IO       (14)
#define I2S_DI_IO       (21)
#define DATA_BIT        (16)
#define MODE_PIN        (17)

// Variables will change:
int lastState = LOW;  // the previous state from the input pin
int currentState;     // the current reading from the input pin

//{0x24, 0x6F, 0x28, 0x22, 0x8E, 0x44}
//{0x24, 0x6F, 0x28, 0x21, 0xFD, 0x84}
uint8_t peer_mac[] = {0x24, 0x6F, 0x28, 0x21, 0xFD, 0x84};

esp_now_peer_info_t peer_info;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  Serial.println("Received data from peer");

  // Assuming the data received is of type int16_t (16-bit audio samples)
  int num_samples = len / sizeof(int16_t);
  int16_t *audio_data = (int16_t *)data;
  const unsigned char * sam = (const unsigned char *)(data[2]|data[3]<<8);
  // XT_Wav_Class Sound(sam);                                      
  // XT_DAC_Audio_Class DacAudio(25,0);
  // uint32_t DemoCounter=0;
  Serial.println((int16_t)(data[2]|data[3]<<8));
  // DacAudio.FillBuffer();
  // if(Sound.Playing==false)        
  //   DacAudio.Play(&Sound);       
}


DFRobot_Microphone microphone(I2S_SCK_IO, I2S_WS_IO, I2S_DI_IO);
char i2sReadrawBuff[100];
void setup() {
  Serial.begin(115200);//Serial rate 115200

  WiFi.mode(WIFI_STA);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_PIN,OUTPUT);
  digitalWrite(MODE_PIN,LOW);//Configure the microphone as receiving data of left channel
  while(microphone.begin(SAMPLE_RATE, DATA_BIT) != 0){
      Serial.println(" I2S init failed");//Init failed
  }
  Serial.println("I2S init success");//Init succeeded

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Register the peer
  memcpy(peer_info.peer_addr, peer_mac, sizeof(peer_mac));
  peer_info.channel = 0;
  peer_info.encrypt = false;
  
  if (esp_now_add_peer(&peer_info) != ESP_OK)
   {
    Serial.println("Failed to add peer");
    return;
  }
  else{
    Serial.println("Peer Adder");
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  currentState = digitalRead(BUTTON_PIN);
  if (lastState == LOW && currentState == LOW){
  microphone.read(i2sReadrawBuff,100);
  //Output data of left channel
  Serial.println((int16_t)(i2sReadrawBuff[2]|i2sReadrawBuff[3]<<8));
  //Serial.println((int16_t)(i2sReadrawBuff[3]));
  delay(100);
  if (esp_now_send(peer_mac, (uint8_t*)i2sReadrawBuff, 100) != ESP_OK)
   {
    Serial.println("Error sending data");
  }

  esp_now_register_send_cb(OnDataSent);
  }
  
  
}
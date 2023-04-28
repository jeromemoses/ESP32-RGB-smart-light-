#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif

#define FASTLED_ESP8266_DMA
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include "SinricPro.h"
#include "SinricProLight.h"

#include <WiFiManager.h>
WiFiManager my_wifi;

//#define WIFI_SSID         "YOUR-WIFI-SSID"    
//#define WIFI_PASS         "YOUR-WIFI-PASSWORD"
#define APP_KEY           "7a6855c7-247d-4403-b7f0-5680bcd57d0d"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "d2deb958-a406-4f47-a032-1a0d1032b336-a553a24c-f345-4f4c-949b-30dc04803503"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define LIGHT_ID          "642fec41918a3c911c76f7dd"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define BAUD_RATE         115200                // Change baudrate to your need

#define NUM_LEDS          10                   // how much LEDs are on the stripe
#define LED_PIN           4                   // LED stripe is connected to PIN 3

bool powerState;        
int globalBrightness = 100;
CRGB leds[NUM_LEDS];

bool onPowerState(const String &deviceId, bool &state) {
  powerState = state;
  if (state) {
    FastLED.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  } else {
    FastLED.setBrightness(0);
  }
  FastLED.show();
  Serial.printf("device: %s \n",state ? "on" : "off");
  return true; // request handled properly
}

bool onBrightness(const String &deviceId, int &brightness) {
  globalBrightness = brightness;
  FastLED.setBrightness(map(brightness, 0, 100, 0, 255));
  FastLED.show();
  Serial.printf("device brightness: %d \n",brightness);
  return true;
}

bool onAdjustBrightness(const String &deviceId, int brightnessDelta) {
  globalBrightness += brightnessDelta;
  brightnessDelta = globalBrightness;
  FastLED.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  FastLED.show();
  Serial.printf("device brightness: %d \n",brightnessDelta);
  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
  fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
  FastLED.show();
  Serial.print("device colour: ");
  Serial.print(r);
  Serial.print(",");
  Serial.print(g);
  Serial.print(",");
  Serial.println(b);
  return true;
}

void setupFastLED() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  
  my_wifi.setTimeout(150);  // hotspot visibility for 3 minutes
  my_wifi.autoConnect("TTP_ESP32","TTP_ESP32");

  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", localIP.toString().c_str());
}

void setupSinricPro() {
  // get a new Light device from SinricPro
  SinricProLight &myLight = SinricPro[LIGHT_ID];

  // set callback function to device
  myLight.onPowerState(onPowerState);
  myLight.onBrightness(onBrightness);
  myLight.onAdjustBrightness(onAdjustBrightness);
  myLight.onColor(onColor);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  //SinricPro.restoreDeviceStates(true); // Uncomment to restore the last known state from the server.
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  setupFastLED();
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
}
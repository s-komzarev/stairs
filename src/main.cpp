#include <Arduino.h>
#include <Ultrasonic.h>
#include "GyverTimer.h"
#include "ESP8266WiFi.h"
#include "ESP8266mDNS.h"
#include "WiFiUdp.h"
#include "ArduinoOTA.h"
#include "stairs/Stairs.h"
// Replace with your network credentials
const char *ssid = "ksp";
const char *password = "we11come2ujcnm";

//кол-во ступеней

const u_int8_t PIN = D3; // пин ленты

const u_int8_t PIN_TRIG = D7;  // ПИН D7
const u_int8_t PIN_ECHO = D6;  // ПИН D6
const u_int8_t PIN_TRIG1 = D0; // ПИН D7
const u_int8_t PIN_ECHO1 = D1; // ПИН D6
const u_int32_t delaySensors = 250;
// const u_int16_t LED_COUNT = STEP_COUNT * STEP_LEN;
u_int32_t lastReadSensors = 0;
u_int16_t cm1, cm2;
u_int8_t distance = 90;
Ultrasonic ultrasonic1(PIN_TRIG, PIN_ECHO);
Ultrasonic ultrasonic2(PIN_TRIG1, PIN_ECHO1);
uint32_t timeoutCounter;
GTimer _timer_time_out = GTimer(MS, 5 * 1000);
Stairs stairs = Stairs(18, 22, 500, 10, PIN);
// bool systemIdleState;
// bool systemOffState;
struct PirSensor
{
  u_int8_t effectDirection;
  u_int8_t pin_t;
  u_int8_t pin_e;
  bool lastState;
};
bool isTop = true;
GTimer timerAnimation(MS, 1000);
GTimer timerSensor(MS);

PirSensor startPirSensor = {1, PIN_TRIG, PIN_ECHO, false};
PirSensor endPirSensor = {0, PIN_TRIG1, PIN_ECHO1, false};
// Adafruit_NeoPixel _strip = Adafruit_NeoPixel(10*8, PIN, NEO_GRB + NEO_KHZ800);
void setup()
{
  _timer_time_out.stop();
  _timer_time_out.reset();
  Serial.begin(115200); // Устанавливаем сокорость соединения с последовательным портом
                        //  _strip.begin();
                        //  _strip.setBrightness(100);
                        //  _strip.setPixelColor(0, _strip.Color(255, 231, 250));
                        //  _strip.show();
                        //     delay(10000);
  Serial.println("Booting");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.setHostname("Stairs");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]()
                     { Serial.println("Start"); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
                       Serial.printf("Error[%u]: ", error);
                       if (error == OTA_AUTH_ERROR)
                         Serial.println("Auth Failed");
                       else if (error == OTA_BEGIN_ERROR)
                         Serial.println("Begin Failed");
                       else if (error == OTA_CONNECT_ERROR)
                         Serial.println("Connect Failed");
                       else if (error == OTA_RECEIVE_ERROR)
                         Serial.println("Receive Failed");
                       else if (error == OTA_END_ERROR)
                         Serial.println("End Failed");
                     });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void readSonic()
{
  unsigned long mil = millis();

  if (mil - lastReadSensors > delaySensors)
  {
    lastReadSensors = mil;
    cm1 = ultrasonic1.read();
    cm2 = ultrasonic2.read();

    if (cm1 < distance)
    {
      Serial.print("==============================================================Sensor 01: ");
      Serial.print(cm1); // Prints the distance on the default unit (centimeters)
      Serial.println("cm");
      stairs.ShowAllStepsFromTop();
      isTop = false;
      _timer_time_out.reset();
      _timer_time_out.start();
    }
    if (cm2 < distance)
    {
      Serial.print("Sensor 02: ========================================== ");
      Serial.print(cm2); // Prints the distance on the default unit (centimeters)
      stairs.ShowAllStepsFromDown();
      isTop = true;
       _timer_time_out.reset();
      _timer_time_out.start();

      Serial.println("cm");
    }
  }
}

void loop()
{
  readSonic();
  // stairs.tick();
  
  if (_timer_time_out.isReady())
  {
    _timer_time_out.stop();
    _timer_time_out.reset();
    if(isTop) {
      Serial.println("From top");
      stairs.ClearAllStepsFromTop();
    } else {
      stairs.ClearAllStepsFromDown();
      Serial.println("From down");
    }
    Serial.println("_timer_time_out");
    stairs.NightLight();
  }
  ArduinoOTA.handle();
}

#include <Arduino.h>
#include <Ultrasonic.h>
#include "GyverTimer.h"
#include "ESP8266WiFi.h"
#include "ESP8266mDNS.h"
#include "WiFiUdp.h"
#include <PubSubClient.h>
#include "ArduinoOTA.h"
#include <string>
#include "Adafruit_NeoPixel.h"
// Replace with your network credentials
const char *ssid = "ksp";
const char *password = "we11come2ujcnm";

const u_int8_t PIN = D8; // пин ленты

const u_int8_t PIN_TRIG = D7;  // ПИН D7
const u_int8_t PIN_ECHO = D6;  // ПИН D6
const u_int8_t PIN_TRIG1 = D0; // ПИН D7
const u_int8_t PIN_ECHO1 = D1; // ПИН D6
const u_int32_t delaySensors = 250;
bool _inProgress;
// const u_int16_t LED_COUNT = STEP_COUNT * STEP_LEN;
u_int32_t lastReadSensors = 0;
u_int16_t cm1, cm2;
u_int8_t distance = 90;
int8_t _current_step = -1;
Ultrasonic ultrasonic1(PIN_TRIG, PIN_ECHO);
Ultrasonic ultrasonic2(PIN_TRIG1, PIN_ECHO1);
uint32_t _time_out_sec = 10;
u_int16_t attempt_count = 0;
GTimer _timer_time_out = GTimer(MS, _time_out_sec * 1000);
GTimer _timer_connection = GTimer(MS, _time_out_sec * 5000);
GTimer _timer_change_nightlight = GTimer(MS, 60 * 1000);
GTimer _timer_live = GTimer(MS, 10 * 1000);
u_int16_t _step_speed = 200;
GTimer _timer_stairs = GTimer(MS, _step_speed);
bool isTop = true;
u_int8_t _step_count = 18;
u_int8_t _led_in_step = 22;
u_int32_t _led_count = _step_count * _led_in_step;
GTimer timerSensor(MS, 100);
Adafruit_NeoPixel _strip = Adafruit_NeoPixel(_led_count, PIN, NEO_GRB + NEO_KHZ800);
u_int32_t _current_color = _strip.Color(255, 255, 60);
u_int8_t nightLightShift[4] = {0, _led_in_step / 2, _led_in_step / 2 - 1, _led_in_step - 1};

const char *mqtt_broker = "192.168.1.5";
const char *topic_stair_color = "stairs/settings/color";
const char *topic_stair_delay = "stairs/settings/delay";
const char *topic_stair = "stairs/settings/work_state";
const char *topic_stair_color_state = "stairs/settings/colorstate";
const char *topic_stair_delay_state = "stairs/settings/delaystate";
const char *topic_stair_state = "stairs/settings/work_statestate";
const char *topic_stair_avaliable = "stairs/settings/avaliable/state";
const char *mqtt_username = "kspMqtt";
const char *mqtt_password = "we11come2ksp";
const int mqtt_port = 1883;
bool _is_disable = false;
WiFiClient espClient;
PubSubClient client(espClient);

void NightLight()
{
  if (_is_disable)
    return;
  _strip.clear();
  Serial.print("NightLight: ");
  for (int i = 0; i < 4; i++)
  {
    _strip.setPixelColor(nightLightShift[i], _current_color);
    _strip.setPixelColor(_led_in_step * (_step_count - 1) + nightLightShift[i], _current_color);
  }
  _timer_change_nightlight.start();
  _strip.show();
}

void randomShow()
{
  _strip.clear();
  _strip.show();
  int16_t j = 0;
  uint32_t color = _strip.Color(255, 0, 0);
  for (u_int16_t i = 0; i < _step_count; i++)
  {
    _strip.setPixelColor(j, color);
    Serial.println(j);
    _strip.show();
    if (i % 2 == 0)
    {
      j = j + _led_in_step * 2 - 1;
    }
    else
    {
      j = j + 1;
    }
    delay(150);
    yield();
  }
  j = j - _led_in_step;
  Serial.println("===================================");
  for (int32_t i = _step_count - 1; i >= 0; i--)
  {
    _strip.setPixelColor(j, color);
    Serial.println(j);
    _strip.show();
    if (i % 2 == 0)
    {
      j = j - _led_in_step * 2;
    }
    else
    {
      j = j - 1;
    }
    delay(300);
    yield();
  }
  Serial.println("===================================");
  NightLight();
}

void publishState()
{
  char textToWrite[16];
  char timeout[16];
  sprintf(textToWrite, "%lu", _current_color);
  sprintf(timeout, "%lu", _time_out_sec);
  client.publish(topic_stair_color_state, textToWrite);

  client.publish(topic_stair_delay_state, timeout);
  client.publish(topic_stair_state, _is_disable ? "OFF" : "ON");
}

void ping()
{
  _strip.setPixelColor(22, _strip.Color(255, 255, 255));
  delay(50);
  _strip.setPixelColor(22, 0);
  delay(50);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String message;
  ping();
  for (int i = 0; i < length; i++)
  {
    message = message + (char)payload[i]; // convert *byte to string
  }
  Serial.print(message);
  Serial.println();
  Serial.println("-----------------------");
  if (String(topic_stair_delay).equals(topic))
  {
    _time_out_sec = atoi(message.c_str());
    publishState();
    return;
  }
  if (String(topic_stair).equals(topic))
  {
    if (message == "ON")
    {
      _is_disable = false;
      publishState();
      NightLight();
      return;
    }
    if (message == "OFF")
    {
      _is_disable = true;
      publishState();
      _strip.clear();
      _strip.show();
      return;
    }
  }

  if (message == "demo")
  {
    _strip.clear();
    _strip.show();
    randomShow();
    return;
  }
  // publishState();
}

void demo()
{
  for (u_int16_t i = 0; i < _led_count; i++)
  {
    _strip.setPixelColor(i, _strip.Color(random(255), random(255), random(255)));
    if (i > 5)
    {
      _strip.setPixelColor(i - 5, 0);
    }
    _strip.show();
    delay(5);
    yield();
  }
  _strip.clear();
  _strip.show();
  for (u_int16_t i = _led_count - 1; i > 0; i--)
  {
    _strip.setPixelColor(i, _strip.Color(i + random(255), random(255), random(255)));
    _strip.setPixelColor(i + 3, 0);
    _strip.show();
    delay(5);
    yield();
  }
  _strip.clear();
  _strip.show();
}
void ShowStep(uint32_t color, u_int8_t step_index)
{
  Serial.print("ShowStep ");
  Serial.println(step_index);
  for (u_int8_t i = 0; i < _led_in_step; i++)
  {
    _strip.setPixelColor(i + _led_in_step * step_index, color);
  }
  _strip.show();
}

void ClearAllStepsFromTop()
{
  Serial.print("ClearAllStepsFromTop ");
  Serial.println(_inProgress);
  if (_inProgress)
    return;
  _inProgress = true;
  _timer_stairs.reset();
  _timer_stairs.start();
  _current_step = _step_count - 1;
  while (true)
  {
    if (_timer_stairs.isReady())
    {
      Serial.print("ClearAllStepsFromTop Step: ");
      Serial.println(_current_step);
      ShowStep(0, _current_step);
      _current_step--;
      if (_current_step < 0)
      {
        _inProgress = false;
        _timer_stairs.stop();
        _timer_stairs.reset();
        _current_step = -1;
        Serial.println("ClearAllStepsFromTop exit from while");
        break;
      }
    }
    yield();
  }
}
void ClearAllStepsFromDown()
{
  Serial.println("ClearAllStepsFromDown exit from while");
  if (_inProgress)
    return;
  _inProgress = true;
  _timer_stairs.reset();
  _timer_stairs.start();
  _current_step++;
  while (true)
  {
    if (_timer_stairs.isReady())
    {
      Serial.println("ClearAllStepsFromDown showStep");
      ShowStep(0, _current_step);
      _current_step++;
      if (_current_step > _step_count)
      {
        _inProgress = false;
        _timer_stairs.stop();
        _timer_stairs.reset();
        _current_step = -1;
        Serial.print("ClearAllStepsFromDown exit from while");
        break;
      }
    }
    yield();
  }
}

void ShowAllStepsFromTop()
{
  if (_is_disable)
    return;
  Serial.println("ShowAllStepsFromTop");
  Serial.println(_inProgress);
  if (_inProgress)
    return;
  _inProgress = true;
  _timer_stairs.reset();
  _timer_stairs.start();
  _current_step = _step_count - 1;
  while (true)
  {
    if (_timer_stairs.isReady())
    {
      Serial.println("ShowAllStepsFromTop ShowStep");
      ShowStep(_current_color, _current_step);
      _current_step--;
      if (_current_step < 0)
      {
        _inProgress = false;
        _timer_stairs.stop();
        _timer_stairs.reset();
        _timer_time_out.reset();
        _timer_time_out.start();
        _current_step = -1;
        Serial.println("ShowAllStepsFromTop Exit while");
        break;
      }
    }
    yield();
  }
}
void ShowAllStepsFromDown()
{
  if (_is_disable)
    return;
  Serial.print("ShowAllStepsFromDown");
  Serial.println(_inProgress);
  if (_inProgress)
    return;
  _inProgress = true;
  _timer_stairs.reset();
  _timer_stairs.start();
  _current_step++;
  while (true)
  {
    if (_timer_stairs.isReady())
    {
      Serial.println("ShowAllStepsFromDown show step");
      ShowStep(_current_color, _current_step);
      _current_step++;
      if (_current_step > _step_count)
      {
        _inProgress = false;
        _timer_stairs.stop();
        _timer_stairs.reset();
        _timer_time_out.reset();
        _timer_time_out.start();
        _current_step = -1;
        Serial.println("ShowAllStepsFromDown Exit while");
        break;
      }
    }
    yield();
  }
}

void connect()
{
  if (!WiFi.isConnected() && _timer_connection.isReady())
  {
    WiFi.reconnect();
    if (attempt_count > 5)
    {
      ESP.restart();
    }
    attempt_count++;

  }

  if (!client.connected() && WiFi.isConnected() && _timer_connection.isReady())
    {
      // char *client_id = "stairs-home";
      Serial.println("Connecting to public emqx mqtt broker.....");
      if (client.connect("stairs-home-3211235446574", mqtt_username, mqtt_password))
      {
        Serial.println("Public emqx mqtt broker connected");
        yield();
        client.subscribe(topic_stair_color);
        client.subscribe(topic_stair);
        client.subscribe(topic_stair_delay);
        publishState();
      }
      else
      {
        Serial.print("failed with state ");
        Serial.print(client.state());
        yield();
      }
    }

}

void setup()
{
  _timer_time_out.stop();
  Serial.begin(115200);      // Устанавливаем сокорость соединения с последовательным портом
  Serial.println("Booting"); //  "Загрузка"
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    //  "Соединиться не удалось! Перезагрузка..."
    delay(5000);
    ESP.restart();
  }

  // строчка для номера порта по умолчанию
  // можно вписать «8266»:
  ArduinoOTA.setPort(8266);

  // строчка для названия хоста по умолчанию;
  // можно вписать «esp8266-[ID чипа]»:
  ArduinoOTA.setHostname("stairs");

  // строчка для аутентификации
  // (по умолчанию никакой аутентификации не будет):
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
                       //  "Ошибка при аутентификации"
                       else if (error == OTA_BEGIN_ERROR)
                         Serial.println("Begin Failed");
                       //  "Ошибка при начале OTA-апдейта"
                       else if (error == OTA_CONNECT_ERROR)
                         Serial.println("Connect Failed");
                       //  "Ошибка при подключении"
                       else if (error == OTA_RECEIVE_ERROR)
                         Serial.println("Receive Failed");
                       //  "Ошибка при получении данных"
                       else if (error == OTA_END_ERROR)
                         Serial.println("End Failed"); });
  ArduinoOTA.begin();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected())
  {
    // char *client_id = "stairs-home";
    Serial.println("Connecting to public emqx mqtt broker.....");
    if (client.connect("stairs-home-3211235446574", mqtt_username, mqtt_password))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe(topic_stair_color);
  client.subscribe(topic_stair);
  client.subscribe(topic_stair_delay);
  publishState();
  Serial.println("Ready");      //  "Готово"
  Serial.print("IP address: "); //  "IP-адрес: "
  Serial.println(WiFi.localIP());
  _strip.begin();
  _strip.clear();
  demo();

  // randomShow();

  NightLight();
}

void readSonic()
{
  if (_is_disable)
    return;
  unsigned long mil = millis();

  if (mil - lastReadSensors > delaySensors && !_inProgress)
  {
    lastReadSensors = mil;
    cm1 = ultrasonic1.read();
    cm2 = ultrasonic2.read();

    if (cm1 < distance)
    {
      Serial.print("==============================================================Sensor 01: ");
      Serial.print(cm1); // Prints the distance on the default unit (centimeters)
      Serial.println("cm");
      ShowAllStepsFromDown();
      isTop = false;
      _timer_time_out.reset();
      _timer_time_out.start();
    }
    if (cm2 < distance)
    {
      Serial.print("Sensor 02: ========================================== ");
      Serial.print(cm2); // Prints the distance on the default unit (centimeters)
      ShowAllStepsFromTop();
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
  connect();
  if (_timer_time_out.isReady() && !_is_disable)
  {
    _timer_time_out.stop();
    Serial.print("Timer ");
    Serial.println(_timer_time_out.isReady());

    if (isTop)
    {
      Serial.println("From top");
      ClearAllStepsFromTop();
    }
    else
    {
      ClearAllStepsFromDown();
      Serial.println("From down");
    }
    Serial.println("_timer_time_out");
    NightLight();
  }

  if (_timer_change_nightlight.isReady() && !_is_disable && _timer_time_out.isReady())
  {
    for (int i = 0; i < 4; i++)
      nightLightShift[i] = nightLightShift[i] + 1 > _led_in_step - 1 ? 0 : nightLightShift[i] + 1;
    // randomShow();
    NightLight();
  }

  if (_timer_live.isReady())
  {
    client.publish(topic_stair_avaliable, "online");
  }
  ArduinoOTA.handle();
  if (client.connected())
    client.loop();
}


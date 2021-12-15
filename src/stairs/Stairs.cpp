#include "Stairs.h"
#include <Adafruit_NeoPixel.h>

Stairs::Stairs(
    u_int8_t step_count,
    u_int8_t led_in_step,
    u_int16_t step_speed,
    u_int32_t timeOut,
    u_int8_t pin)
{
    Serial.println("creating stairs");
    _led_count = step_count * led_in_step;
    _step_count = step_count;
    _led_in_step = led_in_step;
    _step_speed = step_speed;
    _timeOut = timeOut;
    _pin = pin;
    _timer_animation = GTimer(MS, 25);
    _timer_stairs = GTimer(MS, _step_speed);
    // _timer_time_out = GTimer(MS, timeOut * 1000);
    // _timer_animation.start();
    _timer_stairs.start();
    // _timer_time_out.start();
    _strip = Adafruit_NeoPixel(_led_count, _pin, NEO_GRB + NEO_KHZ800);
    _current_color = _strip.Color(255, 255, 255);
    _strip.begin();
    _strip.setBrightness(10);
    _strip.clear();
    _strip.show();
    Serial.println("created stairs");
}

void Stairs::ClearStep(u_int8_t index)
{
    Serial.println("ClearStep");
    for (uint8_t i = 0; i < _step_count; i++)
    {
        ShowStep(0, i);
    }
}

void Stairs::ShowStep(uint32_t color, u_int8_t step_index)
{
    Serial.println("ShowStep");
    for (u_int8_t i = 0; i < _led_in_step; i++)
    {
        _strip.setPixelColor(i + _led_in_step * step_index, color);
    }
    _strip.show();
}

void Stairs::_ClearAllSteps()
{
    ShowStep(0, _current_cleard_step);
    if (_current_cleard_step >= _step_count)
    {
        _inProgress = false;
        _current_cleard_step = -1;
    }
    _current_cleard_step++;
}

void Stairs::ClearAllStepsFromTop()
{
    for (int8_t i = 0; i < _step_count; i++)
    {
        ShowStep(0, i);
        delay(_step_speed);
    }
    _strip.clear();
    // if(_inProgress) return;
    // _inProgress = true;
    // Serial.println("CleadAllSteps");
    // _current_cleard_step++;
    // _ClearAllSteps();
}
void Stairs::ClearAllStepsFromDown()
{
    for (int8_t i = _step_count-1; i >=0 ; i--)
    {
        ShowStep(0, i);
        delay(_step_speed);
    }
    _strip.clear();
    // if(_inProgress) return;
    // _inProgress = true;
    // Serial.println("CleadAllSteps");
    // _current_cleard_step++;
    // _ClearAllSteps();
}

void Stairs::NightLight()
{
    Serial.println("NightLight");
    u_int8_t diodcnt = _led_in_step / 3;
    _strip.clear();
    _strip.show();
    for (u_int8_t i = 0; i < diodcnt; i++)
    {
        Serial.print("NightLight: ");
        Serial.println(i + _led_in_step * 0);
        Serial.print("NightLight: ");
        Serial.println(i + _led_in_step * _step_count - 1);
        Serial.print("NightLight: ");
        _strip.setPixelColor(i + (i % 2)*i + _led_in_step * 0, _current_color);
        _strip.setPixelColor(i + (i % 2)*i + _led_in_step * _step_count - _led_in_step, _current_color);
    }
    _strip.show();
}

void Stairs::_ShowAllSteps()
{
    ShowStep(_current_color, _current_step);
    if (_current_step >= _step_count)
    {
        _inProgress = false;
        _current_step = -1;
    }
    _current_step++;
}

void Stairs::ShowAllStepsFromTop()
{

    for (int8_t i = 0; i < _step_count; i++)
    {
        ShowStep(_current_color, i);
        delay(_step_speed);
    }

    // if(_inProgress) return;
    // _inProgress = true;
    // Serial.println("ShowAllSteps");
    // _current_step++;
    // _ShowAllSteps();
}
void Stairs::ShowAllStepsFromDown()
{
    for (int8_t i = _step_count - 1; i >= 0; i--)
    {
        ShowStep(_current_color, i);
        delay(_step_speed);
    }
}

bool Stairs::isReady()
{
    return !_inProgress;
}

void Stairs::tick()
{

    // if (_current_step > -1 && _current_step <= _step_count && _timer_stairs.isReady())
    // {
    //     Serial.print("_current_step:");
    //     Serial.println(_current_step);
    //     _ShowAllSteps();
    // }
    // if (_current_cleard_step > -1 && _current_cleard_step <= _step_count && _timer_stairs.isReady())
    // {
    //     Serial.print("_current_cleard_step:");
    //     Serial.println(_current_cleard_step);
    //     _ClearAllSteps();
    // }
}

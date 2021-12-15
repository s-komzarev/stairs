#include <Arduino.h>
#include "GyverTimer.h"
#include <Adafruit_NeoPixel.h>

class Stairs
{
private:
    u_int8_t _step_count;
    u_int8_t _led_in_step;
    u_int32_t _led_count;
    u_int16_t _step_speed = 1000;
    GTimer _timer_animation;
    GTimer _timer_stairs;
    GTimer _timer_time_out;
    u_int32_t _timeOut = 15000;
    u_int8_t _pin;
    Adafruit_NeoPixel _strip;
    bool _is_running_effect = false;
    int8_t _current_step = -1;
    u_int32_t _current_color;
    int8_t _current_cleard_step = -1;
    bool _inProgress = false;
    void ShowStep(uint32_t color, u_int8_t step_index);
    void ClearStep(u_int8_t index);
    void _ShowAllSteps();
    void _ClearAllSteps();
public:
    Stairs( u_int8_t step_count,
            u_int8_t led_in_step,
            u_int16_t step_speed,
            u_int32_t timeOut,
            u_int8_t pin);

    void NightLight();
    void ShowAllStepsFromTop();
    void ShowAllStepsFromDown();
    void ClearAllStepsFromTop();
    void ClearAllStepsFromDown();
    void tick();
    bool isReady();
};

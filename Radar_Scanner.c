#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define Buzzer 16
#define Red_LED 17
#define Green_LED 18
#define Servo_Pin 19
#define Trig_Pin 20
#define Echo_Pin 21
#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15

//Setting Up
const float maximum_pw = 19999*(2.5/20); //2499.875
const float minimum_pw = 19999*(0.5/20); //500
const float range = maximum_pw-minimum_pw;

void gpio_level(int Degree);

int main()
{
    stdio_init_all();

    //Setting Up Buzzer
    gpio_init(Buzzer);
    gpio_set_dir(Buzzer, GPIO_OUT);

    //Setting Up LEDs
    gpio_init(Green_LED);
    gpio_set_dir(Green_LED, GPIO_OUT);
    gpio_init(Red_LED);
    gpio_set_dir(Red_LED, GPIO_OUT);

    //Setting Up Ultrasoinc Sensor
    gpio_init(Trig_Pin);
    gpio_set_dir(Trig_Pin, GPIO_OUT);
    gpio_init(Echo_Pin);
    gpio_set_dir(Echo_Pin, GPIO_IN);

    //Setting Up Servo
    gpio_set_function(Servo_Pin, GPIO_FUNC_PWM);
    int slice_num = pwm_gpio_to_slice_num(Servo_Pin);
    pwm_set_clkdiv(slice_num, 150);
    pwm_set_wrap(slice_num, 19999);
    pwm_set_enabled(slice_num, true);

    //Setting Up Display
    i2c_init(I2C_PORT, 400000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_t disp;

    disp.external_vcc = false;

    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);

    int angle = 0;
    int direction = 1;

    while (true) {

        // Move servo
        gpio_level(angle);

        angle += direction;

        if (angle >= 180) {
            direction = -1;
        }

        if (angle <= 0) {
            direction = 1;
        }

        gpio_put(Trig_Pin, 0);
        sleep_us(2);

        gpio_put(Trig_Pin, 1);
        sleep_us(10);
        gpio_put(Trig_Pin, 0);

        while (!gpio_get(Echo_Pin));
        uint64_t start_time = time_us_64();

        while (gpio_get(Echo_Pin));
        uint64_t end_time = time_us_64();

        uint64_t duration_us = end_time - start_time;

        float distance_cm = duration_us * 0.034f / 2.0f;

        ssd1306_clear(&disp);
        
        char buffer[32];
        sprintf(buffer, "%.2f cm", distance_cm);
        ssd1306_draw_string(&disp, 0, 0, 2, "Distance:");
        ssd1306_draw_string(&disp, 0, 40, 2, buffer);

        if (distance_cm <= 20){
            gpio_put(Green_LED, 0);
            gpio_put(Red_LED, 1);
            gpio_put(Buzzer, 1);
        }
        else{
            gpio_put(Red_LED, 0);
            gpio_put(Green_LED, 1);
            gpio_put(Buzzer, 0);
        }
        
        ssd1306_show(&disp);

        sleep_ms(20);
    }
}

void gpio_level(int Degree){
    float fraction = (float)Degree/180.0f;
    int degree_pw = minimum_pw + fraction * range;
    pwm_set_gpio_level(Servo_Pin, degree_pw);
    return;
}

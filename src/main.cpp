#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "arduinoFFT.h"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI 5
#define OLED_CLK 17
#define OLED_DC 19
#define OLED_CS 21
#define OLED_RESET 18

const uint16_t samples = 256;         // This value MUST ALWAYS be a power of 2
const float samplingFrequency = 5000; // Hz, must be less than 10000 due to ADC
unsigned int sampling_period_us;
unsigned long microseconds;
float vReal[samples];
float vImag[samples];
uint16_t t_data[128];
/* Create FFT object with weighing factor storage */
ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency, true);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
bool mode = true;
void buttonInterrupt();
void setup()
{
  sampling_period_us = round(1000000 * (1.0 / samplingFrequency));
  Serial.begin(115200);
  pinMode(4, INPUT);
  pinMode(13, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(13), buttonInterrupt, FALLING);
  if (!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();
  delay(2000); // Pause for 2 seconds
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setRotation(2);
}

void loop()
{
  if (mode)
  {

    display.setCursor(0, 0);
    display.clearDisplay();
    microseconds = micros();
    for (int i = 0; i < samples; i++)
    {
      vReal[i] = analogRead(4);
      vImag[i] = 0;
      while (micros() - microseconds < sampling_period_us)
      {
      }
      microseconds += sampling_period_us;
    }
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    float x = FFT.majorPeak();
    Serial.println(x, 6);
    for (int i = 0; i < samples / 2; i+=2)
    {
      display.drawLine(i, 63, i, 128 - (float)vReal[i] / 4096 * 64, WHITE);
      
    }
    display.setCursor(92, 0);
    display.print((int)x);
    display.print(" Hz");
    display.display();
  }
  else
  {
    display.setCursor(0, 0);
    display.clearDisplay();
    int start_time = millis();
    for (int i = 0; i < 128; i++)
    {
      t_data[i] = analogRead(4);
      display.drawPixel(i, 84 - (float)t_data[i] / 4096 * 100, WHITE);
      //display.drawLine(i, 63, i, 84 - (float)t_data[i] / 4096 * 100, WHITE);
    }
    int tf_time = millis() - start_time;
    display.display();
  }
}
void buttonInterrupt()
{
  if (digitalRead(13) == LOW)
  {
    mode = 1 - mode;
  }
}

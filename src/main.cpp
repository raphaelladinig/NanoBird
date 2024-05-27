#include "Adafruit_NeoPixel.h"
#include "Adafruit_SSD1306.h"
#include "SoftwareSerial.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

int b1 = 2;
int b2 = 3;
SoftwareSerial bt(10, 0);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, 13, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(128, 64, &Wire);

void setup() {
    pinMode(b1, INPUT);
    pinMode(b2, INPUT);

    bt.begin(9600);
    Serial.begin(9600);
    strip.begin();
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    Serial.println("Started");
}

void loop() {
     display.display();
}

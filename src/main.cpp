#include "Adafruit_NeoPixel.h"
#include "Adafruit_SSD1306.h"
#include "HardwareSerial.h"
#include "SoftwareSerial.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

int bPin = 2;

SoftwareSerial bt(10, 0);
struct Signal {
    char id;
    int value;
};
char btChar = ' ';
char btId = ' ';
char btIdTemp = ' ';
bool btIdExpected = true;
bool btNewReading = false;
String btValueString = "";
float btValue;
Signal btQuery();

Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, 13, NEO_GRB + NEO_KHZ800);

Adafruit_SSD1306 display(128, 64, &Wire, -1);
void text(int x, int y, String txt);
void wipe();
void initalize();

int game_state = 1;
int score = 0;
int high_score = 0;
int bird_x = (int)display.width() / 4;
int bird_y;
int speed = 0;

#define SPRITE_HEIGHT 16
#define SPRITE_WIDTH 16
static const unsigned char PROGMEM wing_down_bmp[] = {
    B00000000, B00000000, B00000000, B00000000, B00000011, B11000000, B00011111,
    B11110000, B00111111, B00111000, B01111111, B11111110, B11111111, B11000001,
    B11011111, B01111110, B11011111, B01111000, B11011111, B01111000, B11001110,
    B01111000, B11110001, B11110000, B01111111, B11100000, B00111111, B11000000,
    B00000111, B00000000, B00000000, B00000000,
};
static const unsigned char PROGMEM wing_up_bmp[] = {
    B00000000, B00000000, B00000000, B00000000, B00000011, B11000000, B00011111,
    B11110000, B00111111, B00111000, B01110001, B11111110, B11101110, B11000001,
    B11011111, B01111110, B11011111, B01111000, B11111111, B11111000, B11111111,
    B11111000, B11111111, B11110000, B01111111, B11100000, B00111111, B11000000,
    B00000111, B00000000, B00000000, B00000000,
};

void setup() {
    bt.begin(9600);
    Serial.begin(9600);
    strip.begin();
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextColor(WHITE);
    display.clearDisplay();
    Serial.println("Started");
}

void loop() {
    if (game_state == 0) {
        display.clearDisplay();

        if (digitalRead(bPin) == LOW) {
            speed += 4;
        } else {
            speed -= 2;
        }

        speed = constrain(speed, -8, 8);
        bird_y += speed;

        if (bird_y < 0) {
            bird_y = 0;
        }
        if (bird_y > display.height() - SPRITE_HEIGHT) {
            bird_y = display.height() - SPRITE_HEIGHT;
        }

        if (speed < 0) {
            display.drawBitmap(bird_x, bird_y, wing_down_bmp, 16, 16, WHITE);
        } else {
            display.drawBitmap(bird_x, bird_y, wing_up_bmp, 16, 16, WHITE);
        }

        display.display();
        delay(50);
    } else if (game_state == 1) {
        wipe();
        text(0, 0, "Game Over");
        text(0, 10, "Score: " + String(score));
        if (score > high_score) {
            high_score = score;
            text(0, 20, "New High Score!");
        } else {
            text(0, 20, "High Score: " + String(high_score));
        }
        display.display();

        while (digitalRead(bPin) == LOW)
            ;

        initalize();
        while (digitalRead(bPin) == HIGH)
            ;
        game_state = 0;
    }
}

Signal btQuery() {
    Signal result = {0, 0};

    btNewReading = false;
    if (bt.available()) {
        btChar = bt.read();
        if (btIdExpected) {
            btIdTemp = btChar;
            btIdExpected = false;
            btValueString = "";
        } else {
            if ((btChar != ' ') && (btChar != '\n')) {
                btValueString += btChar;
            }
            if (btChar == '\n') {
                result.id = btIdTemp;
                result.value = btValueString.toFloat();
                Serial.print(result.id);
                Serial.print(" ");
                Serial.println(result.value);
                btValueString = "";
                btIdExpected = true;
                btNewReading = true;
            }
        }
    }

    return result;
}

void text(int x, int y, String txt) {
    display.setCursor(x, y);
    display.print(txt);
}

void wipe() {
    for (int i = 0; i < display.height(); i += 10) {
        display.fillRect(0, i, display.width(), 10, WHITE);
        display.display();
    }
    for (int i = 0; i < display.height(); i += 10) {
        display.fillRect(0, i, display.width(), 10, BLACK);
        display.display();
    }
}

void initalize() {
    bird_y = (int)display.height() / 2;
    speed = 0;
    score = 0;
}

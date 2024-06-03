#include "Adafruit_NeoPixel.h"
#include "Adafruit_SSD1306.h"
#include "HardwareSerial.h"
#include "SoftwareSerial.h"
#include <Arduino.h>
#include <EEPROM.h>
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
void text(int y, String txt);
void wipe();
void initalize();

int state = 2; // 0 = game, 1 = game over, 2 = title
int score;
int high_score;

#define SPRITE_HEIGHT 16
#define SPRITE_WIDTH 16
static const unsigned char PROGMEM wing_down[] = {
    B00000000, B00000000, B00000000, B00000000, B00000011, B11000000, B00011111,
    B11110000, B00111111, B00111000, B01111111, B11111110, B11111111, B11000001,
    B11011111, B01111110, B11011111, B01111000, B11011111, B01111000, B11001110,
    B01111000, B11110001, B11110000, B01111111, B11100000, B00111111, B11000000,
    B00000111, B00000000, B00000000, B00000000,
};
static const unsigned char PROGMEM wing_up[] = {
    B00000000, B00000000, B00000000, B00000000, B00000011, B11000000, B00011111,
    B11110000, B00111111, B00111000, B01110001, B11111110, B11101110, B11000001,
    B11011111, B01111110, B11011111, B01111000, B11111111, B11111000, B11111111,
    B11111000, B11111111, B11110000, B01111111, B11100000, B00111111, B11000000,
    B00000111, B00000000, B00000000, B00000000,
};

struct bird {
    int x;
    int y;
    int speed;
};
bird bird;

struct wall {
    int x;
    int y;
};
wall wall[2];
const int wall_gap = 40;
const int wall_width = 10;


void setup() {
    bt.begin(9600);
    Serial.begin(9600);
    strip.begin();
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextColor(WHITE);
    display.clearDisplay();
    initalize();
    Serial.println("Started");
}

void loop() {
    if (state == 0) {
        display.clearDisplay();

        if (digitalRead(bPin) == LOW) {
            bird.speed += 4;
        } else {
            bird.speed -= 2;
        }

        bird.speed = constrain(bird.speed, -8, 8);
        bird.y += bird.speed;

        if (bird.y < 0) {
            bird.y = 0;
        }
        if (bird.y > display.height() - SPRITE_HEIGHT) {
            bird.y = display.height() - SPRITE_HEIGHT;
        }

        if (bird.speed < 0) {
            display.drawBitmap(bird.x, bird.y, wing_down, 16, 16,
                               WHITE);
        } else {
            display.drawBitmap(bird.x, bird.y, wing_up, 16, 16, WHITE);
        }

        for (int i = 0; i < 2; i++) {
            display.fillRect(wall[i].x, 0, wall_width, wall[i].y, WHITE);

            display.fillRect(wall[i].x, wall[i].y + wall_gap, wall_width,
                             display.height() - wall[i].y + wall_gap, WHITE);

            if (wall[i].x < 0) {
                wall[i].y = random(0, display.height() - wall_gap);
                wall[i].x = display.width();
            }

            if (wall[i].x == bird.x) {
                score++;
            }

            if ((bird.x + SPRITE_WIDTH > wall[i].x &&
                 bird.x < wall[i].x + wall_width) &&
                (bird.y < wall[i].y ||
                 bird.y + SPRITE_HEIGHT > wall[i].y + wall_gap)) {
                display.display();
                delay(200);
                state = 1;
            }

            wall[i].x -= 4;
        }

        display.display();
        delay(25);
    } else if (state == 1) {
        wipe();
        text(0, "Game Over!");
        text(10, "Score: " + String(score));
        if (score > high_score) {
            high_score = score;
            EEPROM.write(0, high_score);
            text(20, "New High Score!");
        } else {
            text(20, "High Score: " + String(high_score));
        }
        display.display();

        while (digitalRead(bPin) == LOW)
            ;
        initalize();
        while (digitalRead(bPin) == HIGH)
            ;
        state = 0;
    } else if (state == 2) {
        text(display.height() / 2, "Nano Bird");
        display.display();

        while (digitalRead(bPin) == LOW)
            ;
        while (digitalRead(bPin) == HIGH)
            ;
        state = 0;
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

void text(int y, String txt) {
    display.setCursor((display.width() - txt.length() * 6) / 2, y);
    display.println(txt);
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
    bird.y = (int)display.height() / 2;
    bird.speed = 0;
    score = 0;
    high_score = EEPROM.read(0);
    wall[0].x = display.width();
    wall[0].y = random(0, display.height() - wall_gap);
    wall[1].x = display.width() + (display.width() / 2);
    wall[1].y = random(0, display.height() - wall_gap);
}

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Menu.h"
#include "Dpad.h"
#include "MenuBar.h"

// Define the pins connected to the D-pad
#define UP_PIN     13
#define DOWN_PIN   12
#define LEFT_PIN   14
#define RIGHT_PIN  27

TFT_eSPI tft = TFT_eSPI();
Dpad dpad(UP_PIN, DOWN_PIN, LEFT_PIN, RIGHT_PIN);
MenuBar bar(tft);

// Create Menu object
Menu menu(tft, dpad, bar);

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    dpad.begin();
    menu.begin();
}

void loop() {
    menu.runMenu();
    delay(10);
}

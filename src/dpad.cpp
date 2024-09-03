#include "Dpad.h"

Dpad *instance = nullptr; // Global pointer to handle ISRs

Dpad::Dpad(int upPin, int downPin, int leftPin, int rightPin) 
    : upPin(upPin), downPin(downPin), leftPin(leftPin), rightPin(rightPin), 
      upPressed(false), downPressed(false), leftPressed(false), rightPressed(false) {
    instance = this;  // Initialize global instance pointer
}

void Dpad::begin() {
    pinMode(upPin, INPUT_PULLUP);
    pinMode(downPin, INPUT_PULLUP);
    pinMode(leftPin, INPUT_PULLUP);
    pinMode(rightPin, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(upPin), upISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(downPin), downISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(leftPin), leftISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(rightPin), rightISR, FALLING);
}

bool Dpad::isUpPressed() {
    return upPressed;
}

bool Dpad::isDownPressed() {
    return downPressed;
}

bool Dpad::isLeftPressed() {
    return leftPressed;
}

bool Dpad::isRightPressed() {
    return rightPressed;
}

void Dpad::clearFlags() {
    upPressed = downPressed = leftPressed = rightPressed = false;
}

void IRAM_ATTR Dpad::upISR() {
    instance->upPressed = true;
}

void IRAM_ATTR Dpad::downISR() {
    instance->downPressed = true;
}

void IRAM_ATTR Dpad::leftISR() {
    instance->leftPressed = true;
}

void IRAM_ATTR Dpad::rightISR() {
    instance->rightPressed = true;
}

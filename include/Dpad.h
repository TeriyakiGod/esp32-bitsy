#ifndef DPAD_H
#define DPAD_H

#include <Arduino.h>

class Dpad {
public:
    Dpad(int upPin, int downPin, int leftPin, int rightPin);

    void begin();

    // Methods to check the state of each button
    bool isUpPressed();
    bool isDownPressed();
    bool isLeftPressed();
    bool isRightPressed();

    void clearFlags();

    // ISR handler methods
    static void IRAM_ATTR upISR();
    static void IRAM_ATTR downISR();
    static void IRAM_ATTR leftISR();
    static void IRAM_ATTR rightISR();

private:
    int upPin;
    int downPin;
    int leftPin;
    int rightPin;

    volatile bool upPressed;
    volatile bool downPressed;
    volatile bool leftPressed;
    volatile bool rightPressed;
};

#endif

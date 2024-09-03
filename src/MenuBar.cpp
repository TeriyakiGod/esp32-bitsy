#include "MenuBar.h"

MenuBar::MenuBar(TFT_eSPI &display) 
    : GUIComponent(display), lastUpdateTime(0), hours(0), minutes(0), seconds(0) {
    strcpy(timeStr, "00:00");
}

void MenuBar::update() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateTime >= 1000) {
        lastUpdateTime = currentMillis;
        incrementTime();
        draw();  // Redraw the clock whenever the time is updated
    }
}

const char* MenuBar::getTimeStr() {
    return timeStr;
}

void MenuBar::draw() {
    tft.fillRect(0, 0, tft.width(), 16, TFT_BLACK);  // Clear previous time area (adjust as needed)
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(2, 0);  // Adjust cursor position based on your layout
    tft.println(timeStr);  // Draw the current time on the display
}

void MenuBar::incrementTime() {
    seconds++;
    if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
            minutes = 0;
            hours++;
            if (hours >= 24) {
                hours = 0;
            }
        }
    }
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hours, minutes);
}

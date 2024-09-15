#include "MenuBar.h"

MenuBar::MenuBar(TFT_eSPI &display) 
    : GUIComponent(display), lastUpdateTime(0), hours(0), minutes(0), seconds(0), day(1), month(1), year(2024) {
    snprintf(timeDateStr, sizeof(timeDateStr), "%02d/%02d %02d:%02d", day, month, hours, minutes);  // Initialize the combined timeDateStr
}

void MenuBar::update() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateTime >= 1000) {
        lastUpdateTime = currentMillis;
        incrementTime();
        draw();
    }
}

void MenuBar::draw() {
    tft.fillRect(0, 0, tft.width(), 16, TFT_WHITE);  // Clear the bar area
    tft.setTextColor(TFT_BLACK, TFT_WHITE);

    tft.setCursor(2, 5);
    tft.print("chiliOS");
    
    // Calculate the width of the timeDateStr text
    int16_t textWidth = tft.textWidth(timeDateStr);
    
    // Calculate the x position to shift the text to the right
    int16_t xPos = tft.width() - textWidth - 2;  // Subtracting 2 for a small padding
    
    // Set cursor to the calculated position
    tft.setCursor(xPos, 5);
    tft.println(timeDateStr);  // Draw the combined date and time on the display
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
                // A new day has started
                updateDate();
            }
        }
    }
    snprintf(timeDateStr, sizeof(timeDateStr), "%02d/%02d %02d:%02d", day, month, hours, minutes);
}

void MenuBar::updateDate() {
    // Simple date increment logic (no leap year handling or month length variation)
    day++;
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
        day = 1;
        month++;
    } else if (month == 2 && day > 28) {
        day = 1;
        month++;
    } else if (day > 31) {
        day = 1;
        month++;
    }
    
    if (month > 12) {
        month = 1;
        year++;
    }
    
    snprintf(timeDateStr, sizeof(timeDateStr), "%02d/%02d %02d:%02d", day, month, hours, minutes);
}

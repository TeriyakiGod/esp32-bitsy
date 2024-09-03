#ifndef MENUBAR_H
#define MENUBAR_H

#include "GUIComponent.h"

class MenuBar : public GUIComponent {
public:
    MenuBar(TFT_eSPI &display);
    void update();
    void draw() override;

private:
    char timeDateStr[30];  // Combined date and time string
    unsigned long lastUpdateTime;
    int hours;
    int minutes;
    int seconds;
    int day;
    int month;
    int year;

    void incrementTime();
    void updateDate();
};

#endif

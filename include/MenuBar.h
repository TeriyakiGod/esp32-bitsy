#ifndef MENUBAR_H
#define MENUBAR_H

#include "GUIComponent.h"

class MenuBar : public GUIComponent {
public:
    MenuBar(TFT_eSPI &display);
    void update();
    const char* getTimeStr();
    void draw() override;

private:
    char timeStr[20];
    unsigned long lastUpdateTime;
    int hours;
    int minutes;
    int seconds;

    void incrementTime();
};

#endif

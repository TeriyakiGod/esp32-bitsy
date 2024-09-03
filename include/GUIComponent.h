#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include <TFT_eSPI.h>

class GUIComponent {
public:
    GUIComponent(TFT_eSPI &display);
    virtual void draw() = 0;

protected:
    TFT_eSPI &tft;
};

#endif

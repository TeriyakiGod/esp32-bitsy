#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <LittleFS.h>
#include "Dpad.h"
#include "MenuBar.h"

class Menu {
public:
    Menu(TFT_eSPI &display, Dpad &dpad, MenuBar &menuBar);
    void begin();
    void runMenu();

private:
    TFT_eSPI &tft;
    Dpad &dpad;
    MenuBar &menuBar;
    int selectedFileIndex;
    int fileCount;
    const char** files;

    void listFiles(fs::FS &fs, const char *dirname);
    void drawFileList();
    void handleNavigation();
};

#endif

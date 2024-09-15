#include "Menu.h"

Menu::Menu(TFT_eSPI &display, Dpad &dpad, MenuBar &menuBar) 
    : tft(display), dpad(dpad), menuBar(menuBar), selectedFileIndex(0), fileCount(0), files(nullptr) {}

void Menu::begin() {
    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    listFiles(LittleFS, "/");
}

void Menu::listFiles(fs::FS &fs, const char *dirname) {
    // Clear any previous file list
    if (files != nullptr) {
        for (int i = 0; i < fileCount; i++) {
            free((void*)files[i]);
        }
        free(files);
        files = nullptr;
    }

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    // Count the number of files first
    fileCount = 0;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            fileCount++;
        }
        file = root.openNextFile();
    }

    // Allocate memory for file names
    files = (const char**)malloc(fileCount * sizeof(char*));
    if (files == nullptr) {
        Serial.println("Failed to allocate memory for file list");
        return;
    }

    // Populate the file list
    root = fs.open(dirname);
    int index = 0;
    file = root.openNextFile();
    while (file) {
        if (!file.isDirectory() && index < fileCount) {
            files[index++] = strdup(file.name());  // Copy file name to list
        }
        file = root.openNextFile();
    }

    drawFileList();
}

void Menu::drawFileList() {
    tft.fillRect(0, 16, tft.width(), tft.height() - 16, TFT_BLACK);  // Clear the screen below the menu bar
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Set text color

    for (int i = 0; i < fileCount; i++) {
        tft.setCursor(2, 18 + i * 12);
        if (i == selectedFileIndex) {
            tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Highlight the selected file
            tft.println(files[i]);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Reset text color
        } else {
            tft.println(files[i]);
        }
    }
}

void Menu::runMenu() {
    handleNavigation();
    menuBar.update();  // Update the menu bar, which updates and draws the clock
}

void Menu::handleNavigation() {
    static int lastSelectedFileIndex = -1;

    if (dpad.isUpPressed() && selectedFileIndex > 0) {
        selectedFileIndex--;
    } else if (dpad.isDownPressed() && selectedFileIndex < fileCount - 1) {
        selectedFileIndex++;
    }

    if (selectedFileIndex != lastSelectedFileIndex) {
        drawFileList();
        lastSelectedFileIndex = selectedFileIndex;
    }

    // Clear the D-pad flags after handling input
    dpad.clearFlags();
}

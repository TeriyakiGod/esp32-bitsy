#include <Arduino.h>
#include <TFT_eSPI.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h> // for getcwd, chdir
#include <limits.h> // for PATH_MAX
#include <dirent.h>
#include "duktape/duktape.h"
#include "LittleFS.h"

#ifndef BUILD_DEBUG
#include "engine.h"
#include "font.h"
#include "boot.h"
#endif

#define SYSTEM_PALETTE_MAX 256
#define SYSTEM_DRAWING_BUFFER_MAX 1024

/* TFT */
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite *drawingBuffers[SYSTEM_DRAWING_BUFFER_MAX];

/* GAME SELECT */
char gameFilePath[256];
int gameCount = 0;

/* GLOBALS */
int screenSize = 128;
int tileSize = 8;
int roomSize = 16;

int shouldContinue = 1;

/* GRAPHICS */
typedef struct Color
{
    int r;
    int g;
    int b;
} Color;

int curGraphicsMode = 0;
Color systemPalette[SYSTEM_PALETTE_MAX];
int curBufferId = -1;

int screenBufferId = 0;
int textboxBufferId = 1;
int tileStartBufferId = 2;
int nextBufferId = 2;

int textboxWidth = 0;
int textboxHeight = 0;

int windowWidth = 0;
int windowHeight = 0;

/* INPUT */
int isButtonUp = 0;
int isButtonDown = 0;
int isButtonLeft = 0;
int isButtonRight = 0;
int isButtonW = 0;
int isButtonA = 0;
int isButtonS = 0;
int isButtonD = 0;
int isButtonR = 0;
int isButtonSpace = 0;
int isButtonReturn = 0;
int isButtonEscape = 0;
int isButtonLCtrl = 0;
int isButtonRCtrl = 0;
int isButtonLAlt = 0;
int isButtonRAlt = 0;
int isButtonPadUp = 0;
int isButtonPadDown = 0;
int isButtonPadLeft = 0;
int isButtonPadRight = 0;
int isButtonPadA = 0;
int isButtonPadB = 0;
int isButtonPadX = 0;
int isButtonPadY = 0;
int isButtonPadStart = 0;

/* FILE LOADING */
int loadScript(duk_context *ctx, char *filepath)
{
    int success = 0;

    Serial.printf("Loading %s ...\n", filepath);

    // Open the file using LittleFS
    File file = LittleFS.open(filepath, "r");
    if (!file)
    {
        Serial.printf("Failed to open file: %s\n", filepath);
        return success;
    }

    // Get file size and allocate memory
    long length = file.size();
    char *fileBuffer = (char *)malloc(length + 1);

    if (fileBuffer)
    {
        // Read file into buffer
        file.readBytes(fileBuffer, length);
        fileBuffer[length] = '\0'; // Ensure null termination
    }

    file.close();

    if (fileBuffer)
    {
        duk_push_lstring(ctx, (const char *)fileBuffer, (duk_size_t)length);

        if (duk_peval(ctx) != 0)
        {
            Serial.printf("Load Script Error: %s\n", duk_safe_to_string(ctx, -1));
        }
        else
        {
            success = 1;
        }

        duk_pop(ctx);
    }

    free(fileBuffer); // Free allocated memory
    return success;
}

int loadFile(duk_context *ctx, char *filepath, char *variableName)
{
    int success = 0;

    Serial.printf("Loading %s ...\n", filepath);

    // Open the file using LittleFS
    File file = LittleFS.open(filepath, "r");
    if (!file)
    {
        Serial.printf("Failed to open file: %s\n", filepath);
        return success;
    }

    // Get file size and allocate memory
    long length = file.size();
    char *fileBuffer = (char *)malloc(length + 1);

    if (fileBuffer)
    {
        // Read file into buffer
        file.readBytes(fileBuffer, length);
        fileBuffer[length] = '\0'; // Ensure null termination
    }

    file.close();

    if (fileBuffer)
    {
        duk_push_lstring(ctx, (const char *)fileBuffer, (duk_size_t)length);
        duk_put_global_string(ctx, variableName);
        success = 1;
    }

    free(fileBuffer); // Free allocated memory
    return success;
}

int loadEmbeddedScript(duk_context *ctx, char *fileStr)
{
    int success = 1;

    if (duk_peval_string(ctx, fileStr) != 0)
    {
        Serial.printf("Load Embedded Script Error: %s\n", duk_safe_to_string(ctx, -1));
        success = 0;
    }
    duk_pop(ctx);

    return success;
}

int loadEmbeddedFile(duk_context *ctx, char *fileStr, char *variableName)
{
    duk_push_string(ctx, fileStr);
    duk_put_global_string(ctx, variableName);

    return 1;
}

/* Bitsy System APIs */
duk_ret_t bitsyLog(duk_context *ctx)
{
    const char *printStr;
    printStr = duk_safe_to_string(ctx, 0);

    Serial.print("bitsy::");
    Serial.println(printStr);

    // todo : category parameter

    return 0;
}

duk_ret_t bitsyGetButton(duk_context *ctx)
{
    int buttonCode = duk_get_int(ctx, 0);

    int isAnyAlt = (isButtonLAlt || isButtonRAlt);
    int isAnyCtrl = (isButtonLCtrl || isButtonRCtrl);
    int isCtrlPlusR = isAnyCtrl && isButtonR;
    int isPadFaceButton = isButtonPadA || isButtonPadB || isButtonPadX || isButtonPadY;

    if (buttonCode == 0)
    {
        duk_push_boolean(ctx, isButtonUp || isButtonW || isButtonPadUp);
    }
    else if (buttonCode == 1)
    {
        duk_push_boolean(ctx, isButtonDown || isButtonS || isButtonPadDown);
    }
    else if (buttonCode == 2)
    {
        duk_push_boolean(ctx, isButtonLeft || isButtonA || isButtonPadLeft);
    }
    else if (buttonCode == 3)
    {
        duk_push_boolean(ctx, isButtonRight || isButtonD || isButtonPadRight);
    }
    else if (buttonCode == 4)
    {
        duk_push_boolean(ctx, isButtonSpace || (isButtonReturn && !isAnyAlt) || isPadFaceButton);
    }
    else if (buttonCode == 5)
    {
        duk_push_boolean(ctx, isButtonEscape || isCtrlPlusR || isButtonPadStart);
    }
    else
    {
        duk_push_boolean(ctx, 0);
    }

    return 1;
}

duk_ret_t bitsySetGraphicsMode(duk_context *ctx)
{
    curGraphicsMode = duk_get_int(ctx, 0);

    return 0;
}

duk_ret_t bitsySetColor(duk_context *ctx)
{
    int paletteIndex = duk_get_int(ctx, 0);
    int r = duk_get_int(ctx, 1);
    int g = duk_get_int(ctx, 2);
    int b = duk_get_int(ctx, 3);

    systemPalette[paletteIndex] = (Color){r, g, b};

    return 0;
}

duk_ret_t bitsyResetColors(duk_context *ctx)
{
    for (int i = 0; i < SYSTEM_PALETTE_MAX; i++)
    {
        systemPalette[i] = (Color){0, 0, 0};
    }

    return 0;
}

duk_ret_t bitsyDrawBegin(duk_context *ctx)
{
    curBufferId = duk_get_int(ctx, 0);
    return 0;
}

duk_ret_t bitsyDrawEnd(duk_context *ctx)
{
    curBufferId = -1;
    return 0;
}

duk_ret_t bitsyDrawPixel(duk_context *ctx)
{
    int paletteIndex = duk_get_int(ctx, 0);
    int x = duk_get_int(ctx, 1);
    int y = duk_get_int(ctx, 2);

    Color color = systemPalette[paletteIndex];

    tft.drawPixel(x, y, tft.color565(color.r, color.g, color.b));

    return 0;
}

duk_ret_t bitsyDrawTile(duk_context *ctx)
{
    // can only draw tiles on the screen buffer in tile mode
    if (curBufferId != 0 || curGraphicsMode != 1)
    {
        return 0;
    }

    int tileId = duk_get_int(ctx, 0);
    int x = duk_get_int(ctx, 1);
    int y = duk_get_int(ctx, 2);

    if (tileId < tileStartBufferId || tileId >= nextBufferId)
    {
        return 0;
    }

    // Copy sprite from drawingBuffers
    drawingBuffers[tileId]->pushSprite(x, y);

    return 0;
}

duk_ret_t bitsyDrawTextbox(duk_context *ctx)
{
    if (curBufferId != 0 || curGraphicsMode != 1)
    {
        return 0;
    }

    int x = duk_get_int(ctx, 0);
    int y = duk_get_int(ctx, 1);

    // Copy textbox buffer to screen buffer
    drawingBuffers[1]->pushSprite(x, y);

    return 0;
}

duk_ret_t bitsyClear(duk_context *ctx)
{
    int paletteIndex = duk_get_int(ctx, 0);

    Color color = systemPalette[paletteIndex];

    if (curBufferId == 0)
    {
        // Clear the screen buffer
        tft.fillScreen(tft.color565(color.r, color.g, color.b));
    }
    else if (curBufferId == 1)
    {
        // Clear the textbox buffer
        tft.fillRect(0, 0, textboxWidth, textboxHeight, tft.color565(color.r, color.g, color.b));
    }
    else if (curBufferId >= tileStartBufferId && curBufferId < nextBufferId)
    {
        // Clear the tile buffer
        tft.fillRect(0, 0, tileSize, tileSize, tft.color565(color.r, color.g, color.b));
    }

    return 0;
}

duk_ret_t bitsyAddTile(duk_context *ctx)
{
    if (nextBufferId >= SYSTEM_DRAWING_BUFFER_MAX)
    {
        // todo : error handling?
        return 0;
    }

    auto newTile = new TFT_eSprite(&tft);
    newTile->createSprite(tileSize, tileSize);
    drawingBuffers[nextBufferId] = newTile;

    duk_push_int(ctx, nextBufferId);

    nextBufferId++;

    return 1;
}

duk_ret_t bitsyResetTiles(duk_context *ctx)
{
    nextBufferId = tileStartBufferId;

    return 0;
}

duk_ret_t bitsySetTextboxSize(duk_context *ctx)
{
    textboxWidth = duk_get_int(ctx, 0);
    textboxHeight = duk_get_int(ctx, 1);

    drawingBuffers[1]->deleteSprite();
    drawingBuffers[1]->createSprite(textboxWidth, textboxHeight);

    return 0;
}

duk_ret_t bitsyOnLoad(duk_context *ctx)
{
    // hacky to just stick it in the global namespace??
    duk_put_global_string(ctx, "__bitsybox_on_load__");

    return 0;
}

duk_ret_t bitsyOnQuit(duk_context *ctx)
{
    duk_put_global_string(ctx, "__bitsybox_on_quit__");

    return 0;
}

duk_ret_t bitsyOnUpdate(duk_context *ctx)
{
    duk_put_global_string(ctx, "__bitsybox_on_update__");

    return 0;
}

static void fatalError(void *udata, const char *msg)
{
    Serial.printf("*** FATAL ERROR: %s\n", (msg ? msg : "no message"));
    Serial.flush();
    ESP.restart();
}

void initBitsySystem(duk_context *ctx)
{
    duk_push_c_function(ctx, bitsyLog, 2);
    duk_put_global_string(ctx, "bitsyLog");

    duk_push_c_function(ctx, bitsyGetButton, 1);
    duk_put_global_string(ctx, "bitsyGetButton");

    duk_push_c_function(ctx, bitsySetGraphicsMode, 1);
    duk_put_global_string(ctx, "bitsySetGraphicsMode");

    duk_push_c_function(ctx, bitsySetColor, 4);
    duk_put_global_string(ctx, "bitsySetColor");

    duk_push_c_function(ctx, bitsyResetColors, 0);
    duk_put_global_string(ctx, "bitsyResetColors");

    duk_push_c_function(ctx, bitsyDrawBegin, 1);
    duk_put_global_string(ctx, "bitsyDrawBegin");

    duk_push_c_function(ctx, bitsyDrawEnd, 0);
    duk_put_global_string(ctx, "bitsyDrawEnd");

    duk_push_c_function(ctx, bitsyDrawPixel, 3);
    duk_put_global_string(ctx, "bitsyDrawPixel");

    duk_push_c_function(ctx, bitsyDrawTile, 3);
    duk_put_global_string(ctx, "bitsyDrawTile");

    duk_push_c_function(ctx, bitsyDrawTextbox, 2);
    duk_put_global_string(ctx, "bitsyDrawTextbox");

    duk_push_c_function(ctx, bitsyClear, 1);
    duk_put_global_string(ctx, "bitsyClear");

    duk_push_c_function(ctx, bitsyAddTile, 0);
    duk_put_global_string(ctx, "bitsyAddTile");

    duk_push_c_function(ctx, bitsyResetTiles, 0);
    duk_put_global_string(ctx, "bitsyResetTiles");

    duk_push_c_function(ctx, bitsySetTextboxSize, 2);
    duk_put_global_string(ctx, "bitsySetTextboxSize");

    duk_push_c_function(ctx, bitsyOnLoad, 1);
    duk_put_global_string(ctx, "bitsyOnLoad");

    duk_push_c_function(ctx, bitsyOnQuit, 1);
    duk_put_global_string(ctx, "bitsyOnQuit");

    duk_push_c_function(ctx, bitsyOnUpdate, 1);
    duk_put_global_string(ctx, "bitsyOnUpdate");
}

void loadEngine(duk_context *ctx)
{
#ifdef BUILD_DEBUG
    // load engine
    shouldContinue = shouldContinue && loadScript(ctx, "bitsy/engine/script.js");
    shouldContinue = shouldContinue && loadScript(ctx, "bitsy/engine/font.js");
    shouldContinue = shouldContinue && loadScript(ctx, "bitsy/engine/transition.js");
    shouldContinue = shouldContinue && loadScript(ctx, "bitsy/engine/dialog.js");
    shouldContinue = shouldContinue && loadScript(ctx, "bitsy/engine/renderer.js");
    shouldContinue = shouldContinue && loadScript(ctx, "bitsy/engine/bitsy.js");
    // load default font
    shouldContinue = shouldContinue && loadFile(ctx, "bitsy/font/ascii_small.bitsyfont", "__bitsybox_default_font__");
#else
    // load engine
    shouldContinue = shouldContinue && loadEmbeddedScript(ctx, script_js);
    shouldContinue = shouldContinue && loadEmbeddedScript(ctx, font_js);
    shouldContinue = shouldContinue && loadEmbeddedScript(ctx, transition_js);
    shouldContinue = shouldContinue && loadEmbeddedScript(ctx, dialog_js);
    shouldContinue = shouldContinue && loadEmbeddedScript(ctx, renderer_js);
    shouldContinue = shouldContinue && loadEmbeddedScript(ctx, bitsy_js);
    // load default font
    shouldContinue = shouldContinue && loadEmbeddedFile(ctx, ascii_small_bitsyfont, "__bitsybox_default_font__");
#endif
}

void bootMenu()
{
    tft.setTextDatum(TC_DATUM); // Align text on the screen

    duk_context *ctx = duk_create_heap_default();
    initBitsySystem(ctx);

    // Load game files
    duk_peval_string(ctx, "__bitsybox_game_files__ = []");
    duk_pop(ctx);

    // Open the games directory using LittleFS
    File dir = LittleFS.open("/games");
    if (dir && dir.isDirectory())
    {
        File file = dir.openNextFile();
        while (file)
        {
            const char *fileName = file.name();
            duk_push_string(ctx, fileName);
            duk_put_global_string(ctx, "__bitsybox_filename__");
            duk_peval_string(
                ctx,
                "var fileSplit = __bitsybox_filename__.split('.');"
                "if (fileSplit[fileSplit.length - 1] === 'bitsy') { __bitsybox_game_files__.push(__bitsybox_filename__); }");
            duk_pop(ctx);
            file = dir.openNextFile();
        }
    }

    // Timing variables
    int prevTime = millis();
    int deltaTime = 0;
    int loopTime = 0;
    int loopTimeMax = 16;
    int isBootFinished = 0;

    loadEngine(ctx);

    // Load boot menu
#ifdef BUILD_DEBUG
    shouldContinue = shouldContinue && loadScript(ctx, "boot/boot.js");
    shouldContinue = shouldContinue && loadFile(ctx, "boot/boot.bitsy", "__bitsybox_game_data__");
#else
    shouldContinue = shouldContinue && loadEmbeddedScript(ctx, boot_js);
    shouldContinue = shouldContinue && loadEmbeddedFile(ctx, boot_bitsy, "__bitsybox_game_data__");
#endif

    if (duk_peval_string(ctx, "__bitsybox_on_load__(__bitsybox_game_data__, __bitsybox_default_font__);") != 0)
    {
        Serial.printf("Load Boot Menu Error: %s\n", duk_safe_to_string(ctx, -1));
    }
    duk_pop(ctx);

    while (shouldContinue && !isBootFinished)
    {
        deltaTime = millis() - prevTime;
        prevTime = millis();
        loopTime += deltaTime;

        // Update input TODO: implement

        if (loopTime >= loopTimeMax && shouldContinue)
        {
            Color bg = systemPalette[0];
            tft.fillScreen(tft.color565(bg.r, bg.g, bg.b)); // Clear screen with background color

            // main loop
            if (duk_peval_string(ctx, "__bitsybox_on_update__();") != 0)
            {
                Serial.printf("Update Boot Menu Error: %s\n", duk_safe_to_string(ctx, -1));
            }
            duk_pop(ctx);

            // copy screen buffer texture to screen
            drawingBuffers[0]->pushSprite(0, 0);

            loopTime = 0;

            duk_peval_string(ctx, "__bitsybox_is_boot_finished__");
            isBootFinished = duk_get_boolean(ctx, -1);
            duk_pop(ctx);
        }
    }

    if (isBootFinished)
    {
        duk_peval_string(ctx, "__bitsybox_selected_game_name__");
        duk_pop(ctx);

        duk_peval_string(ctx, "__bitsybox_selected_game__");
        sprintf(gameFilePath, "/games/%s", duk_get_string(ctx, -1));
        duk_pop(ctx);

        duk_peval_string(ctx, "__bitsybox_game_files__.length");
        gameCount = duk_get_int(ctx, -1);
        duk_pop(ctx);
    }

    if (duk_peval_string(ctx, "__bitsybox_on_quit__();") != 0) {
		Serial.printf("Quit Boot Menu Error: %s\n", duk_safe_to_string(ctx, -1));
	}
	duk_pop(ctx);

    duk_destroy_heap(ctx);
}

void gameLoop()
{
    duk_context *ctx = duk_create_heap(NULL, NULL, NULL, NULL, fatalError);

    initBitsySystem(ctx);

    // Timing variables
    int prevTime = millis();
    int deltaTime = 0;
    int loopTime = 0;
    int loopTimeMax = 16;
    int isGameOver = 0;

    loadEngine(ctx);

    shouldContinue = shouldContinue && loadFile(ctx, gameFilePath, "__bitsybox_game_data__");

    duk_peval_string(ctx, "var __bitsybox_is_game_over__ = false;");
    duk_pop(ctx);

    // main loop
	if (duk_peval_string(ctx, "__bitsybox_on_load__(__bitsybox_game_data__, __bitsybox_default_font__);") != 0) {
		Serial.printf("Load Bitsy Error: %s\n", duk_safe_to_string(ctx, -1));
	}
	duk_pop(ctx);

    	if (gameCount > 1) {
		// hack to return to main menu on game end if there's more than one
		duk_peval_string(ctx, "reset_cur_game = function() { __bitsybox_is_game_over__ = true; };");
		duk_pop(ctx);
	}

    while (shouldContinue && !isGameOver)
    {
        deltaTime = millis() - prevTime;
        prevTime = millis();
        loopTime += deltaTime;

        // Update input TODO: implement

        if (loopTime >= loopTimeMax && shouldContinue)
        {
            Color bg = systemPalette[0];
            tft.fillScreen(tft.color565(bg.r, bg.g, bg.b)); // Clear screen with background color

            // main loop
			if (duk_peval_string(ctx, "__bitsybox_on_update__();") != 0) {
				Serial.printf("Update Bitsy Error: %s\n", duk_safe_to_string(ctx, -1));
			}
			duk_pop(ctx);

            // copy screen buffer texture to screen
            drawingBuffers[0]->pushSprite(0, 0);

            loopTime = 0;
        }

        // kind of hacky way to trigger restart
		if (duk_peval_string(ctx, "if (bitsyGetButton(5)) { reset_cur_game(); }") != 0) {
			Serial.printf("Test Restart Game Error: %s\n", duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);

        if (duk_peval_string(ctx, "__bitsybox_is_game_over__") != 0) {
			Serial.printf("Test Game Over Error: %s\n", duk_safe_to_string(ctx, -1));
		}
		isGameOver = duk_get_boolean(ctx, -1);
		duk_pop(ctx);
    }

    if (duk_peval_string(ctx, "__bitsybox_on_quit__();") != 0) {
		Serial.printf("Quit Bitsy Error: %s\n", duk_safe_to_string(ctx, -1));
	}
	duk_pop(ctx);

    duk_destroy_heap(ctx);
}

void setup()
{
    Serial.begin(115200); // Start the serial communication
    Serial.println("~*~*~ bitsybox ~*~*~");
    Serial.println("[duktape version 2.6.0]");

    // Initialize LittleFS
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed!");
        return; // Exit if filesystem initialization fails
    }

    // Initialize system palette
    systemPalette[0] = (Color){255, 0, 0}; // Red
    systemPalette[1] = (Color){0, 255, 0}; // Green
    systemPalette[2] = (Color){0, 0, 255}; // Blue

    // Initialize TFT display
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    // Set up initial drawing buffers
    drawingBuffers[0] = new TFT_eSprite(&tft);
    drawingBuffers[0]->createSprite(screenSize, screenSize);
    drawingBuffers[1] = new TFT_eSprite(&tft);
    drawingBuffers[1]->createSprite(textboxWidth, textboxHeight);
}

void loop()
{
    if (shouldContinue)
    {
        bootMenu();
        gameLoop();
    }
}

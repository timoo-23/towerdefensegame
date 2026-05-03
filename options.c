#include "raylib.h"
#include "options.h"
#include <stdio.h>

// Settings variables
static int selected = 0;

static float masterVolume = 0.8f;
static float musicVolume  = 0.8f;
static float sfxVolume    = 0.8f;
static int textSpeed      = 3;   // 1 slow, 5 fast

static Music *musicStream = NULL;


void InitOptions(Music *music)
{
    musicStream = music;
    SetMasterVolume(masterVolume);
    if (musicStream) SetMusicVolume(*musicStream, musicVolume);
}

// Update settings
bool UpdateOptions()
{
    // Navigate selection
    if (IsKeyPressed(KEY_UP)) selected--;
    if (IsKeyPressed(KEY_DOWN)) selected++;

    if (selected < 0) selected = 4;
    if (selected > 4) selected = 0;

    // Adjust sliders
    float step = 0.05f;

    if (selected == 0) // Master Volume
    {
        if (IsKeyDown(KEY_LEFT))  masterVolume -= step;
        if (IsKeyDown(KEY_RIGHT)) masterVolume += step;
        SetMasterVolume(masterVolume); // Apply master volume
    }

    if (selected == 1) // Music Volume
    {
        if (IsKeyDown(KEY_LEFT))  musicVolume -= step;
        if (IsKeyDown(KEY_RIGHT)) musicVolume += step;
        if (musicStream) SetMusicVolume(*musicStream, musicVolume); // Apply music volume
    }

    if (selected == 2) // SFX Volume
    {
        if (IsKeyDown(KEY_LEFT))  sfxVolume -= step;
        if (IsKeyDown(KEY_RIGHT)) sfxVolume += step;
    }

    if (selected == 3) // Text Speed
    {
        if (IsKeyPressed(KEY_LEFT)) textSpeed--;
        if (IsKeyPressed(KEY_RIGHT)) textSpeed++;
    }

    if(selected == 4 && IsKeyPressed(KEY_ENTER))
    {
        return true;  // signal "go back to menu"
    }

    return false;

    // Clamp values
    if (masterVolume < 0) masterVolume = 0;
    if (masterVolume > 1) masterVolume = 1;

    if (musicVolume < 0) musicVolume = 0;
    if (musicVolume > 1) musicVolume = 1;

    if (sfxVolume < 0) sfxVolume = 0;
    if (sfxVolume > 1) sfxVolume = 1;

    if (textSpeed < 1) textSpeed = 1;
    if (textSpeed > 5) textSpeed = 5;
}

float GetMyMasterVolume() { return masterVolume; }
float GetMusicVolume()  { return musicVolume; }
float GetSFXVolume()    { return sfxVolume; }
int   GetTextSpeed()    { return textSpeed; }


// Draw slider

void DrawSlider(int x, int y, float value, int width)
{
    DrawRectangle(x, y, width, 6, DARKGRAY);
    DrawRectangle(x, y, width * value, 6, GREEN);
}


// Draw options screen

void DrawOptions(float scaleX, float scaleY)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float boxWidth  = 520 * scaleX;
    float boxHeight = 380 * scaleY;
    Rectangle box = {
        (screenWidth - boxWidth)/2,
        (screenHeight - boxHeight)/2 + 50*scaleY,
        boxWidth,
        boxHeight
    };

    DrawRectangleRounded(box, 0.2f, 10, (Color){40,40,40,180});
    DrawRectangleRoundedLines(box, 0.2f, 10,LIGHTGRAY);

    const char *title = "SETTINGS";
    int fontSize = 60 * scaleY;
    int textWidth = MeasureText(title, fontSize);
    int titleX = (screenWidth - textWidth)/2;
    int titleY = box.y - fontSize - 10;
    DrawText(title, titleX, titleY, fontSize, GREEN);


    int paddingX = 40 * scaleX;
    int paddingY = 40 * scaleY;
    int startX = box.x + paddingX;
    int startY = box.y + paddingY;
    int spacing = 70 * scaleY;

    Color c1 = selected == 0 ? YELLOW : WHITE;
    Color c2 = selected == 1 ? YELLOW : WHITE;
    Color c3 = selected == 2 ? YELLOW : WHITE;
    Color c4 = selected == 3 ? YELLOW : WHITE;
    Color c5 = selected == 4 ? YELLOW : WHITE;

    DrawText("Master Volume", startX, startY, 28*scaleY, c1);
    DrawSlider(startX + 260*scaleX, startY + 12, masterVolume, 190*scaleX);

    DrawText("Music Volume", startX, startY + spacing, 28*scaleY, c2);
    DrawSlider(startX + 260*scaleX, startY + spacing + 12, musicVolume, 190*scaleX);

    DrawText("SFX Volume", startX, startY + spacing*2, 28*scaleY, c3);
    DrawSlider(startX + 260*scaleX, startY + spacing*2 + 12, sfxVolume, 190*scaleX);

    DrawText("Text Speed", startX, startY + spacing*3, 28*scaleY, c4);
    char speedText[10];
    sprintf(speedText, "%d", textSpeed);
    DrawText(speedText, startX + 320*scaleX, startY + spacing*3, 28*scaleY, WHITE);

    DrawText("Back", startX, startY + spacing*4, 28*scaleY, c5);
}
//Code Created by 林义信
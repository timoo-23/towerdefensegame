#include "raylib.h"
#include "menu.h"
#include <math.h>
static int selectedOption = 0;
static Texture2D menuBackground;
static bool menuBgLoaded = false;

void UpdateMenu()
{   
    menuBackground = LoadTexture("assets/menu_bg.png");
    menuBgLoaded = true;

    if (IsKeyPressed(KEY_UP))
        selectedOption--;

    if (IsKeyPressed(KEY_DOWN))
        selectedOption++;

    if (selectedOption < 0)
        selectedOption = 3;

    if (selectedOption > 3)
        selectedOption = 0;
}

void DrawMenu(float scaleX, float scaleY)
{
    if(menuBgLoaded)
    {
        float screenW = GetScreenWidth();
        float screenH = GetScreenHeight();

        float imgW = menuBackground.width;
        float imgH = menuBackground.height;

        // 🔥 aspect ratio scale (IMPORTANT)
        float scale = fmax(screenW / imgW, screenH / imgH);

        float drawW = imgW * scale;
        float drawH = imgH * scale;

        float offsetX = (screenW - drawW) / 2.0f;
        float offsetY = (screenH - drawH) / 2.0f;

        Rectangle src = {0, 0, imgW, imgH};
        Rectangle dst = {offsetX, offsetY, drawW, drawH};

        DrawTexturePro(menuBackground, src, dst, (Vector2){0,0}, 0, WHITE);
        DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.4f));
    }
    else
    {
        ClearBackground(BLACK);
    }
    // ----- TITLE SETTINGS -----
    int fontSize = 60 * scaleY;
    const char *title = "30 DAYS TO CURE";
    
    int textWidth = MeasureText(title, fontSize);
    int posX = (GetScreenWidth() - textWidth) / 2;
    int posY = 120 * scaleY;
    Rectangle titleBox = {
        posX - 20*scaleX,
        posY - 10*scaleY,
        textWidth + 50*scaleX,
        fontSize + 10*scaleY
    };
    // ----- GLOW / OUTLINE EFFECT -----
    DrawRectangleRounded(titleBox, 0.3f, 10, (Color){30,30,30,160});
    DrawRectangleRoundedLines(titleBox, 0.3f, 10, WHITE);
    DrawText(title, posX-5, posY, fontSize, DARKGREEN);
    DrawText(title, posX+5, posY, fontSize, DARKGREEN);
    DrawText(title, posX, posY-5, fontSize, DARKGREEN);
    DrawText(title, posX, posY+5, fontSize, DARKGREEN);
    
    
    // Main title
    DrawText(title, posX, posY, fontSize, GREEN);

    // ----- MENU BOX -----
    Rectangle menuBox = {300*scaleX, 220*scaleY, 300*scaleX, 220*scaleY};

    DrawRectangleRounded(menuBox, 0.2f, 10, (Color){40,40,40,180});
    DrawRectangleRoundedLines(menuBox, 0.2f, 10, LIGHTGRAY);

    // ----- MENU OPTIONS -----
    Color c1 = selectedOption == MENU_CONTINUE ? YELLOW : WHITE;
    Color c2 = selectedOption == MENU_NEWGAME ? YELLOW : WHITE;
    Color c3 = selectedOption == MENU_OPTIONS ? YELLOW : WHITE;
    Color c4 = selectedOption == MENU_QUIT ? YELLOW : WHITE;

    DrawText("Continue", 380*scaleX, 260*scaleY, 28*scaleY, c1);
    DrawText("New Game", 380*scaleX, 300*scaleY, 28*scaleY, c2);
    DrawText("Options", 380*scaleX, 340*scaleY, 28*scaleY, c3);
    DrawText("Quit", 380*scaleX, 380*scaleY, 28*scaleY, c4);
}

int GetMenuSelection()
{
    if (IsKeyPressed(KEY_ENTER))
        return selectedOption;

    return -1;
}
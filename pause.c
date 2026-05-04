#include "pause.h"
#include "raylib.h"
#include <stdio.h>

static bool paused = false;
static int selectedOption = 0;
static int pauseSelection = -1;

/**
 * Initialize pause system
 */
void InitPause()
{
    paused = false;
    selectedOption = 0;
    pauseSelection = -1;
    printf("Pause system initialized\n");
}

/**
 * Toggle pause on/off
 */
void TogglePause()
{
    paused = !paused;
    selectedOption = 0;
    pauseSelection = -1;
    if (paused)
        printf("Game PAUSED\n");
    else
        printf("Game RESUMED\n");
}

/**
 * Check if game is paused
 */
bool IsPaused()
{
    return paused;
}

/**
 * Update pause menu input
 */
void UpdatePause()
{
    if (!paused) return;
    
    // Navigation
    if (IsKeyPressed(KEY_W))
    {
        selectedOption--;
        if (selectedOption < 0)
            selectedOption = PAUSE_OPTION_COUNT - 1;
    }
    
    if (IsKeyPressed(KEY_S))
    {
        selectedOption++;
        if (selectedOption >= PAUSE_OPTION_COUNT)
            selectedOption = 0;
    }
    
    // Selection
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))
    {
        pauseSelection = selectedOption;
        printf("Pause menu selection: %d\n", pauseSelection);
    }
}

/**
 * Get current pause menu selection
 */
int GetPauseSelection()
{
    int temp = pauseSelection;
    pauseSelection = -1; // Reset after reading
    return temp;
}

/**
 * Reset pause selection
 */
void ResetPauseSelection()
{
    pauseSelection = -1;
}

/**
 * Draw pause overlay and menu
 */
void DrawPause(float scaleX, float scaleY)
{
    if (!paused) return;
    
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    // Dark overlay
    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.7f));
    
    // Panel dimensions
    int panelW = 500;
    int panelH = 380;
    int panelX = screenW / 2 - panelW / 2;
    int panelY = screenH / 2 - panelH / 2;
    
    // Panel background
    DrawRectangleRounded((Rectangle){panelX, panelY, panelW, panelH}, 0.15f, 10, (Color){40, 40, 40, 220});
    DrawRectangleRoundedLines((Rectangle){panelX, panelY, panelW, panelH}, 0.15f, 10, LIGHTGRAY);
    
    // Title
    DrawText("PAUSED", panelX + panelW/2 - 80, panelY + 30, 50, YELLOW);
    
    // Divider line
    DrawLineEx((Vector2){panelX + 30, panelY + 100}, (Vector2){panelX + panelW - 30, panelY + 100}, 2, LIGHTGRAY);
    
    // Menu options
    const char *options[] = {"Resume", "Options", "Main Menu"};
    
    for (int i = 0; i < PAUSE_OPTION_COUNT; i++)
    {
        Color color = (selectedOption == i) ? YELLOW : WHITE;
        int yPos = panelY + 140 + i * 60;
        
        // Background for selected option
        if (selectedOption == i)
        {
            DrawRectangle(panelX + 40, yPos - 5, panelW - 80, 45, Fade(YELLOW, 0.2f));
            DrawRectangleLines(panelX + 40, yPos - 5, panelW - 80, 45, YELLOW);
        }
        
        DrawText(options[i], panelX + 70, yPos, 35, color);
    }
    
    // Instructions
    DrawText("W/S: Navigate | SPACE/ENTER: Select | ESC: Resume", 
             panelX + 20, panelY + panelH - 40, 16, LIGHTGRAY);
}

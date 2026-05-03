#include "raylib.h"
#include "vn.h"
#include "menu.h"
#include "options.h"
#include "defense.h"
#include <stdio.h>

typedef enum {
    STATE_MENU,
    STATE_VN,
    STATE_OPTIONS,
    STATE_DEFENSE,
    STATE_BRIEFING,
    STATE_END,
    STATE_VICTORY
} GameState;

int main()
{
    int screenWidth = 1920;
    int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "30 Days to Cure");
    SetTargetFPS(60);
    InitAudioDevice();
    LoadBackground("assets/Postapocalypce1/Bright/postapocalypse1.png");
    InitDefense();
    Music bgm = LoadMusicStream("assets/bgm.ogg");
    PlayMusicStream(bgm);
    InitOptions(&bgm);
    float scaleX = screenWidth / 900.0f;
    float scaleY = screenHeight / 600.0f;
    float briefingTimer = 0;
    int currentDay = 1;
    GameState state = STATE_MENU;
    int deathChoice = 0;
    
    printf("About to load script...\n");
    LoadScript("script/day1.txt");
    printf("Script load finished\n");

    while (!WindowShouldClose())
    {
        //  UPDATE STATE

        if (state == STATE_MENU)
        {
            printf("STATE: MENU\n");
            UpdateMenu();

            int selection = GetMenuSelection();
            if (selection == MENU_NEWGAME)
            {
                printf("Starting new game...\n");
                state = STATE_VN;
            }
            else if (selection == MENU_OPTIONS)
                state = STATE_OPTIONS;

            else if (selection == MENU_QUIT)
                break;
        }
        else if (state == STATE_VN)
        {
            printf("STATE: VN\n");
            UpdateDialogue();

            if(IsDialogueFinished())
            {
                printf("VN finished, getting effects...\n");
                VNEffects effects = GetVNEffects();

                printf("Resetting defense...\n");
                ResetDefense();             
                printf("Preparing defense...\n");
                PrepareDefenseFromVN(effects);            

                printf("Going to briefing...\n");
                state = STATE_BRIEFING;
                briefingTimer = 0;
            }
        }
        else if(state == STATE_DEFENSE)
        {
            printf("STATE: DEFENSE\n");
            UpdateDefense();

            if(IsGameWon())
            {
                printf("Game Won! Returning to menu...\n");
                state = STATE_VICTORY;
            }

            if(IsPlayerDead())
            {
                // handle input ONLY when fade finished
                if(GetDeathFade() >= 1.0f)
                {
                    if(IsKeyPressed(KEY_W)) deathChoice--;
                    if(IsKeyPressed(KEY_S)) deathChoice++;

                    if(deathChoice < 0) deathChoice = 0;
                    if(deathChoice > 1) deathChoice = 1;

                    if(IsKeyPressed(KEY_ENTER))
                    {
                        if(deathChoice == 0)
                        {
                            // RETRY
                            ResetAfterDeath();
                            state = STATE_DEFENSE;
                        }
                        else
                        {
                            // MENU
                            ResetAfterDeath();
                            state = STATE_MENU;
                        }
                    }
                }
            }

            else if(IsDefenseFinished())
            {
                printf("Defense finished, advancing day...\n");
                currentDay++;
                SetCurrentDay(currentDay);
                
                char filename[50];
                sprintf(filename, "script/day%d.txt", currentDay);
                printf("Loading: %s\n", filename);

                LoadScript(filename);
                ResetVNEffects(); 

                state = STATE_VN;
            }
        }
        else if(state == STATE_BRIEFING)
        {
            printf("STATE: BRIEFING\n");
            briefingTimer += GetFrameTime();

            if(briefingTimer > 2.5f)
            {
                printf("Briefing done, starting wave %d\n", GetCurrentWave());
                StartWave(GetCurrentWave());
                state = STATE_DEFENSE;
            }
        }
        else if(state == STATE_VICTORY)
        {
            DrawDefense(scaleX, scaleY); // this already draws victory screen

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                ResetDefense();
                state = STATE_MENU;
            }
        }
        else if (state == STATE_OPTIONS)
        {
            if(UpdateOptions())
            {
                state = STATE_MENU; 
            }
        }
        // DRAW

        BeginDrawing();
        ClearBackground(BLACK);
        if (state == STATE_MENU)
            DrawMenu(scaleX, scaleY);

        else if (state == STATE_OPTIONS)
            DrawOptions(scaleX, scaleY);

        else if (state == STATE_VN)
            DrawDialogue(scaleX, scaleY);

        else if(state == STATE_DEFENSE)
        {
            DrawDefense(scaleX, scaleY);
            if(IsPlayerDead())
            {
                int w = GetScreenWidth();
                int h = GetScreenHeight();

                float fade = GetDeathFade();

                DrawRectangle(0, 0, w, h, Fade(BLACK, fade));

                if(fade > 0.7f)
                {
                    DrawText("YOU DIED", w/2 - 120, h/2 - 120, 50, RED);

                    char waveText[64];
                    sprintf(waveText, "Wave Reached: %d", GetFinalWave());

                    DrawText(waveText, w/2 - 140, h/2 - 40, 30, WHITE);

                    Color retryColor = (deathChoice == 0) ? YELLOW : WHITE;
                    Color menuColor  = (deathChoice == 1) ? YELLOW : WHITE;

                    DrawText("Retry", w/2 - 50, h/2 + 20, 30, retryColor);
                    DrawText("Main Menu", w/2 - 90, h/2 + 60, 30, menuColor);
                }
            }
        }
        else if(state == STATE_BRIEFING)
        {
            DrawText("PREPARE YOURSELF...", 800, 500, 40, WHITE);
        }
        else if(state == STATE_VICTORY)
            DrawDefense(scaleX, scaleY);
        EndDrawing();
    }
    CloseAudioDevice();
    UnloadBackground();
    CloseWindow();
    
    return 0;  
}
//Code Created by 林义信
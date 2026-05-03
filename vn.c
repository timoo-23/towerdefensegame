#include "raylib.h"
#include "vn.h"
#include "options.h"
#include <stdio.h>
#include <string.h>

#define MAX_LINE 512

static char currentLine[MAX_LINE];
static int visibleChars = 0;
static FILE *scriptFile = NULL;
static bool dialogueFinished = false;
static float textTimer = 0.0f;

static VNEffects vnEffects = {0};

static int currentDayNum = 1;

void SetCurrentDay(int day)
{
    currentDayNum = day;
}

int GetCurrentDay()
{
    return currentDayNum;
}

VNEffects GetVNEffects()
{
    return vnEffects;
}

void ResetVNEffects()
{
    vnEffects = (VNEffects){
        .bonusDamage = 0,
        .bonusHP = 0,
        .reloadMultiplier = 1.0f,     
        .speedMultiplier = 1.0f,      
        .weaponType = 0
    };
}

static bool inChoice = false;
static char choice1[128];
static char choice2[128];
static char choice3[128];
static int selectedChoice = 0;
static int choiceCount = 0;

// Background texture
static Texture2D backgroundTexture = {0};
static bool backgroundLoaded = false;

void LoadBackground(const char *filename)
{
    Image img = LoadImage(filename);
    if (img.data != NULL)
    {
        backgroundTexture = LoadTextureFromImage(img);
        UnloadImage(img);
        backgroundLoaded = true;
    }
}

void UnloadBackground()
{
    if (backgroundLoaded)
    {
        UnloadTexture(backgroundTexture);
        backgroundLoaded = false;
    }
}

// Load script
void LoadScript(const char *filename)
{
    printf("Loading script: %s\n", filename);

    scriptFile = fopen(filename, "r");

    if (scriptFile == NULL)
    {
        printf("FAILED TO LOAD SCRIPT\n");
        dialogueFinished = true;
        currentLine[0] = '\0';
        return;
    }

    printf("File opened successfully\n");

    // Safe read
    while (1)
    {
        if (fgets(currentLine, MAX_LINE, scriptFile) == NULL)
        {
            printf("Script empty or read failed\n");
            dialogueFinished = true;
            fclose(scriptFile);
            scriptFile = NULL;
            return;
        }

        if (strlen(currentLine) > 1)
            break;
    }

    currentLine[strcspn(currentLine, "\n")] = '\0';

    visibleChars = 0;
    textTimer = 0;
    dialogueFinished = false;

    printf("FIRST LINE: %s\n", currentLine);
}

// Update dialogue
void UpdateDialogue()
{
    int speed = GetTextSpeed();;
    if (!scriptFile || dialogueFinished) return;
    float delay;
    switch(speed)
    {
        case 1: delay = 0.08f; break;
        case 2: delay = 0.06f; break;
        case 3: delay = 0.045f; break;
        case 4: delay = 0.03f; break;
        case 5: delay = 0.015f; break;
        default: delay = 0.045f;
    }

    textTimer += GetFrameTime();

    if (visibleChars < strlen(currentLine))
    {
        visibleChars += 1;   // always grow
    }

    if (strncmp(currentLine, "EFFECT:", 7) == 0)
    {
        char type[50];
        int value;

        sscanf(currentLine, "EFFECT: %s %d", type, &value);

        if (strcmp(type, "damage") == 0)
            vnEffects.bonusDamage += value;

        else if (strcmp(type, "reload") == 0)
            vnEffects.reloadMultiplier += value / 100.0f;

        else if (strcmp(type, "hp") == 0)
            vnEffects.bonusHP += value;

        // skip displaying this line
        do {
            if (fgets(currentLine, MAX_LINE, scriptFile) == NULL)
            {
                currentLine[strcspn(currentLine, "\n")] = '\0';
                dialogueFinished = true;
                fclose(scriptFile);
                return;
            }
        } while (strlen(currentLine) <= 1); // 🔥 skip empty lines

    visibleChars = 0;
    textTimer = 0;

    printf("NEXT LINE: %s\n", currentLine); // debug

        return;
    }

    visibleChars = strlen(currentLine);

    if(inChoice)
    {
        if(IsKeyPressed(KEY_W)) selectedChoice--;
        if(IsKeyPressed(KEY_S)) selectedChoice++;
        
        if(selectedChoice < 0) selectedChoice = 0;
        if(selectedChoice >= choiceCount) selectedChoice = choiceCount - 1;

        if(IsKeyPressed(KEY_SPACE))
        {
            // Get the selected choice text
            char *selectedText;

            if(selectedChoice == 0) selectedText = choice1;
            else if(selectedChoice == 1) selectedText = choice2;
            else selectedText = choice3;
            
            int day = GetCurrentDay();
            
            // Parse the tag and apply effects
            if(strncmp(selectedText, "[DAMAGE]", 8) == 0)
            {
                vnEffects.bonusDamage += 3;
            }
            else if(strncmp(selectedText, "[HEALTH]", 8) == 0)
            {
                vnEffects.bonusHP += 40;
            }
            else if(strncmp(selectedText, "[LOSE_DAMAGE]", 13) == 0)
            {
                vnEffects.bonusDamage -= 1;
            }
            else if(strncmp(selectedText, "[LOSE_SPEED]", 12) == 0)
            {
                vnEffects.speedMultiplier *= 0.8f;
            }
            else if(strncmp(selectedText, "[FOCUS_FIRE]", 12) == 0)
            {
                vnEffects.bonusDamage += 4;
            }
            else if(strncmp(selectedText, "[SURVIVE_LONGER]", 16) == 0)
            {
                vnEffects.bonusHP += 50;
            }
            else if(strncmp(selectedText, "[RELOAD_BOOST]", 14) == 0)
            {
                vnEffects.reloadMultiplier *= 0.7f; 
            }
            else if(strncmp(selectedText, "[AMMO_BOOST]", 12) == 0)
            {
                vnEffects.bonusHP += 30;  
            }
            else if(strncmp(selectedText, "[POWER_SPIKE]", 13) == 0)
            {
                vnEffects.bonusDamage += 5;
            }
            else if(strncmp(selectedText, "[BALANCED]", 10) == 0)
            {
                vnEffects.bonusDamage += 2;
                vnEffects.bonusHP += 30;
            }
            // WEAPON CHOICES (Day 7)
            else if(strncmp(selectedText, "[MINIGUN]", 9) == 0)
            {
                vnEffects.weaponType = 2;  // 2.png
                vnEffects.bonusDamage -= 2;  // Lower damage
                vnEffects.reloadMultiplier *= 0.5f;  // Much faster fire rate
            }
            else if(strncmp(selectedText, "[AK47]", 6) == 0)
            {
                vnEffects.weaponType = 6;  // 6.png
                vnEffects.bonusDamage += 1;  // Slight damage boost
                vnEffects.reloadMultiplier *= 0.8f;  // Moderate fire rate
            }
            else if(strncmp(selectedText, "[SNIPER]", 8) == 0)
            {
                vnEffects.weaponType = 5;  // 5.png
                vnEffects.bonusDamage += 8;  // High damage
                vnEffects.reloadMultiplier *= 1.5f;  // Slower fire rate
            }

            inChoice = false;

            // continue reading script
            if (fgets(currentLine, MAX_LINE, scriptFile) != NULL)
            {
                currentLine[strcspn(currentLine, "\n")] = '\0';
                visibleChars = 0;
                textTimer = 0;
            }
            return;
        }
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        if (visibleChars < strlen(currentLine))
        {
            visibleChars = strlen(currentLine);
        }
        else
        {
            if (fgets(currentLine, MAX_LINE, scriptFile) != NULL)
            {
                currentLine[strcspn(currentLine, "\n")] = '\0';
                // CHECK FOR CHOICE
                if(strncmp(currentLine, "CHOICE", 6) == 0)
                {
                    sscanf(currentLine, "CHOICE %d", &choiceCount);

                    if(choiceCount >= 1)
                    {
                        fgets(choice1, MAX_LINE, scriptFile);
                        choice1[strcspn(choice1, "\n")] = '\0';
                    }

                    if(choiceCount >= 2)
                    {
                        fgets(choice2, MAX_LINE, scriptFile);
                        choice2[strcspn(choice2, "\n")] = '\0';
                    }

                    if(choiceCount >= 3)
                    {
                        fgets(choice3, MAX_LINE, scriptFile);
                        choice3[strcspn(choice3, "\n")] = '\0';
                    }

                    fgets(currentLine, MAX_LINE, scriptFile);
                    currentLine[strcspn(currentLine, "\n")] = '\0';

                    inChoice = true;
                    selectedChoice = 0;
                    return;
                }

                visibleChars = 0;
                textTimer = 0;
            }
            else
            {
                dialogueFinished = true;
                fclose(scriptFile);
            }
        }
    }
}

// Draw dialogue
void DrawDialogue(float scaleX, float scaleY)
{   
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Draw background first (if loaded)
    if (backgroundLoaded)
    {
        Rectangle source = {0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height};
        Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
        Vector2 origin = {0, 0};
        
        DrawTexturePro(backgroundTexture, source, dest, origin, 0.0f, WHITE);
    }
    // Dialogue box
    Rectangle box = {
        100 * scaleX,
        screenHeight - 250 * scaleY,
        screenWidth - 200 * scaleX,
        180 * scaleY
    };
    DrawRectangleRounded(box, 0.2f, 10, (Color){40,40,40,200});
    DrawRectangleRoundedLines(box, 0.2f, 10, LIGHTGRAY);

    // Visible text
    char visibleText[MAX_LINE];

    int len = strlen(currentLine);
    if (visibleChars > len) visibleChars = len;

    strncpy(visibleText, currentLine, visibleChars);
    visibleText[visibleChars] = '\0';

    DrawText(visibleText,
             box.x + 40 * scaleX,
             box.y + 40 * scaleY,
             28 * scaleY,
             WHITE);

    if(inChoice)
    {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        Color c1 = (selectedChoice == 0) ? YELLOW : WHITE;
        Color c2 = (selectedChoice == 1) ? YELLOW : WHITE;
        Color c3 = (selectedChoice == 2) ? YELLOW : WHITE;

        // Find where the actual text starts (after the tag)
        const char *texts[3] = {choice1, choice2, choice3};

        for(int i = 0; i < choiceCount; i++)
        {
            Color color = (selectedChoice == i) ? YELLOW : WHITE;

            const char *displayText = texts[i];

            // remove tag [XXXX]
            if(displayText[0] == '[')
            {
                char *endTag = strchr(displayText, ']');
                if(endTag) displayText = endTag + 2;
            }

            DrawText(displayText,
                    screenW/2 - 200,
                    screenH/2 + i * 40,
                    30,
                    color);
        }
    }
}


bool IsDialogueFinished()
{
    return dialogueFinished;
}
//Code Created by 林义信
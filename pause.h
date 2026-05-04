#ifndef PAUSE_H
#define PAUSE_H

#include <stdbool.h>

typedef enum {
    PAUSE_RESUME,
    PAUSE_OPTIONS,
    PAUSE_MENU,
    PAUSE_OPTION_COUNT
} PauseOption;

void InitPause();
void UpdatePause();
void DrawPause(float scaleX, float scaleY);
bool IsPaused();
void TogglePause();
int GetPauseSelection();
void ResetPauseSelection();

#endif

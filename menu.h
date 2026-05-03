#ifndef MENU_H
#define MENU_H

typedef enum {
    MENU_CONTINUE,
    MENU_NEWGAME,
    MENU_OPTIONS,
    MENU_QUIT
} MenuOption;

void UpdateMenu();
void DrawMenu(float scaleX, float scaleY);
int GetMenuSelection();

#endif
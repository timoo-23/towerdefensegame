#ifndef VN_H
#define VN_H

typedef enum {
    WEAPON_PISTOL = 0,
    WEAPON_MINIGUN = 2,
    WEAPON_AK47 = 6,
    WEAPON_SNIPER = 5
} WeaponType;

typedef struct {
    int bonusDamage;
    int bonusHP;
    float reloadMultiplier;
    float speedMultiplier;
    int weaponType;
} VNEffects;

VNEffects GetVNEffects();
void ResetVNEffects();
void SetCurrentDay(int day);
int GetCurrentDay();
void LoadBackground(const char *filename);
void UnloadBackground();
void LoadScript(const char *filename);
void UpdateDialogue();
void DrawDialogue(float scaleX, float scaleY);
bool IsDialogueFinished();

#endif
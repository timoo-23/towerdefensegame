#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <stdbool.h>

// Game save data structure
typedef struct {
    // Progress
    int currentDay;
    int currentWave;
    
    // Player stats
    int playerHP;
    int maxHP;
    int ammo;
    int maxAmmo;
    
    // Currency
    int gold;
    
    // Upgrade levels (0-10)
    int hpLevel;
    int ammoLevel;
    int dmgLevel;
    int reloadLevel;
    int speedLevel;
    
    // Game mechanics values
    float bulletDamage;
    float reloadSpeed;
    float moveSpeed;
    
    // Weapon type
    int weaponType;
} SaveData;

// Function declarations
bool SaveGameState(const char* filename, const SaveData* data);
bool LoadGameState(const char* filename, SaveData* data);
bool SaveFileExists(const char* filename);
void DeleteSaveFile(const char* filename);

#endif

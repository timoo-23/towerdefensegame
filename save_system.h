#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <stdbool.h>

// Game state structure to hold all saveable data
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
    
    // Upgrades
    int hpLevel;
    int ammoLevel;
    int dmgLevel;
    int reloadLevel;
    int speedLevel;
    
    // Game mechanics
    float bulletDamage;
    float reloadSpeed;
    float moveSpeed;
    
    // Weapon type
    int weaponType;
} SaveData;

// Save functions
bool SaveGameState(const char* filename, const SaveData* data);
bool LoadGameState(const char* filename, SaveData* data);
bool SaveFileExists(const char* filename);
void DeleteSaveFile(const char* filename);

#endif

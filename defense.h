#ifndef DEFENSE_H
#define DEFENSE_H

#include "vn.h"

void InitDefense();
void UpdateDefense();
void DrawDefense(float scaleX, float scaleY);
bool IsDefenseFinished();
void ResetDefense();
void PrepareDefenseFromVN(VNEffects effects);
int GetCurrentWave();
void SetWeaponSprite(int weaponType);
void StartWave(int waveIndex);
void LoadWaves();
bool IsPlayerDead();
float GetDeathFade();
int GetFinalWave();
void ResetAfterDeath();
bool IsGameWon();
void DrawVictoryScreen();

// SAVE SYSTEM GETTER FUNCTIONS
int GetPlayerHP();
int GetMaxHP();
int GetAmmo();
int GetMaxAmmo();
int GetGold();
int GetHPLevel();
int GetAmmoLevel();
int GetDmgLevel();
int GetReloadLevel();
int GetSpeedLevel();
float GetBulletDamage();
float GetReloadSpeed();
float GetMoveSpeed();
int GetWeaponType();
void LoadGameFromSave(int hp, int maxhp, int ammo, int maxammo, int gold, int hplvl, int ammolvl, int dmglvl, int reloadlvl, int speedlvl, float bulletdmg, float reloadspd, float movespd, int wpntype);

#endif

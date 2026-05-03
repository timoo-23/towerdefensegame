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
#endif
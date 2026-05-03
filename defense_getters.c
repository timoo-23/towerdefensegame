#include "defense.h"

// These getter functions must be added to defense.c
// Add them at the end of defense.c before the comment line

// Declare extern to access defense.c's static variables
// NOTE: This file shows what functions to add to defense.c
// Copy these function definitions and add them directly to defense.c

/*

ADD THESE FUNCTIONS TO THE END OF defense.c (before //Code Created by 林义信):

// ================= SAVE SYSTEM GETTERS =================

int GetPlayerHP() {
    return playerHP;
}

int GetMaxHP() {
    return maxHP;
}

int GetAmmo() {
    return ammo;
}

int GetMaxAmmo() {
    return maxAmmo;
}

int GetGold() {
    return gold;
}

int GetHPLevel() {
    return hpLevel;
}

int GetAmmoLevel() {
    return ammoLevel;
}

int GetDmgLevel() {
    return dmgLevel;
}

int GetReloadLevel() {
    return reloadLevel;
}

int GetSpeedLevel() {
    return speedLevel;
}

float GetBulletDamage() {
    return bulletDamage;
}

float GetReloadSpeed() {
    return reloadSpeed;
}

float GetMoveSpeed() {
    return moveSpeed;
}

int GetWeaponType() {
    return 0; // Default weapon type
}

void LoadGameFromSave(int hp, int maxhp, int ammo, int maxammo, int gold, int hplvl, int ammolvl, int dmglvl, int reloadlvl, int speedlvl, float bulletdmg, float reloadspd, float movespd, int wpntype)
{
    playerHP = hp;
    maxHP = maxhp;
    ammo = ammo;
    maxAmmo = maxammo;
    gold = gold;
    hpLevel = hplvl;
    ammoLevel = ammolvl;
    dmgLevel = dmglvl;
    reloadLevel = reloadlvl;
    speedLevel = speedlvl;
    bulletDamage = bulletdmg;
    reloadSpeed = reloadspd;
    moveSpeed = movespd;
    shootDelay = (0.3f / reloadSpeed);
}

*/

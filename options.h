#ifndef OPTIONS_H
#define OPTIONS_H

bool UpdateOptions();
void DrawOptions(float scaleX, float scaleY);
void InitOptions(Music *music);
float GetMyMasterVolume();
float GetMusicVolume();
float GetSFXVolume();
int GetTextSpeed();
#endif
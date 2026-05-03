#include "save_system.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * SaveGameState - Serializes the current game state to a file
 * @filename: Path to save file
 * @data: Pointer to SaveData structure containing game state
 * 
 * Returns: true on success, false on failure
 */
bool SaveGameState(const char* filename, const SaveData* data)
{
    if (!filename || !data) {
        printf("ERROR: Invalid save parameters\n");
        return false;
    }
    
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("ERROR: Failed to open save file: %s\n", filename);
        return false;
    }
    
    // Write SaveData structure as binary
    size_t written = fwrite(data, sizeof(SaveData), 1, f);
    if (written != 1) {
        printf("ERROR: Failed to write save data\n");
        fclose(f);
        return false;
    }
    
    fclose(f);
    printf("Game saved successfully to: %s\n", filename);
    return true;
}

/**
 * LoadGameState - Deserializes game state from a file
 * @filename: Path to save file
 * @data: Pointer to SaveData structure to populate
 * 
 * Returns: true on success, false on failure
 */
bool LoadGameState(const char* filename, SaveData* data)
{
    if (!filename || !data) {
        printf("ERROR: Invalid load parameters\n");
        return false;
    }
    
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("ERROR: Failed to open save file: %s\n", filename);
        return false;
    }
    
    // Read SaveData structure from binary
    size_t read = fread(data, sizeof(SaveData), 1, f);
    if (read != 1) {
        printf("ERROR: Failed to read save data\n");
        fclose(f);
        return false;
    }
    
    fclose(f);
    printf("Game loaded successfully from: %s\n", filename);
    return true;
}

/**
 * SaveFileExists - Check if a save file exists
 * @filename: Path to save file
 * 
 * Returns: true if file exists, false otherwise
 */
bool SaveFileExists(const char* filename)
{
    if (!filename) return false;
    
    FILE *f = fopen(filename, "rb");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

/**
 * DeleteSaveFile - Removes a save file
 * @filename: Path to save file
 */
void DeleteSaveFile(const char* filename)
{
    if (!filename) return;
    
    if (remove(filename) == 0) {
        printf("Save file deleted: %s\n", filename);
    } else {
        printf("ERROR: Failed to delete save file: %s\n", filename);
    }
}

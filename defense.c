#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "defense.h"
#include "vn.h"

#define MAX_ZOMBIES 50
#define MAX_BULLETS 50
#define NUM_VARIATIONS 3
#define MAX_FRAMES 3
#define MAX_STATES 4

#define MAX_SPAWNS 50
#define MAX_WAVES 30
#define MAX_ENEMY_BULLETS 50
#define MAX_HIT_EFFECTS 50
// ================= ENUM =================

typedef enum {
    STATE_IDLE,
    STATE_WALK,
    STATE_ATTACK,
    STATE_DYING
} AnimState;

// ================= STRUCTS =================

typedef struct Zombie {
    Vector2 position;
    float speed;
    int type;
    int hp;
    bool active;

    float fireRate;   
    float shootTimer; 

    AnimState state;

    int deathFrame;
    float deathTimer;
} Zombie;

typedef struct Bullet {
    Vector2 position;
    Vector2 direction;
    float speed;
    bool active;
} Bullet;

typedef struct SpawnInfo {
    char type[20];
    int amount;
    float speed;
    int extraHP;
    float fireRate;
} SpawnInfo;

typedef struct GameWave {
    int spawnCount;
    SpawnInfo spawns[MAX_SPAWNS];
} GameWave;

typedef struct EnemyBullet {
    Vector2 position;
    Vector2 direction;
    float speed;
    bool active;
} EnemyBullet;

typedef struct HitEffect {
    Vector2 position;
    float timer;
    bool active;
} HitEffect;

// ================= GLOBALS =================

static Zombie zombies[MAX_ZOMBIES];
static Bullet bullets[MAX_BULLETS];
static GameWave waves[MAX_WAVES];

static Texture2D playerAnim[MAX_STATES][MAX_FRAMES];
static Texture2D zombieAnim[NUM_VARIATIONS][MAX_STATES][MAX_FRAMES];

static int playerFrame = 0;
static float playerAnimTimer = 0;

static float zombieAnimTimer = 0;
static int zombieFrames[MAX_ZOMBIES];
static float animSpeed = 0.15f;

static AnimState playerState = STATE_IDLE;

static int totalWaves = 0;
static int currentWave = 0;

static int bulletDamage = 2;
static float reloadSpeed = 1.0f;
static float moveSpeed = 150.0f;

static int hpLevel = 0;
static int ammoLevel = 0;
static int dmgLevel = 0;
static int reloadLevel = 0;
static int speedLevel = 0;

static Texture2D bulletTex;

static Vector2 playerPos;

static EnemyBullet enemyBullets[MAX_ENEMY_BULLETS];
static float enemyShootTimer[MAX_ZOMBIES];
static HitEffect hitEffects[MAX_HIT_EFFECTS];

// sizes
static float zombieSize = 60;
static float playerSize = 50;
static float bulletWidth = 15;
static float bulletHeight = 8;

// systems
static bool waveFinished = false;
static bool inShop = false;
static bool defenseInitialized = false;

// player stats
static int playerHP = 100;
static int maxHP = 100;

// shooting
static float shootCooldown = 0;
static float shootDelay = 0.3f;
static int ammo = 10;
static int maxAmmo = 10;

// currency
static int gold = 0;

static bool waveTransition = false;
static float waveTransitionTimer = 0.0f;
static float waveTransitionDuration = 2.0f;

//weapons
static Texture2D bulletSprite = {0};

static bool isDead = false;
static float deathFade = 0.0f;
static int finalWave = 0;

static bool gameWon = false;

static Texture2D defenseBackground;
static bool defenseBgLoaded = false;

void SetBulletSprite(const char *filename)
{
    if (bulletSprite.id > 0)
    {
        UnloadTexture(bulletSprite);
    }

    bulletSprite = LoadTexture(filename);

    if (bulletSprite.id == 0)
    {
        printf("FAILED TO LOAD BULLET SPRITE: %s\n", filename);
    }
    else
    {
        printf("LOADED BULLET SPRITE: %s\n", filename);
    }
}

bool IsPlayerDead()
{
    return isDead;
}

float GetDeathFade()
{
    return deathFade;
}

int GetFinalWave()
{
    return finalWave;
}

void ResetZombies()
{
    for(int i = 0; i < MAX_ZOMBIES; i++)
        zombies[i].active = false;
}

void ResetBullets()
{
    for(int i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = false;
}

void ResetAfterDeath()
{
    playerHP = 100;
    currentWave = 0;

    currentWave=0;
    ResetZombies();
    ResetBullets();

    isDead = false;
    deathFade = 0.0f;
    StartWave(0);
}
void DrawVictoryScreen();

// ================= WAVE LOADER =================

void LoadWaves()
{
    totalWaves = 0;

    FILE *f = fopen("waves.txt", "r");
    if (!f)
    {
        printf("FAILED TO LOAD waves.txt\n");
        return;
    }

    char word[1024];

    while (fscanf(f, "%s", word) != EOF)
    {
        if (strcmp(word, "wave") == 0)
        {
            fscanf(f, "%*d");

            GameWave *w = &waves[totalWaves];
            w->spawnCount = 0;

            while (1)
            {
                long pos = ftell(f);

                if (fscanf(f, "%s", word) != 1)
                    break;

                if (strcmp(word, "wave") == 0)
                {
                    fseek(f, pos, SEEK_SET); // safe rewind
                    break;
                }

                if (w->spawnCount >= MAX_SPAWNS) break;
                SpawnInfo *s = &w->spawns[w->spawnCount++];

                strcpy(s->type, word);

                fscanf(f, "%d %f %d %f",
                    &s->amount,
                    &s->speed,
                    &s->extraHP,
                    &s->fireRate);
            }

            totalWaves++;
        }
    }

    fclose(f);

    printf("Loaded %d waves\n", totalWaves);
}

// ================= START WAVE =================

void StartWave(int waveIndex)
{
    for(int i=0;i<MAX_ZOMBIES;i++)
        zombies[i].active = false;

    GameWave *w = &waves[waveIndex];
    int zIndex = 0;
    
    // Fix: Loop through ALL spawn types in the wave
    for(int i=0;i<w->spawnCount && zIndex<MAX_ZOMBIES;i++)
    {
        SpawnInfo *s = &w->spawns[i];
    
        for(int j=0;j<s->amount && zIndex<MAX_ZOMBIES;j++)
        {
            // FULL INIT - this prevents spazzing/crashes
            zombies[zIndex] = (Zombie){
                .position = (Vector2){GetRandomValue(0,1920), GetRandomValue(0,1080)},
                .speed = s->speed,
                .type = 0,
                .hp = 1 + s->extraHP,
                .active = true,
                .state = STATE_WALK,
                .fireRate = s->fireRate,
                .shootTimer = 0,
                .deathFrame = 0,
                .deathTimer = 0
            };

            if(strcmp(s->type,"tank")==0)
            {
                zombies[zIndex].type = 1;
                zombies[zIndex].hp = 5 + s->extraHP;
            }
            else if(strcmp(s->type,"ranged")==0)
            {
                zombies[zIndex].type = 2;
                zombies[zIndex].hp = 2 + s->extraHP;
            }

            zIndex++;
        }
    }

    waveFinished = false;
}

// ================= INIT =================

void InitDefense()
{   
    printf("InitDefense START\n");
    if(defenseInitialized) return;
    defenseInitialized = true;
    defenseBackground = LoadTexture("assets/defense_bg.png");
    defenseBgLoaded = true;
    // ZOMBIE
    printf("Loading zombie textures...\n");
    zombieAnim[0][STATE_IDLE][0] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Idle/0_Zombie_Villager_Idle_000.png");
    zombieAnim[0][STATE_IDLE][1] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Idle/0_Zombie_Villager_Idle_001.png");
    zombieAnim[0][STATE_IDLE][2] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Idle/0_Zombie_Villager_Idle_002.png");

    zombieAnim[0][STATE_WALK][0] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Walking/0_Zombie_Villager_Walking_000.png");
    zombieAnim[0][STATE_WALK][1] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Walking/0_Zombie_Villager_Walking_001.png");
    zombieAnim[0][STATE_WALK][2] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Walking/0_Zombie_Villager_Walking_002.png");

    zombieAnim[0][STATE_DYING][0] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Dying/0_Zombie_Villager_Dying_000.png");
    zombieAnim[0][STATE_DYING][1] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Dying/0_Zombie_Villager_Dying_001.png");
    zombieAnim[0][STATE_DYING][2] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Dying/0_Zombie_Villager_Dying_002.png");

    zombieAnim[0][STATE_ATTACK][0] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Throwing/0_Zombie_Villager_Throwing_000.png");
    zombieAnim[0][STATE_ATTACK][1] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Throwing/0_Zombie_Villager_Throwing_001.png");
    zombieAnim[0][STATE_ATTACK][2] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Throwing/0_Zombie_Villager_Throwing_002.png");
    
    // PLAYER
    printf("Loading player textures...\n");
    playerAnim[STATE_IDLE][0] = LoadTexture("assets/Bloody_Alchemist_1/PNG/PNG Sequences/Idle/0_Bloody_Alchemist_Idle_000.png");
    playerAnim[STATE_IDLE][1] = LoadTexture("assets/Bloody_Alchemist_1/PNG/PNG Sequences/Idle/0_Bloody_Alchemist_Idle_001.png");
    playerAnim[STATE_IDLE][2] = LoadTexture("assets/Bloody_Alchemist_1/PNG/PNG Sequences/Idle/0_Bloody_Alchemist_Idle_002.png");

    playerAnim[STATE_WALK][0] = LoadTexture("assets/Bloody_Alchemist_1/PNG/PNG Sequences/Walking/0_Bloody_Alchemist_Walking_000.png");
    playerAnim[STATE_WALK][1] = LoadTexture("assets/Bloody_Alchemist_1/PNG/PNG Sequences/Walking/0_Bloody_Alchemist_Walking_001.png");
    playerAnim[STATE_WALK][2] = LoadTexture("assets/Bloody_Alchemist_1/PNG/PNG Sequences/Walking/0_Bloody_Alchemist_Walking_002.png");
    printf("Loading bulletTex...\n");
    bulletTex = LoadTexture("assets/5 Bullets/1.png");
    playerFrame = 0;
    playerAnimTimer = 0;
    zombieAnimTimer = 0;
    playerState = STATE_IDLE;
    for(int t=1;t<NUM_VARIATIONS;t++)
        for(int s=0;s<MAX_STATES;s++)
            for(int f=0;f<MAX_FRAMES;f++)
                zombieAnim[t][s][f] = zombieAnim[0][s][f];


    printf("Setting bullet sprite...\n");
    SetBulletSprite("assets/5 Bullets/1.png");
    


    playerPos = (Vector2){960,540};

    printf("Init arrays...\n");
    for(int i=0;i<MAX_BULLETS;i++)
        bullets[i].active=false;

    printf("Init enemy bullets...\n");
    for(int i=0;i<MAX_ENEMY_BULLETS;i++)
        enemyBullets[i].active = false;

    printf("Init zombies...\n");
    for(int i=0;i<MAX_ZOMBIES;i++)
    {
        zombies[i].active = false;
        enemyShootTimer[i] = 0.0f;  // Safety init
        zombieFrames[i]=0;
    }
    printf("Init hit effects...\n");
    for(int i=0;i<MAX_HIT_EFFECTS;i++)
        hitEffects[i].active = false;
    printf("Loading waves...\n");
    LoadWaves();
    printf("InitDefense END\n");
}

void ResetDefense()
{   
    gameWon = false;
    playerPos = (Vector2){960,540};
    playerHP = maxHP;
    ammo = maxAmmo;
    gold = 0;
    inShop = false;
    waveFinished = false;
    shootCooldown = 0;
    waveTransition = false;
    waveTransitionTimer = 0.0f;
    for(int i=0;i<MAX_BULLETS;i++)
        bullets[i].active = false;
    for(int i=0;i<MAX_ENEMY_BULLETS;i++)
        enemyBullets[i].active = false;
    StartWave(0);
}

void PrepareDefenseFromVN(VNEffects effects)
{
    bulletDamage += effects.bonusDamage;
    maxHP += effects.bonusHP;
    if (effects.reloadMultiplier > 0) reloadSpeed *= effects.reloadMultiplier;
    if (effects.speedMultiplier > 0) moveSpeed *= effects.speedMultiplier;
    shootDelay = (0.3f / reloadSpeed);
    playerHP = maxHP;

    if(effects.weaponType == 2)  // Minigun
    {
        SetBulletSprite("assets/5 Bullets/2.png");
    }
    else if(effects.weaponType == 6)  // AK-47
    {
        SetBulletSprite("assets/5 Bullets/6.png");
    }
    else if(effects.weaponType == 5)  // Sniper
    {
        SetBulletSprite("assets/5 Bullets/5.png");
    }
}


// ================= UPDATE =================

void UpdateDefense()
{
    //game over
    if(playerHP <= 0 && !isDead)
    {
        isDead = true;
        deathFade = 0.0f;
        finalWave = currentWave;
    }
    if(isDead)
    {
        deathFade += GetFrameTime() * 0.5f;
        if(deathFade > 1.0f) deathFade = 1.0f;
        return;
    }
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    shootCooldown = 0;
    float dt = GetFrameTime();
    if(zombieAnim[0][0][0].id == 0 || playerAnim[0][0].id == 0 || bulletTex.id == 0)
    {
        // Reload critical textures if corrupted
        zombieAnim[0][0][0] = LoadTexture("assets/Zombie_Villager_1/PNG/PNG Sequences/Idle/0_Zombie_Villager_Idle_000.png");
        playerAnim[STATE_IDLE][0] = LoadTexture("assets/Bloody_Alchemist_1/PNG/PNG Sequences/Idle/0_Bloody_Alchemist_Idle_000.png");
    }

    // ANIMATION
    playerAnimTimer += dt;
    if(playerAnimTimer >= animSpeed)
    {
        playerFrame = (playerFrame + 1) % MAX_FRAMES;
        playerAnimTimer = 0;
    }

    zombieAnimTimer += dt;
    if(zombieAnimTimer >= animSpeed)
    {
        for(int i=0;i<MAX_ZOMBIES;i++)
        {
            if(!zombies[i].active) continue;

            if(zombies[i].state == STATE_DYING)
            {
                // dying uses its own frame system
                continue;
            }

            zombieFrames[i] = (zombieFrames[i] + 1) % MAX_FRAMES;
        }

        zombieAnimTimer = 0;
    }

    // SHOP
    if(inShop)
    {
        if(IsKeyPressed(KEY_ENTER))
        {   
            if(currentWave + 1 < totalWaves)
            {
                currentWave++;
                StartWave(currentWave);
                inShop = false;
            }
            else
            {
                printf("All waves finished\n");
                inShop = false;
                waveFinished = true;
            }
        }
    }

    // PLAYER MOVEMENT
    bool moving = false;

    if(!inShop)  
    {
        if(IsKeyDown(KEY_W)) { playerPos.y -= moveSpeed*dt; moving = true; }
        if(IsKeyDown(KEY_S)) { playerPos.y += moveSpeed*dt; moving = true; }
        if(IsKeyDown(KEY_A)) { playerPos.x -= moveSpeed*dt; moving = true; }
        if(IsKeyDown(KEY_D)) { playerPos.x += moveSpeed*dt; moving = true; }
    }

    playerState = moving ? STATE_WALK : STATE_IDLE;

    // ===== PLAYER BOUNDS =====
    if(playerPos.x < 0) playerPos.x = 0;
    if(playerPos.x > 1920) playerPos.x = 1920;
    if(playerPos.y < 0) playerPos.y = 0;
    if(playerPos.y > 1080) playerPos.y = 1080;

    // SHOOTING
    shootCooldown -= dt;
    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && shootCooldown <= 0 && ammo > 0)
    {
        shootCooldown = shootDelay;
        ammo--;

        for(int i=0;i<MAX_BULLETS;i++)
        {
            if(!bullets[i].active)
            {
                bullets[i].active=true;
                bullets[i].position=playerPos;

                Vector2 mouse=GetMousePosition();
                Vector2 dir={mouse.x-playerPos.x,mouse.y-playerPos.y};

                float len=sqrt(dir.x*dir.x+dir.y*dir.y);
                if(len!=0){dir.x/=len;dir.y/=len;}

                bullets[i].direction=dir;
                bullets[i].speed=700;
                break;
            }
        }
    }

    if(IsKeyPressed(KEY_R)) ammo = maxAmmo;
    if(ammo < 0) ammo = 0;
    // BULLETS
    for(int i=0;i<MAX_BULLETS;i++)
    {
        if(!bullets[i].active) continue;

        bullets[i].position.x+=bullets[i].direction.x*bullets[i].speed*dt;
        bullets[i].position.y+=bullets[i].direction.y*bullets[i].speed*dt;

        if(bullets[i].position.x<-100 || bullets[i].position.x>2000 ||
           bullets[i].position.y<-100 || bullets[i].position.y>1200)
            bullets[i].active=false;
    }

    // ZOMBIES
    for(int i=0;i<MAX_ZOMBIES;i++)
    {
        if(!zombies[i].active) continue;

        // DYING animation - CHECK FIRST
        if(zombies[i].state == STATE_DYING)
        {
            zombies[i].deathTimer += dt;

            // advance animation
            if(zombies[i].deathTimer >= animSpeed)
            {
                zombies[i].deathFrame++;
                zombies[i].deathTimer = 0;
            }

            if(zombies[i].deathFrame >= MAX_FRAMES || zombies[i].deathTimer > 1.0f)
            {
                zombies[i].active = false;
            }
            continue;  // Skip all other zombie logic when dying
        }

        // ===== RANGED ZOMBIE SHOOTING =====
        if(zombies[i].type == 2 && zombies[i].state != STATE_DYING)
        {   
            enemyShootTimer[i] += dt;
            zombies[i].state = STATE_ATTACK;
            zombies[i].shootTimer += dt;

            if(zombies[i].shootTimer >= zombies[i].fireRate)
            {
                zombies[i].shootTimer = 0;

                // spawn bullet
                for(int b=0;b<MAX_ENEMY_BULLETS;b++)
                {
                    if(!enemyBullets[b].active)
                    {
                        enemyBullets[b].active = true;
                        enemyBullets[b].position = zombies[i].position;

                        Vector2 dir = {
                            playerPos.x - zombies[i].position.x,
                            playerPos.y - zombies[i].position.y
                        };

                        float len = sqrt(dir.x*dir.x + dir.y*dir.y);
                        if(len != 0)
                        {
                        dir.x /= len;
                        dir.y /= len;
                        }

                        enemyBullets[b].direction = dir;
                        enemyBullets[b].speed = 200;

                        break;
                    }
             }
            }
            zombies[i].state = STATE_WALK;
        }

        Vector2 dir={playerPos.x-zombies[i].position.x,playerPos.y-zombies[i].position.y};
        float len=sqrt(dir.x*dir.x+dir.y*dir.y);

        if(len!=0){dir.x/=len;dir.y/=len;}

        float speed = zombies[i].speed;
        if(zombies[i].type == 1) speed *= 0.5f;

        zombies[i].position.x+=dir.x*speed*dt;
        zombies[i].position.y+=dir.y*speed*dt;

        zombies[i].state = STATE_WALK;

        if(CheckCollisionCircles(playerPos,20,zombies[i].position,20))
        {
            playerHP -= 30*dt;
        }
    }

    // COLLISION
    for(int i=0;i<MAX_BULLETS;i++)
    {   
        if(!bullets[i].active) continue;

        for(int j=0;j<MAX_ZOMBIES;j++)
        {
            if(!zombies[j].active || zombies[j].type >= NUM_VARIATIONS) continue;
    
            if(CheckCollisionCircles(bullets[i].position,12,zombies[j].position,30))
            {
                for(int h=0;h<MAX_HIT_EFFECTS;h++)
                {   
                    if(!hitEffects[h].active)
                    {
                        hitEffects[h].active = true;
                        hitEffects[h].position = zombies[j].position;
                        hitEffects[h].timer = 0;
                        break;
                    }
                }

                // 💀 DAMAGE
                zombies[j].hp -= bulletDamage;

                if(zombies[j].hp <= 0 && zombies[j].state != STATE_DYING)
                {
                    zombies[j].state = STATE_DYING;
                    zombies[j].deathFrame = 0;
                    zombies[j].deathTimer = 0;
                    gold += 10;
                }

                bullets[i].active = false;
                break;
            }
        }
    }
    // ===== ENEMY BULLETS UPDATE =====
    for(int i=0;i<MAX_ENEMY_BULLETS;i++)
    {
        if(!enemyBullets[i].active) continue;

        enemyBullets[i].position.x += enemyBullets[i].direction.x * enemyBullets[i].speed * dt;
        enemyBullets[i].position.y += enemyBullets[i].direction.y * enemyBullets[i].speed * dt;

        // hit player
        if(CheckCollisionCircles(enemyBullets[i].position,10,playerPos,20))
        {
            playerHP -= 10;
            enemyBullets[i].active = false;
        }

        // out of bounds
        if(enemyBullets[i].position.x < -50 || enemyBullets[i].position.x > 2000 ||
        enemyBullets[i].position.y < -50 || enemyBullets[i].position.y > 1200)
        {
            enemyBullets[i].active = false;
        }
    }
    // WAVE END
    int alive=0;
    for(int i=0;i<MAX_ZOMBIES;i++)
        if(zombies[i].active) alive++;

    if(alive==0 && !waveTransition && !inShop)
    {
        waveTransition = true;
        waveTransitionTimer = 0.0f;
    }
    if(waveTransition)
    {   
        waveTransitionTimer += dt;

        if(waveTransitionTimer >= waveTransitionDuration)
        {
            waveTransition = false; 
            inShop=true;
        }
        if(currentWave >= MAX_WAVES && !gameWon)
        {
            gameWon = true;
            inShop = false; 
        }
    }

    for(int i=0;i<MAX_HIT_EFFECTS;i++)
    {
        if(!hitEffects[i].active) continue;

        hitEffects[i].timer += dt;
        
        if(hitEffects[i].timer > 0.2f)
            hitEffects[i].active = false;
    }

        
    
}

// ================= DRAW =================

void DrawDefense(float scaleX,float scaleY)
{
    if(defenseBgLoaded)
    {
        float screenW = GetScreenWidth();
        float screenH = GetScreenHeight();

        float imgW = defenseBackground.width;
        float imgH = defenseBackground.height;

        // 🔥 Calculate scale
        float scale = fmax(screenW / imgW, screenH / imgH);

        // 🔥 Calculate final size
        float drawW = imgW * scale;
        float drawH = imgH * scale;

        // 🔥 Center the image
        float offsetX = (screenW - drawW) / 2.0f;
        float offsetY = (screenH - drawH) / 2.0f;

        // Draw
        Rectangle src = {0, 0, imgW, imgH};
        Rectangle dst = {offsetX, offsetY, drawW, drawH};

        DrawTexturePro(defenseBackground, src, dst, (Vector2){0,0}, 0, WHITE);
    }
    else
    {
        ClearBackground(DARKGRAY);
    }

    // PLAYER ROTATION
    Vector2 mouse = GetMousePosition();
    float angle = atan2(mouse.y - playerPos.y, mouse.x - playerPos.x) * RAD2DEG;

    Texture2D pTex = playerAnim[playerState][playerFrame];
    if(pTex.id == 0) pTex = playerAnim[STATE_IDLE][0]; 

    Rectangle src={0,0,(float)pTex.width,(float)pTex.height};
    Rectangle dst={playerPos.x,playerPos.y,playerSize,playerSize};
    Vector2 origin={playerSize/2,playerSize/2};

    DrawTexturePro(pTex,src,dst,origin,angle,WHITE);

    if(gameWon)
    {
        DrawVictoryScreen();
        return;  // Skip normal drawing
    }   

    // ZOMBIES
    for(int i=0;i<MAX_ZOMBIES;i++)
    {
        if(!zombies[i].active) continue;
        
        // SAFETY CHECKS
        int ztype = zombies[i].type;
        int zstate = zombies[i].state;
        if(ztype >= NUM_VARIATIONS) ztype = 0;
        if(zstate >= MAX_STATES) zstate = STATE_WALK;
        
        int frame = zombieFrames[i];
        if(zstate == STATE_DYING)
        {
            frame = zombies[i].deathFrame;
            if(frame >= MAX_FRAMES) frame = 0;
        }
        
        // DOUBLE SAFETY - check texture is valid
        if(ztype >= NUM_VARIATIONS) ztype = 0;
        if(zstate >= MAX_STATES) zstate = STATE_WALK;
        if(frame >= MAX_FRAMES) frame = MAX_FRAMES-1;
        Texture2D zTex = zombieAnim[ztype][zstate][frame];
        if(zTex.id == 0) continue;  // Skip invalid texture
        
        Rectangle zSrc = {0, 0, (float)zTex.width, (float)zTex.height};
        Rectangle zDst = {zombies[i].position.x, zombies[i].position.y, zombieSize, zombieSize};
        Vector2 zOrigin = {zombieSize/2, zombieSize/2};
        
        DrawTexturePro(zTex, zSrc, zDst, zOrigin, 0, WHITE);
    }

    // BULLETS
    for(int i=0;i<MAX_BULLETS;i++)
    {
        if(!bullets[i].active) continue;

        float rotation=atan2(bullets[i].direction.y,bullets[i].direction.x)*RAD2DEG;

        Rectangle bSrc={0,0,bulletTex.width,bulletTex.height};
        Rectangle bDst={bullets[i].position.x,bullets[i].position.y,bulletWidth,bulletHeight};
        Vector2 bOrigin={bulletWidth/2,bulletHeight/2};

        DrawTexturePro(bulletTex,bSrc,bDst,bOrigin,rotation,WHITE);
    }
    // ===== ENEMY BULLETS DRAW =====
    for(int i=0;i<MAX_ENEMY_BULLETS;i++)
    {
        if(!enemyBullets[i].active) continue;

        DrawCircleV(enemyBullets[i].position, 6, PURPLE);
    }
    // UI
    DrawRectangle(20,60,200,20,GRAY);
    DrawRectangle(20,60,(int)(200*((float)playerHP/maxHP)),20,RED);
    DrawText("HP",20,35,20,WHITE);

    DrawText(TextFormat("Ammo: %d",ammo),20,90,20,WHITE);
    DrawText(TextFormat("Wave: %d",currentWave+1),20,120,20,WHITE);
    DrawText(TextFormat("Gold: %d",gold),20,150,20,WHITE);

    if(inShop)
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        int panelW = 750;
        int panelH = 520;

        int panelX = sw/2 - panelW/2;
        int panelY = sh/2 - panelH/2;

        // ===== BACKGROUND LAYERS (DEPTH) =====
        DrawRectangle(panelX, panelY, panelW, panelH, BEIGE);
        DrawRectangle(panelX+10, panelY+10, panelW-20, panelH-20, BROWN);

        // ===== TITLE TAB =====
        DrawRectangle(panelX+20, panelY-40, 200, 50, BROWN);
        DrawText("UPGRADE", panelX+40, panelY-30, 30, WHITE);

        // ===== GRID SETTINGS =====
        int boxW = 320;
        int boxH = 110;
        int gapX = 360;
        int gapY = 130;

        for(int i=0;i<5;i++)
        {
            int x = panelX + 30 + (i%2)*gapX;
            int y = panelY + 40 + (i/2)*gapY;

            // BOX BACKGROUND
            DrawRectangle(x, y, boxW, boxH, BEIGE);
            DrawRectangleLines(x, y, boxW, boxH, DARKGREEN);

            const char *name;
            int level;

            switch(i)
            {
                case 0: name="HEALTH"; level=hpLevel; break;
                case 1: name="AMMO"; level=ammoLevel; break;
                case 2: name="POWER"; level=dmgLevel; break;
                case 3: name="RELOAD"; level=reloadLevel; break;
                case 4: name="SPEED"; level=speedLevel; break;
            }

            // LABEL
            DrawText(name, x+15, y+10, 22, WHITE);

            // ===== PROGRESS BAR (like dots in your image) =====
            for(int j=0;j<10;j++)
            {
                Color c = (j < level) ? SKYBLUE : DARKBLUE;
                DrawCircle(x+20 + j*25, y+55, 8, c);
            }

            // ===== BUY BUTTON =====
            Rectangle btn = {x+210, y+65, 90, 30};

            Vector2 m = GetMousePosition();
            bool hover = CheckCollisionPointRec(m, btn);

            Color btnColor = hover ? GOLD : ORANGE;

            DrawRectangleRec(btn, btnColor);
            DrawRectangleLinesEx(btn, 2, DARKBROWN);
            DrawText("BUY", btn.x+20, btn.y+5, 20, BLACK);

            // ===== CLICK =====
            if(hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && gold >= 50)
            {
                gold -= 50;

                switch(i)
                {
                    case 0: hpLevel++; maxHP += 20; break;
                    case 1: ammoLevel++; maxAmmo += 2; break;
                    case 2: dmgLevel++; bulletDamage++; break;
                    case 3: reloadLevel++; reloadSpeed += 0.2f; break;
                    case 4: speedLevel++; moveSpeed += 30; break;
                }
            }
        }

        // ===== FOOTER =====
        DrawText(TextFormat("GOLD: %d", gold), panelX+30, panelY+panelH-60, 25, WHITE);

        Rectangle startBtn = {panelX+panelW-220, panelY+panelH-70, 180, 40};

        Vector2 m = GetMousePosition();
        bool hover = CheckCollisionPointRec(m, startBtn);

        DrawRectangleRec(startBtn, hover ? GREEN : DARKGREEN);
        DrawText("START WAVE", startBtn.x+20, startBtn.y+10, 20, WHITE);

        if(hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if(currentWave + 1 < totalWaves) 
            {
                currentWave++;
                StartWave(currentWave);
                inShop = false;
                waveFinished=true;
            } else {
                inShop = false;
                waveFinished = true;
            }
        }
    }
        for(int i=0;i<MAX_HIT_EFFECTS;i++)
        {
            if(!hitEffects[i].active) continue;

            float size = 10 + hitEffects[i].timer * 40;
            DrawCircleLines(hitEffects[i].position.x, hitEffects[i].position.y, size, YELLOW);
        }
    // wave transition
    if(waveTransition)
    {
        float t = waveTransitionTimer;

        // Fade effect (0 → 1 → 0)
        float alpha;
        if(t < waveTransitionDuration / 2)
            alpha = t / (waveTransitionDuration / 2);
        else
            alpha = 1.0f - ((t - waveTransitionDuration/2) / (waveTransitionDuration/2));

        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // Dark overlay
        DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, alpha * 0.6f));

        // Text
        const char* text = (currentWave + 1 < totalWaves)
            ? TextFormat("WAVE %d COMPLETE\nNEXT: WAVE %d", currentWave+1, currentWave+2)
            : "FINAL WAVE COMPLETE";

        int fontSize = 60;
        int textWidth = MeasureText(text, fontSize);

        DrawText(text,
            screenW/2 - textWidth/2,
            screenH/2 - fontSize/2,
            fontSize,
            Fade(WHITE, alpha)
        );
    }
}

void DrawVictoryScreen()
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // Background
    DrawRectangle(0,0,sw,sh,BLACK);

    // Victory text
    DrawText("CONGRATULATIONS!", sw/2 - 250, sh/2 - 100, 50, GOLD);
    DrawText("You survived all 30 waves!", sw/2 - 250, sh/2 - 40, 30, WHITE);

    // Return to menu button
    Rectangle menuBtn = {sw/2 - 100, sh/2 + 50, 200, 60};
    DrawRectangleRounded(menuBtn, 0.2f, 10, LIGHTGRAY);
    DrawText("Return to Menu", menuBtn.x + 20, menuBtn.y + 15, 20, BLACK);

    // Mouse click detection
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse = GetMousePosition();
        if(CheckCollisionPointRec(mouse, menuBtn))
        {
            // Reset everything
            ResetDefense();
            gameWon = false;
        }
    }
}

int GetCurrentWave()
{
    return currentWave;
}

bool IsGameWon()
{
    return gameWon;
}
// ================= FINISH =================

bool IsDefenseFinished()
{
    return waveFinished;
}
//Code Created by 林义信
#pragma once
#include "Display.h"

#define SPRITESTARTX 8
#define SPRITESTARTY 0

//Speeds for Sprites
#define VVVFAST 200
#define VVFAST 400
#define VFAST 800
#define FAST 1200
#define MEDIUM 2400
#define SLOW 9600

enum TAMO_THOUGHT:uint8_t{
  THOUGHT_OF_LOVE = 0, //heart
  THOUGHT_OF_HEARTBREAK = 1, //heart breaking
  THOUGHT_OF_DEATH = 2, //skull
  THOUGHT_OF_REVENGE = 3, //gun
  THOUGHT_OF_FOOD = 4, //pacman? don't like this one
  THOUGHT_OF_MUSIC = 5, //music note
  THOUGHT_OF_MONEY = 6, //$$$
  THOUGHT_OF_LOWBATTERY = 7, //empty battery
  THOUGHT_OF_CHARGING = 8, // charging battery
  THOUGHT_OF_SADTHOUGHTS = 9, //crying face
  THOUGHT_OF_NEUTRALTHOUGHTS = 10, //neutral face
  THOUGHT_OF_HAPPYTHOUGHTS = 11,//happy face
  THOUGHT_OF_SMOKING = 12,
  THOUGHT_OF_FLOWERS = 13
};

const TAMO_THOUGHT happyThoughts[7] = {THOUGHT_OF_FLOWERS,THOUGHT_OF_HAPPYTHOUGHTS,THOUGHT_OF_LOVE,THOUGHT_OF_MUSIC,THOUGHT_OF_MONEY,THOUGHT_OF_REVENGE,THOUGHT_OF_SMOKING};
const TAMO_THOUGHT neutralThoughts[6] = {THOUGHT_OF_NEUTRALTHOUGHTS,THOUGHT_OF_MUSIC,THOUGHT_OF_MONEY,THOUGHT_OF_REVENGE,THOUGHT_OF_DEATH,THOUGHT_OF_SMOKING};
const TAMO_THOUGHT sadThoughts[6] = {THOUGHT_OF_SADTHOUGHTS,THOUGHT_OF_HEARTBREAK,THOUGHT_OF_DEATH,THOUGHT_OF_REVENGE,THOUGHT_OF_MONEY,THOUGHT_OF_SMOKING};

#define HEALTH_LOSS 1
#define HEALTH_GAIN 10000

#define GOOD_HEALTH_THRESHOLD 55535 //drops to this in 25hrs
#define BAD_HEALTH_THRESHOLD 32768 //drops to this in 3 days

#define GOOD_STATE 0
#define OKAY_STATE 1
#define BAD_STATE 2

//Tamo bit flags
#define NEEDS_TO_POOP_BIT 0
#define IS_ASLEEP_BIT 1
#define LOW_BATTERY_BIT 2
#define IS_CHARGING_BIT 3
#define IS_DEAD_BIT 4
#define SMOKING_BIT 5
#define NEEDS_TO_SMOKE_BIT 6
#define RESET_BIT 7

// EEPROM memory addresses
enum EEPROM_ADDRESS:uint8_t{
  EEPROM_MODE_ADDR = 0,
  EEPROM_IDENTITY_ADDR = 1,
  EEPROM_HEALTH_ADDR_MSB = 2,
  EEPROM_HEALTH_ADDR_LSB = 3,
  EEPROM_STATUS_ADDR = 4,
  EEPROM_SLIDESHOW_FRAME_COUNT_ADDR = 5,
  EEPROM_SLIDESHOW_SPEED_ADDR = 6,
  EEPROM_SLIDESHOW_SLEEP_TIME_ADDR = 7,
  EEPROM_SLIDESHOW_BLINK_TIME_ADDR = 8
};

// #define EEPROM_SIZE 256

// Creature identities
enum SPRITE_IDENTITY:uint8_t{
  NO_IDENTITY = 255,
  SPRITE_0 = 0,
  SPRITE_1 = 1,
  SPRITE_2 = 2,
  SPRITE_3 = 3,
  CUSTOM_SPRITE = 4
};

enum TAMO_MOOD:uint8_t{
  MOOD_NEUTRAL = 0,
  MOOD_SAD = 1,
  MOOD_ANGRY = 2,
  MOOD_HAPPY = 3,
  MOOD_EATING = 4,
  MOOD_DEAD = 5,
  MOOD_BIRTH = 6,
  MOOD_RANDOM = 7,
  MOOD_SMOKING = 8,
  MOOD_POOPING = 9,
  MOOD_TALKING = 10,
  MOOD_PECKISH = 11,
  MOOD_QR_CODE  = 12
};

// Sprite ID's
#define IDLE_SPRITE MOOD_NEUTRAL
#define SAD_SPRITE MOOD_SAD
#define MAD_SPRITE MOOD_ANGRY
#define HAPPY_SPRITE MOOD_HAPPY
#define EATING_SPRITE MOOD_EATING

#define NO_FOOD_PREFERENCE 255
#define PREFERS_CHEESE 0
#define PREFERS_COOKIE 1
#define PREFERS_APPLE 2
#define PREFERS_CIG 3

#include "Sprite.h"

enum TAMO_MODE:uint8_t{
  NORMAL_TAMO = 0,
  SLIDESHOW = 1,
  TEXT = 2
};


/*
when tamo sleeps, it needs to jump out of whatever emotion it's in before actually putting itself to sleep. Then 
mood/state can be changed during sleep, and when he wakes up he'll live() or vibecheck() again to get back to it
*/
class Tamo{
  public:
    Tamo();
    Sprite sprite;
    TAMO_MODE mode = NORMAL_TAMO;
    //When tamo boots with "NO_IDENTITY", it reads identity from EEPROM
    SPRITE_IDENTITY identity = NO_IDENTITY;
    // default: birth sequence
    TAMO_MOOD mood = MOOD_BIRTH;
    TAMO_THOUGHT thought = THOUGHT_OF_LOVE;
    int16_t moodTime = 0;
    uint16_t timeSinceLastCig = 0;
    uint8_t timeSinceLastTalk = 0;
    //status register (volatile so that sleep can be turned off from interrupts)
    volatile uint8_t status = 0b00000000;
    /*
      health decreases every 1 second. Food resets it
    */
    volatile uint16_t health = 65535;//decreases by 1 when hunger is 255, so it lasts 18hrs
    volatile uint16_t hunger = 0;//increases up to 255, at which point tamo can eat
    volatile uint16_t updatesSinceLastEEPROMSave = 0;

    void init();
    bool isFeeling();
    void sleepCheck();
    void body();
    //runs an update cycle
    void live();
    //gets new emotion/current state
    void vibeCheck();
    void vibeCheck(bool);

    //randomly eat
    void feedSelf();

    //runs the current emotion
    void feel();
    void debugCheckMoodSprites();

    void game();

    void setMoodSprite(TAMO_MOOD);
    void setMoodSprite();
    void setIdentity(SPRITE_IDENTITY);

    void basicEmotion();
    void waitAndPlayThruSprite(uint16_t,bool);
    void waitAndBlink(uint16_t);
    void walkOn();
    void walkOff();
    void idle();
    void smokeBreak();
    void smokingDamage();
    void talk();
    void poop();
    void feed();
    void eat(const uint16_t *);
    void dead();
    void birth();
    void baby();
    void sleep();
    bool getStatusBit(uint8_t which);
    void setStatusBit(uint8_t which, bool state);
    bool needsToPoop(){return getStatusBit(NEEDS_TO_POOP_BIT);}
    bool isAsleep(){return getStatusBit(IS_ASLEEP_BIT);}
    bool lowBattery(){return getStatusBit(LOW_BATTERY_BIT);}
    bool isCharging(){return getStatusBit(IS_CHARGING_BIT);}
    const uint16_t *  getSprite(uint8_t);
    void batteryCheck();
    void slideshow();
    void qrCode();
};

extern Tamo tamo;
//Need to use drawmode = vertical in image2cpp
//these are the bitmap graphics shared between all creatures!

// #include "creatures/porcini.h"
// #include "creatures/tamo.h"
// #include "creatures/bug.h"
// #include "creatures/vishay.h"


#include "bitmaps.h"

/*
----------------------------------

  Frame arrays for the animations

----------------------------------
*/

/*
		Vishay
*/

const unsigned char* const vishay_idle_sprite[2] PROGMEM = {
	vishay_sprite_idle_1,
	vishay_sprite_idle_2
};
const unsigned char* const vishay_eating_sprite[2] PROGMEM = {
	vishay_sprite_eating_1,
	vishay_sprite_eating_2,
};
const unsigned char* const vishay_sad_sprite[2] PROGMEM = {
	vishay_sprite_sad_1,
	vishay_sprite_sad_2
};
const unsigned char* const vishay_mad_sprite[2] PROGMEM = {
	vishay_sprite_mad_1,
	vishay_sprite_mad_2
};
const unsigned char* const vishay_happy_sprite[2] PROGMEM = {
	vishay_sprite_happy_1,
	vishay_sprite_happy_2
};

const unsigned char* const * vishay_spritesheet[6] = {
	vishay_idle_sprite,
	vishay_eating_sprite,
	vishay_sad_sprite,
	vishay_mad_sprite,
	vishay_happy_sprite
};

/*
		Tamo
*/


const unsigned char* const tamo_idle_sprite[2] PROGMEM = {
	tamo_sprite_idle_1,
	tamo_sprite_idle_2
};
const unsigned char* const tamo_eating_sprite[2] PROGMEM = {
	tamo_sprite_eating_1,
	tamo_sprite_eating_2,
};
const unsigned char* const tamo_sad_sprite[2] PROGMEM = {
	tamo_sprite_sad_1,
	tamo_sprite_sad_2
};
const unsigned char* const tamo_mad_sprite[2] PROGMEM = {
	tamo_sprite_mad_1,
	tamo_sprite_mad_2
};
const unsigned char* const tamo_happy_sprite[2] PROGMEM = {
	tamo_sprite_happy_1,
	tamo_sprite_happy_2
};

const unsigned char* const * tamo_spritesheet[6] = {
	tamo_idle_sprite,
	tamo_eating_sprite,
	tamo_sad_sprite,
	tamo_mad_sprite,
	tamo_happy_sprite
};

/*
		Bug
*/


const unsigned char* const bug_idle_sprite[2] PROGMEM = {
	bug_sprite_idle_1,
	bug_sprite_idle_2
};
const unsigned char* const bug_eating_sprite[2] PROGMEM = {
	bug_sprite_eating_1,
	bug_sprite_eating_2,
};
const unsigned char* const bug_sad_sprite[2] PROGMEM = {
	bug_sprite_sad_1,
	bug_sprite_sad_2
};
const unsigned char* const bug_mad_sprite[2] PROGMEM = {
	bug_sprite_mad_1,
	bug_sprite_mad_2
};
const unsigned char* const bug_happy_sprite[2] PROGMEM = {
	bug_sprite_happy_1,
	bug_sprite_happy_2
};

const unsigned char* const * bug_spritesheet[6] = {
	bug_idle_sprite,
	bug_eating_sprite,
	bug_sad_sprite,
	bug_mad_sprite,
	bug_happy_sprite
};

/*
		Porcini
*/

const unsigned char* const porcini_idle_sprite[2] PROGMEM = {
	porcini_sprite_idle_1,
	porcini_sprite_idle_2
};
const unsigned char* const porcini_eating_sprite[2] PROGMEM = {
	porcini_sprite_eating_1,
	porcini_sprite_eating_2,
};
const unsigned char* const porcini_sad_sprite[2] PROGMEM = {
	porcini_sprite_sad_1,
	porcini_sprite_sad_2
};
const unsigned char* const porcini_mad_sprite[2] PROGMEM = {
	porcini_sprite_mad_1,
	porcini_sprite_mad_2
};
const unsigned char* const porcini_happy_sprite[2] PROGMEM = {
	porcini_sprite_happy_1,
	porcini_sprite_happy_2
};

const unsigned char* const * porcini_spritesheet[6] = {
	porcini_idle_sprite,
	porcini_eating_sprite,
	porcini_sad_sprite,
	porcini_mad_sprite,
	porcini_happy_sprite
};

/*
----------------------------------

 Talking icons

----------------------------------
*/

const unsigned char* const talking_heartbreak[3] PROGMEM = {
	thought_bubble_sprite,
	thinking_love_sprite,
	thinking_heartbreak_sprite
};
const unsigned char* const talking_low_battery[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_low_battery_sprite
};
const unsigned char* const talking_charging[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_charging_sprite
};
const unsigned char* const talking_love[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_love_sprite
};
const unsigned char* const talking_death[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_skull_sprite
};
const unsigned char* const talking_hunger[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_pacman_sprite
};
const unsigned char* const talking_revenge[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_gun_sprite
};
const unsigned char* const talking_me[3] PROGMEM = {
	thought_bubble_sprite,
	thinking_talking_sprite_1,
	thinking_talking_sprite_2
};
const unsigned char* const talking_music[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_music_sprite
};
const unsigned char* const talking_cash[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_cash_sprite
};
const unsigned char* const talking_dog[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_dog_sprite
};
const unsigned char* const talking_sad[3] PROGMEM = {
	thought_bubble_sprite,
	thinking_sad_sprite_1,
	thinking_sad_sprite_2
};
const unsigned char* const talking_neutral[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_neutral_sprite,
};
const unsigned char* const talking_happy[3] PROGMEM = {
	thought_bubble_sprite,
	thinking_happy_sprite_1,
	thinking_happy_sprite_2
};
/*
----------------------------------

 	Poop

----------------------------------
*/

const unsigned char* const poopAnim[2] PROGMEM = {
	poop_sprite_1,
	poop_sprite_4
};

/*
----------------------------------

 	Death/Tombstone

----------------------------------
*/

const unsigned char* const death_sprite[2] PROGMEM = {
	dead_sprite_1,
	dead_sprite_2
};

/*
----------------------------------

 	 Egg hatching

----------------------------------
*/
const unsigned char* const egg_sprite[4] PROGMEM = {
	egg_sprite_1,
	egg_sprite_2,
	egg_sprite_3,
	egg_sprite_4
};

const unsigned char* const food_animation[5] PROGMEM = {
	food_sprite_1,
	food_sprite_2,
	food_sprite_3,
	food_sprite_4,
	food_sprite_5
};


/*
----------------------------------

 	 Titlescreen/'hello' stuff

----------------------------------
*/

// 'palestine', 29x16px
const unsigned char free_palestine_bmp [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x14, 0x1d, 0x50, 0x10, 0x5c, 0x00, 0x40, 0x3c, 0x10, 
	0x14, 0x14, 0x0c, 0x14, 0x10, 0x0f, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x88, 
	0x80, 0x70, 0x20, 0xa0, 0x10, 0xa0, 0x20, 0x2f, 0x30, 0x28, 0x28, 0x30, 0x20, 0x30, 0x20, 0x30, 
	0x20, 0x38, 0x20, 0x1f, 0x20, 0x38, 0x25, 0x38, 0x20, 0x1f
};

//448 bytes per creature (7 emotions, 2 frames per emotion)

//you need 254 bytes for a third creature

#include "spritesheet.h"

/*
----------------------------------

  Frame arrays for the animations

----------------------------------
*/

/*
		Boto
*/

//a constant ptr to a uint16

const uint16_t boto_idle_sprite[2] PROGMEM = {
	boto_sprite_idle_1,
	boto_sprite_idle_2
};
const uint16_t boto_eating_sprite[2] PROGMEM = {
	boto_sprite_eating_1,
	boto_sprite_eating_2
};
const uint16_t boto_sad_sprite[2] PROGMEM = {
	boto_sprite_sad_1,
	boto_sprite_sad_2
};
const uint16_t boto_mad_sprite[2] PROGMEM = {
	boto_sprite_mad_1,
	boto_sprite_mad_2
};
const uint16_t boto_happy_sprite[2] PROGMEM = {
	boto_sprite_happy_1,
	boto_sprite_happy_2
};

const uint16_t * boto_spritesheet[6] = {
	boto_idle_sprite,
	boto_sad_sprite,
	boto_mad_sprite,
	boto_happy_sprite,
	boto_eating_sprite
};

/*
		Tamo
*/


const uint16_t tamo_idle_sprite[2] PROGMEM = {
	tamo_sprite_idle_1,
	tamo_sprite_idle_2
};
const uint16_t tamo_eating_sprite[2] PROGMEM = {
	tamo_sprite_eating_1,
	tamo_sprite_eating_2,
};
const uint16_t tamo_sad_sprite[2] PROGMEM = {
	tamo_sprite_sad_1,
	tamo_sprite_sad_2
};
const uint16_t tamo_mad_sprite[2] PROGMEM = {
	tamo_sprite_mad_1,
	tamo_sprite_mad_2
};
const uint16_t tamo_happy_sprite[2] PROGMEM = {
	tamo_sprite_happy_1,
	tamo_sprite_happy_2
};

const uint16_t * tamo_spritesheet[6] = {
	tamo_idle_sprite,
	tamo_sad_sprite,
	tamo_mad_sprite,
	tamo_happy_sprite,
	tamo_eating_sprite
};

/*
		Bug
*/


const uint16_t bug_idle_sprite[2] PROGMEM = {
	bug_sprite_idle_1,
	bug_sprite_idle_2
};
const uint16_t bug_eating_sprite[2] PROGMEM = {
	bug_sprite_eating_1,
	bug_sprite_eating_2,
};
const uint16_t bug_sad_sprite[2] PROGMEM = {
	bug_sprite_sad_1,
	bug_sprite_sad_2
};
const uint16_t bug_mad_sprite[2] PROGMEM = {
	bug_sprite_mad_1,
	bug_sprite_mad_2
};
const uint16_t bug_happy_sprite[2] PROGMEM = {
	bug_sprite_happy_1,
	bug_sprite_happy_2
};

const uint16_t * bug_spritesheet[6] = {
	bug_idle_sprite,
	bug_sad_sprite,
	bug_mad_sprite,
	bug_happy_sprite,
	bug_eating_sprite
};

/*
		Porcini
*/

const uint16_t porcini_idle_sprite[2] PROGMEM = {
	porcini_sprite_idle_1,
	porcini_sprite_idle_2
};
const uint16_t porcini_eating_sprite[2] PROGMEM = {
	porcini_sprite_eating_1,
	porcini_sprite_eating_2,
};
const uint16_t porcini_sad_sprite[2] PROGMEM = {
	porcini_sprite_sad_1,
	porcini_sprite_sad_2
};
const uint16_t porcini_mad_sprite[2] PROGMEM = {
	porcini_sprite_mad_1,
	porcini_sprite_mad_2
};
const uint16_t porcini_happy_sprite[2] PROGMEM = {
	porcini_sprite_happy_1,
	porcini_sprite_happy_2
};

const uint16_t * porcini_spritesheet[6] = {
	porcini_idle_sprite,
	porcini_sad_sprite,
	porcini_mad_sprite,
	porcini_happy_sprite,
	porcini_eating_sprite
};

/*
----------------------------------

 Talking icons

----------------------------------
*/

const uint16_t talking_heartbreak[3] PROGMEM = {
	thought_bubble_sprite,
	thinking_love_sprite,
	thinking_heartbreak_sprite
};
const uint16_t talking_low_battery[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_low_battery_sprite
};
const uint16_t talking_charging[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_charging_sprite
};
const uint16_t talking_love[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_love_sprite
};
const uint16_t talking_death[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_skull_sprite
};
const uint16_t talking_hunger[3] PROGMEM = {
	thought_bubble_sprite,
	thinking_neutral_sprite,
	thinking_eating_sprite_2
};
const uint16_t talking_revenge[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_gun_sprite
};
const uint16_t talking_music[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_music_sprite
};
const uint16_t talking_cash[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_cash_sprite
};
// const uint16_t talking_dog[2] PROGMEM = {
// 	thought_bubble_sprite,
// 	thinking_dog_sprite
// };
const uint16_t talking_sad[3] PROGMEM = {
	thought_bubble_sprite,
	thinking_sad_sprite_1,
	thinking_sad_sprite_2
};
const uint16_t talking_neutral[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_neutral_sprite,
};
const uint16_t talking_happy[2] PROGMEM = {
	thought_bubble_sprite,
	thinking_happy_sprite_1
};
/*
----------------------------------

 	Poop

----------------------------------
*/

const uint16_t poopAnim[2] PROGMEM = {
	poop_sprite_1,
	poop_sprite_2
};

/*
----------------------------------

 	Death/Tombstone

----------------------------------
*/

const uint16_t death_sprite[2] PROGMEM = {
	dead_sprite_1,
	dead_sprite_2
};

/*
----------------------------------

 	 Egg hatching

----------------------------------
*/
const uint16_t egg_sprite[4] PROGMEM = {
	egg_sprite_1,
	egg_sprite_2,
	egg_sprite_3,
	egg_sprite_4
};

// const uint16_t cookie_animation[5] PROGMEM = {
// 	cookie_sprite_1,
// 	cookie_sprite_2,
// 	cookie_sprite_3,
// 	cookie_sprite_4,
// 	cookie_sprite_5
// };
const uint16_t whiskey_animation[5] PROGMEM = {
	whiskey_sprite_1,
	whiskey_sprite_2,
	whiskey_sprite_3,
	whiskey_sprite_4,
	whiskey_sprite_5
};

const uint16_t cheese_animation[5] PROGMEM = {
	cheese_sprite_1,
	cheese_sprite_2,
	cheese_sprite_3,
	cheese_sprite_4,
	cheese_sprite_5
};

const uint16_t apple_animation[5] PROGMEM = {
	apple_sprite_1,
	apple_sprite_2,
	apple_sprite_3,
	apple_sprite_4,
	apple_sprite_5
};
const uint16_t cig_animation[5] PROGMEM = {
	cig_sprite_1,
	cig_sprite_2,
	cig_sprite_3,
	cig_sprite_4,
	cig_sprite_5
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

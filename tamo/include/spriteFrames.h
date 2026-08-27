
#pragma once
#include "spritesheet.h"
#include <stdint.h>

/*
----------------------------------

  Frame arrays for the animations

----------------------------------
*/

// Creature sprites

const uint16_t idle_sprite_0[2] = {
	sprite_0_idle_1,
	sprite_0_idle_2
};
const uint16_t eating_sprite_0[2] = {
	sprite_0_eating_1,
	sprite_0_eating_2
};
const uint16_t sad_sprite_0[2] = {
	sprite_0_sad_1,
	sprite_0_sad_2
};
const uint16_t mad_sprite_0[2] = {
	sprite_0_mad_1,
	sprite_0_mad_2
};
const uint16_t happy_sprite_0[2] = {
	sprite_0_happy_1,
	sprite_0_happy_2
};

const uint16_t * sprite_0_spritesheet[5] = {
	idle_sprite_0,
	sad_sprite_0,
	mad_sprite_0,
	happy_sprite_0,
	eating_sprite_0
};


const uint16_t idle_sprite_1[2] = {
	sprite_1_idle_1,
	sprite_1_idle_2
};
const uint16_t eating_sprite_1[2] = {
	sprite_1_eating_1,
	sprite_1_eating_2
};
const uint16_t sad_sprite_1[2] = {
	sprite_1_sad_1,
	sprite_1_sad_2
};
const uint16_t mad_sprite_1[2] = {
	sprite_1_mad_1,
	sprite_1_mad_2
};
const uint16_t happy_sprite_1[2] = {
	sprite_1_happy_1,
	sprite_1_happy_2
};

const uint16_t * sprite_1_spritesheet[5] = {
	idle_sprite_1,
	sad_sprite_1,
	mad_sprite_1,
	happy_sprite_1,
	eating_sprite_1
};


const uint16_t idle_sprite_2[2] = {
	sprite_2_idle_1,
	sprite_2_idle_2
};
const uint16_t eating_sprite_2[2] = {
	sprite_2_eating_1,
	sprite_2_eating_2
};
const uint16_t sad_sprite_2[2] = {
	sprite_2_sad_1,
	sprite_2_sad_2
};
const uint16_t mad_sprite_2[2] = {
	sprite_2_mad_1,
	sprite_2_mad_2
};
const uint16_t happy_sprite_2[2] = {
	sprite_2_happy_1,
	sprite_2_happy_2
};

const uint16_t * sprite_2_spritesheet[5] = {
	idle_sprite_2,
	sad_sprite_2,
	mad_sprite_2,
	happy_sprite_2,
	eating_sprite_2
};


const uint16_t idle_sprite_3[2] = {
	sprite_3_idle_1,
	sprite_3_idle_2
};
const uint16_t eating_sprite_3[2] = {
	sprite_3_eating_1,
	sprite_3_eating_2
};
const uint16_t sad_sprite_3[2] = {
	sprite_3_sad_1,
	sprite_3_sad_2
};
const uint16_t mad_sprite_3[2] = {
	sprite_3_mad_1,
	sprite_3_mad_2
};
const uint16_t happy_sprite_3[2] = {
	sprite_3_happy_1,
	sprite_3_happy_2
};

const uint16_t * sprite_3_spritesheet[5] = {
	idle_sprite_3,
	sad_sprite_3,
	mad_sprite_3,
	happy_sprite_3,
	eating_sprite_3
};



/*
		Custom
*/

const uint16_t custom_idle_sprite[2] = {
	custom_sprite_idle_1,
	custom_sprite_idle_2
};
const uint16_t custom_eating_sprite[2] = {
	custom_sprite_eating_1,
	custom_sprite_eating_2,
};
const uint16_t custom_sad_sprite[2] = {
	custom_sprite_sad_1,
	custom_sprite_sad_2
};
const uint16_t custom_mad_sprite[2] = {
	custom_sprite_mad_1,
	custom_sprite_mad_2
};
const uint16_t custom_happy_sprite[2] = {
	custom_sprite_happy_1,
	custom_sprite_happy_2
};

const uint16_t * custom_spritesheet[5] = {
	custom_idle_sprite,
	custom_sad_sprite,
	custom_mad_sprite,
	custom_happy_sprite,
	custom_eating_sprite
};

// superarray holding all the creature spritesheets

const uint16_t ** creature_sprites[5] = {
	sprite_0_spritesheet,
	sprite_1_spritesheet,
	sprite_2_spritesheet,
	sprite_3_spritesheet,
	custom_spritesheet
};


/*
----------------------------------

 Talking icons

----------------------------------
*/

const uint16_t talking_heartbreak[3] = {
	thought_bubble_sprite,
	thinking_love_sprite,
	thinking_heartbreak_sprite
};
const uint16_t talking_low_battery[2] = {
	thought_bubble_sprite,
	thinking_low_battery_sprite
};
const uint16_t talking_charging[2] = {
	thought_bubble_sprite,
	thinking_charging_sprite
};
const uint16_t talking_love[2] = {
	thought_bubble_sprite,
	thinking_love_sprite
};
const uint16_t talking_death[2] = {
	thought_bubble_sprite,
	thinking_skull_sprite
};
const uint16_t talking_hunger[3] = {
	thought_bubble_sprite,
	thinking_neutral_sprite,
	thinking_eating_sprite_2
};
const uint16_t talking_revenge[2] = {
	thought_bubble_sprite,
	thinking_gun_sprite
};
const uint16_t talking_music[2] = {
	thought_bubble_sprite,
	thinking_music_sprite
};
const uint16_t talking_cash[2] = {
	thought_bubble_sprite,
	thinking_cash_sprite
};
const uint16_t talking_sad[3] = {
	thought_bubble_sprite,
	thinking_sad_sprite_1,
	thinking_sad_sprite_2
};
const uint16_t talking_neutral[2] = {
	thought_bubble_sprite,
	thinking_neutral_sprite,
};
const uint16_t talking_happy[2] = {
	thought_bubble_sprite,
	thinking_happy_sprite_1
};
const uint16_t talking_smoking[2] = {
	thought_bubble_sprite,
	thinking_smoking_sprite
};
const uint16_t talking_flowers[3] = {
	thought_bubble_sprite,
	thinking_flower_1,
	thinking_flower_2
};
/*
----------------------------------

 	Poop

----------------------------------
*/

const uint16_t poopAnim[2] = {
	poop_sprite_1,
	poop_sprite_2
};

/*
----------------------------------

 	Death/Tombstone

----------------------------------
*/

const uint16_t death_sprite[2] = {
	dead_sprite_1,
	dead_sprite_2
};

/*
----------------------------------

 	 Egg hatching

----------------------------------
*/
const uint16_t egg_sprite[4] = {
	egg_sprite_1,
	egg_sprite_2,
	egg_sprite_3,
	egg_sprite_4
};

// const uint16_t cookie_animation[5] = {
// 	cookie_sprite_1,
// 	cookie_sprite_2,
// 	cookie_sprite_3,
// 	cookie_sprite_4,
// 	cookie_sprite_5
// };
const uint16_t whiskey_animation[5] = {
	whiskey_sprite_1,
	whiskey_sprite_2,
	whiskey_sprite_3,
	whiskey_sprite_4,
	whiskey_sprite_5
};

const uint16_t penny_animation[5] = {
	penny_sprite_1,
	penny_sprite_2,
	penny_sprite_3,
	penny_sprite_4,
	penny_sprite_5
};

const uint16_t cheese_animation[5] = {
	cheese_sprite_1,
	cheese_sprite_2,
	cheese_sprite_3,
	cheese_sprite_4,
	cheese_sprite_5
};

const uint16_t apple_animation[5] = {
	apple_sprite_1,
	apple_sprite_2,
	apple_sprite_3,
	apple_sprite_4,
	apple_sprite_5
};
const uint16_t cig_animation[5] = {
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
// const unsigned char free_palestine_bmp [] = {
// 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x14, 0x1d, 0x50, 0x10, 0x5c, 0x00, 0x40, 0x3c, 0x10, 
// 	0x14, 0x14, 0x0c, 0x14, 0x10, 0x0f, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x88, 
// 	0x80, 0x70, 0x20, 0xa0, 0x10, 0xa0, 0x20, 0x2f, 0x30, 0x28, 0x28, 0x30, 0x20, 0x30, 0x20, 0x30, 
// 	0x20, 0x38, 0x20, 0x1f, 0x20, 0x38, 0x25, 0x38, 0x20, 0x1f
// };

//448 bytes per creature (7 emotions, 2 frames per emotion)

//you need 254 bytes for a third creature

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "song.h"
#include "interp.h"

/* item type list */
const struct item_type item_table[] = {
    {ITEM_LIGHT, "light"},
    {ITEM_SCROLL, "scroll"},
    {ITEM_WAND, "wand"},
    {ITEM_STAFF, "staff"},
    {ITEM_WEAPON, "weapon"},
    {ITEM_TREASURE, "treasure"},
    {ITEM_ARMOR, "armor"},
    {ITEM_POTION, "potion"},
    {ITEM_CLOTHING, "clothing"},
    {ITEM_FURNITURE, "furniture"},
    {ITEM_TRASH, "trash"},
    {ITEM_CONTAINER, "container"},
    {ITEM_DRINK_CON, "drink"},
    {ITEM_KEY, "key"},
    {ITEM_FOOD, "food"},
    {ITEM_MONEY, "money"},
    {ITEM_BOAT, "boat"},
    {ITEM_CORPSE_NPC, "npc_corpse"},
    {ITEM_CORPSE_PC, "pc_corpse"},
    {ITEM_FOUNTAIN, "fountain"},
    {ITEM_PILL, "pill"},
    {ITEM_PROTECT, "protect"},
    {ITEM_MAP, "map"},
    {ITEM_PORTAL, "portal"},
    {ITEM_WARP_STONE, "warp_stone"},
    {ITEM_ROOM_KEY, "room_key"},
    {ITEM_GEM, "gem"},
    {ITEM_JEWELRY, "jewelry"},
    {ITEM_JUKEBOX, "jukebox"},
    {ITEM_SADDLE, "saddle"},
    {ITEM_PISTOL, "pistol"},
    {ITEM_SMG, "submachinegun"},
    {ITEM_SHOTGUN, "shotgun"},
    {ITEM_RIFLE, "rifle"},
    {ITEM_HEAVYGUN, "heavy gun"},
    {ITEM_ENERGYGUN, "energy gun"},
    {ITEM_AMMO, "ammunition"},
    {ITEM_CLIP, "magazine"},
    {0, NULL}
};



/* weapon selection table */
const struct weapon_type weapon_table[] = {
    {"sword", OBJ_VNUM_SCHOOL_SWORD, WEAPON_SWORD, &gsn_sword},
    {"mace", OBJ_VNUM_SCHOOL_MACE, WEAPON_MACE, &gsn_mace},
    {"dagger", OBJ_VNUM_SCHOOL_DAGGER, WEAPON_DAGGER, &gsn_dagger},
    {"axe", OBJ_VNUM_SCHOOL_AXE, WEAPON_AXE, &gsn_axe},
    {"staff", OBJ_VNUM_SCHOOL_STAFF, WEAPON_SPEAR, &gsn_spear},
    {"flail", OBJ_VNUM_SCHOOL_FLAIL, WEAPON_FLAIL, &gsn_flail},
    {"whip", OBJ_VNUM_SCHOOL_WHIP, WEAPON_WHIP, &gsn_whip},
    {"polearm", OBJ_VNUM_SCHOOL_POLEARM, WEAPON_POLEARM, &gsn_polearm},
    {NULL, 0, 0, NULL}
};

/* wiznet table and prototype for future flag setting */
const struct wiznet_type wiznet_table[] = {
    {"on", WIZ_ON, 95},
    {"prefix", WIZ_PREFIX, 95},
    {"ticks", WIZ_TICKS, 95},
    {"logins", WIZ_LOGINS, 95},
    {"sites", WIZ_SITES, 95},
    {"links", WIZ_LINKS, 96},
    {"newbies", WIZ_NEWBIE, 95},
    {"spam", WIZ_SPAM, 95},
    {"deaths", WIZ_DEATHS, 96},
    {"resets", WIZ_RESETS, 95},
    {"mobdeaths", WIZ_MOBDEATHS, 95},
    {"flags", WIZ_FLAGS, 95},
    {"penalties", WIZ_PENALTIES, 95},
    {"saccing", WIZ_SACCING, 95},
    {"levels", WIZ_LEVELS, 95},
    {"load", WIZ_LOAD, 99},
    {"restore", WIZ_RESTORE, 99},
    {"snoops", WIZ_SNOOPS, 99},
    {"switches", WIZ_SWITCHES, 99},
    {"secure", WIZ_SECURE, 100},
    {"pk", WIZ_PK, 98},
    {"chibi", WIZ_CHIBI, 95},
    {NULL, 0, 0}
};

/* attack table  -- not very organized :( */
const struct attack_type attack_table[] = {
    {"none", "`2hit``", -1},	/*  0 */
    {"slice", "`1slice``", DAM_SLASH},
    {"stab", "`8stab``", DAM_PIERCE},
    {"slash", "`!slash``", DAM_SLASH},
    {"whip", "`@whip``", DAM_SLASH},
    {"claw", "`1claw``", DAM_SLASH},	/*  5 */
    {"blast", "`1b`!l`#a`!s`1t``", DAM_BASH},
    {"pound", "`5pound``", DAM_BASH},
    {"crush", "`8c`*r`8u`*s`8h``", DAM_BASH},
    {"grep", "`2grep``", DAM_SLASH},
    {"bite", "`#bite``", DAM_PIERCE},	/* 10 */
    {"pierce", "`1pierce``", DAM_PIERCE},
    {"suction", "`4suction``", DAM_BASH},
    {"beating", "`1b`8e`1a`8t`1i`8n`1g``", DAM_BASH},
    {"digestion", "`2d`@i`2g`@e`2s`@t`2i`@o`2n``", DAM_ACID},
    {"charge", "`#charge``", DAM_BASH},	/* 15 */
    {"slap", "`1slap``", DAM_BASH},
    {"punch", "`&punch``", DAM_BASH},
    {"wrath", "`8w`2rat`8h``", DAM_ENERGY},
    {"magic", "`4ma`Vgi`4c``", DAM_ENERGY},
    {"divine", "`1d`!ivi`#ne p`!owe`1r``", DAM_HOLY},	/* 20 */
    {"cleave", "`8c``leav`8e``", DAM_SLASH},
    {"scratch", "`!scratch``", DAM_PIERCE},
    {"peck", "`3peck``", DAM_PIERCE},
    {"peckb", "`3peck``", DAM_BASH},
    {"chop", "`2c`1ho`2p``", DAM_SLASH},	/* 25 */
    {"sting", "`2s`#tin`2g``", DAM_PIERCE},
    {"smash", "`^smash``", DAM_BASH},
    {"shbite", "`#shocking `8bite``", DAM_LIGHTNING},
    {"flbite", "`1flaming `8bite``", DAM_FIRE},
    {"frbite", "`^freezing `8bite``", DAM_COLD},	/* 30 */
    {"acbite", "`@a`2cidi`@c `8bite``", DAM_ACID},
    {"chomp", "`2c`8hom`2p``", DAM_PIERCE},
    {"drain", "`2life `*d`8rai`*n``", DAM_NEGATIVE},
    {"thrust", "`4thrust``", DAM_PIERCE},
    {"slime", "`2s`@lim`2e``", DAM_ACID},
    {"shock", "`!s`#hoc`!k``", DAM_LIGHTNING},
    {"thwack", "`1t`2hwac`1k``", DAM_BASH},
    {"flame", "`1f`!l`1a`!m`1e``", DAM_FIRE},
    {"chill", "`^c`&h`Vi`&l`^l``", DAM_COLD},
    {"darkcharge", "`8d`7a`8rk `sc`5h`sa`5r`sg`5e`7", DAM_NEGATIVE},
    {NULL, NULL, 0}
};

/* race table */

const struct race_type race_table[] = {

/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    },

*/

    {"unique", FALSE, 0, 0, 0, 0, 0, 0, 0, 0},



    {
     "Human", TRUE,
     0, 0, 0,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "High-Elf", TRUE,
     0, AFF_INFRARED | AFF_REGENERATION, 0,
     0, RES_CHARM | RES_ENERGY | RES_NEGATIVE, VULN_ACID,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "Dwarf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "Giant", TRUE,
     0, 0, 0,
     0, RES_BASH, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "Dragonkin", TRUE,
     0, AFF_FLYING, 0,
     0, RES_CHARM | RES_SLASH, 0,
     A | H | M | V, A | C | D | E | F | H | J | K | Q | V
    },

    {
     "Vampire", TRUE,
     0, 0, 0,
     IMM_COLD | IMM_LIGHTNING | IMM_POISON, RES_CHARM, VULN_HOLY | VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "Dark-Elf", TRUE,
     0, AFF_INFRARED | AFF_SNEAK, 0,
     0, RES_ENERGY | RES_NEGATIVE, VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "Cyborg", TRUE,
     0, 0, 0,
     0, RES_CHARM | RES_FIRE, VULN_COLD,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | L
    },

    {
     "Esper", TRUE,
     0, 0, 0,
     IMM_LIGHT, RES_MAGIC, VULN_NEGATIVE,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | L
    },

    {
     "Demon", TRUE,
     0, 0, 0,
     IMM_CHARM, 0, VULN_HOLY,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | L
    },

    {
     "Saiyan", TRUE,
     0, 0, 0,
     0, RES_ENERGY, VULN_MENTAL,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | L
    },

/*    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    }, */

    {
     "Feline", TRUE,
     0, 0, 0,
     0, RES_CHARM, VULN_DROWNING | VULN_SOUND,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Q | U | V
    },



    {
     "myrddraal", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_POISON | RES_DISEASE, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "trolloc", FALSE,
     0, 0, 0,
     0, RES_FIRE | RES_COLD, VULN_MENTAL | VULN_LIGHTNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "bat", FALSE,
     0, AFF_FLYING | AFF_DARK_VISION, OFF_DODGE | OFF_FAST,
     0, 0, VULN_LIGHT,
     A | G | V, A | C | D | E | F | H | J | K | P
    },

    {
     "bear", FALSE,
     0, 0, OFF_CRUSH | OFF_DISARM | OFF_BERSERK,
     0, RES_BASH | RES_COLD, 0,
     A | G | V, A | B | C | D | E | F | H | J | K | U | V
    },

    {
     "cat", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V
    },

    {
     "centipede", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_PIERCE | RES_COLD, VULN_BASH,
     A | B | G | O, A | C | K
    },

    {
     "dog", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | U | V
    },

    {
     "doll", FALSE,
     0, 0, 0,
     IMM_COLD | IMM_POISON | IMM_HOLY | IMM_NEGATIVE | IMM_MENTAL |
     IMM_DISEASE | IMM_DROWNING, RES_BASH | RES_LIGHT,
     VULN_SLASH | VULN_FIRE | VULN_ACID | VULN_LIGHTNING | VULN_ENERGY,
     E | J | M | cc, A | B | C | G | H | K
    },

    {
     "fido", FALSE,
     0, 0, OFF_DODGE | ASSIST_RACE,
     0, 0, VULN_MAGIC,
     A | B | G | V, A | C | D | E | F | H | J | K | Q | V
    },

    {
     "fox", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | V
    },

    {
     "goblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE, VULN_MAGIC,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "hobgoblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE | RES_POISON, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Y
    },

    {
     "kobold", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_POISON, VULN_MAGIC,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Q
    },

    {
     "lizard", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | X | cc, A | C | D | E | F | H | K | Q | V
    },

    {
     "modron", FALSE,
     0, AFF_INFRARED, ASSIST_RACE | ASSIST_ALIGN,
     IMM_CHARM | IMM_DISEASE | IMM_MENTAL | IMM_HOLY | IMM_NEGATIVE,
     RES_FIRE | RES_COLD | RES_ACID, 0,
     H, A | B | C | G | H | J | K
    },

    {
     "orc", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE, VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "pig", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K
    },

    {
     "rabbit", FALSE,
     0, 0, OFF_DODGE | OFF_FAST,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K
    },

    {
     "school monster", FALSE,
     ACT_NOALIGN, 0, 0,
     IMM_CHARM | IMM_SUMMON, 0, VULN_MAGIC,
     A | M | V, A | B | C | D | E | F | H | J | K | Q | U
    },

    {
     "snake", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | X | Y | cc, A | D | E | F | K | L | Q | V | X
    },

    {
     "song bird", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P
    },

    {
     "troll", FALSE,
     0, AFF_REGENERATION | AFF_INFRARED | AFF_DETECT_HIDDEN,
     OFF_BERSERK,
     0, RES_CHARM | RES_BASH, VULN_FIRE | VULN_ACID,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V
    },

    {
     "water fowl", FALSE,
     0, AFF_SWIM | AFF_FLYING, 0,
     0, RES_DROWNING, 0,
     A | G | W, A | C | D | E | F | H | K | P
    },

    {
     "wolf", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V
    },

    {
     "wyvern", FALSE,
     0, AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN,
     OFF_BASH | OFF_FAST | OFF_DODGE,
     IMM_POISON, 0, VULN_LIGHT,
     A | B | G | Z, A | C | D | E | F | H | J | K | Q | V | X
    },

    {
     "duergar", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC | RES_POISON | RES_DISEASE, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "gnome", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_POISON, VULN_BASH,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "svirfneblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC | RES_POISON | RES_DISEASE, VULN_BASH,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K
    },

    {
     "unique", FALSE,
     0, 0, 0,
     0, 0, 0,
     0, 0
    },

    {
     NULL, 0, 0, 0, 0, 0, 0
    }

};



/* TC961118 !!!!! donation needs number of races. It will be used

    in do_get and do_sac.

   NUMBER_RACES is defined in merc.h

*/





const struct pc_race_type pc_race_table[] = {
    {"null race", "", 0, {100, 100, 100, 100, 100, 100, 100},
     {""}, {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, 0},
/*
    {	"race name", 	short name, 	points,	{ class multipliers },
		{ bonus skills },
		{ base stats },		{ max stats },		size 
    },

*/
    {"Human", "Human", 0, {100, 100, 100, 100, 125, 125, 100},
     {""},
     {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, SIZE_MEDIUM},

    {"High-Elf", "H.Elf", 0, {100, 100, 100, 100, 125, 125, 100},
     {""},
     {10, 16, 14, 15, 13}, {15, 21, 19, 20, 18}, SIZE_MEDIUM},

    {"Dwarf", "Dwarf", 0, {100, 100, 100, 100, 125, 125, 100},
     {"bash"},
     {16, 12, 14, 11, 16}, {21, 17, 19, 16, 21}, SIZE_MEDIUM},

    {"Giant", "Giant", 0, {100, 100, 100, 100, 125, 125, 100},
     {""},
     {16, 11, 13, 12, 16}, {21, 16, 18, 16, 21}, SIZE_HUGE},

    {"Dragonkin", "Drago", 0, {200, 200, 200, 200, 225, 225, 200},
     {""},
     {16, 16, 16, 16, 16}, {21, 21, 21, 21, 21}, SIZE_MEDIUM},

    {"Vampire", "Vampr", 0, {125, 125, 125, 125, 150, 150, 200},
     {""},
     {16, 14, 15, 14, 14}, {21, 19, 20, 19, 19}, SIZE_MEDIUM},

    {"Dark-Elf", "D.Elf", 0, {125, 125, 125, 125, 150, 150, 200},
     {""},
     {11, 16, 14, 16, 13}, {16, 21, 19, 21, 18}, SIZE_MEDIUM},

    {"Cyborg", "Cybor", 0, {150, 150, 150, 150, 160, 160, 200},
     {""},
     {15, 15, 16, 14, 14}, {20, 20, 21, 19, 19}, SIZE_MEDIUM},

    {"Esper", "Esper", 0, {100, 100, 100, 100, 100, 100, 100},
     {"meditiation"},
     {11, 16, 16, 16, 13}, {16, 21, 21, 21, 19}, SIZE_MEDIUM},

    {"Demon", "Demon", 0, {100, 100, 100, 100, 100, 100, 100},
     {"battle rage"},
     {16, 11, 13, 16, 16}, {21, 16, 18, 21, 21}, SIZE_MEDIUM},

    {"Saiyan", "Saiya", 0, {100, 100, 100, 100, 100, 100, 100},
     {""},
     {16, 13, 13, 15, 16}, {21, 18, 18, 20, 21}, SIZE_MEDIUM},

    {"Feline", "Fline", 0, {100, 100, 100, 100, 100, 100, 100},
     {""},
     {16, 16, 14, 16, 13}, {21, 21, 19, 21, 18}, SIZE_MEDIUM},
};









/*

 * Class table.

 */

const struct class_type class_table[MAX_CLASS] = {

    /*EE960411 The whole class_table modified to Eddings-classes */

    {

     "Mage", "Mag", STAT_INT, OBJ_VNUM_SCHOOL_DAGGER,

     {3018, 3018}, 75, 20, 6, {8, 10}, {16, 20}, {4, 6},

     "mage basics", "mage default"},



    {

     "Cleric", "Cle", STAT_WIS, OBJ_VNUM_SCHOOL_MACE,

     {3003, 3003}, 75, 20, 2, {9, 11}, {14, 18}, {5, 7},

     "cleric basics", "cleric default"},



    {

     "Thief", "Thi", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,

     {3028, 3028}, 75, 20, -4, {11, 14}, {6, 8}, {7, 9},

     "thief basics", "thief default"},



    {

     "Warrior", "War", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,

     {3022, 3022}, 75, 20, -10, {13, 17}, {3, 4}, {8, 10},

     "warrior basics", "warrior default"},



    {

     "Ninja", "Nin", STAT_DEX, OBJ_VNUM_SCHOOL_SWORD,

     {3368, 3368}, 75, 20, -4, {12, 15}, {4, 6}, {9, 11},

     "ninja basics", "ninja default"},



    {

     "Monk", "Mon", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,

     {3366, 3366}, 75, 20, -4, {11, 15}, {5, 7}, {10, 12},

     "monk basics", "monk default"},



    {

     "knight", "Kni", STAT_STR, OBJ_VNUM_SCHOOL_MACE,

     {3367, 3367}, 75, 20, -15, {13, 20}, {6, 8}, {7, 9},

     "knight basics", "knight default"},

};







/*

 * Titles.

 */

char *const title_table[MAX_CLASS][MAX_LEVEL + 1][2] = {

    {

     {"Man", "Woman"},




     {"Apprent`&i``ce of `A+`&*``+ `4Mag`Vi`4c`` +`&*`A+``",
      "Apprent`&i``ce of `A+`&*``+ `4Mag`Vi`4c ``+`&*`A+``"},

     {"One Source Student", "One Source Student"},

     {"Scholar of One Source", "Scholar of One Source"},

     {"Delver in One Source", "Delveress in One Source"},

     {"Medium of One Source", "Medium of One Source"},



     {"Scribe of One Source", "Scribess of One Source"},

     {"Seer", "Seeress"},

     {"Sage", "Sage"},

     {"Illusionist", "Illusionist"},

     {"Abjurer", "Abjuress"},



     {"Invoker", "Invoker"},

     {"Enchanter", "Enchantress"},

     {"Conjurer", "Conjuress"},

     {"One Sourceian", "Witch"},

     {"Creator", "Creator"},



     {"Savant", "Savant"},

     {"Magus", "Craftess"},

     {"Master Magus", "Mistress Crartess"},

     {"Warlock", "War Witch"},

     {"Sorcerer", "Sorceress"},



     {"Elder Sorcerer", "Elder Sorceress"},

     {"Grand Sorcerer", "Grand Sorceress"},

     {"Great Sorcerer", "Great Sorceress"},

     {"Golem Maker", "Golem Maker"},

     {"Greater Golem Maker", "Greater Golem Maker"},



     {"Maker of Stones", "Maker of Stones",},

     {"Maker of Potions", "Maker of Potions",},

     {"Maker of Scrolls", "Maker of Scrolls",},

     {"Maker of Wands", "Maker of Wands",},

     {"Maker of Staves", "Maker of Staves",},



     {"Demon Summoner", "Demon Summoner"},

     {"Greater Demon Summoner", "Greater Demon Summoner"},

     {"Dragon Charmer", "Dragon Charmer"},

     {"Greater Dragon Charmer", "Greater Dragon Charmer"},

     {"Master of all One Source", "Master of all One Source"},



     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},



     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},



     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},

     {"Welder of One Source", "Welder of One Source"},



     {"One Source Hero", "One Source Heroine"},

     {"Immortal", "Immortal"},

     {"Guild Leader", "Guild Leader"},

     {"Very Important Person", "Very Important Person"},

     {"Builder", "Builder"},

     {"Newbie Helper", "Newbie Helper"},

     {"Quest Assistant", "Quest Assistant"},

     {"Quest Leader", "Quest Leader"},

     {"Security_ADM", "Security_ADM"},

     {"Implementor", "Implementress"},

     {"Wizard", "Wizard"},

     },



    {

     {"Man", "Woman"},



     {"Bel`&i``ever", "Bel`&i``ever"},

     {"Attendant", "Attendant"},

     {"Acolyte", "Acolyte"},

     {"Novice", "Novice"},

     {"Missionary", "Missionary"},



     {"Adept", "Adept"},

     {"Deacon", "Deaconess"},

     {"Vicar", "Vicaress"},

     {"Priest", "Priestess"},

     {"Minister", "Lady Minister"},



     {"Canon", "Canon"},

     {"Levite", "Levitess"},

     {"Curate", "Curess"},

     {"Monk", "Nun"},

     {"Healer", "Healess"},



     {"Chaplain", "Chaplain"},

     {"Expositor", "Expositress"},

     {"Bishop", "Bishop"},

     {"Arch Bishop", "Arch Lady of the Church"},

     {"Patriarch", "Matriarch"},



     {"Elder Patriarch", "Elder Matriarch"},

     {"Grand Patriarch", "Grand Matriarch"},

     {"Great Patriarch", "Great Matriarch"},

     {"Demon Killer", "Demon Killer"},

     {"Greater Demon Killer", "Greater Demon Killer"},



     {"Cardinal of the Sea", "Cardinal of the Sea"},

     {"Cardinal of the Earth", "Cardinal of the Earth"},

     {"Cardinal of the Air", "Cardinal of the Air"},

     {"Cardinal of the Ether", "Cardinal of the Ether"},

     {"Cardinal of the Heavens", "Cardinal of the Heavens"},



     {"Avatar of an Immortal", "Avatar of an Immortal"},

     {"Avatar of a Deity", "Avatar of a Deity"},

     {"Avatar of a Supremity", "Avatar of a Supremity"},

     {"Avatar of an Implementor", "Avatar of an Implementor"},

     {"Master of all Divinity", "Mistress of all Divinity"},



     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},



     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},



     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},

     {"Master Cleric", "Master Cleric"},



     {"Holy Hero", "Holy Heroine"},

     {"Immortal", "Immortal"},

     {"Guild Leader", "Guild Leader"},

     {"Very Important Person", "Very Important Person"},

     {"Builder", "Builder"},

     {"Newbie Helper", "Newbie Helper"},

     {"Quest Assistant", "Quest Assistant"},

     {"Quest Leader", "Quest Leader"},

     {"Security_ADM", "Security_ADM"},

     {"Implementor", "Implementress"},

     {"Wizard", "Wizard"},

     },



    {

     {"Man", "Woman"},



     {"Pilferer", "Pilferess"},

     {"Footpad", "Footpad"},

     {"Filcher", "Filcheress"},

     {"Pick-Pocket", "Pick-Pocket"},

     {"Sneak", "Sneak"},



     {"Pincher", "Pincheress"},

     {"Cut-Purse", "Cut-Purse"},

     {"Snatcher", "Snatcheress"},

     {"Sharper", "Sharpress"},

     {"Rogue", "Rogue"},



     {"Robber", "Robber"},

     {"Magsman", "Magswoman"},

     {"Highwayman", "Highwaywoman"},

     {"Burglar", "Burglaress"},

     {"Thief", "Thief"},



     {"Knifer", "Knifer"},

     {"Quick-Blade", "Quick-Blade"},

     {"Killer", "Murderess"},

     {"Brigand", "Brigand"},

     {"Cut-Throat", "Cut-Throat"},



     {"Spy", "Spy"},

     {"Grand Spy", "Grand Spy"},

     {"Master Spy", "Master Spy"},

     {"Assassin", "Assassin"},

     {"Greater Assassin", "Greater Assassin"},



     {"Master of Vision", "Mistress of Vision"},

     {"Master of Hearing", "Mistress of Hearing"},

     {"Master of Smell", "Mistress of Smell"},

     {"Master of Taste", "Mistress of Taste"},

     {"Master of Touch", "Mistress of Touch"},



     {"Crime Lord", "Crime Mistress"},

     {"Infamous Crime Lord", "Infamous Crime Mistress"},

     {"Greater Crime Lord", "Greater Crime Mistress"},

     {"Master Crime Lord", "Master Crime Mistress"},

     {"Godfather", "Godmother"},



     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},



     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},



     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},

     {"Master Thief", "Master Thief"},



     {"Assassin Hero", "Assassin Heroine"},

     {"Immortal", "Immortal"},

     {"Guild Leader", "Guild Leader"},

     {"Very Important Person", "Very Important Person"},

     {"Builder", "Builder"},

     {"Newbie Helper", "Newbie Helper"},

     {"Quest Assistant", "Quest Assistant"},

     {"Quest Leader", "Quest Leader"},

     {"Security_ADM", "Security_ADM"},

     {"Implementor", "Implementress"},

     {"Wizard", "Wizard"},

     },



    {

     {"Man", "Woman"},



     {"Swordpupil", "Swordpupil"},

     {"Recruit", "Recruit"},

     {"Sentry", "Sentress"},

     {"Fighter", "Fighter"},

     {"Soldier", "Soldier"},



     {"Warrior", "Warrior"},

     {"Veteran", "Veteran"},

     {"Swordsman", "Swordswoman"},

     {"Fencer", "Fenceress"},

     {"Combatant", "Combatess"},



     {"Hero", "Heroine"},

     {"Myrmidon", "Myrmidon"},

     {"Swashbuckler", "Swashbuckleress"},

     {"Mercenary", "Mercenaress"},

     {"Swordmaster", "Swordmistress"},



     {"Lieutenant", "Lieutenant"},

     {"Champion", "Lady Champion"},

     {"Dragoon", "Lady Dragoon"},

     {"Cavalier", "Lady Cavalier"},

     {"Knight", "Lady Knight"},



     {"Grand Knight", "Grand Knight"},

     {"Master Knight", "Master Knight"},

     {"Paladin", "Paladin"},

     {"Grand Paladin", "Grand Paladin"},

     {"Demon Slayer", "Demon Slayer"},



     {"Greater Demon Slayer", "Greater Demon Slayer"},

     {"Dragon Slayer", "Dragon Slayer"},

     {"Greater Dragon Slayer", "Greater Dragon Slayer"},

     {"Underlord", "Underlord"},

     {"Overlord", "Overlord"},



     {"Baron of Thunder", "Baroness of Thunder"},

     {"Baron of Storms", "Baroness of Storms"},

     {"Baron of Tornadoes", "Baroness of Tornadoes"},

     {"Baron of Hurricanes", "Baroness of Hurricanes"},

     {"Baron of Meteors", "Baroness of Meteors"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Knight Hero", "Knight Heroine"},

     {"Immortal", "Immortal"},

     {"Guild Leader", "Guild Leader"},

     {"Very Important Person", "Very Important Person"},

     {"Builder", "Builder"},

     {"Newbie Helper", "Newbie Helper"},

     {"Quest Assistant", "Quest Assistant"},

     {"Quest Leader", "Quest Leader"},

     {"Security_ADM", "Security_ADM"},

     {"Implementor", "Implementress"},

     {"Wizard", "Wizard"},

     },



    {

     {"Man", "Woman"},



     {"`AN`!i`ANJA`` Student", "`AN`!i`ANJA`` Student"},

     {"Recruit", "Recruit"},

     {"Sentry", "Sentress"},

     {"Fighter", "Fighter"},

     {"Soldier", "Soldier"},



     {"Warrior", "Warrior"},

     {"Veteran", "Veteran"},

     {"Swordsman", "Swordswoman"},

     {"Fencer", "Fenceress"},

     {"Combatant", "Combatess"},



     {"Hero", "Heroine"},

     {"Myrmidon", "Myrmidon"},

     {"Swashbuckler", "Swashbuckleress"},

     {"Mercenary", "Mercenaress"},

     {"Swordmaster", "Swordmistress"},



     {"Lieutenant", "Lieutenant"},

     {"Champion", "Lady Champion"},

     {"Dragoon", "Lady Dragoon"},

     {"Cavalier", "Lady Cavalier"},

     {"Knight", "Lady Knight"},



     {"Grand Knight", "Grand Knight"},

     {"Master Knight", "Master Knight"},

     {"Paladin", "Paladin"},

     {"Grand Paladin", "Grand Paladin"},

     {"Demon Slayer", "Demon Slayer"},



     {"Greater Demon Slayer", "Greater Demon Slayer"},

     {"Dragon Slayer", "Dragon Slayer"},

     {"Greater Dragon Slayer", "Greater Dragon Slayer"},

     {"Underlord", "Underlord"},

     {"Overlord", "Overlord"},



     {"Baron of Thunder", "Baroness of Thunder"},

     {"Baron of Storms", "Baroness of Storms"},

     {"Baron of Tornadoes", "Baroness of Tornadoes"},

     {"Baron of Hurricanes", "Baroness of Hurricanes"},

     {"Baron of Meteors", "Baroness of Meteors"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Knight Hero", "Knight Heroine"},

     {"Immortal", "Immortal"},

     {"Guild Leader", "Guild Leader"},

     {"Very Important Person", "Very Important Person"},

     {"Builder", "Builder"},

     {"Newbie Helper", "Newbie Helper"},

     {"Quest Assistant", "Quest Assistant"},

     {"Quest Leader", "Quest Leader"},

     {"Security_ADM", "Security_ADM"},

     {"Implementor", "Implementress"},

     {"Wizard", "Wizard"},

     },



    {

     {"Man", "Woman"},




     {"Student of the `&Y`Ai`&NG``/`AY`&a`ANG``",
      "Student of `&Y`Ai`&NG``/`AY`&a`ANG``"},

     {"Recruit", "Recruit"},

     {"Sentry", "Sentress"},

     {"Fighter", "Fighter"},

     {"Soldier", "Soldier"},



     {"Warrior", "Warrior"},

     {"Veteran", "Veteran"},

     {"Swordsman", "Swordswoman"},

     {"Fencer", "Fenceress"},

     {"Combatant", "Combatess"},



     {"Hero", "Heroine"},

     {"Myrmidon", "Myrmidon"},

     {"Swashbuckler", "Swashbuckleress"},

     {"Mercenary", "Mercenaress"},

     {"Swordmaster", "Swordmistress"},



     {"Lieutenant", "Lieutenant"},

     {"Champion", "Lady Champion"},

     {"Dragoon", "Lady Dragoon"},

     {"Cavalier", "Lady Cavalier"},

     {"Knight", "Lady Knight"},



     {"Grand Knight", "Grand Knight"},

     {"Master Knight", "Master Knight"},

     {"Paladin", "Paladin"},

     {"Grand Paladin", "Grand Paladin"},

     {"Demon Slayer", "Demon Slayer"},



     {"Greater Demon Slayer", "Greater Demon Slayer"},

     {"Dragon Slayer", "Dragon Slayer"},

     {"Greater Dragon Slayer", "Greater Dragon Slayer"},

     {"Underlord", "Underlord"},

     {"Overlord", "Overlord"},



     {"Baron of Thunder", "Baroness of Thunder"},

     {"Baron of Storms", "Baroness of Storms"},

     {"Baron of Tornadoes", "Baroness of Tornadoes"},

     {"Baron of Hurricanes", "Baroness of Hurricanes"},

     {"Baron of Meteors", "Baroness of Meteors"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},

     {"Master Warrior", "Master Warrior"},



     {"Knight Hero", "Knight Heroine"},

     {"Immortal", "Immortal"},

     {"Guild Leader", "Guild Leader"},

     {"Very Important Person", "Very Important Person"},

     {"Builder", "Builder"},

     {"Newbie Helper", "Newbie Helper"},

     {"Quest Assistant", "Quest Assistant"},

     {"Quest Leader", "Quest Leader"},

     {"Security_ADM", "Security_ADM"},

     {"Implementor", "Implementress"},

     {"Wizard", "Wizard"},

     },



    {

     {"Man", "Woman"},



     {"Believer", "Believer"},

     {"Attendant", "Attendant"},

     {"Acolyte", "Acolyte"},

     {"Novice", "Novice"},

     {"Missionary", "Missionary"},



     {"Adept", "Adept"},

     {"Deacon", "Deaconess"},

     {"Vicar", "Vicaress"},

     {"Priest", "Priestess"},

     {"Minister", "Lady Minister"},



     {"Canon", "Canon"},

     {"Levite", "Levitess"},

     {"Curate", "Curess"},

     {"Monk", "Nun"},

     {"Healer", "Healess"},



     {"Chaplain", "Chaplain"},

     {"Expositor", "Expositress"},

     {"Bishop", "Bishop"},

     {"Arch Bishop", "Arch Lady of the Church"},

     {"Patriarch", "Matriarch"},



     {"Elder Patriarch", "Elder Matriarch"},

     {"Grand Patriarch", "Grand Matriarch"},

     {"Great Patriarch", "Great Matriarch"},

     {"Demon Killer", "Demon Killer"},

     {"Greater Demon Killer", "Greater Demon Killer"},



     {"Cardinal of the Sea", "Cardinal of the Sea"},

     {"Cardinal of the Earth", "Cardinal of the Earth"},

     {"Cardinal of the Air", "Cardinal of the Air"},

     {"Cardinal of the Ether", "Cardinal of the Ether"},

     {"Cardinal of the Heavens", "Cardinal of the Heavens"},



     {"Avatar of an Immortal", "Avatar of an Immortal"},

     {"Avatar of a Deity", "Avatar of a Deity"},

     {"Avatar of a Supremity", "Avatar of a Supremity"},

     {"Avatar of an Implementor", "Avatar of an Implementor"},

     {"Master of all Divinity", "Mistress of all Divinity"},



     /*EE960411 */

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},



     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},



     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},

     {"Master Knight", "Master Knight"},



     {"Holy Hero", "Holy Heroine"},

     {"Immortal", "Immortal"},

     {"Guild Leader", "Guild Leader"},

     {"Very Important Person", "Very Important Person"},

     {"Builder", "Builder"},

     {"Newbie Helper", "Newbie Helper"},

     {"Quest Assistant", "Quest Assistant"},

     {"Quest Leader", "Quest Leader"},

     {"Security_ADM", "Security_ADM"},

     {"Implementor", "Implementress"},

     {"Wizard", "Wizard"},

     }

};







/*

 * Attribute bonus tables.

 */

const struct str_app_type str_app[26] = {

    {-5, -4, 0, 0},		/* 0  */

    {-5, -4, 3, 1},		/* 1  */

    {-3, -2, 3, 2},

    {-3, -1, 10, 3},		/* 3  */

    {-2, -1, 25, 4},

    {-2, -1, 55, 5},		/* 5  */

    {-1, 0, 80, 6},

    {-1, 0, 90, 7},

    {0, 0, 100, 8},

    {0, 0, 100, 9},

    {0, 0, 115, 10},		/* 10  */

    {0, 0, 115, 11},

    {0, 0, 130, 12},

    {0, 0, 130, 13},		/* 13  */

    {0, 1, 140, 14},

    {1, 1, 150, 15},		/* 15  */

    {1, 2, 165, 16},

    {2, 3, 180, 22},

    {2, 3, 200, 25},		/* 18  */

    {3, 4, 225, 30},

    {3, 5, 250, 35},		/* 20  */

    {4, 6, 300, 40},

    {4, 6, 350, 45},

    {5, 7, 400, 50},

    {5, 8, 450, 55},

    {7, 10, 550, 70}		/* 25   */

};







const struct int_app_type int_app[26] = {

    {3},			/*  0 */

    {5},			/*  1 */

    {7},

    {8},			/*  3 */

    {9},

    {10},			/*  5 */

    {11},

    {12},

    {13},

    {15},

    {17},			/* 10 */

    {19},

    {22},

    {25},

    {28},

    {31},			/* 15 */

    {34},

    {37},

    {40},			/* 18 */

    {44},

    {49},			/* 20 */

    {55},

    {60},

    {70},

    {80},

    {85}			/* 25 */

};







const struct wis_app_type wis_app[26] = {

    {-4, 0},			/*  0 */

    {-3, 0},			/*  1 */

    {-3, 0},

    {-3, 0},			/*  3 */

    {-2, 0},

    {-2, 1},			/*  5 */

    {-2, 1},

    {-1, 1},

    {-1, 1},

    {-1, 1},

    {0, 1},			/* 10 */

    {0, 1},

    {1, 1},

    {1, 1},

    {2, 1},

    {2, 2},			/* 15 */

    {3, 2},

    {3, 2},

    {4, 3},			/* 18 */

    {4, 3},

    {5, 3},			/* 20 */

    {5, 3},

    {6, 4},

    {6, 4},

    {7, 4},

    {8, 5}				/* 25 */

};







const struct dex_app_type dex_app[26] = {

    {-5, 60},			/* 0 */

    {-5, 50},			/* 1 */

    {-4, 50},

    {-4, 40},

    {-3, 30},

    {-3, 20},			/* 5 */

    {-2, 10},

    {-2, 0},

    {-1, 0},

    {-1, 0},

    {0, 0},			/* 10 */

    {0, 0},

    {0, 0},

    {0, 0},

    {0, 0},

    {0, -10},			/* 15 */

    {1, -15},

    {1, -20},

    {2, -30},

    {2, -40},

    {3, -50},			/* 20 */

    {3, -60},

    {4, -75},

    {4, -90},

    {5, -105},

    {6, -140}			/* 25 */

};





const struct con_app_type con_app[26] = {

    {-5, 20},			/*  0 */

    {-5, 25},			/*  1 */

    {-4, 30},

    {-4, 35},			/*  3 */

    {-3, 40},

    {-3, 45},			/*  5 */

    {-2, 50},

    {-2, 55},

    {-1, 60},

    {-1, 65},

    {0, 70},			/* 10 */

    {0, 75},

    {0, 80},

    {0, 85},

    {0, 88},

    {0, 90},			/* 15 */

    {1, 95},

    {2, 97},

    {3, 99},			/* 18 */

    {4, 99},

    {5, 99},			/* 20 */

    {6, 99},

    {7, 99},

    {8, 99},

    {9, 99},

    {10, 99}			/* 25 */

};







/*

 * Liquid properties.

 * Used in world.obj.

 */

const struct liq_type liq_table[] = {

/*    name			color	proof, full, thirst, food, ssize */

    {"water", "clear", {0, 1, 10, 0, 16}},

    {"beer", "amber", {12, 1, 8, 1, 12}},

    {"red wine", "burgundy", {30, 1, 8, 1, 5}},

    {"ale", "brown", {15, 1, 8, 1, 12}},

    {"dark ale", "dark", {16, 1, 8, 1, 12}},



    {"whisky", "golden", {120, 1, 5, 0, 2}},

    {"lemonade", "pink", {0, 1, 9, 2, 12}},

    {"firebreather", "boiling", {190, 0, 4, 0, 2}},

    {"local specialty", "clear", {151, 1, 3, 0, 2}},

    {"slime mold juice", "green", {0, 2, -8, 1, 2}},



    {"milk", "white", {0, 2, 9, 3, 12}},

    {"tea", "tan", {0, 1, 8, 0, 6}},

    {"coffee", "black", {0, 1, 8, 0, 6}},

    {"blood", "red", {0, 2, -1, 2, 6}},

    {"salt water", "clear", {0, 1, -2, 0, 1}},



    {"coke", "brown", {0, 2, 9, 2, 12}},

    {"root beer", "brown", {0, 2, 9, 2, 12}},

    {"elvish wine", "green", {35, 2, 8, 1, 5}},

    {"white wine", "golden", {28, 1, 8, 1, 5}},

    {"champagne", "golden", {32, 1, 8, 1, 5}},



    {"mead", "honey-colored", {34, 2, 8, 2, 12}},

    {"rose wine", "pink", {26, 1, 8, 1, 5}},

    {"benedictine wine", "burgundy", {40, 1, 8, 1, 5}},

    {"vodka", "clear", {130, 1, 5, 0, 2}},

    {"cranberry juice", "red", {0, 1, 9, 2, 12}},



    {"orange juice", "orange", {0, 2, 9, 3, 12}},

    {"absinthe", "green", {200, 1, 4, 0, 2}},

    {"brandy", "golden", {80, 1, 5, 0, 4}},

    {"aquavit", "clear", {140, 1, 5, 0, 2}},

    {"schnapps", "clear", {90, 1, 5, 0, 2}},



    {"icewine", "purple", {50, 2, 6, 1, 5}},

    {"amontillado", "burgundy", {35, 2, 8, 1, 5}},

    {"sherry", "red", {38, 2, 7, 1, 5}},

    {"framboise", "red", {50, 1, 7, 1, 5}},

    {"rum", "amber", {151, 1, 4, 0, 2}},



    {"cordial", "clear", {100, 1, 5, 0, 2}},

    {"tang", "yellow", {0, 1, 5, 0, 2}},

    {NULL, NULL, {0, 0, 0, 0, 0}}

};







/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */

#define SLOT(n)	n

const struct skill_type skill_table[MAX_SKILL] = {

/*
 * One Source spells.
 */

    {
     "reserved", {99, 99, 99, 99, 99, 99, 99}, {99, 99, 99, 99, 99, 99,	99},
     0, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(0), 0, 0,
     "", "", ""},

    {
     "acid blast", {33, 93, 38, 43, 38, 93, 33}, {1, 2, 2, 2, 2, 2, 2},
     spell_acid_blast, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(70), 20, 12,
     "acid blast", "!Acid Blast!"},

    {
     "anger", {2, 2, 7, 12, 7, 5, 2}, {2, 2, 2, 1, 2, 2, 2},
     spell_anger, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(524), 5, 12,
     "", "You lose your temper.", ""},

    {
     "armor", {4, 4, 9, 13, 9, 7, 4}, {1, 1, 2, 2, 1, 2, 2},
     spell_armor, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(1), 5, 12,
     "", "You feel less armored.", ""},

    {
     "black hole", {63, 71, 93, 93, 93, 93, 93}, {3, 3, 3, 3, 3, 3, 3},
     spell_black_hole, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(52), 60, 24,
     "black hole", "", ""},

    {
     "bless", {93, 4, 93, 93, 93, 7, 4}, {2, 1, 2, 2, 2, 2, 2},
     spell_bless, song_null, TAR_OBJ_CHAR_DEF, POS_STANDING,
     NULL, SLOT(3), 5, 12,
     "", "You feel less righteous.", "$p's holy aura fades."},

    {
     "blindness", {10, 10, 15, 20, 15, 13, 10}, {1, 1, 2, 2, 2, 2, 2},
     spell_blindness, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_blindness, SLOT(4), 5, 12,
     "", "You can see again.", ""},

    {
     "burning hands", {12, 93, 17, 23, 17, 93, 12}, {1, 2, 2, 2, 2, 2, 2},
     spell_burning_hands, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(5), 15, 12,
     "burning hands", "!Burning Hands!", ""},

    {
     "call lightning", {93, 16, 93, 93, 93, 93, 16}, {1, 1, 2, 2, 1, 2, 2},
     spell_call_lightning, song_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT(6), 15, 12,
     "lightning bolt", "!Call Lightning!", ""},

    {
     "calm", {16, 16, 21, 26, 21, 19, 16}, {2, 1, 2, 2, 2, 2, 2},
     spell_calm, song_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT(509), 30, 12,
     "", "You have lost your peace of mind.", ""},

    {
     "cancellation", {16, 16, 21, 26, 21, 19, 16}, {1, 1, 2, 2, 2, 2, 2},
     spell_cancellation, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(507), 20, 12,
     "" "!cancellation!", ""},

    {
     "cause critical", {93, 16, 93, 93, 93, 93, 16}, {2, 1, 2, 2, 2, 2, 2},
     spell_cause_critical, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(63), 20, 12,
     "spell of critical wounds", "!Cause Critical!", ""},

    {
     "cause light", {93, 6, 93, 93, 93, 93, 6}, {2, 1, 2, 2, 2, 2, 2},
     spell_cause_light, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(62), 15, 12,
     "spell of light wounds", "!Cause Light!", ""},

    {
     "cause serious", {93, 13, 93, 93, 93, 93, 13}, {2, 1, 2, 2, 2, 2, 2},
     spell_cause_serious, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(64), 17, 12,
     "spell of wounds", "!Cause Serious!", ""},

    {
     "chain lightning", {43, 93, 48, 53, 48, 93, 43}, {1, 2, 2, 2, 2, 2,						       2},
     spell_chain_lightning, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(500), 25, 12,
     "lightning", "!Chain Lightning!", ""},

    {
     "change sex", {49, 49, 53, 58, 53, 52, 49}, {2, 2, 2, 2, 3, 2, 2},
     spell_change_sex, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(82), 15, 12,
     "", "Your body feels familiar again.", ""},

    {
     "charm person", {20, 93, 25, 93, 93, 93, 20}, {1, 2, 2, 2, 2, 2, 2},
     spell_charm_person, song_null, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_charm_person, SLOT(7), 5, 12,
     "", "You feel more self-confident.", ""},

    {
     "chill touch", {6, 93, 11, 16, 11, 93, 6}, {1, 2, 2, 2, 2, 2, 2},
     spell_chill_touch, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(8), 15, 12,
     "chilling touch", "You feel less cold.", ""},

    {
     "colour spray", {12, 93, 17, 22, 17, 93, 12}, {1, 2, 2, 2, 2, 2, 2},
     spell_colour_spray, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(10), 15, 12,
     "colour spray", "!Colour Spray!", ""},

    {
     "concentration", {33, 33, 38, 43, 38, 36, 33}, {2, 2, 2, 1, 2, 2, 2},
     spell_concentration, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(525), 20, 12,
     "", "You lose your train of thought.", ""},

    {
     "continual light", {8, 8, 93, 93, 93, 93, 8}, {1, 1, 2, 2, 1, 2, 2},
     spell_continual_light, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(57), 7, 12,
     "", "!Continual Light!", ""},

    {
     "control weather", {93, 14, 93, 93, 93, 93, 14}, {1, 1, 2, 2, 1, 2, 2},
     spell_control_weather, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(11), 25, 12,
     "", "!Control Weather!", ""},

    {
     "create food", {8, 8, 93, 93, 93, 93, 8}, {1, 1, 2, 2, 1, 2, 2},
     spell_create_food, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(12), 5, 12,
     "", "!Create Food!", ""},

    {
     "create rose", {6, 6, 93, 93, 93, 93, 6}, {1, 1, 2, 2, 1, 2, 2},
     spell_create_rose, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(511), 30, 12,
     "", "!Create Rose!", ""},

    {
     "create spring", {12, 12, 93, 93, 93, 93, 12}, {1, 1, 2, 2, 1, 2, 2},
     spell_create_spring, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(80), 20, 12,
     "", "!Create Spring!", ""},

    {
     "create water", {10, 10, 93, 93, 93, 93, 10}, {1, 1, 2, 2, 1, 2, 2},
     spell_create_water, song_null, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT(13), 5, 12,
     "", "!Create Water!", ""},

    {
     "cry", {24, 24, 29, 34, 29, 27, 24}, {2, 2, 2, 1, 2, 2, 2},
     spell_cry, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(526), 15, 12,
     "", "You gain control and stop crying", ""},

    {
     "cure blindness", {93, 6, 93, 93, 93, 9, 6}, {2, 1, 2, 2, 2, 2, 2},
     spell_cure_blindness, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(14), 5, 12,
     "", "!Cure Blindness!", ""},

    {
     "cure critical", {93, 13, 93, 93, 93, 16, 13}, {2, 1, 2, 2, 2, 2, 2},
     spell_cure_critical, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(15), 20, 12,
     "", "!Cure Critical!", ""},

    {
     "cure disease", {93, 17, 93, 93, 93, 20, 17}, {2, 1, 2, 2, 2, 2, 2},
     spell_cure_disease, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(501), 20, 12,
     "", "!Cure Disease!", ""},

    {
     "cure light", {93, 2, 93, 93, 93, 93, 2}, {2, 1, 2, 2, 2, 2, 2},
     spell_cure_light, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(16), 10, 12,
     "", "!Cure Light!", ""},

    {
     "cure poison", {93, 14, 93, 93, 93, 17, 14}, {2, 1, 2, 2, 2, 2, 2},
     spell_cure_poison, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(43), 5, 12,
     "", "!Cure Poison!", ""},

    {
     "cure serious", {93, 7, 93, 93, 93, 93, 7}, {2, 1, 2, 2, 2, 2, 2},
     spell_cure_serious, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(61), 15, 12,
     "", "!Cure Serious!", ""},

    {
     "curse", {18, 18, 23, 28, 23, 21, 18}, {1, 1, 2, 2, 2, 2, 2},
     spell_curse, song_null, TAR_OBJ_CHAR_OFF, POS_FIGHTING,
     &gsn_curse, SLOT(17), 20, 12,
     "curse", "The curse wears off.", "$p is no longer impure."},

    {
     "demi", {26, 30, 32, 93, 93, 93, 26}, {1, 1, 2, 2, 2, 2, 2},
     spell_demi, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(49), 32, 18,
     "gravity anomaly", "!Demi!", ""},

    {
      "chibichibification",{29, 29, 34, 39, 34, 32, 29},{1, 1, 2, 2, 2, 2, 2},
      spell_chibichibi, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
      &gsn_chibichibi, SLOT(169), 75, 12,
      "", "You feel the pinkness fade..", ""},

    {
     "demonfire", {93, 45, 93, 93, 93, 48, 45}, {2, 1, 2, 2, 2, 2, 2},
     spell_demonfire, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(505), 20, 12,
     "torments", "!Demonfire!", ""},

    {
     "detect evil", {8, 8, 13, 93, 13, 11, 8}, {1, 1, 1, 2, 1, 1, 2},
     spell_detect_evil, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(18), 5, 12,
     "", "The red in your vision disappears.", ""},

    {
     "detect good", {8, 8, 13, 93, 13, 11, 8}, {1, 1, 1, 2, 1, 1, 2},
     spell_detect_good, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(513), 5, 12,
     "", "The gold in your vision disappears.", ""},

    {
     "detect hidden", {13, 13, 18, 93, 18, 16, 13}, {1, 1, 1, 2, 1, 1, 2},
     spell_detect_hidden, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(44), 5, 12,
     "", "You feel less aware of your surroundings.", ""},

    {
     "detect invis", {5, 5, 10, 93, 10, 8, 5}, {1, 1, 2, 2, 1, 2, 2},
     spell_detect_invis, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(19), 5, 12,
     "", "You no longer see invisible objects.", ""},

    {
     "detect magic", {7, 7, 12, 93, 12, 10, 7}, {1, 1, 2, 2, 1, 2, 2},
     spell_detect_magic, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(20), 5, 12,
     "", "The detect magic wears off.", ""},

    {
     "detect poison", {9, 9, 14, 93, 14, 12, 9}, {1, 1, 1, 2, 1, 1, 2},
     spell_detect_poison, song_null, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT(21), 5, 12,
     "", "!Detect Poison!", ""},

    {
     "dispel evil", {93, 15, 93, 93, 93, 18, 15}, {2, 1, 2, 2, 2, 2, 2},
     spell_dispel_evil, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(22), 15, 12,
     "dispel evil", "!Dispel Evil!", ""},

    {
     "dispel good", {93, 15, 93, 93, 93, 18, 15}, {2, 1, 2, 2, 2, 2, 2},
     spell_dispel_good, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(512), 15, 12,
     "dispel good", "!Dispel Good!", ""},

    {
     "dispel magic", {12, 12, 17, 21, 17, 15, 12}, {1, 1, 2, 2, 2, 2, 2},
     spell_dispel_magic, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(59), 15, 12,
     "", "!Dispel One Source!", ""},

    {
     "earthquake", {93, 12, 93, 93, 93, 15, 12}, {2, 1, 2, 2, 2, 2, 2},
     spell_earthquake, song_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT(23), 15, 12,
     "earthquake", "!Earthquake!", ""},

    {				/* EE960502 modified so sendar's of any class can use enchant armor */
     "enchant armor", {32, 93, 93, 93, 93, 93, 32}, {3, 2, 2, 2, 2, 2, 2},
     spell_enchant_armor, song_null, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT(510), 100, 18,
     "", "!Enchant Armor!", ""},

    {				/* EE960502 modified so sendar's of any class can use enchant armor */
     "enchant weapon", {34, 93, 93, 93, 93, 93, 34}, {3, 2, 2, 2, 2, 2, 2},
     spell_enchant_weapon, song_null, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT(24), 100, 18,
     "", "!Enchant Weapon!", ""},

    {
     "energy drain", {18, 18, 23, 28, 23, 21, 18}, {1, 1, 2, 2, 2, 2, 2},
     spell_energy_drain, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(25), 35, 18,
     "energy drain", "!Energy Drain!", ""},

    {
     "faerie fire", {93, 6, 93, 93, 93, 93, 6}, {1, 1, 2, 2, 1, 2, 2},
     spell_faerie_fire, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_faerie_fire, SLOT(72), 5, 12,
     "faerie fire", "The pink aura around you fades away.", ""},

    {
     "faerie fog", {93, 18, 93, 93, 93, 93, 18}, {1, 1, 2, 2, 1, 2, 2},
     spell_faerie_fog, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(73), 12, 12,
     "faerie fog", "!Faerie Fog!", ""},

    {
     "farsight", {20, 20, 25, 93, 25, 23, 20}, {1, 1, 1, 2, 1, 1, 2},
     spell_farsight, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(521), 4, 6,
     "farsight", "!Farsight!", ""},

    {
     "fear", {13, 13, 18, 23, 18, 15, 13}, {2, 2, 2, 1, 2, 2, 2},
     spell_fear, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(528), 10, 12,
     "fear", "!Fear!", ""},

    {
     "fireball", {24, 93, 29, 34, 29, 93, 24}, {1, 2, 2, 2, 2, 2, 2},
     spell_fireball, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(26), 15, 12,
     "fireball", "!Fireball!", ""},

    {
     "fireproof", {16, 16, 21, 26, 21, 19, 16}, {1, 1, 2, 2, 2, 2, 2},
     spell_fireproof, song_null, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT(523), 10, 12,
     "", "", "$p's protective aura fades."},

    {
     "flamestrike", {93, 20, 93, 93, 93, 23, 20}, {2, 1, 2, 2, 2, 2, 2},
     spell_flamestrike, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(65), 20, 12,
     "flamestrike", "!Flamestrike!", ""},

    {
     "fly", {20, 20, 25, 30, 25, 23, 20}, {1, 1, 2, 2, 2, 2, 3},
     spell_fly, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(56), 10, 18,
     "", "You slowly float to the ground.", ""},

    {
     "floating disc", {13, 13, 93, 93, 93, 93, 13}, {1, 1, 2, 2, 1, 2, 2},
     spell_floating_disc, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(522), 40, 24,
     "", "!Floating disc!", ""},

    {
     "freefall", {38, 41, 93, 93, 93, 93, 93}, {2, 2, 2, 2, 2, 2, 2},
     spell_freefall, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(50), 35, 18,
     "gravity shift", "!Freefall!", ""},

    {
     "frenzy", {93, 28, 93, 93, 93, 31, 28}, {2, 1, 2, 2, 2, 2, 2},
     spell_frenzy, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(504), 30, 24,
     "", "Your rage ebbs.", ""},

    {
     "gate", {23, 23, 28, 33, 28, 26, 23}, {1, 1, 3, 3, 3, 3, 4},
     spell_gate, song_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT(83), 65, 12,
     "", "!Gate!", ""},

    {
     "giant strength", {22, 93, 93, 32, 28, 93, 22}, {1, 2, 2, 2, 2, 2, 2},
     spell_giant_strength, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(39), 20, 12,
     "", "You feel weaker.", ""},

    {
     "gravija", {51, 59, 93, 93, 93, 93, 93}, {2, 2, 2, 2, 2, 2, 2},
     spell_gravija, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(51), 48, 24,
     "gravity singularity", "!Gravija!", ""},

    {
     "harm", {93, 23, 93, 93, 93, 93, 23}, {2, 1, 2, 2, 2, 2, 2},
     spell_harm, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(27), 35, 12,
     "harm spell", "!Harm!," ""},

    {
     "haste", {22, 93, 93, 32, 28, 93, 22}, {1, 2, 2, 2, 2, 2, 2},
     spell_haste, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(502), 30, 12,
     "", "You feel yourself slow down.", ""},

    {
     "heal", {93, 21, 93, 93, 93, 93, 21}, {2, 1, 2, 2, 2, 2, 2},
     spell_heal, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(28), 50, 12,
     "", "!Heal!", ""},

    {
     "heavy blow", {15, 18, 22, 93, 93, 93, 15}, {1, 1, 2, 2, 2, 2, 2},
     spell_heavy_blow, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(48), 15, 12,
     "gravity bolt", "!Heavy Blow!", ""},

    {
     "mana", {95, 95, 95, 95, 95, 95, 95}, {10, 10, 10, 10, 10, 10, 10},
     spell_mana, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(45), 50, 12,
     "", "!Mana!", ""},

    {
     "heat metal", {93, 18, 93, 93, 93, 21, 18}, {2, 1, 2, 2, 2, 2, 2},
     spell_heat_metal, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(516), 25, 18,
     "spell", "!Heat Metal!", ""},

    {
     "hellbender", {25, 25, 25, 25, 25, 25, 25}, {1, 2, 1, 1, 1, 1, 1},
     spell_hellbender, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(531), 25, 20,
     "eternal fires of hell", "!Hellbender!", ""},

    {
     "holy word", {93, 49, 93, 93, 93, 52, 49}, {2, 2, 2, 2, 2, 2, 2},
     spell_holy_word, song_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT(506), 75, 24,
     "divine wrath", "!Holy Word!", ""},

    {
     "identify", {30, 30, 35, 93, 35, 33, 30}, {1, 1, 2, 2, 2, 2, 2},
     spell_identify, song_null, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT(53), 12, 24,
     "", "!Identify!", ""},

    {
     "infravision", {18, 93, 93, 28, 23, 93, 18}, {1, 2, 2, 2, 2, 2, 2},
     spell_infravision, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(77), 5, 18,
     "", "You no longer see in the dark.", ""},

    {
     "invisibility", {5, 93, 10, 93, 10, 93, 5}, {1, 2, 2, 2, 2, 2, 2},
     spell_invis, song_null, TAR_OBJ_CHAR_DEF, POS_STANDING,
     &gsn_invis, SLOT(29), 5, 12,
     "", "You are no longer invisible.",
     "$p fades into view."},

    {
     "know alignment", {18, 18, 23, 93, 23, 21, 18}, {1, 1, 2, 2, 2, 2, 2},
     spell_know_alignment, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(58), 9, 12,
     "", "!Know Alignment!", ""},

    {
     "light", {93, 93, 6, 6, 6, 93, 6}, {1, 1, 2, 2, 2, 2, 2},
     spell_light, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(9), 5, 12,
     "", "Your magical light fades away.", ""},

    {
     "lightning bolt", {15, 15, 20, 25, 20, 93, 15}, {1, 1, 2, 2, 1, 2, 2},
     spell_lightning_bolt, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(30), 15, 12,
     "lightning bolt", "!Lightning Bolt!", ""},

    {
     "locate person", {24, 24, 29, 93, 29, 27, 24}, {1, 1, 2, 2, 2, 2, 2},
     spell_locate_person, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(529), 24, 18,
     "", "!Locate Person!", ""},

    {
     "locate object", {18, 18, 23, 93, 23, 21, 18}, {1, 1, 2, 2, 2, 2, 2},
     spell_locate_object, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(31), 20, 18,
     "", "!Locate Object!", ""},

    {
     "magic missile", {1, 93, 6, 11, 6, 93, 1}, {1, 2, 2, 2, 2, 2, 2},
     spell_magic_missile, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(32), 15, 12,
     "magic missile", "!Magic Missile!", ""},

    {
     "mass healing", {93, 47, 93, 93, 93, 93, 47}, {2, 2, 2, 2, 2, 2, 2},
     spell_mass_healing, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(508), 100, 18,
     "", "!Mass Healing!", ""},

    {
     "mass invis", {44, 93, 49, 58, 49, 93, 44}, {1, 2, 1, 2, 2, 1, 2},
     spell_mass_invis, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_mass_invis, SLOT(69), 20, 24,
     "", "You are no longer invisible.", ""},

    {
     "nexus", {42, 42, 47, 52, 47, 45, 42}, {2, 2, 3, 3, 3, 3, 4},
     spell_nexus, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(520), 100, 36,
     "", "!Nexus!", ""},

    {
     "pass door", {28, 28, 33, 38, 33, 31, 28}, {1, 1, 2, 2, 2, 2, 3},
     spell_pass_door, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(74), 20, 12,
     "", "You feel solid again.", ""},

    {
     "plague", {26, 26, 31, 36, 31, 29, 26}, {1, 1, 2, 2, 2, 2, 2},
     spell_plague, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_plague, SLOT(503), 20, 12,
     "sickness", "Your sores vanish.", ""},

    {
     "poison", {17, 17, 21, 26, 21, 20, 17}, {1, 1, 2, 2, 2, 2, 2},
     spell_poison, song_null, TAR_OBJ_CHAR_OFF, POS_FIGHTING,
     &gsn_poison, SLOT(33), 10, 12,
     "poison", "You feel less sick.",
     "The poison on $p dries up."},

    {
     "portal", {37, 37, 42, 47, 42, 40, 37}, {2, 2, 3, 3, 3, 3, 4},
     spell_portal, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(519), 80, 24,
     "", "!Portal!", ""},

    {
     "protection evil", {12, 12, 17, 22, 17, 15, 12}, {1, 1, 2, 2, 2, 2, 2},
     spell_protection_evil, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(34), 5, 12,
     "", "You feel less protected.", ""},

    {
     "protection good", {12, 12, 17, 22, 17, 15, 12}, {1, 1, 2, 2, 2, 2, 2},
     spell_protection_good, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(514), 5, 12,
     "", "You feel less protected.", ""},

    {
     "psionic blast", {43, 43, 48, 52, 48, 45, 43}, {2, 2, 1, 2, 2, 1, 2},
     spell_psionic_blast, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(527), 30, 24,
     "psionic blast", "Your head stops hurting.", ""},

    {
     "ray of frost", {10, 93, 10, 18, 14, 93, 10}, {1, 2, 2, 2, 2, 2, 2},
     spell_ray_of_frost, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(37), 15, 12,
     "ray of frost", "!Ray of Frost!", ""},

    {
     "ray of truth", {93, 38, 93, 93, 93, 41, 38}, {2, 1, 2, 2, 2, 2, 2},
     spell_ray_of_truth, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(518), 20, 12,
     "ray of truth", "!Ray of Truth!", ""},

    {
     "recharge", {22, 93, 93, 32, 28, 93, 22}, {1, 2, 2, 2, 2, 2, 2},
     spell_recharge, song_null, TAR_OBJ_INV, POS_STANDING,
     NULL, SLOT(517), 60, 24,
     "", "!Recharge!", ""},

    {
     "refresh", {14, 14, 93, 24, 19, 93, 14}, {2, 1, 2, 2, 2, 2, 2},
     spell_refresh, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(81), 12, 18,
     "refresh", "!Refresh!", ""},

    {
     "remove curse", {93, 28, 93, 93, 93, 31, 28}, {2, 1, 2, 2, 2, 2, 2},
     spell_remove_curse, song_null, TAR_OBJ_CHAR_DEF, POS_STANDING,
     NULL, SLOT(35), 5, 12,
     "", "!Remove Curse!", ""},

    {
     "resistance", {18, 20, 22, 30, 24, 93, 18}, {1, 2, 2, 2, 2, 2, 2},
     spell_resistance, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(47), 20, 12,
     "", "Your magical resistance wears out.", ""},

    {
     "sanctuary", {29, 29, 34, 39, 34, 32, 29}, {1, 1, 2, 2, 2, 2, 2},
     spell_sanctuary, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     &gsn_sanctuary, SLOT(36), 75, 12,
     "", "The white aura around your body fades.", ""},

    {
     "shield", {20, 20, 25, 30, 25, 23, 20}, {1, 1, 2, 2, 2, 2, 2},
     spell_shield, song_null, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT(67), 12, 18,
     "", "Your force shield shimmers then fades away.", ""},

    {
     "shocking grasp", {14, 93, 19, 24, 19, 93, 14}, {1, 2, 2, 2, 2, 2, 2},
     spell_shocking_grasp, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(46), 15, 12,
     "shocking grasp", "!Shocking Grasp!", ""},

    {
     "sleep", {10, 93, 15, 93, 93, 93, 10}, {1, 2, 2, 2, 2, 2, 2},
     spell_sleep, song_null, TAR_CHAR_OFFENSIVE, POS_STANDING,
     &gsn_sleep, SLOT(38), 15, 12,
     "", "You feel less tired.", ""},

    {
     "slow", {36, 36, 41, 46, 41, 39, 36}, {1, 1, 2, 2, 2, 2, 2},
     spell_slow, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(515), 30, 12,
     "", "You feel yourself speed up.", ""},

    {
     "stone skin", {44, 44, 49, 54, 49, 47, 44}, {1, 1, 2, 2, 2, 2, 2},
     spell_stone_skin, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(66), 12, 18,
     "", "Your skin feels soft again.", ""},

    {
     "summon", {22, 22, 26, 31, 26, 25, 22}, {1, 1, 2, 2, 2, 2, 3},
     spell_summon, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(40), 50, 12,
     "", "!Summon!", ""},

    {
     "teleport", {26, 26, 31, 36, 31, 29, 26}, {1, 1, 2, 2, 2, 2, 3},
     spell_teleport, song_null, TAR_CHAR_SELF, POS_FIGHTING,
     NULL, SLOT(2), 35, 12,
     "", "!Teleport!", ""},

    {
     "true sight", {48, 48, 53, 93, 53, 51, 48}, {1, 1, 2, 2, 2, 2, 2},
     spell_true_sight, song_null, TAR_CHAR_SELF, POS_STANDING,
     NULL, SLOT(530), 30, 12,
     "", "Your vision blurs as it returns to normal.", ""},

    {
     "ventriloquate", {2, 93, 7, 93, 7, 93, 2}, {1, 2, 1, 2, 2, 1, 2},
     spell_ventriloquate, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(41), 5, 12,
     "", "!Ventriloquate!", ""},

    {
     "weaken", {22, 22, 27, 32, 27, 25, 22}, {1, 1, 2, 2, 2, 2, 2},
     spell_weaken, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(68), 20, 12,
     "spell", "You feel stronger.", ""},

    {
     "word of recall", {93, 93, 93, 93, 93, 93, 93}, {1, 1, 2, 2, 2, 2, 3},
     spell_word_of_recall, song_null, TAR_CHAR_SELF, POS_RESTING,
     NULL, SLOT(42), 5, 12,
     "", "!Word of Recall!", ""},

/*

 * Dragon breath

 */

    {
     "acid breath", {36, 93, 93, 93, 93, 93, 36}, {1, 2, 2, 2, 2, 2, 2},
     spell_acid_breath, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(200), 8, 12,
     "blast of acid", "!Acid Breath!", ""},

    {
     "fire breath", {43, 93, 93, 93, 93, 93, 43}, {1, 2, 2, 2, 2, 2, 2},
     spell_fire_breath, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(201), 9, 12,
     "blast of flame", "The smoke leaves your eyes.", ""},

    {
     "frost breath", {39, 93, 93, 93, 93, 93, 39}, {1, 2, 2, 2, 2, 2, 2},
     spell_frost_breath, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(202), 11, 12,
     "blast of frost", "!Frost Breath!", ""},

    {
     "gas breath", {45, 93, 93, 93, 93, 93, 45}, {1, 2, 2, 2, 2, 2, 2},
     spell_gas_breath, song_null, TAR_IGNORE, POS_FIGHTING,
     NULL, SLOT(203), 12, 12,
     "blast of gas", "!Gas Breath!", ""},

    {
     "lightning breath", {37, 93, 93, 93, 93, 93, 37}, {1, 2, 2, 2, 2, 2, 2},
     spell_lightning_breath, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(204), 13, 12,
     "blast of lightning", "!Lightning Breath!", ""},

/*

 * Spells for mega1.are from Glop/Erkenbrand.

 */
    {
     "general purpose", {93, 93, 93, 93, 93, 93, 93}, {0, 0, 0, 0, 0, 0, 0},
     spell_general_purpose, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(401), 0, 12,
     "general purpose ammo", "!General Purpose Ammo!", ""},

    {
     "high explosive", {93, 93, 93, 93, 93, 93, 93}, {0, 0, 0, 0, 0, 0, 0},
     spell_high_explosive, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(402), 0, 12,
     "high explosive ammo", "!High Explosive Ammo!", ""},

    {
     "energizer", {93, 93, 93, 93, 93, 93, 93}, {0, 0, 0, 0, 0, 0, 0},
     spell_energizer, song_null, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
     NULL, SLOT(405), 50, 12,
     "", "!Energizer!", ""},

    {
     "doragu sureibu", {60, 93, 93, 93, 93, 93, 93}, {4, 0, 0, 0, 0, 0, 0},
     spell_draguslave, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(407), 40, 12,
     "dragu slave", "!Dragu Slave!", ""},

    {
     "ra tiruto", {60, 93, 93, 93, 93, 93, 93}, {4, 0, 0, 0, 0, 0, 0},
     spell_ratilt, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     NULL, SLOT(408), 40, 12,
     "ra-tilt", "!Ra-Tilt!", ""},

    {
     "raguna bureedo", {50, 93, 93, 93, 93, 93, 93}, {4, 0, 0, 0, 0, 0, 0},
     spell_laguna_blade, song_null, TAR_IGNORE, POS_STANDING,
     NULL, SLOT(409), 40, 18,
     "", "!Laguna Blade!", ""},

/* Songs */

/*

    {

      "cause anger",                {  1,  60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

      spell_null, song_cause_anger,            TAR_CHAR_SELF,          POS_STANDING,

      NULL,                   SLOT(600),       5,     12,

      "",                     "You lose your temper.",      "" 

    },



    {

	"cause blindness",		{  1,  60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,

	NULL,		SLOT(601),	 5,	12,

	"",			"You can see again.",	""

    },



    {   "cause calm",			{ 1, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_calm,		TAR_IGNORE,		POS_FIGHTING,

	NULL,			SLOT(602),	30,	12,

	"",			"You have lost your peace of mind.",	""

    },



    {

	"cause cancellation", 	{ 1, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_cancellation,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,

	NULL,			SLOT(603),	20,	12,

	""			"!cancellation!",	""

    },



    {

     "cause concentration",         { 60, 60, 60, 60, 60, 60, 60 },    { 1,  1,  2,  2, 1, 1, 1},

     spell_null, song_cause_concentration,     TAR_CHAR_SELF,          POS_STANDING,

     NULL,                    SLOT(605),      20,     12,

     "",                      "You loose your train of thought.",    ""

    },



    {

	"weather control",	{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_weather_control,	TAR_IGNORE,		POS_STANDING,

	NULL,			SLOT(606),	25,	12,

	"",			"!Control Weather!",	""

    },



    {

      "cause cry",                  { 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

      spell_null, song_cause_cry,              TAR_CHAR_OFFENSIVE,     POS_FIGHTING,

      NULL,                   SLOT(607),      15,     12,

      "",                     "You gain control and stop crying", ""

    },



    {

	"evil detection",		{ 60,  60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_evil_detection,	TAR_CHAR_SELF,		POS_STANDING,

	NULL,			SLOT(608),	 5,	12,

	"",			"The red in your vision disappears.",	""

    },



    {

        "good detection",          { 60,  60, 60, 60, 60, 60, 60},     { 1,  1,  2,  2, 1, 1, 1},

        spell_null,  song_good_detection,     TAR_CHAR_SELF,          POS_STANDING,

        NULL,                   SLOT(609),        5,     12,

        "",                     "The gold in your vision disappears.",	""

    },



    {

	"detection hide",	{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_detection_hide,	TAR_CHAR_SELF,		POS_STANDING,

	NULL,			SLOT(610),	 5,	12,

	"",			"You feel less aware of your surroundings.",	

	""

    },



    {

	"detection invis",		{  60,  60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_detection_invis,	TAR_CHAR_SELF,		POS_STANDING,

	NULL,			SLOT(611),	 5,	12,

	"",			"You no longer see invisible objects.",

	""

    },



    {

	"detection magic",		{  60,  60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 9},

	spell_null, song_detection_magic,	TAR_CHAR_SELF,		POS_STANDING,

	NULL,			SLOT(612),	 5,	12,

	"",			"The detect magic wears off.",	""

    },



    {

	"detection poison",	{ 60,  60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_detection_poison,	TAR_OBJ_INV,		POS_STANDING,

	NULL,			SLOT(613),	 5,	12,

	"",			"!Detect Poison!",	""

    },



    {

	"evil dispel",		{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_evil_dispel,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,

	NULL,			SLOT(614),	15,	12,

	"dispel evil",		"!Dispel Evil!",	""

    },



    {

        "good dispel",          { 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

        spell_null, song_good_dispel,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,

        NULL,                   SLOT(615),      15,     12,

        "dispel good",          "!Dispel Good!",	""

    },



    {

	"magic dispel",		{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_magic_dispel,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,

	NULL,			SLOT(616),	15,	12,

	"",			"!Dispel One Source!",	""

    },



    {

	"cause faerie fog",		{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_faerie_fog,	TAR_IGNORE,		POS_STANDING,

	NULL,			SLOT(617),	12,	12,

	"faerie fog",		"!Faerie Fog!",		""

    },



    {

        "cause frenzy",               { 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

        spell_null, song_cause_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,

        NULL,                   SLOT(618),      30,     24,

        "",                     "Your rage ebbs.",	""

    },



    {

	"cause giant strength",	{  60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_giant_strength,	TAR_CHAR_DEFENSIVE,	POS_STANDING,

	NULL,			SLOT(619),	20,	12,

	"",			"You feel weaker.",	""

    },

  

    {

	"cause haste",		{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_haste,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,

	NULL,			SLOT(620),	30,	12,

	"",			"You feel yourself slow down.",	""

    },



    {

	"cause heal",			{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,

	NULL,			SLOT(621),	50,	12,

	"",			"!Heal!",		""

    },



    {

	"cause identify",		{ 60, 60, 60, 60, 60, 60, 60},     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_identify,		TAR_OBJ_INV,		POS_STANDING,

	NULL,			SLOT(622),	12,	24,

	"",			"!Identify!",		""

    },



    {

	"alignment know",	{  60,  60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_alignment_know,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,

	NULL,			SLOT(623),	 9,	12,

	"",			"!Know Alignment!",	""

    },



    {

	"find object",	{  60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_find_object,	TAR_IGNORE,		POS_STANDING,

	NULL,			SLOT(624),	20,	18,

	"",			"!Locate Object!",	""

    },



    {

	"heal mass",		{ 60, 60, 60, 60, 60, 60, 60 },	{ 2,  2,  4,  4, 1, 4, 2},

	spell_null, song_heal_mass,	TAR_IGNORE,		POS_STANDING,

	NULL,			SLOT(625),	100,	36,

	"",			"!Mass Healing!",	""

    },



    {

	"cause mass invis",		{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_mass_invis,	TAR_IGNORE,		POS_STANDING,

	NULL,			SLOT(626),	20,	24,

	"",			"You are no longer invisible.",		""

    },



    {

	"cause refresh",		{  60,  60, 60, 60, 60, 60, 60 },      { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,

	NULL,			SLOT(627),	12,	18,

	"refresh",		"!Refresh!",		""

    },



    {

	"curse remove",		{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_curse_remove,	TAR_OBJ_CHAR_DEF,	POS_STANDING,

	NULL,			SLOT(628),	 5,	12,

	"",			"!Remove Curse!",	""

    },



    {

	"cause sleep",		{ 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 2, 1},

	spell_null, song_cause_sleep,		TAR_CHAR_OFFENSIVE,	POS_STANDING,

	NULL,			SLOT(629),	15,	12,

	"",			"You feel less tired.",	""

    },



    {

        "cause slow",                 { 60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 2, 1},

        spell_null, song_cause_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,

        NULL,                   SLOT(630),      30,     12,

        "",                     "You feel yourself speed up.",	""

    },



    {

	"cause ventriloquate",	{  60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 2, 1},

	spell_null, song_cause_ventriloquate,	TAR_IGNORE,		POS_STANDING,

	NULL,			SLOT(631),	 5,	12,

	"",			"!Ventriloquate!",	""

    },



    {

	"cause weakeness",		{  60, 60, 60, 60, 60, 60, 60 },     { 1,  1,  2,  2, 1, 1, 1},

	spell_null, song_cause_weakeness,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,

	NULL,			SLOT(632),	20,	12,

	"spell",		"You feel stronger.",	""

    },

*/

/* combat and weapons skills */



    {
     "axe", {93, 1, 1, 1, 1, 93, 1}, {0, 6, 5, 4, 5, 0, 1},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_axe, SLOT(0), 0, 0,
     "", "!Axe!", ""},

    {
     "dagger", {1, 1, 1, 1, 1, 1, 1}, {2, 3, 2, 2, 2, 2, 1},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dagger, SLOT(0), 0, 0,
     "", "!Dagger!", ""},

    {
     "energy guns", {93, 93, 34, 21, 93, 93, 21}, {0, 0, 3, 2, 0, 0, 3}, 
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_energygun, SLOT(0), 0, 0,
     "", "!Energy Gun!", ""},

    {
     "flail", {1, 1, 1, 1, 1, 93, 1}, {5, 3, 5, 3, 3, 0, 1},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_flail, SLOT(0), 0, 0,
     "", "!Flail!", ""},

    {
     "gun", {5, 5, 5, 5, 5, 5, 5}, {4, 4, 4, 4, 4, 4, 4},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_gun, SLOT(0), 0, 0,
     "", "!Gun!", ""},

    {
     "heavy guns", {93, 93, 93, 44, 93, 93, 44}, {0, 0, 0, 3, 0, 0, 3},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_heavygun, SLOT(0), 0, 0,
     "", "!Heavy Guns!", ""},

    {
     "mace", {1, 1, 1, 1, 1, 93, 1}, {5, 2, 3, 3, 3, 0, 1},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_mace, SLOT(0), 0, 0,
     "", "!Mace!", ""},

    {
     "pistols", {30, 30, 22, 14, 22, 22, 14}, {3, 3, 2, 2, 2, 2, 2},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_pistol, SLOT(0), 0, 0,
     "", "!Pistols!", ""},

    {
     "polearm", {93, 1, 1, 1, 1, 93, 1}, {0, 6, 6, 4, 4, 0, 1},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_polearm, SLOT(0), 0, 0,
     "", "!Polearm!", ""},

    {
     "shield block", {16, 10, 6, 4, 93, 12, 1}, {6, 5, 5, 4, 0, 5, 4},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_shield_block, SLOT(0), 0, 0,
     "", "!Shield!", ""},

    {
     "counter attack", {93, 93, 93, 20, 93, 93, 1}, {0, 0, 0, 4, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_counter, SLOT(0), 0, 0,
     "counter attack", "!Counter!", ""},

    {
     "phase body", {14, 93, 93, 93, 93, 93, 93}, {3, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_phase, SLOT(0), 0, 0,
     "", "!Phase!", ""},

    {
     "rifles", {93, 93, 34, 21, 34, 34, 21}, {0, 0, 3, 2, 3, 3, 2},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_rifle, SLOT(0), 0, 0,
     "", "!Rifles!", ""},

    {
     "spear", {93, 1, 1, 1, 1, 1, 1}, {0, 4, 4, 3, 3, 2, 1},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_spear, SLOT(0), 0, 0,
     "", "!Spear!", ""},

    {
     "sword", {93, 1, 1, 1, 1, 93, 1}, {0, 4, 3, 2, 2, 0, 1},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_sword, SLOT(0), 0, 0,
     "", "!sword!", ""},

    {
     "whip", {93, 1, 1, 1, 1, 93, 1}, {0, 5, 5, 4, 4, 0, 1},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_whip, SLOT(0), 0, 0,
     "", "!Whip!", ""},

    {
     "backstab", {93, 93, 1, 93, 93, 93, 1}, {0, 0, 5, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_backstab, SLOT(0), 0, 18,
     "backstab", "!Backstab!", ""},

    {
     "bash", {93, 93, 93, 1, 93, 93, 1}, {0, 0, 0, 4, 0, 0, 3},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_bash, SLOT(0), 0, 24,
     "bash", "!Bash!", ""},

    {
     "berserk", {25, 25, 20, 16, 25, 25, 25}, {0, 0, 6, 4, 0, 0, 5},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_berserk, SLOT(0), 0, 24,
     "", "You feel your pulse slow down.", ""},

    {
     "battle rage", {93, 93, 93, 24, 93, 93, 24}, {0, 0, 0, 5, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_battle_rage, SLOT(0), 0, 24,
     "", "Your blood slows as your rage dissipates.", ""},

    {
     "heighten senses", {93, 93, 93, 15, 93, 93, 15}, {0, 0, 0, 3, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_senses, SLOT(0), 0, 12,
     "", "You become less aware of your surroundings.", ""},

    {
     "possess person", {93, 93, 25, 25, 25, 93, 25}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_possess, SLOT(0), 0, 12,
     "", "The evil essence surrenders its possession of your soul.", ""},

    {
     "critical strike", {93, 36, 93, 93, 14, 14, 14}, {0, 5, 0, 0, 5, 5, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_critical_strike, SLOT(0), 0, 0,
     "", "!Critical Strike!", ""},

    {
     "demon rage", {93, 93, 25, 25, 25, 93, 25}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_demonrage, SLOT(0), 0, 12,
     "", "Your demonic rage subsides.", ""},

    {
     "dirt kicking", {93, 93, 12, 7, 93, 12, 4}, {0, 0, 4, 4, 0, 4, 5},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dirt, SLOT(0), 0, 18,
     "kicked dirt", "You rub the dirt out of your eyes.", ""},

    {
     "smoke bomb", {93, 93, 93, 93, 22, 93, 22}, {0, 0, 0, 0, 3, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_smoke, SLOT(0), 0, 24,
     "smoke bomb", "The hazy grey smoke leaves your eyes.", ""},

    {
     "disarm", {93, 93, 24, 22, 28, 30, 10}, {0, 0, 4, 4, 4, 4, 4},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_disarm, SLOT(0), 0, 18,
     "", "!Disarm!", ""},

    /*EE960529 */

    {
     "dual wield", {36, 32, 36, 28, 36, 38, 1}, {0, 0, 5, 4, 6, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_RESTING,
     &gsn_dual_wield, SLOT(0), 0, 24,
     "", "!Dual Wield", ""},

    {
     "dodge", {6, 10, 12, 16, 4, 4, 4}, {6, 6, 4, 5, 5, 4, 6},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dodge, SLOT(0), 0, 0,
     "", "!Dodge!", ""},

    {
     "enhanced damage", {35, 93, 20, 10, 18, 93, 5}, {6, 0, 4, 3, 4, 0, 6},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_enhanced_damage, SLOT(0), 0, 0,
     "", "!Enhanced Damage!", ""},

    {				/*EE960513 */
     "envenom", {93, 93, 20, 93, 20, 93, 20}, {0, 0, 4, 0, 5, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_RESTING,
     &gsn_envenom, SLOT(0), 0, 36,
     "", "!Envenom!", ""},

    {
     "hand to hand", {36, 20, 24, 12, 1, 1, 1}, {6, 5, 5, 4, 4, 4, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_hand_to_hand, SLOT(0), 0, 0,
     "", "!Hand to Hand!", ""},

    {
     "kick", {93, 12, 14, 8, 93, 93, 8}, {0, 4, 5, 3, 0, 0, 4},
     spell_null, song_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
     &gsn_kick, SLOT(0), 0, 12,
     "kick", "!Kick!", ""},

    {
     "parry", {93, 23, 13, 2, 8, 16, 1}, {0, 5, 5, 4, 4, 5, 3},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_parry, SLOT(0), 0, 0,
     "", "!Parry!", ""},

    {
     "rescue", {93, 93, 93, 18, 93, 93, 1}, {0, 0, 0, 4, 0, 0, 3},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_rescue, SLOT(0), 0, 12,
     "", "!Rescue!", ""},

    {
      "t-port", {93, 93, 93, 93, 93, 93, 93}, {0, 0, 0, 0, 0, 0, 0},
      spell_null, song_null, TAR_IGNORE, POS_DEAD,
      &gsn_tport, SLOT(0), 0, 12,
      "", "!T-Port!", ""},

    {
     "trip", {93, 93, 22, 15, 93, 93, 15}, {0, 0, 4, 5, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_trip, SLOT(0), 0, 24,
     "trip", "!Trip!", ""},

    {
     "dragon punch", {93, 93, 93, 93, 93, 24, 24}, {0, 0, 0, 0, 0, 5, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_dragonp, SLOT(0), 0, 0,
     "`2d`@r`&ag`@o`2n `&punch``", "!Dragonp!", ""},

    {
     "hurricane kick", {93, 93, 93, 93, 93, 32, 32}, {0, 0, 0, 0, 0, 5, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_hurricane, SLOT(0), 0, 0,
     "hurricane kick", "!Hurricane!", ""},

    {"footsweep", {93, 93, 93, 93, 14, 13, 13}, {0, 0, 0, 0, 4, 4, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_footsweep, SLOT(0), 0, 0,
     "footsweep", "!Footsweep!",
     },

    {
     "circle", {93, 93, 27, 93, 42, 93, 27}, {0, 0, 5, 0, 6, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_circle, SLOT(0), 0, 0,
     "circle", "!Circle!", ""},

    {
     "roundhouse", {93, 93, 93, 93, 16, 93, 21}, {0, 0, 0, 0, 4, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_roundhouse, SLOT(0), 0, 0,
     "roundhouse", "!Roundhouse", ""},

    {
     "flashkick", {93, 93, 93, 30, 22, 22, 22}, {0, 0, 0, 3, 3, 3, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_flashkick, SLOT(0), 0, 0,
     "flashkick", "!Flashkick!", ""},

    {
     "vanishing shadow", {93, 93, 93, 93, 18, 93, 18}, {0, 0, 0, 0, 3, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_vanishing, SLOT(0), 0, 0,
     "", "You slowly materialize into existance.", ""},

    {
     "optic blast", {1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_optic, SLOT(0), 0, 0,
     "`1o`!p`#t`!i`1c `8b``l`&a``s`8t``", "!Optic!", ""},

    {
     "kamehameha wave",
     {1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_kamehameha, SLOT(0), 0, 0,
     "`3k`#a`7m`&e`8ha`&m`7e`#h`3a `6w`^a`7v`&e`7", "!kamehameha!", ""},

    {
     "feed", {1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_feed, SLOT(0), 0, 0,
     "`1f`!eedin`1g``", "!Feed!", ""},

    {
     "pounce", {1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_pounce, SLOT(0), 0, 0,
     "`&p`7o`8un`7c`&e``", "!Pounce!", ""},

    {
     "throw", {1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_throw, SLOT(0), 0, 0,
     "`6t`^h`&r`^o`6w``", "!Smash!", ""},

    {
     "forge", {1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_DEAD,
     &gsn_forge, SLOT(0), 0, 0,
     "", "", ""},

    {
     "second attack", {18, 15, 12, 5, 10, 14, 5}, {6, 5, 4, 3, 4, 4, 3},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_second_attack, SLOT(0), 0, 0,
     "", "!Second Attack!", ""},

    {
     "third attack", {38, 38, 28, 21, 29, 31, 12}, {0, 5, 5, 4, 5, 5, 6},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_third_attack, SLOT(0), 0, 0,
     "", "!Third Attack!", ""},

    {
     "fourth attack", {93, 93, 93, 41, 57, 57, 41}, {0, 0, 0, 5, 7, 6, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_fourth_attack, SLOT(0), 0, 0,
     "", "!Fourth Attack!", ""},

    {
     "fifth attack", {93, 93, 93, 54, 93, 70, 54}, {0, 0, 0, 6, 0, 7, 0,},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_fifth_attack, SLOT(0), 0, 0,
     "", "!Fifth Attack!", ""},

    {
     "sixth attack", {93, 93, 93, 68, 93, 93, 68}, {0, 0, 0, 6, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_sixth_attack, SLOT(0), 0, 0,
     "", "!Sixth Attack!",
     },

/* non-combat skills */

    {
     "fast healing", {15, 9, 16, 6, 5, 10, 8}, {6, 5, 5, 3, 4, 4, 5},
     spell_null, song_null, TAR_IGNORE, POS_SLEEPING,
     &gsn_fast_healing, SLOT(0), 0, 0,
     "", "!Fast Healing!", ""},

    {
     "regeneration", {1, 1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_SLEEPING,
     &gsn_regeneration, SLOT(0), 0, 0,
     "", "!Regeneration!", ""},

    {
     "haggle", {93, 93, 23, 93, 93, 93, 23}, {0, 0, 3, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_RESTING,
     &gsn_haggle, SLOT(0), 0, 0,
     "", "!Haggle!", ""},

    {
     "healing touch", {93, 93, 93, 93, 93, 36, 36}, {0, 0, 0, 0, 0, 4, 0},
     spell_null, song_null, TAR_IGNORE, POS_RESTING,
     &gsn_touch_healing, SLOT(0), 0, 12,
     "", "!Healing touch!", ""},

    {
     "hide", {93, 93, 4, 12, 93, 7, 4}, {0, 0, 4, 5, 0, 4, 0},
     spell_null, song_null, TAR_IGNORE, POS_RESTING,
     &gsn_hide, SLOT(0), 0, 12,
     "", "!Hide!", ""},

    {
     "lore", {93, 93, 18, 93, 93, 93, 18}, {0, 0, 2, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_RESTING,
     &gsn_lore, SLOT(0), 0, 36,
     "", "!Lore!", ""},

    {
     "meditation", {6, 6, 93, 93, 10, 4, 4}, {4, 4, 0, 0, 5, 2, 0},
     spell_null, song_null, TAR_IGNORE, POS_SLEEPING,
     &gsn_meditation, SLOT(0), 0, 0,
     "", "Meditation", ""},

    {
     "body recovery", {93, 93, 93, 93, 93, 4, 4}, {0, 0, 0, 0, 0, 2, 0},
     spell_null, song_null, TAR_IGNORE, POS_SLEEPING,
     &gsn_recovery, SLOT(0), 0, 0,
     "", "recovery", ""},

    {
     "peek", {93, 93, 18, 93, 93, 93, 18}, {0, 0, 3, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_peek, SLOT(0), 0, 0,
     "", "!Peek!", ""},

    {
     "pick lock", {93, 93, 7, 93, 93, 93, 7}, {0, 0, 3, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_pick_lock, SLOT(0), 0, 12,
     "", "!Pick!", ""},

    {
     "rapid strike", {93, 93, 93, 40, 93, 93, 40}, {0, 0, 0, 4, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_rapid_strike, SLOT(0), 0, 0,
     "`3r`#a`3pid `1str`!i`1ke``", "!Rapid Strike!", ""},

    {
     "sneak", {93, 93, 4, 10, 15, 7, 4}, {0, 0, 4, 6, 5, 4, 6},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_sneak, SLOT(0), 0, 12,
     "", "You no longer feel stealthy.", ""},

    {
     "steal", {93, 93, 5, 93, 93, 93, 5}, {0, 0, 4, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_steal, SLOT(0), 0, 24,
     "", "!Steal!", ""},

    {
     "scrolls", {1, 1, 1, 1, 1, 1, 1}, {2, 3, 5, 7, 4, 5, 3},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_scrolls, SLOT(0), 0, 24,
     "", "!Scrolls!", ""},

    {
     "staves", {1, 1, 1, 1, 1, 1, 1}, {2, 3, 5, 6, 4, 5, 8},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_staves, SLOT(0), 0, 12,
     "", "!Staves!", ""},

    {
     "wands", {1, 1, 1, 1, 1, 1, 1}, {2, 3, 5, 6, 3, 5, 8},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_wands, SLOT(0), 0, 12,
     "", "!Wands!", ""},

    {
     "recall", {1, 1, 1, 1, 1, 1, 1}, {2, 2, 2, 2, 2, 2, 2},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_recall, SLOT(0), 0, 12,
     "", "!Recall!", ""},

    /*EE960529 */
    {
     "brew", {30, 43, 93, 93, 93, 93, 30}, {5, 5, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_brew, 0, 0, 24,
     "", "!Brew!", ""},
    /*EE960530 */
    {
     "scribe", {28, 28, 93, 93, 93, 93, 28}, {6, 6, 0, 0, 0, 0, 0},
     spell_null, song_null, TAR_IGNORE, POS_STANDING,
     &gsn_scribe, 0, 0, 24,
     "", "!Scribe!", ""}

};


const struct group_type group_table[MAX_GROUP] = {



    /*EE960411 Modified to fit class_table */

    {
     "rom basics", {0, 0, 0, 0, 0, 0, 0},
     {"scrolls", "staves", "wands", "recall"}
     },


    {
     "mage basics", {0, -1, -1, -1, -1, -1, -1},
     {"dagger", "brew"}
     },


    {
     "cleric basics", {-1, 0, -1, -1, -1, -1, -1},
     {"mace", "healing"}
     },


    {
     "thief basics", {-1, -1, 0, -1, -1, -1, -1},
     {"dagger", "steal", "backstab", "sneak"}
     },


    {
     "warrior basics", {-1, -1, -1, 0, -1, -1, -1},
     {"sword", "second attack", "third attack"}
     },


    {
     "ninja basics", {-1, -1, -1, -1, 0, -1, -1},
     {"sword", "hand to hand", "roundhouse"}
     },


    {
     "monk basics", {-1, -1, -1, -1, -1, 0, -1},
     {"dagger", "hand to hand", "second attack"}
     },


    {
     "knight basics", {-1, -1, -1, -1, -1, -1, 0},
     {"spear", "sword", "whip"}
     },


    {
     "mage default", {40, -1, -1, -1, -1, -1, -1},
     {"beguiling", "combat", "detection", "emotions", "enhancement",
      "illusion", "maladictions", "protective", "transportation",
      "enchantment"}
     },


    {
     "cleric default", {-1, 40, -1, -1, -1, -1, -1},
     {"flail", "attack", "creation", "curative", "benedictions",
      "detection", "healing", "maladictions", "protective",
      "shield block", "transportation", "weather"}
     },


    {
     "thief default", {-1, -1, 40, -1, -1, -1, -1},
     {"sword", "disarm", "dodge", "second attack",
      "trip", "hide", "peek", "pick lock", "sneak"}
     },


    {
     "warrior default", {-1, -1, -1, 40, -1, -1, -1},
     {"shield block", "bash", "disarm", "enhanced damage", "parry",
      "dual wield", "third attack", "berserk", "fast healing",
      "dirt kicking"}
     },


    {
     "ninja default", {-1, -1, -1, -1, 40, -1, -1},
     {"dagger", "dirt kicking", "illusion", "dodge", "trip",
      "fast healing", "hide", "sneak"}
     },


    {
     "monk default", {-1, -1, -1, -1, -1, 40, -1},
     {"spear", "bash", "dodge", "second attack",
      "footsweep", "hide", "peek", "sneak"}
     },


    {
     "knight default", {-1, -1, -1, -1, -1, -1, 40},
     {"shield block", "bash", "dirt kicking", "enhanced damage",
      "dodge", "second attack", "parry", "rescue", "third attack",
      "fast healing"}
     },

    /*EE960417 Modified to fit the new class_table */

    {
     "weaponsmaster", {-1, 40, 40, 20, 25, -1, 0},
     {"axe", "dagger", "flail", "mace", "polearm", "spear", "sword", "whip"}
     },


    {
     "attack", {-1, 5, -1, -1, -1, 6, -1},
     {"demonfire", "dispel evil", "dispel good", "earthquake",
      "flamestrike", "heat metal", "ray of truth"}
     },


    {
     "beguiling", {3, -1, 5, -1, -1, -1, -1},
     {"calm", "charm person", "sleep"}
     },


    {
     "benedictions", {-1, 5, -1, -1, -1, 5, -1},
     {"bless", "calm", "frenzy", "holy word", "remove curse"}
     },


    {
     "cantrips", {-1, -1, 3, 3, 3, -1, -1},
     {"light", "ray of frost", "resistance"}
     },


    {
     "combat", {5, -1, 6, 7, 6, -1, -1},
     {"acid blast", "burning hands", "chain lightning", "chill touch",
      "colour spray", "fireball", "lightning bolt", "magic missile",
      "ray of frost", "shocking grasp"}
     },


    {
     "creation", {4, 4, -1, -1, -1, -1, -1},
     {"continual light", "create food", "create spring", "create water",
      "create rose", "floating disc"}
     },


    {
     "curative", {-1, 3, -1, -1, -1, 5, -1},
     {"cure blindness", "cure disease", "cure poison"}
     },


    {
     "detection", {4, 4, 5, -1, 6, 6, -1},
     {"detect evil", "detect good", "detect hidden", "detect invis",
      "detect magic", "detect poison", "true sight", "farsight",
      "identify", "know alignment", "locate object", "locate person"}
     },


    {
     "draconian", {5, -1, -1, -1, -1, -1, -1},
     {"acid breath", "fire breath", "frost breath", "gas breath",
      "lightning breath"}
     },


    {
     "slayers spells", {5, -1, -1, -1, -1, -1, -1},
     {"raguna bureedo", "doragu sureibu", "ra tiruto"}
     },


    {
     "emotions", {5, 5, 5, 5, 5, 5, -1},
     {"anger", "concentration", "cry", "psionic blast", "fear"}
     },


    {
     "enchantment", {5, -1, -1, -1, -1, -1, -1},
     {"enchant armor", "enchant weapon", "fireproof", "recharge"}
     },


    {
     "enhancement", {4, -1, -1, 5, 6, -1, -1},
     {"giant strength", "haste", "infravision", "refresh"}
     },


    {
     "gravity arcana", {6, 6, 3, -1, -1, -1, -1},
     {"heavy blow", "demi", "freefall", "gravija", "black hole"}
     },


    {
     "harmful", {-1, 4, -1, -1, -1, -1, -1},
     {"cause critical", "cause light", "cause serious", "harm"}
     },


    {
     "healing", {-1, 5, -1, -1, -1, -1, -1},
     {"cure critical", "cure light", "cure serious", "heal",
      "mass healing", "refresh"}
     },


    {
     "hellbender", {-1, -1, -1, -1, -1, -1, -1},
     {"hellbender"}
     },


    {
     "illusion", {3, -1, 5, -1, 5, -1, -1},
     {"invis", "mass invis", "ventriloquate"}
     },


    {
     "maladictions", {5, 5, 6, 7, 7, 6, -1},
     {"blindness", "change sex", "curse", "energy drain", "plague",
      "poison", "slow", "weaken"}
     },


    {
     "protective", {5, 5, 7, 7, 7, 6, -1},
     {"armor", "cancellation", "dispel magic", "fireproof",
      "protection evil", "protection good", "resistance",
      "sanctuary", "shield", "stone skin"}
     },


    {
     "transportation", {4, 4, 4, 4, 4, 4, 4},
     {"fly", "gate", "nexus", "pass door", "portal", "summon", "teleport",
      "word of recall"}
     },


    {
     "weather", {-1, 5, -1, -1, -1, -1, -1},
     {"call lightning", "control weather", "faerie fire", "faerie fog",
      "lightning bolt"}
     }
};

/* TC961118 Donation table X Y

    X = race 

    Y = room                           */

unsigned int tab_donate[NUMBER_RACES][DON_LAST_ROOM] = {

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Race NULL  */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Human      */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Elf        */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Dwarf      */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Giant      */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Dragonkin  */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Vampire    */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Dark Elf   */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Cyborg     */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Esper      */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Demon      */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379},	/* Saiyan     */

    {3373, 3374, 3375, 3376, 3377, 3378, 3379}	/* Feline     */

};





//struct struct_channel

//{

//      char *name;         /* Name of channel used for do_channel and in channel */

//      char color;         /* signe char defining the color of the text */

//      char *text;         /* text chown. ex: You Gossip ... An s is added */

//      bool output_name;   /* Does nothing for the moment */

//      int  flag;          /* COMM_NOxxx flag */

//}; /* struct struct_channel */



/*struct struct_channel tab_channel[] =
{ 
{ "Gossip   ", '3', "gossip",     TRUE, COMM_NOGOSSIP  },
{ "Flame    ", '&', "`1f`!l`#a`!m`1e`&", TRUE, COMM_NOFLAME   },
{ "Auction  ", '7', "`Va`8u`Vcti`8o`Vn `8:``",    TRUE, COMM_NOAUCTION },
{ "Question ", '#', "a`3s`#k",        TRUE, COMM_NOQUESTION},
{ "Music    ", '6', "`^mus`6i`^c``", TRUE, COMM_NOMUSIC},
{ "Quote    ", '2', "quote",      TRUE, COMM_NOQUOTE   },
{ "Grats    ", '5', "congratulate", TRUE, COMM_NOGRATS },
{ "OOC      ", '#', "O`&O``C",    TRUE, COMM_NOOOC     },
{ "Answer   ", '@', "answer",     TRUE, COMM_NOQUESTION},
};*//* tab_channel */

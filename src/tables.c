
/*

	This .h or .c file is part of a Rom 2.4b2 code written by
Russel Taylor.  It has been further enhanced and edited by Mical
and Kyler.  MOST bugs are removed from this release, and color has
been added.  It also has several new features, and many changes
from the original code.  There are no back doors, and few bugs
left in the code.

	Kyler and I ask that if you use this code base, you add
that this code is greatly enhanced from Rom 2.4b2, along with
Russel Taylor's name.

Mical/Kyler

*/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "tables.h"

/* for position */
const struct position_type position_table[] = {
    {"dead", "dead"},
    {"mortally wounded", "mort"},
    {"incapacitated", "incap"},
    {"stunned", "stun"},
    {"sleeping", "sleep"},
    {"resting", "rest"},
    {"sitting", "sit"},
    {"fighting", "fight"},
    {"standing", "stand"},
    {NULL, NULL}
};

/* for sex */
const struct sex_type sex_table[] = {
    {"none"},
    {"male"},
    {"female"},
    {"either"},
    {NULL}
};

/* for sizes */
const struct size_type size_table[] = {
    {"tiny"},
    {"small"},
    {"medium"},
    {"large"},
    {"huge",},
    {"giant"},
    {NULL}
};

/* various flag tables */
const struct flag_type act_flags[] = {
    {"npc", A, FALSE, 99},
    {"sentinel", B, TRUE, 99},
    {"scavenger", C, TRUE, 99},
    {"banker", D, TRUE, 99},
    {"aggressive", F, TRUE, 99},
    {"stay_area", G, TRUE, 99},
    {"wimpy", H, TRUE, 99},
    {"pet", I, TRUE, 99},
    {"train", J, TRUE, 99},
    {"practice", K, TRUE, 99},
    {"horse", L, TRUE, 99},
    {"quest", Z, TRUE, 99},
    {"undead", O, TRUE, 99},
    {"monk", P, TRUE, 99},
    {"cleric", Q, TRUE, 99},
    {"mage", R, TRUE, 99},
    {"thief", S, TRUE, 99},
    {"warrior", T, TRUE, 99},
    {"noalign", U, TRUE, 99},
    {"nopurge", V, TRUE, 99},
    {"outdoors", W, TRUE, 99},
    {"indoors", Y, TRUE, 99},
    {"healer", aa, TRUE, 99},
    {"gain", bb, TRUE, 99},
    {"update_always", cc, TRUE, 99},
    {"changer", dd, TRUE, 99},
    {NULL, 0, FALSE, 0}
};

const struct flag_type plr_flags[] = {
    {"npc", A, FALSE, 99},
    {"autoassist", C, TRUE, 99},
    {"autoexit", D, TRUE, 99},
    {"autoloot", E, TRUE, 99},
    {"autosac", F, TRUE, 99},
    {"autogold", G, TRUE, 99},
    {"autosplit", H, TRUE, 99},
    {"dead", L, TRUE, 99},
    {"holylight", N, TRUE, 99},
    {"can_loot", P, TRUE, 99},
    {"nosummon", Q, TRUE, 99},
    {"nofollow", R, TRUE, 99},
    {"leader", T, TRUE, 99},
    {"permit", U, TRUE, 99},
    {"log", W, TRUE, 99},
    {"deny", X, TRUE, 99},
    {"freeze", Y, TRUE, 100},
    {"thief", Z, TRUE, 99},
    {"killer", aa, TRUE, 99},
    {"autodamage", bb, TRUE, 99},
    {"rsay", cc, TRUE, 99},
    {NULL, 0, FALSE, 0}
};

const struct flag_type affect_flags[] = {
    {"blind", A, TRUE, 97},
    {"invisible", B, TRUE, 97},
    {"detect_evil", C, TRUE, 97},
    {"detect_invis", D, TRUE, 97},
    {"detect_magic", E, TRUE, 97},
    {"detect_hidden", F, TRUE, 97},
    {"detect_good", G, TRUE, 97},
    {"sanctuary", H, TRUE, 97},
    {"faerie_fire", I, TRUE, 97},
    {"infrared", J, TRUE, 97},
    {"curse", K, TRUE, 97},
    {"poison", M, TRUE, 97},
    {"protect_evil", N, TRUE, 97},
    {"protect_good", O, TRUE, 97},
    {"sneak", P, TRUE, 97},
    {"hide", Q, TRUE, 97},
    {"sleep", R, TRUE, 97},
    {"charm", S, TRUE, 97},
    {"flying", T, TRUE, 97},
    {"pass_door", U, TRUE, 97},
    {"haste", V, TRUE, 97},
    {"calm", W, TRUE, 97},
    {"plague", X, TRUE, 97},
    {"weaken", Y, TRUE, 97},
    {"dark_vision", Z, TRUE, 97},
    {"berserk", aa, TRUE, 97},
    {"swim", bb, TRUE, 97},
    {"regeneration", cc, TRUE, 97},
    {"slow", dd, TRUE, 97},
    {NULL, 0, FALSE, 0}
};

const struct flag_type off_flags[] = {
    {"area_attack", A, TRUE, 99},
    {"backstab", B, TRUE, 99},
    {"bash", C, TRUE, 99},
    {"berserk", D, TRUE, 99},
    {"disarm", E, TRUE, 99},
    {"dodge", F, TRUE, 99},
    {"fade", G, TRUE, 99},
    {"fast", H, TRUE, 99},
    {"kick", I, TRUE, 99},
    {"dirt_kick", J, TRUE, 99},
    {"parry", K, TRUE, 99},
    {"rescue", L, TRUE, 99},
    {"tail", M, TRUE, 99},
    {"trip", N, TRUE, 99},
    {"crush", O, TRUE, 99},
    {"assist_all", P, TRUE, 99},
    {"assist_align", Q, TRUE, 99},
    {"assist_race", R, TRUE, 99},
    {"assist_players", S, TRUE, 99},
    {"assist_guard", T, TRUE, 99},
    {"assist_vnum", U, TRUE, 99},
    {NULL, 0, FALSE, 0}
};

const struct flag_type imm_flags[] = {
    {"summon", A, TRUE, 97},
    {"charm", B, TRUE, 97},
    {"magic", C, TRUE, 97},
    {"weapon", D, TRUE, 97},
    {"bash", E, TRUE, 97},
    {"pierce", F, TRUE, 97},
    {"slash", G, TRUE, 97},
    {"fire", H, TRUE, 97},
    {"cold", I, TRUE, 97},
    {"lightning", J, TRUE, 97},
    {"acid", K, TRUE, 97},
    {"poison", L, TRUE, 97},
    {"negative", M, TRUE, 97},
    {"holy", N, TRUE, 97},
    {"energy", O, TRUE, 97},
    {"mental", P, TRUE, 97},
    {"disease", Q, TRUE, 97},
    {"drowning", R, TRUE, 97},
    {"light", S, TRUE, 97},
    {"sound", T, TRUE, 97},
    {"gravity", U, TRUE, 97},
    {"wood", X, TRUE, 97},
    {"silver", Y, TRUE, 97},
    {"iron", Z, TRUE, 97},
    {"cuendillar", aa, TRUE, 97},
    {NULL, 0, FALSE, 0}
};

const struct flag_type form_flags[] = {
    {"edible", FORM_EDIBLE, TRUE, 99},
    {"poison", FORM_POISON, TRUE, 99},
    {"magical", FORM_MAGICAL, TRUE, 99},
    {"instant_decay", FORM_INSTANT_DECAY, TRUE, 99},
    {"other", FORM_OTHER, TRUE, 99},
    {"animal", FORM_ANIMAL, TRUE, 99},
    {"sentient", FORM_SENTIENT, TRUE, 99},
    {"undead", FORM_UNDEAD, TRUE, 99},
    {"construct", FORM_CONSTRUCT, TRUE, 99},
    {"mist", FORM_MIST, TRUE, 99},
    {"intangible", FORM_INTANGIBLE, TRUE, 99},
    {"biped", FORM_BIPED, TRUE, 99},
    {"centaur", FORM_CENTAUR, TRUE, 99},
    {"insect", FORM_INSECT, TRUE, 99},
    {"spider", FORM_SPIDER, TRUE, 99},
    {"crustacean", FORM_CRUSTACEAN, TRUE, 99},
    {"worm", FORM_WORM, TRUE, 99},
    {"blob", FORM_BLOB, TRUE, 99},
    {"mammal", FORM_MAMMAL, TRUE, 99},
    {"bird", FORM_BIRD, TRUE, 99},
    {"reptile", FORM_REPTILE, TRUE, 99},
    {"snake", FORM_SNAKE, TRUE, 99},
    {"dragon", FORM_DRAGON, TRUE, 99},
    {"amphibian", FORM_AMPHIBIAN, TRUE, 99},
    {"fish", FORM_FISH, TRUE, 99},
    {"cold_blood", FORM_COLD_BLOOD, TRUE, 99},
    {NULL, 0, FALSE, 0}
};

const struct flag_type part_flags[] = {
    {"head", PART_HEAD, TRUE, 99},
    {"arms", PART_ARMS, TRUE, 99},
    {"legs", PART_LEGS, TRUE, 99},
    {"heart", PART_HEART, TRUE, 99},
    {"brains", PART_BRAINS, TRUE, 99},
    {"guts", PART_GUTS, TRUE, 99},
    {"hands", PART_HANDS, TRUE, 99},
    {"feet", PART_FEET, TRUE, 99},
    {"fingers", PART_FINGERS, TRUE, 99},
    {"ear", PART_EAR, TRUE, 99},
    {"eye", PART_EYE, TRUE, 99},
    {"long_tongue", PART_LONG_TONGUE, TRUE, 99},
    {"eyestalks", PART_EYESTALKS, TRUE, 99},
    {"tentacles", PART_TENTACLES, TRUE, 99},
    {"fins", PART_FINS, TRUE, 99},
    {"wings", PART_WINGS, TRUE, 99},
    {"tail", PART_TAIL, TRUE, 99},
    {"claws", PART_CLAWS, TRUE, 99},
    {"fangs", PART_FANGS, TRUE, 99},
    {"horns", PART_HORNS, TRUE, 99},
    {"scales", PART_SCALES, TRUE, 99},
    {"tusks", PART_TUSKS, TRUE, 99},
    {"gills", PART_GILLS, TRUE, 99},
    {NULL, 0, FALSE, 0}
};

const struct flag_type comm_flags[] = {
    {"quiet", COMM_QUIET, TRUE},
    {"deaf", COMM_DEAF, TRUE},
    {"nowiz", COMM_NOWIZ, TRUE},
    {"noclangossip", COMM_NOAUCTION, TRUE},
    {"nogossip", COMM_NOGOSSIP, TRUE},
    {"noquestion", COMM_NOQUESTION, TRUE},
    {"nomusic", COMM_NOMUSIC, TRUE},
    {"noclan", COMM_NOCLAN, TRUE},
    {"noquote", COMM_NOQUOTE, TRUE},
    {"noooc", COMM_NOOOC, TRUE},
    {"shoutsoff", COMM_SHOUTSOFF, TRUE},
    {"true_trust", COMM_TRUE_TRUST, TRUE},
    {"compact", COMM_COMPACT, TRUE},
    {"brief", COMM_BRIEF, TRUE},
    {"prompt", COMM_PROMPT, TRUE},
    {"combine", COMM_COMBINE, TRUE},
    {"telnet_ga", COMM_TELNET_GA, TRUE},
    {"show_affects", COMM_SHOW_AFFECTS, TRUE},
    {"nograts", COMM_NOGRATS, TRUE},
    {"noemote", COMM_NOEMOTE, FALSE},
    {"noshout", COMM_NOSHOUT, FALSE},
    {"notell", COMM_NOTELL, FALSE},
    {"nochannels", COMM_NOCHANNELS, FALSE},
    {"snoop_proof", COMM_SNOOP_PROOF, FALSE},
    {"afk", COMM_AFK, TRUE},
    {NULL, 0, FALSE}
};

/*
const struct flag_type wear_flags[] = {
    {"take", A, TRUE, 97},
    {"finger", B, TRUE, 97},
    {"neck", C, TRUE, 97},
    {"body", D, TRUE, 97},
    {"head", E, TRUE, 97},
    {"legs", F, TRUE, 97},
    {"feet", G, TRUE, 97},
    {"hands", H, TRUE, 97},
    {"arms", I, TRUE, 97},
    {"shield", J, TRUE, 97},
    {"about", K, TRUE, 97},
    {"waist", L, TRUE, 97},
    {"wrist", M, TRUE, 97},
    {"wield", N, TRUE, 97},
    {"hold", O, TRUE, 97},
    {"nosac", P, TRUE, 97},
    {"float", Q, TRUE, 97},
    {NULL, 0, FALSE, 0}
    }; */

/*
const struct flag_type extra_flags[] = {
    {"glow", A, TRUE, 97},
    {"hum", B, TRUE, 97},
    {"dark", C, TRUE, 97},
    {"lock", D, TRUE, 97},
    {"evil", E, TRUE, 97},
    {"invis", F, TRUE, 97},
    {"magic", G, TRUE, 97},
    {"nodrop", H, TRUE, 97},
    {"bless", I, TRUE, 97},
    {"anti-good", J, TRUE, 97},
    {"anti-evil", K, TRUE, 97},
    {"anti-neutral", L, TRUE, 97},
    {"noremove", M, TRUE, 97},
    {"inventory", N, TRUE, 97},
    {"nopurge", O, TRUE, 97},
    {"rotdeath", P, TRUE, 97},
    {"visdeath", Q, TRUE, 97},
    {"nosac", R, TRUE, 97},
    {"nonmetal", S, TRUE, 97},
    {"nolocate", T, TRUE, 97},
    {"meltdrop", U, TRUE, 97},
    {"timer", V, TRUE, 97},
    {"sell", W, TRUE, 97},
    {"gunburst", X, TRUE, 97},
    {"burnproof", Y, TRUE, 97},
    {"nouncurse", Z, TRUE, 97},
    {"clan_eq", aa, FALSE, 99},
    {"autoquest_eq", bb, FALSE, 97},
    {"gunauto", ee, TRUE, 97},
    {NULL, 0, FALSE, 0}
    }; */

const struct flag_type weapon_flags[] = {
    {"exotic", WEAPON_EXOTIC, TRUE, 97},
    {"sword", WEAPON_SWORD, TRUE, 97},
    {"dagger", WEAPON_DAGGER, TRUE, 97},
    {"spear", WEAPON_SPEAR, TRUE, 97},
    {"mace", WEAPON_MACE, TRUE, 97},
    {"axe", WEAPON_AXE, TRUE, 97},
    {"flail", WEAPON_FLAIL, TRUE, 97},
    {"whip", WEAPON_WHIP, TRUE, 97},
    {"pole", WEAPON_POLEARM, TRUE, 97},
    {NULL, 0, FALSE, 0}
};

/* FOR OLC const struct flag_type room_flags[] = {
    {"dark", A, TRUE, 99},
    {"no_mob", C, TRUE, 99},
    {"indoors", D, TRUE, 99},
    {"private", J, TRUE, 99},
    {"safe", K, TRUE, 99},
    {"solitary", L, TRUE, 99},
    {"pet_shop", M, TRUE, 99},
    {"no_recall", N, TRUE, 99},
    {"imp_only", O, TRUE, 99},
    {"gods_only", P, TRUE, 99},
    {"heroes_only", Q, TRUE, 99},
    {"newbies_only", R, TRUE, 95},
    {"law", S, TRUE, 99},
    {"nowhere", T, TRUE, 99},
    {"wizard_only", U, TRUE, 100},
    {NULL, 0, FALSE, 0}
    }; */

const struct race_talk_type race_talk_table[MAX_PC_RACE] = {
    {NULL},
    {"in a normal tongue"},	/* Human */
    {"with an elvish accent"},	/* High-Elf */
    {"with a dwarven-like voice"},	/* Dwarf */
    {"with a booming voice"},	/* Giant */
    {"in a fiery deadly tone"},	/* Dragonkin */
    {"in a soothing voice"},	/* Vampire */
    {"with darkened filled eyes"},	/* Darkelf */
    {"in a cybernetic tone"},	/* Cyborg */
    {"with an enchanting voice"},	/* Esper */
    {"in a darkened tone"},	/* Demon */
    {"with a large grunt"},	/* Saiyan */
    {"with an odd accent"}  /* Feline */
};

const struct struct_channel tab_channel[] = {
    {"Gossip   ", '3', "gossip", TRUE, COMM_NOGOSSIP},
    {"Flame    ", '&', "`1f`!l`#a`!m`1e`&", TRUE, COMM_NOFLAME},
    {"Auction  ", '8', "`Va`8u`Vcti`8o`Vn", TRUE, COMM_NOAUCTION},
    {"Question ", '#', "a`3s`#k", TRUE, COMM_NOQUESTION},
    {"Music    ", '6', "`^mus`6i`^c``", TRUE, COMM_NOMUSIC},
    {"Quote    ", '2', "quote", TRUE, COMM_NOQUOTE},
    {"Grats    ", '5', "congratulate", TRUE, COMM_NOGRATS},
    {"OOC      ", '#', "O`&O``C", TRUE, COMM_NOOOC},
    {"Answer   ", '@', "answer", TRUE, COMM_NOQUESTION}
};				/* tab_channel */

const struct flag_type mprog_flags[] =
{
  { "act",                  TRIG_ACT,               TRUE },
  { "bribe",                TRIG_BRIBE,             TRUE },
  { "death",                TRIG_DEATH,             TRUE },
  { "entry",                TRIG_ENTRY,             TRUE },
  { "fight",                TRIG_FIGHT,             TRUE },
  { "give",                 TRIG_GIVE,              TRUE },
  { "greet",                TRIG_GREET,             TRUE },
  { "grall",                TRIG_GRALL,             TRUE },
  { "kill",                 TRIG_KILL,              TRUE },
  { "hpcnt",                TRIG_HPCNT,             TRUE },
  { "random",               TRIG_RANDOM,            TRUE },
  { "speech",               TRIG_SPEECH,            TRUE },
  { "exit",                 TRIG_EXIT,              TRUE },
  { "exall",                TRIG_EXALL,             TRUE },
  { "delay",                TRIG_DELAY,             TRUE },
  { "surr",                 TRIG_SURR,              TRUE },
  { NULL,                   0,                      TRUE }
};

const struct flag_type area_flags[] =
{
  { "none",                 AREA_NONE,              FALSE },
  { "changed",              AREA_CHANGED,           TRUE },
  { "added",                AREA_ADDED,             TRUE },
  { "loading",              AREA_LOADING,           FALSE },
  { NULL,                   0,                      0 }
};

const struct flag_type sex_flags[] =
{
  { "male",                 SEX_MALE,               TRUE },
  { "female",               SEX_FEMALE,             TRUE },
  { "neutral",              SEX_NEUTRAL,            TRUE },
  {   "random",               3,                      TRUE },   /* ROM */
  { "none",                 SEX_NEUTRAL,            TRUE },
  { NULL,                   0,                      0 }
};

const struct flag_type exit_flags[] =
{
  {   "door",                       EX_ISDOOR,              TRUE },
  { "closed",               EX_CLOSED,              TRUE },
  { "locked",               EX_LOCKED,              TRUE },
  { "pickproof",            EX_PICKPROOF,           TRUE },
  {   "nopass",             EX_NOPASS,              TRUE },
  {   "easy",                       EX_EASY,                TRUE },
  {   "hard",                       EX_HARD,                TRUE },
  { "infuriating",          EX_INFURIATING,         TRUE },
  { "noclose",              EX_NOCLOSE,             TRUE },
  { "nolock",               EX_NOLOCK,              TRUE },
  { NULL,                   0,                      0 }
};

const struct flag_type door_resets[] =
{
  { "open and unlocked",    0,              TRUE },
  { "closed and unlocked",  1,              TRUE },
  { "closed and locked",    2,              TRUE },
  { NULL,                   0,              0 }
};

const struct flag_type room_flags[] =
{
  { "dark",                 ROOM_DARK,              TRUE },
  { "no_mob",               ROOM_NO_MOB,            TRUE },
  { "indoors",              ROOM_INDOORS,           TRUE },
  { "private",              ROOM_PRIVATE,           TRUE },
  { "safe",                 ROOM_SAFE,              TRUE },
  { "solitary",             ROOM_SOLITARY,          TRUE },
  { "pet_shop",             ROOM_PET_SHOP,          TRUE },
  { "no_recall",            ROOM_NO_RECALL,         TRUE },
  { "imp_only",             ROOM_IMP_ONLY,          TRUE },
  { "gods_only",            ROOM_GODS_ONLY,         TRUE },
  { "heroes_only",          ROOM_HEROES_ONLY,       TRUE },
  { "newbies_only",         ROOM_NEWBIES_ONLY,      TRUE },
  { "law",                  ROOM_LAW,               TRUE },
  {   "nowhere",            ROOM_NOWHERE,           TRUE },
  { NULL,                   0,                      0 }
};

const struct flag_type sector_flags[] =
{
  { "inside",       SECT_INSIDE,            TRUE },
  { "city",         SECT_CITY,              TRUE },
  { "field",        SECT_FIELD,             TRUE },
  { "forest",       SECT_FOREST,            TRUE },
  { "hills",        SECT_HILLS,             TRUE },
  { "mountain",     SECT_MOUNTAIN,          TRUE },
  { "swim",         SECT_WATER_SWIM,        TRUE },
  { "noswim",       SECT_WATER_NOSWIM,      TRUE },
  {   "unused",     SECT_UNUSED,            TRUE },
  { "air",          SECT_AIR,               TRUE },
  { "desert",       SECT_DESERT,            TRUE },
  { NULL,           0,                      0 }
};

const struct flag_type type_flags[] =
{
  { "light",                ITEM_LIGHT,             TRUE },
  { "scroll",               ITEM_SCROLL,            TRUE },
  { "wand",                 ITEM_WAND,              TRUE },
  { "staff",                ITEM_STAFF,             TRUE },
  { "weapon",               ITEM_WEAPON,            TRUE },
  { "treasure",             ITEM_TREASURE,          TRUE },
  { "armor",                ITEM_ARMOR,             TRUE },
  { "potion",               ITEM_POTION,            TRUE },
  { "furniture",            ITEM_FURNITURE,         TRUE },
  { "trash",                ITEM_TRASH,             TRUE },
  { "container",            ITEM_CONTAINER,         TRUE },
  { "drinkcontainer",       ITEM_DRINK_CON,         TRUE },
  { "key",                  ITEM_KEY,               TRUE },
  { "food",                 ITEM_FOOD,              TRUE },
  { "money",                ITEM_MONEY,             TRUE },
  { "boat",                 ITEM_BOAT,              TRUE },
  { "npccorpse",            ITEM_CORPSE_NPC,        TRUE },
  { "pc corpse",            ITEM_CORPSE_PC,         FALSE },
  { "fountain",             ITEM_FOUNTAIN,          TRUE },
  { "pill",                 ITEM_PILL,              TRUE },
  { "protect",              ITEM_PROTECT,           TRUE },
  { "map",                  ITEM_MAP,               TRUE },
  {   "portal",             ITEM_PORTAL,            TRUE },
  {   "warpstone",          ITEM_WARP_STONE,        TRUE },
  { "roomkey",              ITEM_ROOM_KEY,          TRUE },
  {         "gem",                  ITEM_GEM,               TRUE },
  { "jewelry",              ITEM_JEWELRY,           TRUE },
  { "jukebox",              ITEM_JUKEBOX,           TRUE },
  { "pistol",		ITEM_PISTOL,		TRUE },
  { "submachinegun",	ITEM_SMG,		TRUE },
  { "shotgun",		ITEM_SHOTGUN,		TRUE },
  { "rifle",		ITEM_RIFLE,		TRUE },
  { "heavy gun",	ITEM_HEAVYGUN,		TRUE },
  { "energy gun",	ITEM_ENERGYGUN,		TRUE },
  { "ammunition",	    ITEM_AMMO,		    TRUE },
  { "magazine",		    ITEM_CLIP,		    TRUE },
  { NULL,                   0,                      0 }
};

const struct flag_type extra_flags[] =
{
  { "glow",                 ITEM_GLOW,              TRUE },
  { "hum",                  ITEM_HUM,               TRUE },
  { "dark",                 ITEM_DARK,              TRUE },
  { "lock",                 ITEM_LOCK,              TRUE },
  { "evil",                 ITEM_EVIL,              TRUE },
  { "invis",                ITEM_INVIS,             TRUE },
  { "magic",                ITEM_MAGIC,             TRUE },
  { "nodrop",               ITEM_NODROP,            TRUE },
  { "bless",                ITEM_BLESS,             TRUE },
  { "antigood",             ITEM_ANTI_GOOD,         TRUE },
  { "antievil",             ITEM_ANTI_EVIL,         TRUE },
  { "antineutral",          ITEM_ANTI_NEUTRAL,      TRUE },
  { "noremove",             ITEM_NOREMOVE,          TRUE },
  { "inventory",            ITEM_INVENTORY,         TRUE },
  { "nopurge",              ITEM_NOPURGE,           TRUE },
  { "rotdeath",             ITEM_ROT_DEATH,         TRUE },
  { "visdeath",             ITEM_VIS_DEATH,         TRUE },
  { "nolocate",             ITEM_NOLOCATE,          TRUE },
  { "nonmetal",             ITEM_NONMETAL,          TRUE },
  { "meltdrop",             ITEM_MELT_DROP,         TRUE },
  { "hadtimer",             ITEM_HAD_TIMER,         TRUE },
  { "sellextract",          ITEM_SELL_EXTRACT,      TRUE },
  { "gunburst",             ITEM_GUNBURST,          TRUE },
  { "burnproof",            ITEM_BURN_PROOF,        TRUE },
  { "nouncurse",            ITEM_NOUNCURSE,         TRUE },
  { "gunauto",              ITEM_GUNAUTO,           TRUE },
  { NULL,                   0,                      0 }
};

const struct flag_type wear_flags[] =
{
  { "take",                 ITEM_TAKE,              TRUE },
  { "finger",               ITEM_WEAR_FINGER,       TRUE },
  { "neck",                 ITEM_WEAR_NECK,         TRUE },
  { "body",                 ITEM_WEAR_BODY,         TRUE },
  { "head",                 ITEM_WEAR_HEAD,         TRUE },
  { "legs",                 ITEM_WEAR_LEGS,         TRUE },
  { "feet",                 ITEM_WEAR_FEET,         TRUE },
  { "hands",                ITEM_WEAR_HANDS,        TRUE },
  { "arms",                 ITEM_WEAR_ARMS,         TRUE },
  { "shield",               ITEM_WEAR_SHIELD,       TRUE },
  { "about",                ITEM_WEAR_ABOUT,        TRUE },
  { "waist",                ITEM_WEAR_WAIST,        TRUE },
  { "wrist",                ITEM_WEAR_WRIST,        TRUE },
  { "wield",                ITEM_WIELD,             TRUE },
  { "hold",                 ITEM_HOLD,              TRUE },
  {   "nosac",              ITEM_NO_SAC,            TRUE },
  { "wearfloat",            ITEM_WEAR_FLOAT,        TRUE },
  /*  {   "twohands",            ITEM_TWO_HANDS,         TRUE    }, */
  { NULL,                   0,                      0 }
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
  { "none",                 APPLY_NONE,             TRUE },
  { "strength",             APPLY_STR,              TRUE },
  { "dexterity",            APPLY_DEX,              TRUE },
  { "intelligence",         APPLY_INT,              TRUE },
  { "wisdom",               APPLY_WIS,              TRUE },
  { "constitution",         APPLY_CON,              TRUE },
  { "sex",                  APPLY_SEX,              TRUE },
  { "class",                APPLY_CLASS,            TRUE },
  { "level",                APPLY_LEVEL,            TRUE },
  { "age",                  APPLY_AGE,              TRUE },
  { "height",               APPLY_HEIGHT,           TRUE },
  { "weight",               APPLY_WEIGHT,           TRUE },
  { "mana",                 APPLY_MANA,             TRUE },
  { "hp",                   APPLY_HIT,              TRUE },
  { "move",                 APPLY_MOVE,             TRUE },
  { "gold",                 APPLY_GOLD,             TRUE },
  { "experience",           APPLY_EXP,              TRUE },
  { "ac",                   APPLY_AC,               TRUE },
  { "hitroll",              APPLY_HITROLL,          TRUE },
  { "damroll",              APPLY_DAMROLL,          TRUE },
  { "saves",                APPLY_SAVES,            TRUE },
  { "savingpara",           APPLY_SAVING_PARA,      TRUE },
  { "savingrod",            APPLY_SAVING_ROD,       TRUE },
  { "savingpetri",          APPLY_SAVING_PETRI,     TRUE },
  { "savingbreath",         APPLY_SAVING_BREATH,    TRUE },
  { "savingspell",          APPLY_SAVING_SPELL,     TRUE },
  { "spellaffect",          APPLY_SPELL_AFFECT,     FALSE },
  { NULL,                   0,                      0 }
};

/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
  { "in the inventory",     WEAR_NONE,      TRUE },
  { "as a light",           WEAR_LIGHT,     TRUE },
  { "on the left finger",   WEAR_FINGER_L,  TRUE },
  { "on the right finger",  WEAR_FINGER_R,  TRUE },
  { "around the neck (1)",  WEAR_NECK_1,    TRUE },
  { "around the neck (2)",  WEAR_NECK_2,    TRUE },
  { "on the body",          WEAR_BODY,      TRUE },
  { "over the head",        WEAR_HEAD,      TRUE },
  { "on the legs",          WEAR_LEGS,      TRUE },
  { "on the feet",          WEAR_FEET,      TRUE },
  { "on the hands",         WEAR_HANDS,     TRUE },
  { "on the arms",          WEAR_ARMS,      TRUE },
  { "as a shield",          WEAR_SHIELD,    TRUE },
  { "about the shoulders",  WEAR_ABOUT,     TRUE },
  { "around the waist",     WEAR_WAIST,     TRUE },
  { "on the left wrist",    WEAR_WRIST_L,   TRUE },
  { "on the right wrist",   WEAR_WRIST_R,   TRUE },
  { "wielded",              WEAR_WIELD,     TRUE },
  { "held in the hands",    WEAR_HOLD,      TRUE },
  { "floating nearby",      WEAR_FLOAT,     TRUE },
  { NULL,                   0             , 0 }
};

const struct flag_type wear_loc_flags[] =
{
  { "none",         WEAR_NONE,      TRUE },
  { "light",        WEAR_LIGHT,     TRUE },
  { "lfinger",      WEAR_FINGER_L,  TRUE },
  { "rfinger",      WEAR_FINGER_R,  TRUE },
  { "neck1",        WEAR_NECK_1,    TRUE },
  { "neck2",        WEAR_NECK_2,    TRUE },
  { "body",         WEAR_BODY,      TRUE },
  { "head",         WEAR_HEAD,      TRUE },
  { "legs",         WEAR_LEGS,      TRUE },
  { "feet",         WEAR_FEET,      TRUE },
  { "hands",        WEAR_HANDS,     TRUE },
  { "arms",         WEAR_ARMS,      TRUE },
  { "shield",       WEAR_SHIELD,    TRUE },
  { "about",        WEAR_ABOUT,     TRUE },
  { "waist",        WEAR_WAIST,     TRUE },
  { "lwrist",       WEAR_WRIST_L,   TRUE },
  { "rwrist",       WEAR_WRIST_R,   TRUE },
  { "wielded",      WEAR_WIELD,     TRUE },
  { "hold",         WEAR_HOLD,      TRUE },
  { "floating",     WEAR_FLOAT,     TRUE },
  { NULL,           0,              0 }
};

const struct flag_type container_flags[] =
{
  { "closeable",            1,              TRUE },
  { "pickproof",            2,              TRUE },
  { "closed",               4,              TRUE },
  { "locked",               8,              TRUE },
  { "puton",                16,             TRUE },
  { NULL,                   0,              0 }
};

/* ROM-specific tables */

const struct flag_type ac_type[] =
{
  {   "pierce",        AC_PIERCE,            TRUE },
  {   "bash",          AC_BASH,              TRUE },
  {   "slash",         AC_SLASH,             TRUE },
  {   "exotic",        AC_EXOTIC,            TRUE },
  {   NULL,              0,                    0 }
};


const struct flag_type size_flags[] =
{
  {   "tiny",          SIZE_TINY,            TRUE },
  {   "small",         SIZE_SMALL,           TRUE },
  {   "medium",        SIZE_MEDIUM,          TRUE },
  {   "large",         SIZE_LARGE,           TRUE },
  {   "huge",          SIZE_HUGE,            TRUE },
  {   "giant",         SIZE_GIANT,           TRUE },
  {   NULL,              0,                    0 },
};

const struct flag_type weapon_class[] =
{
  {   "exotic",     WEAPON_EXOTIC,          TRUE },
  {   "sword",      WEAPON_SWORD,           TRUE },
  {   "dagger",     WEAPON_DAGGER,          TRUE },
  {   "spear",      WEAPON_SPEAR,           TRUE },
  {   "mace",               WEAPON_MACE,            TRUE },
  {   "axe",                WEAPON_AXE,             TRUE },
  {   "flail",      WEAPON_FLAIL,           TRUE },
  {   "whip",               WEAPON_WHIP,            TRUE },
  {   "polearm",    WEAPON_POLEARM,         TRUE },
  {   NULL,         0,                      0 }
};

const struct flag_type weapon_type2[] =
{
  {   "flaming",       WEAPON_FLAMING,       TRUE },
  {   "frost",         WEAPON_FROST,         TRUE },
  {   "vampiric",      WEAPON_VAMPIRIC,      TRUE },
  {   "sharp",         WEAPON_SHARP,         TRUE },
  {   "vorpal",        WEAPON_VORPAL,        TRUE },
  {   "twohands",     WEAPON_TWO_HANDS,     TRUE },
  { "shocking",      WEAPON_SHOCKING,      TRUE },
  { "poison",       WEAPON_POISON,          TRUE },
  {   NULL,              0,                    0 }
};

const struct flag_type res_flags[] =
{
  {	"summon",	RES_SUMMON,	TRUE },
  {	"charm",	RES_CHARM,	TRUE },
  {	"magic",	RES_MAGIC,	TRUE },
  { 	"weapon",	RES_WEAPON,	TRUE },
  {	"bash",		RES_BASH,	TRUE },
  {	"pierce",	RES_PIERCE,	TRUE },
  {	"slash",	RES_SLASH,	TRUE },
  {	"fire",		RES_FIRE,	TRUE },
  {	"cold",		RES_COLD,	TRUE },
  {	"lightning",	RES_LIGHTNING,	TRUE },
  {	"acid",		RES_ACID,	TRUE },
  {	"poison",	RES_POISON,	TRUE },
  {	"negative",	RES_NEGATIVE,	TRUE },
  {	"holy",		RES_HOLY,	TRUE },
  {	"energy",	RES_ENERGY,	TRUE },
  {	"mental",	RES_MENTAL,	TRUE },
  {	"disease",	RES_DISEASE,	TRUE },
  {	"drowning",	RES_DROWNING,	TRUE },
  {	"light",	RES_LIGHT,	TRUE },
  {	"sound",	RES_SOUND,	TRUE },
  {	"gravity",	RES_GRAVITY,	TRUE },
  {	"wood",		RES_WOOD,	TRUE },
  {	"silver",	RES_SILVER,	TRUE },
  {	"iron",		RES_IRON,	TRUE },
  {	NULL,		0,		0 }
};

const struct flag_type vuln_flags[] =
{
  {	"summon",	VULN_SUMMON,	TRUE },
  {	"charm",	VULN_CHARM,	TRUE },
  {	"magic",	VULN_MAGIC,	TRUE },
  {	"weapon",	VULN_WEAPON,	TRUE },
  {	"bash",		VULN_BASH,	TRUE },
  {	"pierce",	VULN_PIERCE,	TRUE },
  {	"slash",	VULN_SLASH,	TRUE },
  {	"fire",		VULN_FIRE,	TRUE },
  {	"cold",		VULN_COLD,	TRUE },
  {	"lightning",	VULN_LIGHTNING,	TRUE },
  {	"acid",		VULN_ACID,	TRUE },
  {	"poison",	VULN_POISON,	TRUE },
  {	"negative",	VULN_NEGATIVE,	TRUE },
  {	"holy",		VULN_HOLY,	TRUE },
  {	"energy",	VULN_ENERGY,	TRUE },
  {	"mental",	VULN_MENTAL,	TRUE },
  {	"disease",	VULN_DISEASE,	TRUE },
  {	"drowning",	VULN_DROWNING,	TRUE },
  {	"light",	VULN_LIGHT,	TRUE },
  {	"sound",	VULN_SOUND,	TRUE },
  {	"gravity",	VULN_GRAVITY,	TRUE },
  {	"wood",		VULN_WOOD,	TRUE },
  {	"silver",	VULN_SILVER,	TRUE },
  {	"iron",		VULN_IRON,	TRUE },
  {	NULL,		0,		0 }
};

const struct flag_type position_flags[] =
{
  {   "dead",           POS_DEAD,            FALSE },
   {   "mortal",         POS_MORTAL,          FALSE },
   {   "incap",          POS_INCAP,           FALSE },
   {   "stunned",        POS_STUNNED,         FALSE },
   {   "sleeping",       POS_SLEEPING,        TRUE },
   {   "resting",        POS_RESTING,         TRUE },
   {   "sitting",        POS_SITTING,         TRUE },
   {   "fighting",       POS_FIGHTING,        FALSE },
   {   "standing",       POS_STANDING,        TRUE },
   {   NULL,              0,                    0 }
};

const struct flag_type portal_flags[]=
 {
   {   "normal_exit",          GATE_NORMAL_EXIT,     TRUE },
   { "no_curse",       GATE_NOCURSE,         TRUE },
   {   "go_with",      GATE_GOWITH,          TRUE },
   {   "buggy",        GATE_BUGGY,           TRUE },
   { "random",         GATE_RANDOM,          TRUE },
   {   NULL,           0,                    0 }
 };

const struct flag_type furniture_flags[]=
{
  {   "stand_at",     STAND_AT,             TRUE },
  { "stand_on",       STAND_ON,             TRUE },
  { "stand_in",       STAND_IN,             TRUE },
  { "sit_at",         SIT_AT,               TRUE },
  { "sit_on",         SIT_ON,               TRUE },
  { "sit_in",         SIT_IN,               TRUE },
  { "rest_at",        REST_AT,              TRUE },
  { "rest_on",        REST_ON,              TRUE },
  { "rest_in",        REST_IN,              TRUE },
  { "sleep_at",       SLEEP_AT,             TRUE },
  { "sleep_on",       SLEEP_ON,             TRUE },
  { "sleep_in",       SLEEP_IN,             TRUE },
  { "put_at",         PUT_AT,               TRUE },
  { "put_on",         PUT_ON,               TRUE },
  { "put_in",         PUT_IN,               TRUE },
  { "put_inside",     PUT_INSIDE,           TRUE },
  { NULL,             0,                    0 }
};

const struct  flag_type       apply_types     []      =
{
  {       "affects",      TO_AFFECTS,     TRUE },
  {       "object",       TO_OBJECT,      TRUE },
  {       "immune",       TO_IMMUNE,      TRUE },
  {       "resist",       TO_RESIST,      TRUE },
  {       "vuln",         TO_VULN,        TRUE },
  {       "weapon",       TO_WEAPON,      TRUE },
  {       NULL,           0,              TRUE }
};

 const struct  bit_type        bitvector_type  []      =
 {
   {       affect_flags,   "affect" },
   {       apply_flags,    "apply" },
   {       imm_flags,      "imm" },
   {       res_flags,      "res" },
   {       vuln_flags,     "vuln" },
   {       weapon_type2,   "weapon" }
 };



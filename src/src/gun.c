#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/* Local functions. */
void gun_hit args((CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * gun, int range));
CHAR_DATA *find_target args((CHAR_DATA * ch, char *argument, int direction, int range));
int seek_target args((CHAR_DATA * ch, CHAR_DATA * victim, int direction, int range));

/* Ammo tables. - Kevin */
const struct ammo_type ammo_table[BULLET_CALIBER] = {
/*
    {
     "caliber name",            - Caliber (9mm, .380, .35, 12ga, etc.)
     {"type 1", "type 2" },     - Ammunition types (FMJ, JHP, AP, etc.)
     { "`1b`!l`#a`!s`1t``",
       "`1b`!l`#a`!s`1t``" },	- Ammunition nouns (shows up in dam_message)
     { 0, 0 },			- Special effects bits: AMMO_FLECHETTE, AMMO_PENETRATING, et al.
     { DAM_NONE, DAM_NONE },    - Damage type per bullet (ice rounds, anyone? O_o)
     { 8, 8 },                  - Recoil lag, measured in pulses per shot
     { {0, 0}, {0, 0} },        - Vnums for unloading single/multiple rounds from a gun
     { {5, 10}, {10, 20} }      - Bullet damage range, lowest to highest
    }
*/
    {
     "9mm",
     { "FMJ", "JHP" },
     { "`7sh`&o`7t``", "`7sh`&o`7t``" },
     { 0, AMMO_FLECHETTE },
     { DAM_PIERCE, DAM_PIERCE },
     { 10, 10 },
     { {1800, 1802}, {1801, 1803} },
     { {40, 80}, {60, 120} }
    },

    {
     ".45ACP",
     { "FMJ", "JHP" },
     { "`8s`7h`&o`8t``", "`8s`7h`&o`8t``" },
     { 0, AMMO_FLECHETTE },
     { DAM_PIERCE, DAM_PIERCE },
     { 12, 12 },
     { {1804, 1806}, {1805, 1807} },
     { {50, 100}, {75, 150} }
    },

    {
     ".22",
     { "FMJ", "JHP" },
     { "shot", "shot" },
     { 0, AMMO_FLECHETTE },
     { DAM_PIERCE, DAM_PIERCE },
     { 8, 8 },
     { {1808, 1810}, {1809, 1811} },
     { {20, 40}, {30, 60} }
    },

    {
     "12 gauge",
     { "shot", "slug" },
     { "`8b`7u`8cksh`7o`8t``", "`&s`7l`8u`7g``" },
     { AMMO_FLECHETTE, AMMO_PENETRATING },
     { DAM_PIERCE, DAM_PIERCE },
     { 16, 16 },
     { {1812, 1814}, {1813, 1815} },
     { {110, 220}, {90, 180} }
    }
};

/* Firing mode toggle for guns. - Kevin */
void do_gtoggle(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int range = 0;
    OBJ_DATA *obj;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    
    if (arg1[0] == '\0') {
	send_to_char("Toggle what?\n\r", ch);
	return;
    } else if ((obj = get_obj_carry(ch, arg1)) == NULL) {
	obj = get_obj_wear(ch, arg1);
	if (obj == NULL) {
	    send_to_char("Toggle what?\n\r", ch);
	    return;
	}
    }

    if (!IS_GUN(obj)) {
	send_to_char("That is not a gun.", ch);
	return;
    }

    if (!IS_OBJ_STAT(obj, ITEM_GUNBURST)
	&& !IS_OBJ_STAT(obj, ITEM_GUNAUTO)) {
	send_to_char("This firearm cannot be toggled.\n\r", ch);
	return;
    }

    if (arg2[0] == '\0') {
	sprintf(buf, "Available fire settings for %s:\n\r", obj->short_descr);
	send_to_char(buf, ch);

	sprintf(buf, "%s%s%s\n\r",
	   "semi-automatic",
	    IS_OBJ_STAT(obj, ITEM_GUNBURST) ? "   burstfire" : "",
	    IS_OBJ_STAT(obj, ITEM_GUNAUTO) ? "   full-automatic" : "");
	send_to_char(buf, ch);
    } else {
	/* Get gun range. */
	range = gun_range(obj);

	/* Now, we mess with the gun's condition.. */
	if (!str_prefix(arg2, "semi-automatic")) {
	    if (!(obj->condition & BURST)
		&& !(obj->condition & FULLAUTO)) {
		sprintf(buf, "%s is already in semi-automatic mode.\n\r", obj->short_descr);
		send_to_char(buf, ch);
		return;
	    }

	    obj->condition = range;
	    sprintf(buf, "Fire mode in %s set to semi-automatic.\n\r", obj->short_descr);
	    send_to_char(buf, ch);
	} else if (!str_prefix(arg2, "burstfire")) {
	    if (!IS_OBJ_STAT(obj, ITEM_GUNBURST)) {
		sprintf(buf, "%s cannot be set to burst mode.\n\r", obj->short_descr);
		send_to_char(buf, ch);
		return;
	    } else if (obj->condition & BURST) {
		sprintf(buf, "%s is already in burst mode.\n\r", obj->short_descr);
		send_to_char(buf, ch);
		return;
	    }

	    obj->condition = BURST + range;
	    sprintf(buf, "Fire mode in %s set to burst.\n\r", obj->short_descr);
	    send_to_char(buf, ch);
	} else if (!str_prefix(arg2, "full-automatic")) {
	    if (!IS_OBJ_STAT(obj, ITEM_GUNAUTO)) {
		sprintf(buf, "%s cannot be set to full-automatic mode.\n\r", obj->short_descr);
		send_to_char(buf, ch);
		return;
	    } else if (obj->condition & FULLAUTO) {
		sprintf(buf, "%s is already in full-automatic mode.\n\r", obj->short_descr);
		send_to_char(buf, ch);
		return;
	    }

	    obj->condition = FULLAUTO + range;
	    sprintf(buf, "Fire mode in %s set to full-automatic.\n\r", obj->short_descr);
	    send_to_char(buf, ch);
	} else {
	    send_to_char("That firing mode does not exist.\n\r", ch);
	    return;
	}
    }
    WAIT_STATE(ch, .5 * PULSE_VIOLENCE);
    return;
}

void do_reload(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *obj_ammo;
    int ammotype = 0, caliber = 0, rounds = 0;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0') {
        send_to_char("Reload what?\n\r", ch);
        return;
    }

    if ((obj_ammo = get_obj_carry(ch, arg1)) == NULL) {
	send_to_char("Reload what?\n\r", ch);
	return;
    } else if (obj_ammo->item_type != ITEM_AMMO) {
	send_to_char("You can't reload that into a gun or clip.\n\r", ch);
	return;
    }

    caliber = obj_ammo->value[0];
    ammotype = obj_ammo->value[1];
    
    if (caliber >= BULLET_CALIBER) {
	bug("do_reload: undefined caliber.", 0);
	sprintf(buf, "%s has an undefined caliber; note/talk to an immortal.\n\r",
	    obj_ammo->short_descr);
	send_to_char(buf, ch);
	return;
    } else if (ammotype >= BULLET_TYPE) {
	bug("do_reload: undefined bullet type.", 0);
	sprintf(buf, "%s has an undefined ammo type; note/talk to an immortal.\n\r",
	    obj_ammo->short_descr);
	send_to_char(buf, ch);
	return;
    }

    if (arg2[0] == '\0') {
	sprintf(buf, "Reload %s into what?\n\r", obj_ammo->short_descr);
	send_to_char(buf, ch);
	return;
    } else {    
	obj = get_obj_wear(ch, arg2);
	if (obj == NULL) {
	    obj = get_obj_carry(ch, arg2);
	    if (obj == NULL) {
		sprintf(buf, "Reload %s into what?\n\r", obj_ammo->short_descr);
		send_to_char(buf, ch);
		return;
	    } else if (!IS_GUN(obj)) {
		sprintf(buf, "You can't reload %s.\n\r", obj->short_descr);
		send_to_char(buf, ch);
		return;
	    }
	} else if (!IS_GUN(obj)) {
	    sprintf(buf, "You can't reload %s.\n\r", obj->short_descr);
	    send_to_char(buf, ch);
	    return;
	}
    }

    if (obj->value[0] != caliber) {
	sprintf(buf, "%s is not chambered for that type of ammunition.\n\r",
	    obj->short_descr);
	send_to_char(buf, ch);
	return;
    }

    if (obj_ammo->value[2] <= 0) {
	send_to_char("You can't reuse spent ammunition.\n\r", ch);
	return;
    }

    if (obj->value[2] >= obj->value[4]) {
	sprintf(buf, "%s cannot hold any more rounds.\n\r", obj->short_descr);
	send_to_char(buf, ch);
	return;
    }

    if (obj->value[2] > 0
	&& obj->value[1] != obj_ammo->value[1]) {
	send_to_char("The bullet types don't match.\n\r", ch);
	return;
    }

    rounds = obj->value[4] - obj->value[2];
    if (obj_ammo->value[2] < rounds) {
	rounds = obj_ammo->value[2];
    }

    obj->value[1] = obj_ammo->value[1];
    obj->value[2] += rounds;
    obj_ammo->value[2] -= rounds;

    sprintf(buf, "You load %d round%s into %s.\n\r",
	rounds, rounds > 1 ? "s" : "", obj->short_descr);
    send_to_char(buf, ch);
    act("$n reloads $p.", ch, obj, NULL, TO_ROOM);

    if (obj_ammo->value[2] <= 0) {
	obj_from_char(obj_ammo);
	extract_obj(obj_ammo);
    }
    WAIT_STATE(ch, 1.5 * PULSE_VIOLENCE);   
    return;
}

void do_unload(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *obj_ammo;
    int ammotype = 0;   
    int caliber = 0;
    int amount = 0;
    int vnum = 0;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Unload what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_wear(ch, arg)) == NULL) {
	obj = get_obj_carry(ch, arg);
	if (obj == NULL) {
	    send_to_char("Unload what?\n\r", ch);
	    return;
	}
    }

    if (!IS_GUN(obj)) {
	send_to_char("That cannot be unloaded.\n\r", ch);
	return;
    }

    if (obj->value[2] <= 0) {
	send_to_char("This gun is empty.\n\r", ch);
	return;
    } else if (obj->value[2] >= 2) {
	amount = BULLET_MAX;
    } else {
	amount = BULLET_MIN;
    }

    caliber = obj->value[0];
    ammotype = obj->value[1];

    vnum = ammo_table[caliber].vnum[ammotype][amount];

    if (caliber >= BULLET_CALIBER) {   
	bug("do_unload: undefined caliber, v0 = %d.", caliber);
	sprintf(buf, "%s has an undefined caliber; have an immortal reset it.\n\r",
	    obj->short_descr);
	send_to_char(buf, ch);
	return;
    } else if (ammotype >= BULLET_TYPE) {
	bug("do_unload: undefined ammo type, v1 = %d.", ammotype);
	sprintf(buf, "%s has an undefined ammotype; have an immortal reset it.\n\r",
	    obj->short_descr);
	send_to_char(buf, ch);
	return;
    }

    if (get_obj_index(vnum) == NULL) {
	bug("do_unload: null ammunition vnum.", 0);
	sprintf(buf, "%s %s has a null vnum; talk/note to an immortal.\n\r",
	    ammo_table[caliber].caliber,
	    ammo_table[caliber].ammotype[ammotype]);
	send_to_char(buf, ch);
	return;
    }

    if (get_obj_index(vnum)->item_type != ITEM_AMMO) {
	bug("do_unload: invalid item type on vnum %d.", vnum);
	sprintf(buf, "Invalid item type on vnum %d.\n\r", vnum);
	send_to_char(buf, ch);
	return;
    }

    obj_ammo = create_object(get_obj_index(vnum), 0);
    obj_ammo->value[2] = obj->value[2];
    obj->value[2] = 0;
    obj_to_char(obj_ammo, ch);

    sprintf(buf, "You unload %ld round%s from %s.\n\r",
	obj_ammo->value[2], 
	obj_ammo->value[2] > 1 ? "s" : "", obj->short_descr);
    send_to_char(buf, ch);
    act("$n unloads $p.", ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, 1.5 * PULSE_VIOLENCE);
    return;
}

void do_shoot(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    char *action;
    CHAR_DATA *vch, *vch_next;
    CHAR_DATA *victim = NULL;
    OBJ_DATA *gun;
    int ammotype, caliber;      // Used to check validity of gun caliber/ammo type
    int count, shots;           // Shot counters
    int mode, recoil;		// Fire mode, recoil;
    int dir, range;		// Direction, range of shot    

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (get_skill(ch, gsn_gun) == 0
	|| IS_NPC(ch)
	|| (!IS_NPC(ch) && ch->level < skill_table[gsn_gun].skill_level[ch->class])) {
	send_to_char("Shooting? What's that?\n\r", ch);
	return;
    }

    if ((gun = get_eq_char(ch, WEAR_HOLD)) == NULL) {
	send_to_char("You need to hold a gun to shoot someone.\n\r", ch);
	return;
    } else if (!IS_GUN(gun)) {
	sprintf(buf, "%s is not a gun, you can't shoot with it.\n\r",
	    gun->short_descr);
	send_to_char(buf, ch);
	return;
    }
    
    if (arg1[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("Shoot whom?\n\r", ch);
	    return;
	}
    }

/* Find direction of shot. */
    if (arg2[0] == '\0') {
	dir = -1;
    } else {
	if (!str_prefix(arg2, "north"))
	    dir = 0;
	else if (!str_prefix(arg2, "east"))
	    dir = 1;
	else if (!str_prefix(arg2, "south"))
	    dir = 2;
	else if (!str_prefix(arg2, "west"))
	    dir = 3;
	else if (!str_prefix(arg2, "up"))
	    dir = 4;
	else if (!str_prefix(arg2, "down"))
	    dir = 5;
	else {
	    send_to_char("That is not a valid direction.\n\r", ch);
	    return;
	}
    }

/* Find range value on the gun.. */
    range = gun_range(gun);

    if (victim == NULL)
	victim = find_target(ch, arg1, dir, range);
    range = seek_target(ch, victim, dir, range);

/* If victim not found, return */
    if (range == -1 || victim == NULL)
	return;

/* NPC check. For now, force players to shoot them in the same room. */
    if (IS_NPC(victim) && range > 0 && !IS_IMMORTAL(ch)) {
	send_to_char("Try shooting mobiles when you're in the same room.\n\r", ch);
	return;
    }

/* Check caliber/ammo validity */
    caliber = gun->value[0];
    ammotype = gun->value[1];

    if (caliber >= BULLET_CALIBER) {
	sprintf(buf, "Invalid caliber: v0 = %ld.\n\r", gun->value[0]);
	send_to_char(buf, ch);
	return;
    }
    if (ammotype >= BULLET_TYPE) {
	sprintf(buf, "Invalid ammunition type: v1 = %ld.\n\r", gun->value[1]);
	send_to_char(buf, ch);
	return;
    }

    if ((IS_SET(victim->act, PLR_DEAD))
	|| ((IS_SET(ch->act, PLR_DEAD)) && (!IS_NPC(victim)))) {
	send_to_char("You are not permitted to kill that person!\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;
                    
    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("You'll shoot your eye out!\n\r", ch);
	return;
    }
    
    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
        send_to_char("You can't bring yourself to cap your beloved master.\n\r", ch);
        return;
    }

/* Determine firing mode, then set number of shots. */
    if (gun->condition & BURST) {
	mode = BURST;
    } else if (gun->condition & FULLAUTO) {
	mode = FULLAUTO;
    } else {
	mode = SEMIAUTO;
    }

    switch (mode) {
	default:
	case SEMIAUTO:  /* 100% recoil per command */
	    shots = 1;
	    recoil = ammo_table[caliber].recoil[ammotype];
	    break;
	case BURST:     /* 180% recoil per command (60% per shot) */
	    shots = 3;
	    if (gun->value[2] < shots)
		shots = gun->value[2];
	    recoil = (shots * ammo_table[caliber].recoil[ammotype]) * 3 / 5;
	    break;
	case FULLAUTO: /* 150+% recoil per command (50% per shot) */
	    shots = dice(1, 4) + 2;
	    if (IS_AFFECTED(ch, AFF_HASTE) || IS_SET(ch->off_flags, OFF_FAST))
		shots++;
	    recoil = (shots * ammo_table[caliber].recoil[ammotype]) / 2;
	    break;
    }
/* Add extra recoil if ranged shot, due to aiming. - Kevin */
    recoil += range * 3;

/* Text of shooting which other players see. Hooray! - Kevin */
    switch (gun->item_type) {
    default:
    case ITEM_PISTOL:
	action = "draw";
	break;
    case ITEM_SMG:
    case ITEM_SHOTGUN:
    case ITEM_RIFLE:
    case ITEM_ENERGYGUN:
    case ITEM_HEAVYGUN:
	switch (number_range(1, 3)) {
	default:
	case 1:
	    action = "level";
	    break;
	case 2:
	    action = "shoulder";
	    break;
	case 3:
	    action = "raise";
	    break;
	}
    }

    if (victim->in_room == ch->in_room) {
	if (gun->value[2] < 1) {
	    act("`^$n tries to shoot $N with an unloaded gun!``",
		ch, gun, victim, TO_NOTVICT);
	    act("`^$n tries to shoot you, but $s gun is empty!``",
		ch, gun, victim, TO_VICT);
	    act("`^You attempt to shoot $N with an unloaded gun. `8*`7Click`8*``",
		ch, NULL, victim, TO_CHAR);
	    WAIT_STATE(ch, PULSE_VIOLENCE);
	    return;
	} else {
	    sprintf(buf, "`^$n %ss $s $p`^, firing at $N!``", action);
	    act(buf, ch, gun, victim, TO_NOTVICT);
	    sprintf(buf, "`^$n %ss $s $p`^, firing at you!``", action);
	    act(buf, ch, gun, victim, TO_VICT);
	    sprintf(buf, "`^You %s your $p`^ at $N and pull the trigger!``",
		action == "draw" ? "point" : action);
	    act(buf, ch, gun, victim, TO_CHAR);
	}
    } else {
	if (gun->value[2] < 1) {
	    sprintf(buf, "`^$n tries to fire his unloaded gun %swards.``", dir_name[dir]);
	    act(buf, ch, NULL, NULL, TO_ROOM);
	    sprintf(buf, "`^You %s your unloaded firearm. `8*`7Click`8*``", action);
	    act(buf, ch, NULL, NULL, TO_CHAR);
	    WAIT_STATE(ch, PULSE_VIOLENCE);
	    return;
	} else {
	    sprintf(buf, "`^You %s your $p`^, aim, and fire %swards at %s!``",
		action, dir_name[dir],
		IS_NPC(victim) ? victim->short_descr : victim->name);
	    act(buf, ch, gun, NULL, TO_CHAR);
	    sprintf(buf, "`^$n %ss $s $p, aims, and fires %swards!``",
		action, dir_name[dir]);
	    act(buf, ch, gun, NULL, TO_ROOM);
	    sprintf(buf, "`^A shot erupts from %s%s towards you!``",
		rev_dir[dir] < 4 ? "the " : "",
		rev_dir[dir] == 5 ? "below" :
		(rev_dir[dir] == 4 ? "above" : dir_name[rev_dir[dir]]));
	    act(buf, victim, NULL, NULL, TO_CHAR);
	    sprintf(buf, "`^A shot echoes from %s%s, towards $n!``",
		rev_dir[dir] < 4 ? "the " : "",
		rev_dir[dir] == 5 ? "below" :
		(rev_dir[dir] == 4 ? "above" : dir_name[rev_dir[dir]]));
	    act(buf, victim, NULL, NULL, TO_ROOM);
	}
    }

/* Area echo text. */
    for (vch = char_list; vch != NULL; vch = vch_next) {
	vch_next = vch->next;

	if (vch->in_room == NULL)
	    continue;

	if ((vch == ch || vch->in_room == ch->in_room)
	    || (vch == victim || vch->in_room == victim->in_room))
	    continue;

	if (vch->in_room->area == ch->in_room->area)
	    send_to_char("The sound of gunfire echoes through the air.\n\r", vch);
    }


    WAIT_STATE(ch, recoil);
    check_killer(ch, victim);
    for (count = 1; count <= shots; count++) {
	if (gun->value[2] < 1) {
	    send_to_char("`8*`7Click`8*`` Your gun is out of ammunition.\n\r", ch);
	    break;
	}
	gun->value[2]--;
	gun_hit(ch, victim, gun, range);

	if (victim->position == POS_DEAD)
	    break;
    }
    return;
}

/* Returns the damage message(noun) of a particular ammunition
   by examining ch's gun. - Kevin */
char *gun_message(CHAR_DATA * ch)
{
    OBJ_DATA * gun = get_eq_char(ch, WEAR_HOLD);
    char *noun;

    if (gun == NULL || !IS_GUN(gun)) {
	noun = "";
    } else {
	if (gun->value[0] >= BULLET_CALIBER || gun->value[1] >= BULLET_TYPE)
	    noun = "`8gunsh`&o`8t``";
	else
	    noun = ammo_table[gun->value[0]].noun[gun->value[1]];
    }

    return noun;
}

/* Returns a range value of a particular gun. - Kevin */
int gun_range(OBJ_DATA * gun)
{
    int range;

    if (!IS_GUN(gun))	/* Is the object a gun? */
	return -1;

    range = gun->condition;

    if (range & BURST)
	range = gun->condition ^ BURST;
    else if (range & FULLAUTO)
	range = gun->condition ^ FULLAUTO;
	
    return range;
}

/* Complement to Valorath's seeker code.
   Finds a character targeted by the shoot command. - Kevin */

CHAR_DATA *find_target(CHAR_DATA *ch, char *argument, int direction, int range)
{
    ROOM_INDEX_DATA *room, *next_room;
    CHAR_DATA *vch = NULL;
    EXIT_DATA *exit;
    char arg[MAX_INPUT_LENGTH];
    int count, number;
    int i;

    room = ch->in_room;

    if ((vch = get_char_room(ch, argument)) != NULL)
	return vch;

    if (direction == -1)
	return NULL;

    count = 0;
    number = number_argument(argument, arg);

    for (i = 0; i < range; i++) {
	if ((exit = room->exit[direction]) == NULL || (next_room = exit->u1.to_room) == NULL)
	    return NULL;
	if (IS_SET(exit->exit_info, EX_CLOSED))
	    return NULL;
	room = next_room;
	next_room = NULL;
	vch = NULL;
	for (vch = room->people; vch != NULL; vch = vch->next_in_room) {
	    if (!can_see(ch, vch) || !is_name(arg, vch->name))
		continue;
	    if (++count == number)
		return vch;
	}
    }
    return NULL;
}

/* Modified seeker code, originally provided by Kyle/Valorath. */
int seek_target(CHAR_DATA *ch, CHAR_DATA *victim, int direction, int range)
{
    ROOM_INDEX_DATA *room, *next_room;
    CHAR_DATA *vch = NULL;
    EXIT_DATA *exit;
    int i;

    room = ch->in_room;

    if (victim == NULL) {
	send_to_char("Victim not found.\n\r", ch);
	return -1;
    }

    for (vch = room->people; vch != NULL; vch = vch->next_in_room) {
	if (vch == victim)
	    return 0;
    }

    if (direction == -1)
	return -1;

    for (i = 0; i < range; i++) {
	if ((exit = room->exit[direction]) == NULL || (next_room = exit->u1.to_room) == NULL) {
	    send_to_char("A wall blocks your line of sight.\n\r", ch);
	    return -1;
	}
	if (IS_SET(exit->exit_info, EX_CLOSED)) {
	    send_to_char("A closed door blocks your line of sight.\n\r", ch);
	    return -1;
	}
	room = next_room;
	next_room = NULL;
	vch = NULL;
	if (IS_SET(room->room_flags, ROOM_SAFE)) {
	    send_to_char("You cannot fire through a safe room.", ch);
	    return -1;
	}
	for (vch = room->people; vch != NULL; vch = vch->next_in_room) {
	    if (vch == victim)
		return (i + 1);
	}
    }
    send_to_char("Victim not found within range.\n\r", ch);
    return -1;
}

/* Called from do_shoot, processes individual shots from a firearm. - Kevin */
void gun_hit(CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * gun, int range)
{
    int ammotype, caliber;	// Caliber and ammunition type; absolutely necessary
    int chance = 0, dam;	// To-hit chance, damage
    int diceroll = 0, dt;	// Bonus damage, damage type
    int flags, victim_ac;	// Ammo flags, victim's AC

    if (!IS_GUN(gun))
	return;

    caliber = gun->value[0];
    ammotype = gun->value[1];
    dt = ammo_table[caliber].attack[ammotype];
    flags = ammo_table[caliber].flags[ammotype];    

/* First, get the victim's AC.. */
    switch(dt) {
    case DAM_BASH:
	victim_ac = GET_AC(victim, AC_BASH);
	break;
    case DAM_PIERCE:
	victim_ac = GET_AC(victim, AC_PIERCE);
	break;
    case DAM_SLASH:
	victim_ac = GET_AC(victim, AC_SLASH);
	break;
    default:
	victim_ac = GET_AC(victim, AC_EXOTIC);
	break;
    }

/* To-hit chance */
    switch(gun->item_type) {
    default:
	break;
    case ITEM_HEAVYGUN:
	chance = (get_skill(ch, gsn_gun) * 2) / 5;
	if (get_skill(ch, gsn_heavygun) > 0)
	    chance += (get_skill(ch, gsn_heavygun) * 2) / 5;
	break;
    case ITEM_ENERGYGUN:
	chance = (get_skill(ch, gsn_gun) * 3) / 5;
	if (get_skill(ch, gsn_energygun) > 0)
	    chance += get_skill(ch, gsn_energygun) / 3;
	break;
    case ITEM_RIFLE:
    case ITEM_SHOTGUN:
	chance = (get_skill(ch, gsn_gun) * 4) / 5;
	if (gun->item_type == ITEM_RIFLE)
	    chance += 10;       /* Rifle accuracy bonus */
	if (get_skill(ch, gsn_rifle) > 0)
	    chance += get_skill(ch, gsn_rifle) / 4;
	break;
    case ITEM_SMG:
    case ITEM_PISTOL:
	chance = get_skill(ch, gsn_gun);
	if (gun->item_type == ITEM_SMG)
	    chance -= 5;	/* SMG accuracy penalty */
	if (get_skill(ch, gsn_pistol) > 0)
	    chance += get_skill(ch, gsn_pistol) / 5;
	break;
    }

    /* Level difference */
    chance += (ch->level - victim->level) / 3;

    /* To-hit bonuses */
    chance += get_stat_mod(ch, STAT_DEX) * 5;
    chance += get_stat_mod(ch, STAT_WIS) * 2;

    /* To-hit penalties */
    chance -= get_stat_mod(ch, STAT_DEX) * 5;
    chance -= get_stat_mod(ch, STAT_WIS) * 2;
    if (IS_AFFECTED(ch, AFF_BLIND)) {
	chance -= 20;
    }

    /* Is the ammunition flechette or penetrating? */
    if (flags & AMMO_FLECHETTE) {
	chance -= victim_ac / 20;
    } else if (flags & AMMO_PENETRATING) {
	chance -= victim_ac / 33;
    } else {
	chance -= victim_ac / 25;
    }

    /* Is the ammunition easy to dodge? */
    if (flags & AMMO_WIDE) {
	chance -= get_skill(victim, gsn_dodge) / (IS_NPC(victim) ? 20 : 10);
    } else {
	chance -= get_skill(victim, gsn_dodge) / (IS_NPC(victim) ? 10 : 5);
    }

    /* Is the ammunition easy to phase through? */
    if (flags & AMMO_ETHEREAL) {
	chance -= get_skill(victim, gsn_phase) / (IS_NPC(victim) ? 20 : 10);
    } else {
	chance -= get_skill(victim, gsn_phase) / (IS_NPC(victim) ? 10 : 5);
    }

    chance -= range * 5;
    if (gun->condition & BURST) {
	chance -= 15;
    } else if (gun->condition & FULLAUTO) {
	chance -= 30;
    }

/* Keep a minor chance of success/failure before moment of truth */
    chance = URANGE(5, chance, 95);
    if (number_percent() > chance) {
	check_improve(ch, gsn_gun, FALSE, 6);
	damage(ch, victim, 0, gsn_gun, DAM_NONE, TRUE);
	return;
    }

/* Base damage. */
    dam = number_range(ammo_table[caliber].dam[ammotype][BULLET_MIN],
	ammo_table[caliber].dam[ammotype][BULLET_MAX]);
    dam += gun->value[3];

    diceroll = number_percent();
    switch(gun->item_type) {
    case ITEM_PISTOL:
    case ITEM_SMG:
	if (diceroll <= get_skill(ch, gsn_pistol)) {
	    check_improve(ch, gsn_pistol, TRUE, 6);
	    dam += dam * diceroll / 100;
	}
	break;
    case ITEM_SHOTGUN:
    case ITEM_RIFLE:
	if (diceroll <= get_skill(ch, gsn_rifle)) {
	    check_improve(ch, gsn_rifle, TRUE, 6);
	    dam += 3 * (dam * diceroll / 400);
	}
	break;
    case ITEM_ENERGYGUN:
	if (diceroll <= get_skill(ch, gsn_energygun)) {
	    check_improve(ch, gsn_energygun, TRUE, 6);
	    dam += 2 * (dam * diceroll / 300);
	}
	break;
    case ITEM_HEAVYGUN:
	if (diceroll <= get_skill(ch, gsn_heavygun)) {
	    check_improve(ch, gsn_heavygun, TRUE, 6);
	    dam += dam * diceroll / 200;
	}
	break;
    }

/* Blindness damage reduction. */
    if (IS_AFFECTED(ch, AFF_BLIND)) {
	dam *= .75;
    }

/* AC damage reduction. Affected by flechette and penetrating */
    if (victim_ac < 0) {
	if (flags & AMMO_FLECHETTE) {
	    dam += victim_ac / 10;
	} else if (flags & AMMO_PENETRATING) {
	    dam += victim_ac / 40;
	} else {
	    dam += victim_ac / 20;
	}
    }

/* Ranged damage reduction. */
    dam -= range * 10;

    if (dam < 1)
	dam = 1;

    check_improve(ch, gsn_gun, TRUE, 6);
    damage(ch, victim, dam, gsn_gun, dt, TRUE);
    return;
}

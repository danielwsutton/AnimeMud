#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"

/* command procedures needed */
DECLARE_DO_FUN(do_rstat);
DECLARE_DO_FUN(do_mstat);
DECLARE_DO_FUN(do_ostat);
DECLARE_DO_FUN(do_rset);
DECLARE_DO_FUN(do_mset);
DECLARE_DO_FUN(do_oset);
DECLARE_DO_FUN(do_sset);
DECLARE_DO_FUN(do_mfind);
DECLARE_DO_FUN(do_ofind);
DECLARE_DO_FUN(do_slookup);
DECLARE_DO_FUN(do_mload);
DECLARE_DO_FUN(do_oload);
DECLARE_DO_FUN(do_force);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_save);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_force);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_echo);
DECLARE_DO_FUN(do_btitle);
DECLARE_DO_FUN(do_award);
/*
 * Local functions.
 */
ROOM_INDEX_DATA *find_location args((CHAR_DATA * ch, char *arg));
void quest_update args((void));	/*TG980628 */

/* External functions */
extern void save_char_fried_obj(CHAR_DATA * ch);

void do_wiznet(CHAR_DATA * ch, char *argument)
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0') {
	if (IS_SET(ch->wiznet, WIZ_ON)) {
	    send_to_char("Signing off of Wiznet.\n\r", ch);
	    REMOVE_BIT(ch->wiznet, WIZ_ON);
	} else {
	    send_to_char("Welcome to Wiznet!\n\r", ch);
	    SET_BIT(ch->wiznet, WIZ_ON);
	}
	return;
    }

    if (!str_prefix(argument, "on")) {
	send_to_char("Welcome to Wiznet!\n\r", ch);
	SET_BIT(ch->wiznet, WIZ_ON);
	return;
    }

    if (!str_prefix(argument, "off")) {
	send_to_char("Signing off of Wiznet.\n\r", ch);
	REMOVE_BIT(ch->wiznet, WIZ_ON);
	return;
    }

    /* show wiznet status */
    if (!str_prefix(argument, "status")) {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet, WIZ_ON))
	    strcat(buf, "off ");

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet, wiznet_table[flag].flag)) {
		strcat(buf, wiznet_table[flag].name);
		strcat(buf, " ");
	    }

	strcat(buf, "\n\r");

	send_to_char("Wiznet status:\n\r", ch);
	send_to_char(buf, ch);
	return;
    }

    if (!str_prefix(argument, "show"))
	/* list of all wiznet options */
    {
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++) {
	    if (wiznet_table[flag].level <= get_trust(ch)) {
		strcat(buf, wiznet_table[flag].name);
		strcat(buf, " ");
	    }
	}

	strcat(buf, "\n\r");

	send_to_char("Wiznet options available to you are:\n\r", ch);
	send_to_char(buf, ch);
	return;
    }

    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level) {
	send_to_char("No such option.\n\r", ch);
	return;
    }

    if (IS_SET(ch->wiznet, wiznet_table[flag].flag)) {
	sprintf(buf, "You will no longer see %s on wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf, ch);
	REMOVE_BIT(ch->wiznet, wiznet_table[flag].flag);
	return;
    } else {
	sprintf(buf, "You will now see %s on wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf, ch);
	SET_BIT(ch->wiznet, wiznet_table[flag].flag);
	return;
    }

}

void
wiznet(char *string, CHAR_DATA * ch, OBJ_DATA * obj,
       long flag, long flag_skip, int min_level)
{
    DESCRIPTOR_DATA *d;

    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->connected == CON_PLAYING && IS_IMMORTAL(d->character)
	    && IS_SET(d->character->wiznet, WIZ_ON)
	    && (!flag || IS_SET(d->character->wiznet, flag))
	    && (!flag_skip || !IS_SET(d->character->wiznet, flag_skip))
	    && get_trust(d->character) >= min_level && d->character != ch) {
	    if (IS_SET(d->character->wiznet, WIZ_PREFIX))
		send_to_char("--> ", d->character);
	    act_new(string, d->character, obj, ch, TO_CHAR, POS_DEAD);
	}
    }

    return;
}


/* equips a character */
void do_outfit(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    int i, sn, vnum;
    /* Thierry 960409 made level to 10 was 5 (for a 100 level) */
    if (ch->level > 10 || IS_NPC(ch)) {
	send_to_char("Find it yourself!\n\r", ch);
	return;
    }

    if ((obj = get_eq_char(ch, WEAR_LIGHT)) == NULL) {
	obj = create_object(get_obj_index(15006), 52);

	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch) / 2) {
	    send_to_char("You are carrying all you can carry! \n\r", ch);
	    return;
	}
	obj->cost = 0;
	obj_to_char(obj, ch);
	equip_char(ch, obj, WEAR_LIGHT);
    }

    if ((obj = get_eq_char(ch, WEAR_BODY)) == NULL) {
	obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0);
	obj->cost = 0;
	obj_to_char(obj, ch);
	equip_char(ch, obj, WEAR_BODY);
    }


    /* do the weapon thing */
    if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
	sn = 0;
	vnum = OBJ_VNUM_SCHOOL_SWORD;	/* just in case! */

	for (i = 0; weapon_table[i].name != NULL; i++) {
	    if (ch->pcdata->learned[sn] <
		ch->pcdata->learned[*weapon_table[i].gsn]) {
		sn = *weapon_table[i].gsn;
		vnum = weapon_table[i].vnum;
	    }
	}

	obj = create_object(get_obj_index(vnum), 0);
	obj_to_char(obj, ch);
	equip_char(ch, obj, WEAR_WIELD);
    }

    if (((obj = get_eq_char(ch, WEAR_WIELD)) == NULL
	 || !IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS))
	&& (obj = get_eq_char(ch, WEAR_SHIELD)) == NULL) {
	obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0);
	obj->cost = 0;
	obj_to_char(obj, ch);
	equip_char(ch, obj, WEAR_SHIELD);
    }

    send_to_char("You have been equipped by the Gods.\n\r", ch);
}


/* RT nochannels command, for those spammers */
void do_nochannels(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Nochannel whom?", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (IS_SET(victim->comm, COMM_NOCHANNELS)) {
	REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
	send_to_char("The gods have restored your channel priviliges.\n\r",
		     victim);
	send_to_char("NOCHANNELS removed.\n\r", ch);
	sprintf(buf, "$N restores channels to %s", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    } else {
	SET_BIT(victim->comm, COMM_NOCHANNELS);
	send_to_char("The gods have revoked your channel priviliges.\n\r",
		     victim);
	send_to_char("NOCHANNELS set.\n\r", ch);
	sprintf(buf, "$N revokes %s's channels.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    return;
}


void do_smote(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *vch;
    char *letter, *name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;

    if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
	send_to_char("You can't show your emotions.\n\r", ch);
	return;
    }

    if (argument[0] == '\0') {
	send_to_char("Emote what?\n\r", ch);
	return;
    }

    if (strstr(argument, ch->name) == NULL) {
	send_to_char("You must include your name in an smote.\n\r", ch);
	return;
    }

    send_to_char(argument, ch);
    send_to_char("\n\r", ch);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument, vch->name)) == NULL) {
	    send_to_char(argument, vch);
	    send_to_char("\n\r", vch);
	    continue;
	}

	strcpy(temp, argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
	last[0] = '\0';
	name = vch->name;

	for (; *letter != '\0'; letter++) {
	    if (*letter == '\'' && matches == strlen(vch->name)) {
		strcat(temp, "r");
		continue;
	    }

	    if (*letter == 's' && matches == strlen(vch->name)) {
		matches = 0;
		continue;
	    }

	    if (matches == strlen(vch->name)) {
		matches = 0;
	    }

	    if (*letter == *name) {
		matches++;
		name++;
		if (matches == strlen(vch->name)) {
		    strcat(temp, "you");
		    last[0] = '\0';
		    name = vch->name;
		    continue;
		}
		strncat(last, letter, 1);
		continue;
	    }

	    matches = 0;
	    strcat(temp, last);
	    strncat(temp, letter, 1);
	    last[0] = '\0';
	    name = vch->name;
	}

	send_to_char(temp, vch);
	send_to_char("\n\r", vch);
    }

    return;
}

void do_bamfin(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC(ch)) {
	smash_tilde(argument);

	if (argument[0] == '\0') {
	    sprintf(buf, "Your poofin is %s\n\r", ch->pcdata->bamfin);
	    send_to_char(buf, ch);
	    return;
	}

	if (strstr(argument, ch->name) == NULL) {
	    send_to_char("You must include your name.\n\r", ch);
	    return;
	}

	free_string(ch->pcdata->bamfin);
	ch->pcdata->bamfin = str_dup(argument);

	sprintf(buf, "Your poofin is now %s\n\r", ch->pcdata->bamfin);
	send_to_char(buf, ch);
    }
    return;
}



void do_bamfout(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC(ch)) {
	smash_tilde(argument);

	if (argument[0] == '\0') {
	    sprintf(buf, "Your poofout is %s\n\r", ch->pcdata->bamfout);
	    send_to_char(buf, ch);
	    return;
	}

	if (strstr(argument, ch->name) == NULL) {
	    send_to_char("You must include your name.\n\r", ch);
	    return;
	}

	free_string(ch->pcdata->bamfout);
	ch->pcdata->bamfout = str_dup(argument);

	sprintf(buf, "Your poofout is now %s\n\r", ch->pcdata->bamfout);
	send_to_char(buf, ch);
    }
    return;
}



void do_deny(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Deny whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char("You are denied access!\n\r", victim);
    sprintf(buf, "$N denies access to %s", victim->name);
    wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    send_to_char("OK.\n\r", ch);
    save_char_obj(victim);
    stop_fighting(victim, TRUE);
/*    do_quit(victim, ""); */ /* Gamecrasher! */
    /* Get rid of the player or mob */

    /* I know they won't have a desc, but I'm paranoid anyway. */

    if (IS_NPC(victim)) {
	extract_char(victim, TRUE);
	return;
    } else {
	/* Log this */
	sprintf(buf, "%s denied by %s.", victim->name, ch->name);
	log_string(buf);

	/* Lose the loser */
	d = victim->desc;
	extract_char(victim, TRUE);
	if (d != NULL) {
	    close_socket(d);
	}
    }

    return;
}



void do_disconnect(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Disconnect whom?\n\r", ch);
	return;
    }

    if (is_number(arg)) {
	int desc;

	desc = atoi(arg);
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if (d->descriptor == desc) {
		close_socket(d);
		send_to_char("Ok.\n\r", ch);
		return;
	    }
	}
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->desc == NULL) {
	act("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
	return;
    }

    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d == victim->desc) {
	    close_socket(d);
	    send_to_char("Ok.\n\r", ch);
	    return;
	}
    }

    bug("Do_disconnect: desc not found.", 0);
    send_to_char("Descriptor not found!\n\r", ch);
    return;
}



void do_pardon(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
	send_to_char
	    ("Syntax: pardon <character> <killer|thief|pkset>.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "killer")) {
	if (IS_SET(victim->act, PLR_KILLER)) {
	    REMOVE_BIT(victim->act, PLR_KILLER);
	    send_to_char("Killer flag removed.\n\r", ch);
	    send_to_char("You are no longer a KILLER.\n\r", victim);
	}
	return;
    }

    if (!str_cmp(arg2, "thief")) {
	if (IS_SET(victim->act, PLR_THIEF)) {
	    REMOVE_BIT(victim->act, PLR_THIEF);
	    send_to_char("Thief flag removed.\n\r", ch);
	    send_to_char("You are no longer a THIEF.\n\r", victim);
	}
	return;
    }

    if (!str_cmp(arg2, "pkset")) {
	if (victim->pcdata->pkset) {
	    victim->pcdata->pkset = 0;
	    ch->max_hit = ch->max_hit - 100;
	    ch->max_mana = ch->max_mana - 100;
	    ch->max_move = ch->max_move - 100;

	    ch->pcdata->perm_hit = ch->pcdata->perm_hit - 100;
	    ch->pcdata->perm_mana = ch->pcdata->perm_mana - 100;
	    ch->pcdata->perm_move = ch->pcdata->perm_move - 100;

	    send_to_char("PKset flag removed.\n\r", ch);
	    send_to_char("You are no longer a PKset.\n\r", victim);
	}
	return;
    }

    send_to_char("Syntax: pardon <character> <killer|thief|pkset>.\n\r",
		 ch);
    return;
}



void do_echo(CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
	send_to_char("Global echo what?\n\r", ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING) {
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("global> ", d->character);
	    send_to_char(argument, d->character);
	    send_to_char("\n\r", d->character);
	}
    }

    return;
}



void do_recho(CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
	send_to_char("Local echo what?\n\r", ch);

	return;
    }

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING
	    && d->character->in_room == ch->in_room) {
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("local> ", d->character);
	    send_to_char(argument, d->character);
	    send_to_char("\n\r", d->character);
	}
    }

    return;
}


void do_zecho(CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
	send_to_char("Zone echo what?\n\r", ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING
	    && d->character->in_room != NULL && ch->in_room != NULL
	    && d->character->in_room->area == ch->in_room->area) {
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("zone> ", d->character);
	    send_to_char(argument, d->character);
	    send_to_char("\n\r", d->character);
	}
    }
}

void do_pecho(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (argument[0] == '\0' || arg[0] == '\0') {
	send_to_char("Personal echo what?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("Target not found.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
	send_to_char("personal> ", victim);

    send_to_char(argument, victim);
    send_to_char("\n\r", victim);
    send_to_char("personal> ", ch);
    send_to_char(argument, ch);
    send_to_char("\n\r", ch);
}


ROOM_INDEX_DATA *find_location(CHAR_DATA * ch, char *arg)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if (is_number(arg))
	return get_room_index(atoi(arg));

    if ((victim = get_char_world(ch, arg)) != NULL)
	return victim->in_room;

    if ((obj = get_obj_world(ch, arg)) != NULL)
	return obj->in_room;

    return NULL;
}



void do_transfer(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0') {
	send_to_char("Transfer whom (and where)?\n\r", ch);
	return;
    }
    if (!str_cmp(arg1, "quest")) {
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if (d->connected == CON_PLAYING
		&& d->character != ch
		&& d->character->in_room != NULL
		&& can_see(ch, d->character)
		&& IS_SET(d->character->act, PLR_QUESTOR)) {
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "%s %s", d->character->name, arg2);
		do_transfer(ch, buf);
	    }
	}
	return;
    }

    if (!str_cmp(arg1, "all")) {
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if (d->connected == CON_PLAYING
		&& d->character != ch
		&& d->character->in_room != NULL
		&& can_see(ch, d->character)) {
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "%s %s", d->character->name, arg2);
		do_transfer(ch, buf);
	    }
	}
	return;
    }

    /*
       * Thanks to Grodyn for the optional location parameter.
     */
    if (arg2[0] == '\0') {
	location = ch->in_room;
    } else {
	if ((location = find_location(ch, arg2)) == NULL) {
	    send_to_char("No such location.\n\r", ch);
	    return;
	}

	if (!is_room_owner(ch, location) && room_is_private(location)
	    && get_trust(ch) < MAX_LEVEL) {
	    send_to_char("That room is private right now.\n\r", ch);
	    return;
	}
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->in_room == NULL) {
	send_to_char("They are in limbo.\n\r", ch);
	return;
    }

    if (victim->fighting != NULL)
	stop_fighting(victim, TRUE);
    act("$n shimmers and vanishes.", victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n arrives from in a pop from unknown means.", victim, NULL, NULL,
	TO_ROOM);
    if (ch != victim)
	act("$n has transferred you.", ch, NULL, victim, TO_VICT);
    do_look(victim, "auto");
    send_to_char("Ok.\n\r", ch);
}



void do_at(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
	send_to_char("At where what?\n\r", ch);
	return;
    }

    if ((location = find_location(ch, arg)) == NULL) {
	send_to_char("No such location.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch, location) && room_is_private(location)
	&& get_trust(ch) < MAX_LEVEL) {
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room(ch);
    char_to_room(ch, location);
    interpret(ch, argument);

    /*
       * See if 'ch' still exists before continuing!
       * Handles 'at XXXX quit' case.
     */
    for (wch = char_list; wch != NULL; wch = wch->next) {
	if (wch == ch) {
	    char_from_room(ch);
	    char_to_room(ch, original);
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto(CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;

    if (argument[0] == '\0') {
	send_to_char("Goto where?\n\r", ch);
	return;
    }

    if ((location = find_location(ch, argument)) == NULL) {
	send_to_char("No such location.\n\r", ch);
	return;
    }

    count = 0;
    for (rch = location->people; rch != NULL; rch = rch->next_in_room)
	count++;

    if (!is_room_owner(ch, location) && room_is_private(location)
	&& (count > 1 || get_trust(ch) < MAX_LEVEL)) {
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    if (ch->fighting != NULL)
	stop_fighting(ch, TRUE);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
	if (get_trust(rch) >= ch->invis_level) {
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
	    else
		act("$n leaves in a swirling mist.", ch, NULL, rch,
		    TO_VICT);
	}
    }

    char_from_room(ch);
    char_to_room(ch, location);


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
	if (get_trust(rch) >= ch->invis_level) {
	    if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
		act("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
	    else
		act("$n appears in a swirling mist.", ch, NULL, rch,
		    TO_VICT);
	}
    }

    do_look(ch, "auto");
    return;
}

void do_violate(CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;

    if (argument[0] == '\0') {
	send_to_char("Goto where?\n\r", ch);
	return;
    }

    if ((location = find_location(ch, argument)) == NULL) {
	send_to_char("No such location.\n\r", ch);
	return;
    }

    if (!room_is_private(location)) {
	send_to_char("That room isn't private, use goto.\n\r", ch);
	return;
    }

    if (ch->fighting != NULL)
	stop_fighting(ch, TRUE);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
	if (get_trust(rch) >= ch->invis_level) {
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
	    else
		act("$n leaves in a swirling mist.", ch, NULL, rch,
		    TO_VICT);
	}
    }

    char_from_room(ch);
    char_to_room(ch, location);


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
	if (get_trust(rch) >= ch->invis_level) {
	    if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
		act("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
	    else
		act("$n appears in a swirling mist.", ch, NULL, rch,
		    TO_VICT);
	}
    }

    do_look(ch, "auto");
    return;
}

/* RT to replace the 3 stat commands */

void do_stat(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    string = one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  stat <name>\n\r", ch);
	send_to_char("  stat obj <name>\n\r", ch);
	send_to_char("  stat mob <name>\n\r", ch);
	send_to_char("  stat room <number>\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "room")) {
	do_rstat(ch, string);
	return;
    }

    if (!str_cmp(arg, "obj")) {
	do_ostat(ch, string);
	return;
    }

    if (!str_cmp(arg, "char") || !str_cmp(arg, "mob")) {
	do_mstat(ch, string);
	return;
    }

    /* do it the old way */


    obj = get_obj_world(ch, argument);
    if (obj != NULL) {
	do_ostat(ch, argument);
	return;
    }

    victim = get_char_world(ch, argument);
    if (victim != NULL) {
	do_mstat(ch, argument);
	return;
    }

    location = find_location(ch, argument);
    if (location != NULL) {
	do_rstat(ch, argument);
	return;
    }

    send_to_char("Nothing by that name found anywhere.\n\r", ch);
}





void do_rstat(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument(argument, arg);
    location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
    if (location == NULL) {
	send_to_char("No such location.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch, location) && ch->in_room != location
	&& room_is_private(location) && !IS_TRUSTED(ch, IMPLEMENTOR)) {
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    sprintf(buf, "Name: '%s'\n\rArea: '%s'\n\r",
	    location->name, location->area->name);
    send_to_char(buf, ch);

    sprintf(buf,
	    "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	    location->vnum,
	    location->sector_type,
	    location->light, location->heal_rate, location->mana_rate);
    send_to_char(buf, ch);

    sprintf(buf,
	    "Room flags: %s.\n\rDescription:\n\r%s",
	    room_bit_name(location->room_flags), location->description);
    send_to_char(buf, ch);

    if (location->extra_descr != NULL) {
	EXTRA_DESCR_DATA *ed;

	send_to_char("Extra description keywords: '", ch);
	for (ed = location->extra_descr; ed; ed = ed->next) {
	    send_to_char(ed->keyword, ch);
	    if (ed->next != NULL)
		send_to_char(" ", ch);
	}
	send_to_char("'.\n\r", ch);
    }

    send_to_char("Characters:", ch);
    for (rch = location->people; rch; rch = rch->next_in_room) {
	if (can_see(ch, rch)) {
	    send_to_char(" ", ch);
	    one_argument(rch->name, buf);
	    send_to_char(buf, ch);
	}
    }

    send_to_char(".\n\rObjects:   ", ch);
    for (obj = location->contents; obj; obj = obj->next_content) {
	send_to_char(" ", ch);
	one_argument(obj->name, buf);
	send_to_char(buf, ch);
    }
    send_to_char(".\n\r", ch);

    for (door = 0; door <= 5; door++) {
	EXIT_DATA *pexit;

	if ((pexit = location->exit[door]) != NULL) {
	    sprintf(buf,
		    "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",
		    door,
		    (pexit->u1.to_room ==
		     NULL ? -1 : pexit->u1.to_room->vnum), pexit->key,
		    pexit->exit_info, pexit->keyword,
		    pexit->description[0] !=
		    '\0' ? pexit->description : "(none).\n\r");
	    send_to_char(buf, ch);
	}
    }

    return;
}



void do_ostat(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    int caliber = 0, ammotype = 0, range = 0;	// Needed for guns - Kevin

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Stat what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_world(ch, argument)) == NULL) {
	send_to_char("Nothing like that in hell, earth, or heaven.\n\r",
		     ch);
	return;
    }

    sprintf(buf, "Name(s): %s\n\r", obj->name);
    send_to_char(buf, ch);
    if (obj->owner != NULL) {
	sprintf(buf, "Owner: %s\n\r", obj->owner);
	send_to_char(buf, ch);
    }

    if (obj->iname != NULL) {
	sprintf(buf, "Loaded by: %s\n\r", obj->iname);
	send_to_char(buf, ch);
    }

    if (obj->clone != NULL) {
	sprintf(buf, "Cloned by: %s\n\r", obj->clone);
	send_to_char(buf, ch);
    }

    sprintf(buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	    obj->pIndexData->vnum,
	    obj->pIndexData->new_format ? "new" : "old",
	    item_type_name(obj), obj->pIndexData->reset_num);
    send_to_char(buf, ch);

    sprintf(buf, "Short description: %s\n\rLong description: %s\n\r",
	    obj->short_descr, obj->description);
    send_to_char(buf, ch);

    sprintf(buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	    wear_bit_name(obj->wear_flags),
	    extra_bit_name(obj->extra_flags));
    send_to_char(buf, ch);

    sprintf(buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	    1, get_obj_number(obj),
	    obj->weight, get_obj_weight(obj), get_true_weight(obj));
    send_to_char(buf, ch);

    sprintf(buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	    obj->level, obj->cost, obj->condition, obj->timer);
    send_to_char(buf, ch);

    sprintf(buf,
	    "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	    obj->in_room == NULL ? 0 : obj->in_room->vnum,
	    obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
	    obj->carried_by == NULL ? "(none)" :
	    can_see(ch, obj->carried_by) ? obj->carried_by->name
	    : "someone", obj->wear_loc);
    send_to_char(buf, ch);

    sprintf(buf, "Values: %ld %ld %ld %ld %ld\n\r",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]);
    send_to_char(buf, ch);

    /* now give out vital statistics as per identify */

    switch (obj->item_type) {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf(buf, "Level %ld spells of:", obj->value[0]);
	send_to_char(buf, ch);

	if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[1]].name, ch);
	    send_to_char("'", ch);
	}

	if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[2]].name, ch);
	    send_to_char("'", ch);
	}

	if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[3]].name, ch);
	    send_to_char("'", ch);
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[4]].name, ch);
	    send_to_char("'", ch);
	}

	send_to_char(".\n\r", ch);
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	sprintf(buf, "Has %ld(%ld) charges of level %ld",
		obj->value[1], obj->value[2], obj->value[0]);
	send_to_char(buf, ch);

	if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[3]].name, ch);
	    send_to_char("'", ch);
	}

	send_to_char(".\n\r", ch);
	break;

    case ITEM_DRINK_CON:
	sprintf(buf, "It holds %s-colored %s.\n\r",
		liq_table[obj->value[2]].liq_color,
		liq_table[obj->value[2]].liq_name);
	send_to_char(buf, ch);
	break;


    case ITEM_WEAPON:
	send_to_char("Weapon type is ", ch);
	switch (obj->value[0]) {
	case (WEAPON_EXOTIC):
	    send_to_char("exotic\n\r", ch);
	    break;
	case (WEAPON_SWORD):
	    send_to_char("sword\n\r", ch);
	    break;
	case (WEAPON_DAGGER):
	    send_to_char("dagger\n\r", ch);
	    break;
	case (WEAPON_SPEAR):
	    send_to_char("spear/staff\n\r", ch);
	    break;
	case (WEAPON_MACE):
	    send_to_char("mace/club\n\r", ch);
	    break;
	case (WEAPON_AXE):
	    send_to_char("axe\n\r", ch);
	    break;
	case (WEAPON_FLAIL):
	    send_to_char("flail\n\r", ch);
	    break;
	case (WEAPON_WHIP):
	    send_to_char("whip\n\r", ch);
	    break;
	case (WEAPON_POLEARM):
	    send_to_char("polearm\n\r", ch);
	    break;
	default:
	    send_to_char("unknown\n\r", ch);
	    break;
	}
	if (obj->pIndexData->new_format)
	    sprintf(buf, "Damage is %ldd%ld (average %ld)\n\r",
		    obj->value[1], obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	else
	    sprintf(buf, "Damage is %ld to %ld (average %ld)\n\r",
		    obj->value[1], obj->value[2],
		    (obj->value[1] + obj->value[2]) / 2);
	send_to_char(buf, ch);

	/* Valorath has shit for brains. */

	if (obj->value[3] > DAM_MAX) {
	    sprintf(buf, "Damage noun is INVALID!.\n\r");
	} else {
	    sprintf(buf, "Damage noun is %s.\n\r",
		    attack_table[obj->value[3]].noun);
	}
	send_to_char(buf, ch);

	if (obj->value[4]) {	/* weapon flags */
	    sprintf(buf, "Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
	    send_to_char(buf, ch);
	}
	break;

    case ITEM_ARMOR:
	sprintf(buf,
		"Armor class is %ld pierce, %ld bash, %ld slash, and %ld vs. magic\n\r",
		obj->value[0], obj->value[1], obj->value[2],
		obj->value[3]);
	send_to_char(buf, ch);
	break;

    case ITEM_SADDLE:
    case ITEM_CONTAINER:
	sprintf(buf, "Capacity: %ld#  Maximum weight: %ld#  flags: %s\n\r",
		obj->value[0], obj->value[3],
		cont_bit_name(obj->value[1]));
	send_to_char(buf, ch);
	if (obj->value[4] != 100) {
	    sprintf(buf, "Weight multiplier: %ld%%\n\r", obj->value[4]);
	    send_to_char(buf, ch);
	}
	break;
    case ITEM_PISTOL:
    case ITEM_SMG:
    case ITEM_SHOTGUN:
    case ITEM_RIFLE:
    case ITEM_HEAVYGUN:
    case ITEM_ENERGYGUN:
	caliber = obj->value[0];
	ammotype = obj->value[1];
	range = gun_range(obj);
	
	sprintf(buf, "Range: %d\n\r", range);
	send_to_char(buf, ch);
	sprintf(buf, "Caliber: %s\n\r",
	    caliber < BULLET_CALIBER ? ammo_table[caliber].caliber : "(null)");
	send_to_char(buf, ch);
	sprintf(buf, "Ammunition type: %s\n\r",
	    caliber < BULLET_CALIBER ? 
	    (ammotype < BULLET_TYPE ?
	    ammo_table[caliber].ammotype[ammotype] : "(null)") : "(null)");
	send_to_char(buf, ch);
	sprintf(buf, "Ammo count: %ld\n\r", obj->value[2]);
	send_to_char(buf, ch);
	if (caliber < BULLET_CALIBER && ammotype < BULLET_TYPE) {
	    sprintf(buf, "Base damage: %d-%d\n\r",
		ammo_table[caliber].dam[ammotype][BULLET_MIN],
		ammo_table[caliber].dam[ammotype][BULLET_MAX]);
	} else {
	    sprintf(buf, "Base damage: (null)\n\r");
	}
	send_to_char(buf, ch);
	sprintf(buf, "Damage modifier: %ld\n\r", obj->value[3]);
	send_to_char(buf, ch);
	sprintf(buf, "Max ammo capacity: %ld\n\r", obj->value[4]);
	send_to_char(buf, ch);
	break;
    case ITEM_AMMO:
	caliber = obj->value[0];
	ammotype = obj->value[1];
	sprintf(buf, "Caliber: %s\n\r",
	    caliber < BULLET_CALIBER ? ammo_table[caliber].caliber : "(null)");
	send_to_char(buf, ch);
	sprintf(buf, "Ammunition type: %s\n\r",
	    caliber < BULLET_CALIBER ?
	    (ammotype < BULLET_TYPE ?
	    ammo_table[caliber].ammotype[ammotype] : "(null)") : "(null)");
	send_to_char(buf, ch);
	sprintf(buf, "Ammo count: %ld\n\r", obj->value[2]);
	send_to_char(buf, ch);
	break;
    case ITEM_CLIP:
	caliber = obj->value[0];
	ammotype = obj->value[1];
	sprintf(buf, "Caliber: %s\n\r",
	    caliber < BULLET_CALIBER ? ammo_table[caliber].caliber : "(null)");
	send_to_char(buf, ch);
	sprintf(buf, "Ammunition type: %s\n\r",
	    caliber < BULLET_CALIBER ?
	    (ammotype < BULLET_TYPE ?
	    ammo_table[caliber].ammotype[ammotype] : "(null)") : "(null)");
	send_to_char(buf, ch);
	sprintf(buf, "Ammo count: %ld\n\r", obj->value[2]);
	send_to_char(buf, ch);
	sprintf(buf, "Ammo max: %ld\n\r", obj->value[3]);
	send_to_char(buf, ch);
	sprintf(buf, "Reloads into: %s\n\r",
	    get_obj_index(obj->value[4]) != NULL ?
	    get_obj_index(obj->value[4])->short_descr : "(null)");
	send_to_char(buf, ch);
	break;
    }


    if (obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL) {
	EXTRA_DESCR_DATA *ed;

	send_to_char("Extra description keywords: '", ch);

	for (ed = obj->extra_descr; ed != NULL; ed = ed->next) {
	    send_to_char(ed->keyword, ch);
	    if (ed->next != NULL)
		send_to_char(" ", ch);
	}

	for (ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next) {
	    send_to_char(ed->keyword, ch);
	    if (ed->next != NULL)
		send_to_char(" ", ch);
	}

	send_to_char("'\n\r", ch);
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
	sprintf(buf, "Affects %s by %d, level %d",
		affect_loc_name(paf->location), paf->modifier, paf->level);
	send_to_char(buf, ch);
	if (paf->duration > -1)
	    sprintf(buf, ", %d hours.\n\r", paf->duration);
	else
	    sprintf(buf, ".\n\r");
	send_to_char(buf, ch);
	if (paf->bitvector) {
	    switch (paf->where) {
	    case TO_AFFECTS:
		sprintf(buf, "Adds %s affect.\n",
			affect_bit_name(paf->bitvector));
		break;
	    case TO_WEAPON:
		sprintf(buf, "Adds %s weapon flags.\n",
			weapon_bit_name(paf->bitvector));
		break;
	    case TO_OBJECT:
		sprintf(buf, "Adds %s object flag.\n",
			extra_bit_name(paf->bitvector));
		break;
	    case TO_IMMUNE:
		sprintf(buf, "Adds immunity to %s.\n",
			imm_bit_name(paf->bitvector));
		break;
	    case TO_RESIST:
		sprintf(buf, "Adds resistance to %s.\n\r",
			imm_bit_name(paf->bitvector));
		break;
	    case TO_VULN:
		sprintf(buf, "Adds vulnerability to %s.\n\r",
			imm_bit_name(paf->bitvector));
		break;
	    default:
		sprintf(buf, "Unknown bit %d: %d\n\r",
			paf->where, paf->bitvector);
		break;
	    }
	    send_to_char(buf, ch);
	}
    }

    if (!obj->enchanted)
	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
	    sprintf(buf, "Affects %s by %d, level %d.\n\r",
		    affect_loc_name(paf->location), paf->modifier,
		    paf->level);
	    send_to_char(buf, ch);
	    if (paf->bitvector) {
		switch (paf->where) {
		case TO_AFFECTS:
		    sprintf(buf, "Adds %s affect.\n",
			    affect_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf, "Adds %s object flag.\n",
			    extra_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    sprintf(buf, "Adds immunity to %s.\n",
			    imm_bit_name(paf->bitvector));
		    break;
		case TO_RESIST:
		    sprintf(buf, "Adds resistance to %s.\n\r",
			    imm_bit_name(paf->bitvector));
		    break;
		case TO_VULN:
		    sprintf(buf, "Adds vulnerability to %s.\n\r",
			    imm_bit_name(paf->bitvector));
		    break;
		default:
		    sprintf(buf, "Unknown bit %d: %d\n\r",
			    paf->where, paf->bitvector);
		    break;
		}
		send_to_char(buf, ch);
	    }
	}

    return;
}



void do_mstat(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Stat whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    sprintf(buf, "Name: %s\n\r", victim->name);
    send_to_char(buf, ch);

    sprintf(buf,
	    "Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	    IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	    IS_NPC(victim) ? victim->
	    pIndexData->new_format ? "new" : "old" : "pc",
	    race_table[victim->race].name,
	    IS_NPC(victim) ? victim->group : 0,
	    sex_table[victim->sex].name,
	    victim->in_room == NULL ? 0 : victim->in_room->vnum);
    send_to_char(buf, ch);

    if (IS_NPC(victim)) {
	sprintf(buf, "Count: %d  Killed: %d\n\r",
		victim->pIndexData->count, victim->pIndexData->killed);
	send_to_char(buf, ch);
    }

    sprintf(buf,
	    "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	    victim->perm_stat[STAT_STR],
	    get_curr_stat(victim, STAT_STR),
	    victim->perm_stat[STAT_INT],
	    get_curr_stat(victim, STAT_INT),
	    victim->perm_stat[STAT_WIS],
	    get_curr_stat(victim, STAT_WIS),
	    victim->perm_stat[STAT_DEX],
	    get_curr_stat(victim, STAT_DEX),
	    victim->perm_stat[STAT_CON], get_curr_stat(victim, STAT_CON));
    send_to_char(buf, ch);

    sprintf(buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d\n\r",
	    victim->hit, victim->max_hit,
	    victim->mana, victim->max_mana, victim->move,
	    victim->max_move);
    send_to_char(buf, ch);

    sprintf(buf,
	    "Practices: %d    Trains: %d		AQpoints: %d	RPoints: %d\n\r",
	    IS_NPC(ch) ? 0 : victim->practice,
	    IS_NPC(ch) ? 0 : victim->train, victim->questpoints,
	    victim->rp_points);
    send_to_char(buf, ch);

    sprintf(buf,
	    "Lv: %d  Class: %s  Align: %d  Gold: %ld  Silver: %ld  Exp: %d\n\r",
	    victim->level,
	    IS_NPC(victim) ? "mobile" : class_table[victim->class].name,
	    victim->alignment, victim->gold, victim->silver, victim->exp);
    send_to_char(buf, ch);

    sprintf(buf, "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim, AC_PIERCE), GET_AC(victim, AC_BASH),
	    GET_AC(victim, AC_SLASH), GET_AC(victim, AC_EXOTIC));
    send_to_char(buf, ch);

    sprintf(buf,
	    "Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	    GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	    size_table[victim->size].name,
	    position_table[victim->position].name, victim->wimpy);
    send_to_char(buf, ch);

    if (IS_NPC(victim) && victim->pIndexData->new_format) {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
		victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
		attack_table[victim->dam_type].noun);
	send_to_char(buf, ch);
    }
    sprintf(buf, "Fighting: %s\n\r",
	    victim->fighting ? victim->fighting->name : "(none)");
    send_to_char(buf, ch);

    if (!IS_NPC(victim)) {
	sprintf(buf,
		"Thirst: %d  Hunger: %d  Full: %d  Drunk: %d\n\r",
		victim->pcdata->condition[COND_THIRST],
		victim->pcdata->condition[COND_HUNGER],
		victim->pcdata->condition[COND_FULL],
		victim->pcdata->condition[COND_DRUNK]);
	send_to_char(buf, ch);
    }

    sprintf(buf, "Carry number: %d  Carry weight: %ld\n\r",
	    victim->carry_number, get_carry_weight(victim) / 10);
    send_to_char(buf, ch);


    if (!IS_NPC(victim)) {
	sprintf(buf,
		"Age: %d  Played: %d  Last Level: %d  Timer: %d  Jail Timer: %d\n\r",
		get_age(victim),
		(int) (victim->played + current_time -
		       victim->logon) / 3600, victim->pcdata->last_level,
		victim->timer, victim->jailtime);
	send_to_char(buf, ch);
    }

    sprintf(buf, "Act: %s\n\r", act_bit_name(victim->act));
    send_to_char(buf, ch);
    sprintf(buf, "Epf: %s\n\r", act_bit_name(victim->epf));
    send_to_char(buf, ch);

    if (victim->comm) {
	sprintf(buf, "Comm: %s\n\r", comm_bit_name(victim->comm));
	send_to_char(buf, ch);
    }

    if (IS_NPC(victim) && victim->off_flags) {
	sprintf(buf, "Offense: %s\n\r", off_bit_name(victim->off_flags));
	send_to_char(buf, ch);
    }

    if (victim->imm_flags) {
	sprintf(buf, "Immune: %s\n\r", imm_bit_name(victim->imm_flags));
	send_to_char(buf, ch);
    }

    if (victim->res_flags) {
	sprintf(buf, "Resist: %s\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf, ch);
    }

    if (victim->vuln_flags) {
	sprintf(buf, "Vulnerable: %s\n\r",
		imm_bit_name(victim->vuln_flags));
	send_to_char(buf, ch);
    }

    sprintf(buf, "Form: %s\n\rParts: %s\n\r",
	    form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf, ch);

    if (victim->affected_by) {
	sprintf(buf, "Affected by %s\n\r",
		affect_bit_name(victim->affected_by));
	send_to_char(buf, ch);
    }

    sprintf(buf,
	    "Master: %s  Leader: %s  \n\rPet: %s (Horse: %s Mounted: %s )Quest: %s\n\r",
	    victim->master ? victim->master->name : "(none)",
	    victim->leader ? victim->leader->name : "(none)",
	    victim->pet ? victim->pet->name : "(none)",
	    victim->horse ? victim->horse->name : "(none)",
	    victim->mount > 0 ? "Yes" : "No",
	    victim->quest ? victim->quest->name : "(none)");
    send_to_char(buf, ch);

    /* OLC - Suzuran */
    if (!IS_NPC(victim))
      {
        sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );
        send_to_char( buf, ch );
      }

    sprintf(buf, "Short description: %s\n\rLong  description: %s",
	    victim->short_descr,
	    victim->long_descr[0] !=
	    '\0' ? victim->long_descr : "(none)\n\r");
    send_to_char(buf, ch);

    if (IS_NPC(victim) && victim->spec_fun != 0) {
	sprintf(buf, "Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf, ch);
    }

    for (paf = victim->affected; paf != NULL; paf = paf->next) {
	sprintf(buf,
		"Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
		skill_table[(int) paf->type].name,
		affect_loc_name(paf->location),
		paf->modifier,
		paf->duration, affect_bit_name(paf->bitvector),
		paf->level);
	send_to_char(buf, ch);
    }

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  vnum obj <name>\n\r", ch);
	send_to_char("  vnum mob <name>\n\r", ch);
	send_to_char("  vnum skill <skill or spell>\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "obj")) {
	do_ofind(ch, string);
	return;
    }

    if (!str_cmp(arg, "mob") || !str_cmp(arg, "char")) {
	do_mfind(ch, string);
	return;
    }

    if (!str_cmp(arg, "skill") || !str_cmp(arg, "spell")) {
	do_slookup(ch, string);
	return;
    }
    /* do both */
    do_mfind(ch, argument);
    do_ofind(ch, argument);
}


void do_mfind(CHAR_DATA * ch, char *argument)
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Find whom?\n\r", ch);
	return;
    }

    fAll = FALSE;		/* !str_cmp( arg, "all" ); */
    found = FALSE;
    nMatch = 0;

    /*
       * Yeah, so iterating over all vnum's takes 10,000 loops.
       * Get_mob_index is fast, and I don't feel like threading another link.
       * Do you?
       * -- Furey
     */
    for (vnum = 0; nMatch < top_mob_index; vnum++) {
	if ((pMobIndex = get_mob_index(vnum)) != NULL) {
	    nMatch++;
	    if (fAll || is_name(argument, pMobIndex->player_name)) {
		found = TRUE;
		sprintf(buf, "[%5d] %s\n\r",
			pMobIndex->vnum, pMobIndex->short_descr);
		send_to_char(buf, ch);
	    }
	}
    }

    if (!found)
	send_to_char("No mobiles by that name.\n\r", ch);

    return;
}



void do_ofind(CHAR_DATA * ch, char *argument)
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Find what?\n\r", ch);
	return;
    }

    fAll = FALSE;		/* !str_cmp( arg, "all" ); */
    found = FALSE;
    nMatch = 0;

    /*
       * Yeah, so iterating over all vnum's takes 10,000 loops.
       * Get_obj_index is fast, and I don't feel like threading another link.
       * Do you?
       * -- Furey
     */
    for (vnum = 0; nMatch < top_obj_index; vnum++) {
	if ((pObjIndex = get_obj_index(vnum)) != NULL) {
	    nMatch++;
	    if (fAll || is_name(argument, pObjIndex->name)) {
		found = TRUE;
		sprintf(buf, "[%5d] %s\n\r",
			pObjIndex->vnum, pObjIndex->short_descr);
		send_to_char(buf, ch);
	    }
	}
    }

    if (!found)
	send_to_char("No objects by that name.\n\r", ch);

    return;
}


void do_owhere(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;


    if (argument[0] == '\0') {
	send_to_char("Find what?\n\r", ch);
	return;
    }

    buffer = new_buf();

    for (obj = object_list; obj != NULL; obj = obj->next) {
	if (!can_see_obj(ch, obj) || !is_name(argument, obj->name))
	    continue;

	found = TRUE;
	number++;

	for (in_obj = obj; in_obj->in_obj != NULL;
	     in_obj = in_obj->in_obj);

	if (in_obj->carried_by != NULL && can_see(ch, in_obj->carried_by)
	    && in_obj->carried_by->in_room != NULL)
	    sprintf(buf, "%3d) %s is carried by %s [Room %d]\n\r",
		    number, obj->short_descr, PERS(in_obj->carried_by, ch),
		    in_obj->carried_by->in_room->vnum);
	else if (in_obj->in_room != NULL
		 && can_see_room(ch, in_obj->in_room)) sprintf(buf,
							       "%3d) %s is in %s [Room %d]\n\r",
							       number,
							       obj->short_descr,
							       in_obj->in_room->name,
							       in_obj->in_room->vnum);
	/*EE96110, took away this else, so that the imms can't owhere to find us when we're wizi :) */
	/*else
	   *  sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr); */

	buf[0] = UPPER(buf[0]);
	add_buf(buffer, buf);
	if (number >= max_found)
	    break;
    }

    if (!found)
	send_to_char("Nothing like that in heaven or earth.\n\r", ch);
    else
	page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
}


void do_mwhere(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if (argument[0] == '\0') {
	DESCRIPTOR_DATA *d;

	/* show characters logged */

	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if (d->character != NULL && d->connected == CON_PLAYING
		&& d->character->in_room != NULL
		&& can_see(ch, d->character)
		&& can_see_room(ch, d->character->in_room)) {
		victim = d->character;
		count++;
		if (d->original != NULL)
		    sprintf(buf,
			    "%3d) %s (in the body of %s) is in %s [%d]\n\r",
			    count, d->original->name, victim->short_descr,
			    victim->in_room->name, victim->in_room->vnum);
		else
		    sprintf(buf, "%3d) %s is in %s [%d]\n\r", count,
			    victim->name, victim->in_room->name,
			    victim->in_room->vnum);
		add_buf(buffer, buf);
	    }
	}
	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for (victim = char_list; victim != NULL; victim = victim->next) {

	/* also check if ch can see the victim -Wiz  */

	if (victim->in_room != NULL && is_name(argument, victim->name)
	    && can_see(ch, victim)
	    && can_see_room(ch, victim->in_room) && victim->incog_level <=
	    ch->level && victim->invis_level <= ch->level) {
	    found = TRUE;
	    count++;
	    sprintf(buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		    IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		    IS_NPC(victim) ? victim->short_descr : victim->name,
		    victim->in_room->vnum, victim->in_room->name);
	    add_buf(buffer, buf);
	}
    }

    if (!found)
	act("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
    else
	page_to_char(buf_string(buffer), ch);
    free_buf(buffer);

    return;
}



void do_reboo(CHAR_DATA * ch, char *argument)
{
    send_to_char("If you want to REBOOT, spell it out.\n\r", ch);
    return;
}



void do_reboot(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;

    if (ch->invis_level < LEVEL_HERO) {
	sprintf(buf, "Reboot by %s.", ch->name);
	do_echo(ch, buf);
    }
    do_force(ch, "all save");
    do_save(ch, "");
    merc_down = TRUE;
    for (d = descriptor_list; d != NULL; d = d_next) {
	d_next = d->next;
	close_socket(d);
    }

    return;
}



void do_shutdow(CHAR_DATA * ch, char *argument)
{
    send_to_char("If you want to SHUTDOWN, spell it out.\n\r", ch);
    return;
}



void do_shutdown(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;

    if (ch->invis_level < LEVEL_HERO)
	sprintf(buf, "Shutdown by %s.", ch->name);
    append_file(ch, SHUTDOWN_FILE, buf);
    strcat(buf, "\n\r");
    if (ch->invis_level < LEVEL_HERO)
	do_echo(ch, buf);
    do_force(ch, "all save");
    do_save(ch, "");
    merc_down = TRUE;
    for (d = descriptor_list; d != NULL; d = d_next) {
	d_next = d->next;
	close_socket(d);
    }
    return;
}

void do_protect(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0') {
	send_to_char("Protect whom from snooping?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL) {
	send_to_char("You can't find them.\n\r", ch);
	return;
    }

    if (IS_SET(victim->comm, COMM_SNOOP_PROOF)) {
	act_new("$N is no longer snoop-proof.", ch, NULL, victim, TO_CHAR,
		POS_DEAD);
	send_to_char("Your snoop-proofing was just removed.\n\r", victim);
	REMOVE_BIT(victim->comm, COMM_SNOOP_PROOF);
    } else {
	act_new("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR,
		POS_DEAD);
	send_to_char("You are now immune to snooping.\n\r", victim);
	SET_BIT(victim->comm, COMM_SNOOP_PROOF);
    }
}



void do_snoop(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Snoop whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->desc == NULL) {
	send_to_char("No descriptor to snoop.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("Cancelling all snoops.\n\r", ch);
	wiznet("$N stops being such a snoop.",
	       ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust(ch));
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if (d->snoop_by == ch->desc)
		d->snoop_by = NULL;
	}
	return;
    }

    if (victim->desc->snoop_by != NULL) {
	send_to_char("Busy already.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch, victim->in_room)
	&& ch->in_room != victim->in_room
	&& room_is_private(victim->in_room)
	&& !IS_TRUSTED(ch, IMPLEMENTOR)) {
	send_to_char("That character is in a private room.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch)
	|| IS_SET(victim->comm, COMM_SNOOP_PROOF)) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (ch->desc != NULL) {
	for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by) {
	    if (d->character == victim || d->original == victim) {
		send_to_char("No snoop loops.\n\r", ch);
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf, "$N starts snooping on %s",
	    (IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust(ch));
    send_to_char("Ok.\n\r", ch);
    return;
}



void do_switch(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Switch into whom?\n\r", ch);
	return;
    }

    if (ch->desc == NULL)
	return;

    if (ch->desc->original != NULL) {
	send_to_char("You are already switched.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("Ok.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim)) {
	send_to_char("You can only switch into mobiles.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch, victim->in_room)
	&& ch->in_room != victim->in_room
	&& room_is_private(victim->in_room)
	&& !IS_TRUSTED(ch, IMPLEMENTOR)) {
	send_to_char("That character is in a private room.\n\r", ch);
	return;
    }

    if (victim->desc != NULL) {
	send_to_char("Character in use.\n\r", ch);
	return;
    }

    sprintf(buf, "$N switches into %s", victim->short_descr);
    wiznet(buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, get_trust(ch));
    ch->desc->character = victim;
    ch->desc->original = ch;
    victim->desc = ch->desc;
    ch->desc = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
	victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char
	("Notice: if your prompt reflects percentages, this will be changed\n\r",
	 victim);
    send_to_char
	("to the mob's hp, mana, and movement. If they are 0, the number will be 0.\n\r",
	 victim);
    send_to_char("Ok.\n\r", victim);
    return;
}



void do_return(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->desc == NULL)
	return;

    if (ch->desc->original == NULL) {
	send_to_char("You aren't switched.\n\r", ch);
	return;
    }

    send_to_char
	("You return to your original body. Type replay to see any missed tells.\n\r",
	 ch);
    if (ch->prompt != NULL) {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    sprintf(buf, "$N returns from %s.", ch->short_descr);
    wiznet(buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE,
	   get_trust(ch));

    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check(CHAR_DATA * ch, OBJ_DATA * obj)
{
    if (IS_TRUSTED(ch, GOD)
	|| (IS_TRUSTED(ch, IMMORTAL) && obj->level <= 20
	    && obj->cost <= 1000) || (IS_TRUSTED(ch, DEMI)
				      && obj->level <= 10
				      && obj->cost <= 500)
	|| (IS_TRUSTED(ch, ANGEL) && obj->level <= 5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch, AVATAR) && obj->level == 0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content) {
	if (obj_check(ch, c_obj)) {
	    t_obj = create_object(c_obj->pIndexData, 0);
	    clone_object(c_obj, t_obj);
	    obj_to_obj(t_obj, clone);
	    recursive_clone(ch, c_obj, t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA *obj;

    rest = one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Clone what?\n\r", ch);
	return;
    }

    if (!str_prefix(arg, "object")) {
	mob = NULL;
	obj = get_obj_here(ch, rest);
	if (obj == NULL) {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}
    } else if (!str_prefix(arg, "mobile") || !str_prefix(arg, "character")) {
	obj = NULL;
	mob = get_char_room(ch, rest);
	if (mob == NULL) {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}
    } else {			/* find both */

	mob = get_char_room(ch, argument);
	obj = get_obj_here(ch, argument);
	if (mob == NULL && obj == NULL) {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL) {
	OBJ_DATA *clone;

	if (!obj_check(ch, obj)) {
	    send_to_char
		("Your powers are not great enough for such a task.\n\r",
		 ch);
	    return;
	}

	clone = create_object(obj->pIndexData, 0);
	clone_object(obj, clone);

	clone->clone = str_dup(ch->name);

	if (obj->carried_by != NULL)
	    obj_to_char(clone, ch);
	else
	    obj_to_room(clone, ch->in_room);
	recursive_clone(ch, obj, clone);

	act("$n has created $p.", ch, clone, NULL, TO_ROOM);
	act("You clone $p.", ch, clone, NULL, TO_CHAR);
	wiznet("$N clones $p.", ch, clone, WIZ_LOAD, WIZ_SECURE,
	       get_trust(ch));
	return;
    } else if (mob != NULL) {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob)) {
	    MOB_INDEX_DATA *pMobIndex;

	    /* Clone a PC to a MOB - Suzuran */
	    /* First, load random mob... Fido will do. */

	    if ((pMobIndex = get_mob_index(3062)) == NULL) {
		send_to_char("Cannot open template mob.\n\r", ch);
		return;
	    }
	    clone = create_mobile(pMobIndex);

	    /* Now, string the mob's name, desc, etc. */
	    clone_pc(mob, clone);
	    char_to_room(clone, ch->in_room);

	    /* Fix up strings */
	    sprintf(buf, "%s%s is standing here.\n", capitalize(mob->name),
		    mob->pcdata->title);
	    clone->long_descr = str_dup(buf);
	    clone->name = str_dup(mob->name);
	    clone->description = str_dup(mob->description);
	    clone->short_descr = str_dup(capitalize(mob->name));

	    /* Set ACT bits */
	    clone->act = ACT_IS_NPC | ACT_SENTINEL;

	    send_to_char("Done.\n\r", ch);

	    act("$n has cloned $N.", ch, NULL, clone, TO_ROOM);
	    sprintf(buf, "$N pc-clones %s.", clone->short_descr);
	    wiznet(buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust(ch));
	    /* Done */
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch, GOD))
	    || (mob->level > 10 && !IS_TRUSTED(ch, IMMORTAL))
	    || (mob->level > 5 && !IS_TRUSTED(ch, DEMI))
	    || (mob->level > 0 && !IS_TRUSTED(ch, ANGEL))
	    || !IS_TRUSTED(ch, AVATAR)) {
	    send_to_char
		("Your powers are not great enough for such a task.\n\r",
		 ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob, clone);

	for (obj = mob->carrying; obj != NULL; obj = obj->next_content) {
	    if (obj_check(ch, obj)) {
		new_obj = create_object(obj->pIndexData, 0);
		clone_object(obj, new_obj);
		recursive_clone(ch, obj, new_obj);
		obj_to_char(new_obj, clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone, ch->in_room);
	act("$n has created $N.", ch, NULL, clone, TO_ROOM);
	act("You clone $N.", ch, NULL, clone, TO_CHAR);
	sprintf(buf, "$N clones %s.", clone->short_descr);
	wiznet(buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust(ch));
	return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  load mob <vnum>\n\r", ch);
	send_to_char("  load obj <vnum> <level>\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "mob") || !str_cmp(arg, "char")) {
	do_mload(ch, argument);
	return;
    }

    if (!str_cmp(arg, "obj")) {
	do_oload(ch, argument);
	return;
    }
    /* echo syntax */
    do_load(ch, "");
}


void do_mload(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (arg[0] == '\0' || !is_number(arg)) {
	send_to_char("Syntax: load mob <vnum>.\n\r", ch);
	return;
    }

    if ((pMobIndex = get_mob_index(atoi(arg))) == NULL) {
	send_to_char("No mob has that vnum.\n\r", ch);
	return;
    }

    victim = create_mobile(pMobIndex);
    char_to_room(victim, ch->in_room);
    act("$n has created $N!", ch, NULL, victim, TO_ROOM);
    sprintf(buf, "$N loads %s.", victim->short_descr);
    wiznet(buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust(ch));
    send_to_char("Ok.\n\r", ch);
    if (IS_SET(ch->act, PLR_QUESTMST)) {
	ch->quest = victim;
	SET_BIT(victim->act, ACT_QUEST);
    }
    return;
}



void do_oload(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;

    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (arg1[0] == '\0' || !is_number(arg1)) {
	send_to_char("Syntax: load obj <vnum> <level>.\n\r", ch);
	return;
    }

    level = get_trust(ch);	/* default */

    if (arg2[0] != '\0') {	/* load with a level */
	if (!is_number(arg2)) {
	    send_to_char("Syntax: oload <vnum> <level>.\n\r", ch);
	    return;
	}
	level = atoi(arg2);
	if (level < 0 || level > get_trust(ch)) {
	    send_to_char("Level must be be between 0 and your level.\n\r",
			 ch);
	    return;
	}
    }

    if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL) {
	send_to_char("No object has that vnum.\n\r", ch);
	return;
    }

    obj = create_object(pObjIndex, level);
    obj->iname = str_dup(ch->name);

    if (CAN_WEAR(obj, ITEM_TAKE))
	obj_to_char(obj, ch);
    else
	obj_to_room(obj, ch->in_room);
    act("$n has created $p!", ch, obj, NULL, TO_ROOM);
    wiznet("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, get_trust(ch));
    send_to_char("Ok.\n\r", ch);
    return;
}


void do_purge(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA *obj_next;

	for (victim = ch->in_room->people; victim != NULL; victim = vnext) {
	    vnext = victim->next_in_room;
	    if (IS_NPC(victim) && !IS_SET(victim->act, ACT_NOPURGE)
		&& victim != ch /* safety precaution */ )
		extract_char(victim, TRUE);
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj, ITEM_NOPURGE))
		extract_obj(obj);
	}

	act("$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char("Ok.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim)) {

	if (ch == victim) {
	    send_to_char("Ho ho ho.\n\r", ch);
	    return;
	}

	if (get_trust(ch) <= get_trust(victim)) {
	    send_to_char("Maybe that wasn't a good idea...\n\r", ch);
	    sprintf(buf, "%s tried to purge you!\n\r", ch->name);
	    send_to_char(buf, victim);
	    return;
	}

	act("$n disintegrates $N.", ch, 0, victim, TO_NOTVICT);

	if (victim->level > 1)
	    save_char_obj(victim);
	d = victim->desc;
	extract_char(victim, TRUE);
	if (d != NULL)
	    close_socket(d);

	return;
    }

    act("$n purges $N.", ch, NULL, victim, TO_NOTVICT);
    extract_char(victim, TRUE);
    return;
}



void do_advance(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);


    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
	send_to_char("Syntax: advance <char> <level>.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("That player is not here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if ((level = atoi(arg2)) < 1 || level > 99) {
	send_to_char("Level must be 1 to 99.\n\r", ch);
	return;
    }

    if (level > get_trust(ch)) {
	send_to_char("Limited to your trust level.\n\r", ch);
	return;
    }

    /*
       * Lower level:
       *   Reset to level 1.
       *   Then raise again.
       *   Currently, an imp can lower another imp.
       *   -- Swiftest
     */
    if (level <= victim->level) {
	int temp_prac;
	int temp_train;

	send_to_char("Lowering a player's level!\n\r", ch);
	send_to_char("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim);
	send_to_char("You have been demoted by an WIZ.\n\r", victim);
	temp_prac = victim->practice;
	temp_train = victim->train;
	victim->level = 1;
	victim->exp = exp_per_level(victim, victim->pcdata->points);
	/* Thierry 960409  pr and tr was 0 made them 6 and 3 */
	/* EE961110 Made practice sessions 5 (as for a new char)
	   * Also changed max_hit to 20 */
	victim->practice = 5;
	victim->train = 3;
	victim->max_hit = 20;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->hit = victim->max_hit;
	victim->mana = victim->max_mana;
	victim->move = victim->max_move;
	for (iLevel = victim->level; iLevel < level; iLevel++) {
	    victim->level += 1;
	    advance_level(victim);
	}
	/*
	   advance_level( victim );
	 */
	victim->practice = temp_prac;
	victim->train = temp_train;
	return;
    } else {
	send_to_char("Raising a player's level!\n\r", ch);
	send_to_char("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim);
	send_to_char("You have been advanced by a WIZ.\n\r", victim);
    }

    for (iLevel = victim->level; iLevel < level; iLevel++) {
	victim->level += 1;
	advance_level(victim);
    }
    victim->exp = exp_per_level(victim, victim->pcdata->points)
	* UMAX(1, victim->level);
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);


    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
	send_to_char("Syntax: trust <char> <level>.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("That player is not here.\n\r", ch);
	return;
    }

    if ((level = atoi(arg2)) < 0 || level > 99) {
	send_to_char("Level must be 0 (reset) or 1 to 99.\n\r", ch);
	return;
    }

    if (level > get_trust(ch)) {
	send_to_char("Limited to your trust.\n\r", ch);
	return;
    }

    victim->trust = level;
    SET_BIT(victim->comm, COMM_TRUE_TRUST);
    return;
}



void do_restore(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument(argument, arg);


    if (arg[0] == '\0' || !str_cmp(arg, "room")) {
	/* cure room */

	for (vch = ch->in_room->people; vch != NULL;
	     vch = vch->next_in_room) {
	    affect_strip(vch, gsn_plague);
	    affect_strip(vch, gsn_poison);
	    affect_strip(vch, gsn_blindness);
	    affect_strip(vch, gsn_sleep);
	    affect_strip(vch, gsn_curse);

	    vch->hit = vch->max_hit;
	    vch->mana = vch->max_mana;
	    vch->move = vch->max_move;
	    update_pos(vch);
	    act("$n has restored you.", ch, NULL, vch, TO_VICT);
	}

	sprintf(buf, "$N restored room %d.", ch->in_room->vnum);
	wiznet(buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust(ch));

	send_to_char("Room restored.\n\r", ch);
	return;

    }

    if ((get_trust(ch) >= 98) && !str_prefix(arg, "all")) {
	/* cure all */

	for (d = descriptor_list; d != NULL; d = d->next) {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;

	    affect_strip(victim, gsn_plague);
	    affect_strip(victim, gsn_poison);
	    affect_strip(victim, gsn_blindness);
	    affect_strip(victim, gsn_sleep);
	    affect_strip(victim, gsn_curse);

	    victim->hit = victim->max_hit;
	    victim->mana = victim->max_mana;
	    victim->move = victim->max_move;
	    update_pos(victim);
	    if (victim->in_room != NULL)
		act("$n has restored you.", ch, NULL, victim, TO_VICT);
	}
	send_to_char("All active players restored.\n\r", ch);
	return;
    }

    if ((get_trust(ch) >= 96) && !str_prefix(arg, "quest")) {
	for (d = descriptor_list; d != NULL; d = d->next) {
	    victim = d->character;

	    if ((victim == NULL) || IS_NPC(victim))
		continue;

	    if ((!IS_QUESTOR(victim)
		 || (victim->pcdata->questgiver == NULL))
		&& !IS_QUESTMST(victim))
		continue;

	    affect_strip(victim, gsn_plague);
	    affect_strip(victim, gsn_poison);
	    affect_strip(victim, gsn_blindness);
	    affect_strip(victim, gsn_sleep);
	    affect_strip(victim, gsn_curse);

	    victim->hit = victim->max_hit;
	    victim->mana = victim->max_mana;
	    victim->move = victim->max_move;
	    update_pos(victim);
	    if (victim->in_room != NULL)
		act("$n has restored you.", ch, NULL, victim, TO_VICT);
	}
	send_to_char("All questing players restored.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    affect_strip(victim, gsn_plague);
    affect_strip(victim, gsn_poison);
    affect_strip(victim, gsn_blindness);
    affect_strip(victim, gsn_sleep);
    affect_strip(victim, gsn_curse);
    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos(victim);
    act("$n has restored you.", ch, NULL, victim, TO_VICT);
    sprintf(buf, "$N restored %s",
	    IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust(ch));
    send_to_char("Ok.\n\r", ch);
    return;
}


void do_freeze(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Freeze whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (IS_SET(victim->act, PLR_FREEZE)) {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char("You can play again.\n\r", victim);
	send_to_char("FREEZE removed.\n\r", ch);
	sprintf(buf, "$N thaws %s.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    } else {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char("You can't do ANYthing!\n\r", victim);
	send_to_char("FREEZE set.\n\r", ch);
	sprintf(buf, "$N puts %s in the deep freeze.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    save_char_obj(victim);

    return;
}



void do_log(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);



    if (arg[0] == '\0') {
	send_to_char("Log whom?\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "all")) {
	if (fLogAll) {
	    fLogAll = FALSE;
	    send_to_char("Log ALL off.\n\r", ch);
	} else {
	    fLogAll = TRUE;
	    send_to_char("Log ALL on.\n\r", ch);
	}
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    /*
       * No level check, gods can log anyone.
     */
    if (IS_SET(victim->act, PLR_LOG)) {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char("LOG removed.\n\r", ch);
    } else {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char("LOG set.\n\r", ch);
    }

    return;
}



void do_noemote(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);



    if (arg[0] == '\0') {
	send_to_char("Noemote whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }


    if (get_trust(victim) >= get_trust(ch)) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (IS_SET(victim->comm, COMM_NOEMOTE)) {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char("You can emote again.\n\r", victim);
	send_to_char("NOEMOTE removed.\n\r", ch);
	sprintf(buf, "$N restores emotes to %s.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    } else {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char("You can't emote!\n\r", victim);
	send_to_char("NOEMOTE set.\n\r", ch);
	sprintf(buf, "$N revokes %s's emotes.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    return;
}



void do_noshout(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);


    if (arg[0] == '\0') {
	send_to_char("Noshout whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (IS_SET(victim->comm, COMM_NOSHOUT)) {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char("You can shout again.\n\r", victim);
	send_to_char("NOSHOUT removed.\n\r", ch);
	sprintf(buf, "$N restores shouts to %s.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    } else {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char("You can't shout!\n\r", victim);
	send_to_char("NOSHOUT set.\n\r", ch);
	sprintf(buf, "$N revokes %s's shouts.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    return;
}



void do_notell(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);


    if (arg[0] == '\0') {
	send_to_char("Notell whom?", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch)) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (IS_SET(victim->comm, COMM_NOTELL)) {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char("You can tell again.\n\r", victim);
	send_to_char("NOTELL removed.\n\r", ch);
	sprintf(buf, "$N restores tells to %s.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    } else {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char("You can't tell!\n\r", victim);
	send_to_char("NOTELL set.\n\r", ch);
	sprintf(buf, "$N revokes %s's tells.", victim->name);
	wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    return;
}



void do_peace(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *rch;


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
	if (rch->fighting != NULL)
	    stop_fighting(rch, TRUE);
	if (IS_NPC(rch) && IS_SET(rch->act, ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act, ACT_AGGRESSIVE);
    }

    send_to_char("Ok.\n\r", ch);
    return;
}

void do_wizlock(CHAR_DATA * ch, char *argument)
{
    extern bool wizlock;
    wizlock = !wizlock;


    if (wizlock) {
	wiznet("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
	send_to_char("Game wizlocked.\n\r", ch);
    } else {
	wiznet("$N removes wizlock.", ch, NULL, 0, 0, 0);
	send_to_char("Game un-wizlocked.\n\r", ch);
    }

    return;
}

/* RT anti-newbie code */

void do_newlock(CHAR_DATA * ch, char *argument)
{
    extern bool newlock;
    newlock = !newlock;


    if (newlock) {
	wiznet("$N locks out new characters.", ch, NULL, 0, 0, 0);
	send_to_char("New characters have been locked out.\n\r", ch);
    } else {
	wiznet("$N allows new characters back in.", ch, NULL, 0, 0, 0);
	send_to_char("Newlock removed.\n\r", ch);
    }

    return;
}


void do_slookup(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Lookup which skill or spell?\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "all")) {
	for (sn = 0; sn < MAX_SKILL; sn++) {
	    if (skill_table[sn].name == NULL)
		break;
	    sprintf(buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		    sn, skill_table[sn].slot, skill_table[sn].name);
	    send_to_char(buf, ch);
	}
    } else {
	if ((sn = skill_lookup(arg)) < 0) {
	    send_to_char("No such skill or spell.\n\r", ch);
	    return;
	}

	sprintf(buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name);
	send_to_char(buf, ch);
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);
    if (arg[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  set mob   <name> <field> <value>\n\r", ch);
	send_to_char("  set char  <name> <field> <value>\n\r", ch);
	send_to_char("  set obj   <name> <field> <value>\n\r", ch);
	send_to_char("  set room  <room> <field> <value>\n\r", ch);
	send_to_char("  set skill <name> <spell or skill> <value>\n\r",
		     ch);
	return;
    }

    if (!str_prefix(arg, "mobile") || !str_prefix(arg, "character")) {
	do_mset(ch, argument);
	return;
    }

    if (!str_prefix(arg, "skill") || !str_prefix(arg, "spell")) {
	do_sset(ch, argument);
	return;
    }

    if (!str_prefix(arg, "object")) {
	do_oset(ch, argument);
	return;
    }

    if (!str_prefix(arg, "room")) {
	do_rset(ch, argument);
	return;
    }
    /* echo syntax */
    do_set(ch, "");
}


void do_sset(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  set skill <name> <spell or skill> <value>\n\r",
		     ch);
	send_to_char("  set skill <name> all <value>\n\r", ch);
	send_to_char("   (use the name of the skill, not the number)\n\r",
		     ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    fAll = !str_cmp(arg2, "all");
    sn = 0;
    if (!fAll && (sn = skill_lookup(arg2)) < 0) {
	send_to_char("No such skill or spell.\n\r", ch);
	return;
    }

    /*
       * Snarf the value.
     */
    if (!is_number(arg3)) {
	send_to_char("Value must be numeric.\n\r", ch);
	return;
    }

    value = atoi(arg3);
    if (value < 0 || value > 100) {
	send_to_char("Value range is 0 to 100.\n\r", ch);
	return;
    }

    if (fAll) {
	for (sn = 0; sn < MAX_SKILL; sn++) {
	    if (skill_table[sn].name != NULL
		&& strcmp(skill_table[sn].name, "reserved")
		&& skill_table[sn].rating[ch->class] != 99)
		victim->pcdata->learned[sn] = value;
	}
    } else {
	victim->pcdata->learned[sn] = value;
    }

    return;
}



void do_mset(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf1[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  set char <name> <field> <value>\n\r", ch);
	send_to_char("  Field being one of:\n\r", ch);
	send_to_char("    str int wis dex con sex class level\n\r", ch);
	send_to_char("    race group gold silver hp mana move prac\n\r",
		     ch);
	send_to_char("    align train thirst hunger drunk full\n\r", ch);
	send_to_char( "    security\n\r",ch );
	sprintf(buf1, "    questpoints pkills pdeaths %s\n\r",
		(ch->level >= MAX_LEVEL) ? "rpoints" : "");
	send_to_char(buf1, ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim) && ch != victim && get_trust(ch) < ADMIN) {
	send_to_char
	    ("Sorry, but you cannot set chars other than yourself.\n\r",
	     ch);
	return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
       * Snarf the value (which need not be numeric).
     */
    value = is_number(arg3) ? atoi(arg3) : -1;

    /*
       * Set something.
     */
    if (!str_cmp(arg2, "str")) {
	if (value < 3 || value > get_max_train(victim, STAT_STR)) {
	    sprintf(buf,
		    "Strength range is 3 to %d\n\r.",
		    get_max_train(victim, STAT_STR));
	    send_to_char(buf, ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    /* OLC - Suzuran */
    if ( !str_cmp( arg2, "security" ) )       /* OLC */
      {
	if ( IS_NPC(ch) )
	  {
	    send_to_char( "Yeah, OK.\n\r", ch );
	    return;
	  }
	
	if ( IS_NPC( victim ) )
	  {
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	  }
	
	if ( value > ch->pcdata->security || value < 0 )
	  {
	    if ( ch->pcdata->security != 0 )
	      {
		sprintf( buf, "Valid security is 0-%d.\n\r",
			 ch->pcdata->security );
		send_to_char( buf, ch );
	      }
	    else
	      {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	      }
	    return;
	  }
	victim->pcdata->security = value;
	return;
      }

    if (!str_cmp(arg2, "int")) {
	if (value < 3 || value > get_max_train(victim, STAT_INT)) {
	    sprintf(buf,
		    "Intelligence range is 3 to %d.\n\r",
		    get_max_train(victim, STAT_INT));
	    send_to_char(buf, ch);
	    return;
	}

	victim->perm_stat[STAT_INT] = value;
	return;
    }

    if (!str_cmp(arg2, "wis")) {
	if (value < 3 || value > get_max_train(victim, STAT_WIS)) {
	    sprintf(buf,
		    "Wisdom range is 3 to %d.\n\r", get_max_train(victim,
								  STAT_WIS));
	    send_to_char(buf, ch);
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if (!str_cmp(arg2, "dex")) {
	if (value < 3 || value > get_max_train(victim, STAT_DEX)) {
	    sprintf(buf,
		    "Dexterity ranges is 3 to %d.\n\r",
		    get_max_train(victim, STAT_DEX));
	    send_to_char(buf, ch);
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if (!str_cmp(arg2, "con")) {
	if (value < 3 || value > get_max_train(victim, STAT_CON)) {
	    sprintf(buf,
		    "Constitution range is 3 to %d.\n\r",
		    get_max_train(victim, STAT_CON));
	    send_to_char(buf, ch);
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if (!str_prefix(arg2, "sex")) {
	if (value < 0 || value > 2) {
	    send_to_char("Sex range is 0 to 2.\n\r", ch);
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if (!str_prefix(arg2, "class")) {
	int class;

	if (IS_NPC(victim)) {
	    send_to_char("Mobiles have no class.\n\r", ch);
	    return;
	}

	class = class_lookup(arg3);
	if (class == -1) {
	    char buf[MAX_STRING_LENGTH];

	    strcpy(buf, "Possible classes are: ");
	    for (class = 0; class < MAX_CLASS; class++) {
		if (class > 0)
		    strcat(buf, " ");
		strcat(buf, class_table[class].name);
	    }
	    strcat(buf, ".\n\r");

	    send_to_char(buf, ch);
	    return;
	}

	victim->class = class;
	return;
    }

    if (!str_prefix(arg2, "level")) {
	if (!IS_NPC(victim)) {
	    send_to_char("Not on PC's.\n\r", ch);
	    return;
	}

	if (value < 0 || value > 100) {
	    send_to_char("Level range is 0 to 100.\n\r", ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if (!str_prefix(arg2, "gold")) {
	victim->gold = value;
	return;
    }

    if (!str_prefix(arg2, "silver")) {
	victim->silver = value;
	return;
    }

    if (!str_prefix(arg2, "hp")) {
	if (value < -10 || value > 30000) {
	    send_to_char("Hp range is -10 to 30,000 hit points.\n\r", ch);
	    return;
	}
	victim->max_hit = value;
	if (!IS_NPC(victim))
	    victim->pcdata->perm_hit = value;
	return;
    }

    if (!str_prefix(arg2, "mana")) {
	if (value < 0 || value > 30000) {
	    send_to_char("Mana range is 0 to 30,000 mana points.\n\r", ch);
	    return;
	}
	victim->max_mana = value;
	if (!IS_NPC(victim))
	    victim->pcdata->perm_mana = value;
	return;
    }

    if (!str_prefix(arg2, "move")) {
	if (value < 0 || value > 30000) {
	    send_to_char("Move range is 0 to 30,000 move points.\n\r", ch);
	    return;
	}
	victim->max_move = value;
	if (!IS_NPC(victim))
	    victim->pcdata->perm_move = value;
	return;
    }

    if (!str_prefix(arg2, "practice")) {
	if (value < 0 || value > 250) {
	    send_to_char("Practice range is 0 to 250 sessions.\n\r", ch);
	    return;
	}
	victim->practice = value;
	return;
    }

    if (!str_prefix(arg2, "train")) {
	if (value < 0 || value > 50) {
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",
			 ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if (!str_prefix(arg2, "align")) {
	if (value < -1000 || value > 1000) {
	    send_to_char("Alignment range is -1000 to 1000.\n\r", ch);
	    return;
	}
	victim->alignment = value;
	return;
    }

    if (!str_prefix(arg2, "thirst")) {
	if (IS_NPC(victim)) {
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}

	if (value < -1 || value > 100) {
	    send_to_char("Thirst range is -1 to 100.\n\r", ch);
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if (!str_prefix(arg2, "drunk")) {
	if (IS_NPC(victim)) {
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}

	if (value < -1 || value > 100) {
	    send_to_char("Drunk range is -1 to 100.\n\r", ch);
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if (!str_prefix(arg2, "full")) {
	if (IS_NPC(victim)) {
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}

	if (value < -1 || value > 100) {
	    send_to_char("Full range is -1 to 100.\n\r", ch);
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if (!str_prefix(arg2, "hunger")) {
	if (IS_NPC(victim)) {
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
	}

	if (value < -1 || value > 100) {
	    send_to_char("Full range is -1 to 100.\n\r", ch);
	    return;
	}

	victim->pcdata->condition[COND_HUNGER] = value;
	return;
    }

    if (!str_prefix(arg2, "race")) {
	int race;

	race = race_lookup(arg3);

	if (race == 0) {
	    send_to_char("That is not a valid race.\n\r", ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race) {
	    send_to_char("That is not a valid player race.\n\r", ch);
	    return;
	}

	if (race == race_lookup("Dragonkin") && victim->race != race) {
	    SET_BIT(victim->affected_by, AFF_FLYING);
	    sprintf(buf, "Adding AFF_FLYING bit to `&%s``.\n\r",
		    victim->name);
	    send_to_char(buf, ch);
	} else if (victim->race == race_lookup("Dragonkin")
		   && race != victim->race) {
	    REMOVE_BIT(victim->affected_by, AFF_FLYING);
	    sprintf(buf, "Removing AFF_FLYING bit from `&%s``.\n\r",
		    victim->name);
	    send_to_char(buf, ch);
	}

	victim->race = race;

	return;
    }

    if (!str_prefix(arg2, "group")) {
	if (!IS_NPC(victim)) {
	    send_to_char("Only on NPCs.\n\r", ch);
	    return;
	}
	victim->group = value;
	return;
    }

    if (!str_prefix(arg2, "questpoints")) {
	if (IS_NPC(victim)) {
	    send_to_char("Only on PCs.\n\r", ch);
	    return;
	}
	if (value < 0 || value > 5000) {
	    send_to_char("Value range is 0 to 5000\n\r", ch);
	    return;
	}
	victim->questpoints = value;
	return;
    }

    if (!str_prefix(arg2, "pkills")) {
	if (IS_NPC(victim)) {
	    send_to_char("Only on PCs.\n\r", ch);
	    return;
	}
	if (value < 0 || value > 1000) {
	    send_to_char("Value range is 0 to 1000.\n\r", ch);
	    return;
	}
	victim->pcdata->pkills = value;
	return;
    }

    if (!str_prefix(arg2, "pdeaths")) {
	if (IS_NPC(victim)) {
	    send_to_char("Only on PCs.\n\r", ch);
	    return;
	}
	if (value < 0 || value > 1000) {
	    send_to_char("Value range is 0 to 1000.\n\r", ch);
	    return;
	}
	victim->pcdata->pdeaths = value;
	return;
    }

    if (!str_prefix(arg2, "rpoints") && (ch->level >= MAX_LEVEL)) {
	if (value < 0 || value > 10000) {
	    send_to_char("Value range is 0 to 10000.\n\r", ch);
	    return;
	}
	victim->rp_points = value;
	return;
    }

    /*
       * Generate usage message.
     */
    do_mset(ch, "");
    return;
}

void do_string(CHAR_DATA * ch, char *argument)
{
    char type[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde(argument);
    argument = one_argument(argument, type);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);


    if (type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0'
	|| arg3[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  string char <name> <field> <string>\n\r", ch);
	send_to_char("    fields: name short long desc title spec\n\r",
		     ch);
	send_to_char("  string obj  <name> <field> <string>\n\r", ch);
	send_to_char("    fields: name short long extended\n\r", ch);
	return;
    }

    if (!str_prefix(type, "character") || !str_prefix(type, "mobile")) {
	if ((victim = get_char_world(ch, arg1)) == NULL) {
	    send_to_char("They aren't here.\n\r", ch);
	    return;
	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

	if (!str_prefix(arg2, "name")) {
	    if (!IS_NPC(victim)) {
		send_to_char("Not on PC's.\n\r", ch);
		return;
	    }
	    free_string(victim->name);
	    victim->name = str_dup(arg3);
	    return;
	}

	if (!str_prefix(arg2, "description")) {
	    free_string(victim->description);
	    victim->description = str_dup(arg3);
	    return;
	}

	if (!str_prefix(arg2, "short")) {
	    free_string(victim->short_descr);
	    victim->short_descr = str_dup(arg3);
	    return;
	}

	if (!str_prefix(arg2, "long")) {
	    free_string(victim->long_descr);
	    strcat(arg3, "\n\r");
	    victim->long_descr = str_dup(arg3);
	    return;
	}

	if (!str_prefix(arg2, "title")) {
	    if (IS_NPC(victim)) {
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	    }

	    set_title(victim, arg3);
	    return;
	}

	if (!str_prefix(arg2, "spec")) {
	    if (!IS_NPC(victim)) {
		send_to_char("Not on PC's.\n\r", ch);
		return;
	    }

	    if ((victim->spec_fun = spec_lookup(arg3)) == 0) {
		send_to_char("No such spec fun.\n\r", ch);
		return;
	    }

	    return;
	}
    }

    if (!str_prefix(type, "object")) {
	/* string an obj */

	if ((obj = get_obj_world(ch, arg1)) == NULL) {
	    send_to_char("Nothing like that in heaven or earth.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg2, "name")) {
	    free_string(obj->name);
	    obj->name = str_dup(arg3);
	    return;
	}

	if (!str_prefix(arg2, "short")) {
	    free_string(obj->short_descr);
	    obj->short_descr = str_dup(arg3);
	    return;
	}

	if (!str_prefix(arg2, "long")) {
	    free_string(obj->description);
	    obj->description = str_dup(arg3);
	    return;
	}

	if (!str_prefix(arg2, "ed") || !str_prefix(arg2, "extended")) {
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument(argument, arg3);
	    if (argument == NULL) {
		send_to_char
		    ("Syntax: oset <object> ed <keyword> <string>\n\r",
		     ch);
		return;
	    }

	    strcat(argument, "\n\r");

	    ed = new_extra_descr();

	    ed->keyword = str_dup(arg3);
	    ed->description = str_dup(argument);
	    ed->next = obj->extra_descr;
	    obj->extra_descr = ed;
	    return;
	}
    }


    /* echo bad use message */
    do_string(ch, "");
}



void do_oset(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    int value;
    int valueb;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    strcpy(arg4, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  set obj <object> <field> <value>\n\r", ch);
	send_to_char("  Field being one of:\n\r", ch);
	send_to_char("    value0 value1 value2 value3 value4 (v0-v4)\n\r",ch);
	send_to_char("    extra wear level weight cost timer aff cond\n\r",ch);
	return;
    }

    if ((obj = get_obj_world(ch, arg1)) == NULL) {
	send_to_char("Nothing like that in heaven or earth.\n\r", ch);
	return;
    }

    /*
       * Snarf the value (which need not be numeric).
     */
    value = atoi(arg3);
    valueb = atoi(arg4);
    /*
       * Set something.
     */
    if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0")) {
	obj->value[0] = UMIN(50, value);
	return;
    }

    if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1")) {
	obj->value[1] = value;
	return;
    }

    if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2")) {
	obj->value[2] = value;
	return;
    }

    if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3")) {
	obj->value[3] = value;
	return;
    }

    if (!str_cmp(arg2, "value4") || !str_cmp(arg2, "v4")) {
	obj->value[4] = value;
	return;
    }

    if (!str_prefix(arg2, "aff")) {
	if (arg4[0] == '\0') {
	    send_to_char("You must apply the amount.\n\r", ch);
	    return;
	}

	paf = alloc_mem(sizeof(*paf));
	paf->where = TO_OBJECT;
	paf->type = 0;
	paf->level = obj->level;
	paf->duration = -1;
	paf->location = value;
	paf->modifier = valueb;
	paf->bitvector = 0;
	paf->next = obj->affected;
	obj->affected = paf;

	return;
    }

    if (!str_prefix(arg2, "extra")) {
	obj->extra_flags = value;
	return;
    }

    if (IS_ADMIN(ch) && (!str_prefix(arg2, "owner"))) {
	if (!str_cmp(arg3, "none")
	    || ((victim = get_char_world(ch, arg3)) == NULL)) {
	    send_to_char("They aren't here.\n\r", ch);
	    return;
	}
	free_string(obj->owner);
	if (!str_cmp(arg3, "none"))
	    obj->owner = str_dup("");
	else
	    obj->owner = str_dup(victim->name);
	return;
    }


    if (!str_prefix(arg2, "wear")) {
	obj->wear_flags = value;
	return;
    }

    if (!str_prefix(arg2, "level")) {
	obj->level = value;
	return;
    }

    if (!str_prefix(arg2, "weight")) {
	obj->weight = value;
	return;
    }

    if (!str_prefix(arg2, "cost")) {
	obj->cost = value;
	return;
    }

    if (!str_prefix(arg2, "timer")) {
	obj->timer = value;
	return;
    }

    if (!str_prefix(arg2, "cond")) {
	obj->condition = value;
	return;
    }
    /*
       * Generate usage message.
     */
    do_oset(ch, "");
    return;
}



void do_rset(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);


    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  set room <location> <field> <value>\n\r", ch);
	send_to_char("  Field being one of:\n\r", ch);
	send_to_char("    flags sector\n\r", ch);
	return;
    }

    if ((location = find_location(ch, arg1)) == NULL) {
	send_to_char("No such location.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch, location) && ch->in_room != location
	&& room_is_private(location) && !IS_TRUSTED(ch, IMPLEMENTOR)) {
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    /*
       * Snarf the value.
     */
    if (!is_number(arg3)) {
	send_to_char("Value must be numeric.\n\r", ch);
	return;
    }
    value = atoi(arg3);

    /*
       * Set something.
     */
    if (!str_prefix(arg2, "flags")) {
	location->room_flags = value;
	return;
    }

    if (!str_prefix(arg2, "sector")) {
	location->sector_type = value;
	return;
    }

    /*
       * Generate usage message.
     */
    do_rset(ch, "");
    return;
}


/* Added the connected states for a better look... --Vorlin */
void do_sockets(CHAR_DATA * ch, char *argument)
{
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH], arg1[MAX_INPUT_LENGTH];
    char *dname, *ename;
    int socketnum[50];
    int x, count, elisted, dlisted;

    /* Did the stuff below for added effects --Vorlin */
    char *st, *est; /* orig. state, duplicate state */
    char di[10];
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d, *e;

    count = 0;
    buf[0] = '\0';

    for (x = 0; x < 50; socketnum[x++] = 0);

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg1);

    if (!str_cmp(arg, "check")) {

	for (d = descriptor_list; d != NULL; d = d->next) {

	    switch (d->connected) {
	    case CON_PLAYING:
		st = "  `@P``  ";
		break;
	    case CON_GET_NAME:
	    case CON_GET_OLD_PASSWORD:
	    case CON_READ_MOTD:
	    case CON_READ_IMOTD:
		st = "  `^L``  ";
		break;
	    case CON_CONFIRM_NEW_NAME:
	    case CON_GET_NEW_PASSWORD:
	    case CON_CONFIRM_NEW_PASSWORD:
	    case CON_GET_NEW_RACE:
	    case CON_GET_NEW_SEX:
	    case CON_GET_NEW_CLASS:
	    case CON_GET_ALIGNMENT:
	    case CON_DEFAULT_CHOICE:
	    case CON_GEN_GROUPS:
	    case CON_PICK_WEAPON:
		st = "  `!C``  ";
		break;
	    case CON_BREAK_CONNECT:
		st = "  `8LD`` ";
		break;
	    default:
		st = "  `%??`` ";
		break;
	    }

	    dname = d->original ? d->original->name :
		d->character ? d->character->name : "(none)";

	    for (e = d->next; e != NULL; e = e->next) {

		switch (e->connected) {
		case CON_PLAYING:
		    est = "  `@P``  ";
		    break;
		case CON_GET_NAME:
		case CON_GET_OLD_PASSWORD:
		case CON_READ_MOTD:
		case CON_READ_IMOTD:
		    est = "  `^L``  ";
		    break;
		case CON_CONFIRM_NEW_NAME:
		case CON_GET_NEW_PASSWORD:
		case CON_CONFIRM_NEW_PASSWORD:
		case CON_GET_NEW_RACE:
		case CON_GET_NEW_SEX:
		case CON_GET_NEW_CLASS:
		case CON_GET_ALIGNMENT:
		case CON_DEFAULT_CHOICE:
		case CON_GEN_GROUPS:
		case CON_PICK_WEAPON:
		    est = "  `!C``  ";
		    break;
		case CON_BREAK_CONNECT:
		    est = "  `8LD`` ";
		    break;
		default:
		    est = "  `%?``  ";
		    break;
		}

		ename = e->original ? e->original->name :
		    e->character ? e->character->name : "(none)";

		if ((d->character != NULL) && can_see(ch, d->character)
		    && (e->character != NULL) && can_see(ch, e->character)
		    && !str_cmp(d->host, e->host)
		    && (str_cmp(dname, ename))) {
		    dlisted = elisted = 0;
		    for (x = 0; x <= count; x++)
			if (socketnum[x] == d->descriptor) {
			    dlisted = 1;
			    x = count + 1;
			}
		    for (x = 0; x <= count; x++)
			if (socketnum[x] == e->descriptor) {
			    elisted = 1;
			    x = count + 1;
			}
		    if (dlisted != 1) {
			socketnum[count] = d->descriptor;
			count++;

			if (!str_cmp(arg1, "ip")) {
			    sprintf(buf + strlen(buf),
				    "[%2d%s] %s@%d.%d.%d.%d\n\r",
				    d->descriptor,
				    st,
				    d->original ? d->original->
				    name : d->character ? d->
				    character->name : "(none)",
				    d->ip_addr[0], d->ip_addr[1],
				    d->ip_addr[2], d->ip_addr[3]);
			} else {
			    sprintf(buf + strlen(buf),
				    "[%2d%s] %s@%s\n\r",
				    d->descriptor,
				    st,
				    d->original ? d->original->
				    name : d->character ? d->
				    character->name : "(none)", d->host);
			}
		    }
		    if (elisted != 1) {
			socketnum[count] = e->descriptor;
			count++;

			if (!str_cmp(arg1, "ip")) {
			    sprintf(buf + strlen(buf),
				    "[%2d%s] %s@%d.%d.%d.%d\n\r",
				    e->descriptor,
				    est,
				    e->original ? e->original->
				    name : e->character ? e->
				    character->name : "(none)",
				    e->ip_addr[0], e->ip_addr[1],
				    e->ip_addr[2], e->ip_addr[3]);
			} else {
			    sprintf(buf + strlen(buf),
				    "[%2d%s] %s@%s\n\r",
				    e->descriptor,
				    est,
				    e->original ? e->original->
				    name : e->character ? e->
				    character->name : "(none)", e->host);
			}
		    }
		}
	    }
	}
    } else {
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if (d->character != NULL && can_see(ch, d->character)
		&& (arg[0] == '\0' || is_name(arg, d->character->name)
		    || !str_cmp(arg, "ip")
		    || (d->original && is_name(arg, d->original->name)))) {

		switch (d->connected) {
		case CON_PLAYING:
		    st = "  `@P``  ";
		    break;
		case CON_GET_NAME:
		case CON_GET_OLD_PASSWORD:
		case CON_READ_MOTD:
		case CON_READ_IMOTD:
		    st = "  `^L``  ";
		    break;
		case CON_CONFIRM_NEW_NAME:
		case CON_GET_NEW_PASSWORD:
		case CON_CONFIRM_NEW_PASSWORD:
		case CON_GET_NEW_RACE:
		case CON_GET_NEW_SEX:
		case CON_GET_NEW_CLASS:
		case CON_GET_ALIGNMENT:
		case CON_DEFAULT_CHOICE:
		case CON_GEN_GROUPS:
		case CON_PICK_WEAPON:
		    st = "  `!C``  ";
		    break;
		case CON_BREAK_CONNECT:
		    st = "  `8LD`` ";
		    break;
		default:
		    st = "  `%?``  ";
		    break;
		}

		/* Count the actual logins, based on CON_PLAYING */
		if (d->connected == 0) {
		    count++;
		}

		/* Add in the useless idle timer */
		vch = d->original ? d->original : d->character;
		if (vch->timer > 0) {
		sprintf(di, "%-2d", vch->timer);
		} else {
		sprintf(di, "  ");
		}

		if (!str_cmp(arg, "ip")
		    || (is_name(arg, d->character->name)
			&& !str_cmp(arg1, "ip"))) {
		    sprintf(buf + strlen(buf),
			    "[%2d%s`&%2s``] %s@%d.%d.%d.%d\n\r",
			    d->descriptor,
			    st,
			    di,
			    d->original ? d->original->
			    name : d->character ? d->character->
			    name : "(none)", d->ip_addr[0], d->ip_addr[1],
			    d->ip_addr[2], d->ip_addr[3]);
		} else {
		    sprintf(buf + strlen(buf), "[%2d%s`&%2s``] %s@%s\n\r",
			    d->descriptor,
			    st,
			    di,
			    d->original ? d->original->name :
			    d->character ? d->character->name : "(none)",
			    d->host);
		}
	    }
	}
	if (count == 0) {
	    send_to_char("No one by that name is connected.\n\r", ch);
	    return;
	}
    }
    sprintf(buf2, "%d user%s currently logged in to `&Anime``\n\r", count,
	    count == 1 ? "" : "s");
    strcat(buf, buf2);
    page_to_char(buf, ch);
    return;
}



/*
* Thanks to Grodyn for pointing out bugs in this function.
*/
void do_force(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);


    if (arg[0] == '\0' || argument[0] == '\0') {
	send_to_char("Force whom to do what?\n\r", ch);
	return;
    }

    one_argument(argument, arg2);

/*
    Here, "gran" and "rev" should be replaced, if necessary, with
    the mininum number of letters required to execute the "grant"
    and "revoke" commands on the mud.  If desired, you can change
    them to "grant" and "revoke" and add a "gran" and "revok"
    command in the same manner as the "delet" command.  If you
    take this course of action, you should add these new commands
    to the pair_table with:

	{"grant","gran",FALSE},
	{"revoke","revok",FALSE}
*/

    /* No! */
    if (!str_cmp(arg2, "delete") || is_name(arg2, "gran")
	|| is_name(arg2, "revo") || is_name(arg2, "mob")) {
	send_to_char("That will NOT be done.\n\r", ch);
	return;
    }

    sprintf(buf, "$n forces you to '%s'.", argument);

    if (!str_cmp(arg, "all")) {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 3) {
	    send_to_char("Not at your level!\n\r", ch);
	    return;
	}

	for (vch = char_list; vch != NULL; vch = vch_next) {
	    vch_next = vch->next;

	    if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch)) {
		act(buf, ch, NULL, vch, TO_VICT);
		interpret(vch, argument);
	    }
	}
    } else if (!str_cmp(arg, "players")) {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 2) {
	    send_to_char("Not at your level!\n\r", ch);
	    return;
	}

	for (vch = char_list; vch != NULL; vch = vch_next) {
	    vch_next = vch->next;

	    if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch)
		&& vch->level < LEVEL_HERO) {
		act(buf, ch, NULL, vch, TO_VICT);
		interpret(vch, argument);
	    }
	}
    } else if (!str_cmp(arg, "gods")) {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 2) {
	    send_to_char("Not at your level!\n\r", ch);
	    return;
	}

	for (vch = char_list; vch != NULL; vch = vch_next) {
	    vch_next = vch->next;

	    if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch)
		&& vch->level >= LEVEL_HERO) {
		act(buf, ch, NULL, vch, TO_VICT);
		interpret(vch, argument);
	    }
	}
    } else {
	CHAR_DATA *victim;

	if ((victim = get_char_world(ch, arg)) == NULL) {
	    send_to_char("They aren't here.\n\r", ch);
	    return;
	}

	if (victim == ch) {
	    send_to_char("Aye aye, right away!\n\r", ch);
	    return;
	}

	if (!is_room_owner(ch, victim->in_room)
	    && ch->in_room != victim->in_room
	    && room_is_private(victim->in_room)
	    && !IS_TRUSTED(ch, IMPLEMENTOR)) {
	    send_to_char("That character is in a private room.\n\r", ch);
	    return;
	}

	if (get_trust(victim) >= get_trust(ch)) {
	    send_to_char("Do it yourself!\n\r", ch);
	    return;
	}

	if (!IS_NPC(victim) && get_trust(ch) < MAX_LEVEL - 3) {
	    send_to_char("Not at your level!\n\r", ch);
	    return;
	}

	act(buf, ch, NULL, victim, TO_VICT);
	interpret(victim, argument);
    }

    send_to_char("Ok.\n\r", ch);
    return;
}



/*
* New routines by Dionysos.
*/
void do_invis(CHAR_DATA * ch, char *argument)
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument(argument, arg);


    if (arg[0] == '\0')
	/* take the default path */

	if (ch->invis_level) {
	    ch->invis_level = 0;
	    act("$n slowly fades into existence.", ch, NULL, NULL,
		TO_ROOM);
	    send_to_char("You slowly fade back into existence.\n\r", ch);
	} else {
	    /* EE961011, changed default invis value so the imm's won't whine about levels */
	    ch->invis_level = ch->level;
	    act("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("You slowly vanish into thin air.\n\r", ch);
    } else
	/* do the level thing */
    {
	level = atoi(arg);
	if (level < 2 || level > get_trust(ch)) {
	    send_to_char
		("Invis level must be between 2 and your level.\n\r", ch);
	    return;
	} else {
	    ch->reply = NULL;
	    ch->invis_level = level;
	    act("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("You slowly vanish into thin air.\n\r", ch);
	}
    }

    return;
}


void do_incognito(CHAR_DATA * ch, char *argument)
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument(argument, arg);


    if (arg[0] == '\0')
	/* take the default path */

	if (ch->incog_level) {
	    ch->incog_level = 0;
	    act("$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("You are no longer cloaked.\n\r", ch);
	} else {

	    /* EE961011, changed default invis value so the imm's won't whine about levels */
	    ch->incog_level = ch->level;
	    act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("You cloak your presence.\n\r", ch);
    } else
	/* do the level thing */
    {
	level = atoi(arg);
	if (level < 2 || level > get_trust(ch)) {
	    send_to_char
		("Incog level must be between 2 and your level.\n\r", ch);
	    return;
	} else {
	    ch->reply = NULL;
	    ch->incog_level = level;
	    act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("You cloak your presence.\n\r", ch);
	}
    }

    return;
}



void do_holylight(CHAR_DATA * ch, char *argument)
{

    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act, PLR_HOLYLIGHT)) {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char("Holy light mode off.\n\r", ch);
    } else {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char("Holy light mode on.\n\r", ch);
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi(CHAR_DATA * ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n", ch);
    return;
}

void do_prefix(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0') {
	if (ch->prefix[0] == '\0') {
	    send_to_char("You have no prefix to clear.\r\n", ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n", ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0') {
	sprintf(buf, "Prefix changed to %s.\r\n", argument);
	free_string(ch->prefix);
    } else {
	sprintf(buf, "Prefix set to %s.\r\n", argument);
    }

    ch->prefix = str_dup(argument);
}

/*EE960518*/
void do_perm(CHAR_DATA * ch, char *argument)
{
    char item[MAX_INPUT_LENGTH], affect[MAX_INPUT_LENGTH],
	mod[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    AFFECT_DATA *aff;

    argument = one_argument(argument, item);
    argument = one_argument(argument, affect);
    argument = one_argument(argument, mod);

    if (item[0] == '\0' || affect[0] == '\0') {
	send_to_char("Syntax: Perm <item> <affect> [modifier]\n\r", ch);
	return;
    }

    if ((obj = get_obj_carry(ch, item)) == NULL) {
	send_to_char("You don't have that item.\n\r", ch);
	return;
    }

    aff = new_affect();
    aff->level = ch->level;
    aff->duration = -1;
    aff->bitvector = 0;
    if ((aff->type = aff->location = get_item_apply_val(affect)) == 0) {
	send_to_char("That is not an affect!\n\r", ch);
	return;
    }

    if (mod[0] != '\0')
	aff->modifier = atoi(mod);
    else
	aff->modifier = ch->level;

    affect_to_obj(obj, aff);

    send_to_char("Ok.\n\r", ch);
    return;
}

void do_quest(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch)) {
	send_to_char("Only players can quest.\n\r", ch);
	return;
    }

    if (!is_granted_name(ch, "qecho")) {
	if (arg1[0] == '\0') {
	    send_to_char("Syntax: Quest apply | stop | master\n\r", ch);
	    return;
	}

	if (!strcmp(arg1, "master")) {
	    if (!IS_QUESTOR(ch)) {
		send_to_char("But you aren't on a quest!\n\r", ch);
		return;
	    }

	    for (victim = char_list; victim != 0; victim = victim->next) {
		if (IS_QUESTMST(victim) && !IS_NPC(victim)) {
		    if (IS_QUESTPKMST(victim))
			act
			    ("\n\r$N is the leader of your `!PK`1-`!Quest``.",
			     ch, NULL, victim, TO_CHAR);
		    else
			act
			    ("\n\r$N is the leader of your `4Q`$$u`^e`$$s`4t``",
			     ch, NULL, victim, TO_CHAR);
		    return;
		}
	    }
	    send_to_char("There is currently no quest.\n\r", ch);
	    return;
	}

	if (!strcmp(arg1, "apply")) {
	    if (IS_QUESTOR(ch)) {
		send_to_char("You are already questing!\n\r", ch);
		return;
	    }

	    if (IS_QUESTWTR(ch)) {
		send_to_char("You have already applied for a quest!\n\r",
			     ch);
		return;
	    }

	    if (ch->fight != 0) {
		send_to_char
		    ("You may not quest right after a pkfight!\n\r", ch);

	    }

	    for (victim = char_list; victim != NULL; victim = victim->next) {
		if (IS_QUESTMST(victim) && !IS_NPC(victim))
		    ch->pcdata->questgiver = victim;
	    }

	    if (ch->pcdata->questgiver == NULL) {
		send_to_char("There is currently no quest.\n\r", ch);
		return;
	    }

	    /*ch->pcdata->quest_waiting = (ch->pcdata->questgiver != NULL); */
	    SET_BIT(ch->act, PLR_QUESTWTR);
	    if (IS_QUESTPKMST(ch->pcdata->questgiver)) {
		act("$n wishes to participate in your `!PK`1-`!Quest``.",
		    ch, NULL, ch->pcdata->questgiver, TO_VICT);
		send_to_char
		    ("You beg the gods that they should allow you to participate in their `1Pk-Quest``.\n\r",
		     ch);
	    } else {
		act
		    ("$n wishes to participate in your `4Q`$$u`^e`$$s`4t``.",
		    ch, NULL, ch->pcdata->questgiver, TO_VICT);
		send_to_char
		    ("You beg the gods that they shall allow you to participate in their `4Q`$u`^e`$s`4t``.\n\r",
		     ch);
	    }
	    return;
	}

	if (!strcmp(arg1, "stop")) {
	    if (IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char
		    ("Whew, You can't be ordered to quest stop.\n\r", ch);
		return;
	    }

	    if (IS_QUESTOR(ch)) {
		REMOVE_BIT(ch->act, PLR_QUESTOR);
		ch->fight = 0;
		if (IS_QUESTPKMST(ch->pcdata->questgiver)) {
		    send_to_char
			("You are no longer on a `!PK`1-`!Quest``.\n\r",
			 ch);
		    act("$n has stopped `1Questing`` for you.", ch, NULL,
			ch->pcdata->questgiver, TO_VICT);
		} else {
		    send_to_char
			("You are no longer on a `4Q`$u`^e`$s`4t``.\n\r",
			 ch);
		    act
			("$n has stopped `4Q`$$u`^e`&st`^i`$$n`4g`` for you.",
			 ch, NULL, ch->pcdata->questgiver, TO_VICT);
		}

		return;
	    }

	    if (IS_QUESTWTR(ch)) {
		REMOVE_BIT(ch->act, PLR_QUESTWTR);
		if (IS_QUESTPKMST(ch->pcdata->questgiver)) {
		    send_to_char
			("You inform the Gods that you no longer wish to `1Quest``.\n\r",
			 ch);
		    act("$n no longer wishes to `1Quest`` for you.", ch,
			NULL, ch->pcdata->questgiver, TO_VICT);
		} else {
		    send_to_char
			("You inform the Gods that you no longer wish to `4Q`$u`^e`$s`4t``.\n\r",
			 ch);
		    act
			("$n no longer wishes to `4Q`$$u`^e`$$s`4t`` for you.",
			 ch, NULL, ch->pcdata->questgiver, TO_VICT);
		}

		return;
	    }
	    send_to_char("But you aren't questing.\n\r", ch);
	    return;
	}
    } else {			/* if this is true, then ch has been granted "qecho" */

	if (arg1[0] == '\0') {
	    send_to_char
		(" Syntax: Quest <char> | start [pk] | stop | list\n\r",
		 ch);
	    return;
	}

	if (!strcmp(arg1, "start")) {
	    for (victim = char_list; victim != NULL; victim = victim->next) {
		if (IS_QUESTMST(victim) && !IS_NPC(victim)) {
		    if (IS_QUESTPKMST(victim))
			send_to_char
			    ("There is already a `!PK`1-`!Quest`` going on.\n\r",
			     ch);
		    else
			send_to_char
			    ("There is already a `4Q`$u`^e`$s`4t`` going on.\n\r",
			     ch);
		    return;
		}
	    }
	    SET_BIT(ch->act, PLR_QUESTMST);
	    if (!strcmp(arg2, "pk")) {
		SET_BIT(ch->act, PLR_QUESTPKMST);
		sprintf(buf, "%s has started a `!PK`1-`!QUEST``!",
			ch->name);
	    } else
		sprintf(buf, "%s has started a `4Q`$u`^e`$s`4t``!",
			ch->name);
	    do_echo(ch, buf);
	    return;
	}

	if (!strcmp(arg1, "stop")) {
	    if (!IS_QUESTMST(ch)) {
		send_to_char("You aren't leading a quest.\n\r", ch);
		return;
	    }

	    for (victim = char_list; victim != NULL; victim = victim->next) {
		if (IS_NPC(victim))
		    continue;

		if (IS_QUESTPKMST(ch))
		    act("$ns `!PK`1-`!Quest`` has ceased to exist.", ch,
			NULL, victim, TO_VICT);
		else
		    act("$ns `4Q`$$u`^e`$$s`4t`` has ceased to exist.", ch,
			NULL, victim, TO_VICT);

		if (IS_QUESTOR(victim)) {
		    victim->fight = 0;
		    REMOVE_BIT(victim->act, PLR_QUESTOR);
		}
		if (IS_QUESTWTR(victim))
		    REMOVE_BIT(victim->act, PLR_QUESTWTR);
	    }
	    if (IS_QUESTPKMST(ch))
		send_to_char("You have stopped your `!PK`1-`!Quest``.\n\r",
			     ch);
	    else
		send_to_char
		    ("You have stopped your `4Q`$u`^e`$s`4t``.\n\r", ch);
	    REMOVE_BIT(ch->act, PLR_QUESTMST);
	    ch->quest = NULL;
	    if (IS_QUESTPKMST(ch))
		REMOVE_BIT(ch->act, PLR_QUESTPKMST);
	    return;
	}

	if (!strcmp(arg1, "list")) {
	    if (!IS_QUESTMST(ch)) {
		send_to_char
		    ("You haven't started a quest yet, use 'quest start'.\n\r",
		     ch);
		return;
	    }

	    for (victim = char_list; victim != NULL; victim = victim->next) {
		if (IS_NPC(victim))
		    continue;
		if (IS_QUESTWTR(victim)
		    && (victim->pcdata->questgiver == ch)) {
		    if (IS_QUESTPKMST(ch))
			act("$N wants to `!PK`1-`!Quest`` for you!", ch,
			    NULL, victim, TO_CHAR);
		    else
			act("$N wants to `4Q`$u`^e`$s`4t`` for you!", ch,
			    NULL, victim, TO_CHAR);
		}
	    }
	    return;
	}

	if (!IS_QUESTMST(ch)) {
	    send_to_char("You aren't leading a quest.\n\r", ch);
	    return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
	    send_to_char("Sorry, that person isn't here.\n\r", ch);
	    return;
	}

	if (IS_QUESTOR(victim) && !IS_NPC(victim)) {
	    REMOVE_BIT(victim->act, PLR_QUESTOR);
	    victim->pcdata->questgiver = NULL;
	    if (IS_QUESTPKMST(ch)) {
		act("$N is no longer on a `!PK`1-`!Quest``.", ch, NULL,
		    victim, TO_CHAR);
		send_to_char
		    ("You are no longer on a `!PK`1-`!Quest``.\n\r",
		     victim);
	    } else {
		act("$N is no longer on a `4Q`$u`^e`$s`4t``.", ch, NULL,
		    victim, TO_CHAR);
		send_to_char
		    ("You are no longer on a `4Q`$u`^e`$s`4t``.\n\r",
		     victim);
	    }
	} else if (!IS_NPC(victim)) {
	    SET_BIT(victim->act, PLR_QUESTOR);
	    if (IS_QUESTWTR(victim))
		REMOVE_BIT(victim->act, PLR_QUESTWTR);
	    /*if (victim->pcdata->questgiver != ch) */
	    victim->pcdata->questgiver = ch;
	    if (IS_QUESTPKMST(ch)) {
		act("You send $N on a `!PK`1-`!Quest``.", ch, NULL, victim,
		    TO_CHAR);
		send_to_char
		    ("You have been sent on a `!PK`1-`!Quest`` by a God!\n\r",
		     victim);
	    } else {
		act("You send $N on a `4Q`$$u`^e`$$s`4t``.", ch, NULL,
		    victim, TO_CHAR);
		send_to_char
		    ("You have been sent on a `4Q`$u`^e`$s`4t`` by a God!\n\r",
		     victim);
	    }
	}
	return;
    }
    send_to_char("For syntax, type quest.\n\r", ch);
    return;
}


void do_jail(CHAR_DATA * ch, char *argument)
{				/*EE960619 */
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int duration;

    argument = one_argument(argument, arg1);
    strcpy(arg2, argument);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
	send_to_char("Syntax: jail <player> <duration>.\n\r", ch);
	return;
    }

    if (!(victim = get_char_world(ch, arg1))) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on mobs.\n\r", ch);
	return;
    }

    duration = atoi(arg2);
    if (duration < 1 || duration > 100) {
	send_to_char("Duration must be between 1 and 100.\n\r", ch);
	return;
    }

    if (victim->level >= ch->level) {
	send_to_char("You can't jail him.\n\r", ch);
	char_from_room(ch);
	char_to_room(ch, get_room_index(ROOM_VNUM_JAIL));
	victim->timer = duration;
	return;
    }

    char_from_room(victim);
    char_to_room(victim, get_room_index(ROOM_VNUM_JAIL));
    ch->timer = duration; 

    /* Suzuran */
    victim->jailtime=duration;

    sprintf(buf, "You have been sent to jail for %d hours.\n\r", duration);
    send_to_char(buf, victim);
    return;
}

void do_necho(CHAR_DATA * ch, char *argument)
{				/*EE960619 */
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    sprintf(buf, "%s> ", ch->name);

    if (argument[0] == '\0') {
	send_to_char("NameEcho what?\n\r", ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING) {
	    send_to_char(buf, d->character);
	    send_to_char(argument, d->character);
	    send_to_char("\n\r", d->character);
	}
    }
    return;
}

void do_qecho(CHAR_DATA * ch, char *argument)
{				/*EE960619 */
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    sprintf(buf, "`!Q`#u`&e`#s`!t``> ");

    if (argument[0] == '\0') {
	send_to_char("QuestEcho what?\n\r", ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING) {
	    send_to_char(buf, d->character);
	    send_to_char(argument, d->character);
	    send_to_char("\n\r", d->character);
	}
    }
    return;
}



void do_lag(CHAR_DATA * ch, char *argument)
{				/*EE960718 */
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int time = 0;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
	send_to_char("Syntax: lag <player> <time>\n\r", ch);
	return;
    }
    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("They aren't here!!\n\r", ch);
	return;
    }
    if (IS_NPC(victim)) {
	send_to_char("Not on mobs.\n\r", ch);
	return;
    }
    if (!is_number(arg2)) {
	send_to_char("Time must be a number!\n\r", ch);
	return;
    }
    time = atoi(arg2);
    if (time < 1) {
	send_to_char("Time must a positive value.\n\r", ch);
	return;
    }
    WAIT_STATE(victim, (time * PULSE_PER_SECOND));
    send_to_char("They are now lagging. *grin*\n\r", ch);
    return;
}

void do_review(CHAR_DATA * ch, char *argument)
{				/*EE960619 */
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
	send_to_char("You are a mob, what do you need a review for?", ch);
	return;
    }

    send_to_char("Here is a review of your settings.\n\r", ch);

    sprintf(buf, "Bamfin     : %s\n\r", ch->pcdata->bamfin);
    send_to_char(buf, ch);

    sprintf(buf, "Bamfout    : %s\n\r", ch->pcdata->bamfout);
    send_to_char(buf, ch);

    sprintf(buf, "Title      :%s\n\r", ch->pcdata->title);
    send_to_char(buf, ch);

    sprintf(buf, "Long Desc  : %s\n\r", ch->long_descr);
    send_to_char(buf, ch);

    sprintf(buf, "Short Desc : %s\n\r", ch->short_descr);
    send_to_char(buf, ch);

    sprintf(buf, "Description: %s\n\r", ch->description);
    send_to_char(buf, ch);

    return;

}

void do_noreply(CHAR_DATA * ch, char *argument)
{				/*EE960619 */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0') {
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if ((victim = d->character) != NULL)
		if (victim->reply == ch)
		    victim->reply = NULL;
	}
	send_to_char("Ok.\r\n", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL || (IS_NPC(victim))) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->desc == NULL && !IS_NPC(victim)) {
	act("$N seems to have misplaced $S link...try again later.",
	    ch, NULL, victim, TO_CHAR);
	return;
    }
    victim->reply = NULL;

    send_to_char("Ok.\r\n", ch);
    return;
}

void do_seize(CHAR_DATA * ch, char *argument)
{				/*EE960619 */
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ((arg1[0] == '\0') || (arg2[0] == '\0')) {
	send_to_char("Syntax : seize <object> <player>\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg2)) == NULL) {
	send_to_char("They are not here!\n\r", ch);
	return;
    }

    if ((obj = get_obj_list(ch, arg1, victim->carrying)) == NULL) {
	send_to_char("They do not have that item.\n\r", ch);
	return;
    }

    if ((victim->level >= ch->level) && (victim != ch)) {
	send_to_char("You failed.\r\n", ch);
	return;
    }

    if (obj->wear_loc != WEAR_NONE)
	unequip_char(victim, obj);

    obj_from_char(obj);
    obj_to_char(obj, ch);

    sprintf(buf,
	    "With a magical hand gesture, %s flys into your hands from %s.\r\n",
	    obj->short_descr, victim->name);
    send_to_char(buf, ch);
    sprintf(buf, "A divine being loosens you from the burden of %s.\r\n",
	    obj->short_descr);
    send_to_char(buf, victim);

    sprintf(buf, "%s seizes %s from %s.", ch->name, obj->short_descr,
	    victim->name);
    wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);

    return;
}

void do_world(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if (IS_NPC(ch)) {
	send_to_char("Switch back ya lamer.", ch);
	return;
    }

    if (argument[0] == '\0') {
	DESCRIPTOR_DATA *d;
	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if (d->character != NULL &&
		d->connected == CON_PLAYING &&
		d->character->in_room != NULL &&
		can_see(ch, d->character) &&
		can_see_room(ch, d->character->in_room)) {
		victim = d->character;
		count++;
		sprintf(buf,
			"`&[`4%3d`&] `^%-10s  `#H`3p`^(`7%d`^/`&%d`^)  `@M`2p`^(`7%d`^/`&%d`^)  `!E`1xp`^(`&%d`^)  `$N`4ext`^(`&%d`^)``\n\r",
			victim->level, victim->name, victim->hit,
			victim->max_hit, victim->mana, victim->max_mana,
			victim->level >= LEVEL_HERO ? 0 : victim->exp,
			ch->level >=
			LEVEL_HERO ? 0
			: (exp_per_level(victim, victim->pcdata->points) *
			   ((victim->level <
			     LEVEL_HERO) ? (victim->level +
					    1) : ((victim->level + 1 -
						   LEVEL_HERO) * 5 +
						  LEVEL_HERO))) -
			victim->exp);
		add_buf(buffer, buf);

		if(victim->pcdata != NULL){
		sprintf(buf,
			"      `!PK`1deaths`&: ``%d  `!PK`1ills`&: ``%d\n\r",
			victim->pcdata->pdeaths, victim->pcdata->pkills);
		add_buf(buffer, buf);
                }

		if (victim->fighting != NULL) {
		    sprintf(buf, "      F`&ighting``: `^%-23s``\n\r",
			    victim->fighting->name);
		    add_buf(buffer, buf);
		}
	    }
	}
	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for (victim = char_list; victim != NULL; victim = victim->next) {
	if (victim->in_room != NULL &&
	    is_name(argument, victim->name) &&
	    can_see(ch, victim) && victim->in_room != NULL) {
	    found = TRUE;
	    count++;
	    sprintf(buf,
		    "`&[`4%3d`&] `^%-10s  `#H`!p`^(`7%d`^/`&%d`^)  `@M`2p`^(`7%d`^/`&%d`^)``  ",
		    victim->level,
		    IS_NPC(victim) ? victim->short_descr : victim->name,
		    victim->hit, victim->max_hit, victim->mana,
		    victim->max_mana);
	    add_buf(buffer, buf);
	    if (IS_NPC(victim)) {
		sprintf(buf, "      `$K`4illed`&: ``%d\n\r",
			victim->pIndexData->killed);
		add_buf(buffer, buf);
	    } else {
		sprintf(buf,
			"`!E`1xp`^(`&%d`^)  `$N`4ext`^(`&%d`^)\n\r      `!PK`1deaths`&: ``%d  `!PK`1ills`&: ``%d\n\r",
			ch->level >= LEVEL_HERO ? 0 : victim->exp,
			ch->level >=
			LEVEL_HERO ? 0
			: (exp_per_level(victim, victim->pcdata->points) *
			   ((victim->level <
			     LEVEL_HERO) ? (victim->level +
					    1) : ((victim->level + 1 -
						   LEVEL_HERO) * 5 +
						  LEVEL_HERO))) -
			victim->exp, victim->pcdata->pdeaths,
			victim->pcdata->pkills);
		add_buf(buffer, buf);
	    }
	    if (victim->fighting != NULL) {
		sprintf(buf, "      F`&ighting``: `^%-23s``\n\r",
			victim->fighting->name);
		add_buf(buffer, buf);
	    }
	}
    }
    if (!found)
	act("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
    else
	page_to_char(buf_string(buffer), ch);

    free_buf(buffer);
    return;
}

void do_fry(CHAR_DATA * ch, char *argument)
{

/* Hacked a little - Suzuran */
    /* Hacked a little more to save fried chars in ../penalty */

    char strsave[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    /* Fetch argument */
    argument = one_argument(argument, arg1);

    /* Look for no parm */
    if (arg1[0] == 0) {
	send_to_char("Target, please?\n\r", ch);
	return;
    }

    if (IS_NPC(ch)) {
	/* Switched imm or some other stupidity */
	send_to_char("Only players can fry people.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("That player is not here.\n\r", ch);
	return;
    }

    if (victim == ch) {
	/* This message sucks!  I'll change it. */
	send_to_char("Use delete for that, it makes less mess.\n\r", ch);
	return;
    }

    /* Level restriction. */
    if (ch->level < 99 && victim->level > 20) {
	send_to_char
	    ("Only immortals level 99 or higher can delete 20s and above.\n\r",
	     ch);
	return;
    }

    /* Save the luser */
    save_char_fried_obj(victim);

    /* Send messages, MANUALLY get rid of the character, and delete the
       pfile.  Forcing a quit command DOES NOT WORK! */

    sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(victim->name));
    wiznet("$N has just been FRIED! `1-`!TOASTY`1-``", victim, NULL, 0, 0,
	   0);

    /* Make sure not to screw up fighting */
    stop_fighting(victim, TRUE);

    /* Entertaining messages to the world - This is safer than forcing
       an echo command.  It's a part of gecho, to everyone not in the
       same room as the victim, and not to the fry-er. */

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING) {
	    if (d->character->in_room != victim->in_room
		&& d->character != ch) {
		send_to_char
		    ("A jagged streak of `&lightning`` flashes across the sky.\n\r",
		     d->character);
	    }
	}
    }

    /* To fry-ee */
    send_to_char
	("Out of nowhere a giant bolt of pure energy `&flashes`` out towards you. The bolt\n\r",
	 victim);
    send_to_char
	("engulfs your body in `$b`4l`$u`4e`` `!f`1l`!a`1m`!e`1s`A...``leaving only `Aashes`` where you stood!\n\r",
	 victim);

    /* To everyone else in the room with the victim */
    act
	("$N takes a massive bolt of `&lightning``!\n\rNothing is left but a puff of black smoke.",
	 victim, NULL, victim, TO_NOTVICT);

    /* And to the fry-er */
    send_to_char("Roasty-toasty!  Got any marshmallows?\n\r", ch);

    /* Get rid of the player or mob */
    if (IS_NPC(victim)) {
	extract_char(victim, TRUE);
	return;
    } else {
	/* Log this */
	sprintf(buf, "%s fried by %s.", victim->name, ch->name);
	log_string(buf);

	/* Lose the loser */
	d = victim->desc;
	extract_char(victim, TRUE);
	if (d != NULL) {
	    close_socket(d);
	}
	/* And the pfile */
	unlink(strsave);
    }

    /* BAD SUZURAN, NO COOKIE! -- SZ053102*/
    /* unlink(strsave); */

    /* All done. */
    return;
}

/*
*
===========================================================================
* The following snippet was written by Erwin S. Andreasen,
erwin@pip.dknet.dk.
* You may use this code freely, as long as you retain his name in all of
the
* files. You also have to mail him telling that you are using it. He's
giving
* this, hopefully useful, piece of source code to you for free, and all
he
* requires from you is some feedback.
*
* Please mail him if you find any bugs or have any new ideas or just
comments.
*
* All Erwin's snippets are publically available at:
*
* http://pip.dknet.dk/~pip1773/
*
* If you do not have WWW access, try ftp'ing to pip.dknet.dk and examine
* the /pub/pip1773 directory.
*
===========================================================================
*
* This snippet was further modified and developed by EE970725 to fit
Prophecy
* MUD and its clones... All 50 of them. I don't care whether anyone uses
this
* or spreads it with my changes in it, Erwin did all the work.
*/


/*
* do_rename renames a player to another name.
* PCs only. Previous file is deleted, if it exists.
* Char is then saved to new file.
* New name is checked against std. checks, existing offline players and
* online players.
* .gz files are checked for too, just in case.
*/

bool check_parse_name(char *name);	/* comm.c */
char *initial(const char *str);	/* comm.c */

void do_rename(CHAR_DATA * ch, char *argument)
{
    char old_name[MAX_INPUT_LENGTH], new_name[MAX_INPUT_LENGTH],
	strsave[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];

    CHAR_DATA *victim;
    FILE *file;

    argument = one_argument(argument, old_name);	/* find new/old name */
    one_argument(argument, new_name);

    /* Trivial checks */
    if (!old_name[0]) {
	send_to_char("Rename who?\n\r", ch);
	return;
    }

    victim = get_char_world(ch, old_name);

    if (!victim) {
	send_to_char("There is no such a person online.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("You cannot use Rename on NPCs.\n\r", ch);
	return;
    }

    /*
       * Clan leaders and imms that aren't coders or security can
       * only rename unvalidated chars.
     */

    if ((get_trust(ch) < ADMIN) && !IS_SET(victim->epf, PPL_VALIDATION)) {
	send_to_char("You can only rename unvalidated chars.\n\r", ch);
	return;
    }

    /* allow rename self new_name,but otherwise only lower level */
    if ((victim != ch) && (get_trust(victim) >= get_trust(ch))) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (!victim->desc || (victim->desc->connected != CON_PLAYING)) {
	send_to_char
	    ("This player has lost his link or the like. Wait a few minutes and try again.",
	     ch);
	return;
    }

    if (!new_name[0]) {
	send_to_char("Rename to what new name?\n\r", ch);
	return;
    }

    if (!check_parse_name(new_name)) {
	send_to_char("The new name is illegal.\n\r", ch);
	return;
    }

    /* First, check if there is a player named that off-line */
    sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(new_name));

    /*
       * Prophecy runs on a dedicated server so we need not to
       * worry about fpReserve. The imps on Anime, Paths of Darkness,
       * TSS, Knights of Camelot and other present and upcoming clones
       * will have to decide for themselves.
     */
/* fclose (fpReserve); *//* close the reserve file */
    file = fopen(strsave, "r");	/* attempt to to open pfile */
    if (file) {
	send_to_char("A player with that name already exists!\n\r", ch);
	fclose(file);
	/* fpReserve = fopen( NULL_FILE, "r" ); */
	return;
    }
    /* fpReserve = fopen( NULL_FILE, "r" ); */

    /* Check .gz file */
    sprintf(strsave, "%s%s.gz", PLAYER_DIR, capitalize(new_name));

    /* fclose (fpReserve); */
    file = fopen(strsave, "r");	/* attempt to to open pfile */
    if (file) {
	send_to_char
	    ("A player with that name already exists in a compressed file!\n\r",
	     ch);
	fclose(file);
	/* fpReserve = fopen( NULL_FILE, "r" ); */
	return;
    }

    /* fpReserve = fopen( NULL_FILE, "r" ); */

    if (get_char_world(ch, new_name)) {	/* check for playing level-1 non-saved
					 */
	send_to_char
	    ("A player with the name you specified already exists!\n\r",
	     ch);
	return;
    }

    /* Save the filename of the old name */
    /* Send this first */
    sprintf(buf, "%s renames %s to %s.", ch->name, old_name, new_name);
    wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);

    sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(victim->name));

    /* Rename the character and save him to a new file */
    /* NOTE: Players who are level 1 do NOT get saved under a new name */

    free_string(victim->name);
    victim->name = str_dup(capitalize(new_name));
    save_char_obj(victim);

    /* unlink the old file */
    unlink(strsave);

    /* That's it! */
    send_to_char("Character renamed.\n\r", ch);
    victim->position = POS_STANDING;
    act("$n has renamed you to $N!", ch, NULL, victim, TO_VICT);
    return;
}				/* do_rename */

/* 
 * Grant procs
 */
const struct pair_type pair_table[] = {
    {"switch", "return", FALSE},
    {"reboo", "reboot", FALSE},
    {"shutdow", "shutdown", FALSE},
    {"sla", "slay", FALSE},
    {"", "", FALSE}
};

bool is_granted(CHAR_DATA * ch, DO_FUN * do_fun)
{
    GRANT_DATA *gran;

    if (ch->desc == NULL)
	return FALSE;

    if (ch->desc->original != NULL)
	ch = ch->desc->original;

    for (gran = ch->pcdata->granted; gran != NULL; gran = gran->next)
	if (do_fun == gran->do_fun)
	    return TRUE;

    return FALSE;
}

bool is_granted_name(CHAR_DATA * ch, char *name)
{
    GRANT_DATA *gran;

    if (ch->desc == NULL)
	return FALSE;

    if (ch->desc->original != NULL)
	ch = ch->desc->original;

    for (gran = ch->pcdata->granted; gran != NULL; gran = gran->next)
	if (is_exact_name(gran->name, name))
	    return TRUE;

    return FALSE;
}

int grant_duration(CHAR_DATA * ch, DO_FUN * do_fun)
{
    GRANT_DATA *gran;

    if (ch->desc->original != NULL)
	ch = ch->desc->original;

    /*  Replace the x's in the line below with the name of
       a character that is allowed to grant commands to
       anyone, even if they don't have the command
       themselves.  This is useful when you add new
       imm commands, and need to give them to yourself.
       Additional names can be added as needed and
       should be seperated by spaces.  */

    if (is_exact_name(ch->name, "Belldandy") ||
	is_exact_name(ch->name, "Biryu"))
	return -1;

    for (gran = ch->pcdata->granted; gran != NULL; gran = gran->next)
	if (gran->do_fun == do_fun)
	    return gran->duration;

    return 0;
}

void
grant_add(CHAR_DATA * ch, char *name, DO_FUN * do_fun, int duration,
	  int level)
{
    GRANT_DATA *gran;

    if (ch->desc->original != NULL)
	ch = ch->desc->original;

    gran = alloc_mem(sizeof(*gran));
    gran->name = str_dup(name);
    gran->do_fun = do_fun;
    gran->duration = duration;
    gran->level = level;

    gran->next = ch->pcdata->granted;
    ch->pcdata->granted = gran;

    return;
}

void grant_remove(CHAR_DATA * ch, DO_FUN * do_fun, bool mshow)
{
    GRANT_DATA *p, *gran;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *rch;

    rch = ch->desc->original ? ch->desc->original : ch;

    p = NULL;
    gran = rch->pcdata->granted;
    if (gran->do_fun == do_fun)
	rch->pcdata->granted = gran->next;
    else
	for (gran = rch->pcdata->granted; gran != NULL; gran = gran->next) {
	    if (gran->do_fun == do_fun)
		break;
	    p = gran;
	}

    if (p != NULL)
	p->next = gran->next;
    sprintf(buf, "You have lost access to the %s command.\n\r",
	    gran->name);
    if (mshow)
	send_to_char(buf, ch);
    free_string(gran->name);
    free_mem(gran, sizeof(*gran));
    return;
}

void
grant_level(CHAR_DATA * ch, CHAR_DATA * victim, int level, int duration)
{
    int cmd;

    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
	if (cmd_table[cmd].level == level
	    && !is_granted(victim, cmd_table[cmd].do_fun)
	    && grant_duration(ch, cmd_table[cmd].do_fun) == -1)
	    grant_add(victim, cmd_table[cmd].name, cmd_table[cmd].do_fun,
		      duration, cmd_table[cmd].level);
    return;
}

void revoke_level(CHAR_DATA * ch, CHAR_DATA * victim, int level)
{
    int cmd;

    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
	if (cmd_table[cmd].level == level
	    && is_granted(victim, cmd_table[cmd].do_fun)
	    && grant_duration(ch, cmd_table[cmd].do_fun) == -1)
	    grant_remove(victim, cmd_table[cmd].do_fun, FALSE);
    return;
}

void do_grant(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL, *rch, *rvictim = NULL;
    int dur, cmd, x;
    bool found = FALSE;
    DESCRIPTOR_DATA *d;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    one_argument(argument, arg3);

    rch = ch->desc->original ? ch->desc->original : ch;

    if (arg1[0] == '\0') {
	send_to_char("Grant who, what?\n\r", ch);
	return;
    }

    for (d = descriptor_list; d != NULL; d = d->next) {
	rvictim = d->original ? d->original : d->character;

	if (rvictim == NULL)
	    continue;

	if (!str_cmp(rvictim->name, arg1)) {
	    victim = d->character;
	    break;
	}
    }

    if (victim == NULL && !str_cmp("self", arg1)) {
	rvictim = rch;
	victim = ch;
    }

    if (victim == NULL) {
	send_to_char("Victim not found.\n\r", ch);
	return;
    }

    if (arg2[0] == '\0') {
	int col = 0;
	int lvl;

	sprintf(buf, "%s has not been granted the following commands:\n\r",
		rvictim->name);
	send_to_char(buf, ch);

	for (lvl = IM; lvl <= (100 + 1); lvl++)
	    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
		if (cmd_table[cmd].level >= LEVEL_IMMORTAL
		    && !is_granted(victim, cmd_table[cmd].do_fun)
		    && cmd_table[cmd].level == lvl) {
		    sprintf(buf, "[L%3d] %-12s", cmd_table[cmd].level,
			    cmd_table[cmd].name);
		    send_to_char(buf, ch);
		    if (++col % 4 == 0)
			send_to_char("\n\r", ch);
		}
	if (col % 4 != 0)
	    send_to_char("\n\r", ch);
	return;
    }

    dur = arg3[0] == '\0' ? -1 : is_number(arg3) ? atoi(arg3) : 0;

    if (dur < 1 && dur != -1) {
	send_to_char("Invalid duration!\n\r", ch);
	return;
    }

    if (is_number(arg2)) {
	if (atoi(arg2) < LEVEL_IMMORTAL || atoi(arg2) > MAX_LEVEL) {
	    send_to_char("Invalid grant level.\n\r", ch);
	    return;
	}
	grant_level(ch, victim, atoi(arg2), dur);
	sprintf(buf, "You have been granted level %d commands.\n\r",
		atoi(arg2));
	send_to_char("Ok.\n\r", ch);
	send_to_char(buf, victim);
	return;
    }

    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
	if (arg2[0] == cmd_table[cmd].name[0]
	    && is_exact_name(arg2, cmd_table[cmd].name)) {
	    found = TRUE;
	    break;
	}

    if (found) {
	if (cmd_table[cmd].level < LEVEL_IMMORTAL) {
	    send_to_char("You can only grant immortal commands.\n\r", ch);
	    return;
	}

	if (grant_duration(ch, cmd_table[cmd].do_fun) != -1) {
	    send_to_char("You can't grant that!\n\r", ch);
	    return;
	}

	if (is_granted(victim, cmd_table[cmd].do_fun)) {
	    send_to_char("They already have that command!\n\r", ch);
	    return;
	}

	grant_add(victim, cmd_table[cmd].name, cmd_table[cmd].do_fun,
		  dur, cmd_table[cmd].level);

	sprintf(buf, "%s has been granted the %s command.\n\r",
		rvictim->name, cmd_table[cmd].name);
	send_to_char(buf, ch);
	sprintf(buf, "%s has granted you the %s command.\n\r", rch->name,
		cmd_table[cmd].name);
	send_to_char(buf, victim);

	for (x = 0; pair_table[x].first[0] != '\0'; x++)
	    if (!str_cmp(arg2, pair_table[x].first)
		&& !is_granted_name(victim, pair_table[x].second)) {
		sprintf(buf, "%s %s %s", rvictim->name,
			pair_table[x].second, arg3);
		do_grant(ch, buf);
	    } else if (!str_cmp(arg2, pair_table[x].second)
		       && pair_table[x].one_way != TRUE
		       && !is_granted_name(victim, pair_table[x].first)) {
		sprintf(buf, "%s %s %s", rvictim->name,
			pair_table[x].first, arg3);
		do_grant(ch, buf);
	    }

	return;
    }
    send_to_char("Command not found!\n\r", ch);
    return;
}

void do_revoke(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL, *rvictim = NULL;
    DESCRIPTOR_DATA *d;
    int cmd, x;
    bool had_return, found = FALSE;

    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (arg1[0] == '\0') {
	send_to_char("Revoke who, what?\n\r", ch);
	return;
    }

    for (d = descriptor_list; d != NULL; d = d->next) {
	rvictim = d->original ? d->original : d->character;

	if (rvictim == NULL)
	    continue;

	if (!str_cmp(rvictim->name, arg1)) {
	    victim = d->character;
	    break;
	}
    }

    if (victim == NULL && !str_cmp("self", arg1)) {
	rvictim = ch->desc->original ? ch->desc->original : ch;
	victim = ch;
    }

    if (victim == NULL) {
	send_to_char("Victim not found.\n\r", ch);
	return;
    }

    had_return = is_granted_name(victim, "return");

    if (arg2[0] == '\0') {
	int col = 0, lvl;
	char buf[MAX_STRING_LENGTH];

	sprintf(buf, "%s has been granted the following commands:\n\r",
		rvictim->name);
	send_to_char(buf, ch);

	for (lvl = IM; lvl <= (100 + 1); lvl++)
	    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
		if (cmd_table[cmd].level >= LEVEL_IMMORTAL
		    && is_granted(victim, cmd_table[cmd].do_fun)
		    && cmd_table[cmd].level == lvl) {
		    sprintf(buf, "[L%3d] %-12s", cmd_table[cmd].level,
			    cmd_table[cmd].name);
		    send_to_char(buf, ch);
		    if (++col % 4 == 0)
			send_to_char("\n\r", ch);
		}
	if (col % 4 != 0)
	    send_to_char("\n\r", ch);
	return;
    }

    if (is_number(arg2)) {
	char buf[MAX_STRING_LENGTH];

	if (atoi(arg2) < LEVEL_IMMORTAL || atoi(arg2) > MAX_LEVEL) {
	    send_to_char("Invalid revoke level.\n\r", ch);
	    return;
	}
	revoke_level(ch, victim, atoi(arg2));
	sprintf(buf, "You have lost acess to level %d commands.\n\r",
		atoi(arg2));
	send_to_char("Ok.\n\r", ch);
	send_to_char(buf, victim);

	if (had_return && !is_granted_name(victim, "return") &&
	    rvictim != victim) do_return(victim, "");

	return;
    }

    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
	if (arg2[0] == cmd_table[cmd].name[0]
	    && is_exact_name(arg2, cmd_table[cmd].name)) {
	    found = TRUE;
	    break;
	}

    if (found) {
	char buf[MAX_STRING_LENGTH];

	if (grant_duration(ch, cmd_table[cmd].do_fun) != -1) {
	    send_to_char("You can't revoke that!\n\r", ch);
	    return;
	}

	if (!is_granted(victim, cmd_table[cmd].do_fun)) {
	    send_to_char("They don't have that command!\n\r", ch);
	    return;
	}

	grant_remove(victim, cmd_table[cmd].do_fun, TRUE);

	sprintf(buf, "%s has lost access to the %s command.\n\r",
		rvictim->name, cmd_table[cmd].name);
	send_to_char(buf, ch);

	for (x = 0; pair_table[x].first[0] != '\0'; x++)
	    if (!str_cmp(arg2, pair_table[x].first)
		&& is_granted_name(victim, pair_table[x].second)) {
		sprintf(buf, "%s %s", rvictim->name, pair_table[x].second);
		do_revoke(ch, buf);
	    } else if (!str_cmp(arg2, pair_table[x].second)
		       && pair_table[x].one_way != TRUE
		       && is_granted_name(victim, pair_table[x].first)) {
		sprintf(buf, "%s %s", rvictim->name, pair_table[x].first);
		do_revoke(ch, buf);
	    }

	if (had_return && !is_granted_name(victim, "return") &&
	    rvictim != victim) do_return(victim, "");

	return;
    }
    send_to_char("Command not found!\n\r", ch);
    return;
}


void do_gstat(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    GRANT_DATA *grant;
    CHAR_DATA *victim;
    int col = 0;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Gstat who?\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on mobs.\n\r", ch);
	return;
    }

    if (get_trust(ch) < get_trust(victim)) {
	send_to_char("You can't do that.\n\r", ch);
	return;
    }

    buffer = new_buf();

    sprintf(buf, "Grant status for %s:\n\r\n\r", victim->name);

    add_buf(buffer, buf);

    for (grant = victim->pcdata->granted; grant != NULL;
	 grant = grant->next) {
	char ds[50], ss[25], s2[25];
	int x, sl;

	sprintf(ds, "%d", grant->duration);
	ss[0] = '\0';
	sl = (int) ((6 - strlen(ds)) / 2);

	for (x = 0; x < sl; x++)
	    strcat(ss, " ");

	strcpy(s2, ss);

	if ((strlen(ss) + strlen(ds)) % 2 == 1)
	    strcat(s2, " ");

	if (grant->duration == -1)
	    sprintf(buf, "[ perm ] %-11s", grant->name);
	else
	    sprintf(buf, "[%s%d%s] %-11s", ss, grant->duration, s2,
		    grant->name);

	add_buf(buffer, buf);

	col++;
	col %= 4;

	if (col == 0)
	    add_buf(buffer, "\n\r");
    }

    if (col != 0)
	add_buf(buffer, "\n\r");

    page_to_char(buf_string(buffer), ch);

    free_buf(buffer);
    return;
}

/*TG980410*/
void do_btitle(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char *pArg;
    char cEnd;
    int x = 0;

    pArg = arg1;
    while (isspace(*argument))
	argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
	cEnd = *argument++;

    while (*argument != '\0') {
	if (*argument == cEnd) {
	    argument++;
	    break;
	}
	if ((*argument != '`') && (*(argument - 1) != '`'))
	    x++;
	*pArg++ = *argument++;
    }
    for (x = x; x <= 12; x++)
	*pArg++ = ' ';
    *pArg = '\0';


    if (IS_NPC(ch)) {
	send_to_char("NPCs can't use this command!\n\r", ch);
	return;
    }

    if (arg1[0] == '\0') {
	send_to_char("Syntax: btitle <arguement>\n\r", ch);
	return;
    }

    if (smash_color(arg1) > 13) {
	send_to_char("Invalid bracket title. String too long.\n\r", ch);
	return;
    }

    free_string(ch->pcdata->bracket_title);
    ch->pcdata->bracket_title = str_dup(arg1);

    return;
}

/*TG980628*/
void do_award(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int points;

    one_argument(argument, arg1);

    if (IS_NPC(ch))
	return;
    if (!is_granted_name(ch, "award")) {
	send_to_char("Huh?\n\r", ch);
	return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("You cannot use this command while charmed.\n\r", ch);
	return;
    }
    if (arg1[0] == '\0') {
	send_to_char("Syntax:  award <victim>\n\r", ch);
	return;
    }
    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    if ((ch == victim) && !IS_IMMORTAL(ch)) {
	send_to_char("You can't award yourself points.\n\r", ch);
	return;
    }

    if (victim->next_points > 0) {
	sprintf(buf, "%d minutes left till %s can be awarded again.\n\r",
		victim->next_points, victim->name);
	send_to_char(buf, ch);
	return;
    }

    if (victim == ch) {
	send_to_char("You have no need for rp points, you luser.\n\r", ch);
	return;
    }


    points = number_range(5, 15);
    sprintf(buf, "You have been awarded %d rp points!\n\r", points);
    send_to_char(buf, victim);
    sprintf(buf, "%s has been awarded %d rp points!\n\r", victim->name,
	    points);
    send_to_char(buf, ch);
    victim->rp_points += points;
    victim->next_points = 15;
    sprintf(buf, "%s awards %s %d RP points.", ch->name, victim->name,
	    points);
    wiznet(buf, ch, NULL, WIZ_FLAGS, 0, 0);
    return;
}

void do_validate(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (IS_NPC(ch))
	return;

    if (arg[0] == '\0') {
	send_to_char("Syntax: validate <character>.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r", ch);
	return;
    }

    if (!IS_VALIDATION(victim)) {
	send_to_char("That character is already validated!\n\r", ch);
	return;
    }
    REMOVE_BIT(victim->epf, PPL_VALIDATION);
    send_to_char("They have now been validated.\n\r", ch);

    /* These two lines are Suzuran's fault. */
    sprintf(buf, "$N validates %s.", victim->name);
    wiznet(buf, ch, NULL, WIZ_NEWBIE, WIZ_SECURE, 0);

    sprintf(buf, "`#Name Validation`8: `&Accepted`8!``\n\r\
You may now `!PK``set, use channels, and notes.\n\r");
    send_to_char(buf, victim);
    return;
}

void do_linkload(CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA d;
    bool isChar = FALSE;
    char name[MAX_INPUT_LENGTH];

    if (argument[0] == '\0') {
	send_to_char("Load who?\n\r", ch);
	return;
    }

    argument[0] = UPPER(argument[0]);
    argument = one_argument(argument, name);

    /* Dont want to load a second copy of a player who's allready online! */
    if (get_char_world(ch, name) != NULL) {
	send_to_char("That person is allready connected!\n\r", ch);
	return;
    }

    isChar = load_char_obj(&d, name);	/* char pfile exists? */

    if (!isChar) {
	send_to_char
	    ("Load Who? Are you sure? I cant seem to find them.\n\r", ch);
	return;
    }

    d.character->desc = NULL;
    d.character->next = char_list;
    char_list = d.character;
    d.connected = CON_PLAYING;
    reset_char(d.character);

    /* bring player to imm */

    if (d.character->in_room != NULL) {
	char_to_room(d.character, ch->in_room);	/* put in room imm is in */
    }

    act("You have sketched `@$N`` into existance!", ch, NULL, d.character,
	TO_CHAR);
    act("$n has sketched `@$N`` into existance!", ch, NULL, d.character,
	TO_ROOM);

    if (d.character->pet != NULL) {
	char_to_room(d.character->pet, d.character->in_room);
	act("$n has entered the game.", d.character->pet, NULL, NULL,
	    TO_ROOM);
    }

}

void do_linkdrop(CHAR_DATA * ch, char *argument)
{

    /* Hacked a little - Suzuran */

    CHAR_DATA *victim;
    char who[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument(argument, who);

    if ((victim = get_char_world(ch, who)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

  /** Person is legitametly logged on... was not ploaded.
   */
    if (victim->desc != NULL) {
	send_to_char("I dont think that would be a good idea...\n\r", ch);
	return;
    }

    if (victim->was_in_room != NULL) {	/* return player and pet to orig room
					 */
	char_to_room(victim, victim->was_in_room);
	if (victim->pet != NULL)
	    char_to_room(victim->pet, victim->was_in_room);
    }

    save_char_obj(victim);
    act("You have cleared your sketchpad of `@$N``.", ch, NULL, victim,
	TO_CHAR);
    act("$n has cleared their sketchpad of `@$N``.", ch, NULL, victim,
	TO_ROOM);

    /* USING DO_QUIT FOR THIS LOOPS THE MUD IN ODD CIRCUMSTANCES.
       DON'T USE DO_QUIT FOR ABNORMAL PLAYER QUITTING! 
       do_quit(victim,"");
     */

    /* Get rid of the player or mob */

    /* I know they won't have a desc, but I'm paranoid anyway. */

    if (IS_NPC(victim)) {
	extract_char(victim, TRUE);
	return;
    } else {
	/* Log this */
	sprintf(buf, "%s linkdropped by %s.", victim->name, ch->name);
	log_string(buf);

	/* Lose the loser */
	d = victim->desc;
	extract_char(victim, TRUE);
	if (d != NULL) {
	    close_socket(d);
	}
    }

    return;
}


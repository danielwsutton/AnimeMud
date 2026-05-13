#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "lookup.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_kill);
DECLARE_DO_FUN(do_say);

char *const dir_name[] = {
    "north", "east", "south", "west", "up", "down"
};

const sh_int rev_dir[] = {
    2, 3, 0, 1, 5, 4
};

const sh_int movement_loss[SECT_MAX] = {
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 10
};



/*
 * Local functions.
 */
void drag_char
args((CHAR_DATA * ch, CHAR_DATA * victim, int door, bool follow));
void push_char
args((CHAR_DATA * ch, CHAR_DATA * victim, int door, bool follow));
int find_door args((CHAR_DATA * ch, char *arg));
bool has_key args((CHAR_DATA * ch, int key));



void move_char(CHAR_DATA * ch, int door, bool follow)
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    char win[MAX_STRING_LENGTH], wout[MAX_STRING_LENGTH];
    int vn;			/*EE960801 */
    char buf[MAX_STRING_LENGTH];	/*EE960801 */

    strcpy(win, "$n has arrived.");
    strcpy(wout, "$n leaves $T.");

    vn = ch->in_room->vnum;	/*EE960801 */

    if (door < 0 || door > 5) {
	bug("Do_move: bad door %d.", door);
	return;
    }

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    in_room = ch->in_room;

    if ((pexit = in_room->exit[door]) == NULL
	|| (to_room = pexit->u1.to_room) == NULL
	|| !can_see_room(ch, pexit->u1.to_room)) {
	send_to_char("Alas, you cannot go that way.\n\r", ch);
	return;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
	&& (!IS_AFFECTED(ch, AFF_PASS_DOOR)
	    || IS_SET(pexit->exit_info, EX_NOPASS))
	&& !IS_TRUSTED(ch, ANGEL)) {
	act("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM)
	&& ch->master != NULL && in_room == ch->master->in_room) {
	send_to_char("What?  And leave your beloved master?\n\r", ch);
	return;
    }

    if (IS_SET(to_room->room_flags, ROOM_SAFE) && ch->fight > 0) {
	send_to_char
	    ("Those engaged in combat are NOT permitted to enter here.\n\r",
	     ch);
	send_to_char("Come back when you've chilled down a bit.\n\r", ch);
	return;
    }

    if (!is_room_owner(ch, to_room) && room_is_private(to_room)) {
	if (ch->master != NULL && ch->master->mount == IS_MOUNTED) {
	    ch->master->mount = NOT_MOUNTED;
	    ch->leader = NULL;
	    send_to_char("You dismount and enter the room.\n\r",
			 ch->master);
	}
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    if (!IS_IMMORTAL(ch) && IS_SET(to_room->room_flags, ROOM_CLAN_ONLY)
	&& ((ch->clan == 0)
/*		||  (IS_SET(to_room->room_flags, ROOM_108) && ch->clan != clan_lookup("108 dragons"))	
		||	(IS_SET(to_room->room_flags, ROOM_JUDGE) && ch->clan != clan_lookup("Judge"))
		||	(IS_SET(to_room->room_flags, ROOM_MDS) && ch->clan != clan_lookup("Mds"))
		||	(IS_SET(to_room->room_flags, ROOM_SAILOR) && ch->clan != clan_lookup("Sailor"))
		||	(IS_SET(to_room->room_flags, ROOM_VAMPIRE) && ch->clan != clan_lookup("Vampire Hunt"))
		||	(IS_SET(to_room->room_flags, ROOM_INVID) && ch->clan != clan_lookup("Invid"))
		||	(IS_SET(to_room->room_flags, ROOM_SHINOBI) && ch->clan != clan_lookup("Shinobi"))
		||	(IS_SET(to_room->room_flags, ROOM_SHOGUN) && ch->clan != clan_lookup("Shogun"))
		||	(IS_SET(to_room->room_flags, ROOM_HUNTER_WARRIOR) && ch->clan != clan_lookup("Hunter-Warrior"))		)) */
	    || (to_room->clan != ch->clan))) {
	send_to_char("You aren't allowed in there.\n\r", ch);
	return;
    }


    if (!IS_NPC(ch)) {
	int iClass, iGuild;
	int move;

	for (iClass = 0; iClass < MAX_CLASS; iClass++) {
	    for (iGuild = 0; iGuild < MAX_GUILD; iGuild++) {
		if (iClass != ch->class
		    && to_room->vnum == class_table[iClass].guild[iGuild]) {
		    send_to_char("You aren't allowed in there.\n\r", ch);
		    return;
		}
	    }
	}

	if (in_room->sector_type == SECT_INSIDE
	    || to_room->sector_type == SECT_INSIDE) {
	    if (!IS_NPC(ch)
		&& (ch->horse != NULL && ch->mount == IS_MOUNTED)
		&& !IS_SET(to_room->room_flags, ROOM_STABLE)) {
		sprintf(buf, "You can't take %s inside.\n\r",
			ch->horse->short_descr);
		send_to_char(buf, ch);
		return;
	    }
	}

	if (in_room->sector_type == SECT_AIR
	    || to_room->sector_type == SECT_AIR) {
	    if (ch->horse != NULL && ch->mount == IS_MOUNTED
		&& !IS_AFFECTED(ch->horse, AFF_FLYING)) {
		send_to_char("But your horse isn't flying.\n\r", ch);
		return;
	    }

	    if (!IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch)
		&& ch->mount == NOT_MOUNTED) {
		send_to_char("You can't fly.\n\r", ch);
		return;
	    }
	}

	if ((in_room->sector_type == SECT_WATER_NOSWIM
	     || to_room->sector_type == SECT_WATER_NOSWIM)
	    && !IS_AFFECTED(ch, AFF_FLYING)) {
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->item_type == ITEM_BOAT) {
		    found = TRUE;
		    break;
		}
	    }
	    if (!found) {
		send_to_char("You need a boat to go there.\n\r", ch);
		return;
	    }
	}

	move = movement_loss[UMIN(SECT_MAX - 1, in_room->sector_type)]
	    + movement_loss[UMIN(SECT_MAX - 1, to_room->sector_type)];

	move /= 2;		/* i.e. the average */


	/* conditional effects */
	if (IS_AFFECTED(ch, AFF_FLYING) || IS_AFFECTED(ch, AFF_HASTE))
	    move /= 2;

	if (IS_AFFECTED(ch, AFF_SLOW))
	    move *= 2;

	if (!IS_NPC(ch) && (ch->horse != NULL) && ch->mount == IS_MOUNTED
	    && ch->horse->move < move) {
	    sprintf("%s is too tired to move.\n\r",
		    ch->horse->short_descr);
	    send_to_char(buf, ch);
	    return;
	}
	if (!IS_NPC(ch) && (ch->horse != NULL) && ch->mount == IS_MOUNTED)
	    move = 0;

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_STABLED_HORSE))
	    return;

	if (ch->move < move) {
	    send_to_char("You are too exhausted.\n\r", ch);
	    return;
	}

	WAIT_STATE(ch, 1);
	if (IS_NPC(ch) && IS_SET(ch->act, ACT_HORSE))
	    move *= 2;

	ch->move -= move;
    }

    if (!IS_AFFECTED(ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO) {
	if (!IS_NPC(ch) && (ch->horse != NULL) && ch->mount == IS_MOUNTED) {
	    if (IS_AFFECTED(ch->horse, AFF_FLYING)) {
		sprintf(buf, "$n flies %s on %s.\n\r", dir_name[door],
			ch->horse->short_descr);
	    } else {
		sprintf(buf, "$n leaves %s on %s.\n\r", dir_name[door],
			ch->horse->short_descr);
	    }
	    act(buf, ch, NULL, NULL, TO_ROOM);
	} else {
	    if (!IS_NPC(ch) || (IS_NPC(ch) && !IS_SET(ch->act, ACT_HORSE))
		|| (IS_NPC(ch) && IS_SET(ch->act, ACT_HORSE)
		    && ch->leader == NULL)) {
		if (IS_AFFECTED(ch, AFF_FLYING))
		    act("$n flies $T.", ch, NULL, dir_name[door], TO_ROOM);
		else
		    act("$n leaves $T.", ch, NULL, dir_name[door],
			TO_ROOM);
	    }
	}
    }

    char_from_room(ch);
    char_to_room(ch, to_room);
    if (!IS_AFFECTED(ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO) {
	if (!IS_NPC(ch) && (ch->horse != NULL) && ch->mount == IS_MOUNTED) {
	    /* DC960611 * Added for visual affects. */
	    if (IS_AFFECTED(ch->horse, AFF_FLYING))
		sprintf(buf, "$n flies in on %s\n\r",
			ch->horse->short_descr);
	    else
		sprintf(buf, "$n rides in on %s\n\r",
			ch->horse->short_descr);
	    act(buf, ch, NULL, NULL, TO_ROOM);
	} else {
	    if (!IS_NPC(ch) || (IS_NPC(ch) && !IS_SET(ch->act, ACT_HORSE))
		|| (IS_NPC(ch) && IS_SET(ch->act, ACT_HORSE)
		    && ch->leader == NULL)) {
		if (IS_AFFECTED(ch, AFF_FLYING))
		    act("$n flies into the room.", ch, NULL,
			dir_name[door], TO_ROOM);
		else
		    act("$n has arrived.", ch, NULL, dir_name[door],
			TO_ROOM);
	    }
	}
    }
    sprintf(buf, "%d", vn);
    /*do_look_directional( ch, buf ); */
    do_look(ch, "auto");

    if (in_room == to_room)	/* no circular follows */
	return;

    for (fch = in_room->people; fch != NULL; fch = fch_next) {
	fch_next = fch->next_in_room;

	if (fch->master == ch && IS_AFFECTED(fch, AFF_CHARM)
	    && fch->position < POS_STANDING)
	    do_stand(fch, "");

	if (fch->master == ch && fch->position == POS_STANDING
	    && can_see_room(fch, to_room)) {

	    if (IS_SET(ch->in_room->room_flags, ROOM_LAW)
		&& (IS_NPC(fch) && IS_SET(fch->act, ACT_AGGRESSIVE))) {
		act("You can't bring $N into the city.",
		    ch, NULL, fch, TO_CHAR);
		act("You aren't allowed in the city.",
		    fch, NULL, NULL, TO_CHAR);
		continue;
	    }
	    if (IS_NPC(fch) && IS_SET(fch->act, ACT_STABLED_HORSE))
		continue;

	    if (IS_NPC(fch) && IS_SET(fch->act, ACT_HORSE)
		&& (fch->in_room->sector_type == SECT_INSIDE
		    || to_room->sector_type == SECT_INSIDE)) {
		act("You can't bring $N indoors.", ch, NULL, fch, TO_CHAR);
		act("You aren't allowed indoors.", fch, NULL, NULL,
		    TO_CHAR);
		continue;
	    }

	    act("You follow $N.", fch, NULL, ch, TO_CHAR);
	    move_char(fch, door, TRUE);
	}
    }

    return;
}



void do_north(CHAR_DATA * ch, char *argument)
{
    move_char(ch, DIR_NORTH, FALSE);
    return;
}



void do_east(CHAR_DATA * ch, char *argument)
{
    move_char(ch, DIR_EAST, FALSE);
    return;
}



void do_south(CHAR_DATA * ch, char *argument)
{
    move_char(ch, DIR_SOUTH, FALSE);
    return;
}



void do_west(CHAR_DATA * ch, char *argument)
{
    move_char(ch, DIR_WEST, FALSE);
    return;
}



void do_up(CHAR_DATA * ch, char *argument)
{
    move_char(ch, DIR_UP, FALSE);
    return;
}



void do_down(CHAR_DATA * ch, char *argument)
{
    move_char(ch, DIR_DOWN, FALSE);
    return;
}



int find_door(CHAR_DATA * ch, char *arg)
{
    EXIT_DATA *pexit;
    int door;

    if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
	door = 0;
    else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
	door = 1;
    else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
	door = 2;
    else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
	door = 3;
    else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
	door = 4;
    else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
	door = 5;
    else {
	for (door = 0; door <= 5; door++) {
	    if ((pexit = ch->in_room->exit[door]) != NULL
		&& IS_SET(pexit->exit_info, EX_ISDOOR)
		&& pexit->keyword != NULL && is_name(arg, pexit->keyword))
		return door;
	}
	act("I see no $T here.", ch, NULL, arg, TO_CHAR);
	return -1;
    }

    if ((pexit = ch->in_room->exit[door]) == NULL) {
	act("I see no door $T here.", ch, NULL, arg, TO_CHAR);
	return -1;
    }

    if (!IS_SET(pexit->exit_info, EX_ISDOOR)) {
	send_to_char("You can't do that.\n\r", ch);
	return -1;
    }

    return door;
}



void do_open(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument(argument, arg);

    REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if (arg[0] == '\0') {
	send_to_char("Open what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_here(ch, arg)) != NULL) {
	/* open portal */
	if (obj->item_type == ITEM_PORTAL) {
	    if (!IS_SET(obj->value[1], EX_ISDOOR)) {
		send_to_char("You can't do that.\n\r", ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED)) {
		send_to_char("It's already open.\n\r", ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED)) {
		send_to_char("It's locked.\n\r", ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_CLOSED);
	    act("You open $p.", ch, obj, NULL, TO_CHAR);
	    act("$n opens $p.", ch, obj, NULL, TO_ROOM);
	    return;
	}

	/* 'open object' */
	if (obj->item_type != ITEM_CONTAINER
	    && obj->item_type != ITEM_SADDLE) {
	    send_to_char("That's not a container.\n\r", ch);
	    return;
	}
	if (!IS_SET(obj->value[1], CONT_CLOSED)) {
	    send_to_char("It's already open.\n\r", ch);
	    return;
	}
	if (!IS_SET(obj->value[1], CONT_CLOSEABLE)) {
	    send_to_char("You can't do that.\n\r", ch);
	    return;
	}
	if (IS_SET(obj->value[1], CONT_LOCKED)) {
	    send_to_char("It's locked.\n\r", ch);
	    return;
	}

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act("You open $p.", ch, obj, NULL, TO_CHAR);
	act("$n opens $p.", ch, obj, NULL, TO_ROOM);
	return;
    }

    if ((door = find_door(ch, arg)) >= 0) {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
	    send_to_char("It's already open.\n\r", ch);
	    return;
	}
	if (IS_SET(pexit->exit_info, EX_LOCKED)) {
	    send_to_char("It's locked.\n\r", ch);
	    return;
	}

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act("$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM);
	send_to_char("Ok.\n\r", ch);

	/* open the other side */
	if ((to_room = pexit->u1.to_room) != NULL
	    && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	    && pexit_rev->u1.to_room == ch->in_room) {
	    CHAR_DATA *rch;

	    REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
	    for (rch = to_room->people; rch != NULL;
		 rch =
		 rch->next_in_room) act("The $d opens.", rch, NULL,
					pexit_rev->keyword, TO_CHAR);
	}
    }

    return;
}



void do_close(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument(argument, arg);

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if (arg[0] == '\0') {
	send_to_char("Close what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_here(ch, arg)) != NULL) {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL) {

	    if (!IS_SET(obj->value[1], EX_ISDOOR)
		|| IS_SET(obj->value[1], EX_NOCLOSE)) {
		send_to_char("You can't do that.\n\r", ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_CLOSED)) {
		send_to_char("It's already closed.\n\r", ch);
		return;
	    }

	    SET_BIT(obj->value[1], EX_CLOSED);
	    act("You close $p.", ch, obj, NULL, TO_CHAR);
	    act("$n closes $p.", ch, obj, NULL, TO_ROOM);
	    return;
	}

	/* 'close object' */
	if (obj->item_type != ITEM_CONTAINER
	    && obj->item_type != ITEM_SADDLE) {
	    send_to_char("That's not a container.\n\r", ch);
	    return;
	}
	if (IS_SET(obj->value[1], CONT_CLOSED)) {
	    send_to_char("It's already closed.\n\r", ch);
	    return;
	}
	if (!IS_SET(obj->value[1], CONT_CLOSEABLE)) {
	    send_to_char("You can't do that.\n\r", ch);
	    return;
	}

	SET_BIT(obj->value[1], CONT_CLOSED);
	act("You close $p.", ch, obj, NULL, TO_CHAR);
	act("$n closes $p.", ch, obj, NULL, TO_ROOM);
	return;
    }

    if ((door = find_door(ch, arg)) >= 0) {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED)) {
	    send_to_char("It's already closed.\n\r", ch);
	    return;
	}

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act("$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM);
	send_to_char("Ok.\n\r", ch);

	/* close the other side */
	if ((to_room = pexit->u1.to_room) != NULL
	    && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
	    && pexit_rev->u1.to_room == ch->in_room) {
	    CHAR_DATA *rch;

	    SET_BIT(pexit_rev->exit_info, EX_CLOSED);
	    for (rch = to_room->people; rch != NULL;
		 rch =
		 rch->next_in_room) act("The $d closes.", rch, NULL,
					pexit_rev->keyword, TO_CHAR);
	}
    }

    return;
}



bool has_key(CHAR_DATA * ch, int key)
{
    OBJ_DATA *obj;

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
	if (obj->pIndexData->vnum == key)
	    return TRUE;
    }

    return FALSE;
}



void do_lock(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument(argument, arg);

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if (arg[0] == '\0') {
	send_to_char("Lock what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_here(ch, arg)) != NULL) {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL) {
	    if (!IS_SET(obj->value[1], EX_ISDOOR)
		|| IS_SET(obj->value[1], EX_NOCLOSE)) {
		send_to_char("You can't do that.\n\r", ch);
		return;
	    }
	    if (!IS_SET(obj->value[1], EX_CLOSED)) {
		send_to_char("It's not closed.\n\r", ch);
		return;
	    }

	    if (obj->value[4] < 0 || IS_SET(obj->value[1], EX_NOLOCK)) {
		send_to_char("It can't be locked.\n\r", ch);
		return;
	    }

	    if (!has_key(ch, obj->value[4])) {
		send_to_char("You lack the key.\n\r", ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED)) {
		send_to_char("It's already locked.\n\r", ch);
		return;
	    }

	    SET_BIT(obj->value[1], EX_LOCKED);
	    act("You lock $p.", ch, obj, NULL, TO_CHAR);
	    act("$n locks $p.", ch, obj, NULL, TO_ROOM);
	    return;
	}

	/* 'lock object' */
	if (obj->item_type != ITEM_CONTAINER
	    || obj->item_type != ITEM_SADDLE) {
	    send_to_char("That's not a container.\n\r", ch);
	    return;
	}
	if (!IS_SET(obj->value[1], CONT_CLOSED)) {
	    send_to_char("It's not closed.\n\r", ch);
	    return;
	}
	if (obj->value[2] < 0) {
	    send_to_char("It can't be locked.\n\r", ch);
	    return;
	}
	if (!has_key(ch, obj->value[2])) {
	    send_to_char("You lack the key.\n\r", ch);
	    return;
	}
	if (IS_SET(obj->value[1], CONT_LOCKED)) {
	    send_to_char("It's already locked.\n\r", ch);
	    return;
	}

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("You lock $p.", ch, obj, NULL, TO_CHAR);
	act("$n locks $p.", ch, obj, NULL, TO_ROOM);
	return;
    }

    if ((door = find_door(ch, arg)) >= 0) {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
	    send_to_char("It's not closed.\n\r", ch);
	    return;
	}
	if (pexit->key < 0) {
	    send_to_char("It can't be locked.\n\r", ch);
	    return;
	}
	if (!has_key(ch, pexit->key)) {
	    send_to_char("You lack the key.\n\r", ch);
	    return;
	}
	if (IS_SET(pexit->exit_info, EX_LOCKED)) {
	    send_to_char("It's already locked.\n\r", ch);
	    return;
	}

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char("*Click*\n\r", ch);
	act("$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

	/* lock the other side */
	if ((to_room = pexit->u1.to_room) != NULL
	    && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
	    && pexit_rev->u1.to_room == ch->in_room) {
	    SET_BIT(pexit_rev->exit_info, EX_LOCKED);
	}
    }

    return;
}



void do_unlock(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument(argument, arg);

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if (arg[0] == '\0') {
	send_to_char("Unlock what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_here(ch, arg)) != NULL) {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL) {
	    if (IS_SET(obj->value[1], EX_ISDOOR)) {
		send_to_char("You can't do that.\n\r", ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED)) {
		send_to_char("It's not closed.\n\r", ch);
		return;
	    }

	    if (obj->value[4] < 0) {
		send_to_char("It can't be unlocked.\n\r", ch);
		return;
	    }

	    if (!has_key(ch, obj->value[4])) {
		send_to_char("You lack the key.\n\r", ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_LOCKED)) {
		send_to_char("It's already unlocked.\n\r", ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_LOCKED);
	    act("You unlock $p.", ch, obj, NULL, TO_CHAR);
	    act("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
	    return;
	}

	/* 'unlock object' */
	if (obj->item_type != ITEM_CONTAINER
	    && obj->item_type != ITEM_SADDLE) {
	    send_to_char("That's not a container.\n\r", ch);
	    return;
	}
	if (!IS_SET(obj->value[1], CONT_CLOSED)) {
	    send_to_char("It's not closed.\n\r", ch);
	    return;
	}
	if (obj->value[2] < 0) {
	    send_to_char("It can't be unlocked.\n\r", ch);
	    return;
	}
	if (!has_key(ch, obj->value[2])) {
	    send_to_char("You lack the key.\n\r", ch);
	    return;
	}
	if (!IS_SET(obj->value[1], CONT_LOCKED)) {
	    send_to_char("It's already unlocked.\n\r", ch);
	    return;
	}

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You unlock $p.", ch, obj, NULL, TO_CHAR);
	act("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
	return;
    }

    if ((door = find_door(ch, arg)) >= 0) {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
	    send_to_char("It's not closed.\n\r", ch);
	    return;
	}
	if (pexit->key < 0) {
	    send_to_char("It can't be unlocked.\n\r", ch);
	    return;
	}
	if (!has_key(ch, pexit->key)) {
	    send_to_char("You lack the key.\n\r", ch);
	    return;
	}
	if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
	    send_to_char("It's already unlocked.\n\r", ch);
	    return;
	}

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char("*Click*\n\r", ch);
	act("$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

	/* unlock the other side */
	if ((to_room = pexit->u1.to_room) != NULL
	    && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	    && pexit_rev->u1.to_room == ch->in_room) {
	    REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
	}
    }

    return;
}



void do_pick(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Pick what?\n\r", ch);
	return;
    }

    WAIT_STATE(ch, skill_table[gsn_pick_lock].beats);

    /* look for guards */
    for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
	if (IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level) {
	    act("$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR);
	    return;
	}
    }

    if (!IS_NPC(ch) && number_percent() > get_skill(ch, gsn_pick_lock)) {
	send_to_char("You failed.\n\r", ch);
	check_improve(ch, gsn_pick_lock, FALSE, 2);
	return;
    }

    if ((obj = get_obj_here(ch, arg)) != NULL) {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL) {
	    if (!IS_SET(obj->value[1], EX_ISDOOR)) {
		send_to_char("You can't do that.\n\r", ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED)) {
		send_to_char("It's not closed.\n\r", ch);
		return;
	    }

	    if (obj->value[4] < 0) {
		send_to_char("It can't be unlocked.\n\r", ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_PICKPROOF)) {
		send_to_char("You failed.\n\r", ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_LOCKED);
	    act("You pick the lock on $p.", ch, obj, NULL, TO_CHAR);
	    act("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
	    check_improve(ch, gsn_pick_lock, TRUE, 2);
	    return;
	}





	/* 'pick object' */
	if (obj->item_type != ITEM_CONTAINER
	    && obj->item_type != ITEM_SADDLE) {
	    send_to_char("That's not a container.\n\r", ch);
	    return;
	}
	if (!IS_SET(obj->value[1], CONT_CLOSED)) {
	    send_to_char("It's not closed.\n\r", ch);
	    return;
	}
	if (obj->value[2] < 0) {
	    send_to_char("It can't be unlocked.\n\r", ch);
	    return;
	}
	if (!IS_SET(obj->value[1], CONT_LOCKED)) {
	    send_to_char("It's already unlocked.\n\r", ch);
	    return;
	}
	if (IS_SET(obj->value[1], CONT_PICKPROOF)) {
	    send_to_char("You failed.\n\r", ch);
	    return;
	}

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You pick the lock on $p.", ch, obj, NULL, TO_CHAR);
	act("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
	check_improve(ch, gsn_pick_lock, TRUE, 2);
	return;
    }

    if ((door = find_door(ch, arg)) >= 0) {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch)) {
	    send_to_char("It's not closed.\n\r", ch);
	    return;
	}
	if (pexit->key < 0 && !IS_IMMORTAL(ch)) {
	    send_to_char("It can't be picked.\n\r", ch);
	    return;
	}
	if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
	    send_to_char("It's already unlocked.\n\r", ch);
	    return;
	}
	if (IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch)) {
	    send_to_char("You failed.\n\r", ch);
	    return;
	}

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char("*Click*\n\r", ch);
	act("$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
	check_improve(ch, gsn_pick_lock, TRUE, 2);

	/* pick the other side */
	if ((to_room = pexit->u1.to_room) != NULL
	    && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	    && pexit_rev->u1.to_room == ch->in_room) {
	    REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
	}
    }

    return;
}

void do_push(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int door;
    int push_orig;
    char buf[100];
    /* BUGFIX - Suzuran */
    int move;

    argument = one_argument(argument, arg);
    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if (arg[0] == '\0') {
	send_to_char("Who do you want to push?\n\r", ch);
	return;
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
	send_to_char("You trip over your feet and fall down.\n\r", ch);
	act("$n trips over $s feet and falls down.", ch, NULL, NULL,
	    TO_ROOM);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("You can't go around pushing yourself!\n\r", ch);
	return;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim)
	&& (ch->pcdata->pkset < 1 || victim->pcdata->pkset < 1)) {
	send_to_char("PKers can't push Non-Pkers and vice/versa.", ch);
	return;
    }

    if ((IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	 && ((!IS_NPC(ch) && ch->fight <= 0)
	     || (!IS_NPC(victim) && victim->fight <= 0)))) {
	send_to_char("Not in this room.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_PASS_DOOR)) {
	act("`4You tried to push $M, but passed right through!``", ch,
	    NULL, victim, TO_CHAR);
	act("`^$n tried to push you, but passed right through!``", ch,
	    NULL, victim, TO_VICT);
	act("`4$n tried to push $M, but passed right through!``", ch, NULL,
	    victim, TO_NOTVICT);
	return;
    }

    if ((victim->level > ch->level + 6) && (IS_NPC(victim))) {
	sprintf(buf, "Who the hell do you think your pushing around??");
	do_say(victim, buf);
	if (!IS_NPC(ch)) {
	    sprintf(buf, "Get ready to feel the wrath of my power!!");
	    do_say(victim, buf);
	    do_kill(victim, ch->name);
	} else {
	    sprintf(buf, "You're an odd one... just go away already.");
	    do_say(victim, buf);
	}
	return;
    }

    if (ch->position == POS_RESTING) {
	send_to_char("Maybe you should stand first?\n\r", ch);
	return;
    }

    if (victim->position == POS_RESTING) {
	act("$N is resting. Try dragging $M.\n\r", ch, NULL, victim,
	    TO_CHAR);
	return;
    }

    if (!IS_AWAKE(ch)) {
	send_to_char("Try waking up first!\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	if (victim->pIndexData->pShop != NULL) {
	    send_to_char("The shopkeeper wouldn't like that.\n\r", ch);
	    return;
	}
    }

    if (!IS_AWAKE(victim)) {
	act("$N is asleep. Try dragging $M.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->position == POS_FIGHTING) {
	send_to_char("Maybe you should stop fighting first?\n\r", ch);
	return;
    }

    if (victim->mount == IS_MOUNTED) {
	act("$n is on a horse! You can't push them.\n\r", ch, NULL, victim,
	    TO_CHAR); return;
    }

    if (victim->position == POS_FIGHTING) {
	act("$n is fighting. Wait your turn!\n\r", ch, NULL, victim,
	    TO_CHAR);
	return;
    }

    argument = one_argument(argument, arg);
    if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
	door = 0;
    else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
	door = 1;
    else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
	door = 2;
    else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
	door = 3;
    else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
	door = 4;
    else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
	door = 5;
    else
	return;

    /* NEED DOOR VALIDITY CHECK! -- Suzuran */
    if (ch->in_room->exit[door] == NULL) {
	send_to_char("You can't go that way!\n\r", ch);
	return;
    }

    /* BUGFIX HERE - Suzuran */
    /* This was not using mv, so if you were out of mv, you could
       summon a low level mob and push/drag it wherever to move. */

    /* Figure up movement points.  I can't do this right because
       I don't know the vnum/etc. of the room we're moving TO,
       but this should be close enough. */

    move = movement_loss[UMIN(SECT_MAX - 1, ch->in_room->sector_type)];

    if (ch->move < move) {
	send_to_char("You're too tired for that!\n\r", ch);
	return;
    }
    ch->move = ch->move - move;

    if (get_curr_stat(ch, STAT_STR) >= (get_curr_stat(victim, STAT_STR))) {
	act("$n slams into $N and sends $m flying.", ch, NULL, victim,
	    TO_ROOM);
	push_orig = victim->in_room->vnum;
	victim->hit = victim->hit - dice(1, 4);
	victim->position = POS_RESTING;
	WAIT_STATE(victim, number_range(1, 2) * PULSE_VIOLENCE);

	if (!IS_AFFECTED(ch, AFF_SNEAK) && (!IS_NPC(ch)))
	    victim->hit = victim->hit - dice(3, 8);

	victim->position = POS_RESTING;
	WAIT_STATE(victim, number_range(1, 2) * PULSE_VIOLENCE);

	push_char(ch, victim, door, FALSE);
	act("$n slams into you and you are knocked flying!\n\r", ch, NULL,
	    victim, TO_VICT);
	sprintf(buf, "You slam into %s with all your weight! OW!\n\r",
		IS_NPC(victim) ? victim->short_descr : victim->name);
	send_to_char(buf, ch);
	sprintf(buf, "PUSH: %s (%d) pushed %s (%d) from [%d] to [%d] !!",
		ch->name, ch->level, victim->name, victim->level,
		push_orig, victim->in_room->vnum);
	log_string(buf);

    } else {
	act("$n tries unsuccessfully to push $N.\n\r", ch, NULL, victim,
	    TO_ROOM);
	act("You shove $N with all your might....and fail.\n\r", ch, NULL,
	    victim, TO_CHAR);
	act
	    ("The wind is knocked out of you from the force of your slam.\n\r",
	     ch, NULL, NULL, TO_CHAR);
    }
    return;
}

void do_drag(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int door;
    int drag_orig;
    char buf[100];
    int move;

    argument = one_argument(argument, arg);
    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if (arg[0] == '\0') {
	send_to_char("Who do you want to drag?\n\r", ch);
	return;
    }
/*
    if (IS_NPC(ch))
    {
	send_to_char( "Switched IMM's may not push or drag.\n\r", ch); 
	return;
    }
*/
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
	send_to_char("You stumble around running into things.\n\r", ch);
	act("$n stumbles around running into things.", ch, NULL, NULL,
	    TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch, AFF_FROZEN)) {
	send_to_char("You are a frozen statue.\n\r", ch);
	ch->position = POS_STUNNED;
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("You can't drag yourself!\n\r", ch);
	return;
    }

    if (!IS_NPC(ch) && !IS_NPC(victim)
	&& (ch->pcdata->pkset < 1 || victim->pcdata->pkset < 1)) {
	send_to_char("PKers can't drag Non-Pkers. Vice/versa.", ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	&& ((!IS_NPC(ch) && ch->fight <= 0)
	    || (!IS_NPC(victim) && victim->fight <= 0))) {
	send_to_char("Not in this room.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	if (victim->pIndexData->pShop != NULL) {
	    send_to_char("The shopkeeper wouldn't like that.\n\r", ch);
	    return;
	}
    }

    if (ch->position == POS_RESTING) {
	send_to_char("Maybe you should stand first?\n\r", ch);
	return;
    }


    if (victim->position == POS_STANDING) {
	act("$N is standing. Try pushing $M.\n\r", ch, NULL, victim,
	    TO_CHAR);
	return;
    }

    if (!IS_AWAKE(ch)) {
	send_to_char("Try waking up first!\n\r", ch);
	return;
    }

    if (ch->position == POS_FIGHTING) {
	send_to_char("Maybe you should stop fighting first?\n\r", ch);
	return;
    }

    if (victim->position == POS_FIGHTING) {
	act("$n is fighting. Wait your turn!\n\r", ch, NULL, victim,
	    TO_CHAR);
	return;
    }

    argument = one_argument(argument, arg);
    if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
	door = 0;
    else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
	door = 1;
    else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
	door = 2;
    else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
	door = 3;
    else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
	door = 4;
    else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
	door = 5;
    else
	return;

    /* NEED DOOR VALIDITY CHECK! -- Suzuran */
    if (ch->in_room->exit[door] == NULL) {
	send_to_char("You can't go that way!\n\r", ch);
	return;
    }

    /* BUGFIX HERE - Suzuran */
    /* This was not using mv, so if you were out of mv, you could
       summon a low level mob and push/drag it wherever to move. */

    /* Figure up movement points.  I can't do this right because
       I don't know the vnum/etc. of the room we're moving TO,
       but this should be close enough. */

    move = movement_loss[UMIN(SECT_MAX - 1, ch->in_room->sector_type)];

    if (ch->move < move) {
	send_to_char("You're too tired for that!\n\r", ch);
	return;
    }
    ch->move = ch->move - move;

    if (get_curr_stat(ch, STAT_STR) >= get_curr_stat(victim, STAT_STR)) {
	drag_orig = ch->in_room->vnum;
	act("$n drags you out of the room!\n\r", ch, NULL, victim,
	    TO_VICT);
	act("$n grabs ahold of $N dragging them out of the room.", ch,
	    NULL, victim, TO_NOTVICT);
	sprintf(buf, "You grab ahold of %s and drag them %s!\n\r",
		IS_NPC(victim) ? victim->short_descr : victim->name,
		dir_name[door]);
	send_to_char(buf, ch);

	drag_char(ch, victim, door, FALSE);
	sprintf(buf, "DRAG: %s (%d) dragged %s (%d) from [%d] to [%d] !!",
		ch->name, ch->level, victim->name, victim->level,
		drag_orig, victim->in_room->vnum);
	log_string(buf);
    } else {
	act("$n tries unsuccessfully to drag $M.\n\r", ch, NULL, victim,
	    TO_ROOM);
    }
    return;
}

void push_char(CHAR_DATA * ch, CHAR_DATA * victim, int door, bool follow)
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;

    REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if (IS_AFFECTED(ch, AFF_FROZEN)) {
	send_to_char("You are a frozen statue.\n\r", ch);
	ch->position = POS_STUNNED;
	return;
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
	send_to_char("You stumble around running into things.\n\r", ch);
	act("$n stumbles around running into things.", ch, NULL, NULL,
	    TO_ROOM);
	return;
    }

    if (door < 0 || door > 5) {
	bug("Do_move: bad door %d.", door);
	return;
    }

    /*
     * Exit trigger, if activated, bail out. Only PCs are triggered.
     */
    if ( !IS_NPC(ch) && mp_exit_trigger( ch, door ) )
	return;

    in_room = ch->in_room;

    if ((pexit = in_room->exit[door]) == NULL
	|| (to_room = pexit->u1.to_room) == NULL
	|| pexit->u1.vnum == 0 || !can_see_room(ch, pexit->u1.to_room)) {
	send_to_char("You are slammed up against a wall.\n\r", ch);
	return;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
	&& !IS_AFFECTED(ch, AFF_PASS_DOOR)) {
	act("The door is closed, you hit your head.", ch, NULL, victim,
	    TO_CHAR);
	return;
    }


    if (room_is_private(to_room)) {
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_CLAN_ONLY)) {
	send_to_char("You can't push into clan rooms.\n\r", ch);
	return;
    }

    if (IS_SET(to_room->room_flags, ROOM_SAFE)) {
	send_to_char("You cannot push into safe rooms.\n\r", ch);
	return;
    }


    if (in_room->sector_type == SECT_AIR
	|| to_room->sector_type == SECT_AIR) {
	if (!IS_AFFECTED(victim, AFF_FLYING) && !IS_IMMORTAL(victim)) {
	    send_to_char("You are pushed into the air and fall down.\n\r",
			 victim);
	    return;
	}
    }


    WAIT_STATE(ch, 1);

    char_from_room(ch);
    char_from_room(victim);
    char_to_room(ch, to_room);
    char_to_room(victim, to_room);
    if (!IS_AFFECTED(ch, AFF_SNEAK)
	&& (!IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS)))
	act("$n pushes $N into the room, $N falls to the ground.", ch,
	    NULL, victim, TO_NOTVICT);

    do_look(ch, "auto");

    if (in_room == to_room)	/* no circular follows */
	return;

    for (fch = in_room->people; fch != NULL; fch = fch_next) {
	fch_next = fch->next_in_room;

	if (fch->master == victim && IS_AFFECTED(fch, AFF_CHARM)
	    && fch->position < POS_STANDING)
	    do_stand(fch, "");

	if (fch->master == victim && fch->position == POS_STANDING) {

	    if (IS_SET(victim->in_room->room_flags, ROOM_LAW)
		&& (IS_NPC(fch) && IS_SET(fch->act, ACT_AGGRESSIVE))) {
		act("You can't bring $N into the city.", ch, NULL, fch,
		    TO_CHAR);
		act("You aren't allowed in the city.", fch, NULL, NULL,
		    TO_CHAR);
		return;
	    }

	    act("You follow $N.", fch, NULL, ch, TO_CHAR);
	    move_char(fch, door, TRUE);
	}
    }

    return;
}

void drag_char(CHAR_DATA * ch, CHAR_DATA * victim, int door, bool follow)
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;

    REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if (IS_AFFECTED(ch, AFF_FROZEN)) {
	send_to_char("You are a frozen statue.\n\r", ch);
	ch->position = POS_STUNNED;
	return;
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
	send_to_char("You stumble around running into things.\n\r", ch);
	act("$n stumbles around running into things.", ch, NULL, NULL,
	    TO_ROOM);
	return;
    }


    if (door < 0 || door > 5) {
	bug("Do_move: bad door %d.", door);
	return;
    }

    in_room = ch->in_room;
    if ((pexit = in_room->exit[door]) == NULL
	|| (to_room = pexit->u1.to_room) == NULL
	|| !can_see_room(ch, pexit->u1.to_room)
	|| (pexit->u1.vnum == 0 || pexit->u1.to_room == NULL)) {
	if (IS_AWAKE(victim)) {
	    send_to_char("You get dragged around the room!\n\r", victim);
	    return;
	} else {
	    send_to_char
		("For some reason you dream that your head is being slammed into a wall!\n\r",
		 victim);
	    send_to_char("There is not an exit in that direction.\n\r",
			 ch);
	    return;
	}
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
	&& !IS_AFFECTED(ch, AFF_PASS_DOOR)) {
	act("The $d is closed, you are dragged against it.", victim, NULL,
	    pexit->keyword, TO_CHAR);
	return;
    }


    if (room_is_private(to_room)) {
	send_to_char("That room is private right now.\n\r", ch);
	return;
    }

    if (IS_SET(to_room->room_flags, ROOM_CLAN_ONLY)) {
	send_to_char("You can't push into clan rooms.\n\r", ch);
	return;
    }

    if (IS_SET(to_room->room_flags, ROOM_SAFE)) {
	send_to_char("You cannot drag into safe rooms.\n\r", ch);
	return;
    }

    if (in_room->sector_type == SECT_AIR
	|| to_room->sector_type == SECT_AIR) {
	if (!IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch)) {
	    send_to_char("You are dragged into the air and fall down.\n\r",
			 victim);
	    return;
	}
    }


    WAIT_STATE(ch, 1);

    char_from_room(ch);
    char_from_room(victim);
    char_to_room(ch, to_room);
    char_to_room(victim, to_room);
    if (!IS_AFFECTED(ch, AFF_SNEAK) && (!IS_NPC(ch)))
	act("$n drags $N into the room.\n\r", ch, NULL, victim,
	    TO_NOTVICT);

    do_look(victim, "auto");
    do_look(ch, "auto");

    if (in_room == to_room)	/* no circular follows */
	return;

    for (fch = in_room->people; fch != NULL; fch = fch_next) {
	fch_next = fch->next_in_room;

	if (fch->master == ch && IS_AFFECTED(fch, AFF_CHARM)
	    && fch->position < POS_STANDING)
	    do_stand(fch, "");

	if (fch->master == ch && fch->position == POS_STANDING) {
	    if (IS_SET(ch->in_room->room_flags, ROOM_LAW)
		&& (IS_NPC(fch) && IS_SET(fch->act, ACT_AGGRESSIVE))) {
		act("You can't bring $N into the city.", ch, NULL, fch,
		    TO_CHAR);
		act("You aren't allowed in the city.", fch, NULL, NULL,
		    TO_CHAR);
		return;
	    }

	    act("You follow $N.", fch, NULL, ch, TO_CHAR);
	    move_char(fch, door, TRUE);
	}
    }

    /* 
     * If someone is following the char, these triggers get activated
     * for the followers before the char, but it's safer this way...
     */
    if ( IS_NPC( ch ) && HAS_TRIGGER( ch, TRIG_ENTRY ) )
	mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_ENTRY );
    if ( !IS_NPC( ch ) )
    	mp_greet_trigger( ch );

    return;
}

void do_stand(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if (argument[0] != '\0') {
	if (ch->position == POS_FIGHTING) {
	    send_to_char("Maybe you should finish fighting first?\n\r",
			 ch);
	    return;
	}
	obj = get_obj_list(ch, argument, ch->in_room->contents);
	if (obj == NULL) {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}
	if (obj->item_type != ITEM_FURNITURE
	    || (!IS_SET(obj->value[2], STAND_AT)
		&& !IS_SET(obj->value[2], STAND_ON)
		&& !IS_SET(obj->value[2], STAND_IN))) {
	    send_to_char("You can't seem to find a place to stand.\n\r",
			 ch);
	    return;
	}
	if (ch->on != obj && count_users(obj) >= obj->value[0]) {
	    act_new("There's no room to stand on $p.",
		    ch, obj, NULL, TO_ROOM, POS_DEAD);
	    return;
	}
    }

    switch (ch->position) {
    case POS_SLEEPING:
	if (IS_AFFECTED(ch, AFF_SLEEP)) {
	    send_to_char("You can't wake up!\n\r", ch);
	    return;
	}

	if (obj == NULL) {
	    send_to_char("You wake and stand up.\n\r", ch);
	    act("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
	    ch->on = NULL;
	} else if (IS_SET(obj->value[2], STAND_AT)) {
	    act_new("You wake and stand at $p.", ch, obj, NULL, TO_CHAR,
		    POS_DEAD);
	    act("$n wakes and stands at $p.", ch, obj, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], STAND_ON)) {
	    act_new("You wake and stand on $p.", ch, obj, NULL, TO_CHAR,
		    POS_DEAD);
	    act("$n wakes and stands on $p.", ch, obj, NULL, TO_ROOM);
	} else {
	    act_new("You wake and stand in $p.", ch, obj, NULL, TO_CHAR,
		    POS_DEAD);
	    act("$n wakes and stands in $p.", ch, obj, NULL, TO_ROOM);
	}
	ch->position = POS_STANDING;
	do_look(ch, "auto");
	break;

    case POS_RESTING:
    case POS_SITTING:
	if (obj == NULL) {
	    send_to_char("You stand up.\n\r", ch);
	    act("$n stands up.", ch, NULL, NULL, TO_ROOM);
	    ch->on = NULL;
	} else if (IS_SET(obj->value[2], STAND_AT)) {
	    act("You stand at $p.", ch, obj, NULL, TO_CHAR);
	    act("$n stands at $p.", ch, obj, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], STAND_ON)) {
	    act("You stand on $p.", ch, obj, NULL, TO_CHAR);
	    act("$n stands on $p.", ch, obj, NULL, TO_ROOM);
	} else {
	    act("You stand in $p.", ch, obj, NULL, TO_CHAR);
	    act("$n stands on $p.", ch, obj, NULL, TO_ROOM);
	}
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char("You are already standing.\n\r", ch);
	break;

    case POS_FIGHTING:
	send_to_char("You are already fighting!\n\r", ch);
	break;
    }

    return;
}



void do_rest(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;
    int race = race_lookup("Dragonkin");

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if (ch->position == POS_FIGHTING) {
	send_to_char("You can't rest while fighting!\n\r", ch);
	return;
    }

    /* Added this so that you can't be flying and dragonkin have to land */
    /* --Vorlin */

    if (IS_AFFECTED(ch, AFF_FLYING) && ch->race == race) {
	send_to_char("You have to land before you can rest.\n\r", ch);
	return;
    } else if (IS_AFFECTED(ch, AFF_FLYING)
	       && ch->race != race && ch->level > 10) {
	send_to_char("You can't rest while flying...\n\r", ch);
	return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0') {
	obj = get_obj_list(ch, argument, ch->in_room->contents);
	if (obj == NULL) {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}
    } else
	obj = ch->on;

    if (obj != NULL) {
	if (!IS_SET(obj->item_type, ITEM_FURNITURE)
	    || (!IS_SET(obj->value[2], REST_ON)
		&& !IS_SET(obj->value[2], REST_IN)
		&& !IS_SET(obj->value[2], REST_AT))) {
	    send_to_char("You can't rest on that.\n\r", ch);
	    return;
	}

	if (obj != NULL && ch->on != obj
	    && count_users(obj) >= obj->value[0]) {
	    act_new("There's no more room on $p.", ch, obj, NULL, TO_CHAR,
		    POS_DEAD);
	    return;
	}

	ch->on = obj;
    }

    switch (ch->position) {
    case POS_SLEEPING:
	if (obj == NULL) {
	    if (IS_AFFECTED(ch, AFF_SLEEP)) {
		send_to_char("You try to open your eyes, but can't.\n\r",
			     ch);
		return;
	    } else {
		send_to_char("You wake up and start resting.\n\r", ch);
		act("$n wakes up and starts resting.", ch, NULL, NULL,
		    TO_ROOM);
	    }
	} else if (IS_SET(obj->value[2], REST_AT)) {
	    act_new("You wake up and rest at $p.",
		    ch, obj, NULL, TO_CHAR, POS_SLEEPING);
	    act("$n wakes up and rests at $p.", ch, obj, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], REST_ON)) {
	    act_new("You wake up and rest on $p.",
		    ch, obj, NULL, TO_CHAR, POS_SLEEPING);
	    act("$n wakes up and rests on $p.", ch, obj, NULL, TO_ROOM);
	} else {
	    act_new("You wake up and rest in $p.",
		    ch, obj, NULL, TO_CHAR, POS_SLEEPING);
	    act("$n wakes up and rests in $p.", ch, obj, NULL, TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char("You are already resting.\n\r", ch);
	break;

    case POS_STANDING:
	if (obj == NULL) {
	    send_to_char("You rest.\n\r", ch);
	    act("$n sits down and rests.", ch, NULL, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], REST_AT)) {
	    act("You sit down at $p and rest.", ch, obj, NULL, TO_CHAR);
	    act("$n sits down at $p and rests.", ch, obj, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], REST_ON)) {
	    act("You sit on $p and rest.", ch, obj, NULL, TO_CHAR);
	    act("$n sits on $p and rests.", ch, obj, NULL, TO_ROOM);
	} else {
	    act("You rest in $p.", ch, obj, NULL, TO_CHAR);
	    act("$n rests in $p.", ch, obj, NULL, TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	if (obj == NULL) {
	    send_to_char("You rest.\n\r", ch);
	    act("$n rests.", ch, NULL, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], REST_AT)) {
	    act("You rest at $p.", ch, obj, NULL, TO_CHAR);
	    act("$n rests at $p.", ch, obj, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], REST_ON)) {
	    act("You rest on $p.", ch, obj, NULL, TO_CHAR);
	    act("$n rests on $p.", ch, obj, NULL, TO_ROOM);
	} else {
	    act("You rest in $p.", ch, obj, NULL, TO_CHAR);
	    act("$n rests in $p.", ch, obj, NULL, TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;
    }


    return;
}


void do_sit(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;
    int race = race_lookup("Dragonkin");

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    if (ch->position == POS_FIGHTING) {
	send_to_char("Maybe you should finish this fight first?\n\r", ch);
	return;
    }

    /* Added this so that you can't be flying and dragonkin have to land */
    /* --Vorlin */

    if (IS_AFFECTED(ch, AFF_FLYING) && ch->race == race) {
	send_to_char("You have to land before you can sit.\n\r", ch);
	return;
    } else if (IS_AFFECTED(ch, AFF_FLYING)
	       && ch->race != race && ch->level > 10) {
	send_to_char("You can't sit while flying...\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP)) {
 	send_to_char("Magical sleep prevents you from sitting up.\n\r", ch);
	return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0') {
	obj = get_obj_list(ch, argument, ch->in_room->contents);
	if (obj == NULL) {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}
    } else
	obj = ch->on;

    if (obj != NULL) {
	if (!IS_SET(obj->item_type, ITEM_FURNITURE)
	    || (!IS_SET(obj->value[2], SIT_ON)
		&& !IS_SET(obj->value[2], SIT_IN)
		&& !IS_SET(obj->value[2], SIT_AT))) {
	    send_to_char("You can't sit on that.\n\r", ch);
	    return;
	}

	if (obj != NULL && ch->on != obj
	    && count_users(obj) >= obj->value[0]) {
	    act_new("There's no more room on $p.", ch, obj, NULL, TO_CHAR,
		    POS_DEAD);
	    return;
	}

	ch->on = obj;
    }
    switch (ch->position) {
    case POS_SLEEPING:
	if (obj == NULL) {
	    send_to_char("You wake and sit up.\n\r", ch);
	    act("$n wakes and sits up.", ch, NULL, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], SIT_AT)) {
	    act_new("You wake and sit at $p.", ch, obj, NULL, TO_CHAR,
		    POS_DEAD);
	    act("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], SIT_ON)) {
	    act_new("You wake and sit on $p.", ch, obj, NULL, TO_CHAR,
		    POS_DEAD);
	    act("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
	} else {
	    act_new("You wake and sit in $p.", ch, obj, NULL, TO_CHAR,
		    POS_DEAD);
	    act("$n wakes and sits in $p.", ch, obj, NULL, TO_ROOM);
	}

	ch->position = POS_SITTING;
	break;
    case POS_RESTING:
	if (obj == NULL)
	    send_to_char("You stop resting.\n\r", ch);
	else if (IS_SET(obj->value[2], SIT_AT)) {
	    act("You sit at $p.", ch, obj, NULL, TO_CHAR);
	    act("$n sits at $p.", ch, obj, NULL, TO_ROOM);
	}

	else if (IS_SET(obj->value[2], SIT_ON)) {
	    act("You sit on $p.", ch, obj, NULL, TO_CHAR);
	    act("$n sits on $p.", ch, obj, NULL, TO_ROOM);
	}
	ch->position = POS_SITTING;
	break;
    case POS_SITTING:
	send_to_char("You are already sitting down.\n\r", ch);
	break;
    case POS_STANDING:
	if (obj == NULL) {
	    send_to_char("You sit down.\n\r", ch);
	    act("$n sits down on the ground.", ch, NULL, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], SIT_AT)) {
	    act("You sit down at $p.", ch, obj, NULL, TO_CHAR);
	    act("$n sits down at $p.", ch, obj, NULL, TO_ROOM);
	} else if (IS_SET(obj->value[2], SIT_ON)) {
	    act("You sit on $p.", ch, obj, NULL, TO_CHAR);
	    act("$n sits on $p.", ch, obj, NULL, TO_ROOM);
	} else {
	    act("You sit down in $p.", ch, obj, NULL, TO_CHAR);
	    act("$n sits down in $p.", ch, obj, NULL, TO_ROOM);
	}
	ch->position = POS_SITTING;
	break;
    }
    return;
}


void do_sleep(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;
    int race = race_lookup("Dragonkin");

    if (IS_AFFECTED(ch, AFF_FLYING) && ch->race == race) {
	send_to_char("You have to land before you can sleep.\n\r", ch);
	return;
    } else if (IS_AFFECTED(ch, AFF_FLYING)
	       && ch->race != race && ch->level > 10) {
	send_to_char("You can't sleep while flying.\n\r", ch);
	return;
    }

    switch (ch->position) {
    case POS_SLEEPING:
	send_to_char("You are already sleeping.\n\r", ch);
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
	if (argument[0] == '\0' && ch->on == NULL) {
	    send_to_char("You go to sleep.\n\r", ch);
	    act("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
	    ch->position = POS_SLEEPING;
	} else {		/* find an object and sleep on it */

	    if (argument[0] == '\0')
		obj = ch->on;
	    else
		obj = get_obj_list(ch, argument, ch->in_room->contents);

	    if (obj == NULL) {
		send_to_char("You don't see that here.\n\r", ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
		|| (!IS_SET(obj->value[2], SLEEP_ON)
		    && !IS_SET(obj->value[2], SLEEP_IN)
		    && !IS_SET(obj->value[2], SLEEP_AT))) {
		send_to_char("You can't sleep on that!\n\r", ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0]) {
		act_new("There is no room on $p for you.",
			ch, obj, NULL, TO_CHAR, POS_DEAD);
		return;
	    }

	    ch->on = obj;
	    if (IS_SET(obj->value[2], SLEEP_AT)) {
		act("You go to sleep at $p.", ch, obj, NULL, TO_CHAR);
		act("$n goes to sleep at $p.", ch, obj, NULL, TO_ROOM);
	    } else if (IS_SET(obj->value[2], SLEEP_ON)) {
		act("You go to sleep on $p.", ch, obj, NULL, TO_CHAR);
		act("$n goes to sleep on $p.", ch, obj, NULL, TO_ROOM);
	    } else {
		act("You go to sleep in $p.", ch, obj, NULL, TO_CHAR);
		act("$n goes to sleep in $p.", ch, obj, NULL, TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;
	}
	break;

    case POS_FIGHTING:
	send_to_char("You are already fighting!\n\r", ch);
	break;
    }

    return;
}



void do_wake(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
	do_stand(ch, argument);
	return;
    }

    if (!IS_AWAKE(ch)) {
	send_to_char("You are asleep yourself!\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_AWAKE(victim)) {
	act("$N is already awake.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_AFFECTED(victim, AFF_SLEEP)) {
	act("You can't wake $M!", ch, NULL, victim, TO_CHAR);
	return;
    }

    act_new("$n wakes you.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
    do_stand(victim, "");
    return;
}



void do_sneak(CHAR_DATA * ch, char *argument)
{
    AFFECT_DATA af;

    /* Added so you can't do this if affected by faerie fire */
    /* Vorlin, 8/23/2000 */

    if (IS_AFFECTED(ch, AFF_SNEAK)) {
	send_to_char("You're already sneaking...\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_FAERIE_FIRE)) {
	send_to_char
	    ("You can't sneak with this glaring `%pink`` aura.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_SNEAK)) {
	send_to_char("You attempt to move silently.\n\r", ch);
	return;
    }

    send_to_char("You attempt to move silently.\n\r", ch);
    affect_strip(ch, gsn_sneak);

    if (number_percent() < get_skill(ch, gsn_sneak)) {
	check_improve(ch, gsn_sneak, TRUE, 3);
	af.where = TO_AFFECTS;
	af.type = gsn_sneak;
	af.level = ch->level;
	af.duration = ch->level;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char(ch, &af);
    } else
	check_improve(ch, gsn_sneak, FALSE, 3);

    return;
}



void do_hide(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;

    if (argument[0] == '\0') {

	if (IS_AFFECTED(ch, AFF_HIDE)) {
	    send_to_char("You're already hiding.\n\r", ch);
	    return;
	}

	/* Added so you can't do this if affected by faerie fire */
	/* Vorlin, 8/23/2000 */

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE)) {
	    send_to_char
		("You can't hide with this glaring `%pink`` aura.\n\r",
		 ch);
	    return;
	}

	send_to_char("You attempt to hide.\n\r", ch);
	affect_strip(ch, gsn_hide);

	if (number_percent() < get_skill(ch, gsn_hide)) {
	    SET_BIT(ch->affected_by, AFF_HIDE);
	    check_improve(ch, gsn_hide, TRUE, 3);
	} else
	    check_improve(ch, gsn_hide, FALSE, 3);
	return;
    } else {
	if ((obj = get_obj_carry(ch, argument)) == NULL) {
	    send_to_char("You aren't carrying that.\n\r", ch);
	    return;
	}

	if (!can_drop_obj(ch, obj)) {
	    send_to_char("You can't let go of it.\n\r", ch);
	    return;
	}

	obj_from_char(obj);
	SET_BIT(obj->extra_flags, ITEM_DARK);
	obj_to_room(obj, ch->in_room);
	act("`2You hide `@$p``.", ch, obj, NULL, TO_CHAR);
	return;
    }

}



/*
 * Contributed by Alander.
 */
void do_visible(CHAR_DATA * ch, char *argument)
{
    affect_strip(ch, gsn_invis);
    affect_strip(ch, gsn_mass_invis);
    affect_strip(ch, gsn_sneak);
    affect_strip(ch, gsn_vanishing);
    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
    REMOVE_BIT(ch->affected_by, AFF_SNEAK);
    send_to_char("Ok.\n\r", ch);
    return;
}



void do_recall(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    sh_int room;

    argument = one_argument(argument, arg);

    REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if (IS_NPC(ch)
	&& (!IS_SET(ch->act, ACT_PET) || !IS_SET(ch->act, ACT_HORSE))) {
	send_to_char("Only players can recall.\n\r", ch);
	return;
    }

    act("$n prays for transportation!", ch, 0, 0, TO_ROOM);
    room = ROOM_VNUM_TEMPLE;


/* Check it out, it's one of Biryu's ex-MudChicks! ^_^ -- Suzuran */
/*
    if ((is_exact_name(ch->name,"Laeris")) && (!str_cmp(arg,"home")))
	room = ROOM_VNUM_BIRYU;
*/

/* Hacked for new clans - Suzuran 
    if ((ch->clan > 0) && (!str_cmp(arg,"clan")))
	room = clan_table[ch->clan].hall;
*/
    if ((ch->clan > 0) && (!str_cmp(arg, "clan")))
	room = get_clan_hall(ch);

    if ((location = get_room_index(room)) == NULL) {
	send_to_char("You are completely lost.\n\r", ch);
	return;
    }

    if (ch->in_room == location)
	return;

    if ((!str_cmp(arg, "clan")) && (ch->fight > 0)) {
	send_to_char("You cannot do that yet.\n\r", ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	|| IS_AFFECTED(ch, AFF_CURSE)) {
	send_to_char("The Light has forsaken you.\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) != NULL) {
	int lose, skill;

	skill = get_skill(ch, gsn_recall);

	if (number_percent() < 80 * skill / 100) {
	    check_improve(ch, gsn_recall, FALSE, 6);
	    WAIT_STATE(ch, 4);
	    sprintf(buf, "You failed!.\n\r");
	    send_to_char(buf, ch);
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp(ch, 0 - lose);
	check_improve(ch, gsn_recall, TRUE, 4);
	sprintf(buf, "You recall from combat!  You lose %d exps.\n\r",
		lose);
	send_to_char(buf, ch);
	stop_fighting(ch, TRUE);

    }

    ch->move /= 5;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    act("$n disappears.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
    do_look(ch, "auto");

    if (ch->horse != NULL && ch->mount == IS_MOUNTED)
	do_recall(ch->horse, "");

    if (ch->pet != NULL)
	do_recall(ch->pet, "");

    /* Reset violence timer if active */
    if (ch->fight > 0) {
	ch->fight = 270;
    }

    return;
}

void do_train(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = -1;
    char *pOutput = NULL;
    int cost;

    if (IS_NPC(ch))
	return;

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    /*
     * Check for trainer.
     */
    for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
	if (IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN))
	    break;
    }

    if (mob == NULL) {
	send_to_char("You can't do that here.\n\r", ch);
	return;
    }

    if (argument[0] == '\0') {
	sprintf(buf, "You have %d training sessions.\n\r", ch->train);
	send_to_char(buf, ch);
	argument = "foo";
    }

    cost = 1;

    if (!str_cmp(argument, "str")) {
	if (class_table[ch->class].attr_prime == STAT_STR)
	    cost = 1;
	stat = STAT_STR;
	pOutput = "strength";
    }

    else if (!str_cmp(argument, "int")) {
	if (class_table[ch->class].attr_prime == STAT_INT)
	    cost = 1;
	stat = STAT_INT;
	pOutput = "intelligence";
    }

    else if (!str_cmp(argument, "wis")) {
	if (class_table[ch->class].attr_prime == STAT_WIS)
	    cost = 1;
	stat = STAT_WIS;
	pOutput = "wisdom";
    }

    else if (!str_cmp(argument, "dex")) {
	if (class_table[ch->class].attr_prime == STAT_DEX)
	    cost = 1;
	stat = STAT_DEX;
	pOutput = "dexterity";
    }

    else if (!str_cmp(argument, "con")) {
	if (class_table[ch->class].attr_prime == STAT_CON)
	    cost = 1;
	stat = STAT_CON;
	pOutput = "constitution";
    }

    else if (!str_cmp(argument, "hp"))
	cost = 1;

    else if (!str_cmp(argument, "mana"))
	cost = 1;

    else if (!str_cmp(argument, "move"))
	cost = 1;

    else {
	strcpy(buf, "You can train:");
	if (ch->perm_stat[STAT_STR] < get_max_train(ch, STAT_STR))
	    strcat(buf, " str");
	if (ch->perm_stat[STAT_INT] < get_max_train(ch, STAT_INT))
	    strcat(buf, " int");
	if (ch->perm_stat[STAT_WIS] < get_max_train(ch, STAT_WIS))
	    strcat(buf, " wis");
	if (ch->perm_stat[STAT_DEX] < get_max_train(ch, STAT_DEX))
	    strcat(buf, " dex");
	if (ch->perm_stat[STAT_CON] < get_max_train(ch, STAT_CON))
	    strcat(buf, " con");
	strcat(buf, " hp mana move");

	if (buf[strlen(buf) - 1] != ':') {
	    strcat(buf, ".\n\r");
	    send_to_char(buf, ch);
	} else {
	    /*
	     * This message dedicated to Jordan ... you big stud!
	     */
	    act("You have nothing left to train, you $T!",
		ch, NULL,
		ch->sex == SEX_MALE ? "big stud" :
		ch->sex == SEX_FEMALE ? "hot babe" : "wild thing",
		TO_CHAR);
	}

	return;
    }

    if (!str_cmp("hp", argument)) {
	if (cost > ch->train) {
	    send_to_char("You don't have enough training sessions.\n\r",
			 ch);
	    return;
	}

	ch->train -= cost;
	ch->pcdata->perm_hit += 10;
	ch->max_hit += 10;
	ch->hit += 10;
	act("Your durability increases!", ch, NULL, NULL, TO_CHAR);
	act("$n's durability increases!", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_cmp("mana", argument)) {
	if (cost > ch->train) {
	    send_to_char("You don't have enough training sessions.\n\r",
			 ch);
	    return;
	}

	ch->train -= cost;
	ch->pcdata->perm_mana += 10;
	ch->max_mana += 10;
	ch->mana += 10;
	act("Your Will increases!", ch, NULL, NULL, TO_CHAR);
	act("$n's Will increases!", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (!str_cmp("move", argument)) {
	if (cost > ch->train) {
	    send_to_char("You don't have enough training sessions.\n\r",
			 ch);
	    return;
	}

	ch->train -= cost;
	ch->pcdata->perm_move += 10;
	ch->max_move += 10;
	ch->move += 10;
	act("Your agility increases!", ch, NULL, NULL, TO_CHAR);
	act("$n's agility increases!", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (ch->perm_stat[stat] >= get_max_train(ch, stat)) {
	act("Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR);
	return;
    }

    if (cost > ch->train) {
	send_to_char("You don't have enough training sessions.\n\r", ch);
	return;
    }

    ch->train -= cost;

    ch->perm_stat[stat] += 1;
    act("Your $T increases!", ch, NULL, pOutput, TO_CHAR);
    act("$n's $T increases!", ch, NULL, pOutput, TO_ROOM);
    return;
}

void do_stonewalk(CHAR_DATA * ch, char *argument)
{
    if ((ch->race != race_lookup("ulgo")) && !IS_IMMORTAL(ch)) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    send_to_char("Sorry, this skill isn't fully implemented yet.\n\r", ch);
    return;
}

/*void do_walkin( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if ( !IS_NPC(ch) )
     {
	smash_tilde( argument );
	
	if (argument[0] == '\0')
	  {
	     sprintf(buf,"Your walkin is %s\n\r",ch->pcdata->walkin);
	     send_to_char(buf, ch);
	     return;
	  }
	
	sprintf(buf, "%s %s", ch->name, argument);
	free_string( ch->pcdata->walkin );
	ch->pcdata->walkin = str_dup( buf );
	
	sprintf(buf,"Your walkin is now %s\n\r",ch->pcdata->walkin);
	send_to_char(buf, ch);
     }
   return;
}

void do_walkout( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if ( !IS_NPC(ch) )
     {
	smash_tilde( argument );
	
	if (argument[0] == '\0')
	  {
	     sprintf(buf,"Your walkout is %s\n\r",ch->pcdata->walkout);
	     send_to_char(buf, ch);
	     return;
	  }
	
	sprintf( buf, "%s %s", ch->name, argument);
	free_string( ch->pcdata->walkout );
	ch->pcdata->walkout = str_dup( buf );
	
	sprintf(buf,"Your walkout is now %s\n\r",ch->pcdata->walkout);
	send_to_char(buf, ch);
     }
   return;
}
*/

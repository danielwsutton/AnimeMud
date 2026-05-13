/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

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
#include "merc.h"
#include "tables.h"

ROOM_INDEX_DATA *find_location args((CHAR_DATA * ch, char *arg));
int flag_lookup
args((const char *name, const struct flag_type * flag_table));

void do_flag(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
	arg3[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *room;
    long *flag, old = 0, new = 0, marked = 0, pos;
    char type;
    int list;
    const struct flag_type *flag_table;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    type = argument[0];

    if (type == '=' || type == '-' || type == '+')
	argument = one_argument(argument, word);

    if (arg1[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char("  flag mob  <name> <field> <flags>\n\r", ch);
	send_to_char("  flag char <name> <field> <flags>\n\r", ch);
	send_to_char("  flag obj  <name> <field> <flags>\n\r", ch);
	send_to_char("  flag room <room> <field> <flags>\n\r", ch);
	send_to_char
	    ("  mob  flags: act,aff,off,imm,res,vuln,form,part\n\r", ch);
	send_to_char("  char flags: plr,comm,aff,imm,res,vuln\n\r", ch);
	send_to_char("  obj  flags: extra,wear,weap\n\r", ch);
	send_to_char("  room flags: flags\n\r", ch);
	send_to_char("  +: add flag, -: remove flag, = set equal to\n\r",
		     ch);
	send_to_char("  otherwise flag toggles the flags listed.\n\r", ch);
	send_to_char("  To list the flags of an area:\n\r", ch);
	send_to_char("  flag list <mob:char:obj:room> <field>\n\r", ch);
	return;
    }

    if (arg3[0] == '\0') {
	send_to_char("You need to specify a flag to set/list.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "list")) {
	flag_table = NULL;
	if (arg2[0] == '\0') {
	    send_to_char("You must provide an area to list.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg2, "room")) {
	    if (!str_prefix(arg3, "flags")) {
		flag_table = room_flags;
	    } else {
		send_to_char("That isn't an exceptable flag area.\n\r",
			     ch);
		return;
	    }
	} else if (!str_prefix(arg2, "obj")) {
	    if (!str_prefix(arg3, "extra")) {
		flag_table = extra_flags;
	    } else if (!str_prefix(arg3, "weap")) {
		flag_table = weapon_flags;
	    } else if (!str_prefix(arg3, "wear")) {
		flag_table = wear_flags;
	    } else {
		send_to_char("You must provide an area to list.\n\r", ch);
		return;
	    }
	} else if (!str_prefix(arg2, "mob") || !str_prefix(arg2, "char")) {
	    if (!str_prefix(arg3, "act")) {
		flag_table = act_flags;
	    } else if (!str_prefix(arg3, "plr")) {
		flag_table = plr_flags;
	    } else if (!str_prefix(arg3, "aff")) {
		flag_table = affect_flags;
	    } else if (!str_prefix(arg3, "immunity")
		       || !str_prefix(arg3, "resist")
		       || !str_prefix(arg3, "vuln")) {
		flag_table = imm_flags;
	    } else if (!str_prefix(arg3, "form")) {
		flag_table = form_flags;
	    } else if (!str_prefix(arg3, "parts")) {
		flag_table = part_flags;
	    } else if (!str_prefix(arg3, "comm")) {
		flag_table = comm_flags;
	    } else {
		send_to_char("That's not an acceptable flag area.\n\r",
			     ch);
		return;
	    }
	}

	if (flag_table != NULL) {
	    send_to_char("Here's a list of flags for chosen type.\n\r",
			 ch);
	    send_to_char
		("\t     `2F`@lag `4N`$ame    `1S`!ettable	`6M`^in`&. `6L`^evel``\n\r",
		 ch);
	    for (list = 0; flag_table[list].name != NULL; list++) {
		sprintf(buf, "    %15s         %s          %d\n\r",
			flag_table[list].name,
			(flag_table[list].settable) ? "`@Yes``" : "`1No``",
			flag_table[list].level);
		send_to_char(buf, ch);
	    }
	    return;
	} else {
	    send_to_char("flag list <room:obj:char:mob> <flag area>\n\r",
			 ch);
	    return;
	}
    }



    if (argument[0] == '\0') {
	send_to_char("Which flags do you wish to change?\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "room")) {

	if (
	    (room =
	     ((arg2[0] == '\0') ? ch->in_room : find_location(ch, arg2)))
	    == NULL) {
	    send_to_char("No such location.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "flags")) {
	    flag = &room->room_flags;
	    flag_table = room_flags;
	} else {
	    send_to_char("That isn't an exceptable flag area.\n\r", ch);
	    return;
	}
	old = *flag;

	if (type != '=')
	    new = old;

	/* mark the words */
	for (;;) {
	    argument = one_argument(argument, word);

	    if (word[0] == '\0')
		break;

	    pos = flag_lookup(word, flag_table);
	    if (pos == 0) {
		send_to_char("That flag doesn't exist!\n\r", ch);
		return;
	    } else
		SET_BIT(marked, pos);
	}


	for (pos = 0; flag_table[pos].name != NULL; pos++) {
	    if (!flag_table[pos].settable
		&& IS_SET(old, flag_table[pos].bit)) {
		SET_BIT(new, flag_table[pos].bit);
		continue;
	    }

	    if (IS_SET(marked, flag_table[pos].bit)) {
		switch (type) {
		case '=':
		case '+':
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else
			SET_BIT(new, flag_table[pos].bit);
		    break;
		case '-':
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else
			REMOVE_BIT(new, flag_table[pos].bit);
		    break;
		default:
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else if (IS_SET(new, flag_table[pos].bit))
			REMOVE_BIT(new, flag_table[pos].bit);
		    else
			SET_BIT(new, flag_table[pos].bit);
		}
	    }
	}
	*flag = new;
	return;
    }

    if (arg2[0] == '\0') {
	send_to_char("What do you wish to set flags on?\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "obj")) {

	if ((obj = get_obj_world(ch, arg2)) == NULL) {
	    send_to_char("It isn't there.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "extra")) {
	    flag = &obj->extra_flags;
	    flag_table = extra_flags;
	} else if (!str_prefix(arg3, "weap")) {
	    flag = &obj->value[0];
	    flag_table = weapon_flags;
	} else if (!str_prefix(arg3, "wear")) {
	    flag = &obj->wear_flags;
	    flag_table = wear_flags;
	} else {
	    send_to_char("That's not an exceptable flag area.\n\r", ch);
	    return;
	}

	old = *flag;

	if (type != '=')
	    new = old;

	/* mark the words */
	for (;;) {
	    argument = one_argument(argument, word);

	    if (word[0] == '\0')
		break;

	    pos = flag_lookup(word, flag_table);
	    if (pos == 0) {
		send_to_char("That flag doesn't exist!\n\r", ch);
		return;
	    } else
		SET_BIT(marked, pos);
	}

	for (pos = 0; flag_table[pos].name != NULL; pos++) {
	    if (!flag_table[pos].settable
		&& IS_SET(old, flag_table[pos].bit)) {
		SET_BIT(new, flag_table[pos].bit);
		continue;
	    }

	    if (IS_SET(marked, flag_table[pos].bit)) {
		switch (type) {
		case '=':
		case '+':
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else
			SET_BIT(new, flag_table[pos].bit);
		    break;
		case '-':
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else
			REMOVE_BIT(new, flag_table[pos].bit);
		    break;
		default:
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else if (IS_SET(new, flag_table[pos].bit))
			REMOVE_BIT(new, flag_table[pos].bit);
		    else
			SET_BIT(new, flag_table[pos].bit);
		}
	    }
	}
	*flag = new;
	return;
    }

    if (!str_prefix(arg1, "mob") || !str_prefix(arg1, "char")) {
	victim = get_char_world(ch, arg2);
	if (victim == NULL) {
	    send_to_char("You can't find them.\n\r", ch);
	    return;
	}

	/* select a flag to set */
	if (!str_prefix(arg3, "act")) {
	    if (!IS_NPC(victim)) {
		send_to_char("Use plr for PCs.\n\r", ch);
		return;
	    }

	    flag = &victim->act;
	    flag_table = act_flags;
	}

	else if (!str_prefix(arg3, "plr")) {
	    if (IS_NPC(victim)) {
		send_to_char("Use act for NPCs.\n\r", ch);
		return;
	    }

	    flag = &victim->act;
	    flag_table = plr_flags;
	}

	else if (!str_prefix(arg3, "aff")) {
	    flag = &victim->affected_by;
	    flag_table = affect_flags;
	}

	else if (!str_prefix(arg3, "immunity")) {
	    flag = &victim->imm_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3, "resist")) {
	    flag = &victim->res_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3, "vuln")) {
	    flag = &victim->vuln_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3, "form")) {
	    if (!IS_NPC(victim)) {
		send_to_char("Form can't be set on PCs.\n\r", ch);
		return;
	    }

	    flag = &victim->form;
	    flag_table = form_flags;
	}

	else if (!str_prefix(arg3, "parts")) {
	    if (!IS_NPC(victim)) {
		send_to_char("Parts can't be set on PCs.\n\r", ch);
		return;
	    }

	    flag = &victim->parts;
	    flag_table = part_flags;
	}

	else if (!str_prefix(arg3, "comm")) {
	    if (IS_NPC(victim)) {
		send_to_char("Comm can't be set on NPCs.\n\r", ch);
		return;
	    }

	    flag = &victim->comm;
	    flag_table = comm_flags;
	}

	else {
	    send_to_char("That's not an acceptable flag.\n\r", ch);
	    return;
	}

	old = *flag;
	victim->zone = NULL;


	if (type != '=')
	    new = old;

	/* mark the words */
	for (;;) {
	    argument = one_argument(argument, word);

	    if (word[0] == '\0')
		break;

	    pos = flag_lookup(word, flag_table);
	    if (pos == 0) {
		send_to_char("That flag doesn't exist!\n\r", ch);
		return;
	    } else
		SET_BIT(marked, pos);
	}

	for (pos = 0; flag_table[pos].name != NULL; pos++) {
	    if (!flag_table[pos].settable
		&& IS_SET(old, flag_table[pos].bit)) {
		SET_BIT(new, flag_table[pos].bit);
		continue;
	    }

	    if (IS_SET(marked, flag_table[pos].bit)) {
		switch (type) {
		case '=':
		case '+':
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else
			SET_BIT(new, flag_table[pos].bit);
		    break;
		case '-':
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else
			REMOVE_BIT(new, flag_table[pos].bit);
		    break;
		default:
		    if (flag_table[pos].level > get_trust(ch)) {
			send_to_char
			    ("You aren't of level to set that.\n\r", ch);
			break;
		    } else if (IS_SET(new, flag_table[pos].bit))
			REMOVE_BIT(new, flag_table[pos].bit);
		    else
			SET_BIT(new, flag_table[pos].bit);
		}
	    }
	}
	*flag = new;
	return;
    }
}

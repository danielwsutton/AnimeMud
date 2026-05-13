#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "ansi.h"
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"

/* command procedures needed */
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_affects);
DECLARE_DO_FUN(do_play);


char *const where_name[] = {
    "`&<``used as light`&>``     ",
    "`&<``worn on finger`&>``    ",
    "`&<``worn on finger`&>``    ",
    "`&<``worn around neck`&>``  ",
    "`&<``worn around neck`&>``  ",
    "`&<``worn on torso`&>``     ",
    "`&<``worn on head`&>``      ",
    "`&<``worn on legs`&>``      ",
    "`&<``worn on feet`&>``      ",
    "`&<``worn on hands`&>``     ",
    "`&<``worn on arms`&>``      ",
    "`&<``worn as shield`&>``    ",
    "`&<``worn about body`&>``   ",
    "`&<``worn about waist`&>``  ",
    "`&<``worn around wrist`&>`` ",
    "`&<``worn around wrist`&>`` ",
    "`&<``wielded`&>``           ",
    "`&<``held`&>``              ",
    "`&<``secondary weapon`&>``  ",
    "`&<``floating nearby`&>``   ",
};



/* for  keeping track of the player count */
int max_on = 0;

/*
 * Local functions.
 */
char *format_obj_to_char
args((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort));
void show_list_to_char
args((OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing));
void show_char_to_char_0 args((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char_1 args((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char args((CHAR_DATA * list, CHAR_DATA * ch));
bool check_blind args((CHAR_DATA * ch));
/*EE960530*/
void show_scar args((CHAR_DATA * victim, CHAR_DATA * ch));

/* Imported functions. */
int gun_range args((OBJ_DATA * gun));

/* 
* For grant
*/
bool is_command(char *arg)
{
    int cmd;

    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
	if (UPPER(arg[0]) == UPPER(cmd_table[cmd].name[0])
	    && is_exact_name(cmd_table[cmd].name, arg))
	    return TRUE;

    return FALSE;
}



char *format_obj_to_char(OBJ_DATA * obj, CHAR_DATA * ch, bool fShort)
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if (
	(fShort
	 && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
	|| (obj->description == NULL || obj->description[0] == '\0'))
	return buf;

    if (IS_OBJ_STAT(obj, ITEM_INVIS))
	strcat(buf, "`$(`4Invis`$)`` ");
    if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_EVIL))
	strcat(buf, "(`!R`1ed `!A`1ura``) ");
    if (IS_AFFECTED(ch, AFF_DETECT_GOOD) && IS_OBJ_STAT(obj, ITEM_BLESS))
	strcat(buf, "(`$B`4lue `$A`4ura``) ");
    if (IS_AFFECTED(ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC))
	strcat(buf, "`6(`^Magical`6)`` ");
    if (IS_AFFECTED(ch, AFF_DETECT_HIDDEN) && IS_OBJ_STAT(obj, ITEM_DARK))
	strcat(buf, "(`8Dark``) ");
    if (IS_OBJ_STAT(obj, ITEM_GLOW))
	strcat(buf, "(`3G`#l``o`&w``i`#n`3g``) ");
    if (IS_OBJ_STAT(obj, ITEM_HUM))
	strcat(buf, "(`5H`%u``m`&m``i`%n`5g``) ");

    if (fShort) {
	if (obj->short_descr != NULL)
	    strcat(buf, obj->short_descr);
    } else if (obj->description != NULL)
	strcat(buf, obj->description);

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void
show_list_to_char(OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
		  bool fShowNothing)
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if (ch->desc == NULL)
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();
    count = 0;

    for (obj = list; obj != NULL; obj = obj->next_content)
	count++;

    prgpstrShow = alloc_mem(count * sizeof(char *));
    prgnShow = alloc_mem(count * sizeof(int));
    nShow = 0;

    /*
     * Format the list of objects.
     */
    for (obj = list; obj != NULL; obj = obj->next_content) {
	if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj)) {
	    pstrShow = format_obj_to_char(obj, ch, fShort);
	    fCombine = FALSE;

	    if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for (iShow = nShow - 1; iShow >= 0; iShow--) {
		    if (!strcmp(prgpstrShow[iShow], pstrShow)) {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if (!fCombine) {
		prgpstrShow[nShow] = str_dup(pstrShow);
		prgnShow[nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for (iShow = 0; iShow < nShow; iShow++) {
	if (prgpstrShow[iShow][0] == '\0') {
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
	    if (prgnShow[iShow] != 1) {
		sprintf(buf, "(%2d) ", prgnShow[iShow]);
		add_buf(output, buf);
	    } else {
		add_buf(output, "     ");
	    }
	}

	add_buf(output, prgpstrShow[iShow]);
	add_buf(output, "\n\r");
	free_string(prgpstrShow[iShow]);
    }

    if (fShowNothing && nShow == 0) {
	if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE))
	    send_to_char("     ", ch);

	send_to_char("Nothing.\n\r", ch);
    }

    page_to_char(buf_string(output), ch);

    /*
     * Clean up.
     */
    free_buf(output);
    free_mem(prgpstrShow, count * sizeof(char *));
    free_mem(prgnShow, count * sizeof(int));

    return;
}


void show_char_to_char_0(CHAR_DATA * victim, CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if (IS_NPC(victim) && IS_SET(victim->act, ACT_HORSE)
	&& victim->master != NULL && victim->leader != NULL
	&& victim->master == victim->leader
	&& victim->in_room == victim->master->in_room
	&& victim->master->mount == IS_MOUNTED)
	return;

    if (victim->desc == NULL && !IS_NPC(victim))
	strcat(buf, "`8[`^LINK`6-`^DEAD`8]`` ");
    if (IS_SET(victim->comm, COMM_AFK))
	strcat(buf, "[`1A`!F`1K``] ");
    if (IS_AFFECTED(victim, AFF_INVISIBLE))
	strcat(buf, "(`iInvis``) ");
    if (victim->invis_level >= LEVEL_HERO)
	strcat(buf, "(`wWizi``) ");
    if (IS_AFFECTED(victim, AFF_HIDE))
	strcat(buf, "(`hHide``) ");
    if (IS_AFFECTED(victim, AFF_CHARM))
	strcat(buf, "`#(`cCharmed`#)`` ");
    if (IS_AFFECTED(victim, AFF_PASS_DOOR))
	strcat(buf, "(`^Translucent``) ");
    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
	strcat(buf, "`!(`%Pink Aura`!)`` ");
    if (IS_AFFECTED(victim, AFF_FLYING))
	strcat(buf, "`&(`8F``lo`&at``in`8g`&)`` ");
    if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
	strcat(buf, "(`!R`1ed `!A`1ura``) ");
    if (IS_GOOD(victim) && IS_AFFECTED(ch, AFF_DETECT_GOOD))
	strcat(buf, "(`#Golden Aura``) ");
    if (IS_AFFECTED(victim, AFF_SANCTUARY))
	strcat(buf, "(`&White Aura``) ");
    if (!IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER))
	strcat(buf, "(`@KILLER``) ");
    if (!IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF))
	strcat(buf, "(`8THIEF``) ");
    if (!IS_NPC(victim) && IS_SET(victim->act, PLR_DEAD))
	strcat(buf, "(`8DEAD``) ");
    if ((!IS_NPC(victim))
	&& ((IS_QUESTOR(victim) && (victim->pcdata->questgiver != NULL))
	    && IS_QUESTPKMST(victim->pcdata->questgiver))) {
	strcat(buf, "(`!PK`1Qst``) ");
    }
    if (IS_NPC(victim) && ch->questmob > 0
	&& victim->pIndexData->vnum == ch->questmob)
	    strcat(buf, "`8[`@TARGET`8]`` ");
    if (victim->position == victim->start_pos
	&& victim->long_descr[0] != '\0') {
	strcat(buf, victim->long_descr);
	send_to_char(buf, ch);
	return;
    }

    strcat(buf, PERS(victim, ch));

    if (!IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF)
	&& victim->position == POS_STANDING && ch->on == NULL)
	strcat(buf, victim->pcdata->title);

    switch (victim->position) {
    case POS_DEAD:
	strcat(buf, " is DEAD!!");
	break;
    case POS_MORTAL:
	strcat(buf, " is mortally wounded.");
	break;
    case POS_INCAP:
	strcat(buf, " is incapacitated.");
	break;
    case POS_STUNNED:
	strcat(buf, " is lying here stunned.");
	break;
    case POS_SLEEPING:
	if (victim->on != NULL) {
	    if (IS_SET(victim->on->value[2], SLEEP_AT)) {
		sprintf(message, " is sleeping at %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else if (IS_SET(victim->on->value[2], SLEEP_ON)) {
		sprintf(message, " is sleeping on %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else {
		sprintf(message, " is sleeping in %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    }
	} else
	    strcat(buf, " is sleeping here.");
	break;
    case POS_RESTING:
	if (victim->on != NULL) {
	    if (IS_SET(victim->on->value[2], REST_AT)) {
		sprintf(message, " is resting at %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else if (IS_SET(victim->on->value[2], REST_ON)) {
		sprintf(message, " is resting on %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else {
		sprintf(message, " is resting in %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    }
	} else
	    strcat(buf, " is resting here.");
	break;
    case POS_SITTING:
	if (victim->on != NULL) {
	    if (IS_SET(victim->on->value[2], SIT_AT)) {
		sprintf(message, " is sitting at %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else if (IS_SET(victim->on->value[2], SIT_ON)) {
		sprintf(message, " is sitting on %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else {
		sprintf(message, " is sitting in %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    }
	} else
	    strcat(buf, " is sitting here.");
	break;
	if (victim->on != NULL) {
	    if (IS_SET(victim->on->value[2], STAND_AT)) {
		sprintf(message, " is standing at %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else if (IS_SET(victim->on->value[2], STAND_ON)) {
		sprintf(message, " is standing on %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else {
		sprintf(message, " is standing in %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    }
	}
	    else if (!IS_NPC(victim) && victim->horse != NULL
		     && victim->in_room == victim->horse->in_room
		     && victim->mount == IS_MOUNTED) {
	    strcat(buf, " is here ridding on ");
	    strcat(buf, victim->horse->short_descr);
	    strcat(buf, ".");
	    break;
	} else
	    strcat(buf, " is here.");
	break;

    case POS_STANDING:
	if (victim->on != NULL) {
	    if (IS_SET(victim->on->value[2], STAND_AT)) {
		sprintf(message, " is standing at %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else if (IS_SET(victim->on->value[2], STAND_ON)) {
		sprintf(message, " is standing on %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    } else {
		sprintf(message, " is standing in %s.",
			victim->on->short_descr);
		strcat(buf, message);
	    }
	}
	    else if (!IS_NPC(victim) && victim->horse != NULL
		     && victim->in_room == victim->horse->in_room
		     && victim->mount == IS_MOUNTED) {
	    strcat(buf, " is here ridding on ");
	    strcat(buf, victim->horse->short_descr);
	    strcat(buf, ".");
	    break;
	} else
	    strcat(buf, " is here.");
	break;

    case POS_FIGHTING:
	strcat(buf, " is here, fighting ");
	if (victim->fighting == NULL)
	    strcat(buf, "thin air??");
	else if (victim->fighting == ch)
	    strcat(buf, "YOU!");
	else if (victim->in_room == victim->fighting->in_room) {
	    strcat(buf, PERS(victim->fighting, ch));
	    strcat(buf, ".");
	} else
	    strcat(buf, "someone who left??");
	break;
    }

    strcat(buf, "\n\r");
    buf[0] = UPPER(buf[0]);
    send_to_char(buf, ch);

    return;
}



/*EE960530*/
void show_scar(CHAR_DATA * victim, CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    int i, n = 0;

    if (victim->pcdata->pkset == FALSE || victim->pcdata->pdeaths == 0)
	return;

    if (victim->pcdata->pdeaths > 15) {
	act("$N's whole body is covered by scars. Ouch, must have hurt!",
	    ch, NULL, victim, TO_CHAR);
	return;
    } else if (victim->pcdata->pdeaths > 10) {
	act("$N has more scars than anyone you've seen before!", ch, NULL,
	    victim, TO_CHAR);
	return;
    } else if (victim->pcdata->pdeaths > 5) {
	act("$N has plenty of scars, on different places on $S body.", ch,
	    NULL, victim, TO_CHAR);
	return;
    }

    sprintf(buf, "%s has ", PERS(victim, ch));

    /*for (i = 1; i <= SCAR_END; i = i*2) */
    for (i = 1; i <= 32768; i = i * 2) {
	if (IS_SET((victim)->pcdata->scars, i)) {
	    n++;
	    /*           switch (i)
	       {
	       case SCAR_NONE:                      strcat(buf, "no scars"                                          ); break;
	       case SCAR_RIGHT_CHEEK:       strcat(buf, "one scar on $S right cheek"        ); break;
	       case SCAR_LEFT_CHEEK:        strcat(buf, "one scar across $S left cheek"     ); break;
	       case SCAR_FOREHEAD:          strcat(buf, "one scar across $S forehead"       ); break;
	       case SCAR_RIGHT_HAND:        strcat(buf, "a big scar from $S right hand to $S elbow"); break;
	       case SCAR_LEFT_HAND: strcat(buf, "one scar on $S left hand"          ); break;
	       case SCAR_LARYNX:            strcat(buf, "one scar just above $S larynx"     ); break;
	       case SCAR_RIGHT_EAR: strcat(buf, "one big scar where $S right ear used to be"); break;
	       case SCAR_LEFT_EAR:          strcat(buf, "only a scar where $S left ear used to be"); break;
	       case SCAR_RIGHT_KNEE:        strcat(buf, "one scar below $S right knee"      ); break;
	       case SCAR_LEFT_KNEE: strcat(buf, "one scar above $S right knee"      ); break;
	       case SCAR_RIGHT_FINGER:      strcat(buf, "no little finger on $S right hand"); break;
	       case SCAR_LEFT_FINGER:       strcat(buf, "no forefinger on $S left hand"     ); break;
	       default:                             strcat(buf, "no scars"                                          ); break;
	       } */
	    if (n == (victim->pcdata->pdeaths - 1))
		strcat(buf, " and ");
	    else if (n == victim->pcdata->pdeaths)
		strcat(buf, ".");
	    else
		strcat(buf, ", ");
	}

	if (buf[strlen(buf) - 1] == '.')
	    break;
    }
    act(buf, ch, NULL, victim, TO_CHAR);

    return;
}



void show_char_to_char_1(CHAR_DATA * victim, CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if (can_see(victim, ch)) {
	if (ch == victim)
	    act("$n looks at $mself.", ch, NULL, NULL, TO_ROOM);
	else {
	    act("$n looks at you.", ch, NULL, victim, TO_VICT);
	    act("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
	}
    }

    /* Added this to toggle descriptions, just like racial says */
    /* 8/26/2000 --Vorlin */

    if (victim->description[0] != '\0' && !IS_SET(ch->act, PLR_DESCR)) {
	send_to_char(victim->description, ch);
    }
	else if (victim->description[0] != '\0'
		 && IS_SET(ch->act, PLR_DESCR)) {
	act("Description hidden for $N.", ch, NULL, victim, TO_CHAR);
    } else {
	act("You see nothing special about $M.", ch, NULL, victim,
	    TO_CHAR);
    }

    if (victim->max_hit > 0)
	percent = (100 * victim->hit) / victim->max_hit;
    else
	percent = -1;

    strcpy(buf, PERS(victim, ch));

    if (percent >= 100)
	strcat(buf, " is in excellent condition.\n\r");
    else if (percent >= 90)
	strcat(buf, " has a few scratches.\n\r");
    else if (percent >= 75)
	strcat(buf, " has some small wounds and bruises.\n\r");
    else if (percent >= 50)
	strcat(buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat(buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat(buf, " looks pretty hurt.\n\r");
    else if (percent >= 0)
	strcat(buf, " is in awful condition.\n\r");
    else
	strcat(buf, " is bleeding to death.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char(buf, ch);
    found = FALSE;

    for (iWear = 0; iWear < MAX_WEAR; iWear++) {
	if ((obj = get_eq_char(victim, iWear)) != NULL
	    && can_see_obj(ch, obj)) {
	    if (!found) {
		send_to_char("\n\r", ch);
		act("$N is using:", ch, NULL, victim, TO_CHAR);
		found = TRUE;
	    }

	    send_to_char(where_name[iWear], ch);
	    send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
	    send_to_char("\n\r", ch);
	}
    }

    if (victim != ch && !IS_NPC(ch)
	&& number_percent() < get_skill(ch, gsn_peek)) {
	send_to_char("\n\rYou peek at the inventory:\n\r", ch);
	check_improve(ch, gsn_peek, TRUE, 4);
	//TDG000207 added so that peek can see gold and silver
	sprintf(buf, "\t%ld gold and %ld silver.\n\r", victim->gold,
		victim->silver);
	send_to_char(buf, ch);
	show_list_to_char(victim->carrying, ch, TRUE, TRUE);
    }

    return;
}



void show_char_to_char(CHAR_DATA * list, CHAR_DATA * ch)
{
    CHAR_DATA *rch;

    for (rch = list; rch != NULL; rch = rch->next_in_room) {
	if (rch == ch)
	    continue;

	if (get_trust(ch) < rch->invis_level)
	    continue;

	if (can_see(ch, rch)) {
	    show_char_to_char_0(rch, ch);
	} else if (room_is_dark(ch->in_room)
		   && IS_AFFECTED(rch, AFF_INFRARED)) {
	    send_to_char
		("`AYou see glowing `!red `Aeyes watching YOU!``\n\r", ch);
	}
    }

    return;
}



bool check_blind(CHAR_DATA * ch)
{
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
	return TRUE;

    if (IS_AFFECTED(ch, AFF_BLIND)) {
	send_to_char("You can't see a thing!\n\r", ch);
	return FALSE;
    }

    return TRUE;
}



/* changes your scroll */
void do_scroll(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r", ch);
	else {
	    sprintf(buf, "You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf, ch);
	}

	return;
    }

    if (!is_number(arg)) {
	send_to_char("You must provide a number.\n\r", ch);
	return;
    }

    lines = atoi(arg);

    /*if (lines == 0) {
       send_to_char("You may only set your paging as low as 10.\n\r", ch);
       return;
       }
     */

    if (lines < 10 || lines > 100) {
	send_to_char("You must provide a reasonable number.\n\r", ch);
	return;
    }

    sprintf(buf, "Scroll set to %d lines.\n\r", lines);
    send_to_char(buf, ch);
    ch->lines = lines - 2;
}



/* Does Color */
void send_color_status(CHAR_DATA * ch, char *a, char *c, byte b)
{
    sprintf(c, "%s %s %s\n\r", color_table[b], a, ANSI_NORMAL);
    send_to_char(c, ch);
}



/*DC962412 * Changed, updated, and cleaned up to make more ledgible. */
void do_color(CHAR_DATA * ch, char *argument)
{
    int i = 0;
    int a = 0, b = 0;
    char buf[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    if (IS_NPC(ch)) {
	send_to_char("NPC's can't see colors!\n", ch);
	return;
    } else if (argument[0] == 0) {
	if (ch->color) {
	    send_to_char("Ansi color turned `4o`$f`4f``.\n\r", ch);
	    ch->color = 0;
	    return;
	} else {
	    ch->color = 1;
	    send_to_char
		("`!A`@n`#s`$i `Ac``o`&l``o`Ar`` turned `!o`1n``.\n\r",
		 ch);
	    return;
	}
    } else {
	argument = one_argument(argument, buf);

	if (strcasecmp(buf, "list") == 0) {
	    i = 0;

	    send_to_char("`&[`4C`$o`^l`$o`4r`&][`@Set`&#][`%Char`&]``\n\r",
			 ch);

	    while (color_table[i]) {
		if (i < 15) {
		    sprintf(buf, " %s*****    `&%2d     %s``\n\r",
			    color_table[i], i, color_symbol[i]);
		    send_to_char(buf, ch);
		    i++;
		    if (i > 14)
			return;
		}
	    }
	    return;
	} else if (strcasecmp(buf, "show") == 0) {
	    send_to_char("Listing color setup.\nNumber of option\n", ch);
	    send_color_status(ch, " 1 combat melee (self)", buf,
			      ch->pcdata->color_combat_s);
	    send_color_status(ch, " 2 combat melee (opponent)", buf,
			      ch->pcdata->color_combat_o);
	    send_color_status(ch, " 3 combat condition (self)", buf,
			      ch->pcdata->color_combat_condition_s);
	    send_color_status(ch, " 4 combat condition (opponent)", buf,
			      ch->pcdata->color_combat_condition_o);
	    send_color_status(ch, " 5 wizi mobs", buf,
			      ch->pcdata->color_wizi);
	    send_color_status(ch, " 6 invis mobs", buf,
			      ch->pcdata->color_invis);
	    send_color_status(ch, " 7 hidden mobs", buf,
			      ch->pcdata->color_hidden);
	    send_color_status(ch, " 8 charmed mobs", buf,
			      ch->pcdata->color_charmed);
	    send_color_status(ch, " 9 hp", buf, ch->pcdata->color_hp);
	    send_color_status(ch, "10 mana", buf, ch->pcdata->color_mana);
	    send_color_status(ch, "11 movement", buf,
			      ch->pcdata->color_move);
	    send_color_status(ch, "12 say", buf, ch->pcdata->color_say);
	    send_color_status(ch, "13 tell", buf, ch->pcdata->color_tell);
	    send_color_status(ch, "14 reply", buf,
			      ch->pcdata->color_reply);
	    send_color_status(ch, "15 psi", buf, ch->pcdata->color_psi);
	} else if (strcasecmp(buf, "set") == 0) {
	    argument = one_argument(argument, arg1);
	    argument = one_argument(argument, arg2);

	    if (arg1[0] == '\0' || arg2[0] == '\0')
		return;

	    i = 0;
	    a = atoi(arg1);
	    b = atoi(arg2);

	    switch (a) {
	    case 1:
		ch->pcdata->color_combat_s = b;
		send_to_char("`a***** `&combat melee (self)``\n\r", ch);
		break;
	    case 2:
		ch->pcdata->color_combat_o = b;
		send_to_char("`a***** `&combat melee (opponent)``\n\r",
			     ch);
		break;
	    case 3:
		ch->pcdata->color_combat_condition_s = b;
		send_to_char("`C***** `&combat condition (self)``\n\r",
			     ch);
		break;
	    case 4:
		ch->pcdata->color_combat_condition_o = b;
		send_to_char("`D***** `&combat condition (opponent)``\n\r",
			     ch);
		break;
	    case 5:
		ch->pcdata->color_wizi = b;
		send_to_char("`w***** `&wizi mobs/exits``\n\r", ch);
		break;
	    case 6:
		ch->pcdata->color_invis = b;
		send_to_char("`i***** `&invis``\n\r", ch);
		break;
	    case 7:
		ch->pcdata->color_hidden = b;
		send_to_char("`h***** `&hidden mobs``\n\r", ch);
		break;
	    case 8:
		ch->pcdata->color_charmed = b;
		send_to_char("`c***** `&charmed mobs``\n\r", ch);
		break;
	    case 9:
		ch->pcdata->color_hp = b;
		send_to_char("`H***** `&hp``\n\r", ch);
		break;
	    case 10:
		ch->pcdata->color_mana = b;
		send_to_char("`M***** `&mana``\n\r", ch);
		break;
	    case 11:
		ch->pcdata->color_move = b;
		send_to_char("`V***** `&movement``\n\r", ch);
		break;
	    case 12:
		ch->pcdata->color_say = b;
		send_to_char("`s***** `&say``\n\r", ch);
		break;
	    case 13:
		ch->pcdata->color_tell = b;
		send_to_char("`t***** `&tell``\n\r", ch);
		break;
	    case 14:
		ch->pcdata->color_reply = b;
		send_to_char("`r***** `&reply``\n\r", ch);
		break;
	    default:
		send_to_char("Change color for which option?\n\r", ch);
		break;
	    }
	} else
	    send_to_char("Color what?\n", ch);
    }
}



/* RT does socials */
void do_socials(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int col = 0;
    int iSocial;

    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++) {
	sprintf(buf, "%-12s", social_table[iSocial].name);
	send_to_char(buf, ch);

	if (++col % 6 == 0)
	    send_to_char("\n\r", ch);
    }

    if (col % 6 != 0)
	send_to_char("\n\r", ch);

    return;
}



/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_motd(CHAR_DATA * ch, char *argument)
{
    do_help(ch, "motd");
}

void do_imotd(CHAR_DATA * ch, char *argument)
{
    do_help(ch, "imotd");
}

void do_rules(CHAR_DATA * ch, char *argument)
{
    do_help(ch, "rules");
}

void do_story(CHAR_DATA * ch, char *argument)
{
    do_help(ch, "story");
}

void do_wizlist(CHAR_DATA * ch, char *argument)
{
    do_help(ch, "wizlist");
}



/* RT this following section holds all the auto commands from ROM, as well as
 * replacements for config */
void do_autolist(CHAR_DATA * ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
	return;

    send_to_char("   action     status\n\r", ch);
    send_to_char("`A-``--`&---------------``--`A-``\n\r", ch);

    send_to_char("Autoassist     ", ch);
    if (IS_SET(ch->act, PLR_AUTOASSIST))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Autoexit       ", ch);
    if (IS_SET(ch->act, PLR_AUTOEXIT))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Autogold       ", ch);
    if (IS_SET(ch->act, PLR_AUTOGOLD))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Autoloot       ", ch);
    if (IS_SET(ch->act, PLR_AUTOLOOT))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Autosac        ", ch);
    if (IS_SET(ch->act, PLR_AUTOSAC))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Autosplit      ", ch);
    if (IS_SET(ch->act, PLR_AUTOSPLIT))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Autodamage     ", ch);
    if (IS_SET(ch->act, PLR_AUTODAMAGE))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Compact mode   ", ch);
    if (IS_SET(ch->comm, COMM_COMPACT))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Prompt         ", ch);
    if (IS_SET(ch->comm, COMM_PROMPT))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Combine items  ", ch);
    if (IS_SET(ch->comm, COMM_COMBINE))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    /*if (!IS_SET(ch->act, PLR_CANLOOT))
       send_to_char("Your corpse is safe from thieves.\n\r",ch);
       else
       send_to_char("Your corpse may be looted.\n\r",ch);
     */

    if (IS_SET(ch->act, PLR_NOSUMMON))
	send_to_char("You cannot be summoned.\n\r", ch);
    else
	send_to_char("You can be summoned.\n\r", ch);

    if (IS_SET(ch->act, PLR_NOFOLLOW))
	send_to_char("You do not welcome followers.\n\r", ch);
    else
	send_to_char("You accept followers.\n\r", ch);

    send_to_char("Racial says    ", ch);
    if (IS_SET(ch->act, PLR_SAYS))
	send_to_char("`&ON``.\n\r", ch);
    else
	send_to_char("`&OFF``.\n\r", ch);

    send_to_char("Descriptions   ", ch);
    if (IS_SET(ch->act, PLR_DESCR))
	send_to_char("`&OFF``.\n\r", ch);
    else
	send_to_char("`&ON``.\n\r", ch);

}

void do_autoassist(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act, PLR_AUTOASSIST)) {
	send_to_char("Autoassist removed.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_AUTOASSIST);
    } else {
	send_to_char("You will now assist when needed.\n\r", ch);
	SET_BIT(ch->act, PLR_AUTOASSIST);
    }
}

/* Added by Vorlin for racial says */
void do_rsay(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch)) {
	send_to_char("You can't do that in NPC's.\n\r", ch);
	return;
    }

    if (IS_SET(ch->act, PLR_SAYS)) {
	send_to_char("Racial says are now `&OFF``.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_SAYS);
    } else {
	send_to_char("Racial says are now `&ON``.\n\r", ch);
	SET_BIT(ch->act, PLR_SAYS);
    }
    return;
}

/* Added by Vorlin for description toggle */
void do_dtoggle(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch)) {
	send_to_char("You can't do that in NPC's.\n\r", ch);
	return;
    }

    if (IS_SET(ch->act, PLR_DESCR)) {
	send_to_char("Mob/player descriptions are now `&viewable``.\n\r",
		     ch);
	REMOVE_BIT(ch->act, PLR_DESCR);
    } else {
	send_to_char
	    ("Mob/player descriptions are no longer `&viewable``.\n\r",
	     ch);
	SET_BIT(ch->act, PLR_DESCR);
    }
    return;
}

void do_autoexit(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act, PLR_AUTOEXIT)) {
	send_to_char("Exits will no longer be displayed.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_AUTOEXIT);
    } else {
	send_to_char("Exits will now be displayed.\n\r", ch);
	SET_BIT(ch->act, PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act, PLR_AUTOGOLD)) {
	send_to_char("Autogold removed.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_AUTOGOLD);
    } else {
	send_to_char("Automatic gold looting set.\n\r", ch);
	SET_BIT(ch->act, PLR_AUTOGOLD);
    }
}


void do_autoloot(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act, PLR_AUTOLOOT)) {
	send_to_char("Autolooting removed.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_AUTOLOOT);
    } else {
	send_to_char("Automatic corpse looting set.\n\r", ch);
	SET_BIT(ch->act, PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act, PLR_AUTOSAC)) {
	send_to_char("Autosacrificing removed.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_AUTOSAC);
    } else {
	send_to_char("Automatic corpse sacrificing set.\n\r", ch);
	SET_BIT(ch->act, PLR_AUTOSAC);
    }
}


void do_autosplit(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->act, PLR_AUTOSPLIT)) {
	send_to_char("Autosplitting removed.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_AUTOSPLIT);
    } else {
	send_to_char("Automatic gold splitting set.\n\r", ch);
	SET_BIT(ch->act, PLR_AUTOSPLIT);
    }
}



void do_brief(CHAR_DATA * ch, char *argument)
{

    if (IS_SET(ch->comm, COMM_BRIEF)) {
	send_to_char("You now see room descriptions.\n\r", ch);
	REMOVE_BIT(ch->comm, COMM_BRIEF);
    } else {
	send_to_char("You no longer see room descriptions.\n\r", ch);
	send_to_char("Don't complain if you get `!ganked``.\n\r", ch);
	SET_BIT(ch->comm, COMM_BRIEF);
    }
    return;
}



void do_compact(CHAR_DATA * ch, char *argument)
{
    if (IS_SET(ch->comm, COMM_COMPACT)) {
	send_to_char("Compact mode removed.\n\r", ch);
	REMOVE_BIT(ch->comm, COMM_COMPACT);
    } else {
	send_to_char("Compact mode set.\n\r", ch);
	SET_BIT(ch->comm, COMM_COMPACT);
    }
}



void do_show(CHAR_DATA * ch, char *argument)
{
    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS)) {
	send_to_char("Affects will no longer be shown in score.\n\r", ch);
	REMOVE_BIT(ch->comm, COMM_SHOW_AFFECTS);
    } else {
	send_to_char("Affects will now be shown in score.\n\r", ch);
	SET_BIT(ch->comm, COMM_SHOW_AFFECTS);
    }
}



void do_prompt(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0') {
	if (IS_SET(ch->comm, COMM_PROMPT)) {
	    send_to_char("You will no longer see prompts.\n\r", ch);
	    REMOVE_BIT(ch->comm, COMM_PROMPT);
	} else {
	    send_to_char("You will now see prompts.\n\r", ch);
	    SET_BIT(ch->comm, COMM_PROMPT);
	}

	return;
    }

    if (!strcmp(argument, "all"))
	strcpy(buf, "<`^%h``hp `#%m``m `$%v``mv``>");
    else {
	if (strlen(argument) > 50)
	    argument[50] = '\0';

	strcpy(buf, argument);
	smash_tilde(buf);

	if (str_suffix("%c", buf))
	    strcat(buf, " ");
    }

    free_string(ch->prompt);
    ch->prompt = str_dup(buf);
    sprintf(buf, "Prompt set to %s\n\r", ch->prompt);
    send_to_char(buf, ch);

    return;
}



void do_autodamage(CHAR_DATA * ch, char *argument)
{
    if (IS_SET(ch->act, PLR_AUTODAMAGE)) {
	send_to_char("Damage amounts will no longer be shown.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_AUTODAMAGE);
    } else {
	send_to_char("Damage amounts will now be shown.\n\r", ch);
	SET_BIT(ch->act, PLR_AUTODAMAGE);
    }
}

void do_combine(CHAR_DATA * ch, char *argument)
{
    if (IS_SET(ch->comm, COMM_COMBINE)) {
	send_to_char("Long inventory selected.\n\r", ch);
	REMOVE_BIT(ch->comm, COMM_COMBINE);
    } else {
	send_to_char("Combined inventory selected.\n\r", ch);
	SET_BIT(ch->comm, COMM_COMBINE);
    }
}


/*
void do_noloot(CHAR_DATA* ch, char* argument) {
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_CANLOOT)) {
		send_to_char("Your corpse is now safe from thieves.\n\r", ch);
		REMOVE_BIT(ch->act, PLR_CANLOOT);
	} else {
		send_to_char("Your corpse may now be looted.\n\r", ch);
		SET_BIT(ch->act, PLR_CANLOOT);
	}
}
*/

void do_nofollow(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("And stray from your master!? Where would you go?!",
		     ch);
	return;
    }

    if (IS_SET(ch->act, PLR_NOFOLLOW)) {
	send_to_char("You now accept followers.\n\r", ch);
	REMOVE_BIT(ch->act, PLR_NOFOLLOW);
    } else {
	send_to_char("You no longer accept followers.\n\r", ch);
	SET_BIT(ch->act, PLR_NOFOLLOW);
	die_follower(ch);
    }

}



void do_nosummon(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch)) {
	if (IS_SET(ch->imm_flags, IMM_SUMMON)) {
	    send_to_char("You are no longer immune to summon.\n\r", ch);
	    REMOVE_BIT(ch->imm_flags, IMM_SUMMON);
	} else {
	    send_to_char("You are now immune to summoning.\n\r", ch);
	    SET_BIT(ch->imm_flags, IMM_SUMMON);
	}
    } else {
	if (IS_SET(ch->act, PLR_NOSUMMON)) {
	    send_to_char("You are no longer immune to summon.\n\r", ch);
	    REMOVE_BIT(ch->act, PLR_NOSUMMON);
	} else {
	    send_to_char("You are now immune to summoning.\n\r", ch);
	    SET_BIT(ch->act, PLR_NOSUMMON);
	}
    }
}



void do_look(CHAR_DATA * ch, char *argument)
{
    extern char *const dir_name[];
    extern const sh_int rev_dir[];

    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *portal;
    ROOM_INDEX_DATA *original;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    sh_int door;
    sh_int door2;
    int number, count;

    pdesc = NULL;

    if (ch->desc == NULL)
	return;
    else if (IS_AFFECTED(ch, AFF_FROZEN)) {
	send_to_char("You are a frozen statue.\n\r", ch);
	ch->position = POS_STUNNED;
	return;
    } else if (ch->position < POS_SLEEPING) {
	send_to_char("You can't see anything but stars!\n\r", ch);
	return;
    } else if (ch->position == POS_SLEEPING) {
	send_to_char("You can't see anything, you're sleeping!\n\r", ch);
	return;
    } else if (!check_blind(ch)) {
	return;
    } else if (!IS_NPC(ch)
	       && !IS_AFFECTED(ch, AFF_DARK_VISION)
	       && !IS_AFFECTED(ch, AFF_INFRARED)
	       && !IS_SET(ch->act, PLR_HOLYLIGHT)
	       && room_is_dark(ch->in_room)) {
	send_to_char("`8It is pitch black`` ... \n\r", ch);
	show_char_to_char(ch->in_room->people, ch);
	return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    number = number_argument(arg1, arg3);
    count = 0;

    if (arg1[0] == '\0' || !str_cmp(arg1, "auto")) {
	/* 'look' or 'look auto' */
	send_to_char(ch->in_room->name, ch);

	if (ch->level > LEVEL_IMMORTAL || IS_BUILDER(ch, ch->in_room->area)) {
	    sprintf(buf, " `8[`!Room`8: `1%d`8]``", ch->in_room->vnum);
	    send_to_char(buf, ch);
	}
	send_to_char("\n\r", ch);

	if (arg1[0] == '\0'
	    || (ch->comm != 0 && !IS_SET(ch->comm, COMM_BRIEF))) {
	    send_to_char("  ", ch);
	    send_to_char(ch->in_room->description, ch);
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) {
	    send_to_char("\n\r", ch);
	    do_exits(ch, "auto");
	}

	show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
	show_char_to_char(ch->in_room->people, ch);
	return;
    }

    if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in")) {
	/* 'look in' */
	if (arg2[0] == '\0') {
	    send_to_char("Look in what?\n\r", ch);
	    return;
	}

	if ((obj = get_obj_here(ch, arg2)) == NULL) {
	    send_to_char("You do not see that here.\n\r", ch);
	    return;
	}

	switch (obj->item_type) {
	default:
	    send_to_char("That is not a container.\n\r", ch);
	    break;
	case ITEM_DRINK_CON:
	    if (obj->value[1] <= 0) {
		send_to_char("It is empty.\n\r", ch);
		break;
	    }

	    sprintf(buf, "It's %s full of a %s liquid.\n\r",
		    obj->value[1] <
		    obj->value[0] / 4 ? "less than" : obj->value[1] <
		    3 * obj->value[0] / 4 ? "about" : "more than",
		    liq_table[obj->value[2]].liq_color);

	    send_to_char(buf, ch);
	    break;

	case ITEM_SADDLE:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if (IS_SET(obj->value[1], CONT_CLOSED)) {
		send_to_char("It is closed.\n\r", ch);
		break;
	    }

	    act("$p contains:", ch, obj, NULL, TO_CHAR);
	    show_list_to_char(obj->contains, ch, TRUE, TRUE);
	    break;
	case ITEM_PORTAL:
	    original = ch->in_room;
	    portal = get_room_index(obj->value[3]);
	    char_from_room(ch);
	    char_to_room(ch, portal);
	    do_look(ch, "auto");
	    char_from_room(ch);
	    char_to_room(ch, original);
	    break;
	case ITEM_PISTOL:
	case ITEM_SMG:
	case ITEM_SHOTGUN:
	case ITEM_RIFLE:
	case ITEM_HEAVYGUN:
	case ITEM_ENERGYGUN:
	case ITEM_AMMO:
	case ITEM_CLIP:
	    break;
	}
	return;
    }

    if ((victim = get_char_room(ch, arg1)) != NULL) {
	show_char_to_char_1(victim, ch);
	WAIT_STATE(ch, (number_range(1, 1) * PULSE_VIOLENCE));
	return;
    }


    for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
	if (can_see_obj(ch, obj))
	    pdesc = get_extra_descr(arg3, obj->extra_descr);

	if (pdesc != NULL) {
	    if (++count == number) {
		send_to_char(pdesc, ch);
		return;
	    } else {
		continue;
	    }
	}

	pdesc = get_extra_descr(arg3, obj->pIndexData->extra_descr);

	if (pdesc != NULL) {
	    if (++count == number) {
		send_to_char(pdesc, ch);
		return;
	    } else {
		continue;
	    }
	}

	if (is_name(arg3, obj->name))
	    if (++count == number) {
		send_to_char(obj->description, ch);
		send_to_char("\n\r", ch);
		return;
	    }
    }

    for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content) {
	if (can_see_obj(ch, obj))
	    pdesc = get_extra_descr(arg3, obj->extra_descr);

	if (pdesc != NULL)
	    if (++count == number) {
		send_to_char(pdesc, ch);
		return;
	    }

	if (is_name(arg3, obj->name))
	    if (++count == number) {
		send_to_char(obj->description, ch);
		send_to_char("\n\r", ch);
		return;
	    }
    }

    if (count > 0 && count != number) {
	if (count == 1)
	    sprintf(buf, "You only see one %s here.\n\r", arg3);
	else
	    sprintf(buf, "You only see %d %s's here.\n\r", count, arg3);

	send_to_char(buf, ch);
	return;
    }

    pdesc = get_extra_descr(arg1, ch->in_room->extra_descr);

    if (pdesc != NULL) {
	send_to_char(pdesc, ch);
	return;
    }

    if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
	door = 0;
    else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
	door = 1;
    else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
	door = 2;
    else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
	door = 3;
    else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
	door = 4;
    else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
	door = 5;
    else {
	send_to_char("You do not see that here.\n\r", ch);
	return;
    }

    /* 'look direction' */
    if ((pexit = ch->in_room->exit[door]) == NULL) {
	send_to_char("Nothing special there.\n\r", ch);
	return;
    }

    if (pexit->keyword != NULL
	&& pexit->keyword[0] != '\0' && pexit->keyword[0] != ' ') {
	if (IS_SET(pexit->exit_info, EX_CLOSED)) {
	    act("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
	} else {
	    if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
		act("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
		/* DC960611 * Added looking in directions peeking in rooms */
		original = ch->in_room;
		char_from_room(ch);
		to_room = pexit->u1.to_room;
		char_to_room(ch, to_room);
		door2 = rev_dir[door];

		if (!IS_AFFECTED(ch, AFF_SNEAK) && !IS_IMMORTAL(ch)) {
		    sprintf(buf, "$n is looking in from the %s.\n\r",
			    dir_name[door2]);
		    act(buf, ch, NULL, NULL, TO_ROOM);
		}

		do_look(ch, "auto");
		char_from_room(ch);
		char_to_room(ch, original);
	    }
	}
    } else {
	/* DC960611 * Added looking in directions peeking in rooms */
	original = ch->in_room;
	char_from_room(ch);
	to_room = pexit->u1.to_room;
	char_to_room(ch, to_room);
	door2 = rev_dir[door];

	if (!IS_AFFECTED(ch, AFF_SNEAK) && !IS_IMMORTAL(ch)) {
	    sprintf(buf, "$n is looking in from the %s.\n\r",
		    dir_name[door2]);
	    act(buf, ch, NULL, NULL, TO_ROOM);
	}

	do_look(ch, "auto");
	char_from_room(ch);
	char_to_room(ch, original);
    }
    return;
}

/*EE960801*/
void do_look_directional(CHAR_DATA * ch, char *argument)
{
    int normal_look;
    char buf[15];
    char *exdesc;

    if (!ch->desc)
	return;

    exdesc = NULL;

    if (IS_AFFECTED(ch, AFF_FROZEN)) {
	send_to_char("You are a frozen statue.\n\r", ch);
	ch->position = POS_STUNNED;
	return;
    } else if (ch->position < POS_SLEEPING) {
	send_to_char("You can't see anything but stars!\n\r", ch);
	return;
    } else if (ch->position == POS_SLEEPING) {
	send_to_char("You can't see anything, you're sleeping!\n\r", ch);
	return;
    } else if (!check_blind(ch))
	return;

    if (!IS_NPC(ch)
	&& !IS_AFFECTED(ch, AFF_DARK_VISION)
	&& !IS_AFFECTED(ch, AFF_INFRARED)
	&& !IS_SET(ch->act, PLR_HOLYLIGHT) && room_is_dark(ch->in_room)) {
	send_to_char("It is pitch black...\n\r", ch);
	show_char_to_char(ch->in_room->people, ch);
	return;
    }

    send_to_char(ch->in_room->name, ch);

    if (ch->level > LEVEL_IMMORTAL) {
	sprintf(buf, " `8[`!Room`8: `1%d`8]``", ch->in_room->vnum);
	send_to_char(buf, ch);
    }
    send_to_char("\n\r", ch);

    if (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)) {
	normal_look = 0;
	buf[0] = '\0';
	sprintf(buf, argument);
	strcat(buf, "_ENTER_WOND");
	exdesc = get_extra_descr(buf, ch->in_room->extra_descr);

	if (exdesc != NULL) {
	    send_to_char(exdesc, ch);
	} else {
	    buf[0] = '\0';
	    sprintf(buf, argument);
	    strcat(buf, "_ENTER_WND");
	    exdesc = get_extra_descr(buf, ch->in_room->extra_descr);
	    normal_look = 1;

	    if (exdesc != NULL) {
		send_to_char(exdesc, ch);
	    }

	    if (normal_look) {
		if (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)) {
		    send_to_char("  ", ch);
		    send_to_char(ch->in_room->description, ch);
		}
	    }
	}
    }

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) {
	send_to_char("\n\r", ch);
	do_exits(ch, "auto");
    }

    show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
    show_char_to_char(ch->in_room->people, ch);
    return;
}




/* RT added back for the hell of it */
void do_read(CHAR_DATA * ch, char *argument)
{
    do_look(ch, argument);
}



void do_examine(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    AFFECT_DATA *paf;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Examine what?\n\r", ch);
	return;
    }

    do_look(ch, arg);

    if ((obj = get_obj_here(ch, arg)) != NULL) {
	if (can_see_obj(ch, obj)
	    && get_skill(ch, gsn_lore) != 0 && (!IS_NPC(ch)
						&& ch->level >=
						skill_table
						[gsn_lore].
						skill_level[ch->class])
	    && get_obj_carry(ch, arg)) {
	    if (number_percent() < get_skill(ch, gsn_lore)) {
		check_improve(ch, gsn_lore, TRUE, 4);
		send_to_char
		    ("\n\rYou integrate your Lore capability into your examination and uncover\n\rthe following information:\n\r",
		     ch);
		switch (obj->item_type) {
		case ITEM_LIGHT:
		    sprintf(buf,
			    "\tThis light source is worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    sprintf(buf,
			    "\tIt looks like it'll stay lit for about %ld hours.\n\r",
			    obj->value[2]);
		    send_to_char(buf, ch);
		    break;
		case ITEM_SCROLL:
		    sprintf(buf, "\tThis scroll is worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_WAND:
		    sprintf(buf, "\tThis wand is worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_STAFF:
		    sprintf(buf, "\tThis staff is worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_WEAPON:
		    send_to_char("\tThis weapon is ", ch);
		    switch (obj->value[0]) {
		    case (WEAPON_EXOTIC):
			send_to_char("an exotic weapon ", ch);
			break;
		    case (WEAPON_SWORD):
			send_to_char("a sword ", ch);
			break;
		    case (WEAPON_DAGGER):
			send_to_char("a dagger ", ch);
			break;
		    case (WEAPON_SPEAR):
			send_to_char("a spear/staff ", ch);
			break;
		    case (WEAPON_MACE):
			send_to_char("a mace/club ", ch);
			break;
		    case (WEAPON_AXE):
			send_to_char("an axe ", ch);
			break;
		    case (WEAPON_FLAIL):
			send_to_char("a flail ", ch);
			break;
		    case (WEAPON_WHIP):
			send_to_char("a whip ", ch);
			break;
		    case (WEAPON_POLEARM):
			send_to_char("a polearm ", ch);
			break;
		    default:
			send_to_char("an unknown weapon ", ch);
			break;
		    }
		    sprintf(buf, "worth %d silver.\n\r", obj->cost);
		    send_to_char(buf, ch);

		    if (obj->pIndexData->new_format)
			sprintf(buf,
				"\tIt looks like it might average around %ld damage a hit.\n\r",
				(1 + obj->value[2]) * obj->value[1] / 2);
		    else
			sprintf(buf,
				"\tIt looks like it might average around %ld damage a hit.\n\r",
				(obj->value[1] + obj->value[2]) / 2);
		    send_to_char(buf, ch);

		    if (obj->value[4]) {	/* weapon flags */
			sprintf(buf,
				"\tIt sizzles with energy from an unreleased power.\n\r");
			send_to_char(buf, ch);
		    }
		    break;
		case ITEM_TREASURE:
		    sprintf(buf,
			    "\tThis is a treasured item worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_ARMOR:
		    sprintf(buf,
			    "\tThis is a piece of armor worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    if (ch->level >= 25) {
			if (obj->value[0] != 0) {
			    sprintf(buf,
				    "\tIt affects pierce ac by %ld\n\r",
				    obj->value[0]);
			    send_to_char(buf, ch);
			}
			if (obj->value[1] != 0) {
			    sprintf(buf, "\tIt affects bash ac by %ld\n\r",
				    obj->value[1]);
			    send_to_char(buf, ch);
			}
			if (obj->value[2] != 0) {
			    sprintf(buf,
				    "\tIt affects slash ac by %ld\n\r",
				    obj->value[2]);
			    send_to_char(buf, ch);
			}
			if (obj->value[3] != 0) {
			    sprintf(buf,
				    "\tIt affects magic ac by %ld\n\r",
				    obj->value[3]);
			    send_to_char(buf, ch);
			}
		    }
		    break;
		case ITEM_POTION:
		    sprintf(buf, "\tThis is a potion worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_CLOTHING:
		    sprintf(buf, "\tThis is clothing worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_FURNITURE:
		    send_to_char
			("This is furniture...  I have no clue how it's in your inventory.\n\r",
			 ch);
		    break;
		case ITEM_TRASH:
		    sprintf(buf,
			    "\tThis is a peace of trash worth %d silver.\n\r",
			    obj->cost);
		case ITEM_SADDLE:
		case ITEM_CONTAINER:
		    sprintf(buf,
			    "\tThis is a storage container worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    sprintf(buf, "\tIt can hold around %ld pounds.\n\r",
			    obj->value[0]);
		    send_to_char(buf, ch);
		    sprintf(buf,
			    "\tYou can fit up to %ld pound objects within it.\n\r",
			    obj->value[3]);
		    send_to_char(buf, ch);
		    if (obj->value[4] != 0) {
			sprintf(buf,
				"\tIt reduces the weight of an object by %ld percent.\n\r",
				obj->value[4]);
			send_to_char(buf, ch);
		    }
		    break;
		case ITEM_DRINK_CON:
		    sprintf(buf,
			    "\tThis is a drink container worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    sprintf(buf,
			    "\tIt currently has %ld drinks of %s in it.\n\r",
			    obj->value[1],
			    liq_table[obj->value[2]].liq_name);
		    send_to_char(buf, ch);
		    if (obj->value[3] > 0)
			send_to_char("\tIt's poisonous!\n\r", ch);
		    break;
		case ITEM_ROOM_KEY:
		case ITEM_KEY:
		    sprintf(buf,
			    "\tThis is a key to a room somewhere worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_FOOD:
		    sprintf(buf,
			    "\tThis is some food worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    sprintf(buf,
			    "\tIt'll keep you from hunger for %ld hours.\n\r",
			    obj->value[0]);
		    send_to_char(buf, ch);
		    if (obj->value[3] > 0)
			send_to_char("\tIt's poisonous!\n\r", ch);
		    break;
		case ITEM_MONEY:
		    break;
		case ITEM_BOAT:
		    sprintf(buf, "\tThis is a boat worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_CORPSE_NPC:
		    sprintf(buf,
			    "\tThis is a corpse to a level %d mob.\n\r",
			    obj->level);
		    send_to_char(buf, ch);
		    break;
		case ITEM_CORPSE_PC:
		    sprintf(buf,
			    "\tThis is a corpse to a level %d player.\n\r",
			    obj->level);
		    send_to_char(buf, ch);
		    break;
		case ITEM_FOUNTAIN:
		    send_to_char
			("This is a fountain... And I'm not going to ask how it got in your inventory.\n\r",
			 ch);
		    break;
		case ITEM_PILL:
		    sprintf(buf, "\tThis is a pill worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_PROTECT:
		    break;
		case ITEM_MAP:
		    sprintf(buf, "\tThis is a map worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_PORTAL:
		    send_to_char
			("This is a portal... And I'm not going to ask how it got in your inventory.\n\r",
			 ch);
		    break;
		case ITEM_WARP_STONE:
		case ITEM_GEM:
		    sprintf(buf,
			    "\tThis is an exotic gem worth %d silver!\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_JEWELRY:
		    sprintf(buf,
			    "\tThis is a beuatiful peace of jewelry worth %d silver.\n\r",
			    obj->cost);
		    send_to_char(buf, ch);
		    break;
		case ITEM_JUKEBOX:
		    send_to_char
			("This is a Jukebox... And I'm not going to ask how it got in your inventory.\n\r",
			 ch);
		    break;
		}
		sprintf(buf, "\tIt weighs in at %d pounds.\n\r",
			obj->weight / 10);
		send_to_char(buf, ch);

		if (!obj->enchanted)
		    for (paf = obj->pIndexData->affected; paf != NULL;
			 paf = paf->next) {
			if (paf->location != APPLY_NONE
			    && paf->modifier != 0) {
			    sprintf(buf, "\tIt affects %s ",
				    affect_loc_name(paf->location));
			    send_to_char(buf, ch);

			    if (paf->modifier > 0)
				send_to_char("positively.\n\r", ch);
			    if (paf->modifier < 0)
				send_to_char("negatively.\n\r", ch);
			    if (paf->modifier == 0)
				send_to_char("by none.\n\r", ch);

			    if (paf->bitvector) {
				switch (paf->where) {
				case TO_AFFECTS:
				    sprintf(buf, "\tIt adds %s affect.\n",
					    affect_bit_name
					    (paf->bitvector));
				    break;
				case TO_OBJECT:
				    sprintf(buf,
					    "\tIt adds %s object flag.\n",
					    extra_bit_name
					    (paf->bitvector));
				    break;
				case TO_IMMUNE:
				    sprintf(buf,
					    "\tIt adds immunity to %s.\n",
					    imm_bit_name(paf->bitvector));
				    break;
				case TO_RESIST:
				    sprintf(buf,
					    "\tIt adds resistance to %s.\n\r",
					    imm_bit_name(paf->bitvector));
				    break;
				case TO_VULN:
				    sprintf(buf,
					    "\tIt adds vulnerability to %s.\n\r",
					    imm_bit_name(paf->bitvector));
				    break;
				default:
				    sprintf(buf,
					    "\tIt adds an unknown bit %d: %d\n\r",
					    paf->where, paf->bitvector);
				    break;
				}
				send_to_char(buf, ch);
			    }
			}
		    }

		for (paf = obj->affected; paf != NULL; paf = paf->next) {
		    if (paf->location != APPLY_NONE && paf->modifier != 0) {
			sprintf(buf, "\tIt affects %s ",
				affect_loc_name(paf->location));
			send_to_char(buf, ch);

			if (paf->modifier > 0)
			    send_to_char("positively", ch);
			if (paf->modifier < 0)
			    send_to_char("negatively", ch);
			if (paf->modifier == 0)
			    send_to_char("by none", ch);
			if (paf->duration > -1) {
			    sprintf(buf, ", %d hours.\n\r", paf->duration);
			    send_to_char(buf, ch);
			} else
			    send_to_char(".\n\r", ch);

			if (paf->bitvector) {
			    switch (paf->where) {
			    case TO_AFFECTS:
				sprintf(buf, "\tIt adds %s affect.\n",
					affect_bit_name(paf->bitvector));
				break;
			    case TO_OBJECT:
				sprintf(buf, "\tIt adds %s object flag.\n",
					extra_bit_name(paf->bitvector));
				break;
			    case TO_WEAPON:
				sprintf(buf,
					"\tIt adds %s weapon flags.\n",
					weapon_bit_name(paf->bitvector));
				break;
			    case TO_IMMUNE:
				sprintf(buf, "\tIt adds immunity to %s.\n",
					imm_bit_name(paf->bitvector));
				break;
			    case TO_RESIST:
				sprintf(buf,
					"\tIt adds resistance to %s.\n\r",
					imm_bit_name(paf->bitvector));
				break;
			    case TO_VULN:
				sprintf(buf,
					"\tIt adds vulnerability to %s.\n\r",
					imm_bit_name(paf->bitvector));
				break;
			    default:
				sprintf(buf,
					"\tIt adds an unknown bit %d: %d\n\r",
					paf->where, paf->bitvector);
				break;
			    }
			    send_to_char(buf, ch);
			}
		    }
		}
	    }
	} else {
	    switch (obj->item_type) {
	    default:
		break;
	    case ITEM_JUKEBOX:
		do_play(ch, "list");
		break;
	    case ITEM_MONEY:
		if (obj->value[0] == 0) {
		    if (obj->value[1] == 0)
			sprintf(buf,
				"Odd...there's no coins in the pile.\n\r");
		    else if (obj->value[1] == 1)
			sprintf(buf, "Wow. One gold coin.\n\r");
		    else
			sprintf(buf,
				"There are %ld gold coins in the pile.\n\r",
				obj->value[1]);
		} else if (obj->value[1] == 0) {
		    if (obj->value[0] == 1)
			sprintf(buf, "Wow. One silver coin.\n\r");
		    else
			sprintf(buf,
				"There are %ld silver coins in the pile.\n\r",
				obj->value[0]);
		} else
		    sprintf(buf,
			    "There are %ld gold and %ld silver coins in the pile.\n\r",
			    obj->value[1], obj->value[0]);
		send_to_char(buf, ch);
		break;
	    case ITEM_SADDLE:
	    case ITEM_DRINK_CON:
	    case ITEM_CONTAINER:
	    case ITEM_CORPSE_NPC:
	    case ITEM_CORPSE_PC:
		sprintf(buf, "in %s", argument);
		do_look(ch, buf);
		break;
	    case ITEM_PISTOL:
	    case ITEM_SMG:
	    case ITEM_SHOTGUN:
	    case ITEM_RIFLE:
	    case ITEM_HEAVYGUN:
	    case ITEM_ENERGYGUN:
		sprintf(buf, "This firearm uses %s rounds.\n\r",
		    obj->value[0] < BULLET_CALIBER ?
		    ammo_table[obj->value[0]].caliber : "(null)");
		send_to_char(buf, ch);
		if (obj->value[2] <= 0) {
		    sprintf(buf, "It is currently unloaded.\n\r");
		    send_to_char(buf, ch);
		} else {
		    sprintf(buf, "It is loaded with %ld rounds of type %s.\n\r",
			obj->value[2],
			(obj->value[0] < BULLET_CALIBER && obj->value[1] < BULLET_TYPE ?
			ammo_table[obj->value[0]].ammotype[obj->value[1]] : "(null)"));
		    send_to_char(buf, ch);
		}
		sprintf(buf, "It can shoot up to %d room%s away.\n\r",
		    gun_range(obj), gun_range(obj) == 1 ? "" : "s");
		send_to_char(buf, ch);
		if (obj->condition & BURST) {
		    send_to_char("It is set to fire in burst mode.\n\r", ch);
		} else if (obj->condition & FULLAUTO) {
		    send_to_char("It is set to fire in full-auto mode.\n\r", ch);
		} else {
		    send_to_char("It is set to fire in semi-auto mode.\n\r", ch);
		}
		break;
	    case ITEM_AMMO:
		if (obj->value[2] <= 0) {
		    sprintf(buf, "This is a spent %s round.\n\r",
			obj->value[0] < BULLET_CALIBER ?
			ammo_table[obj->value[0]].caliber : "(null)");
		    send_to_char(buf, ch);
		} else if (obj->value[2] == 1) {
		    sprintf(buf, "This is a %s round, type %s.\n\r",
			obj->value[0] < BULLET_CALIBER ?
			ammo_table[obj->value[0]].caliber : "(null)",
			(obj->value[0] < BULLET_CALIBER && obj->value[1] < BULLET_TYPE ?
			ammo_table[obj->value[0]].ammotype[obj->value[1]] : "(null)"));
		    send_to_char(buf, ch);
		} else {
		    sprintf(buf, "There are %ld %s rounds, type %s.\n\r",
			obj->value[2],
			obj->value[0] < BULLET_CALIBER ?
			ammo_table[obj->value[0]].caliber : "(null)",
			(obj->value[0] < BULLET_CALIBER && obj->value[1] < BULLET_TYPE ?
			ammo_table[obj->value[0]].ammotype[obj->value[1]] : "(null)"));
		    send_to_char(buf, ch);
		}
		break;
	    case ITEM_CLIP:
		sprintf(buf, "This magazine is made for %s rounds.\n\r",
		    obj->value[0] < BULLET_CALIBER ?
		    ammo_table[obj->value[0]].caliber : "(null)");
		send_to_char(buf, ch);
		if (obj->value[2] <= 0) {
		    sprintf(buf, "It is empty, and can hold up to %ld rounds.\n\r",
			obj->value[3]);
		    send_to_char(buf, ch);
		} else {
		    sprintf(buf, "It holds %ld/%ld rounds of type %s.\n\r",
			obj->value[2], obj->value[3], 
			(obj->value[0] < BULLET_CALIBER && obj->value[1] < BULLET_TYPE ?
			ammo_table[obj->value[0]].ammotype[obj->value[1]] : "(null)"));
		    send_to_char(buf, ch);
		}
		break;
	    }
	}
    }
    return;
}



/*
* Thanks to Zrin for auto-exit part.
*/
void do_exits(CHAR_DATA * ch, char *argument)
{
    extern char *const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found = FALSE;
    bool fAuto = !str_cmp(argument, "auto");
    int door;

    if (!check_blind(ch))
	return;

    if (fAuto)
	sprintf(buf, "`8[`^Exits`8:`^");
    else if (IS_IMMORTAL(ch))
	sprintf(buf, "Obvious exits from room %d:\n\r", ch->in_room->vnum);
    else
	sprintf(buf, "Obvious exits:\n\r");

    for (door = 0; door <= 5; door++) {
	if ((pexit = ch->in_room->exit[door]) != NULL
	    && pexit->u1.to_room != NULL
	    && can_see_room(ch, pexit->u1.to_room)
	    && !IS_SET(pexit->exit_info, EX_CLOSED)) {
	    found = TRUE;

	    if (fAuto) {
		strcat(buf, " ");
		if (IS_SET(pexit->u1.to_room->room_flags, ROOM_PRIVATE))
		    strcat(buf, "`^(`6");
		if (IS_SET(pexit->u1.to_room->room_flags, ROOM_SOLITARY))
		    strcat(buf, "`^<`6");
		if (IS_SET(pexit->u1.to_room->room_flags, ROOM_IMP_ONLY)
		    || IS_SET(pexit->u1.to_room->room_flags,
			      ROOM_WIZARD_ONLY)) strcat(buf, "`^:`6");

		strcat(buf, dir_name[door]);

		if (IS_SET(pexit->u1.to_room->room_flags, ROOM_IMP_ONLY)
		    || IS_SET(pexit->u1.to_room->room_flags,
			      ROOM_WIZARD_ONLY)) strcat(buf, "`^:`^");
		if (IS_SET(pexit->u1.to_room->room_flags, ROOM_SOLITARY))
		    strcat(buf, "`^>`^");
		if (IS_SET(pexit->u1.to_room->room_flags, ROOM_PRIVATE))
		    strcat(buf, "`^)`^");
	    } else {
		sprintf(buf + strlen(buf), "%-5s - %s",
			capitalize(dir_name[door]),
			room_is_dark(pexit->u1.
				     to_room) ? "Too dark to tell" :
			pexit->u1.to_room->name);

		if (IS_IMMORTAL(ch))
		    sprintf(buf + strlen(buf), " (room %d)\n\r",
			    pexit->u1.to_room->vnum);
		else
		    sprintf(buf + strlen(buf), "\n\r");
	    }
	}
    }

    if (!found)
	strcat(buf, fAuto ? " none" : "None.\n\r");

    if (fAuto)
	strcat(buf, "`8]``\n\r");

    send_to_char(buf, ch);
    return;
}

void do_worth(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
	sprintf(buf, "You have %ld gold and %ld silver.\n\r", ch->gold,
		ch->silver);
	send_to_char(buf, ch);
	return;
    }

    sprintf(buf,
	    "You have `&%ld `3g`#o``l`&d``, `&%ld `8s``i`&lver``, and `&%d `$experience`` `8(`^%d `6exp to level`8)``\n\r",
	    ch->gold, ch->silver, ch->exp,
	    (ch->level + 1) * exp_per_level(ch,
					    ch->pcdata->points) - ch->exp);
    send_to_char(buf, ch);

    return;
}


void do_score(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i;

    sprintf(buf,
	    "\n\r`^(`4-``You are %s%s, a `^%d`` year old `^%s``, `^%s``, `^%s``.\n\r",
	    ch->name, IS_NPC(ch) ? "" : ch->pcdata->title, get_age(ch),
	    ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
	    race_table[ch->race].name,
	    IS_NPC(ch) ? "mobile" : class_table[ch->class].name);
    send_to_char(buf, ch);

    sprintf(buf,
	    "`^(`4-``You have lasted `^%d`` hours and gained `^%d`` experience points.\n\r",
	    (ch->played + (int) (current_time - ch->logon)) / 3600,
	    ch->exp);
    send_to_char(buf, ch);

    /* EE961110, *sigh* removed yet one thing for the whining imms... spent a 
     * couple of hours of unnecessary coding just because they can't understand
     * that immlevel does NOT mean status... if they ever complain again... 
     * *grumble* Heh speaking of unnecessary... I think this was one of the most 
     * unnecessary comments I've ever written btw... :) 
     */
    if (get_trust(ch) != ch->level) {
	sprintf(buf, "`^(`4-``You are trusted at level `^%d``.\n\r",
		get_trust(ch));
	send_to_char(buf, ch);
    }

    sprintf(buf, "`^(`4-``Your current stats are as follows:\n\r");
    send_to_char(buf, ch);

    sprintf(buf,
	    "                    `2_``-=`%%*`` Level: `^%d`` `%%*``=-`2_`` \n\r",
	    ch->level);
    send_to_char(buf, ch);
    sprintf(buf,
	    "-`#Str``: `$%d`*[`^%d`*] `#Int``: `$%d`*[`^%d`*] `#Wis``: `$%d`*[`^%d`*] `#Dex``: `$%d`*[`^%d`*] `#Con``: `$%d`*[`^%d`*]``\n\r",
	    ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR),
	    ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT),
	    ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS),
	    ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX),
	    ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON));
    send_to_char(buf, ch);

    sprintf(buf,
	    "-`!Hp``: `1(``%d`*/`^%d`1)  `!Mana``: `1(``%d`*/`^%d`1)  `!Movement``: `1(``%d`*/`^%d`1)``\n\r",
	    ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
	    ch->max_move);
    send_to_char(buf, ch);

/*
    sprintf(buf,
	    "\n\r-Class hp range:   `8[`^%2d``-`^%2d`8]``   Con modifier: `8[`^%d`8]``",
	    class_table[ch->class].hp[MIN],
	    class_table[ch->class].hp[MAX],
	    con_app[get_curr_stat(ch, STAT_CON)].hitp);
    send_to_char(buf, ch);

    sprintf(buf,
	    "\n\r-Class mana range: `8[`#%2d``-`#%2d`8]``   Wis modifier: `8[`#%d`8]``",
	    class_table[ch->class].mana[MIN],
	    class_table[ch->class].mana[MAX],
	    wis_app[get_curr_stat(ch, STAT_WIS)].mana);
    send_to_char(buf, ch);

    sprintf(buf,
	    "\n\r-Class move range: `8[`$%2d``-`$%2d`8]``   Dex modifier: `8[`$%d`8]``\n\r",
	    class_table[ch->class].move[MIN],
	    class_table[ch->class].move[MAX],
	    dex_app[get_curr_stat(ch, STAT_DEX)].move);
    send_to_char(buf, ch);
*/

    sprintf(buf,
	    "\n\r-You are carrying `6%d`*/`^%d`` items weighing `6%ld`*/`^%d`` pounds.\n\r",
	    ch->carry_number, can_carry_n(ch), get_carry_weight(ch) / 10,
	    can_carry_w(ch) / 10);
    send_to_char(buf, ch);

    sprintf(buf,
	    "-You have `^%ld `3g`#o``l`&d`` and `^%ld `As``i`&lver``.\n\r",
	    ch->gold, ch->silver);
    send_to_char(buf, ch);

    sprintf(buf, "-You have `6%ld `3g`#o``l`&d`` in the `2b`@a`2nk``.\n\r",
	    ch->bank);
    send_to_char(buf, ch);

    sprintf(buf,
	    "-You can train `^%d`` times and practice `^%d`` times.\n\r",
	    ch->train, ch->practice);
    send_to_char(buf, ch);

    if(ch->jailtime){
      sprintf(buf,"`1-You have %d minutes remaining of your jail term.\n\r``",ch->jailtime);
      send_to_char(buf,ch);
    }	

    if (!IS_NPC(ch) && ch->pcdata->pkset) {	/*EE960410 */
	send_to_char("-You are allowed to kill other players!\n\r", ch);
	/*EE960530 */
	sprintf(buf,
		"-You have player killed `!%d`` persons and died `!%d`` times.\n\r",
		ch->pcdata->pkills, ch->pcdata->pdeaths);
	send_to_char(buf, ch);
    }

    sprintf(buf,
	    "-You have `V%d`` quest points and have `V%d`` minutes till you may quest again.\n\r",
	    ch->questpoints, ch->nextquest);
    send_to_char(buf, ch);

    sprintf(buf, "-You have `%%%d `@R`#P`` po`&i``nts.\n\r",
	    ch->rp_points);
    send_to_char(buf, ch);

    switch (ch->position) {
    case POS_DEAD:
	send_to_char("-You are DEAD!!\n\r", ch);
	break;
    case POS_MORTAL:
	send_to_char("-You are mortally wounded.\n\r", ch);
	break;
    case POS_INCAP:
	send_to_char("-You are incapacitated.\n\r", ch);
	break;
    case POS_STUNNED:
	send_to_char("-You are stunned.\n\r", ch);
	break;
    case POS_SLEEPING:
	send_to_char("-You are sleeping.\n\r", ch);
	break;
    case POS_RESTING:
	send_to_char("-You are resting.\n\r", ch);
	break;
    case POS_STANDING:
	send_to_char("-You are standing.\n\r", ch);
	break;
    case POS_FIGHTING:
	send_to_char("-You are fighting.\n\r", ch);
	break;
    }

    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_DEAD))
	send_to_char("-You are flagged (`8DEAD``).\n\r", ch);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	send_to_char("-You are drunk.\n\r", ch);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0)
	send_to_char("-You are thirsty.\n\r", ch);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] == 0)
	send_to_char("-You are hungry.\n\r", ch);
    if (!IS_NPC(ch) && is_clan(ch)) {
	/* Changed for new clans - Suzuran
	   sprintf(buf, "-You are a member of %s\n\r",clan_table[ch->clan].who_name); */
	sprintf(buf, "-You are a member of %s\n\r",
		get_clan_longname(ch->clan));
	send_to_char(buf, ch);
    }

    /* RT wizinvis and holy light */
    if (IS_IMMORTAL(ch)) {
	send_to_char("\n\r-`^Imm``  [`3H`#o``l`&y `&L``i`&ght``: ", ch);
	if (IS_SET(ch->act, PLR_HOLYLIGHT))
	    send_to_char("(`#On``)", ch);
	else
	    send_to_char("(`3Off``)", ch);

	if (ch->invis_level) {
	    sprintf(buf, "  `&W``i`&z``i`&nv``i`&s``: (`#%d``)",
		    ch->invis_level);
	    send_to_char(buf, ch);
	}

	if (ch->incog_level) {
	    sprintf(buf, "  `%%Incogn`5i`%%to``: (`#%d``)]",
		    ch->incog_level);
	    send_to_char(buf, ch);
	}
    }

    send_to_char("\n\r", ch);
    sprintf(buf, "-(`@W`#i`@mpy`` set at [`&%d``])", ch->wimpy);
    send_to_char(buf, ch);

    /*RT shows exp to level */
    if (!IS_NPC(ch) && ch->level < LEVEL_HERO) {
	sprintf(buf, "                  II `!ToLevel `1(``%d`1)`` II",
		((ch->level + 1) * exp_per_level(ch, ch->pcdata->points) -
		 ch->exp));
	send_to_char(buf, ch);
    }
    send_to_char("\n\r", ch);

    sprintf(buf, "-`^(`6Saving Throw`^: `6%d`^)``\n\r", ch->saving_throw);
    send_to_char(buf, ch);

    if (ch->level >= 10) {
	sprintf(buf, "-(`^Al`6i`^gnment``: `3%d``) ", ch->alignment);
	send_to_char(buf, ch);
    } else if (ch->level < 10) {
	send_to_char("-`$You are ", ch);
    } else {
	send_to_char("`$You are ", ch);
    }

    if (ch->alignment > 900)
	send_to_char("`^angelic``.\n\r", ch);
    else if (ch->alignment > 700)
	send_to_char("`^saintly``.\n\r", ch);
    else if (ch->alignment > 350)
	send_to_char("`$good``.\n\r", ch);
    else if (ch->alignment > 100)
	send_to_char("`$kind``.\n\r", ch);
    else if (ch->alignment > -100)
	send_to_char("``neutral``.\n\r", ch);
    else if (ch->alignment > -350)
	send_to_char("`!mean``.\n\r", ch);
    else if (ch->alignment > -700)
	send_to_char("`1ev`!i`1l``.\n\r", ch);
    else if (ch->alignment > -900)
	send_to_char("`Ademon``i`Ac``.\n\r", ch);
    else
	send_to_char("`1satan`!i`1c``.\n\r", ch);

    if (ch->level >= 15) {
	sprintf(buf,
		"\n\r             II `!HIT`1roll``: `1(`6%d`1)  `!DAM`1roll``: `1(`6%d`1)`` II\n\r",
		GET_HITROLL(ch), GET_DAMROLL(ch));
	send_to_char(buf, ch);
    } else {
	sprintf(buf, "\n\r");
	send_to_char(buf, ch);
    }

    /* Print AC values */
    if (ch->level >= 25) {
	sprintf(buf,
		"-AC  `![`5Pierce``: %d`!] [`5Bash``: %d`!] [`5Slash``: %d`!] [`5Magic``: %d`!]``\n\r",
		GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH), GET_AC(ch,
								   AC_SLASH),
		GET_AC(ch, AC_EXOTIC));
	send_to_char(buf, ch);
    }

    for (i = 0; i < 4; i++) {
	char *temp;

	switch (i) {
	case (AC_PIERCE):
	    temp = "piercing";
	    break;
	case (AC_BASH):
	    temp = "bashing";
	    break;
	case (AC_SLASH):
	    temp = "slashing";
	    break;
	case (AC_EXOTIC):
	    temp = "magic";
	    break;
	default:
	    temp = "error";
	    break;
	}
	send_to_char("\tYou are ", ch);

	if (GET_AC(ch, i) >= 101)
	    sprintf(buf, "hopelessly vulnerable to %s.\n\r", temp);
	else if (GET_AC(ch, i) >= 80)
	    sprintf(buf, "defenseless against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= 60)
	    sprintf(buf, "barely protected from %s.\n\r", temp);
	else if (GET_AC(ch, i) >= 40)
	    sprintf(buf, "slightly armored against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= 20)
	    sprintf(buf, "somewhat armored against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= 0)
	    sprintf(buf, "armored against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= -20)
	    sprintf(buf, "well-armored against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= -40)
	    sprintf(buf, "very well-armored against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= -60)
	    sprintf(buf, "heavily armored against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= -80)
	    sprintf(buf, "superbly armored against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= -100)
	    sprintf(buf, "almost invulnerable to %s.\n\r", temp);
	else if (GET_AC(ch, i) >= -200)
	    sprintf(buf, "divinely armored against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= -300)
	    sprintf(buf, "near invincible against %s.\n\r", temp);
	else if (GET_AC(ch, i) >= -500)
	    sprintf(buf, "practically immune to %s.\n\r", temp);
	else
	    sprintf(buf, "armored like a GOD against %s.\n\r", temp);
	send_to_char(buf, ch);
    }

    if (!IS_IMMORTAL(ch)) {
	WAIT_STATE(ch, (1.5 * PULSE_VIOLENCE));
    }

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
	do_affects(ch, "");

}

void do_affects(CHAR_DATA * ch, char *argument)
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MAX_STRING_LENGTH];

    if (ch->affected != NULL) {
	send_to_char("You are affected by the following spells:\n\r", ch);

	for (paf = ch->affected; paf != NULL; paf = paf->next) {
	    if (paf_last != NULL && paf->type == paf_last->type) {
		if (ch->level >= 20)
		    sprintf(buf, "                      ");
		else
		    continue;
	    } else
		sprintf(buf, "Spell: %-15s",
			skill_table[paf->type].name);

	    send_to_char(buf, ch);

	    if (ch->level >= 20) {
		sprintf(buf, ": modifies %s by %d ",
			affect_loc_name(paf->location), paf->modifier);
		send_to_char(buf, ch);

		if (paf->duration == -1)
		    sprintf(buf, "permanently");
		else
		    sprintf(buf, "for %d hour%s",
			    paf->duration, paf->duration == 1 ? "" : "s");
		send_to_char(buf, ch);
	    }

	    send_to_char("\n\r", ch);
	    paf_last = paf;
	}
    } else
	send_to_char("You are not affected by any spells.\n\r", ch);

    return;
}



char *const day_name[] = {
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};



char *const month_name[] = {
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};



void do_time(CHAR_DATA * ch, char *argument)
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day = time_info.day + 1;

    if (day > 4 && day < 20)
	suf = "th";
    else if (day % 10 == 1)
	suf = "st";
    else if (day % 10 == 2)
	suf = "nd";
    else if (day % 10 == 3)
	suf = "rd";
    else
	suf = "th";

    sprintf(buf,
	    "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
	    (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	    time_info.hour >= 12 ? "pm" : "am", day_name[day % 7], day,
	    suf, month_name[time_info.month]);

    send_to_char(buf, ch);

    sprintf(buf,
	    "`&A``n`&i``m`&e M``U`&D`` started up at %s\n\rThe system time is %s\n\r",
	    str_boot_time, (char *) ctime(&current_time));

    send_to_char(buf, ch);
    return;
}



void do_weather(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    static char *const sky_look[4] = {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if (!IS_OUTSIDE(ch)) {
	send_to_char("You can't see the weather indoors.\n\r", ch);
	return;
    }

    sprintf(buf, "The sky is %s and %s.\n\r",
	    sky_look[weather_info.sky],
	    weather_info.change >=
	    0 ? "a warm southerly breeze blows" :
	    "a cold northern gust blows");
    send_to_char(buf, ch);

    return;
}



void do_help(CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH], arg1[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';

    while (argument[0] != '\0') {
	argument = one_argument(argument, arg1);

	if (argall[0] != '\0')
	    strcat(argall, " ");

	strcat(argall, arg1);
    }

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next) {
	if (pHelp->level > get_trust(ch)
	    &&
	    ((pHelp->level
	      < LEVEL_IMMORTAL && !is_granted_name(ch, pHelp->keyword))
	     || (pHelp->level >= LEVEL_IMMORTAL
		 && !is_command(pHelp->keyword)))) continue;

	if (pHelp->level >= LEVEL_IMMORTAL && is_command(pHelp->keyword)
	    && !is_granted_name(ch, pHelp->keyword))
	    continue;

	if (is_name(argall, pHelp->keyword)) {
	    if (pHelp->level >= 0 && str_cmp(argall, "imotd")) {
		send_to_char(pHelp->keyword, ch);
		send_to_char("\n\r", ch);
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if (pHelp->text[0] == '.')
		page_to_char(pHelp->text + 1, ch);
	    else
		page_to_char(pHelp->text, ch);
	    return;
	}
    }

    send_to_char("No help on that word.\n\r", ch);
    return;
}


/* whois command */
void do_whois(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;
    char pkbuf[8],		/*EE960522 */
     questbuf[25];		/*EE960522 */

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("You must provide a name.\n\r", ch);
	return;
    }

    if (IS_NPC(ch))
	return;

    output = new_buf();

    for (d = descriptor_list; d != NULL; d = d->next) {
	CHAR_DATA *wch;
	char const *class;

	if (d->connected != CON_PLAYING || !can_see(ch, d->character))
	    continue;

	wch = (d->original != NULL) ? d->original : d->character;

	if (!can_see(ch, wch))
	    continue;

	if (!str_prefix(arg, wch->name)) {
	    found = TRUE;

	    /* work out the printing */
	    class = class_table[wch->class].who_name;

	    switch (wch->level) {
	    default:
		break;
	    case MAX_LEVEL - 7:
		class = "`8A`&N`8G``";
		break;
	    case MAX_LEVEL - 8:
		class = "`3A`#V`3A``";
		break;
	    case MAX_LEVEL - 9:
		class = "`2H`@R`2O``";
		break;
	    }

	    /*EE960522 */
	    if (wch->pcdata->pkset)
			snprintf(pkbuf, sizeof(pkbuf), "`1PK``");
	    else
			snprintf(pkbuf, sizeof(pkbuf), "  ");



	    /*EE960524 */
	    if (IS_QUESTOR(wch) && (wch->pcdata->questgiver != NULL)) {
		if (IS_QUESTPKMST(wch->pcdata->questgiver))	/*EE960526 */
		    strcpy(questbuf, "`A(`!PK`1Qst`A)`` ");
		else
		    strcpy(questbuf, "`&(`4Q`Vu`^e`&s`^t`Vo`4r`&)`` ");
	    } else if (IS_QUESTMST(wch)) {	/*EE960524 */
		if (IS_QUESTPKMST(wch))
		    strcpy(questbuf, "`A(`!PK`1Ldr`A)`` ");
		else
		    strcpy(questbuf, "`&(`4Q`Vs`^t`&l`^d`Ve`4r`&)`` ");
	    } else
		strcpy(questbuf, "");


	    /* a little formatting */
	    if (wch->level <= LEVEL_IMMORTAL) {
		if (wch->pcdata->pkset) {
		    sprintf(buf, "`!+`8[`&%3d `6%4s `@%3s`8]`` ",
			    wch->level,
			    wch->race <
			    MAX_PC_RACE ? pc_race_table[wch->race].
			    who_name : "     ", class);
		    add_buf(output, buf);
		} else {
		    sprintf(buf, "`#-`8[`&%3d `6%4s `@%3s`8]`` ",
			    wch->level,
			    wch->race <
			    MAX_PC_RACE ? pc_race_table[wch->race].
			    who_name : "     ", class);
		    add_buf(output, buf);
		}
	    }
		else if (wch->level <= MAX_LEVEL
			 || (wch->level == 32760 && wch->pcdata != NULL)) {
		if (wch->pcdata->pkset) {
		    sprintf(buf, "`!+`8[``%s`8]`` ",
			    wch->pcdata->bracket_title);
		    add_buf(output, buf);
		} else {
		    sprintf(buf, "`#-`8[``%s`8]`` ",
			    wch->pcdata->bracket_title);
		    add_buf(output, buf);
		}
	    } else {
		sprintf(buf, "[%3d %4s %3s] ",
			wch->level,
			wch->race <
			MAX_PC_RACE ? pc_race_table[wch->race].who_name :
			"     ", class);
		add_buf(output, buf);
	    }

	    sprintf(buf, "%s%s%s%s%s%s%s%s%s%s%s%s\n\r",
		    ch->level >= (MAX_LEVEL - 1)
		    && IS_SET(wch->act, PLR_LOG) ? "(`8LOG``) " : "",
		    wch->incog_level >= 1 ? "`6(`^Incog`6)`` " : "",
		    wch->invis_level >= 1 ? "(`&W``i`&z``i) " : "",
		    IS_SET(wch->epf,
			   PPL_VALIDATION) ? "`6(`^New`6)`` " : "",
/*				clan_table[wch->clan].who_name, */
		    get_clan_longname(wch->clan),
		    IS_SET(wch->act, PLR_LEADER) ? "[`&Ldr``] " : "",
		    IS_SET(wch->comm, COMM_AFK) ? "[`1A`!F`1K``] " : "",
		    IS_SET(wch->act,
			   PLR_KILLER) ? "(`2K`@I`&LL`@E`2R``) " : "",
		    IS_SET(wch->act,
			   PLR_THIEF) ? "(`3T`#H`&I`#E`3F``) " : "",
		    questbuf, wch->name,
		    IS_NPC(wch) ? "" : wch->pcdata->title);
	    add_buf(output, buf);
	}
    }

    if (!found) {
	send_to_char("No one of that name is playing.\n\r", ch);
	free_buf(output);
	return;
    }

    page_to_char(buf_string(output), ch);
    free_buf(output);
}


/*
* New 'who' command originally by Alander of Rivers of Mud.
*/
void do_who(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iClass,
	iRace,
	iClan, iLevelLower = 0, iLevelUpper = MAX_LEVEL, nNumber =
	0, nMatch = 0;
    bool rgfClass[MAX_CLASS], rgfRace[MAX_PC_RACE], rgfClan[MAX_CLAN],
	fClassRestrict = FALSE, fClanRestrict = FALSE, fClan =
	FALSE, fRaceRestrict = FALSE, fImmortalOnly = FALSE;
    bool is_new = FALSE,	/*TG980725 */
     is_pk = FALSE,		/*TG970906 */
     is_nonpk = FALSE,		/* Vorlin */
     is_quest = FALSE;		/*TG971220 */
    char pkbuf[8],		/*EE960410 */
     questbuf[25];		/*EE960522 */

    /* Why? -- Suzuran */
    /* if (IS_NPC(ch))
       return; */

    /*
     * Set default arguments.
     */
    for (iClass = 0; iClass < MAX_CLASS; iClass++)
	rgfClass[iClass] = FALSE;
    for (iRace = 0; iRace < MAX_PC_RACE; iRace++)
	rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;

    /*
     * Parse arguments.
     */
    for (;;) {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg);

	if (arg[0] == '\0')
	    break;

	if (is_number(arg)) {
	    switch (++nNumber) {
	    case 1:
		iLevelLower = atoi(arg);
		if ((iLevelLower) > LEVEL_IMMORTAL)
		    iLevelLower = LEVEL_IMMORTAL;
		break;
	    case 2:
		iLevelUpper = atoi(arg);
		if (iLevelUpper > LEVEL_IMMORTAL) {
		    if (iLevelLower == LEVEL_IMMORTAL)
			iLevelUpper = MAX_LEVEL;
		    else
			iLevelLower = LEVEL_IMMORTAL;
		}
		break;
	    default:
		send_to_char("Only two level numbers allowed.\n\r", ch);
		return;
	    }
	} else if ((arg[0] == 'i') && (arg[1] == 'm')) {
	    /*TG970906 add the check against m so who invid actually works :) */
	    fImmortalOnly = TRUE;
	} else if ((arg[0] == 'p') && (arg[1] == 'k')) {
	    /*TG970906 add in to flag if looking for pk */
	    is_pk = TRUE;
	} else if (!str_cmp(arg, "new")) {
	    /*TG980725 add in this check for non-validated chars */
	    is_new = TRUE;
	} else if ((arg[0] == 'n') && (arg[1] == 'p')) {
	    is_nonpk = TRUE;
	} else if (!str_prefix(arg, "quest")) {
	    is_quest = TRUE;
	} else {
	    /*
	     * Look for classes to turn on.
	     */
	    iClass = class_lookup(arg);

	    if (iClass == -1) {
		iRace = race_lookup(arg);

		if (iRace == 0 || iRace >= MAX_PC_RACE) {
		    if (!str_prefix(arg, "clan"))
			fClan = TRUE;
		    else {
			iClan = clan_lookup(arg);

			if (iClan) {
			    fClanRestrict = TRUE;
			    rgfClan[iClan] = TRUE;
			} else {
			    send_to_char
				("That's not a valid race, class, or clan.\n\r",
				 ch);
			    return;
			}
		    }
		} else {
		    fRaceRestrict = TRUE;
		    rgfRace[iRace] = TRUE;
		}
	    } else {
		fClassRestrict = TRUE;
		rgfClass[iClass] = TRUE;
	    }
	}
    }

    /*
     * Now show matching chars.
     */
    buf[0] = '\0';
    buf2[0] = '\0';
    output = new_buf();

    add_buf(output,
	    "            `6.    ..   ...`^o`&O`6o`^O `&A``n`&i``m`&e M``U`&D `^O`6o`&O`^o`6...   ..    .``\n\r");

    for (d = descriptor_list; d != NULL; d = d->next) {
	CHAR_DATA *wch;
	char const *class;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if (d->connected != CON_PLAYING || !can_see(ch, d->character))
	    continue;

	wch = (d->original != NULL) ? d->original : d->character;

	if (!can_see(ch, wch))
	    continue;

	if (wch->level < iLevelLower
	    || wch->level > iLevelUpper
	    || (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
	    || (fClassRestrict && !rgfClass[wch->class])
	    || (fRaceRestrict && !rgfRace[wch->race])
	    || (fClan && !is_clan(wch))
	    || (fClanRestrict && !rgfClan[wch->clan])
	    || (is_quest
		&& (!IS_QUESTOR(wch) || (wch->pcdata->questgiver == NULL)))
	    || (is_pk && !(wch->pcdata->pkset))
	    || (is_nonpk && (wch->pcdata->pkset))
	    || (is_new && !IS_SET(wch->epf, PPL_VALIDATION)))
	    continue;

	nMatch++;

	/*
	 * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;

	switch (wch->level) {
	default:
	    break;
	    {
		/*EE960522 */
		/*case MAX_LEVEL - 0 : class =   "`7I`&M``P"; break;
		   case MAX_LEVEL - 1 : class =   "`4C`$R`4E``"; break;
		   case MAX_LEVEL - 2 : class =   "`1S`!U`1P``"; break;
		   case MAX_LEVEL - 3 : class =   "`4D`$E`4I``"; break;
		   case MAX_LEVEL - 4 : class =   "`5G`%O`5D``"; break;
		   case MAX_LEVEL - 5 : class =   "`6I`^M`6M``"; break;
		   case MAX_LEVEL - 6 : class =   "`8D``E`8M``"; break;
		 */
	case MAX_LEVEL - 7:
		class = "`8A`&N`8G``";
		break;
	case MAX_LEVEL - 8:
		class = "`3A`#V`3A``";
		break;
	case MAX_LEVEL - 9:
		class = "`2H`@R`2O``";
		break;
	    }
	}

	/*EE960410 */
	if (wch->pcdata->pkset)
	    snprintf(pkbuf, sizeof(pkbuf), "`1PK``");
	else
	    snprintf(pkbuf, sizeof(pkbuf), "  ");

	/*EE960522 */
	if (IS_QUESTOR(wch) && wch->pcdata->questgiver != NULL) {
	    if (IS_QUESTPKMST(wch->pcdata->questgiver))	/*EE960526 */
		strcpy(questbuf, "`8(`!PK`1Qst`8)`` ");
	    else
		strcpy(questbuf, "`&(`4Q`$u`^e`&s`^t`$o`4r`&)`` ");
	} else if (IS_QUESTMST(wch)) {	/*EE960524 */
	    if (IS_QUESTPKMST(wch))
		strcpy(questbuf, "`8(`!PK`1Ldr`8)`` ");
	    else
		strcpy(questbuf, "`&(`4Q`$s`^t`&l`^d`$e`4r`&)`` ");
	} else
	    strcpy(questbuf, "");

	/*
	 * Format it up.
	 */
	if (wch->level <= LEVEL_IMMORTAL) {
	    if (wch->pcdata->pkset) {
		sprintf(buf, "`!+`8[`&%3d `6%4s `@%3s`8]`` ",
			wch->level,
			wch->race <
			MAX_PC_RACE ? pc_race_table[wch->race].who_name :
			"     ", class);
		add_buf(output, buf);
	    } else {
		sprintf(buf, "`#-`8[`&%3d `6%4s `@%3s`8]`` ",
			wch->level,
			wch->race <
			MAX_PC_RACE ? pc_race_table[wch->race].who_name :
			"     ", class);
		add_buf(output, buf);
	    }
	}
	    else if (wch->level <= MAX_LEVEL
		     || (wch->level == 32760 && wch->pcdata != NULL)) {
	    if (wch->pcdata->pkset) {
		sprintf(buf, "`!+`8[``%s`8]`` ",
			wch->pcdata->bracket_title);
		add_buf(output, buf);
	    } else {
		sprintf(buf, "`#-`8[``%s`8]`` ",
			wch->pcdata->bracket_title);
		add_buf(output, buf);
	    }
	} else {
	    sprintf(buf, "[%3d %4s %3s] ",
		    wch->level,
		    wch->race <
		    MAX_PC_RACE ? pc_race_table[wch->race].
		    who_name : "     ", class);
	    add_buf(output, buf);
	}

	/*EE960513       1 2 3 4 5 6 7 8 9 0  */
	sprintf(buf, "%s%s%s%s%s%s%s%s%s%s%s%s\n\r",
		/*ch->level >= (MAX_LEVEL-1) && IS_SET(wch->act, PLR_LOG) ? "(`8LOG``) " : "", */
		wch->incog_level >= 1 ? "`6(`^Incog`6)`` " : "",	/*1 */
		wch->invis_level >= 1 ? "(`&W``i`&z``i) " : "",	/*2 */
		IS_SET(wch->epf, PPL_VALIDATION) ? "`6(`^New`6)`` " : "",
/*			clan_table[wch->clan].who_name, */
		get_clan_longname(wch->clan),
		IS_SET(wch->act, PLR_LEADER) ? "[`&Ldr``] " : "",	/*3 */
		IS_SET(wch->comm, COMM_AFK) ? "[`1A`!F`1K``] " : "",	/*4 */
		IS_SET(wch->act, PLR_KILLER) ? "(`2K`@I`&LL`@E`2R``) " : "",	/*5 */
		IS_SET(wch->act, PLR_THIEF) ? "(`3T`#H`&I`#E`3F``) " : "",	/*6 */
		questbuf,	/*7 */
		(wch->class == class_lookup("knight")) ? "`&Sir`` " : "",	/*8 */
		wch->name,	/*9 */
		IS_NPC(wch) ? "" : wch->pcdata->title);	/*0 */
	add_buf(output, buf);
    }

    sprintf(buf2, "\n\r`&P``layers found`8:`` `&%d``\n\r", nMatch);
    add_buf(output, buf2);

    page_to_char(buf_string(output), ch);
    free_buf(output);

    if (!IS_IMMORTAL(ch) && !IS_NPC(ch)) {
	WAIT_STATE(ch, (1.5 * PULSE_VIOLENCE));
    }
    return;
}




void do_count(CHAR_DATA * ch, char *argument)
{
    int count = 0;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    for (d = descriptor_list; d != NULL; d = d->next)
	if (d->connected == CON_PLAYING && can_see(ch, d->character))
	    count++;

    max_on = UMAX(count, max_on);

    if (max_on == count)
	sprintf(buf,
		"There %s %d character%s on, the most so far today.\n\r",
		count > 1 ? "are" : "is", max_on, count > 1 ? "s" : "");
    else
	sprintf(buf, "There %s %d character%s on right now.\n\r",
		count > 1 ? "are" : "is", count, count > 1 ? "s" : "");

    send_to_char(buf, ch);
}



void do_inventory(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    send_to_char("You are carrying:\n\r", ch);
    show_list_to_char(ch->carrying, ch, TRUE, TRUE);

    sprintf(buf,
	    "\n\rYou are carrying `3%d``/`#%d`` items weighing `3%ld``/`#%d`` pounds.\n\r",
	    ch->carry_number, can_carry_n(ch), get_carry_weight(ch) / 10,
	    can_carry_w(ch) / 10);
    send_to_char(buf, ch);

    return;
}



void do_equipment(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char("You are using:\n\r", ch);
    found = FALSE;

    for (iWear = 0; iWear < MAX_WEAR; iWear++) {
	if ((obj = get_eq_char(ch, iWear)) == NULL)
	    continue;

	send_to_char(where_name[iWear], ch);

	if (can_see_obj(ch, obj)) {
	    send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
	    send_to_char("\n\r", ch);
	} else {
	    send_to_char("something.\n\r", ch);
	}

	found = TRUE;
    }

    if (!found)
	send_to_char("Nothing.\n\r", ch);

    return;
}



void do_compare(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0') {
	send_to_char("Compare what to what?\n\r", ch);
	return;
    }

    if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
	send_to_char("You do not have that item.\n\r", ch);
	return;
    }

    if (arg2[0] == '\0') {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content) {
	    if (obj2->wear_loc != WEAR_NONE && can_see_obj(ch, obj2)
		&& obj1->item_type == obj2->item_type
		&& (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
		break;
	}

	if (obj2 == NULL) {
	    send_to_char("You aren't wearing anything comparable.\n\r",
			 ch);
	    return;
	}
    } else if ((obj2 = get_obj_carry(ch, arg2)) == NULL) {
	send_to_char("You do not have that item.\n\r", ch);
	return;
    }

    msg = NULL;
    value1 = 0;
    value2 = 0;

    if (obj1 == obj2) {
	msg = "You compare $p to itself.  It looks about the same.";
    } else if (obj1->item_type != obj2->item_type) {
	msg = "You can't compare $p and $P.";
    } else {
	switch (obj1->item_type) {
	default:
	    msg = "You can't compare $p and $P.";
	    break;
	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;
	case ITEM_WEAPON:
	    value1 = (1 + obj1->value[2]) * obj1->value[1];
	    value2 = (1 + obj2->value[2]) * obj2->value[1];
	    break;
	}
    }

    if (msg == NULL) {
	if (value1 == value2)
	    msg = "$p and $P look about the `Qsame``.";
	else if (value1 > value2)
	    msg = "$p looks `Rbetter`` than $P.";
	else
	    msg = "$p looks `Lworse`` than $P.";
    }

    act(msg, ch, obj1, obj2, TO_CHAR);
    return;
}


void do_credits(CHAR_DATA * ch, char *argument)
{
    do_help(ch, "diku");
    return;
}



void do_where(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Players near you:\n\r", ch);
	found = FALSE;
	for (d = descriptor_list; d; d = d->next) {
	    if (d->connected == CON_PLAYING
		&& (victim = d->character) != NULL && !IS_NPC(victim)
		&& victim->in_room != NULL
		&& !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE)
		&& (is_room_owner(ch, victim->in_room)
		    || !room_is_private(victim->in_room))
		&& victim->in_room->area == ch->in_room->area
		&& can_see(ch, victim)) {
		found = TRUE;
		sprintf(buf, "%-28s %s\n\r",
			victim->name, victim->in_room->name);
		send_to_char(buf, ch);
	    }
	}
	if (!found)
	    send_to_char("None\n\r", ch);
    } else {
	found = FALSE;
	for (victim = char_list; victim != NULL; victim = victim->next) {
	    if (victim->in_room != NULL
		&& victim->in_room->area == ch->in_room->area
		&& !IS_AFFECTED(victim, AFF_HIDE)
		&& !IS_AFFECTED(victim, AFF_SNEAK)
		&& can_see(ch, victim) && is_name(arg, victim->name)) {
		found = TRUE;
		sprintf(buf, "%-28s %s\n\r",
			PERS(victim, ch), victim->in_room->name);
		send_to_char(buf, ch);
		break;
	    }
	}
	if (!found)
	    act("You did not find any $T.", ch, NULL, arg, TO_CHAR);
    }

    return;
}




void do_consider(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Consider killing whom?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They are not here.\n\r", ch);
	return;
    }

    if (is_safe(ch, victim)) {
	send_to_char("Do not even think about it.\n\r", ch);
	return;
    }

    diff = victim->level - ch->level;

    if (diff <= -10)
	msg = "You can kill $N naked and weaponless.";
    else if (diff <= -5)
	msg = "$N is no match for you.";
    else if (diff <= -2)
	msg = "$N looks like an easy kill.";
    else if (diff <= 1)
	msg = "The perfect match!";
    else if (diff <= 4)
	msg =
	    "$N says 'Ah, a couple seconds work, for a lots of blood cleaning'.";
    else if (diff <= 6)
	msg = "$N grins at you evilly and pats you on your head.";
    else if (diff <= 9)
	msg = "$N laughs at you mercilessly.";
    else if (diff <= 12)
	msg =
	    "$N pulls out their sacrificial dagger and begins to sharpen it.";
    else if (diff <= 20)
	msg = "$N taunts you with a hanging noose.";
    else
	msg = "Death will thank you for your gift.";

    act(msg, ch, NULL, victim, TO_CHAR);
    return;
}



void set_title(CHAR_DATA * ch, char *title)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
	bug("Set_title: NPC.", 0);
	return;
    }

    if (title[0] != '.' && title[0] != ',' && title[0] != '!'
	&& title[0] != '?') {
	buf[0] = ' ';
	strcpy(buf + 1, title);
    } else {
	strcpy(buf, title);
    }

    free_string(ch->pcdata->title);
    ch->pcdata->title = str_dup(buf);

    return;
}



void do_title(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if (argument[0] == '\0') {
	send_to_char("Change your title to what?\n\r", ch);
	return;
    }

    if (strlen(argument) > 45)
	argument[45] = '\0';

    smash_tilde(argument);
    set_title(ch, argument);
    send_to_char("Ok.\n\r", ch);
}



void do_description(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (argument[0] != '\0') {
	buf[0] = '\0';
	smash_tilde(argument);


	if (argument[0] == '-') {
	    int len;
	    bool found = FALSE;

	    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	    if (ch->description == NULL || ch->description[0] == '\0') {
		send_to_char("No lines left to remove.\n\r", ch);
		return;
	    }

	    strcpy(buf, ch->description);

	    for (len = strlen(buf); len > 0; len--) {
		if (buf[len] == '\r') {
		    if (!found) {	/* back it up */
			if (len > 0)
			    len--;

			found = TRUE;
		    } else {	/* found the second one */
			buf[len + 1] = '\0';
			free_string(ch->description);
			ch->description = str_dup(buf);
			send_to_char("Your description is:\n\r", ch);
			send_to_char(ch->description ? ch->description :
				     "(None).\n\r", ch);

			return;
		    }
		}
	    }

	    buf[0] = '\0';
	    free_string(ch->description);
	    ch->description = str_dup(buf);
	    send_to_char("Description cleared.\n\r", ch);
	    return;
	}

	if (argument[0] == '+') {
	    if (ch->description != NULL)
		strcat(buf, ch->description);

	    argument++;

	    while (isspace(*argument))
		argument++;
	}

	if (strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2) {
	    send_to_char("Description too long.\n\r", ch);
	    return;
	}

	strcat(buf, argument);
	strcat(buf, "\n\r");
	free_string(ch->description);
	ch->description = str_dup(buf);
    }

    send_to_char("Your description is:\n\r", ch);
    send_to_char(ch->description ? ch->description : "(None).\n\r", ch);
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

    return;
}



void do_report(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf,
	    "You say 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'\n\r",
	    ch->hit, ch->max_hit,
	    ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);

    send_to_char(buf, ch);

    sprintf(buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'",
	    ch->hit, ch->max_hit,
	    ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp);

    act(buf, ch, NULL, NULL, TO_ROOM);

    return;
}



void do_practice(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if (IS_NPC(ch))
	return;

    if (argument[0] == '\0') {
	int col;

	col = 0;
	for (sn = 0; sn < MAX_SKILL; sn++) {
	    if (skill_table[sn].name == NULL)
		break;
	    if (ch->level < skill_table[sn].skill_level[ch->class]
		|| ch->pcdata->learned[sn] < 1 /* skill is not known */ )
		continue;

	    sprintf(buf, "%-18s `^%3d`6%%``  ",
		    skill_table[sn].name, ch->pcdata->learned[sn]);
	    send_to_char(buf, ch);
	    if (++col % 3 == 0)
		send_to_char("\n\r", ch);
	}

	if (col % 3 != 0)
	    send_to_char("\n\r", ch);

	sprintf(buf, "You have `&%d`` practice sessions left.\n\r",
		ch->practice);
	send_to_char(buf, ch);
    } else {
	CHAR_DATA *mob;
	int adept;

	if (!IS_AWAKE(ch)) {
	    send_to_char("In your dreams, or what?\n\r", ch);
	    return;
	}

	for (mob = ch->in_room->people; mob != NULL;
	     mob = mob->next_in_room) {
	    if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
		break;
	}

	if (mob == NULL) {
	    send_to_char("You can not do that here.\n\r", ch);
	    return;
	}

	if (ch->practice <= 0) {
	    send_to_char("You have no practice sessions left.\n\r", ch);
	    return;
	}

	if ((sn = skill_lookup(argument)) < 0 || (!IS_NPC(ch)
						  && (ch->level <
						      skill_table
						      [sn].skill_level
						      [ch->class]
						      || ch->pcdata->
						      learned[sn] < 1	/* skill is not known */
						      || skill_table[sn].
						      rating[ch->class] ==
						      0))) {
	    send_to_char("You can not practice that.\n\r", ch);
	    return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

	if (ch->pcdata->learned[sn] >= adept) {
	    sprintf(buf, "You are already learned at %s.\n\r",
		    skill_table[sn].name);
	    send_to_char(buf, ch);
	} else {
	    ch->practice--;
	    ch->pcdata->learned[sn] +=
		int_app[get_curr_stat(ch, STAT_INT)].learn /
		skill_table[sn].rating[ch->class];
	    if (ch->pcdata->learned[sn] < adept) {
		act("You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR);
		act("$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM);
	    } else {
		ch->pcdata->learned[sn] = adept;
		act("You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR);
		act("$n is now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM);
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument(argument, arg);

    if (arg[0] == '\0')
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi(arg);

    if (wimpy < 0) {
	send_to_char("Your courage exceeds your wisdom.\n\r", ch);
	return;
    }

    if (wimpy > ch->max_hit / 2) {
	send_to_char("Such cowardice ill becomes you.\n\r", ch);
	return;
    }

    ch->wimpy = wimpy;
    sprintf(buf, "Wimpy set to %d hit points.\n\r", wimpy);
    send_to_char(buf, ch);

    return;
}



void do_password(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if (IS_NPC(ch))
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    cEnd = ' ';

    while (isspace(*argument))
	argument++;


    if (*argument == '\'' || *argument == '"')
	cEnd = *argument++;

    while (*argument != '\0') {
	if (*argument == cEnd) {
	    argument++;
	    break;
	}

	*pArg++ = *argument++;
    }

    *pArg = '\0';
    pArg = arg2;
    cEnd = ' ';

    while (isspace(*argument))
	argument++;

    if (*argument == '\'' || *argument == '"')
	cEnd = *argument++;

    while (*argument != '\0') {
	if (*argument == cEnd) {
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }

    *pArg = '\0';

    if (arg1[0] == '\0' || arg2[0] == '\0') {
	send_to_char("Syntax: password <old> <new>.\n\r", ch);
	return;
    }

    if (strcmp(crypt(arg1, ch->pcdata->pwd), ch->pcdata->pwd)) {
	WAIT_STATE(ch, 40);
	send_to_char("Wrong password.  Wait 10 seconds.\n\r", ch);

	return;
    }

    if (strlen(arg2) < 5) {
	send_to_char
	    ("New password must be at least five characters long.\n\r",
	     ch);
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt(arg2, ch->name);

    for (p = pwdnew; *p != '\0'; p++) {
	if (*p == '~') {
	    send_to_char("New password not acceptable, try again.\n\r",
			 ch);
	    return;
	}
    }

    free_string(ch->pcdata->pwd);
    ch->pcdata->pwd = str_dup(pwdnew);
    save_char_obj(ch);
    send_to_char("Ok.\n\r", ch);

    return;
}



/*EE960420*/
void do_stats(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    send_to_char("`&C``urrent `&S``tats`A:``\n\r", ch);
    sprintf(buf,
	    "`1>```!>`@Str`A:`` %d`&(``%d`&) `@Int`A:`` %d`&(``%d`&) `@Wis`A:`` %d`&(``%d`&) `@Dex`A:`` %d`&(``%d`&) `@Con`A:`` %d`&(``%d`&)``\n\r",
	    ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR),
	    ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT),
	    ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS),
	    ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX),
	    ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON));
    send_to_char(buf, ch);
    sprintf(buf,
	    "`3>`#>`^Strmod`8:`` %+3d `^Intmod`8:`` %+3d `^Wismod`8:`` %+3d `^Dexmod`8:`` %+3d `^Conmod`8:`` %+3d\n\r",
	    get_stat_mod(ch, STAT_STR),
            get_stat_mod(ch, STAT_INT),
            get_stat_mod(ch, STAT_WIS),
            get_stat_mod(ch, STAT_DEX),
	    get_stat_mod(ch, STAT_CON));
    send_to_char(buf, ch);
    return;
}



/*EE960420*/
void do_maxstats(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    sh_int MAX_STR, MAX_INT, MAX_WIS, MAX_DEX, MAX_CON,
	MAX_STR_ITEM, MAX_INT_ITEM, MAX_WIS_ITEM, MAX_DEX_ITEM,
	MAX_CON_ITEM;

    MAX_STR = UMIN(25, (pc_race_table[ch->race].max_stats[STAT_STR]));
    MAX_INT = UMIN(25, (pc_race_table[ch->race].max_stats[STAT_INT]));
    MAX_WIS = UMIN(25, (pc_race_table[ch->race].max_stats[STAT_WIS]));
    MAX_DEX = UMIN(25, (pc_race_table[ch->race].max_stats[STAT_DEX]));
    MAX_CON = UMIN(25, (pc_race_table[ch->race].max_stats[STAT_CON]));

    switch (class_table[ch->class].attr_prime) {
    case STAT_STR:
	{
	    MAX_STR = UMIN(25, MAX_STR + 2);
	    break;
	}
    case STAT_INT:
	{
	    MAX_INT = UMIN(25, MAX_INT + 2);
	    break;
	}
    case STAT_WIS:
	{
	    MAX_WIS = UMIN(25, MAX_WIS + 2);
	    break;
	}
    case STAT_DEX:
	{
	    MAX_DEX = UMIN(25, MAX_DEX + 2);
	    break;
	}
    case STAT_CON:
	{
	    MAX_CON = UMIN(25, MAX_CON + 2);
	    break;
	}
    }

    MAX_STR_ITEM = UMIN(25, MAX_STR + 4);
    MAX_INT_ITEM = UMIN(25, MAX_INT + 4);
    MAX_WIS_ITEM = UMIN(25, MAX_WIS + 4);
    MAX_DEX_ITEM = UMIN(25, MAX_DEX + 4);
    MAX_CON_ITEM = UMIN(25, MAX_CON + 4);


    send_to_char("`&M``ax `&S``tats`A:``\n\r", ch);
    sprintf(buf,
	    "`1>```!>`@Str`A:`` %d`&(``%d`&) `@Int`A:`` %d`&(``%d`&) `@Wis`A:`` %d`&(``%d`&) `@Dex`A:`` %d`&(``%d`&) `@Con`A:`` %d`&(``%d`&)``\n\r",
	    MAX_STR, MAX_STR_ITEM, MAX_INT, MAX_INT_ITEM, MAX_WIS,
	    MAX_WIS_ITEM, MAX_DEX, MAX_DEX_ITEM, MAX_CON, MAX_CON_ITEM);
    send_to_char(buf, ch);

    return;
}



/*EE960504*/
void do_search(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char target_item[MAX_STRING_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found = FALSE;
    int number = 0, max_found = IS_IMMORTAL(ch) ? 200 : ch->level;

    if (!IS_IMMORTAL(ch)) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (!IS_IMMORTAL(ch)) {
	send_to_char("Sorry, this skill isn't fully implemented yet.\n\r",
		     ch);
	return;
    } else {
	send_to_char
	    ("------------------------------------------------------------------\n\r",
	     ch);
	send_to_char
	    ("Ok... as you wish. The mortals don't have access to this skill yet,\n\r ",
	     ch);
	send_to_char
	    ("but because you're a god, I'll let you play with it. :)\n\r",
	     ch);
	send_to_char
	    ("Know though, this code may crash the mud and probably has some big\n\r",
	     ch);
	send_to_char
	    ("bugs in it. If you find any bugs, please write a note to Belgarion.\n\r",
	     ch);
	send_to_char
	    ("------------------------------------------------------------------\n\r\n\r",
	     ch);
    }

    if (argument[0] == '\0') {
	send_to_char("Syntax: search <item>\n\r", ch);
	return;
    } else if (!IS_NPC(ch) && ch->mana < 30) {
	send_to_char("You are too tired to try that.\n\r", ch);
	return;
    } else if (number_percent() > (3 * ch->level)) {
	send_to_char("Nothing like that nearby.\n\r", ch);
	return;
    }

    argument = one_argument(argument, target_item);
    buffer = new_buf();

    for (obj = object_list; obj != NULL; obj = obj->next) {
	if (!can_see_obj(ch, obj)
	    || !is_name(target_item, obj->name)
	    || IS_OBJ_STAT(obj, ITEM_NOLOCATE)
	    || number_percent() > 2 * ch->level || ch->level < obj->level)
	    continue;

	found = TRUE;
	number++;

	for (in_obj = obj; in_obj->in_obj != NULL;
	     in_obj = in_obj->in_obj);

	if (in_obj->carried_by != NULL && can_see(ch, in_obj->carried_by)) {
	    sprintf(buf, "%s is carried by %s\n\r", obj->short_descr,
		    PERS(in_obj->carried_by, ch));
	} else {
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf(buf, "%s is in %s [Room %d]\n\r", obj->short_descr,
			in_obj->in_room->name, in_obj->in_room->vnum);
	    else
		sprintf(buf, "%s is in %s\n\r", obj->short_descr,
			in_obj->in_room ==
			NULL ? "somewhere" : in_obj->in_room->name);
	}

	buf[0] = UPPER(buf[0]);
	add_buf(buffer, buf);

	if (number >= max_found)
	    break;
    }

    if (!found) {
	send_to_char("Nothing like that nearby.\n\r", ch);
	ch->mana -= 15;
    } else {
	page_to_char(buf_string(buffer), ch);
	ch->mana -= 30;
    }

    free_buf(buffer);
    return;
}

void do_ovalue(CHAR_DATA * ch, char *argument)
{
    if ((ch->race != race_lookup("tolnedran")) && !IS_IMMORTAL(ch)) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    send_to_char("Sorry, this skill isn't fully implemented yet.\n\r", ch);
    return;

    if (argument[0] == '\0') {
	send_to_char("Syntax: ovalue <item>\n\r", ch);
	return;
    }
}

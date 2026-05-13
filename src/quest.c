
/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   * 
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.01. Please do not remove this notice from this file. *
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_whisper);

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 15040
#define QUEST_ITEM2 15041
#define QUEST_ITEM3 15060
#define QUEST_ITEM4 15050
#define QUEST_ITEM5 15057
#define QUEST_ITEM6 15059
#define QUEST_ITEM7 15042
#define QUEST_ITEM8 15043
#define QUEST_ITEM9 15056
#define QUEST_ITEM10 15055
#define QUEST_ITEM11 15058
#define QUEST_ITEM12 15044

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
items are worthless and have the rot-death flag, as they are placed
into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1 15045
#define QUEST_OBJQUEST2 15046
#define QUEST_OBJQUEST3 15047
#define QUEST_OBJQUEST4 15048
#define QUEST_OBJQUEST5 15049

/* Local functions */

void generate_quest args((CHAR_DATA * ch, CHAR_DATA * questman));
void quest_update args((void));
bool quest_level_diff args((int clevel, int mlevel));
bool chance args((int num));
ROOM_INDEX_DATA *find_location(CHAR_DATA * ch, char *arg);

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1, 100) <= num)
	return TRUE;
    else
	return FALSE;
}

/* The main quest function */

void do_autoqst(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj = NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch))
	return;

    if (arg1[0] == '\0') {
	send_to_char
	    ("`&AUTOQUEST`` commands`A: `&POINTS INFO TIME REQUEST GIVEUP COMPLETE LIST BUY``.\n\r",
	     ch);
	send_to_char
	    ("For more information, type `A'`&HELP AUTOQUEST`A'``. Related Topic `&HELP AQSTRING``\n\r",
	     ch);
	return;
    }
    if (!strcmp(arg1, "info")) {
	if (IS_SET(ch->act, PLR_AUTOQST)) {
	    if (ch->questmob == -1 && ch->questgiver->short_descr != NULL) {
		sprintf(buf,
			"Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",
			ch->questgiver->short_descr);
		send_to_char(buf, ch);
	    } else if (ch->questobj > 0) {
		questinfoobj = get_obj_index(ch->questobj);
		if (questinfoobj != NULL) {
		    sprintf(buf,
			    "You are on a quest to recover the fabled %s!\n\r",
			    questinfoobj->short_descr);
		    send_to_char(buf, ch);
		} else
		    send_to_char("You aren't currently on a quest.\n\r",
				 ch);
		return;
	    } else if (ch->questmob > 0) {
		questinfo = get_mob_index(ch->questmob);
		if (questinfo != NULL) {
		    sprintf(buf,
			    "You are on a quest to slay the dreaded %s!\n\r",
			    questinfo->short_descr);
		    send_to_char(buf, ch);
		} else
		    send_to_char("You aren't currently on a quest.\n\r",
				 ch);
		return;
	    }
	} else
	    send_to_char("You aren't currently on a quest.\n\r", ch);
	return;
    }
    if (!strcmp(arg1, "points")) {
	sprintf(buf, "You have `V%d`` quest points.\n\r", ch->questpoints);
	send_to_char(buf, ch);
	return;
    }

    else if (!strcmp(arg1, "time")) {
	if (!IS_SET(ch->act, PLR_AUTOQST)) {
	    send_to_char("You aren't currently on a quest.\n\r", ch);
	    if (ch->nextquest > 1) {
		sprintf(buf,
			"There are %d minutes remaining until you can go on another quest.\n\r",
			ch->nextquest);
		send_to_char(buf, ch);
	    } else if (ch->nextquest == 1) {
		sprintf(buf,
			"There is less than a minute remaining until you can go on another quest.\n\r");
		send_to_char(buf, ch);
	    }
	} else if (ch->countdown > 0) {
	    sprintf(buf, "Time left for current quest: %d\n\r",
		    ch->countdown);
	    send_to_char(buf, ch);
	}
	return;
    }

    /* Checks for a character in the room with spec_questmaster set. This special
       procedure must be defined in special.c. You could instead use an 
       ACT_QUESTMASTER flag instead of a special procedure. */

    for (questman = ch->in_room->people; questman != NULL;
	 questman = questman->next_in_room) {
	if (!IS_NPC(questman))
	    continue;
	if (questman->spec_fun == spec_lookup("spec_questmaster"))
	    break;
    }

    if (questman == NULL
	|| questman->spec_fun != spec_lookup("spec_questmaster")) {
	send_to_char("You can't do that here.\n\r", ch);
	return;
    }

    if (questman->fighting != NULL) {
	send_to_char("Wait until the fighting stops.\n\r", ch);
	return;
    }

    ch->questgiver = questman;

    /* And, of course, you will need to change the following lines for YOUR
       quest item information. Quest items on Moongate are unbalanced, very
       very nice items, and no one has one yet, because it takes awhile to
       build up quest points :> Make the item worth their while. */

    if (!strcmp(arg1, "list")) {
	act("$n asks $N for a list of quest items.", ch, NULL, questman,
	    TO_ROOM);
	act("You ask $N for a list of quest items.", ch, NULL, questman,
	    TO_CHAR);

	sprintf(buf,
		"Current Quest Items available for Purchase`A:``\n\r"
		"4000qp..........A Crimson-colored Sash of Champions\n\r"
		"3200qp..........A Heron-Marked Blade\n\r"
		"2400qp..........Pikachu-Hide Boots\n\r"
		"2320qp..........A silver-spiked Warbracer name \"Kaiserflare\"\n\r"
		"2240qp..........A Golden Chocobo Figurine\n\r"
		"2000qp..........Gauntlets of Fury\n\r"
		"1840qp..........Amulet of the Ancients\n\r"
		"1680qp..........The DiMENSiONAL Shield\n\r"
		"1600qp..........Ring of Talpa\n\r"
		"1440qp..........Ronin Band of Wisdom\n\r"
		"1200qp..........Jug of Cabbit Juice\n\r"
		"1200qp..........Decanter of Endless Water\n\r"
		"700qp..........Ability to Restring `6(`^help AQstring`6)``\n\r"
		"To buy an item, type `A'`&AUTOQUEST BUY <``item`&>`A'``.\n\r"
		"To see the color strings for these items, type '`&help autoquest_eq``'\n\r");
	send_to_char(buf, ch);
	return;
    }

    else if (!strcmp(arg1, "buy")) {
	if (arg2[0] == '\0') {
	    send_to_char("To buy an item, type 'QUEST BUY <item>'.\n\r",
			 ch);
	    return;
	}
	if (is_name(arg2, "crimson sash champions")) {
	    if (ch->questpoints >= 4000) {
		ch->questpoints -= 4000;
		obj =
		    create_object(get_obj_index(QUEST_ITEM1), 
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "heron blade marked")) {
	    if (ch->questpoints >= 3200) {
		ch->questpoints -= 3200;
		obj =
		    create_object(get_obj_index(QUEST_ITEM2),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "pikachu-hide hide boots")) {
	    if (ch->questpoints >= 2400) {
		ch->questpoints -= 2400;
		obj =
		    create_object(get_obj_index(QUEST_ITEM3),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "warbracer kaiserflare")) {
	    if (ch->questpoints >= 2320) {
		ch->questpoints -= 2320;
		obj =
		    create_object(get_obj_index(QUEST_ITEM4),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }

	} else if (is_name(arg2, "golden chocobo figurine")) {
	    if (ch->questpoints >= 2240) {
		ch->questpoints -= 2240;
		obj =
		    create_object(get_obj_index(QUEST_ITEM5),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "gauntlets fury")) {
	    if (ch->questpoints >= 2000) {
		ch->questpoints -= 2000;
		obj =
		    create_object(get_obj_index(QUEST_ITEM6),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "amulet ancients")) {
	    if (ch->questpoints >= 1840) {
		ch->questpoints -= 1840;
		obj =
		    create_object(get_obj_index(QUEST_ITEM7),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "dimensional shield")) {
	    if (ch->questpoints >= 1680) {
		ch->questpoints -= 1680;
		obj =
		    create_object(get_obj_index(QUEST_ITEM8),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "ring talpa")) {
	    if (ch->questpoints >= 1600) {
		ch->questpoints -= 1600;
		obj =
		    create_object(get_obj_index(QUEST_ITEM9),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "band ronin")) {
	    if (ch->questpoints >= 1440) {
		ch->questpoints -= 1440;
		obj =
		    create_object(get_obj_index(QUEST_ITEM10),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "jug cabbit juice")) {
	    if (ch->questpoints >= 1200) {
		ch->questpoints -= 1200;
		obj =
		    create_object(get_obj_index(QUEST_ITEM11),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else if (is_name(arg2, "decanter endless water")) {
	    if (ch->questpoints >= 1200) {
		ch->questpoints -= 1200;
		obj =
		    create_object(get_obj_index(QUEST_ITEM12),
				  (ch->level + 20));
		obj->level = ch->level + 20;
	    } else {
		sprintf(buf,
			"Sorry, %s, but you don't have enough quest points for that.",
			ch->name);
		do_say(questman, buf);
		return;
	    }
	} else {
	    sprintf(buf, "I don't have that item, %s.", ch->name);
	    do_say(questman, buf);
	}

	if (obj != NULL) {
	    obj->owner = str_dup(ch->name);
	    /* Pkers buy everything but herons at obj level 20 - KV020516 */
	    if (ch->pcdata->pkset && obj->pIndexData->vnum != QUEST_ITEM2)
		    obj->level = 20;
	    act("$N gives `&$p`` to `&$n``.", ch, obj, questman, TO_ROOM);
	    act("$N gives you `&$p``.", ch, obj, questman, TO_CHAR);
	    obj_to_char(obj, ch);
	}
	return;
    } else if (!strcmp(arg1, "request")) {
	act("$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
	act("You ask $N for a quest.", ch, NULL, questman, TO_CHAR);
	sprintf(log_buf, "AQUEST LOG: after acts");
	log_string(log_buf);

	if (IS_SET(ch->act, PLR_AUTOQST)) {
	    sprintf(log_buf, "AQUEST LOG: if ch is set PLR_AUTOQST");
	    log_string(log_buf);
	    sprintf(buf, "But you're already on a quest!");
	    do_say(questman, buf);
	    return;
	}
	if (ch->nextquest > 0) {
	    sprintf(log_buf, "AQUEST LOG: if timer = 0");
	    log_string(log_buf);
	    sprintf(buf,
		    "You're very brave, %s, but let someone else have a chance.",
		    ch->name);
	    do_say(questman, buf);
	    sprintf(buf, "Come back later.");
	    do_say(questman, buf);
	    return;
	}

	sprintf(buf, "Thank you, brave %s!", ch->name);
	do_say(questman, buf);
	ch->questmob = 0;
	ch->questobj = 0;
	sprintf(log_buf, "AQUEST LOG: after thanks");
	log_string(log_buf);

	generate_quest(ch, questman);
	sprintf(log_buf, "AQUEST LOG: generate_quest");
	log_string(log_buf);

	if (ch->questmob > 0 || ch->questobj > 0) {
	    sprintf(log_buf, "AQUEST LOG: questmob, questman check");
	    log_string(log_buf);
	    ch->countdown = number_range(10, 30);
	    SET_BIT(ch->act, PLR_AUTOQST);
	    sprintf(buf, "You have %d minutes to complete this quest.",
		    ch->countdown);
	    do_say(questman, buf);
	    sprintf(buf, "May the gods go with you!");
	    do_say(questman, buf);
	}
	sprintf(log_buf, "AQUEST LOG: Before Return");
	log_string(log_buf);

	return;
    } else if (!strcmp(arg1, "giveup")) {
	if (IS_SET(ch->act, PLR_AUTOQST)) {
	    sprintf(buf,
		    "I'm quite sorry that you cannot complete the quest.");
	    do_say(questman, buf);
	    sprintf(buf,
		    "You may return to attempt another quest when you feel ready.");
	    do_say(questman, buf);
	    REMOVE_BIT(ch->act, PLR_AUTOQST);
	    ch->questgiver = NULL;
	    ch->countdown = 0;
	    ch->questmob = 0;
	    ch->questobj = 0;
	    ch->nextquest = 6;
	    return;
	} else {
	    sprintf(buf, "You are not currently on a quest.\n\r");
	    send_to_char(buf, ch);
	    return;
	}
    } else if (!strcmp(arg1, "complete")) {
	act("$n informs $N $e has completed $s quest.", ch, NULL, questman,
	    TO_ROOM);
	act("You inform $N you have completed $s quest.", ch, NULL,
	    questman, TO_CHAR);
	if (ch->questgiver != questman) {
	    sprintf(buf,
		    "I never sent you on a quest! Perhaps you're thinking of someone else.");
	    do_say(questman, buf);
	    return;
	}

	if (IS_SET(ch->act, PLR_AUTOQST)) {
	    if (ch->questmob == -1 && ch->countdown > 0) {
		int reward, pointreward, pkbonus;	/*, pracreward; */

		reward = number_range(75, 150);
		pointreward = number_range(75, 150);
		ch->hit = ch->max_hit;	/* restore character */

		/* Restore mana too -- Suzuran */
		ch->mana = ch->max_mana;

		/* Restore movement too -- Kevin */
		ch->move = ch->max_move;

		pkbonus = 0;
		if (ch->pcdata->pkset) {
		    if (ch->level >= 20 && ch->level <= 40)
			pkbonus = 30;
		    else if (ch->level >= 41 && ch->level <= 60)
			pkbonus = 45;
		    else if (ch->level >= 61 && ch->level <= 80)
			pkbonus = 60;
		    else if (ch->level >= 81)
			pkbonus = 75;
		}
                    
		if (chance(pkbonus))
		    pointreward *= 2;

		sprintf(buf, "Congratulations on completing your quest!");
		do_say(questman, buf);
		sprintf(buf,
			"As a reward, I am giving you `V%d `^quest points, and `#%d`` `^gold.``",
			pointreward, reward);
		do_say(questman, buf);

		/* Chance to receive practices taken out. JRO981113..

		   if (chance(15))
		   {
		   pracreward = number_range(1,1);
		   sprintf(buf, "You gain `&%d`` practices!\n\r",pracreward);
		   send_to_char(buf, ch);
		   ch->practice += pracreward;
		   }

		 */

		REMOVE_BIT(ch->act, PLR_AUTOQST);
		/* Check to stop questing with protection of DEAD flag - KV020806 */
		if (IS_SET(ch->act, PLR_DEAD)) {
		    REMOVE_BIT(ch->act, PLR_DEAD);
		}
		ch->questgiver = NULL;
		ch->countdown = 0;
		ch->questmob = 0;
		ch->questobj = 0;
		if (ch->pcdata->pkset)
		    ch->nextquest = number_range(6, 12);
		else
		    ch->nextquest = number_range(6, 12);
		ch->gold += reward;
		ch->questpoints += pointreward;

		return;
	    } else if (ch->questobj > 0 && ch->countdown > 0) {
		bool obj_found = FALSE;

		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		    obj_next = obj->next_content;

		    if (obj != NULL
			&& obj->pIndexData->vnum == ch->questobj) {
			obj_found = TRUE;
			break;
		    }
		}
		if (obj_found == TRUE) {
		    int reward, pointreward, pkbonus;	/* pracreward */

		    reward = number_range(75, 150);
		    pointreward = number_range(75, 150);

		    pkbonus = 0;
		    if (ch->pcdata->pkset) {
			if (ch->level >= 20 && ch->level <= 40)
			    pkbonus = 30;
			else if (ch->level >= 41 && ch->level <= 60)
			    pkbonus = 45;
			else if (ch->level >= 61 && ch->level <= 80)
			    pkbonus = 60;
			else if (ch->level >= 81)
			    pkbonus = 75;
		    }

		    if (chance(pkbonus))
			pointreward *= 2;

		    act("You hand `&$p`` to $N.", ch, obj, questman,
			TO_CHAR);
		    act("$n hands `&$p`` to $N.", ch, obj, questman,
			TO_ROOM);

		    sprintf(buf,
			    "Congratulations on completing your quest!");
		    do_say(questman, buf);
		    sprintf(buf,
			    "As a reward, I am giving you `V%d`` `^quest points, and `#%d `^gold.``",
			    pointreward, reward);
		    do_say(questman, buf);
		    /*
		       if (chance(15))
		       {
		       pracreward = number_range(1,3);
		       sprintf(buf, "You gain `&%d`` practices!\n\r",pracreward);
		       send_to_char(buf, ch);
		       ch->practice += pracreward;
		       }

		     */
		    REMOVE_BIT(ch->act, PLR_AUTOQST);
		    /* Check to stop questing with protection of DEAD flag - KV020806 */
		    if (IS_SET(ch->act, PLR_DEAD)) {
			REMOVE_BIT(ch->act, PLR_DEAD);
		    }
		    ch->questgiver = NULL;
		    ch->countdown = 0;
		    ch->questmob = 0;
		    ch->questobj = 0;
		    if (ch->pcdata->pkset)
			ch->nextquest = number_range(6, 12);
		    else
			ch->nextquest = number_range(6, 12);
		    ch->gold += reward;
		    ch->questpoints += pointreward;
		    extract_obj(obj);
		    return;
		} else {
		    sprintf(buf,
			    "You haven't completed the quest yet, but there is still time!");
		    do_say(questman, buf);
		    return;
		}
		return;
	    } else if ((ch->questmob > 0 || ch->questobj > 0)
		       && ch->countdown > 0) {
		sprintf(buf,
			"You haven't completed the quest yet, but there is still time!");
		do_say(questman, buf);
		return;
	    }
	}
	if (ch->nextquest > 0)
	    sprintf(buf, "But you didn't complete your quest in time!");
	else
	    sprintf(buf, "You have to REQUEST a quest first, %s.",
		    ch->name);
	do_say(questman, buf);
	return;
    }

    send_to_char
	("AUTOQUEST commands: POINTS INFO TIME REQUEST GIVEUP COMPLETE LIST BUY.\n\r",
	 ch);
    send_to_char("For more information, type 'HELP AUTO_QUEST'.\n\r", ch);
    return;
}

void generate_quest(CHAR_DATA * ch, CHAR_DATA * questman)
{
    CHAR_DATA *victim;
    MOB_INDEX_DATA *vsearch=NULL;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    char buf[MAX_STRING_LENGTH];
    long mcounter;
    int mob_vnum;

    /*  Randomly selects a mob from the world mob list. If you don't
       want a mob to be selected, make sure it is immune to summon.
       Or, you could add a new mob flag called ACT_NOQUEST. The mob
       is selected for both mob and obj quests, even tho in the obj
       quest the mob is not used. This is done to assure the level
       of difficulty for the area isn't too great for the player. */

    for (mcounter = 0; mcounter < 99999; mcounter++) {
	mob_vnum = number_range(50, 27000);

	if ((vsearch = get_mob_index(mob_vnum)) != NULL) {
	    if (quest_level_diff(ch->level, vsearch->level) == TRUE
		&& vsearch->pShop == NULL
		/*       && !IS_SET(vsearch->imm_flags, IMM_SUMMON) */
		&& !IS_SET(vsearch->act, ACT_TRAIN)
		&& !IS_SET(vsearch->act, ACT_PRACTICE)
		&& !IS_SET(vsearch->act, ACT_IS_HEALER)
		&& !IS_SET(vsearch->act, ACT_PET)
		&& !IS_SET(vsearch->act, ACT_IS_BANKER)
		&& !IS_SET(vsearch->affected_by, AFF_CHARM)
		&& !IS_SET(vsearch->affected_by, AFF_INVISIBLE)
	        && vsearch->level < 150
		&& chance(40))
		break;
	    else
		vsearch = NULL;
	}
    }
    sprintf(log_buf, "AQUEST LOG: generate_quest after for");
    log_string(log_buf);
    if (vsearch == NULL
	|| (victim = get_char_world(ch, vsearch->player_name)) == NULL) {
	sprintf(log_buf,
		"AQUEST LOG: generate_quest if victim = get_char_world");
	log_string(log_buf);
	sprintf(buf,
		"I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 1;
	return;
    }

    if ((room = find_location(ch, victim->name)) == NULL) {
	sprintf(log_buf,
		"AQUEST LOG: generate_quest room = find_location");
	log_string(log_buf);
	sprintf(buf,
		"I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 1;
	return;
    }

    /*  40% chance it will send the player on a 'recover item' quest. */

    if (chance(40)) {
	int objvnum = 0;

	sprintf(log_buf, "AQUEST LOG: generate_quest chance40");
	log_string(log_buf);

	switch (number_range(0, 4)) {
	case 0:
	    objvnum = QUEST_OBJQUEST1;
	    break;

	case 1:
	    objvnum = QUEST_OBJQUEST2;
	    break;

	case 2:
	    objvnum = QUEST_OBJQUEST3;
	    break;

	case 3:
	    objvnum = QUEST_OBJQUEST4;
	    break;

	case 4:
	    objvnum = QUEST_OBJQUEST5;
	    break;
	}
	sprintf(log_buf, "AQUEST LOG: generate_quest after switch");
	log_string(log_buf);
	questitem = create_object(get_obj_index(objvnum), ch->level);
	obj_to_room(questitem, room);
	questitem->timer = 50;
	ch->questobj = questitem->pIndexData->vnum;

	sprintf(buf,
		" %s Vile pilferers have stolen %s from the royal treasury!",
		ch->name, questitem->short_descr);
	do_whisper(questman, buf);
	sprintf(buf,
		"%s My court wizardess, with her magic mirror, has pinpointed its location.",
		ch->name);
	do_whisper(questman, buf);
	/*         I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "%s Look in the general area of %s for %s!", ch->name,
		room->area->name, room->name);
	do_whisper(questman, buf);
	sprintf(log_buf,
		"AQUEST LOG: generate_quest before end of obj section");
	log_string(log_buf);
	return;
    }

    /*     Quest to kill a mob   */

    else {
	switch (number_range(0, 1)) {
	case 0:
	    sprintf(buf,
		    "%s An enemy of mine, %s, is making vile threats against the crown.",
		    ch->name, victim->short_descr);
	    do_whisper(questman, buf);
	    sprintf(buf, "%s This threat must be eliminated!", ch->name);
	    do_whisper(questman, buf);
	    break;

	case 1:
	    sprintf(buf,
		    "%s Rune's most heinous criminal, %s, has escaped from the dungeon!",
		    ch->name, victim->short_descr);
	    do_whisper(questman, buf);
	    sprintf(buf,
		    "%s Since the escape, %s has murdered %d civillians!",
		    ch->name, victim->short_descr, number_range(2, 20));
	    do_whisper(questman, buf);
	    sprintf(buf,
		    "%s The penalty for this crime is death, and you are to deliver the sentence!",
		    ch->name);
	    do_whisper(questman, buf);
	    break;
	}
	sprintf(log_buf, "AQUEST LOG: generate_quest mob switch");
	log_string(log_buf);
	if (room->name != NULL) {
	    sprintf(log_buf,
		    "AQUEST LOG: generate_quest room->name != NULL");
	    log_string(log_buf);
	    sprintf(buf,
		    " %s Seek %s out somewhere in the vicinity of %s!",
		    ch->name, victim->short_descr, room->name);
	    do_whisper(questman, buf);

	    /* I changed my area names so that they have just the name of the area
	       and none of the level stuff. You may want to comment these next two
	       lines. - Vassago */

	    sprintf(buf, "%s That location is in the general area of %s.",
		    ch->name, room->area->name);
	    do_whisper(questman, buf);
	}
	ch->questmob = victim->pIndexData->vnum;
    }
    sprintf(log_buf, "AQUEST LOG: generate_quest before mob return");
    log_string(log_buf);
    return;
}

/* Level differences to search for. Moongate has 350
levels, so you will want to tweak these greater or
less than statements for yourself. - Vassago */

bool quest_level_diff(int clevel, int mlevel)
{
    if (clevel < 6 && mlevel < 10)
	return TRUE;
    else if (clevel > 5 && clevel < 10 && mlevel < 15)
	return TRUE;
    else if (clevel > 9 && clevel < 20 && mlevel > 25 && mlevel < 35)
	return TRUE;
    else if (clevel > 19 && clevel < 30 && mlevel > 35 && mlevel < 45)
	return TRUE;
    else if (clevel > 29 && clevel < 40 && mlevel > 45 && mlevel < 55)
	return TRUE;
    else if (clevel > 39 && clevel < 50 && mlevel > 55 && mlevel < 65)
	return TRUE;
    else if (clevel > 49 && clevel < 60 && mlevel > 65 && mlevel < 75)
	return TRUE;
    else if (clevel > 59 && clevel < 70 && mlevel > 75 && mlevel < 85)
	return TRUE;
    else if (clevel > 69 && clevel < 85 && mlevel > 95 && mlevel < 105)
	return TRUE;
    else if (clevel > 81 && clevel < 100 && mlevel > 105 && mlevel < 115)
	return TRUE;
    else
	return FALSE;
    /*    if (clevel < 6 && mlevel < 24) return TRUE;
       else if (clevel > 5 && clevel < 15 && mlevel < 20) return TRUE;
       else if (clevel > 14 && clevel < 40 && mlevel > 35 && mlevel < 45) return TRUE;
       else if (clevel > 39 && clevel < 65 && mlevel > 45 && mlevel < 75) return TRUE;
       else if (clevel > 64 && clevel < 91 && mlevel > 75 && mlevel < 101) return TRUE;
       else return FALSE; */
}

/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;

    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->character != NULL && d->connected == CON_PLAYING) {

	    ch = d->character;

	    if (ch->nextquest > 0) {
		ch->nextquest--;
		if (ch->nextquest == 0) {
		    send_to_char("You may now quest again.\n\r", ch);
		    return;
		}
	    } else if (IS_SET(ch->act, PLR_AUTOQST)) {
		if (--ch->countdown <= 0) {
		    char buf[MAX_STRING_LENGTH];

		    ch->nextquest = 6;
		    sprintf(buf,
			    "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r",
			    ch->nextquest);
		    send_to_char(buf, ch);
		    REMOVE_BIT(ch->act, PLR_AUTOQST);
		    ch->questgiver = NULL;
		    ch->countdown = 0;
		    ch->questmob = 0;
		}
		if (ch->countdown > 0 && ch->countdown < 6) {
		    send_to_char
			("Better hurry, you're almost out of time for your quest!\n\r",
			 ch);
		    return;
		}
	    }
	}
    }
    return;
}

/* Auto String by Scott Bjerstedt */
void do_astring(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char *pArg;
    char cEnd;

    OBJ_DATA *obj;
    CHAR_DATA *questman;

    smash_tilde(argument);
    /*
       argument = one_argument( argument, type );
       argument = one_argument( argument, arg1 );
       argument = one_argument( argument, arg2 );
       argument = one_argument( argument, arg3 );
       strcpy( arg4, argument ); */
    /*Can't use one_argument here, so I was inspired by the do_password code */
    /*1st Arg */
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';
    /*2nd ARG */
    pArg = arg2;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';
    /*3rd ARG */
    pArg = arg3;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';
    /*4th ARG */
    pArg = arg4;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if (IS_NPC(ch))
	return;

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0'
	|| arg4[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char
	    ("  aqstring <name> '<newname>' '<short desc>' '<long desc>'\n\r",
	     ch);
	send_to_char
	    ("   Type HELP AQSTRING if you havent used command before\n\r",
	     ch);
	send_to_char
	    (" FORBIDDEN NAMES OR PARTS OF NAMES ARE - quest point qp",
	     ch);
	return;
    }
    obj = get_obj_carry(ch, arg1);	/* does char have the item ? */

    if (obj == NULL) {
	send_to_char("You aren't carrying that.\n\r", ch);
	return;
    }
    for (questman = ch->in_room->people; questman != NULL;
	 questman = questman->next_in_room) {
	if (!IS_NPC(questman))
	    continue;
	if (questman->spec_fun == spec_lookup("spec_questmaster"))
	    break;
    }

    if (questman == NULL
	|| questman->spec_fun != spec_lookup("spec_questmaster")) {
	send_to_char("You can't do that here. Who's gonna do it?\n\r", ch);
	return;
    }

    if (questman->fighting != NULL) {
	send_to_char("Wait until the fighting stops.\n\r", ch);
	return;
    }

    if (ch->questpoints - 700 < 0) {
	do_say(questman, "You don't have enough questpoints to do that.");
	return;
    }
    if (strstr(arg2, "quest") || strstr(arg2, "point")
	|| strstr(arg2, "qp")) {
	send_to_char("You can't make quest points!!!", ch);
	return;
    }

    if (strstr(arg2, "-claneq-")) {
	send_to_char("You can't make clan eq!!!", ch);
	return;
    }

    sprintf(arg2, "%s, %s -astring-", arg2, obj->name);

    ch->questpoints = ch->questpoints - 700;

    act("The questmaster forges a new identity for your $p.",
	ch, obj, NULL, TO_CHAR);
    act("The questmaster forges a new identity for $n's $p.",
	ch, obj, NULL, TO_ROOM);
    /*Does Name */
    free_string(obj->name);
    obj->name = str_dup(arg2);
    /*Does short desc */

    free_string(obj->short_descr);
    obj->short_descr = str_dup(arg3);
    /*Does Long desc */


    free_string(obj->description);
    obj->description = str_dup(arg4);


    sprintf(buf, "Here is your %s brave %s, use it well.", obj->name,
	    ch->name);
    do_say(questman, buf);
}

void do_teststring(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char *pArg;
    char cEnd;

    OBJ_DATA *obj;

    smash_tilde(argument);
    /*
       argument = one_argument( argument, type );
       argument = one_argument( argument, arg1 );
       argument = one_argument( argument, arg2 );
       argument = one_argument( argument, arg3 );
       strcpy( arg4, argument ); */
    /*Can't use one_argument here, so I was inspired by the do_password code */
    /*1st Arg */
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';
    /*2nd ARG */
    pArg = arg2;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';
    /*3rd ARG */
    pArg = arg3;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';
    /*4th ARG */
    pArg = arg4;
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if (IS_NPC(ch))
	return;

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0'
	|| arg4[0] == '\0') {
	send_to_char("Syntax:\n\r", ch);
	send_to_char
	    ("  testaqstring <name> '<newname>' '<short desc>' '<long desc>'\n\r",
	     ch);
	send_to_char
	    ("   Type HELP AQSTRING if you havent used command before\n\r",
	     ch);
	send_to_char
	    (" FORBIDDEN NAMES OR PARTS OF NAMES ARE - quest point qp",
	     ch);
	return;
    }
    obj = get_obj_list(ch, arg1, ch->carrying);	/* does char have the item ? */

    if (obj == NULL) {
	send_to_char("You aren't carrying that.\n\r", ch);
	return;
    }

    if (strstr(arg2, "quest") || strstr(arg2, "point")
	|| strstr(arg2, "qp")) {
	send_to_char("You can't make quest points!!!", ch);
	return;
    }


    if (strstr(arg2, "-claneq-")) {
	send_to_char("You can't make clan eq!!!", ch);
	return;
    }

    sprintf(arg2, "%s, %s -astring-", arg2, obj->name);

    sprintf(buf, "The `^A`6string`` `&V``alues would be:\n\r");
    sprintf(buf,
	    "%s Name: %s\n\r Short Desc: %s\n\r Long Description: %s\n\r",
	    buf, arg2, arg3, arg4);
    send_to_char(buf, ch);
}

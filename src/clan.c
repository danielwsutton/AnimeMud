/* This file contains all commands revalent to the clan system.
 * Belldandy
 */

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
#include <stdarg.h>
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "lookup.h"
#include "interp.h"

/* Command Procedures needed */
DECLARE_DO_FUN(do_cskill);
DECLARE_DO_FUN(do_cload);
DECLARE_DO_FUN(do_clantalk);

bool clan_can_see(CHAR_DATA * ch, CHAR_DATA * victim);


void do_guild(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH], item[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int clan;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
	send_to_char("Syntax: guild <char> <cln name>\n\r", ch);
	return;
    } else if (!IS_ADMIN(ch) && !IS_SET(ch->act, PLR_LEADER)) {
	send_to_char("That command is reserved only for guildleaders.\n\r",
		     ch);
	return;
    } else if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("They aren't playing.\n\r", ch);
	return;
    } else if (IS_ADMIN(ch) && !IS_IMP(ch) && (victim != ch)) {
	send_to_char("You may only (un)guild yourself.\n\r", ch);
	return;
    } else if (IS_NPC(victim)) {
	send_to_char("You cannot guild a NPC!\n\r", ch);
	return;
    } else if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("Whew! No guilding while charmed.\n\r", ch);
	return;
    }

    if (!str_prefix(arg2, "none")) {
	if (!IS_ADMIN(ch) && (victim == ch)) {
	    send_to_char("You can't unguild yourself.\n\r", ch);
	    return;
	} else if (!IS_IMP(ch) && (ch->clan != victim->clan)) {
	    send_to_char("You can't unguild someone not in your clan!\n\r",
			 ch);
	    return;
	} else if (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim)) {
	    send_to_char("You can't unguild an immortal.\n\r", ch);
	    return;
	}

	send_to_char("They are now clanless.\n\r", ch);
	send_to_char("You are no longer a member of a clan!\n\r", victim);
	victim->clan = 0;
	sprintf(item, "-claneq-");

	if (!IS_IMMORTAL(victim)) {
	    if (victim->clan == clan_lookup("108 dragons"))
		group_remove(victim, "foot blade");
	    else if (victim->clan == clan_lookup("Judge"))
		group_remove(victim, "judgement");
	    else if (victim->clan == clan_lookup("Vampire Hunt"))
		group_remove(victim, "immolation");
	    else if (victim->clan == clan_lookup("Mds"))
		group_remove(victim, "fightech mode");
	    else if (victim->clan == clan_lookup("Shogun"))
		group_remove(victim, "reincarnation");
	    else if (victim->clan == clan_lookup("Sailor")) {
		group_remove(victim, "sailor suit");
		group_remove(victim, "sailor burst");
	    } else if (victim->clan == clan_lookup("Invid"))
		group_remove(victim, "invid battle armor");
	    else if (victim->clan == clan_lookup("Shinobi"))
		group_remove(victim, "shinobi rage");
	    else if (victim->clan == clan_lookup("Dragon Slayer"))
		group_remove(victim, "mantaux");
	    else if (victim->clan == clan_lookup("Kenshin"))
		group_remove(victim, "hiten-mitsu...");

	    sprintf(buf, "CSKILL: taken from %s.", victim->name);
	    wiznet(buf, ch, NULL, WIZ_FLAGS, 0, get_trust(ch));

	    if (victim != ch) {
		while ((obj = get_obj_list(ch, item, victim->carrying)) !=
		       NULL) {
		    if (obj->wear_loc != WEAR_NONE)
			unequip_char(victim, obj);

		    obj_from_char(obj);
		    obj_to_char(obj, ch);

		    sprintf(buf,
			    "With a brutal gesture, %s flys into your hands from %s.\r\n",
			    obj->short_descr, victim->name);
		    send_to_char(buf, ch);
		    sprintf(buf,
			    "Abruptly %s flies from your possession.\r\n",
			    obj->short_descr);
		    send_to_char(buf, victim);
		    sprintf(buf, "CLAN: %s seizes %s from %s.", ch->name,
			    obj->short_descr, victim->name);
		    wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
		}
	    }
	}

	return;
    }

    if (!(clan = clan_lookup(arg2))) {
	send_to_char("No such clan exists.\n\r", ch);
	return;
    } else if ((clan != ch->clan) && !IS_IMP(ch)) {
	send_to_char("You are not the leader of that clan.\n\r", ch);
	return;
    }
	else if (victim->pcdata->pkset == FALSE
		 && ch->clan != clan_lookup("Jurai")) {
	send_to_char("You can't guild a `1N`1o`1n``-PK person.\n\r", ch);
	return;
	// TG981111 - taken out to prevent ldrs from guilding non-pk
	// having too many probs with ldrs doing this before they get deleted
	// or get 'hacked'
	/*victim->pcdata->pkset = TRUE;
	   victim->max_hit = victim->max_hit + 100;
	   victim->max_mana = victim->max_mana + 100;
	   victim->max_move = victim->max_move + 100;

	   victim->pcdata->perm_hit = victim->pcdata->perm_hit + 100;
	   victim->pcdata->perm_mana = victim->pcdata->perm_mana + 100;
	   victim->pcdata->perm_move = victim->pcdata->perm_move + 100;
	   send_to_char("You are now able to kill other players!\n\r",victim); */
    } else if (victim->clan != 0) {
	send_to_char("They are already in a clan!\n\r", ch);
	return;
    }

    if (clan_table[clan].independent) {
	sprintf(buf, "They are now a %s.\n\r", clan_table[clan].name);
	send_to_char(buf, ch);

	sprintf(buf, "You are now a %s.\n\r", clan_table[clan].name);
	send_to_char(buf, victim);
    } else {
	sprintf(buf, "They are now a member of clan %s.\n\r",
		capitalize(clan_table[clan].name));
	send_to_char(buf, ch);
	sprintf(buf, "You are now a member of clan %s.\n\r",
		capitalize(clan_table[clan].name));
	send_to_char(buf, victim);
    }

    victim->clan = clan;
}


void do_clan(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH],
	buf1[2 * MAX_INPUT_LENGTH],
	buf2[2 * MAX_INPUT_LENGTH], questbuf[25], *clanbuf;
    DESCRIPTOR_DATA *d;
    BUFFER *output;
    int clan, nMatch = 0;

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0') {
	if (!is_clan(ch) || clan_table[ch->clan].independent)
	    send_to_char("Syntax: clan  In_charge | list \n\r", ch);
	else
	    send_to_char
		("Syntax: clan recall | who | talk | in_charge | list\n\r",
		 ch);
	return;
    }

    buf1[0] = '\0';
    buf2[0] = '\0';

    if (!str_prefix(arg1, "in_charge")) {
	output = new_buf();

	add_buf(output,
		"....-------------        Clan Leaders        ----------....\n\r");

	for (clan = 0; clan_leaders[clan].clan_name != NULL; clan++) {
	    sprintf(buf1, " %s %s \n\r", clan_leaders[clan].clan_name,
		    clan_leaders[clan].leader_name);
	    add_buf(output, buf1);
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);
	return;
    } else if (!str_prefix(arg1, "list")) {
	do_help(ch, "clan list");
	return;
    } else if (ch->clan <= 0) {
	send_to_char("Huh?\n\r", ch);
	return;
    } else if (!str_prefix(arg1, "recall")) {
	do_recall(ch, "clan");
	return;
    } else if (!str_prefix(arg1, "who")) {
	output = new_buf();

	sprintf(buf1, "%s`6Clan Status`A:\n\r\
			`^=====================================================``\n\r", clan_table[ch->clan].who_name);
	add_buf(output, buf1);

	for (d = descriptor_list; d != NULL; d = d->next) {
	    CHAR_DATA *wch;
	    char const *class;

	    if ((d->connected != CON_PLAYING)
		|| (!clan_can_see(ch, d->character)))
		continue;

	    wch = (d->original != NULL) ? d->original : d->character;

	    if (wch->clan != ch->clan)
		continue;

	    nMatch++;

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

	    if (IS_QUESTOR(wch) && wch->pcdata->questgiver != NULL) {
		if (IS_QUESTPKMST(wch->pcdata->questgiver))
		    strcpy(questbuf, "`8(`!PK`1Qst`8)`` ");
		else
		    strcpy(questbuf, "`&(`4Q`$u`^e`&s`^t`$o`4r`&)`` ");
	    } else if (IS_QUESTMST(wch)) {
		if (IS_QUESTPKMST(wch))
		    strcpy(questbuf, "`8(`!PK`1Ldr`8)`` ");
		else
		    strcpy(questbuf, "`&(`4Q`$s`^t`&l`^d`$e`4r`&)`` ");
	    } else
		strcpy(questbuf, "");

	    switch (wch->pcdata->clan_rank) {
	    default:
		clanbuf = "";
		break;
	    case 0:
		clanbuf = "`2(`@Pon`2)`` ";
		break;
	    case 1:
		clanbuf = "`2(`@KP`2)`` ";
		break;
	    case 2:
		clanbuf = "`2(`@9th`2)`` ";
		break;
	    case 3:
		clanbuf = "`2(`@8th`2)`` ";
		break;
	    case 4:
		clanbuf = "`2(`@7th`2)`` ";
		break;
	    case 5:
		clanbuf = "`2(`@6th`2)`` ";
		break;
	    case 6:
		clanbuf = "`2(`@5th`2)`` ";
		break;
	    case 7:
		clanbuf = "`2(`@4th`2)`` ";
		break;
	    case 8:
		clanbuf = "`2(`@3rd`2)`` ";
		break;
	    case 9:
		clanbuf = "`2(`@2nd`2)`` ";
		break;
	    case 10:
		clanbuf = "`2(`@1st`2)`` ";
		break;
	    case 11:
		clanbuf = "`2(`@Imm`2)`` ";
		break;
	    }

	    if (wch->level <= MAX_LEVEL) {
		if (wch->sex == 1)
		    add_buf(output, "`8[`#M``");
		else if (wch->sex == 2)
		    add_buf(output, "`8[`!F``");
		else
		    add_buf(output, "`8[`%?``");
		add_buf(output, "(");
		add_buf(output,
			IS_AFFECTED(wch,
				    AFF_INVISIBLE) ? "`@I`&|``" :
			" `&|``");
		add_buf(output,
			IS_AFFECTED(wch, AFF_HIDE) ? "`@H``" : " ");
		add_buf(output,
			IS_AFFECTED(wch,
				    AFF_SNEAK) ? "`&|`@S``" : "`&|`` ");
		add_buf(output, ")");

		if (wch->level <= LEVEL_IMMORTAL) {
		    sprintf(buf1, "`8[`&%3d `6%4s `@%3s`8]`` ",
			    wch->level,
			    wch->race <
			    MAX_PC_RACE ? pc_race_table[wch->race].
			    who_name : "     ", class);
		    add_buf(output, buf1);
		} else if (wch->level <= MAX_LEVEL) {
		    sprintf(buf1, "`8[``%s`8]`` ",
			    wch->pcdata->bracket_title);
		    add_buf(output, buf1);
		} else {
		    sprintf(buf1, "[%3d %4s %3s] ",
			    wch->level,
			    wch->race <
			    MAX_PC_RACE ? pc_race_table[wch->race].
			    who_name : "     ", class);
		    add_buf(output, buf1);
		}
	    }


	    /*          1 2 3 4 5 6 7 8 9 */
	    sprintf(buf1, "%s%s%s%s%s%s%s%s%s\n\r",
		    questbuf,
		    wch->incog_level >= 1 ? "`6(`^Incog`6)`` " : "",
		    wch->invis_level >= 1 ? "(`&W``i`&z``i) " : "",
		    clanbuf,
		    IS_SET(wch->comm, COMM_AFK) ? "[`1A`!F`1K``] " : "",
		    IS_SET(wch->act,
			   PLR_KILLER) ? "(`1K`@I`&LL`@E`2R``) " : "",
		    IS_SET(wch->act,
			   PLR_THIEF) ? "(`3T`#H`&I`#E`3f``) " : "",
		    wch->name,
		    clan_who_title[wch->pcdata->clan_rank].who_title);

	    add_buf(output, buf1);
	}

	sprintf(buf2, "\n\r`&C``lan Members found`8:`` `@%d``\n\r",
		nMatch);
	add_buf(output, buf2);

	page_to_char(buf_string(output), ch);
	free_buf(output);
	return;
    } else if (!str_prefix(arg1, "ranks")) {
	send_to_char("Ranks being developed.\n\r", ch);
	return;
    } else if (!str_prefix(arg1, "talk")) {
	do_clantalk(ch, argument);
	return;
    } else {
	send_to_char("Huh?\n\r", ch);
	return;
    }
}


void do_cload(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    int iVnumCount = 0, i, iArrVnums[5];

    one_argument(argument, arg1);
    pObjIndex = 0;

    if (!is_granted_name(ch, "cload")) {
	send_to_char("Huh?\n\r", ch);
	return;
    } else if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("Whew, No loading while charmed.\n\r", ch);
	return;
    } else if (arg1[0] == '\0') {
	send_to_char("Syntax:  cload <victim>\n\r", ch);
	return;
    }

    if (IS_ADMIN(ch))
	victim = get_char_world(ch, arg1);
    else
	victim = get_char_room(ch, arg1);

    if (victim == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    } else if ((victim->clan != ch->clan) && !IS_IMP(ch)) {
	send_to_char("ACK! You can't give a non-member clan eq!\n\r", ch);
	return;
    } else if (!victim->clan) {
	sprintf(buf, "%s is a member of no clan.\n\r", victim->name);
	send_to_char(buf, ch);
	return;
    }

    if ((victim == ch) && IS_SET(ch->act, PLR_LEADER)) {
	pObjIndex = get_obj_index(12105);
	obj = create_object(pObjIndex, 0);
	obj_to_char(obj, ch);
	act("You have brought into existance $p.", ch, obj, NULL, TO_CHAR);
	act("$n has brought into existance $p.", ch, obj, NULL, TO_ROOM);
    }


    if (victim->clan == clan_lookup("108 dragons")) {
	iArrVnums[iVnumCount++] = 12110;
	iArrVnums[iVnumCount++] = 12111;
	iArrVnums[iVnumCount++] = 12112;
    } else if (victim->clan == clan_lookup("invid")) {
	iArrVnums[iVnumCount++] = 12113;
	iArrVnums[iVnumCount++] = 12114;
	iArrVnums[iVnumCount++] = 12115;
    } else if (victim->clan == clan_lookup("judge")) {
	iArrVnums[iVnumCount++] = 12116;
	iArrVnums[iVnumCount++] = 12117;
	iArrVnums[iVnumCount++] = 12118;
    } else if (victim->clan == clan_lookup("mds")) {
	iArrVnums[iVnumCount++] = 12119;
	iArrVnums[iVnumCount++] = 12120;
	iArrVnums[iVnumCount++] = 12121;
    } else if (victim->clan == clan_lookup("shinobi")) {
	iArrVnums[iVnumCount++] = 12122;
	iArrVnums[iVnumCount++] = 12123;
	iArrVnums[iVnumCount++] = 12124;
    } else if (victim->clan == clan_lookup("shogun")) {
	iArrVnums[iVnumCount++] = 12125;
	iArrVnums[iVnumCount++] = 12126;
	iArrVnums[iVnumCount++] = 12127;
    } else if (victim->clan == clan_lookup("vampire hunt")) {
	iArrVnums[iVnumCount++] = 12128;
	iArrVnums[iVnumCount++] = 12129;
	iArrVnums[iVnumCount++] = 12130;
    } else if (victim->clan == clan_lookup("wiseone")) {
	iArrVnums[iVnumCount++] = 12131;
	iArrVnums[iVnumCount++] = 12132;
	iArrVnums[iVnumCount++] = 12133;
    } else if (victim->clan == clan_lookup("sailor")) {
	iArrVnums[iVnumCount++] = 12151;
	iArrVnums[iVnumCount++] = 12152;
	iArrVnums[iVnumCount++] = 12153;
    } else if (victim->clan == clan_lookup("hunter-warrior")) {
	iArrVnums[iVnumCount++] = 12154;
	iArrVnums[iVnumCount++] = 12155;
	iArrVnums[iVnumCount++] = 12156;
    } else if (victim->clan == clan_lookup("devil hunter")) {
	iArrVnums[iVnumCount++] = 31;
	iArrVnums[iVnumCount++] = 12158;
	iArrVnums[iVnumCount++] = 12159;
    } else if (victim->clan == clan_lookup("dragon slayers")) {
	iArrVnums[iVnumCount++] = 12160;
	iArrVnums[iVnumCount++] = 12161;
	iArrVnums[iVnumCount++] = 12162;
    }

    for (i = 0; i < iVnumCount; i++) {
	pObjIndex = get_obj_index(iArrVnums[i]);

	if ((obj = create_object(pObjIndex, 0)) != NULL) {
	    obj_to_char(obj, victim);
	    sprintf(buf, "CLOAD: $N to %s.", victim->name);
	    wiznet(buf, ch, NULL, WIZ_FLAGS, 0, get_trust(ch));

	    if (victim != ch) {
		act("You give $p to $N.", ch, obj, victim, TO_CHAR);
		act("$n gives you $p.", ch, obj, victim, TO_VICT);
		act("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
	    } else {
		act("You give yourself $p.", ch, obj, NULL, TO_CHAR);
		act("$n gives $mself $p.", ch, obj, victim, TO_NOTVICT);
	    }
	}
    }

    return;
}


void do_cskill(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int sn;

    one_argument(argument, arg1);

    if (IS_NPC(ch))
	return;

    if (!is_granted_name(ch, "cskill")) {
	send_to_char("Huh?\n\r", ch);
	return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("You cannot use this command while charmed.\n\r", ch);
	return;
    }
    if (arg1[0] == '\0') {
	send_to_char("Syntax:  cskill <victim> grant:ungrant\n\r", ch);
	return;
    }
    if ((victim = get_char_room(ch, arg1)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    if ((victim->clan != ch->clan) && get_trust(ch) < MAX_LEVEL) {
	send_to_char("ACK! They are not even in a clan!!!\n\r", ch);
	return;
    }

    if (ch->clan == clan_lookup("108 dragons")
	&& victim->clan == clan_lookup("108 dragons")) {
	group_add(victim, "foot blade", FALSE);
	victim->pcdata->learned[gsn_footblade] = 100;
    } else if (ch->clan == clan_lookup("Judge")
	       && victim->clan == clan_lookup("Judge")) {
	sn = skill_lookup("judgement");
	group_add(victim, "judgement", FALSE);
	victim->pcdata->learned[sn] = 100;
    } else if (ch->clan == clan_lookup("Vampire Hunt")
	       && victim->clan == clan_lookup("Vampire Hunt")) {
	sn = skill_lookup("immolation");
	group_add(victim, "immolation", FALSE);
	victim->pcdata->learned[sn] = 100;
    } else if (ch->clan == clan_lookup("Mds")
	       && victim->clan == clan_lookup("Mds")) {
	group_add(victim, "fightech mode", FALSE);
	victim->pcdata->learned[gsn_fightech] = 100;
    } else if (ch->clan == clan_lookup("Shogun")
	       && victim->clan == clan_lookup("Shogun")) {
	group_add(victim, "reincarnation", FALSE);
	victim->pcdata->learned[gsn_reincarnation] = 100;
    } else if (ch->clan == clan_lookup("Sailor")
	       && victim->clan == clan_lookup("Sailor")) {
	group_add(victim, "sailor suit", FALSE);
	victim->pcdata->learned[gsn_sailor_suit] = 100;
	group_add(victim, "sailor burst", FALSE);
	victim->pcdata->learned[gsn_sailor_burst] = 100;
    } else if (ch->clan == clan_lookup("Invid")
	       && victim->clan == clan_lookup("Invid")) {
	group_add(victim, "invid battle armor", FALSE);
	victim->pcdata->learned[gsn_battle_armor] = 100;
    } else if (ch->clan == clan_lookup("Shinobi")
	       && victim->clan == clan_lookup("Shinobi")) {
	group_add(victim, "shinobi rage", FALSE);
	victim->pcdata->learned[gsn_rage] = 100;
    } else if (ch->clan == clan_lookup("Dragon Slayer")
	       && victim->clan == clan_lookup("Dragon Slayer")) {
	group_add(victim, "mantaux", FALSE);
	victim->pcdata->learned[gsn_mantaux] = 100;
    } else if (ch->clan == clan_lookup("Kenshin")
	       && victim->clan == clan_lookup("Kenshin")) {
	group_add(victim, "hiten-mitsu...", FALSE);
	victim->pcdata->learned[gsn_hitenmitsuryugyryu] = 100;
    } else {
	send_to_char("That isn't a valid option.\n\r", ch);
	return;
    }
    sprintf(buf, "CSKILL: $N to %s.", victim->name);
    wiznet(buf, ch, NULL, WIZ_FLAGS, 0, get_trust(ch));
    return;
}


/*TG970914*/
void do_promote(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int arg2int;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    arg2int = atoi(arg2);

    if (IS_NPC(ch)) {
	send_to_char("NPC's cannot promote someone.\n\r", ch);
	return;
    }

    if ((ch->pcdata->clan_rank < 9) && (!IS_IMMORTAL(ch))) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if ((arg1[0] == '\0') || (arg2[0] == '\0') || (arg2int < 0)
	|| (arg2int > 11)) {
	send_to_char("Syntax: promote <char> <rank 0-10>\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg1)) == NULL) {
	send_to_char("They must be present to be promoted.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("You must be mad!\n\r", ch);
	return;
    }

    if (victim->clan < 1) {
	send_to_char("They cannot be promoted.\n\r", ch);
	return;
    }

    if (victim == ch) {
	if ((IS_IMMORTAL(ch)) || (get_trust(ch) >= 95)) {
	    ch->pcdata->clan_rank = arg2int;
	    send_to_char("Rank ceremony complete.\n\r", ch);
	    return;
	}
	send_to_char("You cannot promote yourself.\n\r", ch);
	return;
    }

    if ((IS_IMMORTAL(ch)) && (ch->level < 99) && (get_trust(ch) < 99)) {
	send_to_char("You can only promote yourself.\n\r", ch);
	return;
    }

    if ((ch->level < 99) && (get_trust(ch) < 99)) {
	if (ch->clan != victim->clan) {
	    send_to_char("They are not in your clan.\n\r", ch);
	    return;
	}
	if (ch->pcdata->clan_rank <= arg2int) {
	    send_to_char("You cannot promote over your rank.\n\r", ch);
	    return;
	}
	if (ch->pcdata->clan_rank < victim->pcdata->clan_rank) {
	    send_to_char("You cannot promote someone higher than you.\n\r",
			 ch);
	    return;
	}

	victim->pcdata->clan_rank = arg2int;
	send_to_char("Rank ceremony complete.\n\r", ch);
	send_to_char("Rank ceremony complete.\n\r", victim);
	return;
    }

    if ((arg2int > 10) && (!IS_IMMORTAL(victim))) {
	send_to_char("They are not immortal!\n\r", ch);
	return;
    }

    if (((arg2int > 9) && (!IS_SET(victim->act, PLR_LEADER)))
	&& (!IS_IMMORTAL(victim))) {
	send_to_char("They are not qualified to lead.\n\r", ch);
	return;
    }

    victim->pcdata->clan_rank = arg2int;

    send_to_char("Rank ceremony complete.\n\r", ch);
    send_to_char("Rank ceremony complete.\n\r", victim);

}

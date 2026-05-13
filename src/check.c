/* This file created by Belldandy in order to make file downloading and uploading a bit
 * more compact... no more huge fight.c.
 *
 * This file contains all the check procedures used by fight.c
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

extern char *color_table[];

bool is_safe args((CHAR_DATA * ch, CHAR_DATA * victim));
void check_assist args((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge args((CHAR_DATA * ch, CHAR_DATA * victim));
void check_killer args((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_parry args((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_shield_block args((CHAR_DATA * ch, CHAR_DATA * victim));

/*BO980717 */
bool check_counter
args((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt));
bool check_phase args((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_rage args((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_reincarnation args((CHAR_DATA * ch));


bool is_safe(CHAR_DATA * ch, CHAR_DATA * victim)
{
    bool pkquest = FALSE;

    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level >= LEVEL_SECURITY)
	return FALSE;

    /*EE960526 */
    if (IS_QUESTOR(ch) && (ch->pcdata->questgiver != NULL)
	&& IS_QUESTOR(victim) && (victim->pcdata->questgiver != NULL)
	&& IS_QUESTPKMST(ch->pcdata->questgiver)
	&& IS_QUESTPKMST(victim->pcdata->questgiver))
	pkquest = TRUE;
    /* killing mobiles */
    if (IS_NPC(victim)) {

	/* safe room? */
	if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
	    send_to_char
		("The gods foresake you and you can't lift your arms to fight.\n\r",
		 ch);
	    return TRUE;
	}

	if (victim->pIndexData->pShop != NULL) {	/* BO060298 */
	    send_to_char("The shopkeeper wouldn't like that.\n\r", ch);
	    return TRUE;
	}

	if (IS_SET(victim->act, ACT_STABLED_HORSE)
	    && IS_SET(victim->act, ACT_HORSE)) {
	    send_to_char
		("No, that creature is safe from harm at the moment.", ch);
	    return TRUE;
	}

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act, ACT_TRAIN)
	    || IS_SET(victim->act, ACT_PRACTICE)
	    || IS_SET(victim->act, ACT_IS_HEALER)
	    || IS_SET(victim->act, ACT_IS_CHANGER)
	    || IS_SET(victim->act, ACT_IS_BANKER)) {
	    send_to_char("I don't think the Gods would approve.\n\r", ch);
	    return TRUE;
	}

	if (!IS_NPC(ch)) {

	    /* no pets */
	    if (IS_SET(victim->act, ACT_PET)) {
		act("But $N looks so cute and cuddly...",
		    ch, NULL, victim, TO_CHAR);
		return TRUE;
	    }

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim, AFF_CHARM) && ch != victim->master) {
		send_to_char("You don't own that monster.\n\r", ch);
		return TRUE;
	    }
	}

	/* killing players */
    } else {
	/* killer and thief check */
	if (IS_SET(victim->act, PLR_KILLER)
	    || IS_SET(victim->act, PLR_THIEF))
	    return FALSE;

	/* NPC doing the killing */
	if (IS_NPC(ch)) {
	    /* safe room check */
	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
		send_to_char("Not in this room.\n\r", ch);
		return TRUE;
	    }

	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL
		&& ch->master->fighting != victim) {
		send_to_char("Players are your friends!\n\r", ch);
		return TRUE;
	    }

	    /* player doing the killing */
	} else {
	    if (IS_IMMORTAL(ch))
		return FALSE;

	    /* safe room check */
	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
		send_to_char("Not in this room.\n\r", ch);
		return TRUE;
	    }

	    if (victim->desc == NULL && victim->fight <= 0) {
		act("The gods don't allow killing of LD players.", ch,
		    NULL, victim, TO_CHAR);
		return TRUE;
	    }

	    if ((ch->pcdata->logintimer != 0) && !pkquest) {
		send_to_char
		    ("You must wait until you've spent enough time logged on.\n\r",
		     ch);
		return TRUE;
	    }

	    if (!ch->pcdata->pkset && !pkquest) {	/*EE960410 */
		send_to_char
		    ("You must go pk if you want to kill players.\n\r",
		     ch);
		return TRUE;
	    }

	    if (!victim->pcdata->pkset && !pkquest) {	/*EE960411 */
		send_to_char
		    ("I am afraid your victim hasn't chosen the way of a pkiller.\n\r",
		     ch);
		return TRUE;
	    }

	    if ((IS_QUESTOR(ch) || IS_QUESTOR(victim)) && !pkquest) {	/*EE960522 */
		send_to_char
		    ("Killing during a quest is forbidden by the Gods!\n\r",
		     ch);
		return TRUE;
	    }

	    if (IS_SET(victim->act, PLR_KILLER)
		|| IS_SET(victim->act, PLR_THIEF))
		return FALSE;

	    if (ch->level > victim->level + PK_LEVEL
		&& (!IS_QUESTOR(ch) || !IS_QUESTOR(victim))) {
		send_to_char("Pick on someone your own size.\n\r", ch);
		return TRUE;
	    }

	    if (ch->level < victim->level - PK_LEVEL
		&& (!IS_QUESTOR(ch) || !IS_QUESTOR(victim))) {
		send_to_char("Attempt that when your a bit older.\n\r",
			     ch);
		return TRUE;
	    }
	}
    }

    return FALSE;
}

bool is_safe_song(CHAR_DATA * ch, CHAR_DATA * victim, bool area)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area)
	return FALSE;

    /* killing mobiles */
    if (IS_NPC(victim)) {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags, ROOM_SAFE))
	    return TRUE;

	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act, ACT_TRAIN)
	    || IS_SET(victim->act, ACT_PRACTICE)
	    || IS_SET(victim->act, ACT_IS_HEALER)
	    || IS_SET(victim->act, ACT_IS_CHANGER)
	    || IS_SET(victim->act, ACT_IS_BANKER))
	    return TRUE;

	if (!IS_NPC(ch)) {
	    /* no pets */
	    if (IS_SET(victim->act, ACT_PET))
		return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim, AFF_CHARM)
		&& (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL
		&& !is_same_group(ch, victim->fighting)) return TRUE;
	} else {
	    /* area effect songs do not hit other mobs */
	    if (area && !is_same_group(victim, ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else {
	if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch)) {
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL
		&& ch->master->fighting != victim)
		return TRUE;

	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent */
	    if (ch->fighting != NULL
		&& !is_same_group(ch->fighting, victim)) return TRUE;
	}

	/* player doing the killing */
	else {
	    if (!is_clan(ch))
		return TRUE;

	    if (IS_SET(victim->act, PLR_KILLER)
		|| IS_SET(victim->act, PLR_THIEF))
		return FALSE;

	    if (!is_clan(victim))
		return TRUE;

	    if (ch->level > victim->level + 8)
		return TRUE;
	}

    }
    return FALSE;
}

bool is_safe_spell(CHAR_DATA * ch, CHAR_DATA * victim, bool area)
{
    bool pkquest = FALSE;

    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area)
	return FALSE;

    /* is_safe had this check, while is_safe_spell did not - Kevin */
    if (IS_QUESTOR(ch) && (ch->pcdata->questgiver != NULL)
	&& IS_QUESTOR(victim) && (victim->pcdata->questgiver != NULL)
	&& IS_QUESTPKMST(ch->pcdata->questgiver)
	&& IS_QUESTPKMST(victim->pcdata->questgiver))
	pkquest = TRUE;

    /* killing mobiles */
    if (IS_NPC(victim)) {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags, ROOM_SAFE))
	    return TRUE;

	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act, ACT_TRAIN)
	    || IS_SET(victim->act, ACT_PRACTICE)
	    || IS_SET(victim->act, ACT_IS_HEALER)
	    || IS_SET(victim->act, ACT_IS_CHANGER)
	    || IS_SET(victim->act, ACT_IS_BANKER))
	    return TRUE;

	if (!IS_NPC(ch)) {
	    /* no pets */
	    if (IS_SET(victim->act, ACT_PET))
		return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim, AFF_CHARM)
		&& (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL
		&& !is_same_group(ch, victim->fighting)) return TRUE;
	} else {
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim, ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else {
	if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch)) {
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL
		&& ch->master->fighting != victim)
		return TRUE;

	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent */
	    if (ch->fighting != NULL
		&& !is_same_group(ch->fighting, victim)) return TRUE;
	}

	/* player doing the killing */
	else {
/*
			if (!is_clan(ch))
				return TRUE;
			if (IS_SET(victim->act,PLR_KILLER) || IS_SET(victim->act,PLR_THIEF))
				return FALSE;
			
			if (!is_clan(victim))
				return TRUE;
			
			if (ch->level > victim->level + 8)
				return TRUE;
*/
	    if (IS_IMMORTAL(ch))
		return FALSE;

	    /* safe room check */
	    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
		send_to_char("Not in this room.\n\r", ch);
		return TRUE;
	    }

	    if (victim->desc == NULL && victim->fight <= 0) {
		act("The gods don't allow killing of LD players.", ch,
		    NULL, victim, TO_CHAR);
		return TRUE;
	    }

	    if ((ch->pcdata->logintimer != 0) && !pkquest) {
		send_to_char
		    ("You must wait until you've spent enough time logged on.\n\r",
		     ch);
		return TRUE;
	    }

	    if (!ch->pcdata->pkset && !pkquest) {	/*EE960410 */
		send_to_char
		    ("You must go pk if you want to kill players.\n\r",
		     ch);
		return TRUE;
	    }

	    if (!victim->pcdata->pkset && !pkquest) {	/*EE960411 */
		send_to_char
		    ("I am afraid your victim hasn't chosen the way of a pkiller.\n\r",
		     ch);
		return TRUE;
	    }

	    if ((IS_QUESTOR(ch) || IS_QUESTOR(victim)) && !pkquest) {	/*EE960522 */
		send_to_char
		    ("Killing during a quest is forbidden by the Gods!\n\r",
		     ch);
		return TRUE;
	    }

	    if (IS_SET(victim->act, PLR_KILLER)
		|| IS_SET(victim->act, PLR_THIEF))
		return FALSE;

	    if (ch->level > victim->level + PK_LEVEL
		&& (!IS_QUESTOR(ch) || !IS_QUESTOR(victim))) {
		send_to_char("Pick on someone your own size.\n\r", ch);
		return TRUE;
	    }

	    if (ch->level < victim->level - PK_LEVEL
		&& (!IS_QUESTOR(ch) || !IS_QUESTOR(victim))) {
		send_to_char("Attempt that when your a bit older.\n\r",
			     ch);
		return TRUE;
	    }
	}

    }
    return FALSE;
}

/*
* See if an attack justifies a KILLER flag.
*/
void check_killer(CHAR_DATA * ch, CHAR_DATA * victim)
{
    char buf[MAX_STRING_LENGTH];
    int check;

    if (ch->fight != 0 && victim->fight != 0)
	check = 1;
    else
	check = 0;

    if (!IS_NPC(ch) && !IS_NPC(victim)
	&& !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
	ch->fight = 270;
	victim->fight = 270;
	sprintf(buf, "$N is attempting to PK %s", victim->name);
	wiznet(buf, ch, NULL, WIZ_PK, 0, 0);
    }

    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL)
	victim = victim->master;

    REMOVE_BIT(ch->affected_by, AFF_HIDE);
    REMOVE_BIT(ch->affected_by, AFF_SNEAK);	/*EE960801 */

    /*
       * NPC's are fair game.
       * So are killers and thieves.
     */
    if (IS_NPC(victim)
	|| IS_SET(victim->act, PLR_KILLER)
	|| IS_SET(victim->act, PLR_THIEF)
	|| (ch->level < victim->level - 7 || check != 0))
	return;

    /*
       * Charm-o-rama.
     */
    if (IS_SET(ch->affected_by, AFF_CHARM)) {
	if (ch->master == NULL) {
	    char buf[MAX_STRING_LENGTH];

	    sprintf(buf, "Check_killer: %s bad AFF_CHARM",
		    IS_NPC(ch) ? ch->short_descr : ch->name);
	    bug(buf, 0);
	    affect_strip(ch, gsn_charm_person);
	    REMOVE_BIT(ch->affected_by, AFF_CHARM);
	    return;
	}
	/*
	   send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
	   SET_BIT(ch->master->act, PLR_KILLER);
	 */

	stop_follower(ch);
	return;
    }

    /*
       * NPC's are cool of course (as long as not charmed).
       * Hitting yourself is cool too (bleeding).
       * So is being immortal (Alander's idea).
       * Current killers stay as they are.
       * Also, no killer flags during quest.
     */

    if (IS_NPC(ch)
	|| ch == victim
	|| ch->level >= LEVEL_IMMORTAL
	|| ((ch->level - PK_LEVEL) <= (victim->level))
	|| IS_SET(ch->act, PLR_KILLER)
	|| (IS_QUESTOR(ch) && IS_QUESTOR(victim))) return;

    send_to_char("*** You are now a KILLER!! ***\n\r", ch);
    SET_BIT(ch->act, PLR_KILLER);
    sprintf(buf, "$N is attempting to OOL %s", victim->name);
    wiznet(buf, ch, NULL, WIZ_FLAGS, 0, 0);
    save_char_obj(ch);
    return;
}



/*
* Check for parry.
*/
bool check_parry(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE(victim))
	return FALSE;

    chance = get_skill(victim, gsn_parry) / 2;

    if (get_eq_char(victim, WEAR_WIELD) == NULL) {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (!can_see(ch, victim))
	chance /= 2;

    if (number_percent() >= chance + victim->level - ch->level)
	return FALSE;

    if (get_eq_char(victim, WEAR_SECONDARY)) {	/*EE960619 */
	if (number_percent() < get_skill(victim, gsn_dual_wield)) {
	    if (number_percent() <= chance + victim->level - ch->level) {
		act
		    ("`4You parry $n's attack with your second weapon. Woo, way to go!``",
		     ch, NULL, victim, TO_VICT);
		act
		    ("`4$N parries your attack with $S second weapon. Impressive!``",
		     ch, NULL, victim, TO_CHAR);
		check_improve(victim, gsn_parry, TRUE, 6);
		return TRUE;
	    }
	}
    }

    act("`4You parry $n's attack.``", ch, NULL, victim, TO_VICT);
    act("`4$N parries your attack.``", ch, NULL, victim, TO_CHAR);
    check_improve(victim, gsn_parry, TRUE, 6);
    return TRUE;
}

/*
* Check for shield block.
*/
bool check_shield_block(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE(victim))
	return FALSE;


    chance = get_skill(victim, gsn_shield_block) / 5 + 3;


    if (get_eq_char(victim, WEAR_SHIELD) == NULL)
	return FALSE;

    if (number_percent() >= chance + victim->level - ch->level)
	return FALSE;

    act("`4You block $n's attack with your shield.``", ch, NULL, victim,
	TO_VICT);
    act("`4$N blocks your attack with a shield.``", ch, NULL, victim,
	TO_CHAR);
    check_improve(victim, gsn_shield_block, TRUE, 6);
    return TRUE;
}


/*
* Check for dodge.
*/
bool check_dodge(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE(victim))
	return FALSE;

    chance = get_skill(victim, gsn_dodge) / 2;

    if (!can_see(victim, ch))
	chance /= 2;

    if (number_percent() >= chance + victim->level - ch->level)
	return FALSE;

    act("`4You dodge $n's attack.``", ch, NULL, victim, TO_VICT);
    act("`4$N dodges your attack.``", ch, NULL, victim, TO_CHAR);
    check_improve(victim, gsn_dodge, TRUE, 6);
    return TRUE;
}

bool check_counter(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt)
{
    int chance;
    int dam_type;
    OBJ_DATA *wield;

    if ((get_eq_char(victim, WEAR_WIELD) == NULL) ||
	(!IS_AWAKE(victim)) ||
	(!can_see(victim, ch)) || (get_skill(victim, gsn_counter) < 1))
	return FALSE;

    wield = get_eq_char(victim, WEAR_WIELD);

    chance = get_skill(victim, gsn_counter) / 6;

    /* Level check modified for higher level victim/ch */
    /* Vorlin */

    if (ch->level > victim->level) {
	chance -= (ch->level - victim->level) / 2;
    } else {
	chance += (victim->level - ch->level) / 2;
    }

    chance +=
	2 * (get_curr_stat(victim, STAT_DEX) -
	     get_curr_stat(ch, STAT_DEX));
    chance +=
	get_weapon_skill(victim,
			 get_weapon_sn(victim)) - get_weapon_skill(ch,
								   get_weapon_sn
								   (ch));
    chance +=
	(get_curr_stat(victim, STAT_STR) - get_curr_stat(ch, STAT_STR));

    if (number_percent() >= chance)
	return FALSE;

    if (get_skill(victim, gsn_parry) == 0) {
	chance += 25;
    }

    dt = gsn_counter;

    if (dt == TYPE_UNDEFINED) {
	dt = TYPE_HIT;
	if (wield != NULL && wield->item_type == ITEM_WEAPON)
	    dt += wield->value[3];
	else
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
	if (wield != NULL)
	    dam_type = attack_table[wield->value[3]].damage;
	else
	    dam_type = attack_table[ch->dam_type].damage;
    else
	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    act("You reverse $n's attack and counter with your own!", ch, NULL,
	victim, TO_VICT);
    act("$N reverses your attack!", ch, NULL, victim, TO_CHAR);

    damage(victim, ch, dam / 2, gsn_counter, dam_type, TRUE);	/* DAM MSG NUMBER!! */

    check_improve(victim, gsn_counter, TRUE, 6);

    return TRUE;
}

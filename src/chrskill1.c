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

#define MAX_DAMAGE_MESSAGE 43

DECLARE_DO_FUN(do_senses);
DECLARE_DO_FUN(do_possess);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_dragonp);
DECLARE_DO_FUN(do_hurricane);
DECLARE_DO_FUN(do_circle);
DECLARE_DO_FUN(do_roundhouse);
DECLARE_DO_FUN(do_optic);
DECLARE_DO_FUN(do_feed);
DECLARE_DO_FUN(do_throw);
DECLARE_DO_FUN(do_echo);


/*
* Disarm a creature.
* Caller must check for successful attack.
*/

void disarm(CHAR_DATA * ch, CHAR_DATA * victim)
{
    OBJ_DATA *obj;

    if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL)
	return;

    if (IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
	act("`8$S weapon won't budge!``", ch, NULL, victim, TO_CHAR);
	act("`8$n tries to disarm you, but your weapon won't budge!``",
	    ch, NULL, victim, TO_VICT);
	act("`8$n tries to disarm $N, but fails.``", ch, NULL, victim,
	    TO_NOTVICT);
	return;
    }

    act("`8$n `1DISARMS `8you and sends your weapon flying!``", ch, NULL,
	victim, TO_VICT);
    act("`8You disarm $N!``", ch, NULL, victim, TO_CHAR);
    act("`8$n disarms $N!``", ch, NULL, victim, TO_NOTVICT);

    obj_from_char(obj);

    if (IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_INVENTORY))
	obj_to_char(obj, victim);
    else {
	obj_to_room(obj, victim->in_room);

	if (IS_NPC(victim) && victim->wait == 0
	    && can_see_obj(victim, obj)) get_obj(victim, obj, NULL);
    }

    return;
}

void do_berserk(CHAR_DATA * ch, char *argument)
{
    int chance, hp_percent;

    /* Stacking berserk and battle rage, fun for the whole family!... NOT! */
    if(is_affected(ch, gsn_battle_rage)){
      send_to_char("Woah, slow down, Charlie! You'll hemmorhage to death!\n",ch);
      return;
    }


    if ((chance = get_skill(ch, gsn_berserk)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_BERSERK))
	|| (!IS_NPC(ch)
	    && ch->level <
	    skill_table[gsn_berserk].skill_level[ch->class])) {
	send_to_char("You turn red in the face, but nothing happens.\n\r",
		     ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_BERSERK) || is_affected(ch, gsn_berserk)
	|| is_affected(ch, skill_lookup("frenzy"))) {
	send_to_char("You're already in a rage.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CALM)) {
	send_to_char("You're feeling too mellow to berserk.\n\r", ch);
	return;
    }

    if (ch->mana < 50) {
	send_to_char("You can't get up enough energy.\n\r", ch);
	return;
    }

    if (ch->move < 20 + ch->level) {
	send_to_char("You're too fatigued to enter a berserker rage.\n\r", ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 25% of hp helps, above hurts */
    hp_percent = 100 * ch->hit / ch->max_hit;
    chance += 25 - hp_percent / 4;

    if (number_percent() < chance) {
	AFFECT_DATA af;

	WAIT_STATE(ch, PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move -= ch->level;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit, ch->max_hit);

	send_to_char("Your pulse races as you are consumed by rage!\n\r",
		     ch);
	act("$n gets a wild look in $s eyes.", ch, NULL, NULL, TO_ROOM);
	check_improve(ch, gsn_berserk, TRUE, 2);

	af.where = TO_AFFECTS;
	af.type = gsn_berserk;
	af.level = ch->level;
	af.duration = number_fuzzy(ch->level / 8);
	af.modifier = UMAX(1, ch->level / 7);
	af.bitvector = AFF_BERSERK;

	af.location = APPLY_HITROLL;
	affect_to_char(ch, &af);

	af.location = APPLY_DAMROLL;
	affect_to_char(ch, &af);

	af.modifier = UMAX(16, 8 * (ch->level / 6));
	af.location = APPLY_AC;
	affect_to_char(ch, &af);
    } else {
	WAIT_STATE(ch, PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move -= ch->level;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r", ch);
	check_improve(ch, gsn_berserk, FALSE, 2);
    }
}

void do_rage(CHAR_DATA * ch, char *argument)
{
    int chance;

    /* Stacking berserk and battle rage, fun for the whole family!... NOT! */
    if(is_affected(ch, AFF_BERSERK)){
      send_to_char("Woah, slow down, Charlie! You'll hemmorhage to death!\n",ch);
      return;
    }

    if ((chance = get_skill(ch, gsn_battle_rage)) == 0 || (!IS_NPC(ch)
							   && ch->level <
							   skill_table
							   [gsn_battle_rage].skill_level
							   [ch->class])) {
	send_to_char
	    ("You turn blood red in the face, but nothing happens.\n\r",
	     ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_BERSERK) || is_affected(ch, gsn_battle_rage)
	|| is_affected(ch, skill_lookup("frenzy"))
	|| is_affected(ch, skill_lookup("berserk"))) {
	send_to_char("You feed your battle rage a little more.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CALM)) {
	send_to_char("You're feeling a bit calm to go into a rage.\n\r",
		     ch);
	return;
    }

    if (ch->mana < 50) {
	send_to_char("You can't gather enough energy to enter a rage.\n\r", ch);
	return;
    }

    if (ch->move < ch->level + 20) {
	send_to_char("You're too fatigued to enter a rage.\n\r", ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    if (number_percent() < chance) {
	AFFECT_DATA af;

	WAIT_STATE(ch, PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move -= ch->level;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit, ch->max_hit);

	send_to_char
	    ("Rational thought leaves you as you are consumed in a rage!\n\r",
	     ch);
	act("$n gets a wild look in $s eyes.", ch, NULL, NULL, TO_ROOM);
	check_improve(ch, gsn_battle_rage, TRUE, 2);

	af.where = TO_AFFECTS;
	af.type = gsn_battle_rage;
	af.level = ch->level;
	af.duration = number_fuzzy(ch->level / 5);
	af.modifier = UMAX(1, ch->level / 2.25);
	af.bitvector = 0;

	af.location = APPLY_HITROLL;
	affect_to_char(ch, &af);

	af.location = APPLY_DAMROLL;
	affect_to_char(ch, &af);

    } else {
	WAIT_STATE(ch, PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move -= ch->level;

	send_to_char
	    ("Your pulse rapidly increases, but nothing happens.\n\r", ch);
	check_improve(ch, gsn_battle_rage, FALSE, 2);
    }
}

void do_senses(CHAR_DATA * ch, char *argument)
{
    int chance;

    if ((chance = get_skill(ch, gsn_senses)) == 0 || (!IS_NPC(ch)
						      && ch->level <
						      skill_table
						      [gsn_senses].
						      skill_level[ch->
								  class]))
    {
	send_to_char
	    ("You're not skilled in the ability to improve your awareness.\n\r",
	     ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_DETECT_INVIS) || is_affected(ch, gsn_senses)
	|| (IS_AFFECTED(ch, AFF_DETECT_HIDDEN))) {
	send_to_char
	    ("You are already more aware of your surroundings.\n\r", ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance -= 10;

    /* Check here */
    if (ch->move < 50) {
	send_to_char
	    ("You are too tired to concentrate on your surroundings.\n\r",
	     ch);
	return;
    }

    if (number_percent() < chance) {
	AFFECT_DATA af;

	WAIT_STATE(ch, PULSE_VIOLENCE / 2);
	ch->move -= 50;

	send_to_char
	    ("You concentrate and become more aware of your surroundings.\n\r",
	     ch);
	act("$n looks a bit more aware of their surroundings.\n\r", ch,
	    NULL, NULL, TO_ROOM);
	check_improve(ch, gsn_senses, TRUE, 2);

	af.where = TO_AFFECTS;
	af.type = gsn_senses;
	af.level = ch->level;
	af.duration = ch->level / 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char(ch, &af);
	af.bitvector = AFF_DETECT_INVIS;
	affect_to_char(ch, &af);
    } else {
	WAIT_STATE(ch, PULSE_VIOLENCE / 2);
	ch->move -= 20;

	send_to_char
	    ("You attempt to improve your senses, but lose concentration.\n\r",
	     ch);
	check_improve(ch, gsn_senses, FALSE, 2);
    }
}

void do_possess(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int chance;
    AFFECT_DATA af;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_possess)) == 0 || (!IS_NPC(ch)
						       && ch->level <
						       skill_table
						       [gsn_possess].skill_level
						       [ch->class])) {
	send_to_char("Applying the demonic ways does nothing for you.\n\r",
		     ch);
	return;
    }

    /* Conditions to get out of being possessed */
    if (arg[0] == '\0') {
	send_to_char("Possess whom?\n\r", ch);
	return;
    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    } else if (is_affected(victim, gsn_possess)) {
	sprintf(buf,
		"%s's mind is too clouded to be possessed so soon.\n\r",
		capitalize(victim->name));
	send_to_char(buf, ch);
	return;
    } else if (victim->position == POS_FIGHTING) {
	sprintf(buf,
		"You can't seem to grasp the soul of %s while they're fighting.\n\r",
		victim->short_descr);
	send_to_char(buf, ch);
	return;
    } else if ((IS_SET(victim->act, PLR_DEAD))
	       || ((IS_SET(ch->act, PLR_DEAD)) && (!IS_NPC(victim)))) {
	send_to_char("You are not permitted to kill that person!\n\r", ch);
	return;
    } else if (!IS_NPC(victim) && (victim->pcdata->pkset == FALSE)) {
	send_to_char("Let's try on pkset characters, ok?\n\r", ch);
	return;
    } else if (is_safe(ch, victim)) {
	act("$N is safe from your intentions.", ch, NULL, victim, TO_CHAR);
	return;
    } else if (victim == ch) {
	send_to_char("What's the use trying to possess yourself?\n\r", ch);
	return;
    } else if (IS_AFFECTED(victim, AFF_CHARM)
	       || IS_AFFECTED(ch, AFF_CHARM)
	       || IS_SET(victim->imm_flags, IMM_CHARM)
	       || victim->level > (ch->level + PK_LEVEL + 1)
	       || IS_IMMORTAL(victim)) {
	send_to_char("You failed.\n\r", ch);
	act("$n just tried to possess you!", ch, NULL, NULL, TO_VICT);
	act("$n just tried to possess $N!", ch, NULL, victim, TO_NOTVICT);
	WAIT_STATE(ch, number_range(1, 2) * PULSE_VIOLENCE);
	return;
    } else if (ch->move < 100) {
	send_to_char
	    ("You do not have enough strength for possessing currently.\n\r",
	     ch);
	return;
    }

    /* These calculations were HIDEOUS before... */
    /* revamped by Vorlin on 8/15/2000 */

    if (ch->alignment >= -350)	/* no longer evil */
	chance *= .70;

    if (victim->alignment >= 350)	/* victim's align is good */
	chance *= .80;

    if (victim->alignment <= 0)	/* victim's align is neutral or less */
	chance += 10;

    if (chance <= number_percent()) {
	/*
	   act("You failed to possess $N.",ch,NULL,victim,TO_CHAR);
	   act("$n failed to possess you!",ch,NULL,victim,TO_VICT);
	 */
	sprintf(buf, "You failed to possess %s.\n\r",
		!IS_NPC(victim) ? victim->name : capitalize(victim->
							    short_descr));
	send_to_char(buf, ch);
	act("$n failed to possess you!", ch, NULL, victim, TO_VICT);
	ch->move -= 100;
	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	return;
    }

    /* HA HA, it worked */
    if (victim->master)
	stop_follower(victim);

    add_follower(victim, ch);
    victim->leader = ch;

    af.where = TO_AFFECTS;
    af.type = gsn_possess;
    af.level = ch->level;
    af.duration = number_fuzzy(ch->level / 30) + 1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CHARM;

    affect_to_char(victim, &af);
    act("$n's essence violently enters your body, embracing your soul. ",
	ch, NULL, victim, TO_VICT);
    if (ch != victim)
	act("$N's mind, body, and spirit now embraces you.", ch, NULL,
	    victim, TO_CHAR);

    ch->move -= 50;
    WAIT_STATE(ch, number_range(1, 1.5) * PULSE_VIOLENCE);
    WAIT_STATE(victim, number_range(1, 2) * PULSE_VIOLENCE);
    check_killer(ch, victim);

    return;
}

void do_demonrage(CHAR_DATA * ch, char *argument)
{
    int chance;

    if ((chance = get_skill(ch, gsn_demonrage)) == 0
	|| (!IS_NPC(ch)
	&& ch->level < skill_table[gsn_demonrage].skill_level[ch->class])) {
	send_to_char("Only demons can draw upon the powers of Hell in this manner.\n\r", ch);
	return;
    }

    if (is_affected(ch, gsn_demonrage)) {
	send_to_char("Hell's fury already swells within your body.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CALM)) {
	send_to_char("You feel a bit too calm to invoke your demonic fury.\n\r", ch); 
	return;
    }

    /* movement check */
    if (ch->move < 120) {
	send_to_char("You're too tired to incite demon rage.\n\r", ch);
	return;
    }

    if (number_percent() < chance) {
	AFFECT_DATA af;

	ch->move -= 100;

	ch->hit += ch->level * 3.5;
	ch->hit = UMIN(ch->hit, ch->max_hit);

	send_to_char("Flames of hell burst forth as you enter a homicidal fury!\n\r", ch);
	act("$n howls with demonic fury, hellfire surrounding $s body!", ch, NULL, NULL, TO_ROOM);
	check_improve(ch, gsn_demonrage, TRUE, 3);

	af.where = TO_AFFECTS;
	af.type = gsn_demonrage;
	af.level = ch->level;
	af.duration = 10;
	af.modifier = UMAX(1, ch->level / 5);
	af.bitvector = 0;

	af.location = APPLY_HITROLL;
	affect_to_char(ch, &af);

	af.location = APPLY_DAMROLL;
	affect_to_char(ch, &af);

	af.modifier = -3 * (ch->level / 5);
	af.location = APPLY_AC;
	affect_to_char(ch, &af);

	af.modifier = -1 * (ch->level / 5);
	af.location = APPLY_SAVES;
	affect_to_char(ch, &af);
    } else {
	ch->move -= 50;
	WAIT_STATE(ch, PULSE_VIOLENCE);

	send_to_char("You attempt to invoke demon rage, but fail.\n\r", ch);
	check_improve(ch, gsn_demonrage, FALSE, 3);
    }
    return;
}

void do_bash(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_bash)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_BASH))
	|| (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_bash].skill_level[ch->class])) {
	send_to_char("Bashing? What's that?\n\r", ch);
	return;
    }


    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("But you aren't fighting anyone!\n\r", ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_PASS_DOOR)
	|| IS_AFFECTED(ch, AFF_PASS_DOOR)) {
	act("`4You try to bash $M, but pass right through!``", ch,
	    NULL, victim, TO_CHAR);
	act("`^$n tries to bash you, but passes right through!``", ch,
	    NULL, victim, TO_VICT);
	act("`4$n tries to bash $M, but passes right through!``", ch, NULL,
	    victim, TO_NOTVICT);
	damage(ch, victim, 0, gsn_bash, DAM_NONE, FALSE);
	return;
    }

    if (victim->position < POS_FIGHTING) {
	act("You'll have to let $M get back up first.", ch, NULL, victim,
	    TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("You try to bash your brains out, but fail.\n\r", ch);
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

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* modifiers */

    /* size and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10;


    /* stats */
    chance += get_curr_stat(ch, STAT_STR);
    chance -= (get_curr_stat(victim, STAT_DEX) * 4) / 3;
    chance -= GET_AC(victim, AC_BASH) / 25;
    /* speed */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE))
	chance -= 30;

    /* level */
    chance += ch->level - victim->level;

    if (!IS_NPC(victim) && chance < get_skill(victim, gsn_dodge)) {
	act("`2$n tries to bash you, but you dodge it.``", ch, NULL,
	    victim, TO_VICT);
	act("`@$N dodges your bash, you fall flat on your face.``", ch,
	    NULL, victim, TO_CHAR);
	damage(ch, victim, 0, gsn_bash, DAM_NONE, FALSE);
	ch->position = POS_RESTING;
	WAIT_STATE(ch, skill_table[gsn_bash].beats);
	return;
    }

    /* now the attack */
    if (number_percent() < chance) {

	act
	    ("`2$n sends you s`@p`2r`@a`2w`@l`2i`@n`2g with a powerful bash!``",
	     ch, NULL, victim, TO_VICT);
	act("`@You slam into $N, and send $M flying!``", ch, NULL, victim,
	    TO_CHAR);
	act("`@$n sends $N sprawling with a powerful bash.``", ch, NULL,
	    victim, TO_NOTVICT);
	if (is_affected(victim, skill_lookup("stone skin"))) {
	    act("`2$n bounces off and grunts in pain.``", ch, NULL, victim,
		TO_VICT);
	    act("`@$n bounces off of $N, and grunts in pain.``", ch, NULL,
		victim, TO_NOTVICT);
	    act("`@You bounce off and grunts in pain.``", ch, NULL, victim,
		TO_CHAR);
	    damage(victim, ch,
		   number_range(2, 2 + 2 * victim->size + chance / 20),
		   gsn_bash, DAM_BASH, TRUE);
	}
	check_improve(ch, gsn_bash, TRUE, 1);

	WAIT_STATE(victim, number_range(2, 4) * PULSE_VIOLENCE);
	WAIT_STATE(ch, number_range(1, 3) * PULSE_VIOLENCE);
	victim->position = POS_RESTING;
	damage(ch, victim, number_range(2, 2 + 2 * ch->size + chance / 20),
	       gsn_bash, DAM_BASH, TRUE);

    } else {
	/*DC961212* Apparently Damage is done, but isn't showing. Changed to show */
	damage(ch, victim, 0, gsn_bash, DAM_BASH, TRUE);
	act("`@You fall flat on your face!``", ch, NULL, victim, TO_CHAR);
	act("`@$n falls flat on $s face.``", ch, NULL, victim, TO_NOTVICT);
	act("`2You evade $n's bash, causing $m to fall flat on $s face.``",
	    ch, NULL, victim, TO_VICT);
	check_improve(ch, gsn_bash, FALSE, 1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch, skill_table[gsn_bash].beats);
    }
    check_killer(ch, victim);
}

void do_dirt(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_dirt)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK_DIRT))
	|| (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_dirt].skill_level[ch->class])) {
	send_to_char("You get your feet dirty.\n\r", ch);
	return;
    }


    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("But you aren't in combat!\n\r", ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_BLIND)) {
	act("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("Very funny.\n\r", ch);
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

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= 2 * get_curr_stat(victim, STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch (ch->in_room->sector_type) {
    case (SECT_INSIDE):
	chance -= 20;
	break;
    case (SECT_CITY):
	chance -= 10;
	break;
    case (SECT_FIELD):
	chance += 5;
	break;
    case (SECT_FOREST):
	break;
    case (SECT_HILLS):
	break;
    case (SECT_MOUNTAIN):
	chance -= 10;
	break;
    case (SECT_WATER_SWIM):
	chance = 0;
	break;
    case (SECT_UNDERWATER):
	chance = 0;
	break;
    case (SECT_WATER_NOSWIM):
	chance = 0;
	break;
    case (SECT_AIR):
	chance = 0;
	break;
    case (SECT_DESERT):
	chance += 10;
	break;
    }

    if (chance == 0) {
	send_to_char("There isn't any dirt to kick.\n\r", ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance) {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!", victim, NULL, NULL,
	    TO_ROOM);
	act("$n kicks dirt in your eyes!", ch, NULL, victim, TO_VICT);
	damage(ch, victim, number_range(10, 20), gsn_dirt, DAM_NONE,
	       FALSE);
	send_to_char("You can't see a thing!\n\r", victim);
	check_improve(ch, gsn_dirt, TRUE, 2);
	WAIT_STATE(ch, skill_table[gsn_dirt].beats);

	af.where = TO_AFFECTS;
	af.type = gsn_dirt;
	af.level = ch->level;
	af.duration = 0;
	af.location = APPLY_HITROLL;
	af.modifier = -4;
	af.bitvector = AFF_BLIND;

	affect_to_char(victim, &af);
    } else {
	damage(ch, victim, 0, gsn_dirt, DAM_NONE, TRUE);
	check_improve(ch, gsn_dirt, FALSE, 2);
	WAIT_STATE(ch, skill_table[gsn_dirt].beats);
    }
    check_killer(ch, victim);
}

void do_trip(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
/*  int race = race_lookup("Dragonkin"); */

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_trip)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_TRIP))
	|| (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_trip].skill_level[ch->class])) {
	send_to_char("Tripping?  What's that?\n\r", ch);
	return;
    }


    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("But you aren't fighting anyone!\n\r", ch);
	    return;
	} else if (victim == ch) {
	    send_to_char("Why would you want to trip yourself?\n\r", ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
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

    if (victim->position < POS_FIGHTING) {
	act("$N is already down.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("Why would you want to trip yourself?\n\r``", ch);
	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* Added this because people complained... --Vorlin */

    if (IS_AFFECTED(victim, AFF_FLYING)) {
	send_to_char("You can't trip anything that's flying...\n\r", ch);
	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
	damage(ch, victim, 0, gsn_trip, DAM_NONE, FALSE);
	return;
    } 
/* Eliminating the cheese tactic of casting fly in safe to prevent trips.. KV020603
    else if (IS_AFFECTED(ch, AFF_FLYING) && ch->race != race) {
	send_to_char("You can't trip anything while you're flying...\n\r",
		     ch);
	WAIT_STATE(ch, number_range(.5, 1) * PULSE_VIOLENCE);
	damage(ch, victim, 0, gsn_trip, DAM_NONE, FALSE);
	return;
    } else if (ch->race == race && IS_AFFECTED(ch, AFF_FLYING)) {
	send_to_char("You have to land before you can trip anything.\n\r",
		     ch);
	WAIT_STATE(ch, number_range(.5, 1) * PULSE_VIOLENCE);
	damage(ch, victim, 0, gsn_trip, DAM_NONE, FALSE);
	return;
    }
*/
    /* modifiers */

    /* size */
    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 10;	/* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance) {
	act("`4$n trips you and you go down!``", ch, NULL, victim,
	    TO_VICT);
	act("`^You trip $N and $N goes down!``", ch, NULL, victim,
	    TO_CHAR);
	act("`4$n trips $N, sending $M to the ground.``", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(ch, gsn_trip, TRUE, 1);

	WAIT_STATE(victim, (number_range(2, 3) * PULSE_VIOLENCE));
	WAIT_STATE(ch, (number_range(1, 3) * PULSE_VIOLENCE));
	victim->position = POS_RESTING;
	damage(ch, victim, number_range(8, (2 + 8) * victim->size),
	       gsn_trip, DAM_BASH, TRUE);
    } else {
	damage(ch, victim, 0, gsn_trip, DAM_BASH, TRUE);
	WAIT_STATE(ch, skill_table[gsn_trip].beats * 2 / 3);
	check_improve(ch, gsn_trip, FALSE, 1);
    }
    check_killer(ch, victim);
}

void do_dragonp(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int count, i;

    one_argument(argument, arg);

    if (IS_NPC(ch))
	return;

    if ((chance = get_skill(ch, gsn_dragonp)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_TRIP))
	|| (!IS_NPC(ch)
	    && ch->level <
	    skill_table[gsn_dragonp].skill_level[ch->class])) {
	send_to_char("Dragon Punch?  What's that?\n\r", ch);
	return;
    }


    if (arg[0] == '\0' && ch->fighting == NULL) {
	send_to_char
	    ("Dragon punching thin air never accomplishes anything.\n\r",
	     ch);
	WAIT_STATE(ch, 2);
	return;
    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if ((IS_SET(victim->act, PLR_DEAD))
	|| ((IS_SET(ch->act, PLR_DEAD)) && (!IS_NPC(victim)))) {
	send_to_char("You are not permitted to kill this person!\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (ch->fighting != NULL && arg[0] == '\0') {
	victim = ch->fighting;
    } else if (ch == victim) {
	send_to_char("You're not quite that flexible.\n\r", ch);
	return;
    }


    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (victim->position == POS_STUNNED) {
	act("$N is already stunned.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (ch->move <= 50) {
	send_to_char("You're too tired to attempt that.\n\r", ch);
	return;
    }

    /* modifiers */

/* size  */
    if (ch->size > victim->size)
	chance += (ch->size - victim->size) * 10;

/* dex */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX);

/* speed */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE)) {
	chance += 10;
    }

    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE)) {
	chance -= 10;
    }

/* level */
    chance += (ch->level - victim->level) * 2 / 3;

    if (IS_AFFECTED(ch, AFF_SLOW)) {
	chance -= number_range(10, 20);
    } else {
	chance += 5;
    }


    /*  now the attack */
    if (number_percent() < chance) {
	act("`^$n connects his Dragon Punch -OUCH-!``", ch, NULL, victim,
	    TO_VICT);
	act("`^You draw back your fist unleashing your 'Sho-Ryu-Ken'!``",
	    ch, NULL, victim, TO_CHAR);
	act("`4$n uppercuts $N, sending $M into the air.``", ch, NULL,
	    victim, TO_NOTVICT);
	check_improve(ch, gsn_dragonp, TRUE, 1);

	/* Number of DP hits here - Kevin */
	count = number_range(2, ch->level / 15);

	/* DP lag based on number of hits - Kevin */
	WAIT_STATE(ch, (count * PULSE_VIOLENCE) / 2);
	WAIT_STATE(victim, .5 * PULSE_VIOLENCE);
	victim->position = POS_RESTING;
	/* ch->move -= number_range(35, 50); */
	{
	    for (i = 1; i <= count; i++) {
		damage(ch, victim, ch->hit / 7, gsn_dragonp, DAM_FIRE, TRUE);
	    }
	}
    } else {
	act("`4$N dodges $n's uppercut!``", ch, NULL, victim, TO_NOTVICT);
	act("`4You dodges $n's uppercut!``", ch, NULL, victim, TO_VICT);
	act("`4$N dodges your uppercut!``", ch, NULL, victim, TO_CHAR);

	/* Added random lag states and start the fight if not started */
	/* Vorlin */
	/* ch->move -= number_range(25, 35); */
	WAIT_STATE(ch, (number_range(1, 2) * PULSE_VIOLENCE));
	damage(ch, victim, 0, gsn_dragonp, DAM_NONE, FALSE);
	check_improve(ch, gsn_dragonp, FALSE, 1);
    }
    check_killer(ch, victim);
}


void do_hurricane(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int count, i;

    one_argument(argument, arg);

    if (IS_NPC(ch))
	return;

    if ((chance = get_skill(ch, gsn_hurricane)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	|| (!IS_NPC(ch)
	    && ch->level <
	    skill_table[gsn_hurricane].skill_level[ch->class])) {
	send_to_char
	    ("Hurricane Kick?  Trying to hurt ourselves, are we?\n\r", ch);
	return;
    }


    if (arg[0] == '\0' && ch->fighting == NULL) {
	send_to_char("Practicing your hurricane kick for fun?\n\r", ch);
	WAIT_STATE(ch, 2);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	sprintf(buf, "%s isn't here.\n\r", capitalize(arg));
	send_to_char(buf, ch);
	return;
    }

    if (ch->fighting != NULL && arg[0] == '\0') {
	victim = ch->fighting;
    } else if (ch == victim) {
	send_to_char("You're not quite that flexible.\n\r", ch);
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

    if (ch->move < 50 && ch->level <= 50) {
	send_to_char("You don't have enough energy to do that.\n\r", ch);
	return;
    } else if (ch->move < 100 && ch->level > 50) {
	send_to_char("You don't have enough energy to do that.\n\r", ch);
	return;
    }

    if (victim->position == POS_STUNNED) {
	act("$N is already stunned.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* modifiers */

	/* size  */
    if (ch->size < victim->size) {
	chance += (victim->size - ch->size) * 10;
    }

	/* dex */
    chance += get_curr_stat(ch, STAT_DEX) * 3 / 2;
    chance -= get_curr_stat(victim, STAT_DEX) * 4 / 3;

	/* speed check */
    if (IS_AFFECTED(ch, AFF_HASTE) || IS_AFFECTED(ch, OFF_FAST)) {
	chance += number_range(10, 20);
    }

    if (IS_AFFECTED(victim, AFF_HASTE) || IS_AFFECTED(victim, OFF_FAST)) {
	chance -= number_range(5, 15);
    }

	/* level */
    chance += (ch->level - victim->level) * 2 / 3;

    /*  now the attack */
    if (number_percent() < chance) {
		act("`^$n connects his Hurricane Kick -OUCH-!``", ch, NULL, victim,TO_VICT);
		act("`^You draw energy into your legs unleashing your 'tatsu-maki-senpu-kyaku'!``", ch, NULL, victim, TO_CHAR);
		act("`4$n hurricane kick $N, sending $M sprawling onto the floor.``", ch, NULL, victim, TO_NOTVICT);

		/* Number of HK hits here - Kevin */
		count = number_range(2, ch->level / 15);
	
		WAIT_STATE(ch, (count * PULSE_VIOLENCE) / 2);
		WAIT_STATE(victim, .5 * PULSE_VIOLENCE);
		victim->position = POS_RESTING;

		/* Slashed movement costs to 20% of old values - KV020516 */
		if (ch->level <= 50) {
	    	ch->move -= number_range(4, 8);
		} else {
	    	ch->move -= number_range(10, 16);
		}

		check_improve(ch, gsn_hurricane, TRUE, 1);

		for (i = 1; i <= count; i++) {
	    	damage(ch, victim, ch->move / 6, gsn_hurricane, DAM_BASH, TRUE);
		}
    } else {
		act("`^$n attempts to hurricane kick you but fails!``", ch, NULL, victim, TO_VICT);
		act("`^You attemp a hurricane kick but fail!``", ch, NULL, victim, TO_CHAR);
		act("`4$n attempts to hurricane kick $N but fails!``", ch, NULL, victim, TO_NOTVICT);
		WAIT_STATE(ch, PULSE_VIOLENCE);
		damage(ch, victim, 0, gsn_hurricane, DAM_NONE, FALSE);
		/* Added random lag states and starts the fight if not started */
		/* Vorlin */
		if (ch->fighting == NULL) {
		    ch->fighting = victim;
		}
		ch->move -= number_range(2, 6);
		check_improve(ch, gsn_hurricane, FALSE, 1);
    }
    check_killer(ch, victim);
}

void do_optic(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int count;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_optic)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	|| (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_optic].skill_level[ch->class])) {
	send_to_char("Only cyborgs may use optic blast.\n\r", ch);
	return;
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char
		("You can not use optic blast unless you have a target.\n\r",
		 ch);
	    return;
	}

    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if ((IS_SET(victim->act, PLR_DEAD)) || ((IS_SET(ch->act, PLR_DEAD))
					    && (!IS_NPC(victim)))) {
	send_to_char("You are not permitted to kill this person!\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim)
	&& victim->fighting != NULL
	&& !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (ch->move <= 50) {
	send_to_char("Your circuits are currently recharging.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("Optic blast yourself?\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }
/* modifiers */

/* can they see? */
    if (IS_AFFECTED(ch, AFF_BLIND)) {
	send_to_char("You can't do that while blind.\n\r", ch);
	return;
    }

/* dexterity check */
    chance += get_curr_stat(ch, STAT_DEX) * 3 / 2;
    chance -= get_curr_stat(victim, STAT_DEX) * 4 / 3;

/* speed check */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE)) {
	chance += 10;
    }

    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE)) {
	chance -= 20;
    }

/* level */
    if (ch->level > victim->level) {
	chance += (ch->level - victim->level) * 2 / 3;
    } else {
	chance -= (victim->level - ch->level) * 2 / 3;
    }

/* now the attack */
    if (chance > number_percent()) {
	act
	    ("`!$n fires a concentrated beam of energy from $s eyes, striking you!``",
	     ch, NULL, victim, TO_VICT);
	act("`!You fire a concentrated beam of energy from your eyes!``",
	    ch, NULL, victim, TO_CHAR);
	act
	    ("`!$n fires a concentrated beam of energy from $s eyes, striking $N!``",
	     ch, NULL, victim, TO_NOTVICT);
	check_improve(ch, gsn_optic, TRUE, 1);

	for (count = 1; count <= number_range(2, ch->level / 15); count++) {
	    damage(ch, victim, number_range(ch->level * 2, ch->level * 3),
		gsn_optic, DAM_BASH, TRUE);
	}

	WAIT_STATE(ch, number_range(1, 3) * PULSE_VIOLENCE);
	WAIT_STATE(victim, 1 * PULSE_VIOLENCE);
	if (ch->level >= 50) {
	    ch->move -= number_range(20, 50);
	} else {
	    ch->move -= number_range(10, 20);
	}
    } else {
	act("$n tries to fire an optic blast at you, but fails.", ch, NULL,
	    victim, TO_VICT);
	act("You try to fire an optic blast at $N, but fail.", ch, NULL,
	    victim, TO_CHAR);
	act("$n tries to fry $N with an optic blast, but fails.", ch, NULL,
	    victim, TO_NOTVICT);
	WAIT_STATE(ch, number_range(1, 2) * PULSE_VIOLENCE);
	if (ch->fighting == NULL || ch->fighting != victim) {
	    victim->fighting = ch;
	}
	check_improve(ch, gsn_optic, FALSE, 1);
	ch->move -= number_range(10, 25);
    }

    check_killer(ch, victim);
}

void do_feed(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance, count;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_feed)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	|| (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_feed].skill_level[ch->class])) {
	send_to_char("Only vampires may feed for hp replenishment.\n\r",
		     ch);
	return;
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("You must have a target in order to feed.\n\r",
			 ch);
	    return;
	}

    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if ((IS_SET(victim->act, PLR_DEAD))
	|| ((IS_SET(ch->act, PLR_DEAD)) && (!IS_NPC(victim)))) {
	send_to_char("You are not permitted to kill this person!\n\r", ch);
	return;
    }


    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim)
	&& victim->fighting != NULL
	&& !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (ch->move <= 50) {
	send_to_char("You're too tired to attempt to feed.\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("That's completely pointless...\n\r", ch);
	return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* modifiers */

    /* size  */
    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 10;

    /* dex */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /*  now the attack */
    if (number_percent() < chance) {
	act("`!$n quickly moves around your body and bites your neck!``",
	    ch, NULL, victim, TO_VICT);
	act
	    ("`!You quickly move around your opponent and bite them on the neck!``",
	     ch, NULL, NULL, TO_CHAR);
	act("`1$n quickly moves around $N and bites them on the neck!", ch,
	    NULL, victim, TO_NOTVICT);
	check_improve(ch, gsn_feed, TRUE, 1);
	WAIT_STATE(victim, 1 * PULSE_VIOLENCE);
	WAIT_STATE(ch, number_range(1, 3) * PULSE_VIOLENCE);

	{
	    for (count = 1; count <= number_range(2, ch->level / 20);
		 count++)

		if (time_info.hour >= 6 && time_info.hour <= 19) {
		    damage(ch, victim, number_range(ch->level + 15,
						    ch->level * 2),
			   gsn_feed, DAM_BASH, TRUE);
		} else {
		    damage(ch, victim, number_range(ch->level * 2,
						    ch->level * 3),
			   gsn_feed, DAM_BASH, TRUE);
		}
	}
	if (ch->level >= 50) {
	    ch->move -= number_range(25, 50);
	} else {
	    ch->move -= number_range(10, 25);
	}

	if (ch->hit < ch->max_hit) {
	    ch->hit += ch->level + 10;
	} else {
	    return;
	}
    } else {
	act("`!$n attempts to feed on you and misses!``", ch,
	    NULL, victim, TO_VICT);
	act("`!$n attempts to feed on $N but misses!``", ch,
	    NULL, victim, TO_NOTVICT);
	act("`!You attempt to feed on $N's neck but miss!``",
	    ch, NULL, victim, TO_CHAR);
	WAIT_STATE(ch, number_range(1, 2) * PULSE_VIOLENCE);
	damage(ch, victim, 0, gsn_feed, DAM_NONE, FALSE);
	check_improve(ch, gsn_feed, FALSE, 1);
	ch->move -= number_range(15, 30);
    }
    check_killer(ch, victim);
}

void do_throw(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_throw)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	|| (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_throw].skill_level[ch->class])) {
	send_to_char("Giants can only throw their opponents.\n\r", ch);
	return;
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("You must provide a target to throw.\n\r", ch);
	    return;
	}

    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }


    if ((IS_SET(victim->act, PLR_DEAD))
	|| ((IS_SET(ch->act, PLR_DEAD)) && (!IS_NPC(victim)))) {
	send_to_char("You are not permitted to kill this person!\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim)
	&& victim->fighting != NULL
	&& !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (ch->move <= 80) {
	send_to_char("You need more strength to throw your opponent.\n\r",
		     ch);
	return;
    }

    if (victim == ch) {
	send_to_char("How do you plan on throwing yourself?\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

/* modifiers */
/* I was here, fixing this mess... --Vorlin, 8/20/2000 */

/* size  */
    if (ch->size > victim->size) {
	chance += (ch->size - victim->size) * 8;
    }

/* str */
    if (get_curr_stat(ch, STAT_STR) > get_curr_stat(victim, STAT_STR)) {
	chance +=
	    (get_curr_stat(ch, STAT_STR) -
	     get_curr_stat(victim, STAT_STR)) * 4;
    } else {
	chance -=
	    (get_curr_stat(victim, STAT_STR) -
	     get_curr_stat(ch, STAT_STR)) * 8;
    }

/* speed */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE)) {
	chance += 20;
    }

    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE)) {
	chance -= 20;
    }

/* level */
    if (ch->level > victim->level) {
	chance += (ch->level - victim->level) * 3 / 2;
    } else {
	chance -= (victim->level - ch->level) * 4 / 3;
    }

/*  now the attack */
    if (chance > number_percent()) {
	act("$n picks you up and throws you down on the ground!", ch, NULL,
	    victim, TO_VICT);
	act
	    ("You quickly pick up your opponent and throw them onto the ground!",
	     ch, NULL, victim, TO_CHAR);
	act("$n picks up $N and throws them down on the ground!", ch, NULL,
	    victim, TO_NOTVICT);
	check_improve(ch, gsn_throw, TRUE, 1);

	WAIT_STATE(victim, (number_range(1.5, 3) * PULSE_VIOLENCE));
	WAIT_STATE(ch, (number_range(1.5, 4) * PULSE_VIOLENCE));

	damage(ch, victim, number_range(ch->level * 1.5, ch->level * 3),
	       gsn_throw, DAM_BASH, TRUE);
	if (ch->level >= 50) {
	    ch->move -= number_range(50, 75);
	} else {
	    ch->move -= number_range(25, 40);
	}
    } else {
	act("$n failed to throw you!", ch, NULL, victim, TO_VICT);
	act("You failed to throw $N!", ch, NULL, victim, TO_CHAR);
	act("$n fails to throw $N!", ch, NULL, victim, TO_NOTVICT);
	check_improve(ch, gsn_throw, FALSE, 1);

	if (ch->fighting == NULL || ch->fighting != victim) {
	    victim->fighting = ch;
	}
	WAIT_STATE(ch, (number_range(1, 2) * PULSE_VIOLENCE));
	ch->move -= number_range(15, 40);
    }
    check_killer(ch, victim);
}


void do_footsweep(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
/*  int race = race_lookup("Dragonkin"); */

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_footsweep)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	|| (!IS_NPC(ch)
	    && ch->level <
	    skill_table[gsn_footsweep].skill_level[ch->class])) {
	send_to_char("Trying to break dance, I take it?\n\r", ch);
	return;
    }


    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("You do a footsweep to noone in particular.\n\r",
			 ch);
	    return;
	}
    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    } else if (victim == ch) {
	send_to_char("Why would you want to footsweep yourself?\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if ((IS_SET(victim->act, PLR_DEAD))
	|| ((IS_SET(ch->act, PLR_DEAD)) && (!IS_NPC(victim)))) {
	send_to_char("You are not permitted to kill that person!\n\r", ch);
	return;
    }


    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_FLYING)) {
	send_to_char("You can't footsweep anything that's flying...\n\r",
		     ch);
	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
	damage(ch, victim, 0, gsn_footsweep, DAM_NONE, TRUE);
	return;
    } 

/* Eliminating the cheese tactic of casting fly in safe to prevent trips.. KV020603
    else if (IS_AFFECTED(ch, AFF_FLYING) && ch->race != race) {
	send_to_char
	    ("You can't footsweep anything while you're flying...\n\r",
	     ch);
	WAIT_STATE(ch, number_range(.5, 1) * PULSE_VIOLENCE);
	damage(ch, victim, 0, gsn_footsweep, DAM_NONE, TRUE);
	return;
    } else if (ch->race == race && IS_AFFECTED(ch, AFF_FLYING)) {
	send_to_char
	    ("You have to land before you can footsweep anyone.\n\r", ch);
	WAIT_STATE(ch, number_range(.5, 1) * PULSE_VIOLENCE);
	damage(ch, victim, 0, gsn_footsweep, DAM_NONE, TRUE);
	return;
    }
*/

    if (victim->position < POS_FIGHTING) {
	act("$N is already stunned.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("`^You fall down on the floor!\n\r``", ch);
	WAIT_STATE(ch, 2 * skill_table[gsn_footsweep].beats + 1);
	act("`^$n over estimates $s footsweep!``", ch, NULL, victim,
	    TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* modifiers */

    /* size  */
    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 10;

    /* dex */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE))
	chance -= 20;
    /* level */
    chance += (ch->level - victim->level) * 2;


    /*  now the attack */
    if (number_percent() < chance) {
	act("`^$n connects $S footsweep, sending you to the ground!``", ch,
	    NULL, victim, TO_VICT);
	act("`^You quickly slide in a circular motion and trip up $N!``",
	    ch, NULL, victim, TO_CHAR);
	act("`4$n footsweeps $N, sending $M onto the floor!``", ch, NULL,
	    victim, TO_NOTVICT);
	check_improve(ch, gsn_footsweep, TRUE, 1);

	WAIT_STATE(victim, (number_range(1, 3) * PULSE_VIOLENCE));
	WAIT_STATE(ch, (number_range(1, 3) * PULSE_VIOLENCE));
	victim->position = POS_RESTING;
	damage(ch, victim, number_range(ch->move / 8, ch->move / 10),
	       gsn_footsweep, DAM_BASH, TRUE);
    } else {
	act("$n misses attempts to footsweep $N but misses!", ch, NULL,
	    victim, TO_NOTVICT);
	act("You dodge $n's footsweep, nice!", ch, NULL, NULL, TO_VICT);
	act("Your footsweep misses $N!", ch, NULL, victim, TO_CHAR);
	if (ch->fighting == NULL || ch->fighting != victim) {
	    victim->fighting = ch;
	}
	WAIT_STATE(ch, 2);
	check_improve(ch, gsn_footsweep, FALSE, 1);
    }
    check_killer(ch, victim);
}


void do_circle(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0' && ch->fighting == NULL) {
		send_to_char("Practicing circling for fun?\n\r", ch);
		WAIT_STATE(ch, 2);
		return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
		sprintf(buf, "%s isn't here.\n\r", capitalize(arg));
		send_to_char(buf, ch);
		return;
    }

    if (ch->fighting != NULL && arg[0] == '\0') {
		victim = ch->fighting;
    } else if (ch == victim) {
		send_to_char("How can you sneak up on yourself?\n\r", ch);
		WAIT_STATE(ch, 2);
		return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (victim->hit < victim->max_hit / 5) {
	act("$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR);
	return;
    }

    check_killer(ch, victim);
    if (number_percent() < get_skill(ch, gsn_circle)
	|| (get_skill(ch, gsn_circle) >= 2 || !IS_AWAKE(victim))) {

	act("`^$n circles you too quickly for you to follow!!``", ch, NULL,
	    victim, TO_VICT);
	act
	    ("`^You circle around your opponent too quickly for them to follow!``",
	     ch, NULL, victim, TO_CHAR);
	act("`4$n circles $N, causing $M to stagger!``", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(ch, gsn_circle, TRUE, 1);
	WAIT_STATE(victim, .5 * PULSE_VIOLENCE);
	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	multi_hit(ch, victim, gsn_circle);
    } else {
	act("`^$n attempts to circle you, but you manage to follow him!``",
	    ch, NULL, victim, TO_VICT);
	act("`^You attempt to circle your opponent..but fail.``", ch, NULL,
	    victim, TO_CHAR);
	act("`4$n attempts to circle $N, but fails.``", ch, NULL, victim,
	    TO_NOTVICT);
	WAIT_STATE(ch, (number_range(1, 2) * PULSE_VIOLENCE));
	check_improve(ch, gsn_circle, FALSE, 1);
	damage(ch, victim, 0, gsn_circle, DAM_NONE, TRUE);
    }

    return;
}

void do_roundhouse(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int count, i;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_roundhouse)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	|| (!IS_NPC(ch)
	    && ch->level <
	    skill_table[gsn_roundhouse].skill_level[ch->class])) {
	send_to_char
	    ("Better leave the martial arts to the ninjas.\n\r",
	     ch);
	return;
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("You do a roundhouse to no one in particular.\n\r",
			 ch);
	    return;
	}

    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
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
	send_to_char
	    ("`^Now how can you possibly do a roundhouse to yourself?!\n\r``",
	     ch);
	WAIT_STATE(ch, 2);
	act("`^$n attempts to roundhouse themself!``", ch, NULL, NULL,
	    TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* modifiers */

	/* size  */
    if (ch->size < victim->size)
		chance += (victim->size - ch->size) * 10;

	/* dex */
    chance += get_curr_stat(ch, STAT_DEX) * 3 / 2;
    chance -= get_curr_stat(victim, STAT_DEX) * 4 / 3;

	/* speed check */
    if (IS_AFFECTED(ch, AFF_HASTE) || IS_AFFECTED(ch, OFF_FAST))
		chance += number_range(10, 20);
    if (IS_AFFECTED(victim, AFF_HASTE) || IS_AFFECTED(victim, OFF_FAST))
		chance -= number_range(5, 15);

	/* level */
	chance += (ch->level - victim->level) * 2 / 3;

    /*  now the attack */
    if (number_percent() < chance) {
		act("`^$n connects their roundhouse kick, nice move!``", ch, NULL,
		    victim, TO_VICT);
		act
		    ("`^You quickly spin your body in a counter-clock wise motion and connect with a roundhouse kick!``",
		     ch, NULL, victim, TO_CHAR);
		act("`4$n roundhouse kick $N, sending $M sprawling onto the floor!``",
		    ch, NULL, victim, TO_NOTVICT);

		/* Number of Round House Hits */
		count = number_range(2, (ch->level / 33) + 2);

		WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
		WAIT_STATE(victim, number_range(1, 2) * PULSE_VIOLENCE);
		victim->position = POS_RESTING;
		
		for (i = 1; i <= count; i++) {
			damage(ch, victim, number_range(ch->level * 4, ch->level * 5),
				   gsn_roundhouse, DAM_BASH, TRUE);
		}
		check_improve(ch, gsn_roundhouse, TRUE, 1);
    } else {
		act("`^$n attempts to roundhouse kick you but fails!``", ch, NULL,
		    victim, TO_VICT);
		act("`^You attemp a roundhouse kick but fail!``", ch, NULL, victim,
		    TO_CHAR);
		act("`4$n attempts to roundhouse kick $N but fails!``", ch, NULL,
		    victim, TO_NOTVICT);
		damage(ch, victim, 0, gsn_roundhouse, DAM_NONE, FALSE);
		WAIT_STATE(ch, PULSE_VIOLENCE);
		if (ch->fighting == NULL) {
		    ch->fighting = victim;
		}
		ch->move -= number_range(2, 6);
		check_improve(ch, gsn_roundhouse, FALSE, 1);
    }
    check_killer(ch, victim);
}


void do_backstab(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Backstab whom?\n\r", ch);
	return;
    }

    if (ch->fighting != NULL) {
	send_to_char("You're facing the wrong end.\n\r", ch);
	return;
    }

    else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if ((IS_SET(victim->act, PLR_DEAD))
	|| ((IS_SET(ch->act, PLR_DEAD)) && (!IS_NPC(victim)))) {
	send_to_char("You are not permitted to kill this person!\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("How can you sneak up on yourself?\n\r", ch);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
	send_to_char("You need to wield a weapon to backstab.\n\r", ch);
	return;
    }

    if (obj->value[0] != WEAPON_DAGGER && obj->value[0] != WEAPON_EXOTIC
	&& obj->value[0] != WEAPON_SWORD) {
	send_to_char
	    ("You can only backstab with daggers, exotic weapons, and swords.\n\r",
	     ch);
	return;
    }

    if (victim->hit < victim->max_hit / 4) {
	act("$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR);
	return;
    }

    check_killer(ch, victim);
    WAIT_STATE(ch, skill_table[gsn_backstab].beats);
    if (number_percent() < get_skill(ch, gsn_backstab)
	|| (get_skill(ch, gsn_backstab) >= 2 || !IS_AWAKE(victim))) {
	check_improve(ch, gsn_backstab, TRUE, 1);
	multi_hit(ch, victim, gsn_backstab);
    } else {
	WAIT_STATE(ch, 4 * PULSE_VIOLENCE);
	WAIT_STATE(victim, 1 * PULSE_VIOLENCE);
	check_improve(ch, gsn_backstab, FALSE, 1);
	send_to_char
	    ("You fall FLAT ON YOUR A$$ as you miss your backstab.\n\r",
	     ch);
	damage(ch, victim, 0, gsn_backstab, DAM_NONE, TRUE);
    }

    return;
}

void do_kick(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;

    if (!IS_NPC(ch)
	&& ch->level < skill_table[gsn_kick].skill_level[ch->class]) {
	send_to_char("You better leave the martial arts to fighters.\n\r",
		     ch);
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	return;

    if ((victim = ch->fighting) == NULL) {
	send_to_char("You aren't fighting anyone.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, skill_table[gsn_kick].beats);
    if (get_skill(ch, gsn_kick) > number_percent()) {
	damage(ch, victim, number_range(ch->level, ch->level * 2.5),
	    gsn_kick, DAM_BASH, TRUE);
	check_improve(ch, gsn_kick, TRUE, 1);
    } else {
	damage(ch, victim, 0, gsn_kick, DAM_BASH, TRUE);
	check_improve(ch, gsn_kick, FALSE, 1);
    }
    check_killer(ch, victim);
    return;
}

bool check_phase(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE(victim))
	return FALSE;

    chance = get_skill(victim, gsn_phase) / 2;

    if (!can_see(victim, ch))
	chance /= 2;

    if (number_percent() >= chance + victim->level - ch->level)
	return FALSE;

    act("Your body phases to avoid $n's attack.", ch, NULL, victim,
	TO_VICT);
    act("$N's body phases to avoid your attack.", ch, NULL, victim,
	TO_CHAR);
    check_improve(victim, gsn_phase, TRUE, 6);
    return TRUE;
}

void do_refly(CHAR_DATA * ch, char *argument)
{
    int race = race_lookup("Dragonkin");

    if (IS_NPC(ch) || ch->race != race) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (!IS_AWAKE(ch)) {
	send_to_char("You have to be awake to be able to fly again.\n\r",
		     ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_FLYING)) {
	send_to_char("You're already flying.\n\r", ch);
	return;
    }

    if (ch->position == POS_RESTING || ch->position == POS_SITTING) {
	send_to_char("You have to get up before you can fly again.\n\r",
		     ch);
	return;
    }

    WAIT_STATE(ch, number_range(1, 2) * PULSE_VIOLENCE);
    SET_BIT(ch->affected_by, AFF_FLYING);
    if (ch->fighting == NULL) {
	act("$n flaps $s wings and is airborn once again.", ch, NULL, NULL,
	    TO_ROOM);
	act("You flap your wings and are airborn once again.", ch, NULL,
	    NULL, TO_CHAR);
    } else {
	act("$n flaps $s wings and is ready for combat.", ch, NULL, NULL,
	    TO_ROOM);
	act("You flap your wings and are ready for combat.",
	    ch, NULL, NULL, TO_CHAR);
    }

}

void do_nofly(CHAR_DATA * ch, char *argument)
{
    int race = race_lookup("Dragonkin");

    if (IS_NPC(ch) || ch->race != race) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (!IS_AWAKE(ch)) {
	send_to_char("You have to be awake to be able to fly again.\n\r",
		     ch);
	return;
    }

    if (!IS_AFFECTED(ch, AFF_FLYING)) {
	send_to_char("You're not flying right now.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, .5 * PULSE_VIOLENCE);
    REMOVE_BIT(ch->affected_by, AFF_FLYING);
    if (ch->fighting == NULL) {
	act("$n lands gracefully, folding $s wings.", ch, NULL, NULL,
	    TO_ROOM);
	act("You land gracefully, folding your wings.", ch, NULL, NULL,
	    TO_CHAR);
    } else {
	act("$n quickly lands, ready for combat...", ch, NULL, NULL,
	    TO_ROOM);
	act("You quickly land, ready for combat...", ch, NULL, NULL,
	    TO_CHAR);
    }
}

/* Rewritten by Vorlin for better fairness and such... 9/11/2000
   Removed victim align damage mods and other things. - Kevin, 7/15/2002 */
void spell_demonfire(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int count;

    /* Base damage here; clerics have a higher range. */
    if (ch->class == class_lookup("Cleric")) {
	dam = dice(level, number_range(7, 8));
    } else {
	dam = dice(level, number_range(6, 7));
    }

    /* Alignment check, neutral/evil can cast, but not if good. */
    if (IS_GOOD(ch)) {
	victim = ch;
	send_to_char("The demons have turned upon you!\n\r", ch);
	dam *= number_range(.5, 1.0);
    }

    if (victim != ch) {
	act("$n calls forth the demons of Hell upon $N!", ch, NULL, victim,
	    TO_ROOM);
	act("$N calls forth the demons of Hell upon you!", ch, NULL,
	    victim, TO_VICT);
	act("You call forth the demons of Hell upon $N!", ch, NULL, victim,
	    TO_CHAR);
    }

/*  Very good victims take more damage.
    if (victim->alignment >= 800)
	dam *= 1.2;
*/

    /* Less for those with saves against it */
    if (saves_spell(level, victim, DAM_FIRE))
	dam *= .8;

    /* Less damage if ch is neutral, not evil */
    if (IS_NEUTRAL(ch))
	dam *= .65;

    /* Damage dealt here */
    for (count = 1; count <= ch->level / 30; count++)
	damage_old(ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
}

/* Redid the RoT because it was lacking some too for clerics... --Vorlin
   Cleaned up Ray of Truth. -- Kevin, 7/15/2001 */
void
spell_ray_of_truth(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int count;

    /* Base damage here; clerics have a higher range. */
    if (ch->class == class_lookup("Cleric")) {
	dam = dice(level, number_range(7, 8));
    } else {
	dam = dice(level, number_range(6, 7));
    }

    if (IS_EVIL(ch)) {
	victim = ch;
	send_to_char("Your insides feel on fire!\n\r", victim);
	dam *= number_range(.5, 1.0);
    }

    if (victim != ch) {
	act("$n raises $s hand, sending a blinding ray of light at $N!",
	    ch, NULL, victim, TO_ROOM);
	act("$n raises $s hand, shooting a blinding ray of light at you!",
	    ch, NULL, victim, TO_VICT);
	act("You raise your hand, sending a blinding ray of light at $N!",
	    ch, NULL, victim, TO_CHAR);
    }

    /* Good victims take less damage. */
    if (victim->alignment >= 800)
	dam *= .8;

    /* Saves, gotta have saves! */
    if (saves_spell(level, victim, DAM_LIGHT))
	dam *= .8;

/*  Less damage if ch is neutral, not good
    if (IS_NEUTRAL(ch))
	dam *= .75;
*/

    for (count = 1; count <= ch->level / 30; count++)
	damage_old(ch, victim, dam, sn, DAM_HOLY, TRUE);
}

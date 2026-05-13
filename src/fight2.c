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

#define MAX_DAMAGE_MESSAGE 41

/* command procedures needed */
DECLARE_DO_FUN(do_flee);

/* Imported functions */
void one_hit args((CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary));

void do_flashkick(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_flashkick)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	|| (!IS_NPC(ch)
	    && ch->level <
	    skill_table[gsn_flashkick].skill_level[ch->class])) {
	send_to_char
	    ("Better leave the martial arts to the ninjas and monks.\n\r",
	     ch);
	return;
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char
		("You do a flashkick and soar high into the air.\n\r", ch);
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
	    ("Now how can you possibly do a flashkick to yourself?!\n\r",
	     ch);
	WAIT_STATE(ch, 2 * skill_table[gsn_flashkick].beats + 1);
	act("`^$n flys high into the air with their flashkick!``", ch,
	    NULL, NULL, TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* modifiers */

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
	act("`^$n connects their flashkick, nice nice!``", ch, NULL,
	    victim, TO_VICT);
	act("`^You quickly do a back somersault and kick your opponent!``",
	    ch, NULL, victim, TO_CHAR);
	act("`4$n flashkicks $N, sending $M sprawling onto the floor!``",
	    ch, NULL, victim, TO_NOTVICT);
	check_improve(ch, gsn_flashkick, TRUE, 1);

	REMOVE_BIT(victim->affected_by, AFF_FLYING);
	WAIT_STATE(victim, (number_range(1, 2) * PULSE_VIOLENCE));
	WAIT_STATE(ch, (number_range(1, 4) * PULSE_VIOLENCE));
	victim->position = POS_RESTING;
	damage(ch, victim,
	       number_range(ch->level * 1.5, 2 + 2 * victim->size),
	       gsn_flashkick, DAM_BASH, TRUE);
    } else {
	damage(ch, victim, 0, gsn_flashkick, DAM_NONE, TRUE);
	WAIT_STATE(ch, (number_range(1, 1) * PULSE_VIOLENCE));
	check_improve(ch, gsn_flashkick, FALSE, 1);
    }
    check_killer(ch, victim);
}

void do_vanishing(CHAR_DATA * ch, char *argument)
{
    int chance;

    if ((chance = get_skill(ch, gsn_vanishing)) == 0 || (!IS_NPC(ch)
							 && ch->level <
							 skill_table
							 [gsn_vanishing].skill_level
	
						 [ch->class])) {
	send_to_char
	    ("Better leave the disappearing acts to the ninjas.\n\r", ch);
	return;
    }
    if (is_affected(ch, gsn_vanishing)) {
	send_to_char("You must go visible before you can vanish.\n\r", ch);
	return;
    }
    if (ch->move < 50) {
	send_to_char("You feel too exhausted to vanish.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_INVISIBLE))
	REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
    if (IS_AFFECTED(ch, AFF_HIDE))
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if (number_percent() < get_skill(ch, gsn_vanishing)) {
	AFFECT_DATA af;

	WAIT_STATE(ch, PULSE_VIOLENCE / 3);
	ch->move -= 50;

	send_to_char
	    ("You become one with the shadows and vanish from existance.\n\r",
	     ch);
	act("$n begins to concentrate and vanishes from existance.", ch,
	    NULL, NULL, TO_ROOM);
	check_improve(ch, gsn_vanishing, TRUE, 3);

	af.where = TO_AFFECTS;
	af.type = gsn_vanishing;
	af.level = ch->level;
	af.duration = ch->level / 5;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char(ch, &af);

	af.modifier = ch->level / 7;
	af.location = APPLY_HITROLL;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	SET_BIT(ch->affected_by, AFF_HIDE);
	return;
    } else {
	send_to_char
	    ("You attempt to vanish out of existance, but fail.\n\r", ch);
	check_improve(ch, gsn_vanishing, FALSE, 3);
	ch->move -= 25;
	return;
    }
}

void do_smoke(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_smoke)) == 0 || (!IS_NPC(ch)
						     && ch->level <
						     skill_table
						     [gsn_smoke].skill_level
						     [ch->class])) {
	send_to_char("You'll end up blinding yourself doing that.\n\r",
		     ch);
	return;
    }


    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("But you aren't in combat!\n\r", ch);
	    return;
	}
    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_BLIND) || is_affected(victim, gsn_smoke)) {
	act("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (victim == ch) {
	send_to_char("There would be no point in doing that.\n\r", ch);
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
	chance += 15;
    if (IS_SET(victim->off_flags, OFF_FAST)
	|| IS_AFFECTED(victim, AFF_HASTE))
	chance -= 30;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* now the attack */
    if (number_percent() < chance) {
	AFFECT_DATA af;
	act("$n is blinded by smoke in $s eyes!", victim, NULL, NULL,
	    TO_ROOM);
	act("Your eyes tear up as $n throws a smoke bomb at you feet!", ch,
	    NULL, victim, TO_VICT);

	damage(ch, victim, number_range(ch->level / 2, ch->level),
	       gsn_smoke, DAM_FIRE, TRUE);

	send_to_char("You can't see a thing!\n\r", victim);
	check_improve(ch, gsn_smoke, TRUE, 2);
	WAIT_STATE(ch, skill_table[gsn_smoke].beats);

	af.where = TO_AFFECTS;
	af.type = gsn_smoke;
	af.level = ch->level;
	af.duration = 0;	/* Duration was ch->level/30 */
	af.location = APPLY_HITROLL;
	af.modifier = (ch->level / 18) * -1;
	af.bitvector = AFF_BLIND;
	affect_to_char(victim, &af);

	af.location = APPLY_DAMROLL;
	af.modifier = (ch->level / 18) * -1;
	affect_to_char(victim, &af);
	do_flee(ch, "");
    } else {
	act("$n smoke bomb explodes near you with no affect.\n\r", ch,
	    NULL, NULL, TO_ROOM);
	send_to_char
	    ("Your smoke bomb fails to hit at your opponent's feet.\n\r",
	     ch);
	damage(ch, victim, 0, gsn_smoke, DAM_NONE, TRUE);
	check_improve(ch, gsn_smoke, FALSE, 2);
	WAIT_STATE(ch, skill_table[gsn_smoke].beats);
    }
    check_killer(ch, victim);
}

void do_drain(CHAR_DATA * ch, char *argument)
{
    int jizz;
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument(argument, arg);

/*  This will need to be modified for rom 2.4 clan formats
    
    if ( ch->guild_info == NULL )
    {
        send_to_char ("Huh?\n\r", ch);
        return;
    }
        
        
    if (str_cmp( ch->guild_info->color, "VAMPIRE"))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
*/
    if (IS_NPC(ch))
	return;

    if (ch->race != race_lookup("vampire")) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || !str_cmp(arg, ch->name)) {
	act("$n nips at his wrists.", ch, NULL, NULL, TO_ROOM);
	send_to_char("You can only feed on the dead.\n\r", ch);
	return;
    }

    obj = get_obj_list(ch, arg, ch->in_room->contents);
    if (obj == NULL) {
	send_to_char("You can only feed on the dead.\n\r", ch);
	return;
    }

    if (obj->item_type == ITEM_CORPSE_NPC
	|| obj->item_type == ITEM_CORPSE_PC) {
	if (obj->level < 10 && obj->level != 0) {
	    send_to_char
		("The corpse doesn't have enough blood to fill you.\n\r",
		 ch);
	    obj->level = 0;
	    obj->description = str_dup("A `&pale`` corpse rests here.");
	    return;
	}
	jizz = number_range(4, 10);
	if (obj->level == 0) {
	    AFFECT_DATA af;
	    act("$n `@c`2h`@o`2k`@e`2s`` and coughs up blood.", ch, NULL,
		NULL, TO_ROOM);
	    send_to_char("You `@c`2h`@o`2k`@e`` on the rotten blood.\n\r",
			 ch);

	    af.type = gsn_poison;
	    af.level = number_fuzzy(jizz);
	    af.duration = 5 * jizz;
	    af.location = APPLY_NONE;
	    af.modifier = 0;
	    af.bitvector = AFF_POISON;
	    affect_join(ch, &af);
	    obj->description = str_dup("A `&pale`` corpse rests here.");
	    return;
	}
	act("$n `8dr`7ai`8ns`` the corpse of it's life `!b`1l`!o`1o`!d``.",
	    ch, NULL, NULL, TO_ROOM);
	send_to_char
	    ("`8Warmth`` fills your body as `!b`1l`!o`1o`!d`` fills your veins.\n\r",
	     ch);
	gain_condition(ch, COND_FULL, 40);
	gain_condition(ch, COND_THIRST, 40);
	gain_condition(ch, COND_HUNGER, 40);
	ch->hit = ch->hit + (obj->level * 2);
	if (ch->hit > ch->max_hit)
	    ch->hit = ch->max_hit;
	obj->level -= 15;
	obj->description =
	    str_dup("A slightly `&pale`` corpse rests here.");
	return;
    } else {
	send_to_char("You hurt your teeth!", ch);
	return;
    }
}

void do_kamehameha(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int count;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_kamehameha)) == 0 || (!IS_NPC(ch)
							  && ch->level <
							  skill_table
							  [gsn_kamehameha].skill_level
							  [ch->class])) {
	send_to_char("Only Saiyans can use the Kamehameha technique.\n\r",
		     ch);
	return;
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char
		("Whom are you trying to hit with this technique?\n\r",
		 ch);
	    return;
	}
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

    if (is_safe(ch, victim))
	return;

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }

    if (ch->move <= 50 || ch->hit <= 50) {
	send_to_char
	    ("You don't have enough in you to perform this technique!\n\r",
	     ch);
	return;
    }

    if (victim == ch) {
	send_to_char("You can not do that to yourself.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    /* I mucked with this because it was a wreck...again */
    /* --Vorlin, 8/22/2000 */

    /* modifiers */

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

/* slow check? */
    if (IS_AFFECTED(ch, AFF_SLOW)) {
	chance -= number_range(10, 20);
    } else {
	chance += 5;
    }

    /*  now the attack */
    if (number_percent() < chance) {
	act
	    ("$n draws back $s fists and yells `8'`#KAME`3-`#HAME`3-`#HA`8'`7!!!\n\rThen releases a powerful blast from $s hands towards you!",
	     ch, NULL, victim, TO_VICT);
	act
	    ("You draw back your fists and yell `8'`#KAME`3-`#HAME`3-`#HA`8'`7!!!\n\rThen release a powerful blast towards $N!",
	     ch, NULL, victim, TO_CHAR);
	act
	    ("$n draws back $s fists and yells `8'`#KAME`3-`#HAME`3-`#HA`8'`7!!!\n\rThen relases a powerful blast towards $N!",
	     ch, NULL, victim, TO_NOTVICT);
	check_improve(ch, gsn_kamehameha, TRUE, 1);

	WAIT_STATE(ch, (number_range(1.5, 3) * PULSE_VIOLENCE));
	{
	    for (count = 1; count <= (number_range(2, ch->level / 15));
		 count++)
		damage(ch, victim,
		       number_range(ch->level * 1.5, ch->level * 3),
		       gsn_kamehameha, DAM_ENERGY, TRUE);
	}
	ch->move -= number_range(25, 50);
    } else {
	act("You fail to connect your kamehameha...", ch, NULL, victim,
	    TO_CHAR);
	act("$n's kamehameha totally misses you...", ch, NULL, victim,
	    TO_VICT);
	act("$n fails to hit $N with $s kamehameha...", ch, NULL, victim,
	    TO_NOTVICT);
	WAIT_STATE(ch, number_range(1.5, 2.5) * PULSE_VIOLENCE);
	check_improve(ch, gsn_kamehameha, FALSE, 1);
	ch->move -= number_range(20, 40);
    }
    check_killer(ch, victim);
    return;
}

void do_pounce(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int chance, count, i;
    long dam;


    one_argument(argument, arg);

    if (IS_NPC(ch))
	return;

    if ((chance = get_skill(ch, gsn_pounce)) == 0
	|| (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_KICK))
	|| (!IS_NPC(ch)
	    && ch->level <
	    skill_table[gsn_pounce].skill_level[ch->class])) {
	send_to_char
	    ("You try to imitate the feline's graceful pounce, and fail miserably.\n\r", ch);
	return;
    }


    if (arg[0] == '\0' && ch->fighting == NULL) {
	send_to_char("Whom are you trying to hit with this technique?\n\r", ch);
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
	send_to_char("How can you possibly pounce on yourself?\n\r", ch);
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
		act("$n pounces forward, tackling you to the ground!", ch, NULL, victim, TO_VICT);
		act("`^You gracefully leap forward, pouncing on $N!``", ch, NULL, victim, TO_CHAR);
		act("$n leaps through the air, pouncing on $N!", ch, NULL, victim, TO_NOTVICT);

		// Number Of Pounce Hits
	    count = number_range(2, ch->level / 20);

	    REMOVE_BIT(victim->affected_by, AFF_FLYING);
		WAIT_STATE(ch, (count * PULSE_VIOLENCE) / 2);
		WAIT_STATE(victim, .5 * PULSE_VIOLENCE);
		victim->position = POS_RESTING;

		// Cost of Pounce
		if (ch->level <= 50) {
	    	ch->move -= number_range(4, 8);
		} else {
	    	ch->move -= number_range(10, 16);
		}

		check_improve(ch, gsn_pounce, TRUE, 1);

		dam = number_range(ch->level * 1, ch->level * 2) + number_range(ch->move / 14, ch->move / 10);
		for (i = 1; i <= count; i++) {
		    damage(ch, victim, dam, gsn_pounce, DAM_BASH, TRUE);
		}
    } else {
    	act("`^$n attempts to pounce you but fails!``", ch, NULL, victim, TO_VICT);
		act("`^You attemp a pounce but fail!``", ch, NULL, victim, TO_CHAR);
		act("`4$n attempts to pounce $N but fails!``", ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_pounce, FALSE, 1);
		ch->move -= number_range(10, 20);
		WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
		damage(ch, victim, 0, gsn_pounce, DAM_NONE, TRUE);
    }
    check_killer(ch, victim);
    return;
}

void do_rapidstrike(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument, arg);

    if ((chance = get_skill(ch, gsn_rapid_strike)) == 0
	|| IS_NPC(ch)
	|| (!IS_NPC(ch) 
	    && ch->level <
	    skill_table[gsn_rapid_strike].skill_level[ch->class])) {
	send_to_char("Rapid strike? Better leave that to the warriors.\n\r", ch);
	return;
    }

    /*
    if (ch->rstrike > 2) {
	send_to_char("You've exhausted yourself from the rapid hits, catch your breath.\n\r", ch);
	return;
    }
    */

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    WAIT_STATE(ch, PULSE_VIOLENCE);
	    send_to_char("You do a rapid strike, attacking the air with finesse.\n\r", ch);
	    /* ch->rstrike++; */
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

    if (ch->move < 50) {
	send_to_char("You're too tired to use this attack.\n\r", ch);
	return;
    }

    if (IS_NPC(victim) &&
	victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
	send_to_char("Kill stealing is not permitted.\n\r", ch);
	return;
    }
     
    if (victim == ch) {
	send_to_char("You smack yourself in the head and become dizzy. @_@\n\r", ch);
	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	act("$n punches $mself in the head, falling to the floor with swirly eyes. @_@", ch,
	    NULL, NULL, TO_ROOM);
	return;
    }
    
    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
	return;
    }

    chance += get_curr_stat(ch, STAT_DEX);
    chance -= get_curr_stat(victim, STAT_DEX);

    chance += (ch->level - victim->level) / 2;

    chance -= get_skill(victim, gsn_dodge) / 7;
    if (IS_NPC(victim)) {
	chance -= get_skill(victim, gsn_phase) / 7;
    } else {
	chance -= get_skill(victim, gsn_phase) / 5;
    }

    if (number_percent() < chance) {
	act("`^$n lunges forward, striking at $N!``", ch, NULL, victim, TO_NOTVICT);
	act("`^$n quickly lunges towards you!``", ch, NULL, victim, TO_VICT);
	act("`^You lash out at $N in a rapid strike!``", ch, NULL, victim, TO_CHAR);
	check_improve(ch, gsn_rapid_strike, TRUE, 1);

	WAIT_STATE(ch, PULSE_VIOLENCE);
	/* ch->rstrike++; */
	one_hit(ch, victim, gsn_rapid_strike, FALSE);
	ch->move -= number_range(6, 12);
    } else {
	act("`^$n attempts to lunge at $N and misses.``", ch, NULL, victim, TO_NOTVICT);
	act("`^$n attempts to lunge at you, but you dodge.``", ch, NULL, victim, TO_VICT);
	act("`^You try to lunge at $N, but $E dodges.``", ch, NULL, victim, TO_CHAR);
	check_improve(ch, gsn_rapid_strike, FALSE, 1);

	WAIT_STATE(ch, PULSE_VIOLENCE);
	/* ch->rstrike++; */
	damage(ch, victim, 0, gsn_rapid_strike, DAM_NONE, TRUE);
	ch->move -= number_range(4, 8);
    }
    check_killer(ch, victim);
    return;
}

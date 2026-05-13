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

/* command procedures needed */
DECLARE_DO_FUN(do_backstab);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_berserk);
DECLARE_DO_FUN(do_bash);
DECLARE_DO_FUN(do_trip);
DECLARE_DO_FUN(do_dirt);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_disarm);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_look);


/*
* Local functions.
*/
void dam_message
args((CHAR_DATA * ch, CHAR_DATA * victim, long dam, int dt, bool immune));
void death_cry args((CHAR_DATA * ch));
void group_gain args((CHAR_DATA * ch, CHAR_DATA * victim));
int xp_compute
args((CHAR_DATA * gch, CHAR_DATA * victim, int total_levels));
void make_corpse args((CHAR_DATA * ch, char *description));

/*EE960524*/
void one_hit args((CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary));
void mob_hit args((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
void raw_kill args((CHAR_DATA * victim, char *description));
void set_fighting args((CHAR_DATA * ch, CHAR_DATA * victim));
void disarm args((CHAR_DATA * ch, CHAR_DATA * victim));
void save_char_died_obj args((CHAR_DATA * ch));


/* imported functions */
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

/* Helper functions for multi_hit */
int calc_chance(CHAR_DATA *ch, CHAR_DATA *victim, int sn, int chance, int mod1min, int mod1max, int mod2min, int mod2max);
void attempt_secondary_strike(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int chance, int tier, bool racial_adjustment);

/*
* Control the fights going on.
* Called periodically by update_handler.
*/
void violence_update(void)
{
    CHAR_DATA *ch, *ch_next, *victim;

    /* For OLC */
    for (ch = char_list; ch != NULL; ch = ch_next) {
    ch_next = ch->next;

    /* Rapid strike lag! One round for each increment past the first.
    if (ch->rstrike >= 2) {
        WAIT_STATE(ch, (ch->rstrike - 1) * PULSE_VIOLENCE);
        ch->rstrike = 0;
    }
    */

    if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
        continue;

    if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
        multi_hit(ch, victim, TYPE_UNDEFINED);
    else
        stop_fighting(ch, FALSE);

    if ((victim = ch->fighting) == NULL)
        continue;

    /*
     * Fun for the whole family!
     */
    check_assist(ch, victim);
    if ( IS_NPC( ch ) )
      {
        if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
          mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
        if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
          mp_hprct_trigger( ch, victim );
      }
    }
    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA * ch, CHAR_DATA * victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next) {
    rch_next = rch->next_in_room;

    if (IS_AWAKE(rch) && rch->fighting == NULL) {

        /* quick check for ASSIST_PLAYER */
        if ((!IS_NPC(ch) && IS_NPC(rch)
         && IS_SET(rch->off_flags, ASSIST_PLAYERS))) {
        do_emote(rch, "screams and attacks!");
        multi_hit(rch, victim, TYPE_UNDEFINED);
        continue;
        }

        /* PCs next */
        if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)) {
        if (((!IS_NPC(rch) && IS_SET(rch->act, PLR_AUTOASSIST))
             || IS_AFFECTED(rch, AFF_CHARM))
            && is_same_group(ch, rch) && !is_safe(rch, victim))
            multi_hit(rch, victim, TYPE_UNDEFINED);

        continue;
        }

        /* now check the NPC cases */
        if (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM)) {
        if ((IS_NPC(rch) && IS_SET(rch->off_flags, ASSIST_ALL))
            || (IS_NPC(rch) && rch->group
            && rch->group == ch->group) || (IS_NPC(rch)
                            && rch->race ==
                            ch->race
                            &&
                            IS_SET
                            (rch->off_flags,
                             ASSIST_RACE))
            || (IS_NPC(rch) && IS_SET(rch->off_flags, ASSIST_ALIGN)
            && ((IS_GOOD(rch) && IS_GOOD(ch))
                || (IS_EVIL(rch) && IS_EVIL(ch))
                || (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))
            || (rch->pIndexData == ch->pIndexData
            && IS_SET(rch->off_flags, ASSIST_VNUM))) {
            CHAR_DATA *vch, *target;
            int number = 0;

            if (number_bits(1) == 0)
            continue;

            target = NULL;

            for (vch = ch->in_room->people; vch; vch = vch->next) {
            if (can_see(rch, vch)
                && is_same_group(vch, victim)
                && number_range(0, number) == 0) {
                target = vch;
                number++;
            }
            }

            if (target != NULL) {
            do_emote(rch, "screams and attacks!");
            multi_hit(rch, target, TYPE_UNDEFINED);
            }
        }
        }
    }
    }
}



/*
* Do one group of attacks.
*/
void multi_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
    int chance;

    /* decrement the wait */
    if (ch->desc == NULL)
        ch->wait = UMAX(0, ch->wait - PULSE_VIOLENCE);

    if (ch->desc == NULL)
        ch->daze = UMAX(0, ch->daze - PULSE_VIOLENCE);

    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
        return;

    if (IS_NPC(ch)) {
        mob_hit(ch, victim, dt);
        return;
    }

    /*****************
     * First Attack  *
     *****************/
    one_hit(ch, victim, dt, FALSE);
    if (ch->fighting != victim)
        return;
    // Extra Hit from haste buff
    if (IS_AFFECTED(ch, AFF_HASTE)) {
        one_hit(ch, victim, dt, FALSE);
        if (ch->fighting != victim)
            return;
    }
    chance = calc_chance(ch, victim, gsn_dual_wield, 1, 2, 2, 3, 3);
    attempt_secondary_strike(ch, victim, dt, chance, 4, FALSE);
    if (ch->fighting != victim)
        return;

    // Stop Here In Case Of Backstab and Circle
    if (dt == gsn_backstab || dt == gsn_circle)
        return;

    /*****************
     * Second Attack *
     *****************/
    chance = calc_chance(ch, victim, gsn_second_attack, 1, 2, 2, 3, 3);
    if (number_percent() < chance) {
        one_hit(ch, victim, dt, FALSE);
        check_improve(ch, gsn_second_attack, TRUE, 5);
        if (ch->fighting != victim)
            return;
        chance = calc_chance(ch, victim, gsn_dual_wield, 1, 2, 2, 3, 3);
        attempt_secondary_strike(ch, victim, dt, chance, 5, TRUE);
        if (ch->fighting != victim)
            return;
    }
    /*****************
     * Third Attack  *
     *****************/
    chance = calc_chance(ch, victim, gsn_third_attack, 1, 2, 3, 3, 4);
    if (number_percent() < chance) {
        one_hit(ch, victim, dt, FALSE);
        check_improve(ch, gsn_third_attack, TRUE, 6);
        if (ch->fighting != victim)
            return;
        chance = calc_chance(ch, victim, gsn_dual_wield, 1, 2, 3, 3, 4);
        attempt_secondary_strike(ch, victim, dt, chance, 6, TRUE);
        if (ch->fighting != victim)
            return;
    }
    /*****************
     * Fourth Attack *
     *****************/
    chance = calc_chance(ch, victim, gsn_fourth_attack, 2, 3, 4, 4, 5);
    if (number_percent() < chance) {
        one_hit(ch, victim, dt, FALSE);
        check_improve(ch, gsn_fourth_attack, TRUE, 7);
        if (ch->fighting != victim)
            return;

        chance = calc_chance(ch, victim, gsn_dual_wield, 2, 3, 4, 4, 5);
        attempt_secondary_strike(ch, victim, dt, chance, 7, TRUE);
        if (ch->fighting != victim)
            return;
    }
    /*****************
     * Fifth Attack  *
     *****************/
    chance = calc_chance(ch, victim, gsn_fifth_attack, 3, 4, 5, 5, 6);
    if (number_percent() < chance) {
        one_hit(ch, victim, dt, FALSE);
        check_improve(ch, gsn_fifth_attack, TRUE, 8);
        if (ch->fighting != victim)
            return;
        chance = calc_chance(ch, victim, gsn_dual_wield, 3, 4, 5, 5, 6);
        attempt_secondary_strike(ch, victim, dt, chance, 8, TRUE);
        if (ch->fighting != victim)
            return;
    }
    /*****************
     * Sixth Attack  *
     *****************/
    chance = calc_chance(ch, victim, gsn_sixth_attack, 4, 6, 6, 7, 7);
    if (number_percent() < chance) {
        one_hit(ch, victim, dt, FALSE);
        check_improve(ch, gsn_sixth_attack, TRUE, 10);
        if (ch->fighting != victim)
            return;
        chance = calc_chance(ch, victim, gsn_dual_wield, 4, 6, 6, 7, 7);
        attempt_secondary_strike(ch, victim, dt, chance, 10, TRUE);
    }
    return;
}

int calc_chance(CHAR_DATA *ch, CHAR_DATA *victim, int skill, int base_div,
                int slow_min, int slow_max, int cry_min, int cry_max)
{
    // Get Base Chance
    int chance = get_skill(ch, skill) / base_div;

    // Early Out if Chance is Zero (eg. the character doesn't know the skill)
    if (chance <= 0)
        return 0;

    // Warrior, Ninja, and Monk Class Bonus
    if (ch->class == class_lookup("Warrior"))
        chance += 20;
    if (ch->class == class_lookup("Ninja") || ch->class == class_lookup("Monk"))
        chance += 10;
    // Haste Effects
    if (IS_AFFECTED(ch, AFF_HASTE) || IS_SET(ch->off_flags, OFF_FAST))
        chance += number_range(3, 4);
    if (victim && (IS_AFFECTED(victim, AFF_HASTE) || IS_SET(victim->off_flags, OFF_FAST)))
        chance -= number_range(3, 4);
    // Debuff Effects
    if (IS_AFFECTED(ch, AFF_SLOW))
        chance -= number_range(slow_min, slow_max);
    if (is_affected(ch, skill_lookup("cry")))
        chance -= number_range(cry_min, cry_max);

    return URANGE(0, chance, 100);
}

bool is_unarmed_ninja(CHAR_DATA *ch)
{
    return (ch->class == class_lookup("Ninja") &&
            get_eq_char(ch, WEAR_WIELD) == NULL &&
            get_eq_char(ch, WEAR_SECONDARY) == NULL);
}

void attempt_secondary_strike(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int base_chance,
                              int improve_rating, bool racial_adjustment)
{
    bool has_secondary = get_eq_char(ch, WEAR_SECONDARY) != NULL;
    bool unarmed_ninja = is_unarmed_ninja(ch);
    int chance = base_chance;
    if (racial_adjustment) {
        if (ch->race == race_lookup("Dark-Elf"))
            chance *= .6;
        else
            chance *= .5;
    }
    if ((has_secondary || unarmed_ninja) && number_percent() < chance) {
        one_hit(ch, victim, dt, has_secondary);
        check_improve(ch, gsn_dual_wield, TRUE, improve_rating);
    }
}

/* procedure for all mobile attacks */
void mob_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
    int chance, number;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch, victim, dt, FALSE);

    if (ch->fighting != victim)
    return;

    /* Area attack -- BALLS nasty! */

    if (IS_SET(ch->off_flags, OFF_AREA_ATTACK)) {
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
        vch_next = vch->next;
        if ((vch != victim && vch->fighting == ch))
        one_hit(ch, vch, dt, FALSE);
    }
    }

    if (IS_AFFECTED(ch, AFF_HASTE)
    || (IS_SET(ch->off_flags, OFF_FAST) && !IS_AFFECTED(ch, AFF_SLOW)))
    one_hit(ch, victim, dt, FALSE);

    if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
    return;

    chance = get_skill(ch, gsn_second_attack) / 2;

    if (IS_AFFECTED(ch, AFF_SLOW) && !IS_SET(ch->off_flags, OFF_FAST))
    chance /= 2;

    if (is_affected(ch, skill_lookup("cry")))
    chance /= 4;

    if (number_percent() < chance) {
    one_hit(ch, victim, dt, FALSE);
    if (ch->fighting != victim)
        return;
    }

    chance = get_skill(ch, gsn_third_attack) / 4;

    if (IS_AFFECTED(ch, AFF_SLOW) && !IS_SET(ch->off_flags, OFF_FAST))
    chance = 0;

    if (number_percent() < chance) {
    one_hit(ch, victim, dt, FALSE);
    if (ch->fighting != victim)
        return;
    }

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
    return;

    number = number_range(0, 2);

    if (number == 1 && (IS_SET(ch->act, ACT_MAGE))) {
    /*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET(ch->act, ACT_CLERIC)) {
    /* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range(0, 8);

    switch (number) {
    case (0):
    if (IS_SET(ch->off_flags, OFF_BASH))
        do_bash(ch, "");
    break;

    case (1):
    if (IS_SET(ch->off_flags, OFF_BERSERK)
        && !IS_AFFECTED(ch, AFF_BERSERK))
        do_berserk(ch, "");
    break;


    case (2):
    if (IS_SET(ch->off_flags, OFF_DISARM)
        || (get_weapon_sn(ch) != gsn_hand_to_hand
        && (IS_SET(ch->act, ACT_WARRIOR)
            || IS_SET(ch->act, ACT_MONK)
            || IS_SET(ch->act, ACT_THIEF)))) do_disarm(ch, "");
    break;

    case (3):
    if (IS_SET(ch->off_flags, OFF_KICK))
        do_kick(ch, "");
    break;

    case (4):
    if (IS_SET(ch->off_flags, OFF_KICK_DIRT))
        do_dirt(ch, "");
    break;

    case (5):
    if (IS_SET(ch->off_flags, OFF_TAIL)) {
        /* do_tail(ch,"") */ ;
    }
    break;

    case (6):
    if (IS_SET(ch->off_flags, OFF_TRIP))
        do_trip(ch, "");
    break;

    case (7):
    if (IS_SET(ch->off_flags, OFF_CRUSH)) {
        /* do_crush(ch,"") */ ;
    }
    break;
    case (8):
    if (IS_SET(ch->off_flags, OFF_BACKSTAB)) {
        do_backstab(ch, "");
    }
    }
}


/*
* Hit one guy once.
*/
void one_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary)
{

    OBJ_DATA *wield;
    int victim_ac, thac0, thac0_00, thac0_32, dam = 0, diceroll, dam_type,
    weapon_skill_name = -1, skill_multiplier, chance = 0, vorpalc = 0;
    bool result;

    /* just in case */
    /* 
       Added the PLR_DEAD checks so people don't get killed or do killing
       when they're dead 
     */
    if ((!IS_NPC(victim) && !IS_NPC(ch))
    && (victim == ch
        || ch == NULL
        || victim == NULL
        || IS_SET(victim->act, PLR_DEAD) || IS_SET(ch->act, PLR_DEAD)))
    return;

    /*
       * Can't beat a dead char!
       * Guard against weird room-leavings.
     */
    if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
    return;

    /*
       * Figure out the type of damage message.
     */

    if (secondary)      /*EE960524 */
        wield = get_eq_char(ch, WEAR_SECONDARY);
    else
        wield = get_eq_char(ch, WEAR_WIELD);

    if (dt == TYPE_UNDEFINED) {
        dt = TYPE_HIT;
        if (wield != NULL && wield->item_type == ITEM_WEAPON)
            dt += wield->value[3];
        else
            dt += ch->dam_type;
    }
    
    if (dt < TYPE_HIT) {
        if (wield != NULL)
            dam_type = attack_table[wield->value[3]].damage;
        else
            dam_type = attack_table[ch->dam_type].damage;
    } else {
        dam_type = attack_table[dt - TYPE_HIT].damage;
    }

    if (dam_type == -1)
        dam_type = DAM_BASH;

    /* get the weapon skill */
    weapon_skill_name = get_weapon_sn(ch);
    skill_multiplier = 20 + get_weapon_skill(ch, weapon_skill_name);

    /* If weapon is flagged vorpal, chance of extra damage */
    if (wield != NULL && wield->item_type == ITEM_WEAPON && IS_WEAPON_STAT(wield, WEAPON_VORPAL)) {
        chance = number_range(1, 1000);

        if (chance == 1) {
            act("$p flares blue and vibrates tremendously in your hands.",
            ch, wield, NULL, TO_CHAR);
            act("$p flares blue and vibrates tremendously in their hands.",
            ch, wield, NULL, TO_ROOM);
            act("$p flares blue and vibrates tremendously in their hands.",
            ch, wield, victim, TO_VICT);
            vorpalc = 3;
        } else if (chance <= 10) {
            act("$p glows blue and vibrates violently in your hands.", ch,
            wield, NULL, TO_CHAR);
            act("$p glows blue and vibrates violently in their hands.", ch,
            wield, NULL, TO_ROOM);
            act("$p glows blue and vibrates violently in their hands.", ch,
            wield, victim, TO_VICT);
            vorpalc = 2;
        } else if (chance <= 50) {
            act("$p shimmers blue and vibrates in your hands.", ch, wield,
            NULL, TO_CHAR);
            act("$p shimmers blue and vibrates in their hands.", ch, wield,
            NULL, TO_ROOM);
            act("$p shimmers blue and vibrates in their hands.", ch, wield,
            victim, TO_VICT);
            vorpalc = 1;
        }
    }

    /*
       * Calculate to-hit-armor-class-0 versus armor.
     */
    if (IS_NPC(ch)) {
    thac0_00 = 20;
    thac0_32 = -4;      /* as good as a thief */

    if (IS_SET(ch->act, ACT_WARRIOR))
        thac0_32 = -10;
    else if (IS_SET(ch->act, ACT_MONK))
        thac0_32 = -10;
    else if (IS_SET(ch->act, ACT_THIEF))
        thac0_32 = -4;
    else if (IS_SET(ch->act, ACT_CLERIC))
        thac0_32 = 2;
    else if (IS_SET(ch->act, ACT_MAGE))
        thac0_32 = 6;

    } else {
    thac0_00 = class_table[ch->class].thac0_00;
    thac0_32 = class_table[ch->class].thac0_32;
    }

    thac0 = interpolate(ch->level, thac0_00, thac0_32);

    if (thac0 < 0)
        thac0 /= 2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill_multiplier / 100;
    thac0 += (100 - skill_multiplier) / 20;

    if (dt == gsn_backstab)
        thac0 -= 10 * (100 - get_skill(ch, gsn_backstab));
    else if (dt == gsn_circle)
        thac0 -= 10 * (100 - get_skill(ch, gsn_circle));


    switch (dam_type) {
    case DAM_PIERCE:
        victim_ac = GET_AC(victim, AC_PIERCE) / 10;
        break;
    case DAM_BASH:
        victim_ac = GET_AC(victim, AC_BASH) / 10;
        break;
    case DAM_SLASH:
        victim_ac = GET_AC(victim, AC_SLASH) / 10;
        break;
    default:
        victim_ac = GET_AC(victim, AC_EXOTIC) / 10;
        break;
    };

    if (victim_ac < -15)
        victim_ac = (victim_ac + 15) / 5 - 15;

    if (!can_see(ch, victim))
        victim_ac -= 4;

    if (victim->position < POS_FIGHTING)
        victim_ac += 4;

    if (victim->position < POS_RESTING)
        victim_ac += 6;

    /*
       * The moment of excitement!
     */
    while ((diceroll = number_bits(5)) >= 20);

    if (dt != gsn_rapid_strike) {
        if (diceroll == 0 || (diceroll != 19 && (diceroll < (thac0 - victim_ac)))) {
            /* Miss. */
            damage(ch, victim, 0, dt, dam_type, TRUE);
            tail_chain();
            return;
        }
    }

    if (!IS_NPC(ch) && weapon_skill_name != -1) // sn -1 is exotic 
        check_improve(ch, weapon_skill_name, TRUE, 5);

    /*
     * Base Damage
     */
    if (IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL)) {
        // Legacy NPCs or Unarmed NPCs
        if (!ch->pIndexData->new_format) {
            dam = number_range(ch->level / 2, ch->level * 3 / 2);
            if (wield != NULL)
                dam += dam / 2;
        } else {
            dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
        }
    } else {
        // Everyone Else: Players and New Format NPCs
        if (wield != NULL) {
            if (wield->pIndexData->new_format) {
                int base_min = wield->value[1];
                int base_max = wield->value[2];
                dam = dice(base_min, base_max) * skill_multiplier / 100;
            } else {
                int base_min = wield->value[1] * skill_multiplier / 100;
                int base_max = wield->value[2] * skill_multiplier / 100;
                dam = number_range(base_min, base_max);
            }
        } else {
            int min = ch->level / 3 * skill_multiplier / 100;
            int max = 2 * ch->level / 3 * skill_multiplier / 100;
            dam = number_range(min, max);
        }
    }

    /*
       * Small Bonus if Victim is Sleeping or Resting
       * NOTE: This happens prior to adding Damage Rolls (if it moves after, then change math)
     */
    if (!IS_AWAKE(victim))
        dam = TIMES_2(dam);
    else if (victim->position < POS_FIGHTING)
        dam = BOOST_50_PERCENT(dam);

    /*
       * Small Bonuses for Skills Enhanced Damage and Critical Strike
       * NOTE: This happens prior to adding Damage Rolls (if it moves after, then change math)
     */
    if (get_skill(ch, gsn_enhanced_damage) > 0) {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch, gsn_enhanced_damage)) {
            check_improve(ch, gsn_enhanced_damage, TRUE, 6);
            dam += 2 * (dam * diceroll / 300);
        }
    }
    if (get_skill(ch, gsn_critical_strike) > 0) {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch, gsn_critical_strike)) {
            check_improve(ch, gsn_critical_strike, TRUE, 6);
            dam += 3 * (dam * diceroll / 300);
        }
    }

    /*
       * Bonus From Damage Rolls * Skill Multiplier (range is from 1 to 1.2)
       * This is place were players who are min maxing can really break the game!
     */
    dam += GET_DAMROLL(ch) * UMIN(100, skill_multiplier) / 100;
    if (dam <= 1)
        dam = 1;

    /*
       * Bonuses for Attacks (backstab, circle, rapid strike)
     */
    if (dt == gsn_circle || dt == gsn_backstab) {
        if (wield != NULL) {
            if (wield->value[0] == DAM_PIERCE || IS_WEAPON_STAT(wield, WEAPON_DAGGER)) {
                dam = BOOST_75_PERCENT(dam);
            } else {
                dam = BOOST_50_PERCENT(dam);
            }
        } else if (is_unarmed_ninja(ch)) {
            dam = BOOST_50_PERCENT(dam);
        }
    } else if (dt == gsn_rapid_strike) {
        dam = TIMES_2(dam);
    }

    /*
       * Bonuses for Weapon Type
     */
    if (wield != NULL && IS_WEAPON_STAT(wield, WEAPON_TWO_HANDS))
        dam = BOOST_20_PERCENT(dam);
    if (wield != NULL && IS_WEAPON_STAT(wield, WEAPON_SHARP) && number_percent() <= (skill_multiplier / 8))
        dam = BOOST_25_PERCENT(dam);
    switch (vorpalc) {
    case 1:
        dam = BOOST_50_PERCENT(dam);
        break;
    case 2:
        dam = BOOST_75_PERCENT(dam);
        break;
    case 3:
        dam = TIMES_4(dam);
        break;
    default:
        break;
    }

    if (!check_counter(ch, victim, dam, dt))
        result = damage(ch, victim, dam, dt, dam_type, TRUE);
    else
        return;


    /*  result = damage( ch, victim, dam, dt, dam_type, TRUE ); */

    /* but do we have a funky weapon? */
    if (result && wield != NULL) {
    int dam;

    if (ch->fighting == victim && IS_WEAPON_STAT(wield, WEAPON_POISON)) {
        int level;
        AFFECT_DATA *poison, af;

        if ((poison = affect_find(wield->affected, gsn_poison)) ==
        NULL) level = wield->level;
        else
        level = poison->level;

        if (!saves_spell(level / 2, victim, DAM_POISON)) {
        send_to_char
            ("You feel poison coursing through your veins.\n\r",
             victim);
        act("$n is poisoned by the venom on $p.", victim, wield,
            NULL, TO_ROOM);

        af.where = TO_AFFECTS;
        af.type = gsn_poison;
        af.level = level * 3 / 4;
        af.duration = level / 2;
        af.location = APPLY_STR;
        af.modifier = -1;
        af.bitvector = AFF_POISON;
        affect_join(victim, &af);
        }

        /* weaken the poison if it's temporary */
        if (poison != NULL) {
        poison->level = UMAX(0, poison->level - 2);
        poison->duration = UMAX(0, poison->duration - 1);

        if (poison->level == 0 || poison->duration == 0)
            act("The poison on $p has worn off.", ch, wield, NULL,
            TO_CHAR);
        }
    }

    /* Vampiric flags hosed alignment: moderate annoyance. -- Kevin, 4/15/02 */
    if (ch->fighting == victim
        && IS_WEAPON_STAT(wield, WEAPON_VAMPIRIC)) {
        dam = number_range(1, wield->level / 5 + 1);
        act("$p draws life from $n.", victim, wield, NULL, TO_ROOM);
        act("You feel $p drawing your life away.",
        victim, wield, NULL, TO_CHAR);
        damage_old(ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
        /* ch->alignment = UMAX(-1000, ch->alignment - 1); */
        ch->hit += dam / 2;
    }

    if (ch->fighting == victim
        && IS_WEAPON_STAT(wield, WEAPON_FLAMING)) {
        dam = number_range(1, wield->level / 4 + 1);
        act("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
        act("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
        fire_effect((void *) victim, wield->level / 2, dam,
            TARGET_CHAR);
        damage(ch, victim, dam, 0, DAM_FIRE, FALSE);
    }

    if (ch->fighting == victim && IS_WEAPON_STAT(wield, WEAPON_FROST)) {
        dam = number_range(1, wield->level / 6 + 2);
        act("$p freezes $n.", victim, wield, NULL, TO_ROOM);
        act("The cold touch of $p surrounds you with ice.",
        victim, wield, NULL, TO_CHAR);
        cold_effect(victim, wield->level / 2, dam, TARGET_CHAR);
        damage(ch, victim, dam, 0, DAM_COLD, FALSE);
    }

    if (ch->fighting == victim
        && IS_WEAPON_STAT(wield, WEAPON_SHOCKING)) {
        dam = number_range(1, wield->level / 5 + 2);
        act("$n is struck by lightning from $p.", victim, wield, NULL,
        TO_ROOM);
        act("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
        shock_effect(victim, wield->level / 2, dam, TARGET_CHAR);
        damage(ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
    }
    }

    if (!IS_NPC(ch) && !IS_NPC(victim)) {
    ch->fight = 270;
    victim->fight = 270;
    }
    tail_chain();
    return;
}


/*
 * Inflict damage from a hit.
 */

bool
damage(CHAR_DATA * ch, CHAR_DATA * victim, long dam, int dt, int dam_type,
    bool show)
{
    OBJ_DATA *corpse;
    bool immune;
    char log_buf[MAX_STRING_LENGTH];
    char quest[MAX_STRING_LENGTH];
    int number;


    if (victim->position == POS_DEAD)
    return FALSE;

    /* damage reduction */
    if (dam > 35 && IS_NPC(ch))
    dam = (dam - 35) / 2 + 35;
    if (dam > 80 && IS_NPC(ch))
    dam = (dam - 80) / 2 + 80;

    /*
       Added this for monk hand to hand, only if they're not 
       wearing a weapon, and dualing a weapon... --Vorlin

       Edited by Kevin to prevent DP/Hurricane from being boosted.

       Edited by Umberone to give Ninja a bonus as well
    */
 
    if (ch->class == class_lookup("Monk") && dt != gsn_dragonp && dt != gsn_hurricane &&
        get_eq_char(ch, WEAR_WIELD) == NULL && get_eq_char(ch, WEAR_SECONDARY) == NULL) {
        dam = BOOST_40_PERCENT(dam);
    } else if (is_unarmed_ninja(ch) && dt != gsn_roundhouse) {
        dam = BOOST_20_PERCENT(dam);
    }

    /* 
       Added this for berserk, since battle rage is so much more
       effected... --Vorlin
     */
    if ((is_affected(ch, skill_lookup("berserk")) || IS_AFFECTED(ch, AFF_BERSERK)) &&
        dt != gsn_energygun && dt != gsn_heavygun && dt != gsn_pistol && dt != gsn_rifle) {
        dam = BOOST_10_PERCENT(dam);
    }

    if (victim != ch) {
    /*
       * Certain attacks are forbidden.
       * Most other attacks are returned.
     */
        if (is_safe(ch, victim))
            return FALSE;
        check_killer(ch, victim);

        if (victim->position > POS_STUNNED) {
            if (victim->fighting == NULL){
                if (victim->in_room == ch->in_room)
                    set_fighting(victim, ch);
                if (IS_NPC(victim) && HAS_TRIGGER(victim, TRIG_KILL))
                    mp_percent_trigger(victim, ch, NULL, NULL, TRIG_KILL);
            } else if (victim->timer <= 4) {
                victim->position = POS_FIGHTING;
            }

            if (ch->fighting == NULL && victim->in_room == ch->in_room) 
                set_fighting(ch, victim);
        }

        /*
           * More charm stuff.
         */
        if (victim->master == ch)
            stop_follower(victim);
    }

    /*
       * Inviso attacks ... not.
     */
    if (IS_AFFECTED(ch, AFF_INVISIBLE)
    && ch->class != class_lookup("ninja")) {
    affect_strip(ch, gsn_invis);
    affect_strip(ch, gsn_mass_invis);
    REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
    act("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

    /*
       * Damage modifiers.
     */

    if (dam > 1 && !IS_NPC(victim)
    && victim->pcdata->condition[COND_DRUNK] > 10)
        dam = REDUCE_10_PERCENT(dam);

    if (dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY))
        dam = REDUCE_50_PERCENT(dam);

    if (dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
            || (IS_AFFECTED(victim, AFF_PROTECT_GOOD)
            && IS_GOOD(ch))))
    dam = REDUCE_25_PERCENT(dam);

    if (ch->mount == IS_MOUNTED)
        dam += (dam / 10 + dam);

    /* PK damage modifier - Kevin, 4/17/02 */
    if (!IS_NPC(victim)) {
    if (!IS_NPC(ch) && !IS_QUESTOR(ch) && !IS_QUESTOR(victim)) {
        dam = REDUCE_25_PERCENT(dam);
    } else if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL) {
        dam = REDUCE_25_PERCENT(dam);
    }
    }

    immune = FALSE;

    /*
       * Check for parry, and dodge, shield block and phase.
     */
    if (dt >= TYPE_HIT && ch != victim) {
    if (check_parry(ch, victim))
        return FALSE;
    if (check_dodge(ch, victim))
        return FALSE;
    if (check_shield_block(ch, victim))
        return FALSE;
    if (check_phase(ch, victim))
        return FALSE;
    }

    switch (check_immune(victim, dam_type)) {
    case (IS_IMMUNE):
    immune = TRUE;
    dam = 0;
    break;
    case (IS_RESISTANT):
    dam = REDUCE_20_PERCENT(dam);  /* 20% less damage if victim is resistant - KV020528 */
    break;
    case (IS_VULNERABLE):
    dam = BOOST_20_PERCENT(dam); /* 20% more damage if victim is vulnerable - KV020528 */
    break;
    }

    if (show)
    dam_message(ch, victim, dam, dt, immune);

    if (dam == 0)
    return FALSE;

    /*
       * Hurt the victim.
       * Inform the victim of his new state.
     */

    /* Allow for stupidly high damages - avoids certain
       buffer overflows WRT imm weapons and other oddities - Suzuran */

    if (dam > (victim->hit + 100)) {
    victim->hit = -100;
    } else {
    victim->hit -= dam;
    }
    /* This was the old code 
       if (IS_NPC(victim) && ((victim->hit - dam) < 1))
       victim->hit = 0;
       else
       victim->hit -= dam;
     */
    
    /* This is the other IMMORTAL LEET BIT.  Avoid it. */
    if (!IS_NPC(victim) && IS_IMMORTAL(victim) && victim->hit < 1){
      if(!IS_QUESTOR(victim)){
    victim->hit = 1;
      }
    }

    update_pos(victim);

    switch (victim->position) {
    case POS_MORTAL:
    act("$n is mortally wounded, and will die soon, if not aided.",
        victim, NULL, NULL, TO_ROOM);
    send_to_char
        ("You are mortally wounded, and will die soon, if not aided.\n\r",
         victim);
    break;

    case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.",
        victim, NULL, NULL, TO_ROOM);
    send_to_char
        ("You are incapacitated and will slowly die, if not aided.\n\r",
         victim);
    break;

    case POS_STUNNED:
    act("$n is stunned, but will probably recover.", victim, NULL,
        NULL, TO_ROOM);
    send_to_char("You are stunned, but will probably recover.\n\r",
             victim);
    break;

    case POS_DEAD:
    act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
    send_to_char("You have been KILLED!!\n\r\n\r", victim);
    break;

    default:
    if (dam > victim->max_hit / 4)
        send_to_char("That really did HURT!\n\r", victim);
    if (victim->hit < victim->max_hit / 4)
        send_to_char("You sure are BLEEDING!\n\r", victim);
    break;
    }

    /*
       * Sleep spells and extremely wounded folks.
     */
    if (!IS_AWAKE(victim))
    stop_fighting(victim, FALSE);

    /*
       * Payoff for killing things.
     */
    if (victim->position == POS_DEAD) {
    group_gain(ch, victim);

    if (!IS_NPC(victim) && !IS_NPC(ch)) {   /*DC960914 */
        save_char_obj(ch);
        save_char_died_obj(victim);
        if ((!IS_SET(victim->act, PLR_KILLER))
        && (!IS_NPC(victim) && !IS_NPC(ch))
        && (!IS_QUESTOR(ch) && !IS_QUESTOR(victim)))
        SET_BIT(victim->act, PLR_DEAD);
    }

    if (!IS_NPC(victim)) {  /*EE960522* */
        if ((ch->in_room != victim->in_room) && dt == gsn_gun) {
        sprintf(log_buf,
            (IS_QUESTOR(victim) ? "%s quest-killed(shot) by %s at %d/%d" :
             "%s shot by %s at %d/%d"), victim->name,
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->vnum, victim->in_room->vnum);
        } else {
        sprintf(log_buf,
            (IS_QUESTOR(victim) ? "%s quest-killed by %s at %d" :
            "%s killed by %s at %d"), victim->name,
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->vnum);
        }
        log_string(log_buf);
        /* CHANGED THIS TO LEVEL 95 */
        wiznet(log_buf, NULL, NULL, WIZ_DEATHS, 0, 95);

        if ((IS_QUESTOR(victim) && !IS_NPC(victim))) {
                number = number_range(0, 16);
 
                switch (number) {
 
                case 0:
                sprintf(quest,
                         "`!PK`1Qst``: %s shoves the remains of %s in a Ziploc bag...\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 1:
                    sprintf(quest,
                            "`!PK`1Qst``: %s ripped %s's heart out, spraying blood everywhere...\n\r", 
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 2:
                    sprintf(quest,
                            "`!PK`1Qst``: %s ran %s sword through %s, cleaving %s in two...\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            ch->sex == 1 ? "his" : ch->sex ==
                            2 ? "her" : "a huge",
                            victim->name,
                            victim->sex == 1 ? "him" : victim->sex ==
                            2 ? "her" : "them");
                    break;
 
                case 3:
                    sprintf(quest,
                            "`!PK`1Qst``: %s stands victorious amidst the crimson remains of %s.\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;

                case 4:
                    sprintf(quest,
                            "`!PK`1Qst``: %s pulls %s weapon from %s's bowels...marinara sauce anyone?\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            ch->sex == 1 ? "his" : ch->sex ==
                            2 ? "her" : "a",
                            victim->name);
                    break;
 
                case 5:
                    sprintf(quest,
                            "`!PK`1Qst``: %s holds %s's head as a trophy!\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 6:
                    sprintf(quest,
                            "`!PK`1Qst``: %s just ripped %s's throat out...\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 7:
                    sprintf(quest,
                            "`!PK`1Qst``: %s just curb-stomped %s ... it wasn't pretty...\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 8:
                sprintf(quest,
                        "`!PK`1Qst``: %s up and gibbed %s... Booya!\n\r",
                        !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;

                case 9:
                sprintf(quest,
                        "`!PK`1Qst``: %s introduced %s's brain to a sharp object...\n\r",
                        !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 10:
                    sprintf(quest,
                            "`!PK`1Qst``: %s just wasted %s... Next?\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 11:
                    sprintf(quest,
                            "`!PK`1Qst``: %s just fragged %s ... Pain much?\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 12:
                    sprintf(quest,
                           "`!PK`1Qst``: %s just whipped %s's ass ...\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 13:
                    sprintf(quest,
                           "`!PK`1Qst``: %s makes %s kiss %s ass goodbye ...\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name,
                            victim->sex == 1 ? "his" : victim->sex == 2 ? "her"
                            : "their");
                    break;

                case 14:
                    sprintf(quest,
                           "`!PK`1Qst``: %s just burned %s to the ground ...\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name);
                    break;
 
                case 15:
                    sprintf(quest,
                           "`!PK`1Qst``: %s just relieved %s of %s right to live...\n\r",
                            !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name,
                            victim->sex == 1 ? "his" :
                                victim->sex == 2 ? "her" : "the");
                    break;
 
                case 16:
                sprintf(quest,
                        "`!PK`1Qst``: %s stuck a fork in %s ... Yup! %s done!\n\r",                !IS_NPC(ch) ? ch->
                            name : capitalize(ch->short_descr),
                            victim->name,
                            victim->sex == 1 ? "he's" :
                                victim->sex == 2 ? "she's" : "they're");
                break;
 
                default:
                    bug("Quest death problem: who knows?", 0);
                    break;
                }
                info(NULL, 0, quest);
        }

        /*EE960622 */
        if (!IS_NPC(ch) && !IS_QUESTOR(ch) && !IS_QUESTOR(victim)) {
        ch->pcdata->pkills++;
        victim->pcdata->pdeaths++;
        }

        /*
           * Dying penalty:
           * 2/3 way back to previous level.
         */
        if (!IS_QUESTOR(victim)
        && (victim->exp >
            exp_per_level(victim, victim->pcdata->points) * victim->level)) /*EE960522 */
        gain_exp(victim,
             (2 *
              (exp_per_level(victim, victim->pcdata->points) *
               victim->level - victim->exp) / 3) + 50);
    } else {
        if ((victim->in_room != ch->in_room) && dt == gsn_gun) {    /* Modified by Kevin */
        sprintf(log_buf,
            (IS_QUESTOR(victim) ?
             "%s got quest-killed(shot) by %s at %s/%s [rooms %d/%d]" :
             "%s got shot by %s at %s/%s [rooms %d/%d]"),
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, victim->in_room->name,
            ch->in_room->vnum, victim->in_room->vnum);
        } else {
        sprintf(log_buf,
            (IS_QUESTOR(victim) ?
             "%s got quest-killed by %s at %s [room %d]" :
             "%s got toasted by %s at %s [room %d]"),
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);  /*EE960522 */
        }
        wiznet(log_buf, NULL, NULL, WIZ_MOBDEATHS, 0, get_trust(ch));
    }

    /* For mobprogs - Suzuran */
    /*
     * Death trigger
     */
    if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
      {
        victim->position = POS_STANDING;
        mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
      }

    switch (dam_type) { /*EE960529 */
    case DAM_FIRE:
        raw_kill(victim, "The charred remains of %s are lying here.");
        break;
    case DAM_COLD:
        raw_kill(victim, "The frozen remains of %s are lying here.");
        break;
    case DAM_ACID:
        raw_kill(victim, "The corroded remains of %s are lying here.");
        break;
    case DAM_ENERGY:
        raw_kill(victim, "The twisted remains of %s are lying here.");
        break;

    default:
        raw_kill(victim, NULL); /*raw_kill( victim, "The corpse of %s are lying here." ); */
        break;
    }


    /* RT new auto commands */
    if (!IS_NPC(ch) && IS_NPC(victim)) {
        OBJ_DATA *coins;

        corpse = get_obj_list(ch, "corpse", ch->in_room->contents);

        if (IS_SET(ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)    /* exists and not empty */
        do_get(ch, "all corpse");
        if (IS_SET(ch->act, PLR_AUTOGOLD)
        && !IS_SET(ch->act, PLR_AUTOLOOT) && corpse && corpse->contains /* exists and not empty */
        )
        if ((coins = get_obj_list(ch, "gcash", corpse->contains))
            != NULL)
            do_get(ch, "all.gcash corpse");

        if (IS_SET(ch->act, PLR_AUTOSAC)) {
        if (IS_SET(ch->act, PLR_AUTOLOOT) && corpse
            && corpse->contains) return TRUE;   /* leave if corpse has treasure */
        else
            do_sacrifice(ch, "corpse");
        }
    }


    return TRUE;
    }

    if (victim == ch)
    return TRUE;

    /*
       * Take care of link dead people.
     */
    if (!IS_NPC(victim) && victim->desc == NULL)
    if (number_range(0, victim->wait) == 0) {
        do_recall(victim, "");
        return TRUE;
    }

    /*
       * Wimp out?
     */
    if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2) {
    if ((IS_SET(victim->act, ACT_WIMPY) && number_bits(2) == 0
         && victim->hit < victim->max_hit / 5)
        || (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
        && victim->master->in_room != victim->in_room))
        do_flee(victim, "");
    }

    if (!IS_NPC(victim)
    && victim->hit > 0
    && victim->hit <= victim->wimpy
    && victim->wait < PULSE_VIOLENCE / 2) do_flee(victim, "");

    tail_chain();
    return TRUE;
}



/*
* Inflict damage from a hit.
*
* This seems to be for magic attacks, while damage is for weapon attacks.
*/
bool
damage_old(CHAR_DATA * ch, CHAR_DATA * victim, long dam, int dt,
       int dam_type, bool show)
{

    OBJ_DATA *corpse;
    bool immune;
    char log_buf[MAX_STRING_LENGTH];

    if (victim->position == POS_DEAD)
    return FALSE;

    /* damage reduction */
    if (dam > 35 && IS_NPC(ch))
    dam = (dam - 35) / 2 + 35;
    if (dam > 80 && IS_NPC(ch))
    dam = (dam - 80) / 2 + 80;

    /*
       Modified/checked this for espers... --Vorlin
     */

    /* Esper casters: 120% spell damage, all others: 110% - KV020811 */
    if (ch->race == race_lookup("Esper")) {
    if (ch->class == class_lookup("Cleric")
        || ch->class == class_lookup("Mage")) {
        dam *= 1.2;
    } else {
        dam *= 1.1;
    }
    }

    /* High Elves deal 110% spell damage - KV020811 */
    if (ch->race == race_lookup("High-Elf"))
    dam *= 1.1;

    if (victim != ch) {
    /*
       * Certain attacks are forbidden.
       * Most other attacks are returned.
     */
    if (is_safe(ch, victim))
        return FALSE;
    check_killer(ch, victim);

    if (victim->position > POS_STUNNED) {
        if (victim->fighting == NULL)
        set_fighting(victim, ch);
        else if (victim->timer <= 4)
        victim->position = POS_FIGHTING;

        if (ch->fighting == NULL)
        set_fighting(ch, victim);


    }

    /*
       * More charm stuff.
     */
    if (victim->master == ch)
        stop_follower(victim);
    }

    /*
       * Inviso attacks ... not.
     */
    if (IS_AFFECTED(ch, AFF_INVISIBLE)) {
    affect_strip(ch, gsn_invis);
    affect_strip(ch, gsn_mass_invis);
    REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
    act("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

    /*
       * Damage modifiers.
     */

    if (dam > 1 && !IS_NPC(victim)
    && victim->pcdata->condition[COND_DRUNK] > 10)
    dam *= .9;

    if (dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY))
    dam /= 2;

    if (dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
            || (IS_AFFECTED(victim, AFF_PROTECT_GOOD)
            && IS_GOOD(ch))))
    dam *= .75;

    /* PK damage modifier - Kevin, 4/17/02 */
    if (!IS_NPC(victim)) {
    if (!IS_NPC(ch) && !IS_QUESTOR(ch) && !IS_QUESTOR(victim)) {
        dam *= .75;
    } else if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL) {
        dam *= .75;
    }       
    }

    immune = FALSE;


    /*
       * Check for parry, dodge, shield block and phase.
     */
    if (dt >= TYPE_HIT && ch != victim) {
    if (check_parry(ch, victim))
        return FALSE;
    if (check_dodge(ch, victim))
        return FALSE;
    if (check_shield_block(ch, victim))
        return FALSE;
    if (check_phase(ch, victim))
        return FALSE;

    }

    switch (check_immune(victim, dam_type)) {
    case (IS_IMMUNE):
    immune = TRUE;
    dam = 0;
    break;
    case (IS_RESISTANT):
    dam *= .8;  /* 20% less damage if victim is resistant - KV020528 */
    break;
    case (IS_VULNERABLE):
    dam *= 1.2; /* 20% more damage if victim is vulnerable - KV020528 */
    break;
    }

    if (show)
    dam_message(ch, victim, dam, dt, immune);

    if (dam == 0)
    return FALSE;

    /*
       * Hurt the victim.
       * Inform the victim of his new state.
     */

    /* Same fix here - Suzuran */
    if (dam > (victim->hit + 100)) {
    victim->hit = -100;
    } else {
    victim->hit -= dam;
    }


    /* This is the IMMORTAL LEET BIT. */
    /* We want to skip around this if we are questing. */
    if (!IS_NPC(victim) && IS_IMMORTAL(victim) && victim->hit < 1){
      if(!IS_QUESTOR(victim)){
    victim->hit = 1;
      }
    }
    /* That should do */

    update_pos(victim);

    switch (victim->position) {
    case POS_MORTAL:
    act("$n is mortally wounded, and will die soon, if not aided.",
        victim, NULL, NULL, TO_ROOM);
    send_to_char
        ("You are mortally wounded, and will die soon, if not aided.\n\r",
         victim);
    break;

    case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.",
        victim, NULL, NULL, TO_ROOM);
    send_to_char
        ("You are incapacitated and will slowly die, if not aided.\n\r",
         victim);
    break;

    case POS_STUNNED:
    act("$n is stunned, but will probably recover.",
        victim, NULL, NULL, TO_ROOM);
    send_to_char("You are stunned, but will probably recover.\n\r",
             victim);
    break;

    case POS_DEAD:
    act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
    send_to_char("You have been KILLED!!\n\r\n\r", victim);
    break;

    default:
    if (dam > victim->max_hit / 4)
        send_to_char("That really did HURT!\n\r", victim);
    if (victim->hit < victim->max_hit / 4)
        send_to_char("You sure are BLEEDING!\n\r", victim);
    break;
    }

    /*
       * Sleep spells and extremely wounded folks.
     */
    if (!IS_AWAKE(victim))
    stop_fighting(victim, FALSE);

    /*
       * Payoff for killing things.
     */
    if (victim->position == POS_DEAD) {
    group_gain(ch, victim);

    if (!IS_NPC(victim) && !IS_NPC(ch)) {
        save_char_obj(ch);
        save_char_died_obj(victim);
        if (!IS_SET(victim->act, PLR_KILLER)
        && (!IS_NPC(victim) && !IS_NPC(ch)) && (!IS_QUESTOR(ch)
                            &&
                            !IS_QUESTOR
                            (victim)))
            SET_BIT(victim->act, PLR_DEAD);
    }

    if (!IS_NPC(victim)) {  /*EE960522 */
        sprintf(log_buf,
            (IS_QUESTOR(victim) ? "%s quest-killed by %s at %d" :
             "%s killed by %s at %d"), victim->name,
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->vnum);
        log_string(log_buf);
        wiznet(log_buf, NULL, NULL, WIZ_DEATHS, 0, get_trust(ch));

        if (IS_QUESTOR(victim) && IS_QUESTOR(ch))
        info(NULL, 0,
             "%s(`!PK`1Qst``) just `#wasted`` %s(`!PK`1Qst``). CARNAGE !!!!\n\r",
             !IS_NPC(ch) ? ch->name : capitalize(ch->short_descr),
             victim->name);

        /*EE960622 */
        if (!IS_NPC(ch) && !IS_QUESTOR(ch) && !IS_QUESTOR(victim)) {
        ch->pcdata->pkills++;
        victim->pcdata->pdeaths++;
        }

        /*
           * Dying penalty:
           * 2/3 way back to previous level.
         */
        if (!IS_QUESTOR(victim)
        && (victim->exp >
            exp_per_level(victim, victim->pcdata->points) * victim->level)) /*EE960522 */
        gain_exp(victim,
             (2 *
              (exp_per_level(victim, victim->pcdata->points) *
               victim->level - victim->exp) / 3) + 50);
    } else {
        sprintf(log_buf,
            (IS_QUESTOR(victim) ?
             "%s got quest-killed by %s at %s [room %d]" :
             "%s got toasted by %s at %s [room %d]"),
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);  /*EE960522 */
        wiznet(log_buf, NULL, NULL, WIZ_MOBDEATHS, 0, get_trust(ch));
    }

    switch (dam_type) { /*EE960529 */
    case DAM_FIRE:
        raw_kill(victim, "The charred remains of %s are lying here.");
        break;
    case DAM_COLD:
        raw_kill(victim, "The frozen remains of %s are lying here.");
        break;
    case DAM_ACID:
        raw_kill(victim, "The corroded remains of %s are lying here.");
        break;
    case DAM_ENERGY:
        raw_kill(victim, "The twisted remains of %s are lying here.");
        break;

    default:
        raw_kill(victim, NULL); /*raw_kill( victim, "The corpse of %s are lying here." ); */
        break;
    }

    /* RT new auto commands */
    if (!IS_NPC(ch) && IS_NPC(victim)) {
        corpse = get_obj_list(ch, "corpse", ch->in_room->contents);

        if (IS_SET(ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)    /* exists and not empty */
        do_get(ch, "all corpse");

        if (IS_SET(ch->act, PLR_AUTOGOLD)
        && !IS_SET(ch->act, PLR_AUTOLOOT) && corpse && corpse->contains /* exists and not empty */
        )
        do_get(ch, "gold corpse");

        if (IS_SET(ch->act, PLR_AUTOSAC)) {
        if (IS_SET(ch->act, PLR_AUTOLOOT) && corpse
            && corpse->contains) return TRUE;   /* leave if corpse has treasure */
        else
            do_sacrifice(ch, "corpse");
        }
    }

    return TRUE;
    }

    if (victim == ch)
    return TRUE;

    /*
       * Take care of link dead people.
     */
    if (!IS_NPC(victim) && victim->desc == NULL)
    if (number_range(0, victim->wait) == 0) {
        do_recall(victim, "");
        return TRUE;
    }

    /*
       * Wimp out?
     */
    if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    if ((IS_SET(victim->act, ACT_WIMPY) && number_bits(2) == 0
         && victim->hit < victim->max_hit / 5)
        || (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
        && victim->master->in_room != victim->in_room))
        do_flee(victim, "");

    if (!IS_NPC(victim)
    && victim->hit > 0
    && victim->hit <= victim->wimpy
    && victim->wait < PULSE_VIOLENCE / 2) do_flee(victim, "");

    tail_chain();
    return TRUE;
}

/*
* Set position of a victim.
*/
void update_pos(CHAR_DATA * victim)
{
    if (victim->hit > 0) {
    if (victim->position <= POS_STUNNED)
        victim->position = POS_STANDING;
    return;
    }

    if (IS_NPC(victim) && victim->hit < 1) {
    victim->position = POS_DEAD;
    return;
    }

    if (victim->hit <= -11) {
    victim->position = POS_DEAD;
    return;
    }

    if (victim->hit <= -6)
    victim->position = POS_MORTAL;
    else if (victim->hit <= -3)
    victim->position = POS_INCAP;
    else
    victim->position = POS_STUNNED;

    return;
}



/*
* Start fights.
*/
void set_fighting(CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (ch->fighting != NULL) {
    bug("Set_fighting: already fighting", 0);
    return;
    }

    if (IS_AFFECTED(ch, AFF_SLEEP))
    affect_strip(ch, gsn_sleep);

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
* Stop fights.
*/
void stop_fighting(CHAR_DATA * ch, bool fBoth)
{
    CHAR_DATA *fch;

    for (fch = char_list; fch != NULL; fch = fch->next)
    if (fch == ch || (fBoth && fch->fighting == ch)) {
        fch->fighting = NULL;
        fch->position = IS_NPC(fch) ? fch->default_pos : POS_STANDING;
        update_pos(fch);
    }

    return;
}



/*
* Make a corpse out of a character.
*/
void make_corpse(CHAR_DATA * ch, char *description)
{

    char buf[MAX_STRING_LENGTH], *name;
    OBJ_DATA *corpse, *obj, *obj_next;

    /*
       * bah, I am tired of seeing mucho corpses from pkquests... no more corpses from quests
 *//*TG990619 */
    if (IS_QUESTOR(ch) && (ch->pcdata->questgiver != NULL)
    && IS_QUESTPKMST(ch->pcdata->questgiver))
    return;

    if (IS_NPC(ch)) {
    name = ch->short_descr;
    corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
    corpse->timer = number_range(3, 6);

    if (ch->gold > 0) {
        obj_to_obj(create_money(ch->gold, ch->silver), corpse);
        ch->gold = 0;
        ch->silver = 0;
    }

    corpse->cost = 0;
    } else {

    name = ch->name;
    corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
    corpse->timer = number_range(25, 40);
    REMOVE_BIT(ch->act, PLR_CANLOOT);
    /*if (!is_clan(ch)) */
    if (!ch->pcdata->pkset)
        corpse->owner = str_dup(ch->name);
    else {
        corpse->owner = NULL;
        if (!IS_QUESTOR(ch) && (ch->gold > 1 || ch->silver > 1)) {  /*EE960522 */
        obj_to_obj(create_money(ch->gold / 2, ch->silver / 2),
               corpse);
        ch->gold -= ch->gold / 2;
        ch->silver -= ch->silver / 2;
        }
    }

    corpse->cost = 0;
    save_char_obj(ch);
    }


    corpse->level = ch->level;


    /*EE960529 */

    if (description == NULL)
    sprintf(buf, corpse->short_descr, name);
    else
    sprintf(buf, "the remains of %s", name);

    free_string(corpse->short_descr);
    corpse->short_descr = str_dup(buf);

    if (description == NULL)
    sprintf(buf, corpse->description, name);
    else
    sprintf(buf, description, name);
    free_string(corpse->description);
    corpse->description = str_dup(buf);


    if (!IS_QUESTOR(ch)) {  /*EE960522 */
    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
        bool floating = FALSE;

        obj_next = obj->next_content;
        if (obj->wear_loc == WEAR_FLOAT)
        floating = TRUE;
        obj_from_char(obj);
        if (obj->item_type == ITEM_POTION)
        obj->timer = number_range(500, 1000);
        if (obj->item_type == ITEM_SCROLL)
        obj->timer = number_range(1000, 2500);
        if (IS_SET(obj->extra_flags, ITEM_ROT_DEATH) && !floating) {
        obj->timer = number_range(5, 10);
        REMOVE_BIT(obj->extra_flags, ITEM_ROT_DEATH);
        }
        REMOVE_BIT(obj->extra_flags, ITEM_VIS_DEATH);

        if (IS_SET(obj->extra_flags, ITEM_INVENTORY))
        extract_obj(obj);
        else if (floating) {
        if (IS_OBJ_STAT(obj, ITEM_ROT_DEATH)) { /* get rid of it! */
            if (obj->contains != NULL) {
            OBJ_DATA *in, *in_next;

            act("$p evaporates,scattering its contents.",
                ch, obj, NULL, TO_ROOM);
            for (in = obj->contains; in != NULL; in = in_next) {
                in_next = in->next_content;
                obj_from_obj(in);
                obj_to_room(in, ch->in_room);
            }
            } else
            act("$p evaporates.", ch, obj, NULL, TO_ROOM);
            extract_obj(obj);
        } else {
            act("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
            obj_to_room(obj, ch->in_room);
        }
        } else
        obj_to_obj(obj, corpse);
    }
    }

    obj_to_room(corpse, ch->in_room);
    return;
}



/*
* Improved Death_cry contributed by Diavolo.
*/
void death_cry(CHAR_DATA * ch)
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch (number_bits(4)) {
    case 0:
    msg = "$n hits the ground ... DEAD.";
    break;
    case 1:
    if (ch->material == 0) {
        msg = "$n splatters blood on your armor.";
        break;
    }
    case 2:
    if (IS_SET(ch->parts, PART_GUTS)) {
        msg = "$n spills $s guts all over the floor.";
        vnum = OBJ_VNUM_GUTS;
    }
    break;
    case 3:
    if (IS_SET(ch->parts, PART_HEAD)) {
        msg = "$n's severed head plops on the ground.";
        vnum = OBJ_VNUM_SEVERED_HEAD;
    }
    break;
    case 4:
    if (IS_SET(ch->parts, PART_HEART)) {
        msg = "$n's heart is torn from $s chest.";
        vnum = OBJ_VNUM_TORN_HEART;
    }
    break;
    case 5:
    if (IS_SET(ch->parts, PART_ARMS)) {
        msg = "$n's arm is sliced from $s dead body.";
        vnum = OBJ_VNUM_SLICED_ARM;
    }
    break;
    case 6:
    if (IS_SET(ch->parts, PART_LEGS)) {
        msg = "$n's leg is sliced from $s dead body.";
        vnum = OBJ_VNUM_SLICED_LEG;
    }
    break;
    case 7:
    if (IS_SET(ch->parts, PART_BRAINS)) {
        msg =
        "$n's head is shattered, and $s brains splash all over you.";
        vnum = OBJ_VNUM_BRAINS;
    }
    }

    act(msg, ch, NULL, NULL, TO_ROOM);

    if (vnum != 0) {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    char *name;

    name = IS_NPC(ch) ? ch->short_descr : ch->name;
    obj = create_object(get_obj_index(vnum), 0);
    obj->timer = number_range(4, 7);

    sprintf(buf, obj->short_descr, name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);

    sprintf(buf, obj->description, name);
    free_string(obj->description);
    obj->description = str_dup(buf);

    if (obj->item_type == ITEM_FOOD) {
        if (IS_SET(ch->form, FORM_POISON))
        obj->value[3] = 1;
        else if (!IS_SET(ch->form, FORM_EDIBLE))
        obj->item_type = ITEM_TRASH;
    }

    obj_to_room(obj, ch->in_room);
    }

    if (IS_NPC(ch))
    msg = "You hear something's death cry.";
    else
    msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for (door = 0; door <= 5; door++) {
    EXIT_DATA *pexit;

    if ((pexit = was_in_room->exit[door]) != NULL
        && pexit->u1.to_room != NULL
        && pexit->u1.to_room != was_in_room) {
        ch->in_room = pexit->u1.to_room;
        act(msg, ch, NULL, NULL, TO_ROOM);
    }
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill(CHAR_DATA * victim, char *description)
{
    int i;

    stop_fighting(victim, TRUE);

    if (!IS_NPC(victim) && victim->horse != NULL
    && victim->horse->fighting != NULL)
    stop_fighting(victim->horse, TRUE);

    if (!IS_NPC(victim) && victim->mount == IS_MOUNTED) {
    victim->horse->leader = NULL;
    victim->horse->master = NULL;
    victim->horse = NULL;
    }

    death_cry(victim);
    make_corpse(victim, description);

    if (IS_NPC(victim)) {
    victim->pIndexData->killed++;
    kill_table[URANGE(0, victim->level, MAX_LEVEL - 1)].killed++;
    extract_char(victim, TRUE);
    return;
    }

    /*    if (!IS_QUESTOR(victim))
       {
       REMOVE_BIT(victim->act, PLR_KILLER);
       REMOVE_BIT(victim->act, PLR_THIEF);
       }
     */
    extract_char(victim, FALSE);
    while (victim->affected)
    affect_remove(victim, victim->affected);

    victim->affected_by = race_table[victim->race].aff;

    if (!IS_QUESTOR(victim)) {
    for (i = 0; i < 4; i++)
        victim->armor[i] = 100;
    }

    victim->position = POS_RESTING;
    victim->hit = 1;
    victim->mana = 1;
    victim->move = 1;
    WAIT_STATE(victim, 5 * PULSE_VIOLENCE);
    save_char_obj(victim);
    return;
}



void group_gain(CHAR_DATA * ch, CHAR_DATA * victim)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    int xp;
    int members;
    int group_levels;

    /*
       * Monsters don't get kill xp's or alignment changes.
       * P-killing doesn't help either.
       * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if (victim == ch)
    return;

    if (IS_QUESTOR(ch))     /*EE960524 */
    return;         /*Questers don't receive any xp */

    if (IS_SET(ch->act, PLR_AUTOQST) && IS_NPC(victim))
    if (ch->questmob == victim->pIndexData->vnum) {
        send_to_char("You have almost completed your QUEST!\n\r", ch);
        send_to_char
        ("Return to the questmaster before your time runs out!\n\r",
         ch);
        ch->questmob = -1;
    }

    /* NPCs don't count as group members for XP purposes. - Kevin */
    members = 0;
    group_levels = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    if (is_same_group(gch, ch) && !IS_NPC(gch)) {
        members++;
        group_levels += gch->level;
    }

    if (members == 0) {
    bug("Group_gain: members.", members);
    members = 1;
    group_levels = ch->level;
    }

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if (!is_same_group(gch, ch) || IS_NPC(gch))
        continue;

    xp = xp_compute(gch, victim, group_levels);

    /* Lower XP if the group is larger than two people.. - Kevin */
    if (members > 2)
        xp = (xp * UMAX(40, 100 - ((members - 2) * 15))) / 100;

    sprintf(buf, "You receive `&%d`` experience points.\n\r", xp);
    send_to_char(buf, gch);
    gain_exp(gch, xp);

    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
        obj_next = obj->next_content;
        if (obj->wear_loc == WEAR_NONE)
        continue;

        if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
        || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
        || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
        act("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
        act("$n is zapped by $p.", ch, obj, NULL, TO_ROOM);
        obj_from_char(obj);
        obj_to_room(obj, ch->in_room);
        }
    }
    }

    return;
}



/*
* Compute xp for a kill.
* Also adjust alignment of killer.
* Edit this function to change xp computations.
*/
int xp_compute(CHAR_DATA * gch, CHAR_DATA * victim, int total_levels)
{
    int xp, base_exp, align, level_range =
    victim->level - gch->level, change;

    /* compute the base exp */
    switch (level_range) {
    default:
    base_exp = 0;
    break;
    case -9:
    base_exp = 1;
    break;
    case -8:
    base_exp = 3;
    break;
    case -7:
    base_exp = 5;
    break;
    case -6:
    base_exp = 9;
    break;
    case -5:
    base_exp = 11;
    break;
    case -4:
    base_exp = 22;
    break;
    case -3:
    base_exp = 33;
    break;
    case -2:
    base_exp = 50;
    break;
    case -1:
    base_exp = 66;
    break;
    case 0:
    base_exp = 88;
    break;
    case 1:
    base_exp = 99;
    break;
    case 2:
    base_exp = 121;
    break;
    case 3:
    base_exp = 143;
    break;
    case 4:
    base_exp = 165;
    break;
    }

    if (level_range > 4)
    base_exp = 160 + 20 * (level_range - 4);

    /* do alignment computations */

    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act, ACT_NOALIGN)) { /* no change */
    } else if (align > 500) {   /* monster is more good than slayer */
    change =
        (align - 500) * base_exp / 500 * gch->level / total_levels;
    change = UMAX(1, change);
    gch->alignment = UMAX(-1000, gch->alignment - change);
    } else if (align < -500) {  /* monster is more evil than slayer */
    change =
        (-1 * align -
         500) * base_exp / 500 * gch->level / total_levels;
    change = UMAX(1, change);
    gch->alignment = UMIN(1000, gch->alignment + change);
    } else {            /* improve this someday */
    change =
        gch->alignment * base_exp / 500 * gch->level / total_levels;
    gch->alignment -= change;
    }

    /* calculate exp multiplier */
    if (IS_SET(victim->act, ACT_NOALIGN))
    xp = base_exp;
    else if (gch->alignment > 500) {    /* for goodie two shoes */
    if (victim->alignment < -750)
        xp = (base_exp * 4) / 3;
    else if (victim->alignment < -500)
        xp = (base_exp * 5) / 4;
    else if (victim->alignment > 250)
        xp = (base_exp * 3) / 4;
    else if (victim->alignment > 750)
        xp = base_exp / 4;
    else if (victim->alignment > 500)
        xp = base_exp / 2;
    else
        xp = base_exp;
    } else if (gch->alignment < -500) { /* for baddies */
    if (victim->alignment > 750)
        xp = (base_exp * 5) / 4;
    else if (victim->alignment > 500)
        xp = (base_exp * 11) / 10;
    else if (victim->alignment < -750)
        xp = base_exp / 2;
    else if (victim->alignment < -500)
        xp = (base_exp * 3) / 4;
    else if (victim->alignment < -250)
        xp = (base_exp * 9) / 10;
    else
        xp = base_exp;
    } else if (gch->alignment > 200) {  /* a little good */
    if (victim->alignment < -500)
        xp = (base_exp * 6) / 5;
    else if (victim->alignment > 750)
        xp = base_exp / 2;
    else if (victim->alignment > 0)
        xp = (base_exp * 3) / 4;
    else
        xp = base_exp;
    } else if (gch->alignment < -200) { /* a little bad */
    if (victim->alignment > 500)
        xp = (base_exp * 6) / 5;
    else if (victim->alignment < -750)
        xp = base_exp / 2;
    else if (victim->alignment < 0)
        xp = (base_exp * 3) / 4;
    else
        xp = base_exp;
    } else {            /* neutral */
    if (victim->alignment > 500 || victim->alignment < -500)
        xp = (base_exp * 4) / 3;
    else if (victim->alignment < 200 && victim->alignment > -200)
        xp = base_exp / 2;
    else
        xp = base_exp;
    }

    /* more exp at the low levels */
    if (gch->level < 10)
    xp = 10 * xp / (gch->level + 4);

    /* less at high */
    /*    if (gch->level > 72)
      xp = 15 * xp / (gch->level - 25);  -- Conflicts with other fix */

    /* randomize the rewards */
    xp = number_range(xp * 1 / 2, xp * 3 / 4);

    /* adjust for grouping 
       xp = xp * ( .05 + gch->level/( UMAX(1,total_levels -1) )); */

    /* People are un-creative -- Suzuran.
      
    xp = xp * gch->level / (UMAX(1, total_levels - 1));
    */

    xp = xp * 2;

    /* reward for PK */
    if (!IS_NPC(gch) && !IS_NPC(victim))
    xp = xp * 3;        /*EE960617 */

    if (IS_NPC(victim) && IS_SET(victim->act, ACT_HORSE))
    xp = 0;         /*No exp for killing horses.  Due to horses having less hp
                   * to balance game
                   * DC962412 */

    /* 5x experience because who's got time for that!?more exp because */
    xp = xp * 5;

    return xp;
}


void
dam_message(CHAR_DATA * ch, CHAR_DATA * victim, long dam, int dt,
        bool immune)
{
    char buf1[256], buf2[256], buf3[256], punct;
    const char *vs, *vp, *attack;

    if (ch == NULL || victim == NULL)
    return;

    if (dam == 0) {
    vs = "`&miss``";
    vp = "`&misses``";
    } else if (dam <= 4) {
    vs = "`&scratch``";
    vp = "`&scratches``";
    } else if (dam <= 8) {
    vs = "`&graze``";
    vp = "`&grazes``";
    } else if (dam <= 12) {
    vs = "`&hit``";
    vp = "`&hits``";
    } else if (dam <= 16) {
    vs = "`&injure``";
    vp = "`&injures``";
    } else if (dam <= 20) {
    vs = "`&wound``";
    vp = "`&wounds``";
    } else if (dam <= 24) {
    vs = "`&maul``";
    vp = "`&mauls``";
    } else if (dam <= 28) {
    vs = "`&decimate``";
    vp = "`&decimates``";
    } else if (dam <= 32) {
    vs = "`&devastate``";
    vp = "`&devastates``";
    } else if (dam <= 36) {
    vs = "`&maim``";
    vp = "`&maims``";
    } else if (dam <= 40) {
    vs = "`1MUTILATE``";
    vp = "`1MUTILATES``";
    } else if (dam <= 44) {
    vs = "`1DISEMBOWEL``";
    vp = "`1DISEMBOWELS``";
    } else if (dam <= 48) {
    vs = "`1DISMEMBER``";
    vp = "`1DISMEMBERS``";
    } else if (dam <= 52) {
    vs = "`1MASSACRE``";
    vp = "`1MASSACRES``";
    } else if (dam <= 56) {
    vs = "`1MANGLE``";
    vp = "`1MANGLES``";
    } else if (dam <= 60) {
    vs = "`1*`!*`1* `!DEMOLISH `1*`!*`1*``";
    vp = "`1*`!*`1* `!DEMOLISHES `1*`!*`1*``";
    } else if (dam <= 75) {
    vs = "`1*`!*`1* `!DEVASTATE `1*`!*`1*``";
    vp = "`1*`!*`1* `!DEVASTATES `1*`!*`1*``";
    } else if (dam <= 100) {
    vs = "`3=`#=`3= `#OBLITERATE `3=`#=`3=``";
    vp = "`3=`#=`3= `#OBLITERATES `3=`#=`3=``";
    } else if (dam <= 125) {
    vs = "`#>`3>`#> `3AN`#NI`&HI`#LA`3TE `#<`3<`#<``";
    vp = "`#>`3>`#> `3AN`#NI`&HIL`#AT`3ES `#<`3<`#<``";
    } else if (dam <= 150) {
    vs = "`^<`6<`^< `6ER`^AD`&I`^CA`6TE `^>`6>`^>``";
    vp = "`^<`6<`^< `6ER`^AD`&IC`^AT`6ES `^>`6>`^>``";
    } else if (dam <= 300) {
    vs = "`!-`1*`!- `1D`!ES`#OL`!AT`1E `!-`1*`!-``";
    vp = "`!-`1*`!- `1D`!ES`#OLA`!TE`1S `!-`1*`!-``";
    } else if (dam <= 600) {
    vs = "`1-=`!< `8R``A`&VA``G`8E `!>`1=-";
    vp = "`1-=`!< `8R``A`&VA``GE`8S `!>`1=-``";
    } else if (dam < 1500) {
    vs = "`7do `&UNSPEAKABLE `#PAIN`` to";
    vp = "`7does `&UNSPEAKABLE `#PAIN`` to";
    } else if (dam >= 1500) {
    vs = "`1-=`!( `1M`!O`#R`&T`#I`!F`1Y `!)`1=-``";
    vp = "`1-=`!( `1M`!O`#R`&TIF`#I`!E`1S `!)`1=-``";
    }

    punct = (dam <= 24) ? '.' : '!';

    if (dt == TYPE_HIT) {
    if (ch == victim) {
        if (IS_SET(ch->act, PLR_AUTODAMAGE)) {
        sprintf(buf1, "`5$n's %s `5$m%c `8(`1%ld `!Dmg`8)``", vp,
            punct, dam);
        sprintf(buf2, "`5You %s `5yourself%c `8(`1%ld `!Dmg`8)``",
            vp, punct, dam);
        } else {
        sprintf(buf1, "`5$n's %s `5$m%c``", vp, punct);
        sprintf(buf2, "`5You %s `5yourself%c``", vp, punct);
        }
    } else {
        if (IS_SET(ch->act, PLR_AUTODAMAGE)) {
        sprintf(buf1, "`5$n %s `5$N%c  `8(`1%ld `!Dmg`8)``", vp,
            punct, dam);
        sprintf(buf2, "`5You %s `5$N%c `8(`1%ld `!Dmg`8)``", vs,
            punct, dam);
        sprintf(buf3, "`5$n %s `5you%c `8(`1%ld `!Dmg`8)``", vp,
            punct, dam);
        } else {
        sprintf(buf1, "`5$n %s `5$N%c``", vp, punct);
        sprintf(buf2, "`5You %s `5$N%c``", vs, punct);
        sprintf(buf3, "`5$n %s `5you%c``", vp, punct);
        }
    }
    } else {
    if (dt >= 0 && dt < MAX_SKILL) {
        if (dt != gsn_gun) {
        attack = skill_table[dt].noun_damage;
        } else {
        attack = gun_message(ch);
        }
    } else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE) {
        attack = attack_table[dt - TYPE_HIT].noun;
    } else {
        bug("Dam_message: bad dt %d.", dt);
        dt = TYPE_HIT;
        attack = attack_table[0].name;
    }

    if (immune) {
        if (ch == victim) {
        sprintf(buf1, "`A$n is unaffected by $s own %s.``", attack);
        sprintf(buf2, "`aLuckily, you are immune to that.``");
        } else {
        sprintf(buf1, "`A$N is unaffected by $n's %s!``", attack);
        sprintf(buf2, "`a$N is unaffected by your %s!``", attack);
        sprintf(buf3, "`A$n's %s is powerless against you.``", attack);
        }
    } else {
        if (ch == victim) {
        if (IS_SET(ch->act, PLR_AUTODAMAGE)) {
            sprintf(buf1, "`A$n's %s %s `A$m%c `8(`2%ld `@Dmg`8)``",
                attack, vp, punct, dam);
            sprintf(buf2, "`aYour %s %s `ayou%c `8(`2%ld `@Dmg`8)``",
                attack, vp, punct, dam);
        } else {
            sprintf(buf1, "`A$n's %s %s `A$m%c``", attack, vp, punct);
            sprintf(buf2, "`aYour %s %s `ayou%c``", attack, vp, punct);
        }
        } else {
        if (IS_SET(ch->act, PLR_AUTODAMAGE)) {
            if ((victim->in_room != ch->in_room) && dt == gsn_gun) {
            sprintf(buf1, "`ASomeone's %s %s `A$n%c `8(`1%ld `!Dmg`8)``",
                attack, vp, punct, dam);
            } else {
            sprintf(buf1, "`A$n's %s %s `A$N%c `8(`1%ld `!Dmg`8)``",
                attack, vp, punct, dam);
            }
            sprintf(buf2, "`aYour %s %s `a$N%c `8(`1%ld `!Dmg`8)``",
                attack, vp, punct, dam);
            sprintf(buf3, "`A$n's %s %s `Ayou%c `8(`1%ld `!Dmg`8)``",
                attack, vp, punct, dam);
        } else {
            if ((victim->in_room != ch->in_room) && dt == gsn_gun) {
            sprintf(buf1, "`ASomeone's %s %s `A$N%c``", attack, vp, punct);
            } else {
            sprintf(buf1, "`A$n's %s %s `A$N%c``", attack, vp, punct);
            }
            sprintf(buf2, "`aYour %s %s `a$N%c``", attack, vp, punct);
            sprintf(buf3, "`A$n's %s %s `Ayou%c``", attack, vp, punct);
        }
        }
    }
    }

    if (ch == victim) {
    act(buf1, ch, NULL, NULL, TO_ROOM);
    act(buf2, ch, NULL, NULL, TO_CHAR);
    } else {
    if ((ch->in_room != victim->in_room) && dt == gsn_gun) {
        act(buf1, victim, NULL, NULL, TO_ROOM);
    } else {
        act(buf1, ch, NULL, victim, TO_NOTVICT);
    }
    act(buf2, ch, NULL, victim, TO_CHAR);
    act(buf3, ch, NULL, victim, TO_VICT);
    }

    return;
}



void do_kill(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
    send_to_char("Kill whom?\n\r", ch);
    return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
    send_to_char("They aren't here.\n\r", ch);
    return;
    }

    if ((!IS_NPC(victim) && IS_SET(victim->act, PLR_DEAD))
    || (IS_SET(ch->act, PLR_DEAD) && !IS_NPC(victim))) {
    send_to_char("You are not permitted to kill this person!\n\r", ch);
    return;
    }

    if (victim == ch) {
    send_to_char("You hit yourself.  Ouch!\n\r", ch);
    multi_hit(ch, ch, TYPE_UNDEFINED);
    return;
    }

    if (is_safe(ch, victim))
    return;

    if (!IS_NPC(ch) && !IS_NPC(victim)) {
    if (!(ch->pcdata->pkset && victim->pcdata->pkset)
        && victim->fighting != NULL
        && !is_same_group(ch, victim->fighting)) {
        send_to_char("Kill stealing is not permitted.\n\r", ch);
        return;
    }
    }


    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
    act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
    }

    if (ch->position == POS_FIGHTING) {
    send_to_char("You do the best you can!\n\r", ch);
    return;
    }

    WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
    check_killer(ch, victim);
    multi_hit(ch, victim, TYPE_UNDEFINED);
    return;
}



void do_murde(CHAR_DATA * ch, char *argument)
{
    send_to_char("If you want to MURDER, spell it out.\n\r", ch);
    return;
}



void do_murder(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
    send_to_char("Murder whom?\n\r", ch);
    return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM)
    || (IS_NPC(ch) && IS_SET(ch->act, ACT_PET)))
    return;

    if ((victim = get_char_room(ch, arg)) == NULL) {
    send_to_char("They aren't here.\n\r", ch);
    return;
    }

    if (victim == ch) {
    send_to_char("Suicide is a mortal sin.\n\r", ch);
    return;
    }

    if (is_safe(ch, victim))
    return;

    if (IS_NPC(victim) && victim->fighting != NULL
    && !is_same_group(ch, victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r", ch);
    return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
    act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
    return;
    }

    if (IS_SET(victim->act, PLR_DEAD)) {
    send_to_char("You cannot strike a dead person.\n\r", ch);
    return;
    }

    if (ch->position == POS_FIGHTING) {
    send_to_char("You do the best you can!\n\r", ch);
    return;
    }

    WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
    if (IS_NPC(ch))
    sprintf(buf, "Help! I am being attacked by %s!", ch->short_descr);
    else
    sprintf(buf, "Help!  I am being attacked by %s!", ch->name);

    do_yell(victim, buf);
    check_killer(ch, victim);
    multi_hit(ch, victim, TYPE_UNDEFINED);
    return;
}


void do_flee(CHAR_DATA * ch, char *argument)
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ((victim = ch->fighting) == NULL) {
    if (ch->position == POS_FIGHTING)
        ch->position = POS_STANDING;
    send_to_char("You aren't fighting anyone.\n\r", ch);
    return;
    }

    was_in = ch->in_room;
    for (attempt = 0; attempt < 6; attempt++) {
    EXIT_DATA *pexit;
    int door;

    door = number_door();
    if ((pexit = was_in->exit[door]) == 0
        || pexit->u1.to_room == NULL
        || IS_SET(pexit->exit_info, EX_CLOSED)
        || number_range(0, ch->daze) != 0 || (IS_NPC(ch)
                          && IS_SET(pexit->
                                u1.to_room->room_flags,
                                ROOM_NO_MOB)))
        continue;

    move_char(ch, door, FALSE);
    if ((now_in = ch->in_room) == was_in)
        continue;

    ch->in_room = was_in;
    act("$n has fled!", ch, NULL, NULL, TO_ROOM);
    ch->in_room = now_in;

    if (!IS_NPC(ch)) {
        send_to_char("You flee from combat!\n\r", ch);

        if (ch->class == class_lookup("Ninja"))
        send_to_char("You snuck away safely.\n\r", ch);
        else {
        send_to_char("You lost 10 exp.\n\r", ch);
        gain_exp(ch, -10);
        }
    }

    WAIT_STATE(ch, .75 * PULSE_VIOLENCE);
    stop_fighting(ch, TRUE);
    return;
    }

    send_to_char("PANIC! You couldn't escape!\n\r", ch);
    return;
}

void do_tport(CHAR_DATA *ch, char *argument){

  /* Teleportation for non-spellcasters.
     Should be based on movement points.
  */

  CHAR_DATA *victim;
  ROOM_INDEX_DATA *pRoomIndex;
  char buf[MAX_STRING_LENGTH];
  int failurefactor=0;
  int tmp=0;
  
  /* All attempts to teleport will lag user one round - KV020518 */
  WAIT_STATE(ch, PULSE_VIOLENCE);

  /* Argue. */
  argument=one_argument(argument,buf);

  /* First, find and validate the target. */
  if((victim = get_char_world(ch, buf))==NULL){
    send_to_char("Target doesn't exist.\n\r",ch);
    return;
  }

  /* Bail if we cannot see the target */
  if(!can_see_room(ch,victim->in_room)){
    send_to_char("Target doesn't exist.\n\r",ch);
    return;
  }
 
  /* Bail if trying to get in a safe room with active timer */
  if(ch->fight > 0
    && IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
    send_to_char("An unknown force prevents you from teleporting there.\n\r", ch);
    return;
  }

  /* Bail if cursed */
  if(IS_AFFECTED(ch, AFF_CURSE)) {
    send_to_char("Supernatural taint prevents you from teleporting.\n\r", ch);
    return;
  }

  /* Bail if we teleport to ourself */
  if(victim==ch){
    send_to_char("Teleporting to yourself is useless.\n\r",ch);
    return;
  }

  /* Bail if NPC tries/is ordered to teleport */
  if(IS_NPC(ch))
    return;

  /* Don't teleport into limbo or to people we can't see */
  if(victim->in_room->vnum == ROOM_VNUM_LIMBO){
    failurefactor=100; /* Ensure random teleport */
  }

  /* Screw people trying to teleport out of norecall */
  if(IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    || room_is_private(victim->in_room)
    || victim->level > LEVEL_HERO){
    failurefactor=500; /* Go nowhere */
  }

  /* If we are fighting, increase fudge factor */
  if(ch->fighting != NULL){ failurefactor += 40; }

  /* Now fudge based on level */
  if(victim->level > ch->level){
    failurefactor += ((victim->level - ch->level)*5);
  }
  /* That is, if we are 8 levels apart, 40% chance of failure */

  /* Send messages */
  act("$n begins to concentrate in preparation for teleport.",ch,NULL,NULL,
      TO_ROOM);
  send_to_char("You gather all of your strength and concentrate...\n\r",ch);

  /* Now do what we are doing. */
  if(failurefactor > 499 || ch->move < 100){
    /* Large failure. */
    ch->move = (ch->move * 2) / 3;
    send_to_char("... but utterly fail to teleport.\n\r",ch);
    act("$n utterly fails to teleport!",ch,NULL,NULL,TO_ROOM);
    return;
  }
  if (number_percent() > get_skill(ch, gsn_tport)) {
    ch->move = (ch->move * 2) / 3;
    send_to_char("... but utterly fail to teleport.\n\r",ch);
    act("$n utterly fails to teleport!",ch,NULL,NULL,TO_ROOM);
    check_improve(ch, gsn_tport, FALSE, 1);
    return;
  }
  /* Do this here to avoid secondary calls below */
  check_improve(ch, gsn_tport, TRUE, 1);

  /* Reset violence timer if active */
  if(ch->fight > 0)
    ch->fight = 270;

  if(failurefactor > 99){
    /* Random teleport */
    ch->move -= 75; /* MOVE COST */
    pRoomIndex = get_random_room(ch);
    act("$n disappears with a loud bang!",ch,NULL,NULL,TO_ROOM);
    char_from_room(ch); char_to_room(ch,pRoomIndex);
    act("$n appears with a loud bang!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You teleported... but I don't think it worked...\n\r",ch);
    do_look(ch,"auto");
    return;
  }
  /* Now, random roll to see if we win or not */
  tmp=number_range(0,100);
  if(tmp > failurefactor){
    /* Win */
    ch->move -= 75; /* MOVE COST */
    act("$n disappears with a loud bang!",ch,NULL,NULL,TO_ROOM);
    char_from_room(ch); char_to_room(ch,victim->in_room);
    if(ch->pet != NULL){
      char_from_room(ch->pet); char_to_room(ch->pet,victim->in_room);
    }
    act("$n appears with a loud bang!",ch,NULL,NULL,TO_ROOM);
    do_look(ch,"auto");
    return;
  }else{
    /* Lose */
    tmp = (failurefactor - tmp);
    if(tmp > 10){
      /* Random teleport */
      ch->move -= 75; /* MOVE COST */
      pRoomIndex = get_random_room(ch);
      act("$n disappears with a loud bang!",ch,NULL,NULL,TO_ROOM);
      char_from_room(ch); char_to_room(ch,pRoomIndex);
      act("$n appears with a loud bang!",ch,NULL,NULL,TO_ROOM);
      send_to_char("You teleported... but I don't think it worked...\n\r",ch);
      do_look(ch,"auto");
      return;
    }else{
      /* Index of target room */
      pRoomIndex=get_room_index(victim->in_room->vnum);
      /* Now wander X number of exits */
      while(tmp > 0){
    EXIT_DATA *pexit;
    int door=0;

    /* Decrement counter */
    tmp--;

    door=number_range(0,5);

    if((pexit = ch->in_room->exit[door]) != NULL
       && pexit->u1.to_room != NULL){
      /* Good exit - Take it */
      pRoomIndex = pexit->u1.to_room;
    }
      }
      /* All done wandering, go */
      ch->move -= 75; /* MOVE COST */
      pRoomIndex = get_random_room(ch);
      act("$n disappears with a loud bang!",ch,NULL,NULL,TO_ROOM);
      char_from_room(ch); char_to_room(ch,pRoomIndex);
      act("$n appears with a loud bang!",ch,NULL,NULL,TO_ROOM);
      send_to_char("You teleported... but I don't think it worked...\n\r",ch);
      do_look(ch,"auto");
      return;
    }
    /* Done with failure */
  }
  /* Not failure or win (!) */
}

void do_rescue(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument(argument, arg);
    if (arg[0] == '\0') {
    send_to_char("Rescue whom?\n\r", ch);
    return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
    send_to_char("They aren't here.\n\r", ch);
    return;
    }

    if (victim == ch) {
    send_to_char("What about fleeing instead?\n\r", ch);
    return;
    }

    if (!IS_NPC(ch) && IS_NPC(victim)) {
    send_to_char("They don't need your help!\n\r", ch);
    return;
    }

    if (ch->fighting == victim) {
    send_to_char("Too late.\n\r", ch);
    return;
    }

    if ((fch = victim->fighting) == NULL) {
    send_to_char("That person isn't fighting right now.\n\r", ch);
    return;
    }

    if (IS_NPC(fch) && !is_same_group(ch, victim)) {
    send_to_char("Kill stealing is not permitted.\n\r", ch);
    return;
    }

    /* Added this because DEAD flagged people could be attacked */
    /* 8/26/2000 --Vorlin */

    if (IS_AFFECTED(ch, PLR_DEAD)) {
    send_to_char("You're currently incapable of doing this.\n\r", ch);
    return;
    }

    WAIT_STATE(ch, number_range(1, 1.5) * PULSE_VIOLENCE);

    if (number_percent() > get_skill(ch, gsn_rescue)) {
    send_to_char("You fail the rescue.\n\r", ch);
    check_improve(ch, gsn_rescue, FALSE, 1);
    return;
    }

    act("You rescue $N!", ch, NULL, victim, TO_CHAR);
    act("$n rescues you!", ch, NULL, victim, TO_VICT);
    act("$n rescues $N!", ch, NULL, victim, TO_NOTVICT);
    check_improve(ch, gsn_rescue, TRUE, 1);

    stop_fighting(fch, FALSE);
    stop_fighting(victim, FALSE);

    check_killer(ch, fch);

    if (ch->fighting == NULL)
    set_fighting(ch, fch);
    if (fch->fighting == NULL)
    set_fighting(fch, ch);

    return;
}





void do_disarm(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill(ch, gsn_disarm)) == 0) {
    send_to_char("You don't know how to disarm opponents.\n\r", ch);
    return;
    }

    if (get_eq_char(ch, WEAR_WIELD) == NULL
    && ((hth = get_skill(ch, gsn_hand_to_hand)) == 0
        || (IS_NPC(ch) && !IS_SET(ch->off_flags, OFF_DISARM)))) {
    send_to_char("You must wield a weapon to disarm.\n\r", ch);
    return;
    }

    if ((victim = ch->fighting) == NULL) {
    send_to_char("You aren't fighting anyone.\n\r", ch);
    return;
    }

    if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL) {
    send_to_char("Your opponent is not wielding a weapon.\n\r", ch);
    return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch, get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim, get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch, get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if (get_eq_char(ch, WEAR_WIELD) == NULL)
    chance = chance * hth / 150;
    else
    chance = chance * ch_weapon / 100;

    chance += (ch_vict_weapon / 2 - vict_weapon) / 2;

    /* dex vs. strength */
    chance += get_curr_stat(ch, STAT_DEX);
    chance -= 2 * get_curr_stat(victim, STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* and now the attack */
    if (number_percent() < chance) {
    WAIT_STATE(ch, skill_table[gsn_disarm].beats);
    disarm(ch, victim);
    check_improve(ch, gsn_disarm, TRUE, 1);
    } else {
    WAIT_STATE(ch, skill_table[gsn_disarm].beats);
    act("You fail to disarm $N.", ch, NULL, victim, TO_CHAR);
    act("$n tries to disarm you, but fails.", ch, NULL, victim,
        TO_VICT);
    act("$n tries to disarm $N, but fails.", ch, NULL, victim,
        TO_NOTVICT);
    check_improve(ch, gsn_disarm, FALSE, 1);
    }

    check_killer(ch, victim);
    return;
}



void do_sla(CHAR_DATA * ch, char *argument)
{
    send_to_char("If you want to SLAY, spell it out.\n\r", ch);
    return;
}

void do_surrender( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    if ( (mob = ch->fighting) == NULL )
    {
    send_to_char( "But you're not fighting!\n\r", ch );
    return;
    }
    act( "You surrender to $N!", ch, NULL, mob, TO_CHAR );
    act( "$n surrenders to you!", ch, NULL, mob, TO_VICT );
    act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT );
    stop_fighting( ch, TRUE );

    if ( !IS_NPC( ch ) && IS_NPC( mob ) 
    &&   ( !HAS_TRIGGER( mob, TRIG_SURR ) 
        || !mp_percent_trigger( mob, ch, NULL, NULL, TRIG_SURR ) ) )
    {
    act( "$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR );
    multi_hit( mob, ch, TYPE_UNDEFINED );
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_slay(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);
    if (arg[0] == '\0') {
    send_to_char("Slay whom?\n\r", ch);
    return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
    send_to_char("They aren't here.\n\r", ch);
    return;
    }

    if (ch == victim) {
    send_to_char("Suicide is a mortal sin.\n\r", ch);
    return;
    }

    if (!IS_NPC(victim) && victim->level >= get_trust(ch)) {
    send_to_char("You failed.\n\r", ch);
    return;
    }

    act("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
    act("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
    act("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
    raw_kill(victim, "The blackened remains of %s are lying here.");
    return;
}

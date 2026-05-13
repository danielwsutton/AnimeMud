/*
This .h or .c file is part of a Rom 2.4b2 code written by
Russel Taylor.  It has been further enhanced and edited by Mical
and Kyler.  MOST bugs are removed from this release, and color has
been added.  It also has several new features, and many changes
from the original code.  There are no back doors, and few bugs
left in the code.

  Kyler and I ask that if you use this code base, you add
  that this code is greatly enhanced from Rom 2.4b2, along with
  Russel Taylor's name.
  
	Mical/Kyler
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
#include "music.h"
#include "bets.h"

void talk_auction args((char *argument));

/* command procedures needed */
DECLARE_DO_FUN(do_quit);

/*
* Local functions.
*/

int hit_gain args((CHAR_DATA * ch));
int mana_gain args((CHAR_DATA * ch));
int move_gain args((CHAR_DATA * ch));
void mobile_update args((void));
void weather_update args((void));
void char_update args((void));
void obj_update args((void));
void aggr_update args((void));
void raw_kill args((CHAR_DATA * victim));
void quest_update args((void));	/* Vassago - quest.c */
void rp_update args((void));	/*TG980628 */
void auction_update args((void));
void talk_auction args((char *argument));
/* Suzuran - For jail timer */
void jail_update args((void));

/* used for saving */
int save_number = 0;

/*
 * Advancement stuff.
*/

void advance_level(CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    int add_hp, add_mana, add_move, add_prac;

    ch->pcdata->last_level =
	(ch->played + (int) (current_time - ch->logon)) / 3600;

    add_hp =
	con_app[get_curr_stat(ch, STAT_CON)].hitp +
	number_range(class_table[ch->class].hp[MIN],
		     class_table[ch->class].hp[MAX]);
    add_mana =
	wis_app[get_curr_stat(ch, STAT_WIS)].mana +
	number_range(class_table[ch->class].mana[MIN],
		     class_table[ch->class].mana[MAX]);

    add_move =
	dex_app[get_curr_stat(ch, STAT_DEX)].move +
	number_range(class_table[ch->class].move[MIN],
		     class_table[ch->class].move[MAX]);    

    add_prac = wis_app[get_curr_stat(ch, STAT_WIS)].practice;

/*  Commented out the 95% kneecap -- KV020318
    add_hp = add_hp * .95;
    add_mana = add_mana * .95;
    add_move = add_move * .95; 
*/
    add_hp = UMAX(8, add_hp);
    add_mana = UMAX(3, add_mana);
    add_move = UMAX(6, add_move);

    if (ch->race == race_lookup("Demon")) {
		if (ch->class == class_lookup("Warrior")) {	
		    add_hp = BOOST_50_PERCENT(add_hp);
		} else {
		    add_hp = BOOST_30_PERCENT(add_hp);
		}
    }
    if (ch->race == race_lookup("Esper")) {
    	add_mana = TIMES_2(add_mana);
    	add_move = BOOST_20_PERCENT(add_move);
    	if (ch->class == class_lookup("Monk"))
    		// this help esper monks not have terrible hp
    		add_hp = BOOST_10_PERCENT(add_hp);
    }
    if (ch->race == race_lookup("Feline")) {
    	add_hp = REDUCE_10_PERCENT(add_hp);
    	add_move = BOOST_20_PERCENT(add_move);
    }
    if (ch->race == race_lookup("High-Elf")) {
    	add_mana = BOOST_20_PERCENT(add_mana);
    }
    if (ch->race == race_lookup("Saiyan")) {
    	add_hp = BOOST_10_PERCENT(add_hp);
    }

    ch->max_hit += add_hp;
    ch->max_mana += add_mana;
    ch->max_move += add_move;
    ch->practice += add_prac;
    ch->train += 1;
    ch->pcdata->perm_hit += add_hp;
    ch->pcdata->perm_mana += add_mana;
    ch->pcdata->perm_move += add_move;

    sprintf(buf,
	    "Your gain/loss is: %d/%d hp, %d/%d m, %d/%d mv %d/%d prac.\n\r",
	    add_hp, ch->max_hit, add_mana, ch->max_mana, add_move,
	    ch->max_move, add_prac, ch->practice);
    send_to_char(buf, ch);
    return;
}

void gain_exp(CHAR_DATA * ch, int gain)
{
    char buf[MAX_STRING_LENGTH];
    int chance;

    if (IS_NPC(ch) || ch->level >= LEVEL_HERO)
		return;

    chance = number_percent();
    ch->exp = UMAX(exp_per_level(ch, ch->pcdata->points), ch->exp + gain);

    while (ch->level < LEVEL_HERO && ch->exp >= exp_per_level(ch, ch->pcdata->points) * (ch->level + 1)) {
		if (ch->race == race_lookup("Human")
		    && chance <= 2
		    && ch->alignment == 1000
		    && ch->pcdata->pkset == TRUE
		    && (ch->class == class_lookup("Mage")
			|| ch->class == class_lookup("Monk")
			|| ch->class == class_lookup("Cleric"))
		    && ch->level >= 25
		    && ch->level <= 50) {
		    ch->race = race_lookup("Esper");
		    send_to_char("`^Burning in the fires of power, you become one with magick!!``\n\r", ch);
		}

		if (ch->race == race_lookup("Human")
		    && chance <= 2
		    && ch->alignment == -1000
		    && ch->pcdata->pkset == TRUE
		    && (ch->class == class_lookup("Warrior")
			|| ch->class == class_lookup("Thief")
			|| ch->class == class_lookup("Ninja"))
		    && ch->level >= 25
		    && ch->level <= 50) {
		    ch->race = race_lookup("Demon");
		    send_to_char("`!Fires of hell engulf your soul, and you become a minion of the damned!``\n\r", ch);
		    group_add(ch, "hellbender", FALSE);
		    group_add(ch, "demon rage", FALSE);
		    ch->pcdata->learned[gsn_demonrage] = 100;
		}

		send_to_char("You raise a level!!  ", ch);
		ch->level += 1;
		sprintf(buf, "$N has attained level %d!", ch->level);
		wiznet(buf, ch, NULL, WIZ_LEVELS, 0, 0);
		advance_level(ch);

		if (IS_SET(ch->act, PLR_DEAD))
		    REMOVE_BIT(ch->act, PLR_DEAD);

		save_char_obj(ch);
	}
    return;
}

/*
 * Regeneration stuff.
 */
int hit_gain(CHAR_DATA * ch)
{
    int gain, number;

    if (ch->in_room == NULL)
	return 0;

    if (IS_NPC(ch)) {
	gain = 5 + ch->level;
	if (IS_AFFECTED(ch, AFF_REGENERATION))
	    gain = TIMES_3(gain);
	if (IS_SET(ch->act, ACT_STABLED_HORSE))
	    gain = TIMES_3(gain);
	switch (ch->position) {
	default:
	    break;
	case POS_SLEEPING:
	    gain = TIMES_3(gain);
	    break;
	case POS_RESTING:
	    gain = TIMES_2(gain);
	    break;
	case POS_FIGHTING:
	    gain = REDUCE_50_PERCENT(gain);
	    break;
	}
    } else {
	gain = UMAX(3, get_curr_stat(ch, STAT_CON) - 3 + ch->level / 2);
	gain += class_table[ch->class].hp[MAX] - 10;
	number = number_percent();
	if (number < get_skill(ch, gsn_fast_healing)) {
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch, gsn_fast_healing, TRUE, 8);
	}

	if (number < get_skill(ch, gsn_regeneration)) {
	    gain += number * gain / 50;
	    if (ch->hit < ch->max_hit)
		check_improve(ch, gsn_regeneration, TRUE, 8);
	}

	switch (ch->position) {
	default:
	    gain = REDUCE_50_PERCENT(gain);
	    break;
	case POS_SLEEPING:
	    gain = TIMES_2(gain);
	    break;
	case POS_RESTING:
	    break;
	case POS_FIGHTING:
	    gain /= 3;
	    break;
	}

	if (ch->pcdata->condition[COND_HUNGER] == 0)
	    gain = REDUCE_50_PERCENT(gain);

	if (ch->pcdata->condition[COND_THIRST] == 0)
	    gain = REDUCE_50_PERCENT(gain);

    }

    gain = gain * ch->in_room->heal_rate / 100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if (IS_AFFECTED(ch, AFF_POISON))
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch, AFF_HASTE) || IS_AFFECTED(ch, AFF_SLOW))
	gain = REDUCE_50_PERCENT(gain);

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain(CHAR_DATA * ch)
{
    int gain, number;

    if (ch->in_room == NULL)
	return 0;

    if (IS_NPC(ch)) {
	gain = 5 + ch->level;

	if (IS_AFFECTED(ch, AFF_REGENERATION))
	    gain *= 3;

	switch (ch->position) {
	default:
	    break;
	case POS_SLEEPING:
	    gain = TIMES_3(gain);
	    break;
	case POS_RESTING:
	    gain = TIMES_2(gain);
	    break;
	case POS_FIGHTING:
	    gain = REDUCE_50_PERCENT(gain);
	    break;
	}
    } else {
	gain = (get_curr_stat(ch, STAT_WIS)
		+ get_curr_stat(ch, STAT_INT) + ch->level) / 2;
	gain += UMAX(0, class_table[ch->class].mana[MAX] - 10);
	number = number_percent();

	if (number < get_skill(ch, gsn_meditation)) {
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
		check_improve(ch, gsn_meditation, TRUE, 8);
	}

	if (number < get_skill(ch, gsn_regeneration)) {
	    gain += number * gain / 50;
	    if (ch->mana < ch->max_mana)
		check_improve(ch, gsn_regeneration, TRUE, 8);
	}

	switch (ch->position) {
	default:
	    gain /= 2;
	    break;
	case POS_SLEEPING:
	    gain *= 2;
	    break;
	case POS_RESTING:
	    break;
	case POS_FIGHTING:
	    gain /= 3;
	    break;
	}

	if (ch->pcdata->condition[COND_HUNGER] == 0)
	    gain /= 2;

	if (ch->pcdata->condition[COND_THIRST] == 0)
	    gain /= 2;
    }

    gain = gain * ch->in_room->mana_rate / 100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[4] / 100;

    if (IS_AFFECTED(ch, AFF_POISON))
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch, AFF_HASTE) || IS_AFFECTED(ch, AFF_SLOW))
	gain /= 2;

    return UMIN(gain, ch->max_mana - ch->mana);
}

int move_gain(CHAR_DATA * ch)
{
    int gain, number;

    if (ch->in_room == NULL)
	return 0;

    if (IS_NPC(ch)) {
	gain = ch->level;
	if (IS_SET(ch->act, ACT_STABLED_HORSE))
	    gain *= 2;
    } else {
	gain = UMAX(15, ch->level);
	gain += UMAX(0, class_table[ch->class].move[MAX] - 7);

	if (IS_AFFECTED(ch, AFF_REGENERATION))
	    gain *= 3;

	switch (ch->position) {
	case POS_SLEEPING:
	    gain += get_curr_stat(ch, STAT_DEX) * 2;
	    break;
	case POS_RESTING:
	    gain += get_curr_stat(ch, STAT_DEX);
	    break;
	}

	if (ch->pcdata->condition[COND_HUNGER] == 0)
	    gain /= 2;

	if (ch->pcdata->condition[COND_THIRST] == 0)
	    gain /= 2;
    }

    number = number_percent();

    if (!IS_NPC(ch) && number < get_skill(ch, gsn_recovery)) {
	gain += number * gain / 100;
	if (ch->move < ch->max_move)
	    check_improve(ch, gsn_recovery, TRUE, 8);
    }

    gain = gain * ch->in_room->heal_rate / 100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if (IS_AFFECTED(ch, AFF_POISON))
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch, AFF_HASTE) || IS_AFFECTED(ch, AFF_SLOW))
	gain /= 2;

    return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition(CHAR_DATA * ch, int iCond, int value)
{
    int condition;

    if (value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
	return;

    condition = ch->pcdata->condition[iCond];

    if (condition == -1)
	return;

    ch->pcdata->condition[iCond] = URANGE(0, condition + value, 48);

    if (ch->pcdata->condition[iCond] == 0) {
	switch (iCond) {
	case COND_HUNGER:
	    send_to_char("`&You are `*hungry``.\n\r", ch);
	    ++ch->pcdata->dhunger;
	    if (ch->pcdata->dhunger > 19) {
		ch->pcdata->dhunger = 18;
		if (ch->hit > 20)
		    ch->hit = ch->hit - 10;
		if (ch->mana > 20)
		    ch->mana = ch->mana - 10;
		if (ch->move > 35)
		    ch->move = ch->move - 25;
		send_to_char("Your Stomach begins to cramp fiercely.\n\r",
			     ch);
	    }
	    break;

	case COND_THIRST:
	    send_to_char("`&You are `^thirsty``.\n\r", ch);
	    ++ch->pcdata->dthirst;
	    if (ch->pcdata->dthirst > 24) {
		ch->pcdata->dthirst = 23;
		if (ch->hit > 20)
		    ch->hit = ch->hit - 10;
		if (ch->mana > 20)
		    ch->mana = ch->mana - 10;
		if (ch->move > 40)
		    ch->move = ch->move - 30;
		send_to_char("Your throat stings from thirst.\n\r", ch);
	    }
	    break;

	case COND_DRUNK:
	    if (condition != 0)
		send_to_char("You are sober.\n\r", ch);
	    break;
	}
    }

    return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
*/
void mobile_update(void)
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */

    for (ch = char_list; ch != NULL; ch = ch_next) {
	ch_next = ch->next;

	if (!IS_NPC(ch) || ch->in_room == NULL
	    || IS_AFFECTED(ch, AFF_CHARM)) continue;

	if (ch->in_room->area->empty
	    && !IS_SET(ch->act, ACT_UPDATE_ALWAYS)) continue;

	/* Examine call for special procedure */
	if (ch->spec_fun != 0) {
	    if ((*ch->spec_fun) (ch))
		continue;
	}

	if (ch->pIndexData->pShop != NULL)	/* give him some gold */
	    if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth) {
		ch->gold +=
		    ch->pIndexData->wealth * number_range(1, 20) / 5000000;
		ch->silver +=
		    ch->pIndexData->wealth * number_range(1, 20) / 50000;
	    }

	/*
	 * Check triggers only if mobile still in default position
	 */
	if ( ch->position == ch->pIndexData->default_pos )
	  {
	    /* Delay */
            if ( HAS_TRIGGER( ch, TRIG_DELAY)
		 &&   ch->mprog_delay > 0 )
	      {
		if ( --ch->mprog_delay <= 0 )
		  {
		    mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_DELAY );
		    continue;
		  }
	      }
	    if ( HAS_TRIGGER( ch, TRIG_RANDOM) )
	      {
		if( mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_RANDOM ) )
		  continue;
	      }
	  }

	/* That's all for sleeping / busy monster, and empty zones */
	if (ch->position != POS_STANDING)
	    continue;

	/* Scavenge */
	if (IS_SET(ch->act, ACT_SCAVENGER)
	    && ch->in_room->contents != NULL && number_bits(6) == 0) {
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max = 1;
	    obj_best = 0;
	    for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
		if (CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj)
		    && obj->cost > max && obj->cost > 0) {
		    obj_best = obj;
		    max = obj->cost;
		}
	    }

	    if (obj_best) {
		obj_from_room(obj_best);
		obj_to_char(obj_best, ch);
		act("$n gets $p.", ch, obj_best, NULL, TO_ROOM);
	    }
	}

	/* Wander */
	if (!IS_SET(ch->act, ACT_SENTINEL)
	    && number_bits(3) == 0
	    && (door = number_bits(5)) <= 5
	    && (pexit = ch->in_room->exit[door]) != NULL
	    && pexit->u1.to_room != NULL
	    && !IS_SET(pexit->exit_info, EX_CLOSED)
	    && !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	    && (!IS_SET(ch->act, ACT_STAY_AREA)
		|| pexit->u1.to_room->area == ch->in_room->area)
	    && (!IS_SET(ch->act, ACT_OUTDOORS)
		|| !IS_SET(pexit->u1.to_room->room_flags, ROOM_INDOORS))
	    && (!IS_SET(ch->act, ACT_INDOORS)
		|| IS_SET(pexit->u1.to_room->room_flags, ROOM_INDOORS))) {
	    move_char(ch, door, FALSE);
	}
    }

    return;
}

/*
 * Update the weather.
 */
void weather_update(void)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    switch (++time_info.hour) {
    case 5:
	weather_info.sunlight = SUN_LIGHT;
	strcat(buf, "The day has begun.\n\r");
	break;
    case 6:
	weather_info.sunlight = SUN_RISE;
	strcat(buf, "The sun rises in the east.\n\r");
	break;
    case 19:
	weather_info.sunlight = SUN_SET;
	strcat(buf, "The sun slowly disappears in the west.\n\r");
	break;
    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat(buf, "The night has begun.\n\r");
	break;
    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if (time_info.day >= 35) {
	time_info.day = 0;
	time_info.month++;
    }

    if (time_info.month >= 17) {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if (time_info.month >= 9 && time_info.month <= 16)
	diff = weather_info.mmhg > 985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change = UMAX(weather_info.change, -12);
    weather_info.change = UMIN(weather_info.change, 12);
    weather_info.mmhg += weather_info.change;
    weather_info.mmhg = UMAX(weather_info.mmhg, 960);
    weather_info.mmhg = UMIN(weather_info.mmhg, 1040);

    switch (weather_info.sky) {
    default:
	bug("Weather_update: bad sky %d.", weather_info.sky);
	weather_info.sky = SKY_CLOUDLESS;
	break;
    case SKY_CLOUDLESS:
	if (weather_info.mmhg < 990
	    || (weather_info.mmhg < 1010 && number_bits(2) == 0)) {
	    strcat(buf, "The sky is getting cloudy.\n\r");
	    weather_info.sky = SKY_CLOUDY;
	}
	break;
    case SKY_CLOUDY:
	if (weather_info.mmhg < 970
	    || (weather_info.mmhg < 990 && number_bits(2) == 0)) {
	    strcat(buf, "It starts to rain.\n\r");
	    weather_info.sky = SKY_RAINING;
	}

	if (weather_info.mmhg > 1030 && number_bits(2) == 0) {
	    strcat(buf, "The clouds disappear.\n\r");
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;
    case SKY_RAINING:
	if (weather_info.mmhg < 970 && number_bits(2) == 0) {
	    strcat(buf, "Lightning flashes in the sky.\n\r");
	    weather_info.sky = SKY_LIGHTNING;
	}

	if (weather_info.mmhg > 1030
	    || (weather_info.mmhg > 1010 && number_bits(2) == 0)) {
	    strcat(buf, "The rain stopped.\n\r");
	    weather_info.sky = SKY_CLOUDY;
	}
	break;
    case SKY_LIGHTNING:
	if (weather_info.mmhg > 1010
	    || (weather_info.mmhg > 990 && number_bits(2) == 0)) {
	    strcat(buf, "The lightning has stopped.\n\r");
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if (buf[0] != '\0')
	for (d = descriptor_list; d != NULL; d = d->next) {
	    if (d->connected == CON_PLAYING
		&& IS_OUTSIDE(d->character) && IS_AWAKE(d->character))
		send_to_char(buf, d->character);
	}

    return;
}

/*
 * Update all chars, including mobs.
 */
void char_update(void)
{
    CHAR_DATA *ch, *ch_next, *ch_quit;
#ifndef SZ_NOAFKQUITFIX
    DESCRIPTOR_DATA *d = NULL;
    char buf[MAX_STRING_LENGTH];
#endif

    ch_quit = NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;

    for (ch = char_list; ch != NULL; ch = ch_next) {
	AFFECT_DATA *paf, *paf_next;
	ch_next = ch->next;

	if (ch->timer > 30)
	    ch_quit = ch;

	/* BO990303 - Anti Alt Jumping PK login timer */
	if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
	    && (ch->pcdata->logintimer > 1))
	    --ch->pcdata->logintimer;

	if (!IS_NPC(ch) && ch->pcdata->logintimer == 1) {
	    if (ch->pcdata->pkset)
		send_to_char
		    ("You have stayed the duration of time needed to PK, good hunting.\n\r",
		     ch);
	    ch->pcdata->logintimer = 0;
	}

	if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
	&& (ch->pcdata->bank_timer > 1)) {
	--ch->pcdata->bank_timer;
	}

	if (!IS_NPC(ch) && ch->pcdata->bank_timer == 1) {
	send_to_char("Your bank privileges have been restored.\n\r", ch);
	ch->pcdata->bank_timer = 0;
	}

	if (ch->position >= POS_STUNNED) {
	    /* check to see if we need to go home */
	    if (IS_NPC(ch) && ch->zone != NULL
		&& ch->zone != ch->in_room->area && ch->desc == NULL
		&& ch->fighting == NULL
		&& IS_SET(ch->in_room->room_flags, ROOM_SAFE)
		&& !IS_AFFECTED(ch, AFF_CHARM) && number_percent() < 5) {
		act("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
		extract_char(ch, TRUE);
		continue;
	    }

	    if (ch->hit < ch->max_hit)
		ch->hit += hit_gain(ch);
	    else
		ch->hit = ch->max_hit;

	    if (ch->mana < ch->max_mana)
		ch->mana += mana_gain(ch);
	    else
		ch->mana = ch->max_mana;

	    if (ch->move < ch->max_move)
		ch->move += move_gain(ch);
	    else
		ch->move = ch->max_move;
	}

	if (ch->position == POS_STUNNED)
	    update_pos(ch);

	if (!IS_NPC(ch) && !IS_IMMORTAL(ch)) {
	    OBJ_DATA *obj;

	    if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
		&& obj->item_type == ITEM_LIGHT && obj->value[2] > 0) {
		if (--obj->value[2] == 0 && ch->in_room != NULL) {
		    --ch->in_room->light;
		    act("$p goes out.", ch, obj, NULL, TO_ROOM);
		    act("$p flickers and goes out.", ch, obj, NULL,
			TO_CHAR);
		    extract_obj(obj);
		} else if (obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.", ch, obj, NULL, TO_CHAR);
	    }

	    if (IS_IMMORTAL(ch))
		ch->timer = 0;

	    if (++ch->timer >= 12)
		if (ch->was_in_room == NULL && ch->in_room != NULL) {
		    ch->was_in_room = ch->in_room;
		    if (ch->fighting != NULL)
			stop_fighting(ch, TRUE);

		    act("$n disappears into the void.", ch, NULL, NULL,
			TO_ROOM);
		    send_to_char("You disappear into the void.\n\r", ch);

		    if (ch->level > 1)
			save_char_obj(ch);
		    char_from_room(ch);
		    char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
		}

	    gain_condition(ch, COND_DRUNK, -1);
	    gain_condition(ch, COND_FULL,
			   ch->size > SIZE_MEDIUM ? -4 : -2);
	    gain_condition(ch, COND_THIRST, -1);
	    gain_condition(ch, COND_HUNGER,
			   ch->size > SIZE_MEDIUM ? -2 : -1);
	}

	for (paf = ch->affected; paf != NULL; paf = paf_next) {
	    paf_next = paf->next;

	    if (paf->duration > 0) {
		paf->duration--;

		if (number_range(0, 4) == 0 && paf->level > 0)
		    paf->level--;	/* spell strength fades with time */
	    } else if (paf->duration < 0) {;
	    } else {
		if (paf_next == NULL || paf_next->type != paf->type
		    || paf_next->duration > 0)
		    if (paf->type > 0 && skill_table[paf->type].msg_off) {
			send_to_char(skill_table[paf->type].msg_off, ch);
			send_to_char("\n\r", ch);
		    }

		affect_remove(ch, paf);
	    }
	}

	if (!IS_NPC(ch) && ch->pcdata->granted != NULL) {
	    GRANT_DATA *gran, *gran_next, *gran_prev;
	    char buf[MAX_STRING_LENGTH];

	    gran_prev = ch->pcdata->granted;

	    for (gran = ch->pcdata->granted; gran != NULL;
		 gran = gran_next) {
		gran_next = gran->next;

		if (gran->duration > 0)
		    gran->duration--;

		if (gran->duration == 0) {
		    if (gran == ch->pcdata->granted)
			ch->pcdata->granted = gran_next;
		    else
			gran_prev->next = gran->next;
		    sprintf(buf,
			    "Your time runs out on the %s command.\n\r",
			    gran->name);
		    send_to_char(buf, ch);
		    free_string(gran->name);
		    free_mem(gran, sizeof(*gran));
		    gran = NULL;
		}

		if (gran != NULL)
		    gran_prev = gran;
	    }
	}

	/* Underwater stuffs - Suzuran */
	/* DO NOT REFER TO CH IF THE CHAR TAKES DAMAGE!
	   IF YOU DO LETHAL DAMAGE TO A MOBILE, AND THE
	   MOBILE DIES, CH IS INVALID AND WILL CRASH
	   THE MUD! */

	if (ch->in_room == NULL
	    || ch->in_room->sector_type != SECT_UNDERWATER) {
	    ch->air = 10;	/* 10 ticks is good enough */
	} else {
	    /* We are underwater.  Drop some air, echo appropriate
	       message */
	    /* Unless we have gills */
	    if (ch->air > 0 && !(ch->parts & PART_GILLS)
		&& !IS_IMMORTAL(ch)) {
		ch->air--;
		send_to_char("You exhale a stream of bubbles.\n\r", ch);
		act("$n exhales a stream of bubbles.", ch, NULL, NULL,
		    TO_ROOM);
	    }
	    if (ch->air > 0 && ch->air < 3) {
		send_to_char("You are running out of air!\n\r", ch);
	    }
	    if (ch->air <= 0) {
		send_to_char("You choke down a mouthful of water!\n\r",
			     ch);
		act("$n inhales a load of water!", ch, NULL, NULL,
		    TO_ROOM);
		/* Drowning is a very swift and very painful death */
		damage_old(ch, ch, 750 + number_range(50, 500), 0,
			   DAM_DROWNING, FALSE);
	    }
	}

	/*
	   * Careful with the damages here,
	   *   MUST NOT refer to ch after damage taken,
	   *   as it may be lethal damage (on NPC).
	 */
	if (is_affected(ch, gsn_plague) && ch != NULL) {
	    AFFECT_DATA *af, plague;
	    CHAR_DATA *vch;
	    int dam;

	    if (ch->in_room == NULL)
		return;

	    act("$n writhes in agony as plague sores erupt from $s skin.",
		ch, NULL, NULL, TO_ROOM);
	    send_to_char("You writhe in agony from the plague.\n\r", ch);

	    for (af = ch->affected; af != NULL; af = af->next) {
		if (af->type == gsn_plague)
		    break;
	    }

	    if (af == NULL) {
		REMOVE_BIT(ch->affected_by, AFF_PLAGUE);
		return;
	    }

	    if (af->level == 1)
		return;

	    plague.where = TO_AFFECTS;
	    plague.type = gsn_plague;
	    plague.level = af->level - 1;
	    plague.duration = number_range(1, 2 * plague.level);
	    plague.location = APPLY_STR;
	    plague.modifier = -5;
	    plague.bitvector = AFF_PLAGUE;

	    for (vch = ch->in_room->people; vch != NULL;
		 vch = vch->next_in_room) {
		if (!saves_spell(plague.level - 2, vch, DAM_DISEASE)
		    && !IS_IMMORTAL(vch)
		    && !IS_AFFECTED(vch, AFF_PLAGUE) && number_bits(4) == 0
		    && !(ch->desc == NULL && !IS_NPC(ch))
		    && !(vch->desc == NULL && !IS_NPC(vch))) {
		    send_to_char("You feel hot and feverish.\n\r", vch);
		    act("$n shivers and looks very ill.", vch, NULL, NULL,
			TO_ROOM);
		    affect_join(vch, &plague);
		}
	    }

	    dam = UMIN(ch->level, af->level / 5 + 1);
	    ch->mana -= dam;
	    ch->move -= dam;
	    damage_old(ch, ch, dam, gsn_plague, DAM_DISEASE, FALSE);
	}
	    else if (IS_AFFECTED(ch, AFF_POISON) && ch != NULL
		     && !IS_AFFECTED(ch, AFF_SLOW)) {
	    AFFECT_DATA *poison;

	    poison = affect_find(ch->affected, gsn_poison);

	    if (poison != NULL) {
		act("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
		send_to_char("You shiver and suffer.\n\r", ch);
		damage_old(ch, ch, poison->level / 10 + 1, gsn_poison,
			   DAM_POISON, FALSE);
	    }
	} else if (ch->position == POS_INCAP && number_range(0, 1) == 0)
	    damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
	else if (ch->position == POS_MORTAL)
	    damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
    }

    /*
       * Autosave and autoquit.
       * Check that these chars still exist.
     */
    for (ch = char_list; ch != NULL; ch = ch_next) {
	/* Don't save pets. That screws the player to player linkage. */
	if (!IS_VALID(ch)) {
	    bug("update_char: Invalid char to autosave\n", 0);
	    break;
	}

	ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
	    save_char_obj(ch);

	if (ch == ch_quit)
#ifndef SZ_NOAFKQUITFIX
	{
	    /* Forcing a do_quit doesn't work. */
	    sprintf(buf, "Idledropping %s.", ch->name);
	    log_string(buf);
	    d = ch->desc;
	    save_char_obj(ch);
	    extract_char(ch, TRUE);
	    if (d != NULL) {
		close_socket(d);
	    }
	}
#else
	    do_quit(ch, "");
#endif
    }

    return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update(void)
{
    OBJ_DATA *obj, *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for (obj = object_list; obj != NULL; obj = obj_next) {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	/* go through affects and decrement */
	for (paf = obj->affected; paf != NULL; paf = paf_next) {
	    paf_next = paf->next;
	    if (paf->duration > 0) {
		paf->duration--;
		if (number_range(0, 4) == 0 && paf->level > 0)
		    paf->level--;	/* spell strength fades with time */
	    } else if (paf->duration < 0) {
	    } else {
		if (paf_next == NULL
		    || paf_next->type != paf->type
		    || paf_next->duration > 0) {
		    if (paf->type > 0 && skill_table[paf->type].msg_obj) {
			if (obj->carried_by != NULL) {
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj,
				rch, obj, NULL, TO_CHAR);
			}

			if (obj->in_room != NULL
			    && obj->in_room->people != NULL) {
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj,
				rch, obj, NULL, TO_ALL);
			}
		    }
		}

		affect_remove_obj(obj, paf);
	    }
	}

	if (obj->timer <= 0 || --obj->timer > 0)
	    continue;

	switch (obj->item_type) {
	default:
	    message = "$p crumbles into dust.";
	    break;
	case ITEM_FOUNTAIN:
	    message = "$p dries up.";
	    break;
	case ITEM_CORPSE_NPC:
	    message = "$p decays into dust.";
	    break;
	case ITEM_CORPSE_PC:
	    message = "$p decays into dust.";
	    break;
	case ITEM_FOOD:
	    message = "$p decomposes.";
	    break;
	case ITEM_POTION:
	    message = "$p has evaporated from disuse.";
	    break;
	case ITEM_PORTAL:
	    message = "$p fades out of existence.";
	    break;
	case ITEM_CONTAINER:
	case ITEM_SADDLE:
	    if (CAN_WEAR(obj, ITEM_WEAR_FLOAT)) {
		if (obj->contains)
		    message =
			"$p flickers and vanishes, spilling its contents on the floor.";
		else
		    message = "$p flickers and vanishes.";
	    } else
		message = "$p crumbles into dust.";

	    break;
	}

	if (obj->carried_by != NULL) {
	    if (IS_NPC(obj->carried_by)
		&& obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->silver += obj->cost / 5;
	    else {
		act(message, obj->carried_by, obj, NULL, TO_CHAR);

		if (obj->wear_loc == WEAR_FLOAT)
		    act(message, obj->carried_by, obj, NULL, TO_ROOM);
	    }
	}
	    else if (obj->in_room != NULL
		     && (rch = obj->in_room->people) != NULL) {
	    if (!
		(obj->in_obj
		 && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
		 && !CAN_WEAR(obj->in_obj, ITEM_TAKE))) {
		act(message, rch, obj, NULL, TO_ROOM);
		act(message, rch, obj, NULL, TO_CHAR);
	    }
	}

	if (
	    (obj->item_type == ITEM_CORPSE_PC
	     || obj->wear_loc == WEAR_FLOAT) && obj->contains) {	/* save the contents */
	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj) {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj)	/* in another object */
		    obj_to_obj(t_obj, obj->in_obj);
		else if (obj->carried_by)	/* carried */
		    if (obj->wear_loc == WEAR_FLOAT)
			if (obj->carried_by->in_room == NULL)
			    extract_obj(t_obj);
			else
			    obj_to_room(t_obj, obj->carried_by->in_room);
		    else
			obj_to_char(t_obj, obj->carried_by);
		else if (obj->in_room == NULL)	/* destroy it */
		    extract_obj(t_obj);
		else		/* to a room */
		    obj_to_room(t_obj, obj->in_room);
	    }
	}

	extract_obj(obj);
    }

    return;
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update(void)
{
    CHAR_DATA *wch, *wch_next, *ch, *ch_next, *vch, *vch_next, *victim;

    for (wch = char_list; wch != NULL; wch = wch_next) {
	wch_next = wch->next;

	if (IS_NPC(wch)
	    || wch->level >= LEVEL_IMMORTAL
	    || wch->in_room == NULL || wch->in_room->area->empty) continue;

	for (ch = wch->in_room->people; ch != NULL; ch = ch_next) {
	    int count;

	    ch_next = ch->next_in_room;

	    if (!IS_NPC(ch)
		|| !IS_SET(ch->act, ACT_AGGRESSIVE)
		|| IS_SET(ch->in_room->room_flags, ROOM_SAFE)
		|| IS_AFFECTED(ch, AFF_CALM)
		|| ch->fighting != NULL || IS_AFFECTED(ch, AFF_CHARM)
		|| !IS_AWAKE(ch)
		|| (IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch))
		|| !can_see(ch, wch) || number_bits(1) == 0)
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */

	    count = 0;
	    victim = NULL;

	    for (vch = wch->in_room->people; vch != NULL; vch = vch_next) {

		vch_next = vch->next_in_room;

		if (!IS_NPC(vch)
		    && vch->level < LEVEL_IMMORTAL
		    && ch->level >= vch->level - 5
		    && (!IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch))
		    && can_see(ch, vch)) {
		    if (number_range(0, count) == 0)
			victim = vch;
		    count++;
		}
	    }

	    if (victim == NULL)
		continue;

	    multi_hit(ch, victim, TYPE_UNDEFINED);
	}
    }
    return;
}

/*
* Handle all kinds of updates.
* Called once per pulse from game loop.
* Random times to defeat tick-timing clients and players.
*/
void update_handler(void)
{
    static int pulse_area;
    static int pulse_mobile;
    static int pulse_violence;
    static int pulse_point;
    static int pulse_music;

    if (--pulse_area <= 0) {
	pulse_area = PULSE_AREA;
	/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
	quest_update();
	area_update();
	jail_update();
	rp_update();
    }

    if (--pulse_music <= 0) {
	pulse_music = PULSE_MUSIC;
	song_update();
    }

    if (--pulse_mobile <= 0) {
	pulse_mobile = PULSE_MOBILE;
	mobile_update();
    }

    if (--pulse_violence <= 0) {
	pulse_violence = PULSE_VIOLENCE;
	violence_update();
    }

    if (--pulse_point <= 0) {
	wiznet("TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
	pulse_point = PULSE_TICK;
	/* number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 ); */
	weather_update();
	char_update();
	obj_update();
    }

    auction_update();
    aggr_update();
    tail_chain();
    return;
}

void auction_update(void)
{
    char buf[MAX_STRING_LENGTH];
    AUCTION_LIST *ptemp_obj;

    if (auction_info->current_obj != NULL) {

	if (--auction_info->pulse <= 0) {	/* decrease pulse */
	    auction_info->pulse = PULSE_AUCTION;
	    switch (++auction_info->going) {	/* increase the going state */
	    case 1:		/* going once */

	    case 2:		/* going twice */

		if ((auction_info->bet > 0)
		    && (auction_info->buyer != NULL)) {
		    sprintf(buf, "%s: going %s for %ld.",
			    auction_info->current_obj->item->short_descr,
			    ((auction_info->going == 1) ? "once" :
			     "twice"), auction_info->bet);
		    talk_auction(buf);
		} else {
		    sprintf(buf, "%s: going %s (no bet received yet).",
			    auction_info->current_obj->item->short_descr,
			    ((auction_info->going == 1) ? "once" :
			     "twice"));
		    talk_auction(buf);
		}
		break;

	    case 3:		/* SOLD! */
		if ((auction_info->buyer != NULL)
		    && (auction_info->bet > 0)) {
		    sprintf(buf, "%s sold to %s for %ld.",
			    auction_info->current_obj->item->short_descr,
			    IS_NPC(auction_info->buyer) ? auction_info->
			    buyer->short_descr : auction_info->buyer->name,
			    auction_info->bet);
		    talk_auction(buf);
		    obj_to_char(auction_info->current_obj->item,
				auction_info->buyer);
		    act
			("The auctioneer appears before you in a puff of smoke and hands you $p.",
			 auction_info->buyer,
			 auction_info->current_obj->item, NULL, TO_CHAR);
		    act
			("The auctioneer appears before $n, and hands $m $p",
			auction_info->buyer,
			auction_info->current_obj->item, NULL, TO_ROOM);

		    auction_info->current_obj->seller->bank += auction_info->bet;	/* give him the money */

		    /* set pointers for next item in auction_list */
		    ptemp_obj = auction_info->current_obj;
		    auction_info->current_obj =
			auction_info->current_obj->next;

		    /* free memory from auction_list */
		    free_mem(ptemp_obj, sizeof(*ptemp_obj));

		} else {	/* not sold */
		    ptemp_obj = auction_info->current_obj;
		    sprintf(buf,
			    "No bets received for %s - object has been removed.",
			    ptemp_obj->item->short_descr);
		    talk_auction(buf);
		    act
			("The auctioneer appears before you to return $p to you.",
			 ptemp_obj->seller, ptemp_obj->item, NULL,
			 TO_CHAR);
		    act
			("The auctioneer appears before $n to return $p to $m.",
			 ptemp_obj->seller, ptemp_obj->item, NULL,
			 TO_ROOM);
		    obj_to_char(ptemp_obj->item, ptemp_obj->seller);

		    /* set pointers for next item in auction_list */
		    auction_info->current_obj =
			auction_info->current_obj->next;

		    /* free memory from auction_list */
		    free_mem(ptemp_obj, sizeof(*ptemp_obj));
		}		/* else */

		/* if item still in auction_list, resets parameters for auction_info */
		if (auction_info->current_obj != NULL) {
		    auction_info->bank_bet = 0;
		    auction_info->bet = 0;
		    auction_info->buyer = NULL;
		    auction_info->pulse = PULSE_AUCTION;
		    auction_info->going = 0;

		    if (auction_info->current_obj->min_bid > 0) {	/* has min bid */
			sprintf(buf,
				"A new item has been received: %s.'\n\r`VM`8i`Vn B`8i`Vd `8: '`#%ld``",
				auction_info->current_obj->item->
				short_descr,
				auction_info->current_obj->min_bid);
			talk_auction(buf);
		    } else {	/* no min bid */
			sprintf(buf, "A new item has been received: %s.",
				auction_info->current_obj->item->
				short_descr);
			talk_auction(buf);
		    }
		}
	    }
	}
    }

    return;
}				/* func */

/*TG980628*/
void rp_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;

    for (d = descriptor_list; d != NULL; d = d->next)
	if (d->character != NULL && d->connected == CON_PLAYING) {
	    ch = d->character;

	    if (ch->next_points > 0)
		ch->next_points--;
	}
}

/* Suzuran's */
void jail_update(void){

  CHAR_DATA *ch;
  DESCRIPTOR_DATA *d;
  ROOM_INDEX_DATA *rIndex=NULL;

  extern void do_look(CHAR_DATA *ch,char *argument);

  for(d=descriptor_list; d != NULL; d = d->next){

    ch = d->character;
    
    /* Heeey, paranoia! */
    if(ch == NULL){continue;}
    if(IS_NPC(ch)){continue;}

    if(ch->jailtime){
      ch->jailtime--;
      if(ch->jailtime == 0){
	send_to_char("You've served your time, you may go. Behave yourself!\n\r",ch);
	wiznet("Jailer releasing $N.",ch,NULL,0,0,95);
	if((rIndex = get_room_index(ROOM_VNUM_TEMPLE))==NULL){
	  send_to_char("Ano, I can't find the recall point?  This can't be good.",ch);
	  wiznet("Jailer cannot find recall point! (This is not good.)",NULL,NULL,0,0,95);
	  continue;
	}
	char_from_room(ch);
	char_to_room(ch,rIndex);
	do_look(ch,"auto");
	continue;
      }
    }
  }
}

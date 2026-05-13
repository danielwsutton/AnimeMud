
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
#include "magic.h"
#include "interp.h"

void do_heal(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost, sn;
    SPELL_FUN *spell;
    char *words;

    /* check for healer */
    for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
	if (IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER))
	    break;
    }

    if (mob == NULL) {
	send_to_char("You can't do that here.\n\r", ch);
	return;
    }
/*
    if (ch->level > 5)
    {
	send_to_char( "I will only heal the youngsters of this world.\n\r",ch);
	send_to_char( "You're too old.\n\r", ch );
	return;
    }
*/
    one_argument(argument, arg);

    if (arg[0] == '\0') {
	/* display price list */
	act("$N says 'I offer the following spells:'", ch, NULL, mob,
	    TO_CHAR);
	send_to_char("  light: cure light wounds      10 gold\n\r", ch);
	send_to_char("  serious: cure serious wounds  15 gold\n\r", ch);
	send_to_char("  critic: cure critical wounds  25 gold\n\r", ch);
	send_to_char("  heal: healing spell           50 gold\n\r", ch);
	send_to_char("  blind: cure blindness         20 gold\n\r", ch);
	send_to_char("  disease: cure disease         15 gold\n\r", ch);
	send_to_char("  poison:  cure poison          25 gold\n\r", ch);
	send_to_char("  uncurse: remove curse         50 gold\n\r", ch);
	send_to_char("  refresh: restore movement      5 gold\n\r", ch);
	send_to_char("  mana:  restore mana           10 gold\n\r", ch);
	send_to_char("  sanctuary: casts sanctuary   500 gold\n\r", ch);
	send_to_char("  frenzy: casts frenzy         375 gold\n\r", ch);
	send_to_char("  bless: increases hitroll     300 gold\n\r", ch);
	send_to_char(" Type heal <type> to be healed.\n\r", ch);
	return;
    }

    if (!str_prefix(arg, "light")) {
	spell = spell_cure_light;
	sn = skill_lookup("cure light");
	words = "`@judicandus dies``";
	cost = 1000;
    }

    else if (!str_prefix(arg, "serious")) {
	spell = spell_cure_serious;
	sn = skill_lookup("cure serious");
	words = "`@judicandus gzfuajg``";
	cost = 1600;
    }

    else if (!str_prefix(arg, "critical")) {
	spell = spell_cure_critical;
	sn = skill_lookup("cure critical");
	words = "`@judicandus qfuhuqar``";
	cost = 2500;
    }

    else if (!str_prefix(arg, "heal")) {
	spell = spell_heal;
	sn = skill_lookup("heal");
	words = "`@pzar``";
	cost = 5000;
    }

    else if (!str_prefix(arg, "blindness")) {
	spell = spell_cure_blindness;
	sn = skill_lookup("cure blindness");
	words = "`@judicandus noselacri``";
	cost = 2000;
    }

    else if (!str_prefix(arg, "disease")) {
	spell = spell_cure_disease;
	sn = skill_lookup("cure disease");
	words = "`@judicandus eugzagz``";
	cost = 1500;
    }

    else if (!str_prefix(arg, "poison")) {
	spell = spell_cure_poison;
	sn = skill_lookup("cure poison");
	words = "`@judicandus sausabru``";
	cost = 2500;
    }

    else if (!str_prefix(arg, "uncurse") || !str_prefix(arg, "curse")) {
	spell = spell_remove_curse;
	sn = skill_lookup("remove curse");
	words = "`@candussido judifgz``";
	cost = 5000;
    }

    else if (!str_prefix(arg, "mana") || !str_prefix(arg, "energize")) {
	spell = NULL;
	sn = -1;
	words = "`@energizer``";
	cost = 1000;
    }


    else if (!str_prefix(arg, "refresh") || !str_prefix(arg, "moves")) {
	spell = spell_refresh;
	sn = skill_lookup("refresh");
	words = "`@candusima``";
	cost = 500;
    }


    else if (!str_prefix(arg, "sanctuary")) {
	spell = spell_sanctuary;
	sn = skill_lookup("sanctuary");
	words = "`@gaiqhjabral``";
	cost = 50000;
    }


    else if (!str_prefix(arg, "frenzy")) {
	spell = spell_frenzy;
	sn = skill_lookup("frenzy");
	words = "`@ycandusikl``";
	cost = 37500;
    }

    else if (!str_prefix(arg, "bless")) {
	spell = spell_bless;
	sn = skill_lookup("bless");
	words = "`@fido``";
	cost = 30000;
    }

    else {
	act("$N says 'Type 'heal' for a list of spells.'",
	    ch, NULL, mob, TO_CHAR);
	return;
    }

    if (cost > (ch->gold * 100 + ch->silver)) {
	act("$N says 'You do not have enough gold for my services.'",
	    ch, NULL, mob, TO_CHAR);
	return;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);

    deduct_cost(ch, cost);
    mob->gold += cost;
    /* 
       Got rid of room says for healer, sent to character only...
       --Vorlin
     */

    act("The healer utters the words '$T'.", ch, NULL, words, TO_CHAR);

    if (spell == NULL) {	/* restore mana trap...kinda hackish */
	ch->mana += dice(2, 8) + mob->level / 3;
	ch->mana = UMIN(ch->mana, ch->max_mana);
	send_to_char("A warm glow passes through you.\n\r", ch);
	return;
    }

    if (sn == -1)
	return;

    spell(sn, mob->level, mob, ch, TARGET_CHAR);
}

void do_bank(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *mob;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    long amount;

    for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
	if (IS_NPC(mob) && IS_SET(mob->act, ACT_IS_BANKER))
	    break;
    }

    if(IS_NPC(ch)){send_to_char("NO BANK FOR YOU!\n\r",ch); return;}

    if (mob == NULL) {
	send_to_char("You can't do that here.\n\r", ch);
	return;
    }

    if (ch->pcdata->bank_timer > 0) {
	send_to_char("Your banking privileges have been revoked.\n\r", ch);
	return;
    }

    smash_tilde(argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    /*strcpy(arg2, argument); */

    if (arg1[0] == '\0') {
	act
	    ("`6$N says, '`^What kind of transaction would you like to make?`6'``",
	     ch, NULL, mob, TO_CHAR);
	send_to_char("   withdraw `#<`3gold amount`#>``\n\r", ch);
	send_to_char("   deposit  `#<`3gold amount`#>``\n\r", ch);
	return;
    } else {
	if (!str_prefix(arg1, "withdraw")) {
	    amount = atoi(arg2);
	    if (amount < 1) {
		act
		    ("`6$N says, '`^How much gold are you withdrawing?`6'``",
		    ch, NULL, mob, TO_CHAR);
		return;
	    } else {
		if (amount > ch->bank) {
		    act
			("`6$N says, '`^You don't have that much gold in your account!`6'``",
			 ch, NULL, mob, TO_CHAR);
		    return;
		}
		ch->bank -= amount;
		ch->gold += amount;
		sprintf(buf,
			"You take `#%ld`` gold from your account.\n\r",
			amount);
		send_to_char(buf, ch);
		act("`6$N says, '`^Thank you for your business!`6'``", ch,
		    NULL, mob, TO_CHAR);
		act("$n just made a transaction.", ch, NULL, mob,
		    TO_NOTVICT);
		return;
	    }
	}
	if (!str_prefix(arg1, "deposit")) {
	    amount = atoi(arg2);

	    if (amount < 0) {	
		if (!IS_IMMORTAL(ch)) {
		ch->pcdata->bank_timer = number_range(4,10);
		sprintf(buf, 
		"Your bank privileges have been revoked for %d ticks.\n\r", 
		ch->pcdata->bank_timer);
		send_to_char(buf, ch);
		do_save(ch, "");
		} else {
		send_to_char("You know you shouldn't be trying to abuse this.\n\r", ch);
		}
		return;
	    } else {
		if (amount > ch->gold) {
		    act
			("`6$N says, '`^You don't have that much gold on you!`6'``",
			 ch, NULL, mob, TO_CHAR);
		    return;
		}
		ch->bank += amount;
		ch->gold -= amount;
		sprintf(buf, "You put `#%ld`` gold into your account.\n\r",
			amount);
		send_to_char(buf, ch);
		act("`6$N says, '`^Thank you for your business!`6'``", ch,
		    NULL, mob, TO_CHAR);
		act("$n just made a transaction.", ch, NULL, mob,
		    TO_NOTVICT);
		return;
	    }
	}
	if (!str_prefix(arg1, "balance")) {
	    sprintf(buf, "You have `#%ld`` gold in your account.\n\r",
		    ch->bank);
	    send_to_char(buf, ch);
	    return;
	} else {
	    act("$N says, 'That's an invalid transaction.'", ch, NULL, mob,
		TO_CHAR);
	    return;
	}
    }
}

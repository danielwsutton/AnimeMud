
The dual function,
    I put it in act_obj.c.You will have to modify interp.
    c and.h as usual with new skills / spells. -- ------ACT_OBJ.C-- --
    --------------------void do_dual(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    if (get_skill(ch, gsn_dual_wield) < 0
	|| ch->level < skill_table[gsn_dual_wield].skill_level[ch->class]) {
	send_to_char
	    ("You almost cut your hands off as you try to hold one weapon in each hand.\n\r",
	     ch);
	return;
    }

    if (argument[0] == '\0') {
	send_to_char("Wield which weapon in your off-hand?\n\r", ch);
	return;
    }

    if ((obj = get_obj_carry(ch, argument)) == NULL) {
	send_to_char("You have no such thing in your backpack.\n\r", ch);
	return;
    }

    if ((get_eq_char(ch, WEAR_SHIELD) != NULL) ||
	(get_eq_char(ch, WEAR_HOLD) != NULL)) {
	send_to_char
	    ("You cannot wield two weapons while using a shield or holding an item.\n\r",
	     ch);
	return;
    }

    if (ch->level < (obj->level - EQ_LEVEL_RESTRICTION)) {
	sprintf(buf, "You must be level %d to use this object.\n\r",
		(obj->level - EQ_LEVEL_RESTRICTION) > 0 ?
		(obj->level - EQ_LEVEL_RESTRICTION) : obj->level);
	send_to_char(buf, ch);
	act("$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM);
	return;
    }

    if (get_eq_char(ch, WEAR_WIELD) == NULL) {
	send_to_char
	    ("You need to wield a primary weapon, before you using a secondarey one.\n\r",
	     ch);
	return;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)) {
	send_to_char
	    ("You cannot wield a two-handed weapon with one hand.\n\r",
	     ch);
	return;
    }

    if (get_obj_weight(obj) >
	(str_app[get_curr_stat(ch, STAT_STR)].wield * 5)) {
	send_to_char
	    ("This weapon is too heavy to be used as a secondary weapon by you.\n\r",
	     ch);
	return;
    }

    if ((get_obj_weight(obj)) >
	get_obj_weight(get_eq_char(ch, WEAR_WIELD))) {
	send_to_char
	    ("You must wield the lightest of your weapons in your off-hand.",
	     ch);
	return;
    }

    if (!remove_obj(ch, WEAR_SECONDARY, TRUE))
	return;

    act("$n wields `&$p`` in $s off-hand.", ch, obj, NULL, TO_ROOM);
    act("You wield `&$p`` in your off-hand.", ch, obj, NULL, TO_CHAR);
    equip_char(ch, obj, WEAR_SECONDARY);
    return;
}

-----------END ACT_OBJ.C section-- -- -----Here comes the actual combat -
    implementation. -- ---------FIGHT.C-- -- --------Add a new variable,
    "secondary" in the declaration of one_hit. --
    -void one_hit
args((CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary));

---Replace multi_hit with this:
---
/*
 * Do one group of attacks.
 */
// void multi_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt)
// {
//     int chance;

//     /* decrement the wait */
//     if (ch->desc == NULL)
// 	ch->wait = UMAX(0, ch->wait - PULSE_VIOLENCE);

//     if (ch->desc == NULL)
// 	ch->daze = UMAX(0, ch->daze - PULSE_VIOLENCE);


//     /* no attacks for stunnies -- just a check */
//     if (ch->position < POS_RESTING)
// 	return;

//     if (IS_NPC(ch)) {
// 	mob_hit(ch, victim, dt);
// 	return;
//     }

//     one_hit(ch, victim, dt, FALSE);

// 	// Add extra hit if dual wielding
// 	if (get_eq_char(ch, WEAR_SECONDARY)
// 	    && number_percent() < get_skill(ch, gsn_dual_wield))
// 	{
// 	    one_hit(ch, victim, dt, TRUE);
// 	}

// 	// Unarmed Ninja bonus strike (similiar to dual wield)
// 	if (!get_eq_char(ch, WEAR_WIELD)
// 	    && !get_eq_char(ch, WEAR_SECONDARY)
// 	    && !get_eq_char(ch, WEAR_SHIELD)
// 	    && ch->class == class_lookup("Ninja")
// 	    && number_percent() < get_skill(ch, gsn_dual_wield))
// 	{
// 	    one_hit(ch, victim, dt, FALSE);
// 	}

//     if (ch->fighting != victim)
// 	return;

//     if (IS_AFFECTED(ch, AFF_HASTE))
// 	one_hit(ch, victim, dt, FALSE);

//     if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
// 	return;

//     chance = get_skill(ch, gsn_second_attack) / 2;

//     if (IS_AFFECTED(ch, AFF_SLOW))
// 	chance /= 2;

//     if (is_affected(ch, skill_lookup("cry")))
// 	chance /= 3;

//     if (number_percent() < chance) {
// 	one_hit(ch, victim, dt, FALSE);
// 	check_improve(ch, gsn_second_attack, TRUE, 5);
// 	if (ch->fighting != victim)
// 	    return;
//     }

//     chance /= 3;		/*for the secondary weapon */
//     if (get_eq_char(ch, WEAR_SECONDARY)) {
// 	if (number_percent() < chance) {
// 	    one_hit(ch, victim, dt, TRUE);
// 	    check_improve(ch, gsn_second_attack, TRUE, 5);
// 	    if (ch->fighting != victim)
// 		return;
// 	}
//     }


//     chance = get_skill(ch, gsn_third_attack) / 4;

//     if (IS_AFFECTED(ch, AFF_SLOW))
// 	chance += 10;

//     if (is_affected(ch, skill_lookup("cry")))
// 	chance += 15;

//     if (number_percent() < chance) {
// 	one_hit(ch, victim, dt, FALSE);
// 	check_improve(ch, gsn_third_attack, TRUE, 6);
// 	if (ch->fighting != victim)
// 	    return;
//     }

//     /*EE960618 */
//     chance /= 3;		/*For the secondary weapon */
//     if (get_eq_char(ch, WEAR_SECONDARY)) {
// 	if (number_percent() < chance) {
// 	    one_hit(ch, victim, dt, TRUE);
// 	    check_improve(ch, gsn_third_attack, TRUE, 6);
// 	    if (ch->fighting != victim)
// 		return;
// 	}
//     }

//     chance = get_skill(ch, gsn_fourth_attack) / 5;

//     if (IS_AFFECTED(ch, AFF_SLOW))
// 	chance = 0;

//     if (is_affected(ch, skill_lookup("cry")))
// 	chance += 10;

//     if (number_percent() < chance) {
// 	one_hit(ch, victim, dt, FALSE);
// 	check_improve(ch, gsn_fourth_attack, TRUE, 7);
// 	if (ch->fighting != victim) {
// 	    return;
// 	}
//     }

//     return;
// }

/* 
I one_hit, replace the part where you
find out what weapon you're wielding with this:
   
Figure out the type of damage message.
 */

if (secondary) {		/*EE960524 */
    wield = get_eq_char(ch, WEAR_SECONDARY);
} else {
    wield = get_eq_char(ch, WEAR_WIELD);
}

/*
Replace your check_parry with this:

Check for parry.
 */
bool check_parry(CHAR_DATA * ch, CHAR_DATA * victim)
{
    int chance;

    if (!IS_AWAKE(victim))
	return FALSE;

    chance = get_skill(victim, gsn_parry) / 2;

    if (get_eq_char(victim, WEAR_WIELD) == NULL) {
	if (IS_NPC(victim)) {
	    chance /= 2;
	} else {
	    return FALSE;
	}
    }

    if (!can_see(ch, victim))
	chance /= 2;

    if (number_percent() >= chance + victim->level - ch->level)
	return FALSE;

    if (get_eq_char(ch, WEAR_SECONDARY)) {
	if (number_percent() < get_skill(ch, gsn_dual_wield)) {
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

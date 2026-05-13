
-- ----MAIN FUNCTION PERM(ACT_WIZ.C)-- -- ----void
do_perm(CHAR_DATA * ch, char *argument)
{
    char item[MAX_INPUT_LENGTH], affect[MAX_INPUT_LENGTH],
	mod[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    AFFECT_DATA *aff;

    argument = one_argument(argument, item);
    argument = one_argument(argument, affect);
    argument = one_argument(argument, mod);

    if (item[0] == '\0' || affect[0] == '\0') {
	send_to_char("Syntax: Perm <item> <affect> [modifier]\n\r", ch);
	return;
    }

    if ((obj = get_obj_carry(ch, item)) == NULL) {
	send_to_char("You don't have that item.\n\r", ch);
	return;
    }

    aff = new_affect();
    aff->where = TO_AFFECTS;
    aff->type = skill_lookup(affect);
    aff->level = ch->level;
    aff->duration = -1;
    aff->bitvector = 0;
    if ((aff->location = affect_location(affect)) == -1) {
	send_to_char("There is no such affect.\n\r", ch);
	return;
    }

    if (mod[0] != '\0')
	aff->modifier = atoi(mod);
    else
	aff->modifier = ch->level;

    affect_to_obj(obj, aff);
    send_to_char("Ok.\n\r", ch);
    return;
}




---------FUNCTIONS IN HANDLER.C-- -- -----
/* DECLARATION
 * Local functions.
 */
void affect_modify args((CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd));
void affect_strip_perm args((CHAR_DATA * ch, int sn));
void affect_remove_perm args((CHAR_DATA * ch, AFFECT_DATA * paf));

---
/*
 * Apply or remove an affect to a character.
 * Modified EE960521 to handle permed items
 */
void affect_modify(CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd)
{
    OBJ_DATA *wield;
    AFFECT_DATA *paf_new;	/*perm items */
    int mod, i;

    paf_new = new_affect();	/*perm items */
    mod = paf->modifier;

    if (fAdd) {
	switch (paf->where) {
	case TO_AFFECTS:
	    SET_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    SET_BIT(ch->imm_flags, paf->bitvector);
	    break;
	case TO_RESIST:
	    SET_BIT(ch->res_flags, paf->bitvector);
	    break;
	case TO_VULN:
	    SET_BIT(ch->vuln_flags, paf->bitvector);
	    break;
	}
    } else {
	switch (paf->where) {
	case TO_AFFECTS:
	    REMOVE_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    REMOVE_BIT(ch->imm_flags, paf->bitvector);
	    break;
	case TO_RESIST:
	    REMOVE_BIT(ch->res_flags, paf->bitvector);
	    break;
	case TO_VULN:
	    REMOVE_BIT(ch->vuln_flags, paf->bitvector);
	    break;
	}
	mod = 0 - mod;
    }

    switch (paf->location) {
    default:
	bug("Affect_modify: unknown location %d.", paf->location);
	return;

    case APPLY_NONE:
	break;
    case APPLY_STR:
	ch->mod_stat[STAT_STR] += mod;
	break;
    case APPLY_DEX:
	ch->mod_stat[STAT_DEX] += mod;
	break;
    case APPLY_INT:
	ch->mod_stat[STAT_INT] += mod;
	break;
    case APPLY_WIS:
	ch->mod_stat[STAT_WIS] += mod;
	break;
    case APPLY_CON:
	ch->mod_stat[STAT_CON] += mod;
	break;
    case APPLY_SEX:
	ch->sex += mod;
	break;
    case APPLY_CLASS:
	break;
    case APPLY_LEVEL:
	break;
    case APPLY_AGE:
	break;
    case APPLY_HEIGHT:
	break;
    case APPLY_WEIGHT:
	break;
    case APPLY_MANA:
	ch->max_mana += mod;
	break;
    case APPLY_HIT:
	ch->max_hit += mod;
	break;
    case APPLY_MOVE:
	ch->max_move += mod;
	break;
    case APPLY_GOLD:
	break;
    case APPLY_EXP:
	break;
    case APPLY_AC:
	for (i = 0; i < 4; i++)
	    ch->armor[i] += mod;
	break;
    case APPLY_HITROLL:
	ch->hitroll += mod;
	break;
    case APPLY_DAMROLL:
	ch->damroll += mod;
	break;
    case APPLY_SAVES:
	ch->saving_throw += mod;
	break;
    case APPLY_SAVING_ROD:
	ch->saving_throw += mod;
	break;
    case APPLY_SAVING_PETRI:
	ch->saving_throw += mod;
	break;
    case APPLY_SAVING_BREATH:
	ch->saving_throw += mod;
	break;
    case APPLY_SAVING_SPELL:
	ch->saving_throw += mod;
	break;
    case APPLY_SPELL_AFFECT:
	break;
    case APPLY_SANCTUARY:	/*This is new */
	{

	    paf->type = skill_lookup("sanctuary");
	    if (fAdd) {
		if (is_affected(ch, paf->type)) {
		    send_to_char("You are already in sanctuary.\n\r", ch);
		    break;
		}
		paf->level = mod;
		paf->duration = -1;
		paf->location = APPLY_SANCTUARY;
		paf->modifier = 0;
		paf->bitvector = AFF_SANCTUARY;

		*paf_new = *paf;
		paf_new->next = ch->affected;
		ch->affected = paf_new;
		SET_BIT(ch->affected_by, paf_new->bitvector);

		send_to_char("You are surrounded by a white aura.\n\r",
			     ch);
		act("$n is surrounded by a white aura.", ch, NULL, NULL,
		    TO_ROOM);
		break;
	    } else {
		affect_strip_perm(ch, paf->type);
		send_to_char
		    ("The white aura around your body vanishes.\n\r", ch);
		act("The white aura around $n's body vanishes.\n\r", ch,
		    NULL, NULL, TO_ROOM);
		break;
	    }
	}
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if (!IS_NPC(ch) && (wield = get_eq_char(ch, WEAR_WIELD)) != NULL
	&& get_obj_weight(wield) >
	(str_app[get_curr_stat(ch, STAT_STR)].wield * 10)) {
	static int depth;

	if (depth == 0) {
	    depth++;
	    act("You drop $p.", ch, wield, NULL, TO_CHAR);
	    act("$n drops $p.", ch, wield, NULL, TO_ROOM);
	    obj_from_char(wield);
	    obj_to_room(wield, ch->in_room);
	    depth--;
	}
    }

    return;
}

---void affect_strip_perm(CHAR_DATA * ch, int sn)
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for (paf = ch->affected; paf != NULL; paf = paf_next) {
	paf_next = paf->next;
	if (paf->type == sn)
	    affect_remove_perm(ch, paf);
    }

    return;
}


void affect_remove_perm(CHAR_DATA * ch, AFFECT_DATA * paf)
{
    int where;
    int vector;

    if (ch->affected == NULL) {
	bug("Affect_remove_perm: no affect.", 0);
	return;
    }

    where = paf->where;
    vector = paf->bitvector;

    if (paf == ch->affected) {
	ch->affected = paf->next;
    } else {
	AFFECT_DATA *prev;

	for (prev = ch->affected; prev != NULL; prev = prev->next) {
	    if (prev->next == paf) {
		prev->next = paf->next;
		break;
	    }
	}

	if (prev == NULL) {
	    bug("Affect_remove_perm: cannot find paf.", 0);
	    return;
	}
    }

    free_affect(paf);

    affect_check(ch, where, vector);
    return;
}

------------------[EOB]-- -- ---------------

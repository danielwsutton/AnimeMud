#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_exits);

/*
* Local functions.
*/
void say_spell args((CHAR_DATA * ch, int sn));

/* imported functions */
bool remove_obj args((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj args((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));
void show_list_to_char args((OBJ_DATA * list, CHAR_DATA * ch,
			     bool fShort, bool fShowNothing));
void show_char_to_char args((CHAR_DATA * list, CHAR_DATA * ch));



/*
* Lookup a skill by name.
*/
int skill_lookup(const char *name)
{
    int sn;

    for (sn = 0; sn < MAX_SKILL; sn++) {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	    && !str_prefix(name, skill_table[sn].name))
	    return sn;
    }

    return -1;
}

int find_spell(CHAR_DATA * ch, const char *name)
{
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for (sn = 0; sn < MAX_SKILL; sn++) {
	if (skill_table[sn].name == NULL)
	    break;
	if (skill_table[sn].spell_fun == spell_null)
	    break;

	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	    && !str_prefix(name, skill_table[sn].name)) {
	    if (found == -1)
		found = sn;
	    if (ch->level >= skill_table[sn].skill_level[ch->class]
		&& ch->pcdata->learned[sn] > 0)
		return sn;
	}
    }
    return found;
}

/*
* Lookup a skill by slot number.
* Used for object loading.
*/
int slot_lookup(int slot)
{
    extern bool fBootDb;
    int sn;

    if (slot <= 0)
	return -1;

    for (sn = 0; sn < MAX_SKILL; sn++) {
	if (slot == skill_table[sn].slot)
	    return sn;
    }

    if (fBootDb) {
	bug("Slot_lookup: bad slot %d.", slot);
	abort();
    }

    return -1;
}



/*
* Utter mystical words for an sn.
*/
void say_spell(CHAR_DATA * ch, int sn)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type {
	char *old;
	char *new;
    };

    static const struct syl_type syl_table[] = {
	{" ", " "},
	{"ar", "abra"},
	{"au", "kada"},
	{"bless", "fido"},
	{"blind", "nose"},
	{"bur", "mosa"},
	{"cu", "judi"},
	{"de", "oculo"},
	{"en", "unso"},
	{"light", "dies"},
	{"lo", "hi"},
	{"mor", "zak"},
	{"move", "sido"},
	{"ness", "lacri"},
	{"ning", "illa"},
	{"per", "duda"},
	{"ra", "gru"},
	{"fresh", "ima"},
	{"re", "candus"},
	{"son", "sabru"},
	{"tect", "infra"},
	{"tri", "cula"},
	{"ven", "nofo"},
	{"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
	{"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
	{"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
	{"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
	{"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
	{"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
	{"y", "l"}, {"z", "k"},
	{"", ""}
    };

    buf[0] = '\0';
    for (pName = skill_table[sn].name; *pName != '\0'; pName += length) {
	for (iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++) {
	    if (!str_prefix(syl_table[iSyl].old, pName)) {
		strcat(buf, syl_table[iSyl].new);
		break;
	    }
	}

	if (length == 0)
	    length = 1;
    }

    sprintf(buf2, "$n utters the words, '`5%s``'.", buf);
    sprintf(buf, "$n utters the words, '`5%s``'.", skill_table[sn].name);

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
	if (rch != ch)
	    act(ch->class == rch->class ? buf : buf2, ch, NULL, rch,
		TO_VICT);
    }

    return;
}

int resist(int saves, int level)
{
    if (saves < -10)
	saves = (saves + 10) / 2 - 10;

    if (saves < -25)
	saves = UMAX(saves, -25 - level / 6);

    return saves;
}

/*
* Compute a saving throw.
* Negative apply's make saving throw better.
*/
bool saves_spell(int level, CHAR_DATA * victim, int dam_type)
{
    int save;

    save = 50 + (victim->level - level) - resist(victim->saving_throw, victim->level);
    if (IS_AFFECTED(victim, AFF_BERSERK))
	save += victim->level / 6;

    switch (check_immune(victim, dam_type)) {
    case IS_IMMUNE:
	return TRUE;
    case IS_RESISTANT:
	save += 2;
	break;
    case IS_VULNERABLE:
	save -= 2;
	break;
    }

    save = URANGE(5, save, 95);
    return number_percent() < save;
}

/* RT save for dispels */

bool saves_dispel(int dis_level, int spell_level, int duration)
{
    int save;

    if (duration == -1)
	spell_level += 5;
    /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level);
    save = URANGE(5, save, 95);
    return number_percent() < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel(int dis_level, CHAR_DATA * victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn)) {
	for (af = victim->affected; af != NULL; af = af->next) {
	    if (af->type == sn) {
		if (!saves_dispel(dis_level, af->level, af->duration)) {
		    affect_strip(victim, sn);
		    if (skill_table[sn].msg_off) {
			send_to_char(skill_table[sn].msg_off, victim);
			send_to_char("\n\r", victim);
		    }
		    return TRUE;
		} else
		    af->level--;
	    }
	}
    }
    return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost(CHAR_DATA * ch, int min_mana, int level)
{
    if (ch->level + 2 == level)
	return 1000;
    return UMAX(min_mana, (100 / (2 + ch->level - level)));
}



/*
* The kludgy global is for spells who want more stuff from command line.
*/
char *target_name;

void do_cast(CHAR_DATA * ch, char *argument)
{
    /*char buf[MAX_STRING_LENGTH]; */
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target;

    REMOVE_BIT(ch->affected_by, AFF_HIDE);

    /*
       * Switched NPC's can cast spells, but others can't.
     */
    if (IS_NPC(ch) && ch->desc == NULL)
	return;

    target_name = one_argument(argument, arg1);
    one_argument(target_name, arg2);

    if (arg1[0] == '\0') {
	send_to_char("Cast on who?\n\r", ch);
	return;
    }

    if ((sn = find_spell(ch, arg1)) < 0 || (!IS_NPC(ch)
					    && (ch->level <
						skill_table[sn].skill_level
						[ch->class]
						|| ch->
						pcdata->learned[sn] == 0))) {
	send_to_char("You don't know any spells of that name.\n\r", ch);
	return;
    }

    if (ch->position < skill_table[sn].minimum_position) {
	send_to_char("You can't concentrate enough.\n\r", ch);
	return;
    }

    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	mana = 50;
    else
	mana = UMAX(skill_table[sn].min_mana,
		    100 / (2 + ch->level -
			   skill_table[sn].skill_level[ch->class]));

    if (ch->race == race_lookup("Esper")) {
	mana /= 2;
    }

    /*
       * Locate targets.
     */
    victim = NULL;
    obj = NULL;
    vo = NULL;
    target = TARGET_NONE;

    switch (skill_table[sn].target) {
    default:
	bug("Do_cast: bad target for sn %d.", sn);
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if (arg2[0] == '\0') {
	    if ((victim = ch->fighting) == NULL) {
		send_to_char("Cast the spell at whom?\n\r", ch);
		return;
	    }
	} else {
	    if ((victim = get_char_room(ch, target_name)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	    }
	}

	if (ch == victim) {
	    send_to_char("You can't do that to yourself.\n\r", ch);
	    return;
	}



	if (!IS_NPC(ch)) {

	    if (is_safe(ch, victim) && victim != ch) {
		send_to_char("Not on that target.\n\r", ch);
		return;
	    }
	    if ((IS_SET(victim->act, PLR_DEAD))
		|| ((IS_SET(ch->act, PLR_DEAD)) && (!IS_NPC(victim)))) {
		send_to_char
		    ("You are not permitted to kill this person!\n\r", ch);
		return;
	    }

	    check_killer(ch, victim);
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	    send_to_char("You can't do that on your own follower.\n\r",
			 ch);
	    return;
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	/*
	   if (!str_cmp(arg2, "brew") )
	   {
	   OBJ_DATA *potion;
	   if (!IS_NPC(ch) && ch->mana < mana) 
	   {
	   send_to_char("You don't have enough mana.\n\r",ch);
	   break;
	   return;
	   }

	   if (!IS_IMMORTAL(ch)) {
		WAIT_STATE(ch,skill_table[sn].beats);
	   } else {
		WAIT_STATE(ch, 6);
	   }

	   say_spell(ch,sn);
	   if (number_percent( ) > get_skill(ch,sn) )
	   {
	   send_to_char("You lost your concentration.\n\r",ch);
	   check_improve(ch,sn,FALSE,1);
	   ch->mana -= mana /2;
	   return;
	   }
	   ch->mana -= mana;
	   check_improve(ch,sn,TRUE,1);

	   potion = create_object(get_obj_index(15007), 0 );
	   potion->value[0] = ch->level;
	   potion->value[1] = sn;

	   sprintf(buf,potion->name,skill_table[sn].name);
	   free_string(potion->name);
	   potion->name = str_dup(buf);

	   sprintf(buf,potion->short_descr,skill_table[sn].name);
	   free_string(potion->short_descr);
	   potion->short_descr = str_dup(buf);

	   sprintf(buf,potion->description,skill_table[sn].name);
	   free_string(potion->description);
	   potion->description = str_dup(buf);

	   act("$n has created a potion.",ch,NULL,NULL,TO_ROOM);
	   sprintf(buf,"You've brewed a potion of %s!\n\r", skill_table[sn].name);
	   send_to_char(buf,ch);
	   obj_to_char(potion,ch);
	   return;
	   }
	 */
	if (arg2[0] == '\0') {
	    victim = ch;
	} else {
	    if ((victim = get_char_room(ch, target_name)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	    }
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:


	if (!str_cmp(arg2, "brew")) {
	    /*OBJ_DATA *potion;
	       if (!IS_NPC(ch) && ch->mana < mana)
	       {
	       send_to_char("You don't have enough mana.\n\r",ch);
	       return;
	       }
	       WAIT_STATE(ch,skill_table[sn].beats);
	       say_spell(ch,sn);
	       if (number_percent( ) > get_skill(ch,sn) )
	       {
	       send_to_char("You lost your concentration.\n\r",ch);
	       check_improve(ch,sn,FALSE,1);
	       ch->mana -= mana /2;
	       return;
	       }
	       ch->mana -= mana;
	       check_improve(ch,sn,TRUE,1);
	       potion = create_object(get_obj_index(15007), 0 );
	       potion->value[0] = ch->level;
	       potion->value[1] = sn;

	       sprintf(buf,potion->name,skill_table[sn].name);
	       free_string(potion->name);
	       potion->name = str_dup(buf);

	       sprintf(buf,potion->short_descr,skill_table[sn].name);
	       free_string(potion->short_descr);
	       potion->short_descr = str_dup(buf);

	       sprintf(buf,potion->description,skill_table[sn].name);
	       free_string(potion->description);
	       potion->description = str_dup(buf);

	       act("$n has created a potion.",ch,NULL,NULL,TO_ROOM);
	       sprintf(buf,"You've brewed a potion of %s!\n\r", skill_table[sn].name);
	       send_to_char(buf,ch);
	       obj_to_char(potion,ch);
	     */
	    return;
	}

	else {
	    if (arg2[0] != '\0' && !is_name(target_name, ch->name)) {
		send_to_char("You can't do that.\n\r", ch);
		return;
	    }
	}
	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if (arg2[0] == '\0') {
	    send_to_char("What item should vibrate from this spell?\n\r",
			 ch);
	    return;
	}

	if ((obj = get_obj_carry(ch, target_name)) == NULL) {
	    send_to_char("You are not carrying that.\n\r", ch);
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0') {
	    if ((victim = ch->fighting) == NULL) {
		send_to_char("Cast on whom or what?\n\r", ch);
		return;
	    }

	    target = TARGET_CHAR;
	} else if ((victim = get_char_room(ch, target_name)) != NULL) {
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) {	/* check the sanity of the attack */
	    if (is_safe_spell(ch, victim, FALSE) && victim != ch) {
		send_to_char("Not on that target.\n\r", ch);
		return;
	    }

	    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		send_to_char("You can't do that on your own follower.\n\r",
			     ch);
		return;
	    }

	    if (!IS_NPC(ch))
		check_killer(ch, victim);

	    vo = (void *) victim;
	} else if ((obj = get_obj_here(ch, target_name)) != NULL) {
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	} else {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}
	break;

    case TAR_OBJ_CHAR_DEF:
	if (arg2[0] == '\0') {
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	} else if ((victim = get_char_room(ch, target_name)) != NULL) {
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	} else if ((obj = get_obj_carry(ch, target_name)) != NULL) {
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	} else {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}
	break;
    }

    if (!IS_NPC(ch) && ch->mana < mana) {
	send_to_char("You don't have enough mana.\n\r", ch);
	return;
    }

    if (str_cmp(skill_table[sn].name, "ventriloquate"))
	say_spell(ch, sn);

    if (IS_IMMORTAL(ch)) {
	WAIT_STATE(ch, skill_table[sn].beats / 4);
    } else {
	WAIT_STATE(ch, skill_table[sn].beats);
    }

    if (number_percent() > get_skill(ch, sn)) {
	send_to_char("You lost your concentration.\n\r", ch);
	check_improve(ch, sn, FALSE, 1);
	ch->mana -= mana / 2;
    } else {
	ch->mana -= mana;
	(*skill_table[sn].spell_fun) (sn, ch->level, ch, vo, target);
	check_improve(ch, sn, TRUE, 1);
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
	 || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
	     && target == TARGET_CHAR)) && victim != ch
	&& victim->master != ch) {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for (vch = ch->in_room->people; vch; vch = vch_next) {
	    vch_next = vch->next_in_room;
	    if (victim == vch && victim->fighting == NULL) {
		check_killer(victim, ch);
		multi_hit(victim, ch, TYPE_UNDEFINED);
		break;
	    }
	}
    }

    return;
}

void
obj_cast_spell(int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim,
	       OBJ_DATA * obj)
{
    void *vo;
    int target = TARGET_NONE;

    REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if (sn <= 0)
	return;

    if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0) {
	bug("Obj_cast_spell: bad sn %d.", sn);
	return;
    }

    switch (skill_table[sn].target) {
    default:
	bug("Obj_cast_spell: bad target for sn %d.", sn);
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if (victim == NULL)
	    victim = ch->fighting;
	if (victim == NULL) {
	    send_to_char("You can't do that.\n\r", ch);
	    return;
	}
	if (is_safe(ch, victim) && ch != victim) {
	    send_to_char("Something isn't right...\n\r", ch);
	    return;
	}
	if (!IS_NPC(victim)) {
	    if ((ch->level - victim->level > PK_LEVEL)
		|| (victim->level - ch->level > PK_LEVEL)) {
		send_to_char("You can't do that!\n\r", ch);
		return;
	    }
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if (victim == NULL)
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if (obj == NULL) {
	    send_to_char("You can't do that.\n\r", ch);
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (victim == NULL && obj == NULL) {
	    if (ch->fighting != NULL)
		victim = ch->fighting;
	    else {
		send_to_char("You can't do that.\n\r", ch);
		return;
	    }
	}

	if (victim != NULL) {
	    if (is_safe_spell(ch, victim, FALSE) && ch != victim) {
		send_to_char("Something isn't right...\n\r", ch);
		return;
	    }
	    if (!IS_NPC(victim)) {
		if ((ch->level - PK_LEVEL > victim->level)
		    || (victim->level - PK_LEVEL > ch->level)) {
		    send_to_char("You can't do that!\n\r", ch);
		    return;
		}
	    }
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	} else {
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	break;


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL) {
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	} else if (victim != NULL) {
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	} else {
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}

	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) (sn, level, ch, vo, target);



    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
	 || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
	     && target == TARGET_CHAR)) && victim != ch
	&& victim->master != ch) {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for (vch = ch->in_room->people; vch; vch = vch_next) {
	    vch_next = vch->next_in_room;
	    if (victim == vch && victim->fighting == NULL) {
		check_killer(victim, ch);
		multi_hit(victim, ch, TYPE_UNDEFINED);
		break;
	    }
	}
    }

    return;
}



/*
* Spell functions.
*/
void
spell_acid_blast(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level, 13);
    if (saves_spell(level, victim, DAM_ACID))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_ACID, TRUE);
    return;
}



void spell_anger(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are already pissed off.\n\r", ch);
	else
	    act("$N is already pissed off.", ch, NULL, victim, TO_CHAR);
	return;
    }
    if (is_affected(victim, skill_lookup("concentration"))) {
	if (victim == ch)
	    send_to_char("You are to busy concentrating to be angry.\n\r",
			 ch);
	else
	    act("$N is to busy concentrating to be angry.", ch, NULL,
		victim, TO_CHAR);
	return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;

    af.duration = 2 * level;
    af.location = APPLY_DAMROLL;
    af.modifier = +5;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = 10;
    affect_to_char(victim, &af);
    send_to_char("You become infuriated with anger!\n\r", victim);
    act("$n's eyes become bloodshot.", victim, NULL, NULL, TO_ROOM);
    return;
}



void spell_armor(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are already armored.\n\r", ch);
	else
	    act("$N is already armored.", ch, NULL, victim, TO_CHAR);
	return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 24;
    af.modifier = -20;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    send_to_char("You feel someone protecting you.\n\r", victim);
    if (ch != victim)
	act("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);
    return;
}

void spell_bless(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ) {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj, ITEM_BLESS)) {
	    act("$p is already blessed.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected, gsn_curse);
	    if (!saves_dispel
		(level, paf != NULL ? paf->level : obj->level, 0)) {
		if (paf != NULL)
		    affect_remove_obj(obj, paf);
		act("$p glows a pale blue.", ch, obj, NULL, TO_ALL);
		REMOVE_BIT(obj->extra_flags, ITEM_EVIL);
		return;
	    } else {
		act("The evil of $p is too powerful for you to overcome.",
		    ch, obj, NULL, TO_CHAR);
		return;
	    }
	}

	af.where = TO_OBJECT;
	af.type = sn;
	af.level = level;
	af.duration = 6 + level;
	af.location = APPLY_SAVES;
	af.modifier = -1;
	af.bitvector = ITEM_BLESS;
	affect_to_obj(obj, &af);
	SET_BIT(obj->extra_flags, ITEM_BLESS);

	act("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);
	return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if (victim->position == POS_FIGHTING || is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are already blessed.\n\r", ch);
	else
	    act("$N already has divine favor.", ch, NULL, victim, TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 6 + level;
    af.location = APPLY_HITROLL;
    af.modifier = level / 5;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = 0 - level / 6;
    affect_to_char(victim, &af);
    send_to_char("You feel righteous.\n\r", victim);
    if (ch != victim)
	act("You grant $N the favor of your god.", ch, NULL, victim,
	    TO_CHAR);
    return;
}

void spell_black_hole(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int count, dam;

    act("$n calls forth a swirling black vortex of nothingness!",
	ch, NULL, victim, TO_NOTVICT);
    act("You summon a black hole, banishing $N to a cold vacuum!",
	ch, NULL, victim, TO_CHAR);

    dam = dice(level, number_range(10, 12));

    if (saves_spell(level, victim, DAM_GRAVITY))
	dam /= 2;

    for (count = 1; count <= (level / 30); count++)
	damage_old(ch, victim, dam, sn, DAM_GRAVITY, TRUE);

    return;
}

void spell_blindness(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_BLIND))
	return;

    /* Fail message - KV020805 */
    if (saves_spell(level, victim, DAM_OTHER)) {
	act("You fail to blind $N!", ch, NULL, victim, TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = 1 + level;
    af.bitvector = AFF_BLIND;
    affect_to_char(victim, &af);
    send_to_char("You are blinded!\n\r", victim);
    act("$n appears to be blinded.", victim, NULL, NULL, TO_ROOM);
    return;
}



void
spell_burning_hands(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 14, 16, 18, 20, 22, 24, 26, 28, 30,
	32, 34, 36, 38, 40, 42, 44, 46, 48, 50,
	52, 54, 56, 58, 60, 62, 64, 66, 68, 70,
	72, 74, 76, 78, 80, 82, 84, 86, 88, 90,
	92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
	112, 115, 117, 120, 122, 125, 127, 130, 132, 135,
	137, 140, 142, 145, 147, 150, 152, 155, 157, 160,
	162, 165, 167, 170, 172, 175, 177, 180, 182, 185
    };
    int dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level], dam_each[level] * 2);
    if (saves_spell(level, victim, DAM_FIRE))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_FIRE, TRUE);
    return;
}



void
spell_call_lightning(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if (!IS_OUTSIDE(ch)) {
	send_to_char("You must be out of doors.\n\r", ch);
	return;
    }

    if (weather_info.sky < SKY_RAINING) {
	send_to_char("You need bad weather.\n\r", ch);
	return;
    }

    dam = dice(level / 2, 8);

    send_to_char("The Light's lightning strikes your foes!\n\r", ch);
    act("$n calls The Light's lightning to strike $s foes!", ch, NULL,
	NULL, TO_ROOM);

    for (vch = char_list; vch != NULL; vch = vch_next) {
	vch_next = vch->next;
	if (vch->in_room == NULL)
	    continue;
	if (vch->in_room == ch->in_room) {
	    if (vch != ch && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch)))
		damage_old(ch, vch, saves_spell(level, vch, DAM_LIGHTNING)
			   ? dam / 2 : dam, sn, DAM_LIGHTNING, TRUE);
	    continue;
	}

	if (vch->in_room->area == ch->in_room->area
	    && IS_OUTSIDE(vch) && IS_AWAKE(vch))
	    send_to_char("Lightning flashes in the sky.\n\r", vch);
    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
	if (vch->position == POS_FIGHTING) {
	    count++;
	    if (IS_NPC(vch))
		mlevel += vch->level;
	    else
		mlevel += vch->level / 2;
	    high_level = UMAX(high_level, vch->level);
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch))	/* always works */
	mlevel = 0;

    if (number_range(0, chance) >= mlevel) {	/* hard to stop large fights */
	for (vch = ch->in_room->people; vch != NULL;
	     vch = vch->next_in_room) {
	    if (IS_NPC(vch)
		&& (IS_SET(vch->imm_flags, IMM_MAGIC)
		    || IS_SET(vch->act, ACT_UNDEAD)))
		return;

	    if (IS_AFFECTED(vch, AFF_CALM) || IS_AFFECTED(vch, AFF_BERSERK)
		|| is_affected(vch, skill_lookup("frenzy")))
		return;

	    send_to_char("A wave of calm passes over you.\n\r", vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
		stop_fighting(vch, FALSE);


	    af.where = TO_AFFECTS;
	    af.type = sn;
	    af.level = level;
	    af.duration = level / 4;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
		af.modifier = -5;
	    else
		af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch, &af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch, &af);
	}
    }
}


void
spell_cancellation(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim) &&
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)) ||
	(IS_NPC(ch) && !IS_NPC(victim)) || (ch != victim)) {
	send_to_char("You failed, try dispel magic.\n\r", ch);
	return;
    }

    /* unlike dispel magic, the victim gets NO save */

    /* begin running through the spells */

    if (check_dispel(level, victim, skill_lookup("anger"))) {
	found = TRUE;
	act("$n looses $s temper.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("armor")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("bless")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("blindness"))) {
	found = TRUE;
	act("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("calm"))) {
	found = TRUE;
	act("$n no longer looks so peaceful...", victim, NULL, NULL,
	    TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("concentration"))) {
	found = TRUE;
	act("$n looses $s train of thought.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("cry"))) {
	found = TRUE;
	act("$n gains control and stops crying.", victim, NULL, NULL,
	    TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("change sex"))) {
	found = TRUE;
	act("$n looks more like $mself again.", victim, NULL, NULL,
	    TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("charm person"))) {
	found = TRUE;
	act("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("chill touch"))) {
	found = TRUE;
	act("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("curse")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect evil")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect good")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect hidden")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect invis")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect hidden")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect magic")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("faerie fire"))) {
	act("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("fly"))) {
	act("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("frenzy"))) {
	act("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);;
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("giant strength"))) {
	act("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("haste"))) {
	act("$n is no longer moving so quickly.", victim, NULL, NULL,
	    TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("infravision")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("invis"))) {
	act("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("light"))) {
	act("The light around $n's body disappears.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("mass invis"))) {
	act("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("pass door")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("protection evil")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("protection good")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("resistance")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("sanctuary"))) {
	act("The white aura around $n's body vanishes.",
	    victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("shield"))) {
	act("The shield protecting $n vanishes.", victim, NULL, NULL,
	    TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("sleep")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("slow"))) {
	act("$n is no longer moving so slowly.", victim, NULL, NULL,
	    TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("stone skin"))) {
	act("$n's skin regains its normal texture.", victim, NULL, NULL,
	    TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("weaken"))) {
	act("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("chibichibification"))) {
      found = TRUE;
      act("$n looks more like $mself again.", victim, NULL, NULL,
          TO_ROOM);
    }


    if (found)
	send_to_char("Ok.\n\r", ch);
    else
	send_to_char("Spell failed.\n\r", ch);
}

void
spell_cause_light(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice(level, 4);

    /* Feline mod here, since RES_HARM does not exist.. - Kevin */
    if (victim->race == race_lookup("Feline"))
        dam /= 1.5;

    damage_old(ch, (CHAR_DATA *) vo, dam, sn, DAM_HARM, TRUE);
    return;
}

void
spell_cause_critical(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice(level, 8);

    /* Feline mod here, since RES_HARM does not exist.. - Kevin */
    if (victim->race == race_lookup("Feline"))
        dam /= 1.5;

    damage_old(ch, (CHAR_DATA *) vo, dam, sn, DAM_HARM, TRUE);
    return;
}



void
spell_cause_serious(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = dice(level, 6);

    /* Feline mod here, since RES_HARM does not exist.. - Kevin */
    if (victim->race == race_lookup("Feline"))
        dam /= 1.5;

    damage_old(ch, (CHAR_DATA *) vo, dam, sn, DAM_HARM, TRUE);
    return;
}

void
spell_chain_lightning(int sn, int level, CHAR_DATA * ch, void *vo,
		      int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict, *last_vict, *next_vict;
    bool found;
    int dam;

    /* first strike */

    act("A lightning bolt leaps from $n's hand and arcs to $N.",
	ch, NULL, victim, TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.",
	ch, NULL, victim, TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!",
	ch, NULL, victim, TO_VICT);

    dam = dice(level, 6);
    if (saves_spell(level, victim, DAM_LIGHTNING))
	dam /= 3;
    damage_old(ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    last_vict = victim;
    level -= 4;			/* decrement damage */

    /* new targets */
    while (level > 0) {
	found = FALSE;
	for (tmp_vict = ch->in_room->people;
	     tmp_vict != NULL; tmp_vict = next_vict) {
	    next_vict = tmp_vict->next_in_room;
	    if (!is_safe_spell(ch, tmp_vict, TRUE)
		&& tmp_vict != last_vict) {
		found = TRUE;
		last_vict = tmp_vict;
		act("The bolt arcs to $n!", tmp_vict, NULL, NULL, TO_ROOM);
		act("The bolt hits you!", tmp_vict, NULL, NULL, TO_CHAR);
		dam = dice(level, 6);
		if (saves_spell(level, tmp_vict, DAM_LIGHTNING))
		    dam /= 3;
		damage_old(ch, tmp_vict, dam, sn, DAM_LIGHTNING, TRUE);
		level -= 4;	/* decrement damage */
	    }
	}			/* end target searching loop */

	if (!found) {		/* no target found, hit the caster */
	    if (ch == NULL)
		return;

	    if (last_vict == ch) {	/* no double hits */
		act("The bolt seems to have fizzled out.", ch, NULL, NULL,
		    TO_ROOM);
		act("The bolt grounds out through your body.", ch, NULL,
		    NULL, TO_CHAR);
		return;
	    }

	    last_vict = ch;
	    act("The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_ROOM);
	    send_to_char("You are struck by your own lightning!\n\r", ch);
	    dam = dice(level, 6);
	    if (saves_spell(level, ch, DAM_LIGHTNING))
		dam /= 3;
	    damage_old(ch, ch, dam, sn, DAM_LIGHTNING, TRUE);
	    level -= 4;		/* decrement damage */
	    if (ch == NULL)
		return;
	}
	/* now go back and find more targets */
    }
}


void
spell_change_sex(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You've already been changed.\n\r", ch);
	else
	    act("$N has already had $s(?) sex changed.", ch, NULL, victim,
		TO_CHAR);
	return;
    }
    if (saves_spell(level, victim, DAM_OTHER))
	return;
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
    af.location = APPLY_SEX;
    do {
	af.modifier = number_range(0, 2) - victim->sex;
    }
    while (af.modifier == 0);
    af.bitvector = 0;
    affect_to_char(victim, &af);
    send_to_char("You feel different.\n\r", victim);
    act("$n doesn't look like $mself anymore...", victim, NULL, NULL,
	TO_ROOM);
    return;
}



void
spell_charm_person(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_safe(ch, victim))
	return;

    /* Added to prevent stupid bug - Suzuran */
    if (victim->fighting != NULL) {
	send_to_char("They're too busy fighting to be charmed!\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("You like yourself even better!\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_CHARM)
	|| IS_AFFECTED(ch, AFF_CHARM)
	|| level < victim->level || IS_SET(victim->imm_flags, IMM_CHARM)
	|| saves_spell(level, victim, DAM_CHARM))
	return;

    /*
       if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
       {
       send_to_char(
       "The mayor does not allow charming in the city limits.\n\r",ch);
       return;
       }
     */

    if (victim->master)
	stop_follower(victim);
    add_follower(victim, ch);
    victim->leader = ch;
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy(level / 4);
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);
    act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
    if (ch != victim)
	act("$N looks at you with adoring eyes.", ch, NULL, victim,
	    TO_CHAR);
    return;
}



void
spell_chill_touch(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
	0,
	0, 0, 8, 8, 8, 8, 9, 11, 12, 14,
	15, 17, 18, 20, 21, 23, 24, 26, 27, 29,
	30, 32, 33, 35, 36, 38, 39, 41, 42, 44,
	45, 47, 48, 50, 51, 53, 54, 56, 57, 59,
	60, 62, 63, 65, 66, 68, 69, 71, 72, 74,
	75, 77, 78, 80, 81, 83, 84, 86, 87, 89,
	91, 93, 95, 97, 99, 101, 103, 105, 107, 109,
	111, 113, 115, 117, 119, 121, 123, 125, 127, 129,
	131, 133, 135, 137, 139, 141, 143, 145, 147, 149,
	151, 153, 155, 157, 159, 161, 163, 165, 167, 169
    };
    AFFECT_DATA af;
    int dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level], dam_each[level] * 2);
    if (!saves_spell(level, victim, DAM_COLD)) {
	act("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = 6;
	af.location = APPLY_STR;
	af.modifier = -1;
	af.bitvector = 0;
	affect_join(victim, &af);
    } else {
	dam /= 2;
    }

    damage_old(ch, victim, dam, sn, DAM_COLD, TRUE);
    return;
}



void
spell_colour_spray(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	30, 30, 33, 36, 39, 42, 45, 48, 51, 54,
	57, 60, 63, 66, 69, 72, 75, 78, 81, 84,
	87, 90, 93, 96, 99, 102, 105, 108, 111, 114,
	117, 120, 123, 126, 129, 132, 135, 138, 141, 144,
	147, 150, 153, 156, 159, 162, 165, 168, 171, 174,
	177, 181, 184, 188, 191, 195, 198, 202, 205, 209,
	212, 216, 219, 223, 226, 230, 233, 237, 240, 244,
	247, 251, 254, 258, 261, 265, 268, 272, 275, 279,
	282, 286, 289, 293, 296, 300, 303, 307, 310, 314
    };
    int dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level], dam_each[level] * 2);
    if (saves_spell(level, victim, DAM_LIGHT)) {
	dam /= 2;
    } else {
	spell_blindness(skill_lookup("blindness"),
	    level / 2, ch, (void *) victim, TARGET_CHAR);
    }

    damage_old(ch, victim, dam, sn, DAM_LIGHT, TRUE);
    return;
}



void
spell_concentration(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are already concentrating.\n\r", ch);
	else
	    act("$N is already concentrating.", ch, NULL, victim, TO_CHAR);
	return;
    }
    if (is_affected(victim, skill_lookup("anger"))) {
	if (victim == ch)
	    send_to_char("You are too pissed off to concentrate!\n\r", ch);
	else
	    act("$N is too pissed off to concentrate.", ch, NULL, victim,
		TO_CHAR);
	return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level - 2;
    af.duration = level / 2;
    af.location = APPLY_DAMROLL;
    af.modifier = -1 * (level / 9);
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_HITROLL;
    af.modifier = level / 5;
    affect_to_char(victim, &af);

    af.location = APPLY_INT;
    af.modifier = 2;
    affect_to_char(victim, &af);
    send_to_char("You concentrate on your every move.\n\r", victim);
    act("$n burrows $s brow and concentrates.", victim, NULL, NULL,
	TO_ROOM);
    return;
}

void
spell_continual_light(int sn, int level, CHAR_DATA * ch, void *vo,
		      int target)
{
    OBJ_DATA *light;

    if (target_name[0] != '\0') {	/* do a glow on some object */
	light = get_obj_carry(ch, target_name);

	if (light == NULL) {
	    send_to_char("You don't see that here.\n\r", ch);
	    return;
	}

	if (IS_OBJ_STAT(light, ITEM_GLOW)) {
	    act("$p is already glowing.", ch, light, NULL, TO_CHAR);
	    return;
	}

	SET_BIT(light->extra_flags, ITEM_GLOW);
	act("$p glows with a white light.", ch, light, NULL, TO_ALL);
	return;
    }

    light = create_object(get_obj_index(OBJ_VNUM_LIGHT_BALL), 0);
    obj_to_room(light, ch->in_room);
    act("$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM);
    act("You twiddle your thumbs and $p appears.", ch, light, NULL,
	TO_CHAR);
    return;
}



void
spell_control_weather(int sn, int level, CHAR_DATA * ch, void *vo,
		      int target)
{
    if (!str_cmp(target_name, "better"))
	weather_info.change += dice(level / 3, 4);
    else if (!str_cmp(target_name, "worse"))
	weather_info.change -= dice(level / 3, 4);
    else
	send_to_char("Do you want it to get better or worse?\n\r", ch);

    send_to_char("Ok.\n\r", ch);
    return;
}



void
spell_create_food(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *mushroom;

    mushroom = create_object(get_obj_index(OBJ_VNUM_MUSHROOM), 0);
    mushroom->value[0] = level / 2;
    mushroom->value[1] = level;
    obj_to_room(mushroom, ch->in_room);
    act("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
    act("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
    return;
}

void
spell_create_rose(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    act("$n has created a beautiful red rose.", ch, rose, NULL, TO_ROOM);
    send_to_char("You create a beautiful red rose.\n\r", ch);
    obj_to_char(rose, ch);
    return;
}

void
spell_create_spring(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    OBJ_DATA *spring;

    spring = create_object(get_obj_index(OBJ_VNUM_SPRING), 0);
    spring->timer = level;
    obj_to_room(spring, ch->in_room);
    act("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
    act("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
    return;
}



void
spell_create_water(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if (obj->item_type != ITEM_DRINK_CON) {
	send_to_char("It is unable to hold water.\n\r", ch);
	return;
    }

    if (obj->value[2] != LIQ_WATER && obj->value[1] != 0) {
	send_to_char("It contains some other liquid.\n\r", ch);
	return;
    }

    water = UMIN(level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		 obj->value[0] - obj->value[1]);

    if (water > 0) {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if (!is_name("water", obj->name)) {
	    char buf[MAX_STRING_LENGTH];

	    sprintf(buf, "%s water", obj->name);
	    free_string(obj->name);
	    obj->name = str_dup(buf);
	}
	act("$p is filled.", ch, obj, NULL, TO_CHAR);
    }

    return;
}

void spell_fear(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_IMMORTAL(victim)
	|| get_curr_stat(victim, STAT_INT) >= get_curr_stat(ch, STAT_INT)) {
	act("$N's will is too strong.", ch, NULL, victim, TO_CHAR);
	return;
    }

    victim->wimpy = victim->hit;
    do_flee(victim, "");
    act("$N gets a bewildered look on $S face.", ch, NULL, victim,
	TO_NOTVICT);
    act("A sudden fear takes over you!", ch, NULL, victim, TO_VICT);
    act("$N cringes in fear.", ch, NULL, victim, TO_CHAR);
    return;
}

void spell_cry(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are alreading crying madly!\n\r", ch);
	else
	    act("$N is already bawling.", ch, NULL, victim, TO_CHAR);
	return;
    }


    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level - 2;
    af.duration = level / 4;
    af.location = APPLY_INT;
    af.modifier = -2 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_DEX;
    affect_to_char(victim, &af);

    af.location = APPLY_STR;
    af.modifier = -3;
    affect_to_char(victim, &af);
    send_to_char("You begin to cry nonstop!\n\r", victim);
    act("$n breaks down and cries.", victim, NULL, NULL, TO_ROOM);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_cure_blindness(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!is_affected(victim, gsn_blindness)) {
	if (victim == ch)
	    send_to_char("You aren't blind.\n\r", ch);
	else
	    act("$N doesn't appear to be blinded.", ch, NULL, victim,
		TO_CHAR);
	return;
    }

    if (check_dispel(level, victim, gsn_blindness)) {
	send_to_char("Your vision returns!\n\r", victim);
	act("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    } else
	send_to_char("Spell failed.\n\r", ch);
}

void spell_cure_critical(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(3, 10) + level;
    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);
    send_to_char("You feel better!\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

/* RT added to cure plague */
void spell_cure_disease(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!is_affected(victim, gsn_plague)) {
	if (victim == ch)
	    send_to_char("You aren't ill.\n\r", ch);
	else
	    act("$N doesn't appear to be diseased.", ch, NULL, victim,
		TO_CHAR);
	return;
    }

    if (check_dispel(level, victim, gsn_plague)) {
	send_to_char("Your sores vanish.\n\r", victim);
	act("$n looks relieved as $s sores vanish.", victim, NULL, NULL,
	    TO_ROOM);
    } else
	send_to_char("Spell failed.\n\r", ch);
}

void spell_cure_light(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(1, 10) + level / 2.5;
    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);
    send_to_char("You feel better!\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_cure_poison(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!is_affected(victim, gsn_poison)) {
	if (victim == ch)
	    send_to_char("You aren't poisoned.\n\r", ch);
	else
	    act("$N doesn't appear to be poisoned.", ch, NULL, victim,
		TO_CHAR);
	return;
    }

    if (check_dispel(level, victim, gsn_poison)) {
	send_to_char("A warm feeling runs through your body.\n\r", victim);
	act("$n looks much better.", victim, NULL, NULL, TO_ROOM);
    } else
	send_to_char("Spell failed.\n\r", ch);
}

void spell_cure_serious(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(2, 10) + level / 1.5;
    victim->hit = UMIN(victim->hit + heal, victim->max_hit);
    update_pos(victim);
    send_to_char("You feel better!\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_curse(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;
    char buf[MAX_INPUT_LENGTH];

    /* deal with the object case first */
    if (target == TARGET_OBJ) {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
	    act("$p is already filled with evil.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj, ITEM_BLESS)) {
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected, skill_lookup("bless"));
	    if (!saves_dispel
		(level, paf != NULL ? paf->level : obj->level, 0)) {
		if (paf != NULL)
		    affect_remove_obj(obj, paf);
		act("$p glows with a red aura.", ch, obj, NULL, TO_ALL);
		REMOVE_BIT(obj->extra_flags, ITEM_BLESS);
		return;
	    } else {
		act
		    ("The holy aura of $p is too powerful for you to overcome.",
		     ch, obj, NULL, TO_CHAR);
		return;
	    }
	}
	SET_BIT(obj->extra_flags, ITEM_EVIL);
	SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
	SET_BIT(obj->extra_flags, ITEM_NODROP);
	af.where = TO_OBJECT;
	af.type = sn;
	af.level = level;
	af.duration = 2 * level;
	af.location = APPLY_SAVES;
	af.modifier = +1;
	af.bitvector = ITEM_EVIL;
	affect_to_obj(obj, &af);

	act("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
	return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;

    if (victim == ch) {
	send_to_char("You can't curse yourself.\n\r", ch);
	return;
    }

    if (is_safe_spell(ch, victim, TRUE))
	return;

    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
	send_to_char("You can't do that in a safe room.\n\r", ch);
	return;
    }

    if (IS_SET(ch->act, PLR_DEAD) || IS_SET(victim->act, PLR_DEAD)) {
	send_to_char("You can't curse them right now.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_CURSE)) {
	sprintf(buf, "%s is already tainted by a curse.\n\r",
	    IS_NPC(victim) ? victim->short_descr : victim->name);
	send_to_char(buf, ch);	
	return;
    }

    if (saves_spell(level, victim, DAM_NEGATIVE)) {
	act("Darkness lingers over $N briefly, then fades.", ch, victim, NULL, TO_ROOM);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
    af.location = APPLY_HITROLL;
    af.modifier = -1 * (level / 4);
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = level / 6;
    affect_to_char(victim, &af);

    send_to_char("You feel unclean.\n\r", victim);
    if (ch != victim)
	act("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
    return;
}

void spell_hellbender(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int duration = 0;
    int count;

    if (IS_GOOD(ch)) {
	send_to_char("Your soul is not dark enough to evoke Hell's wrath.\n\r", ch);
	return;
    }

    if (victim != ch) {
	act("$n calls forth the fires of Hell upon $N!",
	    ch, NULL, victim, TO_ROOM);
	act("$n has assailed you with the fires of Hell!",
	    ch, NULL, victim, TO_VICT);
	send_to_char("You conjure forth the fires of hell!\n\r", ch);
    }
    dam = dice(level, number_range(6, 7));

    /* Less damage if neutral - Kevin */
    if (IS_NEUTRAL(ch))
	dam *= .8;

    if (saves_spell(level, victim, DAM_NEGATIVE))
	dam /= 2;

    for (count = 1; (count <= (level / 20)); count++) {
	damage_old(ch, victim, dam, sn, DAM_FIRE, TRUE);
    }

    /* Only the evil users inflict maladictions - Kevin */
    if (IS_EVIL(ch)) {
	duration = (level * 2) / 3;
	spell_curse(gsn_curse, duration, ch, (void *) victim, TARGET_CHAR);
	spell_blindness(gsn_blindness, duration, ch, (void *) victim, TARGET_CHAR);
	spell_poison(gsn_poison, duration, ch, (void *) victim, TARGET_CHAR);
	spell_plague(gsn_plague, duration, ch, (void *) victim, TARGET_CHAR);
    }
}

void spell_demi(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    act("$n draws upon the power of time-space to attack $N!",
	ch, NULL, victim, TO_NOTVICT);
    act("As $n gestures at you, waves of gravity assault your body!",
	ch, NULL, victim, TO_VICT);
    act("You evoke the power of time-space to crush $N!",
	ch, NULL, victim, TO_CHAR);

    if (IS_NPC(victim) && !IS_IMMORTAL(ch)) {
	dam = victim->hit / 10;
    } else {
	dam = victim->hit / 4;
    }

    /* Successful save lowers damage */
    if (saves_spell(level + 10, victim, DAM_GRAVITY))
	dam /= 2;

    damage_old(ch, victim, dam, sn, DAM_GRAVITY, TRUE);

    return;
}

void spell_detect_evil(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_DETECT_EVIL)) {
	if (victim == ch)
	    send_to_char("You can already sense evil.\n\r", ch);
	else
	    act("$N can already detect evil.", ch, NULL, victim, TO_CHAR);
	return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_detect_good(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_DETECT_GOOD)) {
	if (victim == ch)
	    send_to_char("You can already sense good.\n\r", ch);
	else
	    act("$N can already detect good.", ch, NULL, victim, TO_CHAR);
	return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_detect_hidden(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_DETECT_HIDDEN)) {
	if (victim == ch)
	    send_to_char("You are already as alert as you can be. \n\r",
			 ch);
	else
	    act("$N can already sense hidden lifeforms.", ch, NULL, victim,
		TO_CHAR);
	return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char(victim, &af);
    send_to_char("Your awareness improves.\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_detect_invis(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_DETECT_INVIS)) {
	if (victim == ch)
	    send_to_char("You can already see invisible.\n\r", ch);
	else
	    act("$N can already see invisible things.", ch, NULL, victim,
		TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_detect_magic(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_DETECT_MAGIC)) {
	if (victim == ch)
	    send_to_char("You can already sense magical auras.\n\r", ch);
	else
	    act("$N can already detect magic.", ch, NULL, victim, TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_detect_poison(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD) {
	if (obj->value[3] != 0)
	    send_to_char("You smell poisonous fumes.\n\r", ch);
	else
	    send_to_char("It looks delicious.\n\r", ch);
    } else {
	send_to_char("It doesn't look poisoned.\n\r", ch);
    }

    return;
}

void spell_dispel_evil(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC(ch) && IS_EVIL(ch))
	victim = ch;

    if (IS_GOOD(victim)) {
	act("The Light protects $N.", ch, NULL, victim, TO_ROOM);
	return;
    }

    dam = dice(level, number_range(6, 8));

    if (victim->alignment <= -800) {
	dam += dam / 3;
    } else if (victim->alignment >= -200) {
	dam -= dam / 3;
    }

    if (saves_spell(level, victim, DAM_HOLY))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_HOLY, TRUE);
    return;
}

void spell_dispel_good(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC(ch) && IS_GOOD(ch))
	victim = ch;

    if (IS_EVIL(victim)) {
	act("$N is protected by $S evil.", ch, NULL, victim, TO_ROOM);
	return;
    }

    dam = dice(level, number_range(6, 8));

    if (victim->alignment >= 800) {
	dam += dam / 3;
    } else if (victim->alignment <= 200) {
	dam -= dam / 3;
    }

    if (saves_spell(level, victim, DAM_NEGATIVE))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
    return;
}

/* modified for enhanced use */

void spell_dispel_magic(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if (saves_spell(level, victim, DAM_OTHER)) {
	send_to_char("You feel a brief tingling sensation.\n\r", victim);
	send_to_char("You failed.\n\r", ch);
	return;
    }

    /* begin running through the spells */

    if (check_dispel(level, victim, skill_lookup("anger"))) {
	found = TRUE;
	act("$n looses their temper.", victim, NULL, NULL, TO_ROOM);
    }
    if (check_dispel(level, victim, skill_lookup("armor")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("bless")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("blindness"))) {
	found = TRUE;
	act("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("calm"))) {
	found = TRUE;
	act("$n no longer looks so peaceful...", victim, NULL, NULL,
	    TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("concentration"))) {
	found = TRUE;
	act("$n looses $s train of thought.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("cry"))) {
	found = TRUE;
	act("$n gains control and stops crying.", victim, NULL, NULL,
	    TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("change sex"))) {
	found = TRUE;
	act("$n looks more like $mself again.", victim, NULL, NULL,
	    TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("chibichibification"))) {
      found = TRUE;
      act("$n looks more like $mself again.", victim, NULL, NULL,
          TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("charm person"))) {
	found = TRUE;
	act("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("chill touch"))) {
	found = TRUE;
	act("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("concentration"))) {
	found = TRUE;
	act("$n lost $p train of thought.", victim, NULL, NULL, TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("curse"))) {
	found = TRUE;
	act("$n gains control and stops crying.", victim, NULL, NULL,
	    TO_ROOM);
    }

    if (check_dispel(level, victim, skill_lookup("curse")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect evil")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect good")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect hidden")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect invis")))
	found = TRUE;

    found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect hidden")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("detect magic")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("faerie fire"))) {
	act("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("fly"))) {
	act("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("frenzy"))) {
	act("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("giant strength"))) {
	act("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("haste"))) {
	act("$n is no longer moving so quickly.", victim, NULL, NULL,
	    TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("infravision")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("invis"))) {
	act("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("light"))) {
	act("The light around $n's body disappears.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("mass invis"))) {
	act("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("pass door")))
	found = TRUE;


    if (check_dispel(level, victim, skill_lookup("protection evil")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("protection good")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("resistance")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("sanctuary"))) {
	act("The white aura around $n's body vanishes.",
	    victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (IS_AFFECTED(victim, AFF_SANCTUARY)
	&& !saves_dispel(level, victim->level, -1)
	&& !is_affected(victim, skill_lookup("sanctuary"))) {
	REMOVE_BIT(victim->affected_by, AFF_SANCTUARY);
	act("The white aura around $n's body vanishes.",
	    victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("shield"))) {
	act("The shield protecting $n vanishes.", victim, NULL, NULL,
	    TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("sleep")))
	found = TRUE;

    if (check_dispel(level, victim, skill_lookup("slow"))) {
	act("$n is no longer moving so slowly.", victim, NULL, NULL,
	    TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("stone skin"))) {
	act("$n's skin regains its normal texture.", victim, NULL, NULL,
	    TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("true sight"))) {
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("weaken"))) {
	act("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level, victim, skill_lookup("berserk"))) {
	act("$n looks more relaxed.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (found)
	send_to_char("Ok.\n\r", ch);
    else
	send_to_char("Spell failed.\n\r", ch);
    return;
}

void spell_earthquake(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    dam = dice(level, 3) + ch->mana / 5;
    send_to_char("The earth trembles beneath your feet!\n\r", ch);
    act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

    for (vch = char_list; vch != NULL; vch = vch_next) {
	vch_next = vch->next;

	if (vch->in_room == NULL)
	    continue;

	if (vch->in_room == ch->in_room) {
	    if (vch != ch && (!is_safe_spell(ch, vch, TRUE)
			      && !IS_SET(vch->act, PLR_DEAD))) {
		if (IS_AFFECTED(vch, AFF_FLYING)) {
		    damage_old(ch, vch, 0, sn, DAM_BASH, TRUE);
		} else {
		    damage_old(ch, vch, dam, sn, DAM_BASH, TRUE);
		}
	    }
	    continue;
	}

	if (vch->in_room->area == ch->in_room->area)
	    send_to_char("The earth trembles and shivers.\n\r", vch);
    }

    return;
}

void spell_enchant_armor(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    if (obj->item_type != ITEM_ARMOR) {
	send_to_char("That isn't an armor.\n\r", ch);
	return;
    }

    if (obj->wear_loc != -1) {
	send_to_char("The item must be carried to be enchanted.\n\r", ch);
	return;
    }

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;			/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
	    if (paf->location == APPLY_AC) {
		ac_bonus = paf->modifier;
		ac_found = TRUE;
		fail += 5 * (ac_bonus * ac_bonus);
	    }

	    else		/* things get a little harder */
		fail += 20;
	}

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
	if (paf->location == APPLY_AC) {
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus);
	}

	else			/* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj, ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj, ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5, fail, 85);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5)) {	/* item destroyed */
	act("$p flares blindingly... and evaporates!", ch, obj, NULL,
	    TO_CHAR);
	act("$p flares blindingly... and evaporates!", ch, obj, NULL,
	    TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 3)) {	/* item disenchanted */
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.", ch, obj, NULL,
	    TO_CHAR);
	act("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next) {
	    paf_next = paf->next;
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if (result <= fail) {	/* failed, no bad result */
	send_to_char("Nothing seemed to happen.\n\r", ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted) {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
	    af_new = new_affect();

	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where = paf->where;
	    af_new->type = UMAX(0, paf->type);
	    af_new->level = paf->level;
	    af_new->duration = paf->duration;
	    af_new->location = paf->location;
	    af_new->modifier = paf->modifier;
	    af_new->bitvector = paf->bitvector;
	}
    }

    if (result <= (90 - level / 5)) {	/* success! */
	act("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
	act("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = -1;
    }

    else {			/* exceptional enchant */

	act("$p glows a brillant gold!", ch, obj, NULL, TO_CHAR);
	act("$p glows a brillant gold!", ch, obj, NULL, TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	SET_BIT(obj->extra_flags, ITEM_GLOW);
	added = -2;
    }

    /* now add the enchantments */

    if (obj->level < LEVEL_HERO)
	obj->level = UMIN(LEVEL_HERO - 1, obj->level + 1);

    if (ac_found) {
	for (paf = obj->affected; paf != NULL; paf = paf->next) {
	    if (paf->location == APPLY_AC) {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level, level);
	    }
	}
    } else {			/* add a new affect */

	paf = new_affect();

	paf->where = TO_OBJECT;
	paf->type = sn;
	paf->level = level;
	paf->duration = -1;
	paf->location = APPLY_AC;
	paf->modifier = added;
	paf->bitvector = 0;
	paf->next = obj->affected;
	obj->affected = paf;
    }

}

void spell_enchant_weapon(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if (obj->item_type != ITEM_WEAPON) {
	send_to_char("That isn't a weapon.\n\r", ch);
	return;
    }

    if (obj->wear_loc != -1) {
	send_to_char("The item must be carried to be enchanted.\n\r", ch);
	return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;			/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
	    if (paf->location == APPLY_HITROLL) {
		hit_bonus = paf->modifier;
		hit_found = TRUE;
		fail += 2 * (hit_bonus * hit_bonus);
	    }

	    else if (paf->location == APPLY_DAMROLL) {
		dam_bonus = paf->modifier;
		dam_found = TRUE;
		fail += 2 * (dam_bonus * dam_bonus);
	    }

	    else		/* things get a little harder */
		fail += 25;
	}

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
	if (paf->location == APPLY_HITROLL) {
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL) {
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	}

	else			/* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level / 2;

    if (IS_OBJ_STAT(obj, ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj, ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5, fail, 95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5)) {	/* item destroyed */
	act("$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR);
	act("$p shivers violently and explodeds!", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) {	/* item disenchanted */
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.", ch, obj, NULL,
	    TO_CHAR);
	act("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next) {
	    paf_next = paf->next;
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if (result <= fail) {	/* failed, no bad result */
	send_to_char("Nothing seemed to happen.\n\r", ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted) {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
	    af_new = new_affect();

	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where = paf->where;
	    af_new->type = UMAX(0, paf->type);
	    af_new->level = paf->level;
	    af_new->duration = paf->duration;
	    af_new->location = paf->location;
	    af_new->modifier = paf->modifier;
	    af_new->bitvector = paf->bitvector;
	}
    }

    if (result <= (100 - level / 5)) {	/* success! */
	act("$p glows blue.", ch, obj, NULL, TO_CHAR);
	act("$p glows blue.", ch, obj, NULL, TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 1;
    }

    else {			/* exceptional enchant */

	act("$p glows a brillant blue!", ch, obj, NULL, TO_CHAR);
	act("$p glows a brillant blue!", ch, obj, NULL, TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	SET_BIT(obj->extra_flags, ITEM_GLOW);
	added = 2;
    }

    /* now add the enchantments */

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO - 1, obj->level + 1);

    if (dam_found) {
	for (paf = obj->affected; paf != NULL; paf = paf->next) {
	    if (paf->location == APPLY_DAMROLL) {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level, level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags, ITEM_HUM);
	    }
	}
    } else {			/* add a new affect */

	paf = new_affect();

	paf->where = TO_OBJECT;
	paf->type = sn;
	paf->level = level;
	paf->duration = -1;
	paf->location = APPLY_DAMROLL;
	paf->modifier = added;
	paf->bitvector = 0;
	paf->next = obj->affected;
	obj->affected = paf;
    }

    if (hit_found) {
	for (paf = obj->affected; paf != NULL; paf = paf->next) {
	    if (paf->location == APPLY_HITROLL) {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level, level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags, ITEM_HUM);
	    }
	}
    } else {			/* add a new affect */

	paf = new_affect();

	paf->type = sn;
	paf->level = level;
	paf->duration = -1;
	paf->location = APPLY_HITROLL;
	paf->modifier = added;
	paf->bitvector = 0;
	paf->next = obj->affected;
	obj->affected = paf;
    }

}

/*
* Drain XP, MANA, HP.
* Caster gains HP.
*/

void spell_energy_drain(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

/*  Alignment hose removed. -- Kevin
    if (victim != ch)
	ch->alignment = UMAX(-1000, ch->alignment - 50);
*/
    dam = dice(level, 6);

    if (saves_spell(level, victim, DAM_NEGATIVE)) {
	dam /= 2;
    }

    /* Mana/move burn, 100% base damage */
 
    if (victim->mana > 110 - level) {
	victim->mana = UMAX(10, victim->mana - dam);
    }
    if (victim->move > 110 - level) {
	victim->move = UMAX(10, victim->move - dam);
    }

    ch->hit += (dam / 2 <= victim->max_hit) ? dam / 2 : victim->max_hit;

    send_to_char("You feel your life slipping away!\n\r", victim);
    send_to_char("Wow....what a rush!\n\r", ch);
    damage_old(ch, victim, dam, sn, DAM_NEGATIVE, TRUE);

    return;
}

void spell_fireball(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
	0, 0, 0, 0, 30, 35, 40, 45, 50, 55,
	60, 65, 70, 75, 80, 85, 90, 95, 100, 105,
	110, 115, 120, 125, 130, 135, 140, 145, 150, 155,
	160, 165, 170, 175, 180, 185, 190, 195, 200, 205,
	210, 215, 220, 225, 230, 235, 240, 245, 250, 255,
	260, 266, 271, 277, 282, 288, 293, 299, 304, 310,
	315, 321, 326, 332, 337, 343, 348, 354, 359, 365,
	370, 376, 381, 387, 392, 398, 403, 409, 414, 420,
	425, 431, 436, 442, 447, 453, 458, 464, 469, 475
    };
    int dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level], dam_each[level] * 2);
    if (saves_spell(level, victim, DAM_FIRE))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_FIRE, TRUE);
    return;
}

void spell_fireproof(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
	act("$p is already protected from burning.", ch, obj, NULL,
	    TO_CHAR);
	return;
    }

    af.where = TO_OBJECT;
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy(level / 4);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = ITEM_BURN_PROOF;

    affect_to_obj(obj, &af);

    act("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
    act("$p is surrounded by a protective aura.", ch, obj, NULL, TO_ROOM);
}

void spell_flamestrike(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level + 10, 8);
    if (saves_spell(level, victim, DAM_FIRE))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_FIRE, TRUE);
    return;
}

void spell_faerie_fire(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) {
	send_to_char("They're already glowing `%pink``.\n\r", ch);
	return;
    }

    /* Added by Vorlin so you lose these if hit by faerie fire */
    /* 8/23/2000 */

    if (IS_AFFECTED(victim, AFF_SNEAK)) {
	affect_strip(victim, gsn_sneak);
    }

    if (IS_AFFECTED(victim, AFF_INVISIBLE)) {
	affect_strip(victim, gsn_invis);
    }

    if (IS_AFFECTED(victim, AFF_HIDE)) {
	affect_strip(victim, gsn_hide);
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a pink outline.\n\r", victim);
    act("$n is surrounded by a pink outline.", victim, NULL, NULL,
	TO_ROOM);
    return;

}

void spell_faerie_fog(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *ich;

    act("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You conjure a cloud of purple smoke.\n\r", ch);

    for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room) {
	if (ich->invis_level > 0)
	    continue;

	if (ich == ch || saves_spell(level, ich, DAM_OTHER))
	    continue;

	affect_strip(ich, gsn_invis);
	affect_strip(ich, gsn_mass_invis);
	affect_strip(ich, gsn_sneak);
	REMOVE_BIT(ich->affected_by, AFF_HIDE);
	REMOVE_BIT(ich->affected_by, AFF_INVISIBLE);
	REMOVE_BIT(ich->affected_by, AFF_SNEAK);
	act("$n is revealed!", ich, NULL, NULL, TO_ROOM);
	send_to_char("You are revealed!\n\r", ich);
    }

    return;
}

void spell_floating_disc(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *disc, *floating;

    floating = get_eq_char(ch, WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT(floating, ITEM_NOREMOVE)) {
	act("You can't remove $p.", ch, floating, NULL, TO_CHAR);
	return;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0] = ch->level * 10;	/* 10 pounds per level capacity */
    disc->value[3] = ch->level * 5;	/* 5 pounds per level max per item */
    disc->timer = ch->level * 2 - number_range(0, level / 2);

    act("$n has created a floating black disc.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You create a floating disc.\n\r", ch);
    obj_to_char(disc, ch);
    wear_obj(ch, disc, TRUE);
    return;
}

void spell_fly(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_FLYING)) {
	if (victim == ch)
	    send_to_char("You are already airborne.\n\r", ch);
	else
	    act("$N doesn't need your help to fly.", ch, NULL, victim,
		TO_CHAR);
	return;
    }
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level + 3;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char(victim, &af);
    send_to_char("Your feet rise off the ground.\n\r", victim);
    act("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
    return;
}

void spell_freefall(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int count;

    act("$n invokes a gravity shift which throws $N into a free fall!",
	ch, NULL, victim, TO_NOTVICT);
    act("$n reverses gravity around you, flinging you up into the air!",
	ch, NULL, victim, TO_VICT);
    act("You reverse gravity around $N, hurling $M into a free fall!",
	ch, NULL, victim, TO_CHAR);

    /* Base damage range 30-180 */
    dam = dice(30, 6);

    for (count = 1; (count <= (level / 5)); count++) {
	if (count == 1)
	    send_to_char("You are falling.", victim);
	else
	    send_to_char(".", victim);

	/* Each loop increases damage by 12.5%*/
	dam += dam / 8;
    }

    if (IS_AFFECTED(victim, AFF_FLYING))
	REMOVE_BIT(victim->affected_by, AFF_FLYING);

    if (saves_spell(level, victim, DAM_GRAVITY))
	dam /= 1.5;

    send_to_char("\n\rYou land on the ground with a heavy *thud*!\n\r", victim);
    damage_old(ch, victim, dam, sn, DAM_GRAVITY, TRUE);
    return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn) || IS_AFFECTED(victim, AFF_BERSERK)
	|| is_affected(victim, skill_lookup("battle rage"))) {
	if (victim == ch)
	    send_to_char("You are already in a frenzy.\n\r", ch);
	else
	    act("$N is already in a frenzy.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (is_affected(victim, skill_lookup("calm"))) {
	if (victim == ch)
	    send_to_char("Why don't you just relax for a while?\n\r", ch);
	else
	    act("$N doesn't look like $e wants to fight anymore.",
		ch, NULL, victim, TO_CHAR);
	return;
    }

    /*     if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
       (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
       (IS_EVIL(ch) && !IS_EVIL(victim))
       ) 
       {
       act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
       return;
       }
     */

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 3;
    af.modifier = level / 6;
    af.bitvector = 0;

    af.location = APPLY_HITROLL;
    affect_to_char(victim, &af);

    af.location = APPLY_DAMROLL;
    affect_to_char(victim, &af);

    af.modifier = 10 * (level / 12);
    af.location = APPLY_AC;
    affect_to_char(victim, &af);

    if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
	send_to_char("You are filled with holy wrath!\n\r", victim);
    }
    if (IS_EVIL(victim)) {
	send_to_char("You are filled with unholy wrath!\n\r", victim);
    }
    act("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}

/* RT ROM-style gate */

void 
spell_gate(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool gate_pet;
    bool gate_horse;

    if ((victim = get_char_world(ch, target_name)) == NULL
	|| victim == ch
	|| IS_AFFECTED(ch, AFF_CURSE)
	|| victim->in_room == NULL
	|| !can_see_room(ch, victim->in_room)
	|| victim->in_room->vnum == ROOM_VNUM_LIMBO	/* No gating into Limbo */
	|| IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	|| victim->level >= level + 10 || (!IS_NPC(victim)
					   && victim->level > LEVEL_HERO)
	/* NOT trust */
	||(IS_NPC(victim) && saves_spell(level + 20, victim, DAM_OTHER))
	/* li'l easier gating to NPCs */
	|| IS_SET(victim->in_room->room_flags, ROOM_CLAN_ONLY)
	|| room_is_private(victim->in_room)
	|| ((ch->fight > 0)
	    && IS_SET(victim->in_room->room_flags, ROOM_SAFE))) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;

    if (ch->horse != NULL && ch->in_room == ch->horse->in_room &&
	!IS_SET(ch->horse->act, ACT_STABLED_HORSE))
	gate_horse = TRUE;
    else
	gate_horse = FALSE;

    if (gate_horse) {
	sprintf(buf, "%s rides through a gate on %s and vanishes.",
		ch->name, ch->horse->short_descr);
	act(buf, ch, NULL, NULL, TO_ROOM);
	send_to_char("You ride through the gate and vanish.\n\r", ch);
	char_from_room(ch);
	char_from_room(ch->horse);
	char_to_room(ch, victim->in_room);
	char_to_room(ch->horse, victim->in_room);
	sprintf(buf, "%s has arrived through a gate riding %s.", ch->name,
		ch->horse->short_descr);
	act(buf, ch, NULL, NULL, TO_ROOM);
	do_look(ch, "auto");
	do_look(ch->horse, "auto");
    } else {
	act("$n steps through a gate and vanishes.", ch, NULL, NULL,
	    TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r", ch);
	char_from_room(ch);
	char_to_room(ch, victim->in_room);

	act("$n has arrived through a gate.", ch, NULL, NULL, TO_ROOM);
	do_look(ch, "auto");
    }

    if (gate_pet) {
	act("$n steps through a gate and vanishes.", ch->pet, NULL, NULL,
	    TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r", ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet, victim->in_room);
	act("$n has arrived through a gate.", ch->pet, NULL, NULL,
	    TO_ROOM);
	do_look(ch->pet, "auto");
    }

    /* Reset violence timer if active */
    if (ch->fight > 0) {
	ch->fight = 270;
    }
}

void spell_giant_strength(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are already as strong as you can get!\n\r", ch);
	else
	    act("$N can't get any stronger.", ch, NULL, victim, TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_STR;
    af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char(victim, &af);
    send_to_char("Your muscles surge with heightened power!\n\r", victim);
    act("$n's muscles surge with heightened power.", victim, NULL, NULL,
	TO_ROOM);
    return;
}

void spell_gravija(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    act("$n crackles with energy, assaulting $N with a maelstrom of dark power!",
	ch, NULL, victim, TO_NOTVICT);
    act("$n's gravity storm engulfs you, crushing and tearing at your body!",
	ch, NULL, victim, TO_VICT);
    act("You crackle with energy, unleashing a storm of gravitational force against $N!",
	ch, NULL, victim, TO_CHAR);

    if (IS_IMMORTAL(ch)) {
	dam = (victim->hit * 3) / 4;
    } else {
	if (!IS_NPC(victim)) {
	    dam = (victim->hit * 3) / 8;
	} else {
	    dam = (victim->hit * 3) / 20;
	}
    }

    /* Successful save lowers damage */
    if (saves_spell(level + 10, victim, DAM_GRAVITY))
        dam /= 2;

    damage_old(ch, victim, dam, sn, DAM_GRAVITY, TRUE);
    return;
}

void spell_harm(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level, 12);

    /* Feline mod here, since RES_HARM does not exist.. - Kevin */
    if (victim->race == race_lookup("Feline"))
	dam /= 1.5;

    if (saves_spell(level, victim, DAM_HARM))
	dam /= 2;

    damage_old(ch, victim, dam, sn, DAM_HARM, TRUE);
    return;
}

/* RT haste spell */

void spell_haste(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn) || IS_AFFECTED(victim, AFF_HASTE)
	|| IS_SET(victim->off_flags, OFF_FAST)) {
	if (victim == ch)
	    send_to_char("You can't move any faster!\n\r", ch);
	else
	    act("$N is already moving as fast as $E can.",
		ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_AFFECTED(victim, AFF_SLOW)) {
	if (!check_dispel(level, victim, skill_lookup("slow"))) {
	    if (victim != ch)
		send_to_char("Spell failed.\n\r", ch);
	    send_to_char("You feel momentarily faster.\n\r", victim);
	    return;
	}
	act("$n is moving less slowly.", victim, NULL, NULL, TO_ROOM);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = APPLY_DEX;
    af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char(victim, &af);
    send_to_char("You feel yourself moving more quickly.\n\r", victim);
    act("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_heal(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    if (ch->race == race_lookup("Esper"))
	victim->hit = UMIN(victim->hit + 225, victim->max_hit);
    else
	victim->hit = UMIN(victim->hit + 150, victim->max_hit);
    update_pos(victim);
    send_to_char("A warm feeling fills your body.\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void spell_heat_metal(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;

    if (!saves_spell(level + 2, victim, DAM_FIRE)
	&& !IS_SET(victim->imm_flags, IMM_FIRE)) {
	for (obj_lose = victim->carrying; obj_lose != NULL;
	     obj_lose = obj_next) {
	    obj_next = obj_lose->next_content;
	    if (number_range(1, 2 * level) > obj_lose->level
		&& !saves_spell(level, victim, DAM_FIRE)
		&& !IS_OBJ_STAT(obj_lose, ITEM_NONMETAL)
		&& !IS_OBJ_STAT(obj_lose, ITEM_BURN_PROOF)) {
		switch (obj_lose->item_type) {
		case ITEM_ARMOR:
		    if (obj_lose->wear_loc != -1) {	/* remove the item */
			if (can_drop_obj(victim, obj_lose)
			    && (obj_lose->weight / 10) <
			    number_range(1,
					 2 * get_curr_stat(victim,
							   STAT_DEX))
			    && remove_obj(victim, obj_lose->wear_loc,
					  TRUE)) {
			    act("$n yelps and throws $p to the ground!",
				victim, obj_lose, NULL, TO_ROOM);
			    act
				("You remove and drop $p before it burns you.",
				 victim, obj_lose, NULL, TO_CHAR);
			    dam += (number_range(1, obj_lose->level) / 3);
			    obj_from_char(obj_lose);
			    obj_to_room(obj_lose, victim->in_room);
			    fail = FALSE;
			} else {	/* stuck on the body! ouch! */

			    act("Your skin is seared by $p!",
				victim, obj_lose, NULL, TO_CHAR);
			    dam += (number_range(1, obj_lose->level));
			    fail = FALSE;
			}

		    } else {	/* drop it if we can */

			if (can_drop_obj(victim, obj_lose)) {
			    act("$n yelps and throws $p to the ground!",
				victim, obj_lose, NULL, TO_ROOM);
			    act("You and drop $p before it burns you.",
				victim, obj_lose, NULL, TO_CHAR);
			    dam += (number_range(1, obj_lose->level) / 6);
			    obj_from_char(obj_lose);
			    obj_to_room(obj_lose, victim->in_room);
			    fail = FALSE;
			} else {	/* cannot drop */

			    act("Your skin is seared by $p!",
				victim, obj_lose, NULL, TO_CHAR);
			    dam += (number_range(1, obj_lose->level) / 2);
			    fail = FALSE;
			}
		    }
		    break;
		case ITEM_WEAPON:
		    if (obj_lose->wear_loc != -1) {	/* try to drop it */
			if (IS_WEAPON_STAT(obj_lose, WEAPON_FLAMING))
			    continue;

			if (can_drop_obj(victim, obj_lose)
			    && remove_obj(victim, obj_lose->wear_loc,
					  TRUE)) {
			    act
				("$n is burned by $p, and throws it to the ground.",
				 victim, obj_lose, NULL, TO_ROOM);
			    send_to_char
				("You throw your red-hot weapon to the ground!\n\r",
				 victim);
			    dam += 1;
			    obj_from_char(obj_lose);
			    obj_to_room(obj_lose, victim->in_room);
			    fail = FALSE;
			} else {	/* YOWCH! */

			    send_to_char
				("Your weapon sears your flesh!\n\r",
				 victim);
			    dam += number_range(1, obj_lose->level);
			    fail = FALSE;
			}
		    } else {	/* drop it if we can */

			if (can_drop_obj(victim, obj_lose)) {
			    act
				("$n throws a burning hot $p to the ground!",
				victim, obj_lose, NULL, TO_ROOM);
			    act("You and drop $p before it burns you.",
				victim, obj_lose, NULL, TO_CHAR);
			    dam += (number_range(1, obj_lose->level) / 6);
			    obj_from_char(obj_lose);
			    obj_to_room(obj_lose, victim->in_room);
			    fail = FALSE;
			} else {	/* cannot drop */

			    act("Your skin is seared by $p!",
				victim, obj_lose, NULL, TO_CHAR);
			    dam += (number_range(1, obj_lose->level) / 2);
			    fail = FALSE;
			}
		    }
		    break;
		}
	    }
	}
    }
    if (fail) {
	send_to_char("Your spell had no effect.\n\r", ch);
	send_to_char("You feel momentarily warmer.\n\r", victim);
    } else {			/* damage! */

	if (saves_spell(level, victim, DAM_FIRE))
	    dam = 2 * dam / 3;
	if (ch->race == race_lookup("cyborg"))
	    dam *= 3;
	damage_old(ch, victim, dam, sn, DAM_FIRE, TRUE);
    }
}

void spell_heavy_blow(int sn, int level, CHAR_DATA * ch, void *vo, int taret)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    act("$n raises $s palm, summoning a bolt of supergravity to slam $N!",
	ch, NULL, victim, TO_NOTVICT);
    act("$n raises $s palm towards you, releasing a bolt of gravity!",
	ch, NULL, victim, TO_VICT);
    act("You summon the power of gravity, releasing a blast to smite $N!",
	ch, NULL, victim, TO_CHAR);

    dam = dice(level, 6);

    if (saves_spell(level, victim, DAM_GRAVITY))
	dam /= 1.5;

    damage_old(ch, victim, dam, sn, DAM_GRAVITY, TRUE);
    return;
}

/* RT really nasty high-level attack spell */
void spell_holy_word(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;

    bless_num = skill_lookup("bless");
    curse_num = skill_lookup("curse");
    frenzy_num = skill_lookup("frenzy");

    act("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
    send_to_char("You utter a word of divine power.\n\r", ch);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
	vch_next = vch->next_in_room;

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch))) {
	    send_to_char("You feel full more powerful.\n\r", vch);
	    spell_frenzy(frenzy_num, level, ch, (void *) vch, TARGET_CHAR);
	    spell_bless(bless_num, level, ch, (void *) vch, TARGET_CHAR);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch))) {
	    if (!is_safe_spell(ch, vch, TRUE)) {
		spell_curse(curse_num, level, ch, (void *) vch,
			    TARGET_CHAR);
		send_to_char("You are struck down!\n\r", vch);
		dam = dice(level, 6);
		damage_old(ch, vch, dam, sn, DAM_ENERGY, TRUE);
	    }
	}

	else if (IS_NEUTRAL(ch)) {
	    if (!is_safe_spell(ch, vch, TRUE)) {
		spell_curse(curse_num, level / 2, ch, (void *) vch,
			    TARGET_CHAR);
		send_to_char("You are struck down!\n\r", vch);
		dam = dice(level, 4);
		damage_old(ch, vch, dam, sn, DAM_ENERGY, TRUE);
	    }
	}
    }

    send_to_char("You feel drained.\n\r", ch);
    ch->move = 0;
}

void
spell_identify(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    sprintf(buf,
	    "Object '%s' is type %s, extra flags %s%s%s.\n\rWeight is %d, value is %d, level is %d.\n\r",
	    obj->name,
	    item_type_name(obj),
	    extra_bit_name(obj->extra_flags),
	    obj->owner == NULL ? "" : " and is owned by ",
	    obj->owner == NULL ? "" : obj->owner,
	    obj->weight / 10, obj->cost, obj->level);
    send_to_char(buf, ch);

    switch (obj->item_type) {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf(buf, "Level %ld spells of:", obj->value[0]);
	send_to_char(buf, ch);

	if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[1]].name, ch);
	    send_to_char("'", ch);
	}

	if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[2]].name, ch);
	    send_to_char("'", ch);
	}

	if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[3]].name, ch);
	    send_to_char("'", ch);
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[4]].name, ch);
	    send_to_char("'", ch);
	}

	send_to_char(".\n\r", ch);
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	sprintf(buf, "Has %ld charges of level %ld",
		obj->value[2], obj->value[0]);
	send_to_char(buf, ch);

	if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL) {
	    send_to_char(" '", ch);
	    send_to_char(skill_table[obj->value[3]].name, ch);
	    send_to_char("'", ch);
	}

	send_to_char(".\n\r", ch);
	break;

    case ITEM_DRINK_CON:
	sprintf(buf, "It holds %s-colored %s.\n\r",
		liq_table[obj->value[2]].liq_color,
		liq_table[obj->value[2]].liq_name);
	send_to_char(buf, ch);
	break;

    case ITEM_SADDLE:
    case ITEM_CONTAINER:
	sprintf(buf, "Capacity: %ld#  Maximum weight: %ld#  flags: %s\n\r",
		obj->value[0], obj->value[3],
		cont_bit_name(obj->value[1]));
	send_to_char(buf, ch);
	if (obj->value[4] != 100) {
	    sprintf(buf, "Weight multiplier: %ld%%\n\r", obj->value[4]);
	    send_to_char(buf, ch);
	}
	break;

    case ITEM_WEAPON:
	send_to_char("Weapon type is ", ch);
	switch (obj->value[0]) {
	case (WEAPON_EXOTIC):
	    send_to_char("exotic.\n\r", ch);
	    break;
	case (WEAPON_SWORD):
	    send_to_char("sword.\n\r", ch);
	    break;
	case (WEAPON_DAGGER):
	    send_to_char("dagger.\n\r", ch);
	    break;
	case (WEAPON_SPEAR):
	    send_to_char("spear/staff.\n\r", ch);
	    break;
	case (WEAPON_MACE):
	    send_to_char("mace/club.\n\r", ch);
	    break;
	case (WEAPON_AXE):
	    send_to_char("axe.\n\r", ch);
	    break;
	case (WEAPON_FLAIL):
	    send_to_char("flail.\n\r", ch);
	    break;
	case (WEAPON_WHIP):
	    send_to_char("whip.\n\r", ch);
	    break;
	case (WEAPON_POLEARM):
	    send_to_char("polearm.\n\r", ch);
	    break;
	default:
	    send_to_char("unknown.\n\r", ch);
	    break;
	}
	if (obj->pIndexData->new_format)
	    sprintf(buf, "Damage is %ldd%ld (average %ld).\n\r",
		    obj->value[1], obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	else
	    sprintf(buf, "Damage is %ld to %ld (average %ld).\n\r",
		    obj->value[1], obj->value[2],
		    (obj->value[1] + obj->value[2]) / 2);
	send_to_char(buf, ch);
	if (obj->value[4]) {	/* weapon flags */
	    sprintf(buf, "Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
	    send_to_char(buf, ch);
	}
	break;

    case ITEM_ARMOR:
	sprintf(buf,
		"Armor class is %ld pierce, %ld bash, %ld slash, and %ld vs. magic.\n\r",
		obj->value[0], obj->value[1], obj->value[2],
		obj->value[3]);
	send_to_char(buf, ch);
	break;
    }

    if (!obj->enchanted)
	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
	    if (paf->location != APPLY_NONE && paf->modifier != 0) {
		sprintf(buf, "Affects %s by %d.\n\r",
			affect_loc_name(paf->location), paf->modifier);
		send_to_char(buf, ch);
		if (paf->bitvector) {
		    switch (paf->where) {
		    case TO_AFFECTS:
			sprintf(buf, "Adds %s affect.\n",
				affect_bit_name(paf->bitvector));
			break;
		    case TO_OBJECT:
			sprintf(buf, "Adds %s object flag.\n",
				extra_bit_name(paf->bitvector));
			break;
		    case TO_IMMUNE:
			sprintf(buf, "Adds immunity to %s.\n",
				imm_bit_name(paf->bitvector));
			break;
		    case TO_RESIST:
			sprintf(buf, "Adds resistance to %s.\n\r",
				imm_bit_name(paf->bitvector));
			break;
		    case TO_VULN:
			sprintf(buf, "Adds vulnerability to %s.\n\r",
				imm_bit_name(paf->bitvector));
			break;
		    default:
			sprintf(buf, "Unknown bit %d: %d\n\r",
				paf->where, paf->bitvector);
			break;
		    }
		    send_to_char(buf, ch);
		}
	    }
	}

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
	if (paf->location != APPLY_NONE) {
	    if (paf->modifier != 0) {
		sprintf(buf, "Affects %s by %d",
			affect_loc_name(paf->location), paf->modifier);
		send_to_char(buf, ch);
	    }

	    if (paf->duration > -1)
		sprintf(buf, ", %d hours.\n\r", paf->duration);
	    else
		sprintf(buf, ".\n\r");
	    send_to_char(buf, ch);
	    if (paf->bitvector) {
		switch (paf->where) {
		case TO_AFFECTS:
		    sprintf(buf, "Adds %s affect.\n",
			    affect_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf, "Adds %s object flag.\n",
			    extra_bit_name(paf->bitvector));
		    break;
		case TO_WEAPON:
		    sprintf(buf, "Adds %s weapon flags.\n",
			    weapon_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    sprintf(buf, "Adds immunity to %s.\n",
			    imm_bit_name(paf->bitvector));
		    break;
		case TO_RESIST:
		    sprintf(buf, "Adds resistance to %s.\n\r",
			    imm_bit_name(paf->bitvector));
		    break;
		case TO_VULN:
		    sprintf(buf, "Adds vulnerability to %s.\n\r",
			    imm_bit_name(paf->bitvector));
		    break;
		default:
		    sprintf(buf, "Unknown bit %d: %d\n\r",
			    paf->where, paf->bitvector);
		    break;
		}
		send_to_char(buf, ch);
	    }
	}
    }

    return;
}



void
spell_infravision(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_INFRARED)) {
	if (victim == ch)
	    send_to_char("You can already see in the dark.\n\r", ch);
	else
	    act("$N already has infravision.\n\r", ch, NULL, victim,
		TO_CHAR);
	return;
    }
    act("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char(victim, &af);
    send_to_char("Your eyes glow red.\n\r", victim);
    return;
}



void spell_invis(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ) {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj, ITEM_INVIS)) {
	    act("$p is already invisible.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	af.where = TO_OBJECT;
	af.type = sn;
	af.level = level;
	af.duration = level + 12;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = ITEM_INVIS;
	affect_to_obj(obj, &af);

	act("$p fades out of sight.", ch, obj, NULL, TO_ALL);
	return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim, AFF_INVISIBLE)) {
	if (victim == ch) {
	    send_to_char("You're already invisible.\n\r", ch);
	    return;
	} else {
	    act("$N is already invisible.", ch, NULL, victim, TO_CHAR);
	    return;
	}
    }

    /* Added this so you can't cast invis on someone/thing if they're faerie */
    /* fired - Vorlin 8/23/2000 */

    if (victim == ch && IS_AFFECTED(victim, AFF_FAERIE_FIRE)) {
	send_to_char
	    ("How're you going to be invisible with this `%pink`` aura?\n\r",
	     ch);
	return;
    }

    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) {
	send_to_char
	    ("They're glowing `%pink`` so how are they going to be invisible?\n\r",
	     ch);
	return;
    }

    act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level + 12;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char(victim, &af);
    send_to_char("You fade out of existence.\n\r", victim);
    return;
}



void
spell_know_alignment(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

    if (ap > 700)
	msg = "$N has a pure and good aura.";
    else if (ap > 350)
	msg = "$N is of excellent moral character.";
    else if (ap > 100)
	msg = "$N is often kind and thoughtful.";
    else if (ap > -100)
	msg = "$N doesn't have a firm moral commitment.";
    else if (ap > -350)
	msg = "$N lies to $S friends.";
    else if (ap > -700)
	msg = "$N is a black-hearted murderer.";
    else
	msg = "$N is the embodiment of pure evil!.";

    act(msg, ch, NULL, victim, TO_CHAR);
    return;
}

void spell_light(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You're already glowing with light.\n\r", ch);
	else
	    act("$N is already glowing with light.", ch, NULL, victim, TO_CHAR);
	return;
    } 

    if (number_percent() <= 50 && IS_AFFECTED(victim, AFF_SNEAK))
	affect_strip(victim, gsn_sneak);

    if (number_percent() <= 50 && IS_AFFECTED(victim, AFF_INVISIBLE))
	affect_strip(victim, gsn_invis);
                
    if (number_percent() <= 50 && IS_AFFECTED(victim, AFF_HIDE))
	affect_strip(victim, gsn_hide);

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    
    af.duration = level / 3;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    send_to_char("Your body glows with a magical radiance.\n\r", victim);
    act("$n's body glows with a soft light.", victim, NULL, NULL, TO_ROOM);
    return;
}

void spell_lightning_bolt(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 40, 44, 48, 52, 56, 60,
	64, 68, 72, 76, 80, 84, 88, 92, 96, 100,
	104, 108, 112, 116, 120, 124, 128, 132, 136, 140,
	144, 148, 152, 156, 160, 164, 168, 172, 176, 180,
	184, 188, 192, 196, 200, 204, 208, 212, 216, 220,
	224, 229, 233, 238, 242, 247, 251, 256, 260, 265,
	269, 274, 278, 283, 287, 292, 296, 301, 305, 310,
	314, 319, 323, 328, 332, 337, 341, 346, 350, 355,
	359, 364, 368, 373, 377, 382, 386, 391, 395, 400
    };
    int dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level], dam_each[level] * 2);
    if (saves_spell(level, victim, DAM_LIGHTNING))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    return;
}



void
spell_locate_person(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *original;

    if ((victim = get_char_world(ch, target_name)) == NULL
	|| victim == ch
	|| victim->in_room == NULL
	|| !can_see_room(ch, victim->in_room)
	|| ch->level + 10 < victim->level || !can_see(ch, victim)) {
	send_to_char("You couldn't locate them.\n\r", ch);
	return;
    }

    original = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, victim->in_room);
    send_to_char(ch->in_room->name, ch);
    if (ch->level > LEVEL_IMMORTAL) {
	sprintf(buf, " `8[`#Room`8: `3%d`8]``", ch->in_room->vnum);
	send_to_char(buf, ch);
    }
    send_to_char("\n\r", ch);
    if (ch->level >= 20) {
	send_to_char("   ", ch);
	send_to_char(ch->in_room->description, ch);
	if (ch->level >= 25) {
	    send_to_char("\n\r", ch);
	    do_exits(ch, "auto");
	    if (ch->level >= 30) {
		show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
	    }
	}
    }
    show_char_to_char(ch->in_room->people, ch);
    char_from_room(ch);
    char_to_room(ch, original);
    if ((IS_AFFECTED(victim, AFF_DETECT_MAGIC))
	&& (ch->level < LEVEL_IMMORTAL))
	act("$n is trying to locate you!", ch, NULL, victim, TO_VICT);
    return;
}

void
spell_locate_object(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

    buffer = new_buf();

    for (obj = object_list; obj != NULL; obj = obj->next) {
	if (!can_see_obj(ch, obj) || !is_name(target_name, obj->name)
	    || IS_OBJ_STAT(obj, ITEM_NOLOCATE)
	    || number_percent() > 2 * level || ch->level < obj->level)
	    continue;
	if (obj->carried_by != NULL)

	    if ((get_trust(obj->carried_by) > LEVEL_IMMORTAL)
		&& (get_trust(ch) < get_trust(obj->carried_by)))
		continue;


	found = TRUE;
	number++;

	for (in_obj = obj; in_obj->in_obj != NULL;
	     in_obj = in_obj->in_obj);

	if (in_obj->carried_by != NULL && can_see(ch, in_obj->carried_by)) {
	    sprintf(buf, "%s is carried by %s\n\r",
		    obj->short_descr, PERS(in_obj->carried_by, ch));
	} else {
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf(buf, "%s is in %s [Room %d]\n\r",
			obj->short_descr, in_obj->in_room->name,
			in_obj->in_room->vnum);
	    else
		sprintf(buf, "%s is in %s\n\r", obj->short_descr,
			in_obj->in_room ==
			NULL ? "somewhere" : in_obj->in_room->name);
	}

	buf[0] = UPPER(buf[0]);
	add_buf(buffer, buf);

	if (number >= max_found)
	    break;
    }

    if (!found)
	send_to_char("Nothing like that in heaven or earth.\n\r", ch);
    else
	page_to_char(buf_string(buffer), ch);

    free_buf(buffer);

    return;
}

/*
void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
CHAR_DATA *victim = (CHAR_DATA *) vo;
static const sh_int dam_each[] = 
{
0,
3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
13, 13, 13, 13, 13,	14, 14, 14, 14, 14
};
int dam;

  level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
  level	= UMAX(0, level);
  dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
  if ( saves_spell( level, victim,DAM_ENERGY) )
  dam /= 2;
  damage_old( ch, victim, dam, sn, DAM_ENERGY ,TRUE);
  return;
} */


void
spell_magic_missile(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
	0,
	3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
	8, 9, 9, 10, 10, 11, 11, 12, 12, 13,
	13, 14, 14, 15, 15, 16, 16, 17, 17, 18,
	18, 19, 19, 20, 20, 21, 21, 22, 22, 23,
	23, 24, 24, 25, 25, 26, 26, 27, 27, 28,
	28, 29, 29, 30, 30, 31, 31, 32, 32, 33,
	33, 34, 35, 35, 36, 37, 37, 38, 39, 39,
	40, 41, 41, 42, 43, 43, 44, 45, 45, 46,
	47, 47, 48, 49, 49, 50, 51, 51, 52, 53,
	53, 54, 55, 55, 56, 57, 57, 58, 59, 59
    };

    int dam, i;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);

    /* 1 Missile for every ten levels of the caster */

    if (level < 10)
	i = 1;
    else
	i = level / 10;

    for (i = i; i > 0; i--) {
	dam = number_range(dam_each[level], dam_each[level] * 2);
	if (saves_spell(level, victim, DAM_ENERGY))
	    dam /= 2;
	damage_old(ch, victim, dam, sn, DAM_ENERGY, TRUE);
    }
    return;
}


void
spell_mass_healing(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;

    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh");

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
	if ((IS_NPC(ch) && IS_NPC(gch)) || (!IS_NPC(ch) && !IS_NPC(gch))) {
	    spell_heal(heal_num, level, ch, (void *) gch, TARGET_CHAR);
	    spell_refresh(refresh_num, level, ch, (void *) gch,
			  TARGET_CHAR);
	}
    }
}


void
spell_mass_invis(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
	if (!is_same_group(gch, ch) || IS_AFFECTED(gch, AFF_INVISIBLE))
	    continue;
	act("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
	send_to_char("You slowly fade out of existence.\n\r", gch);

	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level / 2;
	af.duration = 24;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char(gch, &af);
    }
    send_to_char("Ok.\n\r", ch);

    return;
}



void spell_null(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    send_to_char("That's not a spell!\n\r", ch);
    return;
}



void
spell_pass_door(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_PASS_DOOR)) {
	if (victim == ch)
	    send_to_char("You are already out of phase.\n\r", ch);
	else
	    act("$N is already shifted out of phase.", ch, NULL, victim,
		TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy(level / 4);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char(victim, &af);
    act("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
    send_to_char("You turn translucent.\n\r", victim);
    return;
}

/* RT plague spell, very nasty */

void spell_plague(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level, victim, DAM_DISEASE) ||
	(IS_NPC(victim) && IS_SET(victim->act, ACT_UNDEAD))) {
	if (ch == victim) {
	    send_to_char("You feel momentarily ill, but it passes.\n\r",
			 ch);
	} else {
	    act("$N seems to be unaffected by your plague.",
		ch, NULL, victim, TO_CHAR);
	}
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level * 3 / 4;
    af.duration = level;
    af.location = APPLY_STR;
    af.modifier = -5;
    af.bitvector = AFF_PLAGUE;
    affect_join(victim, &af);

    send_to_char
	("You scream in agony as plague sores erupt from your skin.\n\r",
	 victim);
    act("$n screams in agony as plague sores erupt from $s skin.", victim,
	NULL, NULL, TO_ROOM);
}

void spell_poison(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;


    if (target == TARGET_OBJ) {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD
	    || obj->item_type == ITEM_DRINK_CON) {
	    if (IS_OBJ_STAT(obj, ITEM_BLESS)
		|| IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
		act("Your spell fails to corrupt $p.", ch, obj, NULL,
		    TO_CHAR);
		return;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.", ch, obj, NULL,
		TO_ALL);
	    return;
	}

	if (obj->item_type == ITEM_WEAPON) {
	    if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)
		|| IS_WEAPON_STAT(obj, WEAPON_FROST)
		|| IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
		|| IS_WEAPON_STAT(obj, WEAPON_SHARP)
		|| IS_WEAPON_STAT(obj, WEAPON_VORPAL)
		|| IS_WEAPON_STAT(obj, WEAPON_SHOCKING)
		|| IS_OBJ_STAT(obj, ITEM_BLESS)
		|| IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
		act("You can't seem to envenom $p.", ch, obj, NULL,
		    TO_CHAR);
		return;
	    }

	    if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
		act("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
		return;
	    }

	    af.where = TO_WEAPON;
	    af.type = sn;
	    af.level = level / 2;
	    af.duration = level / 8;
	    af.location = 0;
	    af.modifier = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj, &af);

	    act("$p is coated with deadly venom.", ch, obj, NULL, TO_ALL);
	    return;
	}

	act("You can't poison $p.", ch, obj, NULL, TO_CHAR);
	return;
    }

    victim = (CHAR_DATA *) vo;

/*
    if (victim == ch) {
	send_to_char("You can't poison yourself.\n\r", ch);
	return;
    }
*/

    if (is_safe_spell(ch, victim, TRUE) && victim != ch) {
	return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_SAFE)) {
	send_to_char("You can't do that in a safe room.\n\r", ch);
	return;
    }

    if (IS_SET(ch->act, PLR_DEAD) || IS_SET(victim->act, PLR_DEAD)) {
	send_to_char("You can't poison them right now.\n\r", ch);
	return;
    }

    if (saves_spell(level, victim, DAM_POISON)) {
	act("$n turns slightly green, but it passes.", victim, NULL, NULL,
	    TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",
		     victim);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_STR;
    af.modifier = -2;
    af.bitvector = AFF_POISON;
    affect_join(victim, &af);
    send_to_char("You feel very sick.\n\r", victim);
    act("$n looks very ill.", victim, NULL, NULL, TO_ROOM);
    return;
}



void
spell_protection_evil(int sn, int level, CHAR_DATA * ch, void *vo,
		      int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_PROTECT_EVIL)
	|| IS_AFFECTED(victim, AFF_PROTECT_GOOD)) {
	if (victim == ch)
	    send_to_char("You are already protected.\n\r", ch);
	else
	    act("$N is already protected.", ch, NULL, victim, TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 24;
    af.location = APPLY_SAVING_SPELL;
    af.modifier = -1;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
    send_to_char("You feel holy and pure.\n\r", victim);
    if (ch != victim)
	act("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
    return;
}

void
spell_protection_good(int sn, int level, CHAR_DATA * ch, void *vo,
		      int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_PROTECT_GOOD)
	|| IS_AFFECTED(victim, AFF_PROTECT_EVIL)) {
	if (victim == ch)
	    send_to_char("You are already protected.\n\r", ch);
	else
	    act("$N is already protected.", ch, NULL, victim, TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 24;
    af.location = APPLY_SAVING_SPELL;
    af.modifier = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char(victim, &af);
    send_to_char("You feel aligned with darkness.\n\r", victim);
    if (ch != victim)
	act("$N is protected from good.", ch, NULL, victim, TO_CHAR);
    return;
}


void
spell_psionic_blast(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = {
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 320, 330, 340, 350, 360, 370, 380, 390,
	400, 410, 420, 430, 440, 450, 460, 470, 480, 490,
	495, 500, 505, 510, 515, 520, 525, 530, 535, 540,
	545, 550, 555, 560, 565, 570, 575, 580, 585, 590,
	595, 600, 605, 610, 615, 620, 625, 630, 635, 640,
	645, 650, 655, 660, 665, 670, 675, 680, 685, 690
    };
    AFFECT_DATA af;
    int dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level], dam_each[level] * 2);

    if (!saves_spell(level, victim, DAM_MENTAL)) {
	act("$n turns pale from a terrible migraine.", victim, NULL, NULL,
	    TO_ROOM);

	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level - 2;
	af.duration = 2;
	af.location = APPLY_INT;
	af.modifier = -3;
	af.bitvector = 0;
	affect_join(victim, &af);

    } else {
	dam /= 2;
    }
    damage_old(ch, victim, dam, sn, DAM_MENTAL, TRUE);
    return;


}

void spell_ray_of_frost(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int count;

    act("From $s palm, $n shoots a thin freezing ray at $N!",
	ch, NULL, victim, TO_NOTVICT);
    act("$n fires a freezing ray from $s palm at you!",
	ch, NULL, victim, TO_VICT);
    act("You fire a ray of freezing air and ice at $N!",
	ch, NULL, victim, TO_CHAR);

    dam = dice(level, 4);
    if (saves_spell(level, victim, DAM_COLD))
	dam /= 1.5;

    for (count = number_range(1, level / 30); count > 0; count--)
	damage_old(ch, victim, dam, sn, DAM_COLD, TRUE);

    return;
}

void spell_recharge(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF) {
	send_to_char("That item does not carry charges.\n\r", ch);
	return;
    }


    if (obj->value[1] == 0) {
	send_to_char("That item has already been recharged once.\n\r", ch);
	return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3];	/* harder to do high-level spells */
    chance -=
	(obj->value[1] - obj->value[2]) * (obj->value[1] - obj->value[2]);

    chance = UMAX(level / 2, chance);

    percent = number_percent();

    if (percent < chance / 2) {
	act("$p glows softly.", ch, obj, NULL, TO_CHAR);
	act("$p glows softly.", ch, obj, NULL, TO_ROOM);
	obj->value[2] = UMAX(obj->value[1], obj->value[2]);
	obj->value[1] = 0;
	return;
    }

    else if (percent <= chance) {
	int chargeback, chargemax;

	act("$p glows softly.", ch, obj, NULL, TO_CHAR);
	act("$p glows softly.", ch, obj, NULL, TO_CHAR);

	chargemax = obj->value[1] - obj->value[2];

	if (chargemax > 0)
	    chargeback = UMAX(1, chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return;
    }

    else if (percent <= UMIN(95, 3 * chance / 2)) {
	send_to_char("Nothing seems to happen.\n\r", ch);
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return;
    }

    else {			/* whoops! */

	act("$p glows brightly and explodes!", ch, obj, NULL, TO_CHAR);
	act("$p glows brightly and explodes!", ch, obj, NULL, TO_ROOM);
	extract_obj(obj);
    }
}

void spell_refresh(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN(victim->move + level, victim->max_move);
    if (victim->max_move == victim->move)
	send_to_char("You feel fully refreshed!\n\r", victim);
    else
	send_to_char("You feel less tired.\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void
spell_remove_curse(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ) {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj, ITEM_NODROP)
	    || IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
	    if (!IS_OBJ_STAT(obj, ITEM_NOUNCURSE)
		&& !saves_dispel(level + 2, obj->level, 0)) {
		REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
		act("$p glows blue.", ch, obj, NULL, TO_ALL);
		return;
	    }

	    act("The curse on $p is beyond your power.", ch, obj, NULL,
		TO_CHAR);
	    return;
	} else {
	    send_to_char("This item is not cursed.\n\r", ch);
	    return;
	}
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

    if (check_dispel(level, victim, gsn_curse)) {
	send_to_char("You feel better.\n\r", victim);
	act("$n looks more relaxed.", victim, NULL, NULL, TO_ROOM);
    }

    for (obj = victim->carrying; (obj != NULL && !found);
	 obj = obj->next_content) {
	if ((IS_OBJ_STAT(obj, ITEM_NODROP)
	     || IS_OBJ_STAT(obj, ITEM_NOREMOVE)) && !IS_OBJ_STAT(obj, ITEM_NOUNCURSE)) {	/* attempt to remove curse */
	    if (!saves_dispel(level, obj->level, 0)) {
		found = TRUE;
		REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
		act("Your $p glows blue.", victim, obj, NULL, TO_CHAR);
		act("$n's $p glows blue.", victim, obj, NULL, TO_ROOM);
	    }
	}
    }
}

void spell_resistance(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if (is_affected(victim, sn)) {
        if (victim == ch)  
            send_to_char("You already resist attacks.\n\r", ch);
        else
            act("$N is already resisting attacks.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 6;
    af.modifier = -10 - (level / 6);
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVES;
    af.modifier = -1 - (level / 30);
    affect_to_char(victim, &af);

    send_to_char("You are imbued with protective energy.\n\r", victim);
    if (ch != victim)
        act("$N is shielded by your resistance magic.", ch, NULL, victim, TO_CHAR);
    return;
}

void
spell_sanctuary(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
	if (victim == ch)
	    send_to_char("You are already in sanctuary.\n\r", ch);
	else
	    act("$N is already in sanctuary.", ch, NULL, victim, TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 6;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(victim, &af);
    act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
    send_to_char("You are surrounded by a white aura.\n\r", victim);
    return;
}



void spell_shield(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are already shielded from harm.\n\r", ch);
	else
	    act("$N is already protected by a shield.", ch, NULL, victim,
		TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 8 + level;
    af.location = APPLY_AC;
    af.modifier = -20;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    act("$n is surrounded by a force shield.", victim, NULL, NULL,
	TO_ROOM);
    send_to_char("You are surrounded by a force shield.\n\r", victim);
    return;
}



void
spell_shocking_grasp(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] = {
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 20, 22, 25, 27, 30, 32, 35,
	37, 40, 42, 45, 47, 50, 52, 55, 57, 60,
	62, 65, 67, 70, 72, 75, 77, 80, 82, 85,
	87, 90, 92, 95, 97, 100, 102, 105, 107, 110,
	112, 115, 117, 120, 122, 125, 127, 130, 132, 135,
	138, 141, 144, 147, 150, 153, 156, 159, 162, 165,
	168, 171, 174, 177, 180, 183, 186, 189, 192, 195,
	198, 201, 204, 207, 210, 213, 216, 219, 222, 225,
	228, 231, 234, 237, 240, 243, 246, 249, 252, 255
    };
    int dam;

    level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
    level = UMAX(0, level);
    dam = number_range(dam_each[level], dam_each[level] * 2);
    if (saves_spell(level, victim, DAM_LIGHTNING))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    return;
}



void spell_sleep(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_SLEEP)
	|| (IS_NPC(victim) && IS_SET(victim->act, ACT_UNDEAD))
	|| (level + 2) < victim->level
	|| saves_spell(level - 4, victim, DAM_CHARM)) return;

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = 4 + level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SLEEP;
    affect_join(victim, &af);

    if (IS_AWAKE(victim)) {
	/* Added this, because sleep should remove flying... --Vorlin */

	if (!IS_AFFECTED(victim, AFF_FLYING)) {
	    send_to_char("You feel very sleepy ..... zzzzzz.\n\r", victim);
	    act("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
	} else {
	    send_to_char("You feel very sleepy ... *`&THUD``*\n\r",
			 victim);
	    act
		("$n falls asleep in mid-air, and falls to the ground with a *`&THUD``*",
		 victim, NULL, NULL, TO_ROOM);
	    REMOVE_BIT(victim->affected_by, AFF_FLYING);
	}
    } else {
	if (IS_AFFECTED(victim, AFF_FLYING)) {
	    REMOVE_BIT(victim->affected_by, AFF_FLYING);
	}
    }
    victim->position = POS_SLEEPING;

    return;
}

void spell_slow(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn) || IS_AFFECTED(victim, AFF_SLOW)) {
	if (victim == ch)
	    send_to_char("You can't move any slower!\n\r", ch);
	else
	    act("$N can't get any slower than that.", ch, NULL, victim,
		TO_CHAR);
	return;
    }

    if (saves_spell(level, victim, DAM_OTHER)
	|| IS_SET(victim->imm_flags, IMM_MAGIC)) {
	if (victim != ch)
	    send_to_char("Nothing seemed to happen.\n\r", ch);
	send_to_char("You feel momentarily lethargic.\n\r", victim);
	return;
    }

    if (IS_AFFECTED(victim, AFF_HASTE)) {
	if (!check_dispel(level, victim, skill_lookup("haste"))) {
	    if (victim != ch)
		send_to_char("Spell failed.\n\r", ch);
	    send_to_char("You feel momentarily slower.\n\r", victim);
	    return;
	}

	act("$n is moving less quickly.", victim, NULL, NULL, TO_ROOM);
	return;
    }


    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = APPLY_DEX;
    af.modifier = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char(victim, &af);
    send_to_char("You feel yourself slowing d o w n...\n\r", victim);
    act("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
    return;
}




void
spell_stone_skin(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(ch, sn)) {
	if (victim == ch)
	    send_to_char("Your skin is already as hard as a rock.\n\r",
			 ch);
	else
	    act("$N is already as hard as can be.", ch, NULL, victim,
		TO_CHAR);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = -40;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    act("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
    send_to_char("Your skin turns to stone.\n\r", victim);
    return;
}



void spell_summon(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim;

    if ((victim = get_char_world(ch, target_name)) == NULL
	|| victim == ch
	|| victim->in_room == NULL
	|| IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	|| (IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE))
	|| (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
	|| victim->fighting != NULL
	|| (IS_NPC(victim) && IS_SET(victim->imm_flags, IMM_SUMMON))
	|| (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
	|| (!IS_NPC(victim) && IS_SET(victim->act, PLR_NOSUMMON))
	|| (IS_NPC(victim) && saves_spell(level, victim, DAM_OTHER))
	|| (IS_SET(ch->in_room->room_flags, ROOM_CLAN_ONLY)
	    && (victim->fight > 0))) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    act("$n disappears suddenly.", victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, ch->in_room);
    act("$n arrives suddenly.", victim, NULL, NULL, TO_ROOM);
    act("$n has summoned you!", ch, NULL, victim, TO_VICT);
    do_look(victim, "auto");
    return;
}



void spell_teleport(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if (victim->in_room == NULL
	|| IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	|| IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	|| (victim != ch && IS_SET(victim->imm_flags, IMM_SUMMON))
	|| (victim != ch && (saves_spell(level - 5, victim, DAM_OTHER)))) {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    pRoomIndex = get_random_room(victim);

    if (victim != ch)
	send_to_char("You have been teleported!\n\r", victim);

    act("$n vanishes!", victim, NULL, NULL, TO_ROOM);
    if (victim->horse != NULL && victim->mount == IS_MOUNTED) {
	victim->mount = NOT_MOUNTED;
	victim->horse->leader = NULL;
    }

    char_from_room(victim);
    char_to_room(victim, pRoomIndex);
    act("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
    do_look(victim, "auto");
    return;
}

void spell_true_sight(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if (IS_AFFECTED(ch, skill_lookup("true sight"))) {
	send_to_char("You can already see everyone and thing.\n\r", ch);
	return;
    } else {   
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = level;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_DETECT_EVIL;
	affect_to_char(victim, &af);   
	af.bitvector = AFF_DETECT_GOOD;
	affect_to_char(victim, &af);   
	af.bitvector = AFF_DETECT_MAGIC;
	affect_to_char(victim, &af);
	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char(victim, &af);
	af.bitvector = AFF_DETECT_INVIS;
	affect_to_char(victim, &af);
	send_to_char("Your eyes burn furiously.\n\r", victim);
	if (ch != victim)
	    send_to_char("Ok.\n\r", ch);
    }
    return;
}

void spell_ventriloquate(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument(target_name, speaker);

    sprintf(buf1, "`6%s says '`^%s`6'``\n\r", speaker, target_name);
    sprintf(buf2, "`6Someone makes %s say '`^%s`6'``.\n\r", speaker, target_name);
    sprintf(buf3, "%s> `6%s says '`^%s`6'``\n\r",
	IS_NPC(ch) ? ch->short_descr : ch->name, speaker, target_name);
    buf1[0] = UPPER(buf1[0]);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
	if (!is_name(speaker, vch->name)) {
	    if (!IS_IMMORTAL(vch)) {
		send_to_char(saves_spell(level, vch, DAM_OTHER) ? buf2 : buf1, vch);
	    } else {
		send_to_char(buf3, vch);
	    }
	}
    }

    return;
}



void spell_weaken(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn) || saves_spell(level, victim, DAM_OTHER))
	return;

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = APPLY_STR;
    af.modifier = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char(victim, &af);
    send_to_char("You feel your strength slip away.\n\r", victim);
    act("$n looks tired and weak.", victim, NULL, NULL, TO_ROOM);
    return;
}



/* RT recall spell is back */

void
spell_word_of_recall(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(victim))
	return;

    if ((location = get_room_index(ROOM_VNUM_TEMPLE)) == NULL) {
	send_to_char("You are completely lost.\n\r", victim);
	return;
    }

    if ((victim != ch) && (is_safe(ch, victim))) {
	send_to_char("You can't do that!", ch);
	return;
    }

    if (IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL) ||
	IS_AFFECTED(victim, AFF_CURSE)) {
	send_to_char("Spell failed.\n\r", victim);
	return;
    }

    if (victim->fighting != NULL)
	stop_fighting(victim, TRUE);

    ch->move /= 2;
    act("$n disappears.", victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n appears in the room.", victim, NULL, NULL, TO_ROOM);
    do_look(victim, "auto");
    if (victim->horse != NULL && victim->mount == IS_MOUNTED) {
	char_from_room(victim->horse);
	char_to_room(victim->horse, location);
	do_look(victim->horse, "auto");
    }

}

/*
* NPC spells.
*/
void
spell_acid_breath(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, hp_dam, dice_dam, hpch;

    act("$n spits acid at $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n spits a stream of corrosive acid at you.", ch, NULL, victim,
	TO_VICT);
    act("You spit acid at $N.", ch, NULL, victim, TO_CHAR);

    hpch = UMAX(12, ch->hit);
    hp_dam = number_range(hpch / 11 + 1, hpch / 6);
    dice_dam = dice(level, 16);

    dam = UMIN(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
    if (saves_spell(level, victim, DAM_ACID)) {
	acid_effect(victim, level / 2, dam / 4, TARGET_CHAR);
	damage_old(ch, victim, dam / 2, sn, DAM_ACID, TRUE);
    } else {
	acid_effect(victim, level, dam, TARGET_CHAR);
	damage_old(ch, victim, dam, sn, DAM_ACID, TRUE);
    }
}

void spell_fire_breath(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam, hp_dam, dice_dam;
    int hpch;

    act("$n breathes forth a cone of fire.", ch, NULL, victim, TO_NOTVICT);
    act("$n breathes a cone of hot fire over you!", ch, NULL, victim,
	TO_VICT);
    act("You breath forth a cone of fire.", ch, NULL, NULL, TO_CHAR);

    hpch = UMAX(10, ch->hit);
    hp_dam = number_range(hpch / 9 + 1, hpch / 5);
    dice_dam = dice(level, 20);
    dam = UMIN(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
    fire_effect(victim->in_room, level, dam / 2, TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next) {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch, vch, TRUE)
	    || (IS_NPC(vch) && IS_NPC(ch)
		&& (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	/* Added this in to avoid DEAD flags and nonpk in the room */
	/* Mobs are still hit and PK too... --Vorlin */
	if ((!IS_NPC(ch) && !IS_NPC(vch) && (ch->pcdata->pkset != 1 ||
					     ch->pcdata->pkset !=
					     vch->pcdata->pkset))
	    || IS_SET(vch->act, PLR_DEAD)) {
	    continue;
	}

	if (vch == victim) {	/* full damage */
	    if (saves_spell(level, vch, DAM_FIRE)) {
		fire_effect(vch, level / 2, dam / 4, TARGET_CHAR);
		damage_old(ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
	    } else {
		fire_effect(vch, level, dam, TARGET_CHAR);
		damage_old(ch, vch, dam, sn, DAM_FIRE, TRUE);
	    }
	} else {		/* partial damage */

	    if (saves_spell(level - 2, vch, DAM_FIRE)) {
		fire_effect(vch, level / 4, dam / 8, TARGET_CHAR);
		damage_old(ch, vch, dam / 4, sn, DAM_FIRE, TRUE);
	    } else {
		fire_effect(vch, level / 2, dam / 4, TARGET_CHAR);
		damage_old(ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
	    }
	}
    }
}

void spell_frost_breath(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam, hp_dam, dice_dam, hpch;

    act("$n breathes out a freezing cone of frost!", ch, NULL, victim,
	TO_NOTVICT);
    act("$n breathes a freezing cone of frost over you!", ch, NULL, victim,
	TO_VICT);
    act("You breath out a cone of frost.", ch, NULL, NULL, TO_CHAR);

    hpch = UMAX(12, ch->hit);
    hp_dam = number_range(hpch / 11 + 1, hpch / 6);
    dice_dam = dice(level, 16);

    dam = UMIN(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
    cold_effect(victim->in_room, level, dam / 2, TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next) {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch, vch, TRUE)
	    || (IS_NPC(vch) && IS_NPC(ch)
		&& (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	/* Added this in to avoid DEAD flags and nonpk in the room */
	/* Mobs are still hit and PK too... --Vorlin */
	if ((!IS_NPC(ch) && !IS_NPC(vch) && (ch->pcdata->pkset != 1 ||
					     ch->pcdata->pkset !=
					     vch->pcdata->pkset))
	    || IS_SET(vch->act, PLR_DEAD)) {
	    continue;
	}

	if (vch == victim) {	/* full damage */
	    if (saves_spell(level, vch, DAM_COLD)) {
		cold_effect(vch, level / 2, dam / 4, TARGET_CHAR);
		damage_old(ch, vch, dam / 2, sn, DAM_COLD, TRUE);
	    } else {
		cold_effect(vch, level, dam, TARGET_CHAR);
		damage_old(ch, vch, dam, sn, DAM_COLD, TRUE);
	    }
	} else {
	    if (saves_spell(level - 2, vch, DAM_COLD)) {
		cold_effect(vch, level / 4, dam / 8, TARGET_CHAR);
		damage_old(ch, vch, dam / 4, sn, DAM_COLD, TRUE);
	    } else {
		cold_effect(vch, level / 2, dam / 4, TARGET_CHAR);
		damage_old(ch, vch, dam / 2, sn, DAM_COLD, TRUE);
	    }
	}
    }
}


void spell_gas_breath(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam, hp_dam, dice_dam, hpch;

    act("$n breathes out a cloud of poisonous gas!", ch, NULL, NULL,
	TO_ROOM);
    act("You breath out a cloud of poisonous gas.", ch, NULL, NULL,
	TO_CHAR);

    hpch = UMAX(16, ch->hit);
    hp_dam = number_range(hpch / 15 + 1, 8);
    dice_dam = dice(level, 12);

    dam = UMIN(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
    poison_effect(ch->in_room, level, dam, TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch, vch, TRUE)
	    || (IS_NPC(ch) && IS_NPC(vch)
		&& (ch->fighting == vch || vch->fighting == ch)))
	    continue;

	/* Added this in to avoid DEAD flags and nonpk in the room */
	/* Mobs are still hit and PK too... --Vorlin */
	if ((!IS_NPC(ch) && !IS_NPC(vch) && (ch->pcdata->pkset != 1 ||
					     ch->pcdata->pkset !=
					     vch->pcdata->pkset))
	    || IS_SET(vch->act, PLR_DEAD)) {
	    continue;
	}

	if (saves_spell(level, vch, DAM_POISON)) {
	    poison_effect(vch, level / 2, dam / 4, TARGET_CHAR);
	    damage_old(ch, vch, dam / 2, sn, DAM_POISON, TRUE);
	} else {
	    poison_effect(vch, level, dam, TARGET_CHAR);
	    damage_old(ch, vch, dam, sn, DAM_POISON, TRUE);
	}
    }
}

void
spell_lightning_breath(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, hp_dam, dice_dam, hpch;

    act("$n breathes a bolt of lightning at $N.", ch, NULL, victim,
	TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!", ch, NULL, victim,
	TO_VICT);
    act("You breathe a bolt of lightning at $N.", ch, NULL, victim,
	TO_CHAR);

    hpch = UMAX(10, ch->hit);
    hp_dam = number_range(hpch / 9 + 1, hpch / 5);
    dice_dam = dice(level, 20);

    dam = UMIN(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

    if (saves_spell(level, victim, DAM_LIGHTNING)) {
	shock_effect(victim, level / 2, dam / 4, TARGET_CHAR);
	damage_old(ch, victim, dam / 2, sn, DAM_LIGHTNING, TRUE);
    } else {
	shock_effect(victim, level, dam, TARGET_CHAR);
	damage_old(ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    }
}

/*
* Spells for mega1.are from Glop/Erkenbrand.
*/
void
spell_general_purpose(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range(25, 100);
    if (saves_spell(level, victim, DAM_PIERCE))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_PIERCE, TRUE);
    return;
}

void
spell_high_explosive(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range(30, 120);
    if (saves_spell(level, victim, DAM_PIERCE))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_PIERCE, TRUE);
    return;
}

/*EE960529*/
void spell_imprint(int sn, int level, CHAR_DATA * ch, void *vo)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int sp_slot, i, mana;
    char buf[MAX_STRING_LENGTH];
    bool gravcheck = FALSE;

    if (skill_table[sn].spell_fun == spell_null) {
	send_to_char("That is not a spell.\n\r", ch);
	return;
    }

    for (sp_slot = i = 1; i < 4; i++) {
	if (obj->value[i] != -1) {
	    if (obj->value[i] == skill_lookup("demi")
		|| obj->value[i] == skill_lookup("gravija"))
		gravcheck = TRUE;
	    sp_slot++;
	}
    }

    if (sp_slot > 3) {
	act("`&$p`` cannot contain any more spells.", ch, obj, NULL,
	    TO_CHAR);
	return;
    }

    mana = 4 * 20;

    if (!IS_NPC(ch) && ch->mana < mana) {
	send_to_char("You don't have enough mana.\n\r", ch);
	return;
    }

    if (number_percent() > ch->pcdata->learned[sn]) {
	send_to_char("You lost your concentration.\n\r", ch);
	ch->mana -= mana / 2;
	return;
    }

    if (gravcheck && (sn == skill_lookup("demi") || sn == skill_lookup("gravija"))) {
	send_to_char("The spell is too long to fit on the remaining space on the parchment.\n\r", ch);
	return;
    }

    ch->mana -= mana;
    obj->value[sp_slot] = sn;

    switch (sp_slot) {
    case 1:
	if (number_percent() > 95) {
	    sprintf(buf,
		    "The magical enchantment has failed --- the %s vanishes into thin air.\n\r",
		    item_name(obj->item_type));
	    send_to_char(buf, ch);
	    extract_obj(obj);
	    return;
	}
	break;
    case 2:
	if (number_percent() > 50) {
	    sprintf(buf,
		    "The magical enchantment has failed --- the %s vanishes into thin air.\n\r",
		    item_name(obj->item_type));
	    send_to_char(buf, ch);
	    extract_obj(obj);
	    return;
	}
	break;
    case 3:

	if (number_percent() > 25) {
	    sprintf(buf,
		    "The magical enchantment has failed --- the %s vanishes into thin air.\n\r",
		    item_name(obj->item_type));
	    send_to_char(buf, ch);
	    extract_obj(obj);
	    return;
	}
	break;

    default:
	bug("sp_slot has more than %d spells.", sp_slot);
	return;
    }

    free_string(obj->short_descr);
    sprintf(buf, "a %s of ", item_name(obj->item_type));
    for (i = 1; i <= sp_slot; i++) {
	strcat(buf, skill_table[obj->value[i]].name);
	if (i == sp_slot - 1)
	    strcat(buf, " and ");
	else if (i != sp_slot)
	    strcat(buf, ", ");
	else
	    strcat(buf, "");
    }
    obj->short_descr = str_dup(buf);

    /*sprintf( buf, "%s %s %s %s %s", obj->name, item_name(obj->item_type),
       *       skill_table[obj->value[1]].name, 
       *       ((obj->value[2] != -1) ? skill_table[obj->value[2]].name : ""),
       ((obj->value[3] != -1) ? skill_table[obj->value[3]].name : "") ); */

    sprintf(buf, "%s %s %s", obj->name, item_name(obj->item_type),
	    skill_table[obj->value[1]].name);
    free_string(obj->name);
    obj->name = str_dup(buf);

    sprintf(buf, "You have imbued a new spell to the %s.\n\r",
	    item_name(obj->item_type));
    send_to_char(buf, ch);

    return;
}


void
spell_draguslave(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int count;

    if (IS_NPC(ch) && !strstr(ch->name, "(Slayers)"))
	return;

    if (victim != ch
	&& (!IS_SET(victim->act, PLR_DEAD) && !IS_NPC(victim))) {
	act("$n summons the infamous `8'`@DRAGU SLAVE`8'`7 spell on $N!",
	    ch, NULL, victim, TO_ROOM);
	act("$n calls for the power of `8'`@DRAGU SLAVE`8'`7 on you!", ch,
	    NULL, victim, TO_VICT);
	send_to_char("You utter the words `8'`@DRAGU SLAVE`8'`7!\n\r", ch);
    }
    dam = dice(level, 8);
    if (saves_spell(level, victim, DAM_NEGATIVE))
	dam /= 2;

    for (count = 1; (count <= (level / 20)); count++)
	damage_old(ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
}

void spell_ratilt(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int count;
    /* Alternate saving throw and initial damage - Kevin */
    int dt = (number_percent() <= 50 ? DAM_LIGHT : DAM_HOLY);

    if (IS_NPC(ch) && !strstr(ch->name, "(Slayers)"))
	return;

    if (victim != ch) {
	act("$n summons the spirit energy of `8'`&RA`7-`&T`7i`&LT`8'`7 on $N!",
	    ch, NULL, victim, TO_ROOM);
	act("$n summons the spirit of `8'`&RA`7-`&T`7i`&LT`8'`7 on you!",
	    ch, NULL, victim, TO_VICT);
	send_to_char("You utter the words `8'`&RA`7-`&T`7i`&LT`8'`7!\n\r", ch);
    }
    dam = dice(level, 8);
    if (saves_spell(level, victim, dt))
	dam /= 2;

    for (count = 1; (count <= (level / 20)); count++) {
	damage_old(ch, victim, dam, sn, dt, TRUE);
	dt = (number_percent() <= 50 ? DAM_LIGHT : DAM_HOLY);
    }

}

void
spell_laguna_blade(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    OBJ_DATA *blade;

    blade = create_object(get_obj_index(OBJ_VNUM_LAGUNA_BLADE), 0);
    blade->level = ch->level + 20;
    blade->timer = number_range(50, 100);
    obj_to_char(blade, ch);
    act("$n summons all $m energy and has created `8'`^$p`8'`7.", ch,
	blade, NULL, TO_ROOM);
    act("You use all existing energy and create `8'`^$p`8'`7.", ch,
	blade, NULL, TO_CHAR);
    return;
}

	/* 
	   Added this for the almighty mana potions!
	   --Vorlin
	 */

void spell_mana(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (!IS_IMMORTAL(ch) || IS_NPC(ch)) {
	send_to_char("Huh?\n\r", ch);
	return;
    }

    if (ch->race == race_lookup("Esper")) {
	victim->mana = UMIN(victim->mana + 150, victim->max_mana);
    } else {
	victim->mana = UMIN(victim->mana + 100, victim->max_mana);
    }
    update_pos(victim);
    send_to_char("You feel mentally revived.\n\r", victim);
    return;
}

void spell_chibichibi(int sn, int level, CHAR_DATA * ch, void *vo, int target){

  /* It's ba-ack! ^_^ */

  /* Variables */
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  AFFECT_DATA af;
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if(IS_NPC(victim)){return;}  /* Not on NPCs */

  if(is_affected(victim,sn)){
    if(victim == ch){
      send_to_char("You're already as pink as you can get!\n\r",ch);
    }else{
      act("$N is already as pink as $s can get.",ch,NULL,victim,TO_CHAR);
    }
    return;
  }

  /* At this point, we are going through with this. */
  /* First, change the target's sex... */

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_SEX;
  af.modifier = SEX_FEMALE - victim->sex;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  /* Now, the hard parts... */

  /* Fix up eq.  Stolen from fukubuster code */

  obj=get_eq_char(victim,WEAR_FEET);
  if(obj != NULL){unequip_char(victim,obj);}
  obj=get_eq_char(victim,WEAR_LEGS);
  if(obj != NULL){unequip_char(victim,obj);}
  obj=get_eq_char(victim,WEAR_BODY);
  if(obj != NULL){unequip_char(victim,obj);}
  obj=get_eq_char(victim,WEAR_HANDS);
  if(obj != NULL){unequip_char(victim,obj);}
  obj=get_eq_char(victim,WEAR_NECK_1);
  if(obj != NULL){unequip_char(victim,obj);}

  obj=create_object(get_obj_index(OBJ_VNUM_SCHOOL_VEST),0);
  obj->name=str_dup("-chibichibified- sailor top");
  obj->short_descr=str_dup("a white sailor-fuku top with a red bow");
  obj->description=str_dup("A white sailor-fuku top lies here.");
  obj->cost=0;
  obj->extra_flags |= (ITEM_NOREMOVE|ITEM_NOUNCURSE|ITEM_BURN_PROOF|ITEM_NODROP|ITEM_NOPURGE|ITEM_NOAUCTION);
  obj_to_char(obj,victim);
  equip_char(victim,obj,WEAR_BODY);

  obj=create_object(get_obj_index(3708),0);
  obj->name=str_dup("-chibichibified- sailor boots");
  obj->short_descr=str_dup("a pair of pink knee-high boots");
  obj->description=str_dup("A pair of pink knee-high boots lie here.");
  obj->cost=0;
  obj->extra_flags |= (ITEM_NOREMOVE|ITEM_NOUNCURSE|ITEM_BURN_PROOF|ITEM_NODROP|ITEM_NOPURGE|ITEM_NOAUCTION);
  obj_to_char(obj,victim);
  equip_char(victim,obj,WEAR_FEET);

  obj=create_object(get_obj_index(3707),0);
  obj->name=str_dup("-chibichibified- sailor skirt");
  obj->short_descr=str_dup("a short pink pleated skirt");
  obj->description=str_dup("A short pink pleated skirt lies here.");
  obj->cost=0;
  obj->extra_flags |= (ITEM_NOREMOVE|ITEM_NOUNCURSE|ITEM_BURN_PROOF|ITEM_NODROP|ITEM_NOPURGE|ITEM_NOAUCTION);
  obj_to_char(obj,victim);
  equip_char(victim,obj,WEAR_LEGS);

  obj=create_object(get_obj_index(3709),0);
  obj->name=str_dup("-chibichibified- sailor gloves");
  obj->short_descr=str_dup("a pair of long white gloves");
  obj->description=str_dup("A pair of long white gloves lie here.");
  obj->cost=0;
  obj->extra_flags  |= (ITEM_NOREMOVE|ITEM_NOUNCURSE|ITEM_BURN_PROOF|ITEM_NODROP|ITEM_NOPURGE|ITEM_NOAUCTION);
  obj_to_char(obj,victim);
  equip_char(victim,obj,WEAR_HANDS);

  obj=create_object(get_obj_index(3705),0);
  obj->name=str_dup("-chibichibified- sailor choker");
  obj->short_descr=str_dup("a pink choker with a gold crescent moon on it");
  obj->description=str_dup("A small pink choker lies here.");
  obj->cost=0;
  obj->extra_flags |= (ITEM_NOREMOVE|ITEM_NOUNCURSE|ITEM_BURN_PROOF|ITEM_NODROP|ITEM_NOPURGE|ITEM_NOAUCTION);
  obj_to_char(obj,victim);
  equip_char(victim,obj,WEAR_NECK_1);

  /* Save the old desc.  */
  free_string(victim->savedesc);
  sprintf(buf2,"%s",victim->description);
  victim->savedesc = str_dup(buf2);

  /* Can overwrite with a new one... */
  free_string(victim->description);
  sprintf(buf,"You see a short blue-eyed girl with pink hair in the style of\n\rChibi-Chibi...  But something tells you it's really %s!  How could that be?  She's too cute for that!\n\r",capitalize(victim->name));
  victim->description = str_dup(buf);

  /* Could fix title if I wanted... ^_^ */
  /* Let the poor loser know. */

  act("$N is suddenly enveloped in a puff of pink smoke!",victim,NULL,victim,TO_NOTVICT);
  act("You are suddenly enveloped in a puff of pink smoke... You feel very different!",victim,NULL,victim,TO_CHAR);

  return;
}


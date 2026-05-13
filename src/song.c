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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "song.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look);

/*
 * Local functions.
 */
void say_song args((CHAR_DATA * ch, int sn));
int find_song args((CHAR_DATA * ch, const char *name));

/* imported functions */
bool remove_obj args((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj args((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));
bool check_dispel args((int dis_level, CHAR_DATA * victim, int sn));
bool saves_dispel args((int dis_level, int spell_level, int duration));



int find_song(CHAR_DATA * ch, const char *name)
{
    /* finds a song the character can sing if possible */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for (sn = 0; sn < MAX_SKILL; sn++) {
	if (skill_table[sn].name == NULL)
	    break;
	if (skill_table[sn].song_fun == song_null)
	    break;

	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	    && !str_prefix(name, skill_table[sn].name)) {
/*	    if ( found == -1) */
	    found = sn;
	    if (ch->level >= skill_table[sn].skill_level[ch->class]
		&& ch->pcdata->learned[sn] > 0)
		return sn;
	}
    }
    return found;
}

/*
 * Utter mystical words for an sn.
 */
void say_song(CHAR_DATA * ch, int sn)
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

    sprintf(buf2, "$n utters the words, '`@%s``'.", buf);
    sprintf(buf, "$n utters the words, '`@%s``'.", skill_table[sn].name);

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
	if (rch != ch)
	    act(ch->class == rch->class ? buf : buf2, ch, NULL, rch,
		TO_VICT);
    }

    return;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
extern char *target_name;

void do_sing(CHAR_DATA * ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target;

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if (IS_NPC(ch) && ch->desc == NULL)
	return;

    target_name = one_argument(argument, arg1);
    one_argument(target_name, arg2);

    if (arg1[0] == '\0') {
	send_to_char("Sing to who?\n\r", ch);
	return;
    }

    if ((sn = find_song(ch, arg1)) < 0 || (!IS_NPC(ch)
					   && (ch->level <
					       skill_table[sn].skill_level
					       [ch->class]
					       || ch->pcdata->
					       learned[sn] == 0))) {
	send_to_char("You don't know any songs of that name.\n\r", ch);
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
		send_to_char("Sing song at whom?\n\r", ch);
		return;
	    }
	} else {
	    if ((victim = get_char_room(ch, target_name)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	    }
	}
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/


	if (!IS_NPC(ch)) {

	    if (is_safe(ch, victim) && victim != ch) {
		send_to_char("Not on that target.\n\r", ch);
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
	if (arg2[0] != '\0' && !is_name(target_name, ch->name)) {
	    send_to_char("You can't do that.\n\r", ch);
	    return;
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
		send_to_char("Sing song at whom or what?\n\r", ch);
		return;
	    }

	    target = TARGET_CHAR;
	} else if ((victim = get_char_room(ch, target_name)) != NULL) {
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) {	/* check the sanity of the attack */
	    if (is_safe_song(ch, victim, FALSE) && victim != ch) {
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

    WAIT_STATE(ch, skill_table[sn].beats);

    if (number_percent() > get_skill(ch, sn)) {
	send_to_char("You lost your concentration.\n\r", ch);
	check_improve(ch, sn, FALSE, 1);
	ch->mana -= mana / 2;
    } else {
	ch->mana -= mana;
	(*skill_table[sn].song_fun) (sn, ch->level, ch, vo, target);
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

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_song(int level, CHAR_DATA * victim, int dam_type)
{
    int save;

    save = 50 + (victim->level - level) * 5 - victim->saving_throw * 2;
    if (IS_AFFECTED(victim, AFF_BERSERK))
	save += victim->level / 2;

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

 /*
    *  Song functions.
  */

void
song_cause_anger(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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

void
song_cause_blindness(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim, AFF_BLIND)
	|| saves_song(level, victim, DAM_OTHER))
	return;


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
song_cause_calm(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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
song_cause_cancellation(int sn, int level, CHAR_DATA * ch, void *vo,
			int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim) &&
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)) ||
	(IS_NPC(ch) && !IS_NPC(victim))) {
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

    if (found)
	send_to_char("Ok.\n\r", ch);
    else
	send_to_char("Song failed.\n\r", ch);
}

void
song_cause_concentration(int sn, int level, CHAR_DATA * ch, void *vo,
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
song_weather_control(int sn, int level, CHAR_DATA * ch, void *vo,
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
song_cause_cry(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are alreading crying madly!\n\r", ch);
	else
	    act("$N is already balling.", ch, NULL, victim, TO_CHAR);
	return;
    }


    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level - 2;
    af.duration = level / 4;
    af.location = APPLY_DEX;
    af.modifier = -2 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = 0;
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

void
song_evil_detection(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
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


void
song_good_detection(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
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



void
song_detection_hide(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
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



void
song_detection_invis(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
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



void
song_detection_magic(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
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

void
song_detection_poison(int sn, int level, CHAR_DATA * ch, void *vo,
		      int target)
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

void
song_evil_dispel(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC(ch) && IS_EVIL(ch))
	victim = ch;

    if (IS_GOOD(victim)) {
	act("The Light protects $N.", ch, NULL, victim, TO_ROOM);
	return;
    }

    if (IS_NEUTRAL(victim)) {
	act("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (victim->hit > (ch->level * 4))
	dam = dice(level, 4);
    else
	dam = UMAX(victim->hit, dice(level, 4));
    if (saves_spell(level, victim, DAM_HOLY))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_HOLY, TRUE);
    return;
}


void
song_good_dispel(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (!IS_NPC(ch) && IS_GOOD(ch))
	victim = ch;

    if (IS_EVIL(victim)) {
	act("$N is protected by $S evil.", ch, NULL, victim, TO_ROOM);
	return;
    }

    if (IS_NEUTRAL(victim)) {
	act("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (victim->hit > (ch->level * 4))
	dam = dice(level, 4);
    else
	dam = UMAX(victim->hit, dice(level, 4));
    if (saves_spell(level, victim, DAM_NEGATIVE))
	dam /= 2;
    damage_old(ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
    return;
}


/* modified for enhanced use */

void
song_magic_dispel(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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

    if (check_dispel(level, victim, skill_lookup("weaken"))) {
	act("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
	found = TRUE;
    }

    if (found)
	send_to_char("Ok.\n\r", ch);
    else
	send_to_char("Song failed.\n\r", ch);
    return;
}

void
song_cause_faerie_fog(int sn, int level, CHAR_DATA * ch, void *vo,
		      int target)
{
    CHAR_DATA *ich;

    act("$n sings up a cloud of green smoke.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You sing up a cloud of green smoke.\n\r", ch);

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

void
song_cause_frenzy(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn) || IS_AFFECTED(victim, AFF_BERSERK)) {
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

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
	(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
	(IS_EVIL(ch) && !IS_EVIL(victim))) {
	act("Your god doesn't seem to like $N", ch, NULL, victim, TO_CHAR);
	return;
    }

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

    send_to_char("You are filled with holy wrath!\n\r", victim);
    act("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}

void
song_cause_giant_strength(int sn, int level, CHAR_DATA * ch, void *vo,
			  int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn)) {
	if (victim == ch)
	    send_to_char("You are already as strong as you can get!\n\r",
			 ch);
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

void
song_cause_haste(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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
		send_to_char("Song failed.\n\r", ch);
	    send_to_char("You feel momentarily faster.\n\r", victim);
	    return;
	}
	act("$n is moving less slowly.", victim, NULL, NULL, TO_ROOM);
	return;
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    if (victim == ch)
	af.duration = level / 2;
    else
	af.duration = level / 4;
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

void
song_cause_heal(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN(victim->hit + 100, victim->max_hit);
    update_pos(victim);
    send_to_char("A warm feeling fills your body.\n\r", victim);
    if (ch != victim)
	send_to_char("Ok.\n\r", ch);
    return;
}

void
song_cause_identify(int sn, int level, CHAR_DATA * ch, void *vo,
		    int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    sprintf(buf,
	    "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",
	    obj->name,
	    item_type_name(obj),
	    extra_bit_name(obj->extra_flags),
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
	if (paf->location != APPLY_NONE && paf->modifier != 0) {
	    sprintf(buf, "Affects %s by %d",
		    affect_loc_name(paf->location), paf->modifier);
	    send_to_char(buf, ch);
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
song_alignment_know(int sn, int level, CHAR_DATA * ch, void *vo,
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

void
song_find_object(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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

	found = TRUE;
	number++;

	for (in_obj = obj; in_obj->in_obj != NULL;
	     in_obj = in_obj->in_obj);

	if (in_obj->carried_by != NULL && can_see(ch, in_obj->carried_by)) {
	    sprintf(buf, "one is carried by %s\n\r",
		    PERS(in_obj->carried_by, ch));
	} else {
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf(buf, "one is in %s [Room %d]\n\r",
			in_obj->in_room->name, in_obj->in_room->vnum);
	    else
		sprintf(buf, "one is in %s\n\r",
			in_obj->in_room == NULL
			? "somewhere" : in_obj->in_room->name);
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

void
song_heal_mass(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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
song_cause_mass_invis(int sn, int level, CHAR_DATA * ch, void *vo,
		      int target)
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

void song_null(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    send_to_char("That's not a song!\n\r", ch);
    return;
}

void
song_cause_refresh(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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
song_curse_remove(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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

void
song_cause_sleep(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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
	send_to_char("You feel very sleepy ..... zzzzzz.\n\r", victim);
	act("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
	victim->position = POS_SLEEPING;
    }
    return;
}

void
song_cause_slow(int sn, int level, CHAR_DATA * ch, void *vo, int target)
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
		send_to_char("Song failed.\n\r", ch);
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
song_cause_ventriloquate(int sn, int level, CHAR_DATA * ch, void *vo,
			 int target)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument(target_name, speaker);

    sprintf(buf1, "%s says '%s'.\n\r", speaker, target_name);
    sprintf(buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name);
    buf1[0] = UPPER(buf1[0]);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
	if (!is_name(speaker, vch->name))
	    send_to_char(saves_spell(level, vch, DAM_OTHER) ? buf2 : buf1,
			 vch);
    }

    return;
}



void
song_cause_weakeness(int sn, int level, CHAR_DATA * ch, void *vo,
		     int target)
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

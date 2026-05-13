/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

/**************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor                        *
*	ROM has been brought to you by the ROM consortium		  *
*	    Russ Taylor (rtaylor@pacinfo.com)	      			  *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)		          *
*	    Brian Moore (rom@rom.efn.org)				  *
*	By using this code, you have agreed to follow the terms of the	  *
*	ROM license, in the file Rom24/doc/rom.license			  *
***************************************************************************/

/* This was a mess, so I retabbed it K&R style - Suzuran */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"

#if !defined(macintosh)
extern int _filbuf args((FILE *));
#endif


int rename(const char *oldfname, const char *newfname);

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32; count++)
	if (IS_SET(flag, 1 << count)) {
	    if (count < 26)
		buf[pos] = 'A' + count;
	    else
		buf[pos] = 'a' + (count - 26);
	    pos++;
	}

    if (pos == 0) {
	buf[pos] = '0';
	pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
* Array of containers read for proper re-nesting of objects.
*/
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];



/*
* Local functions.
*/
void fwrite_char args((CHAR_DATA * ch, FILE * fp));
void fwrite_obj
args((CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest));
void fwrite_quest args((CHAR_DATA * quest, FILE * fp));
void fwrite_pet args((CHAR_DATA * pet, FILE * fp));
void fread_char args((CHAR_DATA * ch, FILE * fp));
void fread_quest args((CHAR_DATA * ch, FILE * fp));
void fread_pet args((CHAR_DATA * ch, FILE * fp));
void fread_obj args((CHAR_DATA * ch, FILE * fp));

void save_char_crash_obj(CHAR_DATA * ch){
  char strsave[MAX_INPUT_LENGTH];
  FILE *fp;

  if (IS_NPC(ch))
    return;
  if (ch->level == 1)
    return;

  /* Don't save an invalid character.  Nasty shit will happen. - Suzuran */
  if (!IS_VALID(ch)) {
    bug("save_char_obj: Saving invalid chars is a no-no.\n", 0);
    return;
  }

  if (ch->desc != NULL && ch->desc->original != NULL)
    ch = ch->desc->original;

#if defined(unix)
  /* create god log */
  if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL) {
    fclose(fpReserve);
    sprintf(strsave, "%s%s", GOD_DIR, capitalize(ch->name));
    if ((fp = fopen(strsave, "w")) == NULL) {
      bug("Save_char_obj: fopen", 0);
      perror(strsave);
    }

    fprintf(fp, "Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, ch->pcdata->title);
    fclose(fp);
    fpReserve = fopen(NULL_FILE, "r");
  }
#endif

  fclose(fpReserve);
  sprintf(strsave, "%s%s", PLAYER_CRASH_DIR, capitalize(ch->name));
  if ((fp = fopen(TEMP_FILE, "w")) == NULL) {
    bug("Save_char_obj: fopen", 0);
    perror(strsave);
  } else {
    fwrite_char(ch, fp);
    if (ch->carrying != NULL)
      fwrite_obj(ch, ch->carrying, fp, 0);
    /* save the pets */
    if (ch->quest != NULL)
      fwrite_quest(ch->quest, fp);
    if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
      fwrite_pet(ch->pet, fp);
    fprintf(fp, "#END\n");
  }
  fclose(fp);
  rename(TEMP_FILE, strsave);
  fpReserve = fopen(NULL_FILE, "r");
  return;

}

void save_char_died_obj(CHAR_DATA * ch)
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if (IS_NPC(ch))
	return;

    if (ch->desc != NULL && ch->desc->original != NULL)
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL) {
	fclose(fpReserve);
	sprintf(strsave, "%s%s", GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave, "w")) == NULL) {
	    bug("Save_char_obj: fopen", 0);
	    perror(strsave);
	}

	fprintf(fp, "Lev %2d Trust %2d  %s%s\n",
		ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
    }
#endif

    fclose(fpReserve);
    sprintf(strsave, "%s%s", PLAYER_DIED_DIR, capitalize(ch->name));
    if ((fp = fopen(TEMP_FILE, "w")) == NULL) {
	bug("Save_char_obj: fopen", 0);
	perror(strsave);
    } else {
	fwrite_char(ch, fp);
	if (ch->carrying != NULL)
	    fwrite_obj(ch, ch->carrying, fp, 0);
	/* save the pets */
	if (ch->quest != NULL)
	    fwrite_quest(ch->quest, fp);
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet, fp);
	fprintf(fp, "#END\n");
    }
    fclose(fp);
    rename(TEMP_FILE, strsave);
    fpReserve = fopen(NULL_FILE, "r");
    return;
}

void save_char_fried_obj(CHAR_DATA * ch)
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if (IS_NPC(ch))
	return;

    if (ch->desc != NULL && ch->desc->original != NULL)
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL) {
	fclose(fpReserve);
	sprintf(strsave, "%s%s", GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave, "w")) == NULL) {
	    bug("Save_char_obj: fopen", 0);
	    perror(strsave);
	}

	fprintf(fp, "Lev %2d Trust %2d  %s%s\n",
		ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
    }
#endif

    fclose(fpReserve);
    sprintf(strsave, "%s%s", PLAYER_FRIED_DIR, capitalize(ch->name));
    if ((fp = fopen(TEMP_FILE, "w")) == NULL) {
	bug("Save_char_fried_obj: fopen", 0);
	perror(strsave);
    } else {
	fwrite_char(ch, fp);
	if (ch->carrying != NULL)
	    fwrite_obj(ch, ch->carrying, fp, 0);
	/* save the pets */
	if (ch->quest != NULL)
	    fwrite_quest(ch->quest, fp);
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet, fp);
	fprintf(fp, "#END\n");
    }
    fclose(fp);
    rename(TEMP_FILE, strsave);
    fpReserve = fopen(NULL_FILE, "r");
    return;
}

/*
* Save a character and inventory.
* Would be cool to save NPC's too for quest purposes,
*   some of the infrastructure is provided.
*/
void save_char_obj(CHAR_DATA * ch)
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if (IS_NPC(ch))
	return;
    if (ch->level == 1)
	return;

    /* Don't save an invalid character.  Nasty shit will happen. - Suzuran */
    if (!IS_VALID(ch)) {
	bug("save_char_obj: Saving invalid chars is a no-no.\n", 0);
	return;
    }

    if (ch->desc != NULL && ch->desc->original != NULL)
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL) {
	fclose(fpReserve);
	sprintf(strsave, "%s%s", GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave, "w")) == NULL) {
	    bug("Save_char_obj: fopen", 0);
	    perror(strsave);
	}

	fprintf(fp, "Lev %2d Trust %2d  %s%s\n",
		ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
    }
#endif

    fclose(fpReserve);
    sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(ch->name));
    if ((fp = fopen(TEMP_FILE, "w")) == NULL) {
	bug("Save_char_obj: fopen", 0);
	perror(strsave);
    } else {
	fwrite_char(ch, fp);
	if (ch->carrying != NULL)
	    fwrite_obj(ch, ch->carrying, fp, 0);
	/* save the pets */
	if (ch->quest != NULL)
	    fwrite_quest(ch->quest, fp);
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet, fp);
	fprintf(fp, "#END\n");
    }
    fclose(fp);
    rename(TEMP_FILE, strsave);
    fpReserve = fopen(NULL_FILE, "r");
    return;
}



/*
* Write the char.
*/
void fwrite_char(CHAR_DATA * ch, FILE * fp)
{
    AFFECT_DATA *paf;
    int sn, gn, pos;
    GRANT_DATA *gran;

    /* smash tildes from all string variables TC970224 hack */
    smash_tilde(ch->name);

    fprintf(fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER");

    fprintf(fp, "Name %s~\n", ch->name);
    fprintf(fp, "Id   %ld\n", ch->id);

    /* Write the socket information --Vorlin */
    if (ch->pcdata->socket != NULL && ch->desc != NULL) {
	fprintf(fp, "Sock %s~\n", ch->pcdata->socket);
    } else {
	fprintf(fp, "Sock none~\n");
    }

    fprintf(fp, "LogO %ld\n", current_time);
    fprintf(fp, "Vers %d\n", 5);
    if (ch->short_descr[0] != '\0') {
	smash_tilde(ch->short_descr);
	fprintf(fp, "ShD  %s~\n", ch->short_descr);
    }
    if (ch->long_descr[0] != '\0') {
	smash_tilde(ch->long_descr);
	fprintf(fp, "LnD  %s~\n", ch->long_descr);
    }
    if (ch->description[0] != '\0') {
	smash_tilde(ch->description);
	fprintf(fp, "Desc %s~\n", ch->description);
    }
    if (ch->savedesc != NULL && ch->savedesc[0] != '\0') {
      smash_tilde(ch->savedesc);
      fprintf(fp, "SDsc %s~\n", ch->savedesc);}
 
    if (ch->prompt != NULL
	|| !str_cmp(ch->prompt, "<`^%h``hp `#%m``m `$%v``mv``> ")) {
	smash_tilde(ch->prompt);
	fprintf(fp, "Prom %s~\n", ch->prompt);
    }
    fprintf(fp, "Race %s~\n", pc_race_table[ch->race].name);

    /* if (ch->clan) */
/* For new clans - Suzuran */
/*		fprintf( fp, "Clan %s~\n",clan_table[ch->clan].name); */
    /*        fprintf(fp, "Clan %s~\n", get_clan_name(ch->clan)); */

    fprintf(fp, "Sex  %d\n", ch->sex);
    fprintf(fp, "Cla  %d\n", ch->class);
    fprintf(fp, "Levl %d\n", ch->level);

    if (ch->trust != 0)
	fprintf(fp, "Tru  %d\n", ch->trust);
    fprintf( fp, "Sec  %d\n",    ch->pcdata->security );      /* OLC */

    fprintf(fp, "Plyd %d\n",
	    ch->played + (int) (current_time - ch->logon));
    fprintf(fp, "Not  %ld %ld %ld %ld %ld %ld\n", ch->pcdata->last_note,
	    ch->pcdata->last_idea, ch->pcdata->last_penalty,
	    ch->pcdata->last_news, ch->pcdata->last_changes,
	    ch->pcdata->last_bounty);
    fprintf(fp, "Scro %d\n", ch->lines);
    fprintf(fp, "Room %d\n",
	    (ch->in_room == get_room_index(ROOM_VNUM_LIMBO)
	     && ch->was_in_room != NULL)
	    ? ch->was_in_room->vnum
	    : ch->in_room == NULL ? 3001 : ch->in_room->vnum);

    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
	    ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
	    ch->max_move);
    if (ch->gold > 0)
	fprintf(fp, "Gold %ld\n", ch->gold);
    else
	fprintf(fp, "Gold %d\n", 0);
    if (ch->silver > 0)
	fprintf(fp, "Silv %ld\n", ch->silver);
    else
	fprintf(fp, "Silv %d\n", 0);
    fprintf(fp, "Exp  %d\n", ch->exp);
    if (ch->act != 0)
	fprintf(fp, "Act  %s\n", print_flags(ch->act));
    if (ch->affected_by != 0)
	fprintf(fp, "AfBy %s\n", print_flags(ch->affected_by));
    if (ch->epf != 0)
	fprintf(fp, "Epf  %s\n", print_flags(ch->epf));
    fprintf(fp, "Comm %s\n", print_flags(ch->comm));
    if (ch->wiznet)
	fprintf(fp, "Wizn %s\n", print_flags(ch->wiznet));
    if (ch->invis_level)
	fprintf(fp, "Invi %d\n", ch->invis_level);
    if (ch->incog_level)
	fprintf(fp, "Inco %d\n", ch->incog_level);
    fprintf(fp, "Pos  %d\n",
	    ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
    if (ch->practice != 0)
	fprintf(fp, "Prac %d\n", ch->practice);
    if (ch->train != 0)
	fprintf(fp, "Trai %d\n", ch->train);
    if (ch->saving_throw != 0)
	fprintf(fp, "Save  %d\n", ch->saving_throw);
    fprintf(fp, "Alig  %d\n", ch->alignment);
    if (ch->hitroll != 0)
	fprintf(fp, "Hit   %d\n", ch->hitroll);
    if (ch->damroll != 0)
	fprintf(fp, "Dam   %d\n", ch->damroll);
    fprintf(fp, "ACs %d %d %d %d\n",
	    ch->armor[0], ch->armor[1], ch->armor[2], ch->armor[3]);
    if (ch->wimpy != 0)
	fprintf(fp, "Wimp  %d\n", ch->wimpy);
    fprintf(fp, "Attr %d %d %d %d %d\n",
	    ch->perm_stat[STAT_STR],
	    ch->perm_stat[STAT_INT],
	    ch->perm_stat[STAT_WIS],
	    ch->perm_stat[STAT_DEX], ch->perm_stat[STAT_CON]);

    fprintf(fp, "AMod %d %d %d %d %d\n",
	    ch->mod_stat[STAT_STR],
	    ch->mod_stat[STAT_INT],
	    ch->mod_stat[STAT_WIS],
	    ch->mod_stat[STAT_DEX], ch->mod_stat[STAT_CON]);

    if (IS_NPC(ch)) {
	fprintf(fp, "Vnum %d\n", ch->pIndexData->vnum);
    } else {
	/*EE960410 */
	fprintf(fp, "Pkset %d\n", ch->pcdata->pkset);
	/*TG970927 */
	if (ch->pcdata->bracket_title[0] != '\0') {
	    smash_tilde(ch->pcdata->bracket_title);
	    fprintf(fp, "Btitle %s~\n", ch->pcdata->bracket_title);
	}

	for (gran = ch->pcdata->granted; gran != NULL; gran = gran->next)
	    fprintf(fp, "GLvl '%s' %d\n", gran->name, gran->duration);
	/*TG980628 */
	if (ch->rp_points > 0)
	    fprintf(fp, "RPoints %d\n", ch->rp_points);
	if (ch->next_points > 0)
	    fprintf(fp, "RPnext %d\n", ch->next_points);
/*		if (ch->pcdata->clan_rank > 0)
			fprintf( fp, "CRank %d\n", ch->pcdata->clan_rank); */
	/*EE960522 */
	if (ch->pcdata->pkills != 0)
	    fprintf(fp, "Pkills %d\n", ch->pcdata->pkills);
	if (ch->pcdata->pdeaths != 0)
	    fprintf(fp, "Pdeaths %d\n", ch->pcdata->pdeaths);
	/* Autoquest Saving */
	if (ch->questpoints != 0)
	    fprintf(fp, "QuestPnts %d\n", ch->questpoints);
	if (ch->nextquest != 0)
	    fprintf(fp, "QuestNext %d\n", ch->nextquest);
	else if (ch->countdown != 0)
	    fprintf(fp, "QuestNext %d\n", 10);

	/* Jail timer */
	if(ch->jailtime != 0)
	    fprintf(fp, "JailTime %d\n", ch->jailtime);
	


	/*EE960530 */
	if (ch->pcdata->scars != 0)
	    fprintf(fp, "ScarFlg  %s\n", print_flags(ch->pcdata->scars));

	fprintf(fp, "Pass %s~\n", ch->pcdata->pwd);
	if (ch->pcdata->bamfin[0] != '\0')
	    fprintf(fp, "Bin  %s~\n", ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0] != '\0')
	    fprintf(fp, "Bout %s~\n", ch->pcdata->bamfout);
	/*EE960526 */
	/*if (ch->pcdata->walkin[0] != '\0')
	   *    fprintf( fp, "Walkin  %s~\n", ch->pcdata->walkin);
	   * if (ch->pcdata->walkout[0] != '\0')
	   fprintf( fp, "Walkout %s~\n", ch->pcdata->walkout); */

	fprintf(fp, "Titl %s~\n", ch->pcdata->title);
	fprintf(fp, "Pnts %d\n", ch->pcdata->points);
	fprintf(fp, "TSex %d\n", ch->pcdata->true_sex);
	fprintf(fp, "LLev %d\n", ch->pcdata->last_level);
	fprintf(fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit,
		ch->pcdata->perm_mana, ch->pcdata->perm_move);
	fprintf(fp, "Cnd  %d %d %d %d\n",
		ch->pcdata->condition[0],
		ch->pcdata->condition[1],
		ch->pcdata->condition[2], ch->pcdata->condition[3]);
	fprintf(fp, "Dhngr %d\n", ch->pcdata->dhunger);
	fprintf(fp, "Dthrst %d\n", ch->pcdata->dthirst);

	fprintf(fp,
		"Color %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		ch->color, ch->pcdata->color_hp, ch->pcdata->color_mana,
		ch->pcdata->color_move, ch->pcdata->color_combat_s,
		ch->pcdata->color_combat_o,
		ch->pcdata->color_combat_condition_s,
		ch->pcdata->color_combat_condition_o,
		ch->pcdata->color_invis, ch->pcdata->color_wizi,
		ch->pcdata->color_hidden, ch->pcdata->color_charmed,
		ch->pcdata->color_say, ch->pcdata->color_tell,
		ch->pcdata->color_reply, ch->pcdata->color_psi);

	if (ch->bank > 0)
	    fprintf(fp, "Bank %ld\n", ch->bank);
	else
	    fprintf(fp, "Bank %d\n", 0);

	/* write alias */
	for (pos = 0; pos < MAX_ALIAS; pos++) {
	    if (ch->pcdata->alias[pos] == NULL
		|| ch->pcdata->alias_sub[pos] == NULL)
		break;

	    fprintf(fp, "Alias %s %s~\n", ch->pcdata->alias[pos],
		    ch->pcdata->alias_sub[pos]);
	}

	for (sn = 0; sn < MAX_SKILL; sn++) {
	    if (skill_table[sn].name != NULL
		&& ch->pcdata->learned[sn] > 0) {
		fprintf(fp, "Sk %d '%s'\n", ch->pcdata->learned[sn],
			skill_table[sn].name);
	    }
	}

	for (gn = 0; gn < MAX_GROUP; gn++) {
	    if (group_table[gn].name != NULL
		&& ch->pcdata->group_known[gn]) {
		fprintf(fp, "Gr '%s'\n", group_table[gn].name);
	    }
	}
    }

    for (paf = ch->affected; paf != NULL; paf = paf->next) {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;

	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].name,
		paf->where,
		paf->level,
		paf->duration, paf->modifier, paf->location,
		paf->bitvector);
    }

    fprintf(fp, "End\n\n");
    return;
}

/* write a quest mob */
void fwrite_quest(CHAR_DATA * quest, FILE * fp)
{
    AFFECT_DATA *paf;

    fprintf(fp, "#QUEST\n");

    fprintf(fp, "Vnum %d\n", quest->pIndexData->vnum);

    fprintf(fp, "Name %s~\n", quest->name);
    fprintf(fp, "LogO %ld\n", current_time);
    if (quest->short_descr != quest->pIndexData->short_descr)
	fprintf(fp, "ShD  %s~\n", quest->short_descr);
    if (quest->long_descr != quest->pIndexData->long_descr)
	fprintf(fp, "LnD  %s~\n", quest->long_descr);
    if (quest->description != quest->pIndexData->description)
	fprintf(fp, "Desc %s~\n", quest->description);
    if (quest->race != quest->pIndexData->race)
	fprintf(fp, "Race %s~\n", race_table[quest->race].name);
    if (quest->clan)
/*        fprintf( fp, "Clan %s~\n",clan_table[quest->clan].name); */
	/*        fprintf(fp, "Clan %s~\n", get_clan_name(quest->clan)); */
	fprintf(fp, "Sex  %d\n", quest->sex);
    if (quest->level != quest->pIndexData->level)
	fprintf(fp, "Levl %d\n", quest->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
	    quest->hit, quest->max_hit, quest->mana, quest->max_mana,
	    quest->move, quest->max_move);
    if (quest->gold > 0)
	fprintf(fp, "Gold %ld\n", quest->gold);
    if (quest->silver > 0)
	fprintf(fp, "Silv %ld\n", quest->silver);
    if (quest->exp > 0)
	fprintf(fp, "Exp  %d\n", quest->exp);
    if (quest->act != quest->pIndexData->act)
	fprintf(fp, "Act  %s\n", print_flags(quest->act));
    if (quest->affected_by != quest->pIndexData->affected_by)
	fprintf(fp, "AfBy %s\n", print_flags(quest->affected_by));
    if (quest->comm != 0)
	fprintf(fp, "Comm %s\n", print_flags(quest->comm));
    fprintf(fp, "Pos  %d\n", quest->position =
	    POS_FIGHTING ? POS_STANDING : quest->position);
    if (quest->saving_throw != 0)
	fprintf(fp, "Save %d\n", quest->saving_throw);
    if (quest->alignment != quest->pIndexData->alignment)
	fprintf(fp, "Alig %d\n", quest->alignment);
    if (quest->hitroll != quest->pIndexData->hitroll)
	fprintf(fp, "Hit  %d\n", quest->hitroll);
    if (quest->damroll != quest->pIndexData->damage[DICE_BONUS])
	fprintf(fp, "Dam  %d\n", quest->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
	    quest->armor[0], quest->armor[1], quest->armor[2],
	    quest->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n", quest->perm_stat[STAT_STR],
	    quest->perm_stat[STAT_INT], quest->perm_stat[STAT_WIS],
	    quest->perm_stat[STAT_DEX], quest->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n", quest->mod_stat[STAT_STR],
	    quest->mod_stat[STAT_INT], quest->mod_stat[STAT_WIS],
	    quest->mod_stat[STAT_DEX], quest->mod_stat[STAT_CON]);

    for (paf = quest->affected; paf != NULL; paf = paf->next) {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;

	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].name,
		paf->where, paf->level, paf->duration, paf->modifier,
		paf->location, paf->bitvector);
    }

    fprintf(fp, "End\n");
    return;
}

/* write a pet */
void fwrite_pet(CHAR_DATA * pet, FILE * fp)
{
    AFFECT_DATA *paf;

    fprintf(fp, "#PET\n");

    fprintf(fp, "Vnum %d\n", pet->pIndexData->vnum);

    /* smash tildes from all string variables TC970224 hack */
    smash_tilde(pet->name);

    fprintf(fp, "Name %s~\n", pet->name);
    fprintf(fp, "LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
	fprintf(fp, "ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
	fprintf(fp, "LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
	fprintf(fp, "Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
	fprintf(fp, "Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
/*        fprintf( fp, "Clan %s~\n",clan_table[pet->clan].name); */
	/*        fprintf(fp, "Clan %s~\n", get_clan_name(pet->clan)); */
	fprintf(fp, "Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
	fprintf(fp, "Levl %d\n", pet->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
	    pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move,
	    pet->max_move);
    if (pet->gold > 0)
	fprintf(fp, "Gold %ld\n", pet->gold);
    if (pet->silver > 0)
	fprintf(fp, "Silv %ld\n", pet->silver);
    if (pet->exp > 0)
	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
	fprintf(fp, "Act  %s\n", print_flags(pet->act));
    if (pet->affected_by != pet->pIndexData->affected_by)
	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->comm != 0)
	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    fprintf(fp, "Pos  %d\n", pet->position =
	    POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
	    pet->armor[0], pet->armor[1], pet->armor[2], pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
	    pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
	    pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
	    pet->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n",
	    pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
	    pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
	    pet->mod_stat[STAT_CON]);

    for (paf = pet->affected; paf != NULL; paf = paf->next) {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;

	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].name,
		paf->where, paf->level, paf->duration, paf->modifier,
		paf->location, paf->bitvector);
    }

    fprintf(fp, "End\n");
    return;
}

/*
* Write an object and its contents.
*/
void fwrite_obj(CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest)
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
       * Slick recursion to write lists backwards,
       *   so loading them will load in forwards order.
     */
    if (obj->next_content != NULL)
	fwrite_obj(ch, obj->next_content, fp, iNest);

    /*
       * Castrate storage characters.


       if ( (ch->level < obj->level - 2 && obj->item_type != ITEM_CONTAINER)
       ||   obj->item_type == ITEM_KEY
       ||   (obj->item_type == ITEM_MAP && !obj->value[0]))
       return; */

    fprintf(fp, "#O\n");
    fprintf(fp, "Vnum %d\n", obj->pIndexData->vnum);
    if (!obj->pIndexData->new_format)
	fprintf(fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf(fp, "Enchanted\n");
    fprintf(fp, "Nest %d\n", iNest);

    /* these data are only used if they do not match the defaults */

    if (obj->name != obj->pIndexData->name)
	fprintf(fp, "Name %s~\n", obj->name);
    if (obj->short_descr != obj->pIndexData->short_descr)
	fprintf(fp, "ShD  %s~\n", obj->short_descr);
    if (obj->description != obj->pIndexData->description)
	fprintf(fp, "Desc %s~\n", obj->description);
    if (obj->owner != NULL)
	fprintf(fp, "Owner %s~\n", obj->owner);

    /* Added by Vorlin */
    if (obj->iname != NULL) {
	fprintf(fp, "Ldby %s~\n", obj->iname);
    }

    if (obj->clone != NULL) {
	fprintf(fp, "Clnd %s~\n", obj->clone);
    }

    if (obj->extra_flags != obj->pIndexData->extra_flags)
	fprintf(fp, "ExtF %ld\n", obj->extra_flags);
    if (obj->wear_flags != obj->pIndexData->wear_flags)
	fprintf(fp, "WeaF %ld\n", obj->wear_flags);
    if (obj->item_type != obj->pIndexData->item_type)
	fprintf(fp, "Ityp %d\n", obj->item_type);
    if (obj->weight != obj->pIndexData->weight)
	fprintf(fp, "Wt   %d\n", obj->weight);
    if (obj->condition != obj->pIndexData->condition)
	fprintf(fp, "Cond %d\n", obj->condition);

    /* variable data */

    fprintf(fp, "Wear %d\n", obj->wear_loc);
    if (obj->level != obj->pIndexData->level)
	fprintf(fp, "Lev  %d\n", obj->level);
    if (obj->timer != 0)
	fprintf(fp, "Time %d\n", obj->timer);
    fprintf(fp, "Cost %d\n", obj->cost);
    if (obj->value[0] != obj->pIndexData->value[0]
	|| obj->value[1] != obj->pIndexData->value[1]
	|| obj->value[2] != obj->pIndexData->value[2]
	|| obj->value[3] != obj->pIndexData->value[3]
	|| obj->value[4] != obj->pIndexData->value[4])
	fprintf(fp, "Val  %ld %ld %ld %ld %ld\n",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3],
		obj->value[4]);

    switch (obj->item_type) {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if (obj->value[1] > 0) {
	    fprintf(fp, "Spell 1 '%s'\n", skill_table[obj->value[1]].name);
	}

	if (obj->value[2] > 0) {
	    fprintf(fp, "Spell 2 '%s'\n", skill_table[obj->value[2]].name);
	}

	if (obj->value[3] > 0) {
	    fprintf(fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name);
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if (obj->value[3] > 0) {
	    fprintf(fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name);
	}

	break;
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;
	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].name,
		paf->where,
		paf->level,
		paf->duration, paf->modifier, paf->location,
		paf->bitvector);
    }
    for (ed = obj->extra_descr; ed != NULL; ed = ed->next) {
	fprintf(fp, "ExDe %s~ %s~\n", ed->keyword, ed->description);
    }

    fprintf(fp, "End\n\n");

    if (obj->contains != NULL)
	fwrite_obj(ch, obj->contains, fp, iNest + 1);

    return;
}



/*
* Load a char and inventory into a new ch structure.
*/
bool load_char_obj(DESCRIPTOR_DATA * d, char *name)
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;

    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character = ch;
    ch->desc = d;
    ch->name = str_dup(name);
    ch->id = get_pc_id();
    ch->race = race_lookup("human");
    ch->act = PLR_NOSUMMON;
    ch->comm = COMM_COMBINE | COMM_PROMPT;
    ch->prompt = str_dup("<`^%h``hp `#%m``m `$%v``mv``>\n\r ");
    ch->epf = 0;
    ch->pcdata->confirm_delete = FALSE;
    ch->pcdata->pkset = FALSE;	/*EE960410 */
    ch->pcdata->confirm_pkset = FALSE;	/*EE960410 */
/*    ch->pcdata->clan_rank				= 0; *//*TG970927 */
    ch->rp_points = 0;		/*TG980628 */
    ch->pcdata->granted = NULL;	/* Grant code */
    ch->pcdata->pkills = 0;	/*EE960522 */
    ch->pcdata->pdeaths = 0;	/*EE960522 */
/*ch->pcdata->questgiver              = NULL; *//*EE960522 */
    /*ch->pcdata->quest_waiting           = FALSE; */
    ch->pcdata->pwd = str_dup("");
    ch->pcdata->bamfin = str_dup("");
    ch->pcdata->bamfout = str_dup("");
/*ch->pcdata->walkin                  = str_dup( "" ); *//*EE960526 */
/*ch->pcdata->walkout                 = str_dup( "" ); *//*EE960526 */
    ch->pcdata->title = str_dup("");
    ch->pcdata->bracket_title = str_dup("");
    ch->color = 0;
    ch->pcdata->color_combat_condition_s = 0x1;
    ch->pcdata->color_combat_s = 0x2;
    ch->pcdata->color_invis = 0x3;
    ch->pcdata->color_wizi = 0x4;
    ch->pcdata->color_hp = 0x5;
    ch->pcdata->color_combat_condition_o = 0x6;
    ch->pcdata->color_combat_o = 0x7;
    ch->pcdata->color_hidden = 0x8;
    ch->pcdata->color_charmed = 0x9;
    ch->pcdata->color_mana = 0xa;
    ch->pcdata->color_move = 0xb;
    ch->pcdata->color_say = 0xc;
    ch->pcdata->color_tell = 0x0;
    ch->pcdata->color_reply = 0x1;
    ch->pcdata->color_psi = 0x2;

    for (stat = 0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat] = 13;
    ch->pcdata->condition[COND_THIRST] = 48;
    ch->pcdata->condition[COND_FULL] = 48;
    ch->pcdata->condition[COND_HUNGER] = 48;
    ch->pcdata->security              = 0;    /* OLC */

    found = FALSE;
    fclose(fpReserve);

#if defined(unix)
    /* decompress if .gz file exists */
    sprintf(strsave, "%s%s%s", PLAYER_DIR, capitalize(name), ".gz");
    if ((fp = fopen(strsave, "r")) != NULL) {
	fclose(fp);
	sprintf(buf, "gzip -dfq %s", strsave);
	system(buf);
    }
#endif

    sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(name));
    if ((fp = fopen(strsave, "r")) != NULL) {
	int iNest;

	for (iNest = 0; iNest < MAX_NEST; iNest++)
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for (;;) {
	    char letter;
	    char *word;

	    letter = fread_letter(fp);
	    if (letter == '*') {
		fread_to_eol(fp);
		continue;
	    }

	    if (letter != '#') {
		bug("Load_char_obj: # not found.", 0);
		break;
	    }

	    word = fread_word(fp);
	    if (!str_cmp(word, "PLAYER"))
		fread_char(ch, fp);
	    else if (!str_cmp(word, "OBJECT"))
		fread_obj(ch, fp);
	    else if (!str_cmp(word, "O"))
		fread_obj(ch, fp);
	    else if (!str_cmp(word, "QUEST"))
		fread_quest(ch, fp);
	    else if (!str_cmp(word, "PET"))
		fread_pet(ch, fp);
	    else if (!str_cmp(word, "END"))
		break;
	    else {
		bug("Load_char_obj: bad section.", 0);
		break;
	    }
	}
	fclose(fp);
    }

    fpReserve = fopen(NULL_FILE, "r");


    /* initialize race */
    if (found) {
	int i;

	if (ch->race == 0)
	    ch->race = race_lookup("human");

	ch->size = pc_race_table[ch->race].size;
	ch->dam_type = 17;	/*punch */

	for (i = 0; i < 5; i++) {
	    if (pc_race_table[ch->race].skills[i] == NULL)
		break;
	    group_add(ch, pc_race_table[ch->race].skills[i], FALSE);
	}
	ch->affected_by = ch->affected_by | race_table[ch->race].aff;
	ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags = ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
	ch->form = race_table[ch->race].form;
	ch->parts = race_table[ch->race].parts;
    }


    /* RT initialize skills */

    if (found && ch->version < 2) {	/* need to add the new skills */
	group_add(ch, "rom basics", FALSE);
	group_add(ch, class_table[ch->class].base_group, FALSE);
	group_add(ch, class_table[ch->class].default_group, TRUE);
	ch->pcdata->learned[gsn_recall] = 50;
    }

    /* fix levels */
    if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35)) {
	switch (ch->level) {
	case (40):
	    ch->level = 60;
	    break;		/* imp -> imp */
	case (39):
	    ch->level = 58;
	    break;		/* god -> supreme */
	case (38):
	    ch->level = 56;
	    break;		/* deity -> god */
	case (37):
	    ch->level = 53;
	    break;		/* angel -> demigod */
	}

	switch (ch->trust) {
	case (40):
	    ch->trust = 60;
	    break;		/* imp -> imp */
	case (39):
	    ch->trust = 58;
	    break;		/* god -> supreme */
	case (38):
	    ch->trust = 56;
	    break;		/* deity -> god */
	case (37):
	    ch->trust = 53;
	    break;		/* angel -> demigod */
	case (36):
	    ch->trust = 51;
	    break;		/* hero -> hero */
	}
    }

    /* ream gold */
    if (found && ch->version < 4) {
	ch->gold /= 100;
    }
    return found;
}

extern	OBJ_DATA	*obj_free;

/*
* Read in a char.
*/

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )			  	   \
				if ( !str_cmp( word, literal ) )   \
				{			           \
				field  = value;                    \
                                fMatch = TRUE;			   \
                                break; }

/* To fix memory leakage - Suzuran */

#define KEYS( literal, field, value )                              \
                               if(!str_cmp(word,literal)){         \
                                  free_string(field);              \
                                  field=value;                     \
                                  fMatch=TRUE;                     \
                                  break;                           \
                                }

void fread_char(CHAR_DATA * ch, FILE * fp)
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int count = 0;
    int lastlogoff = current_time;
    int percent;

    sprintf(buf, "Loading %s.", ch->name);
    log_string(buf);

    for (;;) {
	word = feof(fp) ? "End" : fread_word(fp);
	fMatch = FALSE;

	switch (UPPER(word[0])) {
	case '*':
	    fMatch = TRUE;
	    fread_to_eol(fp);
	    break;

	case 'A':
	    KEY("Act", ch->act, fread_flag(fp));
	    KEY("AffectedBy", ch->affected_by, fread_flag(fp));
	    KEY("AfBy", ch->affected_by, fread_flag(fp));
	    KEY("Alignment", ch->alignment, fread_number(fp));
	    KEY("Alig", ch->alignment, fread_number(fp));

	    if (!str_cmp(word, "Alia")) {
		if (count >= MAX_ALIAS) {
		    fread_to_eol(fp);
		    fMatch = TRUE;
		    break;
		}

		ch->pcdata->alias[count] = str_dup(fread_word(fp));
		ch->pcdata->alias_sub[count] = str_dup(fread_word(fp));
		count++;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Alias")) {
		if (count >= MAX_ALIAS) {
		    fread_to_eol(fp);
		    fMatch = TRUE;
		    break;
		}

		ch->pcdata->alias[count] = str_dup(fread_word(fp));
		ch->pcdata->alias_sub[count] = fread_string(fp);
		count++;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AC") || !str_cmp(word, "Armor")) {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "ACs")) {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD")) {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.", 0);
		else
		    paf->type = sn;

		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
		paf->modifier = fread_number(fp);
		paf->location = fread_number(fp);
		paf->bitvector = fread_number(fp);
		paf->next = ch->affected;
		ch->affected = paf;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Affc")) {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.", 0);
		else
		    paf->type = sn;

		paf->where = fread_number(fp);
		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
		paf->modifier = fread_number(fp);
		paf->location = fread_number(fp);
		paf->bitvector = fread_number(fp);
		paf->next = ch->affected;
		ch->affected = paf;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AttrMod") || !str_cmp(word, "AMod")) {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AttrPerm") || !str_cmp(word, "Attr")) {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEYS("Bamfin", ch->pcdata->bamfin, fread_string(fp));
	    KEYS("Bamfout", ch->pcdata->bamfout, fread_string(fp));
	    KEY("Bank", ch->bank, fread_number(fp));
	    KEYS("Bin", ch->pcdata->bamfin, fread_string(fp));
	    KEYS("Bout", ch->pcdata->bamfout, fread_string(fp));
	    KEYS("Btitle", ch->pcdata->bracket_title, fread_string(fp));
	    break;

	case 'C':
	    KEY("Class", ch->class, fread_number(fp));
	    KEY("Cla", ch->class, fread_number(fp));
	    /* KEY( "Clan",     ch->clan, clan_lookup(fread_string(fp))); */

	    if (!str_cmp(word, "Condition") || !str_cmp(word, "Cond")) {
		ch->pcdata->condition[0] = fread_number(fp);
		ch->pcdata->condition[1] = fread_number(fp);
		ch->pcdata->condition[2] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word, "Cnd")) {
		ch->pcdata->condition[0] = fread_number(fp);
		ch->pcdata->condition[1] = fread_number(fp);
		ch->pcdata->condition[2] = fread_number(fp);
		ch->pcdata->condition[3] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Color")) {
		log_string("Reading colors");
		ch->color = fread_number(fp);
		ch->pcdata->color_hp = fread_number(fp);
		ch->pcdata->color_mana = fread_number(fp);
		ch->pcdata->color_move = fread_number(fp);
		ch->pcdata->color_combat_s = fread_number(fp);
		ch->pcdata->color_combat_o = fread_number(fp);
		ch->pcdata->color_combat_condition_s = fread_number(fp);
		ch->pcdata->color_combat_condition_o = fread_number(fp);
		ch->pcdata->color_invis = fread_number(fp);
		ch->pcdata->color_wizi = fread_number(fp);
		ch->pcdata->color_hidden = fread_number(fp);
		ch->pcdata->color_charmed = fread_number(fp);
		ch->pcdata->color_say = fread_number(fp);
		ch->pcdata->color_tell = fread_number(fp);
		ch->pcdata->color_reply = fread_number(fp);
		ch->pcdata->color_psi = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    KEY("Comm", ch->comm, fread_flag(fp));
	    /*TG970927 */
	    /*          KEY( "CRank",   ch->pcdata->clan_rank,fread_number( fp ) ); */

	    break;

	case 'D':
	    KEY("Damroll", ch->damroll, fread_number(fp));
	    KEY("Dam", ch->damroll, fread_number(fp));
	    KEYS("Description", ch->description, fread_string(fp));
	    KEYS("Desc", ch->description, fread_string(fp));
	    if (!str_cmp(word, "Dhngr")) {
		ch->pcdata->dhunger = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word, "Dthrst")) {
		ch->pcdata->dthirst = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'E':
	    if (!str_cmp(word, "End")) {
		/* adjust hp mana move up  -- here for speed's sake */
		percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);

		percent = UMIN(percent, 100);

		if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON)
		    && !IS_AFFECTED(ch, AFF_PLAGUE)) {
		    ch->hit += (ch->max_hit - ch->hit) * percent / 100;
		    ch->mana += (ch->max_mana - ch->mana) * percent / 100;
		    ch->move += (ch->max_move - ch->move) * percent / 100;
		}

		/* SET CLAN STATUS - SUZURAN */
		ch->clan = is_clannie(ch->name);
		if (ch->clan != 0 && is_leader(ch)) {
		    SET_BIT(ch->act, PLR_LEADER);
		} else {
		    REMOVE_BIT(ch->act, PLR_LEADER);
		}

		return;
	    }
	    KEY("Epf", ch->epf, fread_flag(fp));
	    KEY("Exp", ch->exp, fread_number(fp));
	    break;

	case 'G':
	    KEY("Gold", ch->gold, fread_number(fp));
	    if (!str_cmp(word, "Group") || !str_cmp(word, "Gr")) {
		int gn;
		char *temp;

		temp = fread_word(fp);
		gn = group_lookup(temp);
		/* gn    = group_lookup( fread_word( fp ) ); */
		if (gn < 0) {
		    fprintf(stderr, "%s", temp);
		    bug("Fread_char: unknown group. ", 0);
		} else {
		  /* Selective group add/remove here - Suzuran */
		  if(!strcmp("transportation",temp)){
		    if(ch->class != class_lookup("Mage") &&
		       ch->class != class_lookup("Cleric")){
		      /* Char is NOT a mage or cleric and should not
			 get transportation */
		      fMatch = TRUE;
		      gn_add(ch,gn);
		      /* Now flush the group and it's contained spells */
		      gn_remove(ch,gn);
		      /* And fix up trains */
		      ch->train += 7;
		    }else{
		      /* Char IS a mage or cleric and should get it */
		      gn_add(ch, gn);
		      fMatch = TRUE;
		    }
		  }else{ /* Not transportation group */
		    gn_add(ch,gn);
		    fMatch = TRUE;
		  }
		}
	    }
	
	    if (!str_cmp(word, "GLvl")) {
		GRANT_DATA *gran;
		int cmd;

		gran = alloc_mem(sizeof(*gran));
		gran->name = str_dup(fread_word(fp));
		gran->duration = fread_number(fp);
		gran->next = NULL;
		gran->do_fun = NULL;

		for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
		    if (gran->name[0] == cmd_table[cmd].name[0]
			&& is_exact_name(gran->name, cmd_table[cmd].name)) {
			gran->do_fun = cmd_table[cmd].do_fun;
			gran->level = cmd_table[cmd].level;
			break;
		    }

		gran->next = ch->pcdata->granted;
		ch->pcdata->granted = gran;

		if (gran->do_fun == NULL) {
		    sprintf(buf,
			    "Grant: Command %s not found in pfile for %s",
			    gran->name, ch->name);
		    log_string(buf);
		}

		fMatch = TRUE;
	    }

	    break;

	case 'H':
	    KEY("Hitroll", ch->hitroll, fread_number(fp));
	    KEY("Hit", ch->hitroll, fread_number(fp));

	    if (!str_cmp(word, "HpManaMove") || !str_cmp(word, "HMV")) {
		ch->hit = fread_number(fp);
		ch->max_hit = fread_number(fp);
		ch->mana = fread_number(fp);
		ch->max_mana = fread_number(fp);
		ch->move = fread_number(fp);
		ch->max_move = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "HpManaMovePerm") || !str_cmp(word, "HMVP")) {
		ch->pcdata->perm_hit = fread_number(fp);
		ch->pcdata->perm_mana = fread_number(fp);
		ch->pcdata->perm_move = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'I':
	    KEY("Id", ch->id, fread_number(fp));
	    KEY("InvisLevel", ch->invis_level, fread_number(fp));
	    KEY("Inco", ch->incog_level, fread_number(fp));
	    KEY("Invi", ch->invis_level, fread_number(fp));
	    break;

	case 'J':
	    KEY("JailTime", ch->jailtime, fread_number(fp));
	    break;

	case 'L':
	    KEY("LastLevel", ch->pcdata->last_level, fread_number(fp));
	    KEY("LLev", ch->pcdata->last_level, fread_number(fp));
	    KEY("Level", ch->level, fread_number(fp));
	    KEY("Lev", ch->level, fread_number(fp));
	    KEY("Levl", ch->level, fread_number(fp));
	    KEY("LogO", lastlogoff, fread_number(fp));
	    KEYS("LongDescr", ch->long_descr, fread_string(fp));
	    KEYS("LnD", ch->long_descr, fread_string(fp));
	    break;

	case 'N':
	    KEYS("Name", ch->name, fread_string(fp));
	    KEY("Note", ch->pcdata->last_note, fread_number(fp));
	    if (!str_cmp(word, "Not")) {
		ch->pcdata->last_note = fread_number(fp);
		ch->pcdata->last_idea = fread_number(fp);
		ch->pcdata->last_penalty = fread_number(fp);
		ch->pcdata->last_news = fread_number(fp);
		ch->pcdata->last_changes = fread_number(fp);
		ch->pcdata->last_bounty = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'P':
	    KEYS("Password", ch->pcdata->pwd, fread_string(fp));
	    KEYS("Pass", ch->pcdata->pwd, fread_string(fp));
	    /*EE960410 */
	    KEY("Pkset", ch->pcdata->pkset, fread_number(fp));

	    /*EE960522 */
	    KEY("Pkills", ch->pcdata->pkills, fread_number(fp));
	    KEY("Pdeaths", ch->pcdata->pdeaths, fread_number(fp));

	    KEY("Played", ch->played, fread_number(fp));
	    KEY("Plyd", ch->played, fread_number(fp));
	    KEY("Points", ch->pcdata->points, fread_number(fp));
	    KEY("Pnts", ch->pcdata->points, fread_number(fp));
	    KEY("Position", ch->position, fread_number(fp));
	    KEY("Pos", ch->position, fread_number(fp));
	    KEY("Practice", ch->practice, fread_number(fp));
	    KEY("Prac", ch->practice, fread_number(fp));
	    KEYS("Prompt", ch->prompt, fread_string(fp));
	    KEYS("Prom", ch->prompt, fread_string(fp));
	    break;

	case 'Q':
	    KEY("QuestPnts", ch->questpoints, fread_number(fp));
	    KEY("QuestNext", ch->nextquest, fread_number(fp));
	    break;

	case 'R':
	    /* KEY("Race", ch->race, race_lookup(fread_string(fp))); */
	    /* Replacement race reader - Suzuran */
	    if (!str_cmp(word, "Race")) {
		char *tmp = fread_string(fp);
		ch->race = race_lookup(tmp);
		free_string(tmp);
		fMatch = TRUE;
		break;
	    }

	    /*TG980628 */
	    KEY("RPoints", ch->rp_points, fread_number(fp));
	    KEY("RPnext", ch->next_points, fread_number(fp));

	    if (!str_cmp(word, "Room")) {
		ch->in_room = get_room_index(fread_number(fp));
		if (ch->in_room == NULL)
		    ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY("SavingThrow", ch->saving_throw, fread_number(fp));
	    KEY("Save", ch->saving_throw, fread_number(fp));

	    KEY("Scro", ch->lines, fread_number(fp));
            KEYS("SDsc", ch->savedesc, fread_string(fp));
	    KEY("Sex", ch->sex, fread_number(fp));
	    KEYS("ShortDescr", ch->short_descr, fread_string(fp));
	    KEYS("ShD", ch->short_descr, fread_string(fp));
	    KEY( "Sec",         ch->pcdata->security,   fread_number( fp ) );
	    /* OLC */
	    KEY("Silv", ch->silver, fread_number(fp));
	    

	    if (!str_cmp(word, "Skill") || !str_cmp(word, "Sk")) {
		int sn;
		int value;
		char *temp;

		value = fread_number(fp);
		temp = fread_word(fp);
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if (sn < 0) {
		    fprintf(stderr, "%s", temp);
		    bug("Fread_char: unknown skill. ", 0);
		} else {
		    if (!str_cmp("possess person", temp)) {
			group_remove(ch, "possess person");
			group_add(ch, "demon rage", FALSE);
			ch->pcdata->learned[gsn_demonrage] = 100;
		    } else {
			ch->pcdata->learned[sn] = value;
		    }
		}
		fMatch = TRUE;
	    }

	    KEYS("Sock", ch->pcdata->socket, fread_string(fp));

	    break;

	case 'T':
	    KEY("TrueSex", ch->pcdata->true_sex, fread_number(fp));
	    KEY("TSex", ch->pcdata->true_sex, fread_number(fp));
	    KEY("Trai", ch->train, fread_number(fp));
	    KEY("Trust", ch->trust, fread_number(fp));
	    KEY("Tru", ch->trust, fread_number(fp));

	    if (!str_cmp(word, "Title") || !str_cmp(word, "Titl")) {
		/* Leak fix - Suzuran */
		free_string(ch->pcdata->title);
		ch->pcdata->title = fread_string(fp);
		if (ch->pcdata->title[0] != '.'
		    && ch->pcdata->title[0] != ','
		    && ch->pcdata->title[0] != '!'
		    && ch->pcdata->title[0] != '?') {
		    sprintf(buf, " %s", ch->pcdata->title);
		    free_string(ch->pcdata->title);
		    ch->pcdata->title = str_dup(buf);
		}
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'V':
	    KEY("Version", ch->version, fread_number(fp));
	    KEY("Vers", ch->version, fread_number(fp));
	    if (!str_cmp(word, "Vnum")) {
		ch->pIndexData = get_mob_index(fread_number(fp));
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':

/*KEY( "Walkin",    ch->pcdata->walkin,     fread_string( fp ) ); *//*EE960526 */
/*KEY( "Walkout",   ch->pcdata->walkout,    fread_string( fp ) ); *//*EE960526 */
	    KEY("Wimpy", ch->wimpy, fread_number(fp));
	    KEY("Wimp", ch->wimpy, fread_number(fp));
	    KEY("Wizn", ch->wiznet, fread_flag(fp));
	    break;
	}

	if (!fMatch) {
	    bug("Fread_char: no match.", 0);
	    fread_to_eol(fp);
	}
    }
}

void fread_quest(CHAR_DATA * ch, FILE * fp)
{
    char *word;
    CHAR_DATA *quest;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word, "Vnum")) {
	int vnum;

	vnum = fread_number(fp);
	if (get_mob_index(vnum) == NULL) {
	    bug("Fread_quest: bad vnum %d.", vnum);
	    quest = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	} else
	    quest = create_mobile(get_mob_index(vnum));
    } else {
	bug("Fread_quest: no vnum in file.", 0);
	quest = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }

    for (;;) {
	word = feof(fp) ? "END" : fread_word(fp);
	fMatch = FALSE;

	switch (UPPER(word[0])) {
	case '*':
	    fMatch = TRUE;
	    fread_to_eol(fp);
	    break;

	case 'A':
	    KEY("Act", quest->act, fread_flag(fp));
	    KEY("AfBy", quest->affected_by, fread_flag(fp));
	    KEY("Alig", quest->alignment, fread_number(fp));

	    if (!str_cmp(word, "ACs")) {
		int i;

		for (i = 0; i < 4; i++)
		    quest->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD")) {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.", 0);
		else
		    paf->type = sn;

		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
		paf->modifier = fread_number(fp);
		paf->location = fread_number(fp);
		paf->bitvector = fread_number(fp);
		paf->next = quest->affected;
		quest->affected = paf;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Affc")) {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.", 0);
		else
		    paf->type = sn;

		paf->where = fread_number(fp);
		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
		paf->modifier = fread_number(fp);
		paf->location = fread_number(fp);
		paf->bitvector = fread_number(fp);
		paf->next = quest->affected;
		quest->affected = paf;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AMod")) {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    quest->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Attr")) {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    quest->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'C':
	    /* KEY( "Clan",       quest->clan,       clan_lookup(fread_string(fp))); */
	    KEY("Comm", quest->comm, fread_flag(fp));
	    break;

	case 'D':
	    KEY("Dam", quest->damroll, fread_number(fp));
	    KEYS("Desc", quest->description, fread_string(fp));
	    break;

	case 'E':
	    if (!str_cmp(word, "End")) {
		quest->leader = ch;
		quest->master = ch;
		ch->quest = quest;
		/* adjust hp mana move up  -- here for speed's sake */
		percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);

		if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON)
		    && !IS_AFFECTED(ch, AFF_PLAGUE)) {
		    percent = UMIN(percent, 100);
		    quest->hit +=
			(quest->max_hit - quest->hit) * percent / 100;
		    quest->mana +=
			(quest->max_mana - quest->mana) * percent / 100;
		    quest->move +=
			(quest->max_move - quest->move) * percent / 100;
		}
		return;
	    }
	    KEY("Exp", quest->exp, fread_number(fp));
	    break;

	case 'G':
	    KEY("Gold", quest->gold, fread_number(fp));
	    break;

	case 'H':
	    KEY("Hit", quest->hitroll, fread_number(fp));

	    if (!str_cmp(word, "HMV")) {
		quest->hit = fread_number(fp);
		quest->max_hit = fread_number(fp);
		quest->mana = fread_number(fp);
		quest->max_mana = fread_number(fp);
		quest->move = fread_number(fp);
		quest->max_move = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'L':
	    KEY("Levl", quest->level, fread_number(fp));
	    KEYS("LnD", quest->long_descr, fread_string(fp));
	    KEY("LogO", lastlogoff, fread_number(fp));
	    break;

	case 'N':
	    KEYS("Name", quest->name, fread_string(fp));
	    break;

	case 'P':
	    KEY("Pos", quest->position, fread_number(fp));
	    break;

	case 'R':
	    KEY("Race", quest->race, race_lookup(fread_string(fp)));
	    break;

	case 'S':
	    KEY("Save", quest->saving_throw, fread_number(fp));
	    KEY("Sex", quest->sex, fread_number(fp));
	    KEYS("ShD", quest->short_descr, fread_string(fp));
	    KEY("Silv", quest->silver, fread_number(fp));
	    break;

	    if (!fMatch) {
		bug("Fread_quest: no match.", 0);
		fread_to_eol(fp);
	    }

	}
    }
}

/* load a pet from the forgotten reaches */
void fread_pet(CHAR_DATA * ch, FILE * fp)
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word, "Vnum")) {
	int vnum;

	vnum = fread_number(fp);
	if (get_mob_index(vnum) == NULL) {
	    bug("Fread_pet: bad vnum %d.", vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	} else
	    pet = create_mobile(get_mob_index(vnum));
    } else {
	bug("Fread_pet: no vnum in file.", 0);
	pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }

    for (;;) {
	word = feof(fp) ? "END" : fread_word(fp);
	fMatch = FALSE;

	switch (UPPER(word[0])) {
	case '*':
	    fMatch = TRUE;
	    fread_to_eol(fp);
	    break;

	case 'A':
	    KEY("Act", pet->act, fread_flag(fp));
	    KEY("AfBy", pet->affected_by, fread_flag(fp));
	    KEY("Alig", pet->alignment, fread_number(fp));

	    if (!str_cmp(word, "ACs")) {
		int i;

		for (i = 0; i < 4; i++)
		    pet->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD")) {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.", 0);
		else
		    paf->type = sn;

		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
		paf->modifier = fread_number(fp);
		paf->location = fread_number(fp);
		paf->bitvector = fread_number(fp);
		paf->next = pet->affected;
		pet->affected = paf;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Affc")) {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.", 0);
		else
		    paf->type = sn;

		paf->where = fread_number(fp);
		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
		paf->modifier = fread_number(fp);
		paf->location = fread_number(fp);
		paf->bitvector = fread_number(fp);
		paf->next = pet->affected;
		pet->affected = paf;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AMod")) {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    pet->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Attr")) {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    pet->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'C':
	    /* KEY("Clan", pet->clan, clan_lookup(fread_string(fp))); */
	    KEY("Comm", pet->comm, fread_flag(fp));
	    break;

	case 'D':
	    KEY("Dam", pet->damroll, fread_number(fp));
	    KEYS("Desc", pet->description, fread_string(fp));
	    break;

	case 'E':
	    if (!str_cmp(word, "End")) {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
		/* adjust hp mana move up  -- here for speed's sake */
		percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);

		if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON)
		    && !IS_AFFECTED(ch, AFF_PLAGUE)) {
		    percent = UMIN(percent, 100);
		    pet->hit += (pet->max_hit - pet->hit) * percent / 100;
		    pet->mana +=
			(pet->max_mana - pet->mana) * percent / 100;
		    pet->move +=
			(pet->max_move - pet->move) * percent / 100;
		}
		return;
	    }
	    KEY("Exp", pet->exp, fread_number(fp));
	    break;

	case 'G':
	    KEY("Gold", pet->gold, fread_number(fp));
	    break;

	case 'H':
	    KEY("Hit", pet->hitroll, fread_number(fp));

	    if (!str_cmp(word, "HMV")) {
		pet->hit = fread_number(fp);
		pet->max_hit = fread_number(fp);
		pet->mana = fread_number(fp);
		pet->max_mana = fread_number(fp);
		pet->move = fread_number(fp);
		pet->max_move = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'L':
	    KEY("Levl", pet->level, fread_number(fp));
	    KEYS("LnD", pet->long_descr, fread_string(fp));
	    KEY("LogO", lastlogoff, fread_number(fp));
	    break;

	case 'N':
	    KEYS("Name", pet->name, fread_string(fp));
	    break;

	case 'P':
	    KEY("Pos", pet->position, fread_number(fp));
	    break;

	case 'R':
	    KEY("Race", pet->race, race_lookup(fread_string(fp)));
	    break;

	case 'S':
	    KEY("Save", pet->saving_throw, fread_number(fp));
	    KEY("Sex", pet->sex, fread_number(fp));
	    KEYS("ShD", pet->short_descr, fread_string(fp));
	    KEY("Silv", pet->silver, fread_number(fp));
	    break;

	    if (!fMatch) {
		bug("Fread_pet: no match.", 0);
		fread_to_eol(fp);
	    }

	}
    }
}



void fread_obj(CHAR_DATA * ch, FILE * fp)
{
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;		/* to prevent errors */
    bool make_new;		/* update object */

    fVnum = FALSE;
    obj = NULL;
    first = TRUE;		/* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word = feof(fp) ? "End" : fread_word(fp);
    if (!str_cmp(word, "Vnum")) {
	int vnum;
	first = FALSE;		/* fp will be in right place */

	vnum = fread_number(fp);

	// This is just a check to change vnums for claneq getting moved to newclan.are
	// Sooner or later, this if can be removed again... safer for me to change
	// here, rather than trying to write up a shell script ;)
	if (((vnum >= 15000) && (vnum < 15006))
	    || ((vnum > 15009) && (vnum < 15040))
	    || ((vnum > 15050) && (vnum <= 15053)))
	    vnum -= 2900;

	// 990118TG
	if (get_obj_index(vnum) == NULL) {
	    bug("Fread_obj: bad vnum %d.", vnum);
	} else {
	    obj = create_object(get_obj_index(vnum), -1);
	    new_format = TRUE;
	}

    }

    if (obj == NULL) {		/* either not found or old style */
	obj = new_obj();
	obj->name = str_dup("");
	obj->short_descr = str_dup("");
	obj->description = str_dup("");
    }

    fNest = FALSE;
    fVnum = TRUE;
    iNest = 0;

    for (;;) {
	if (first)
	    first = FALSE;
	else
	    word = feof(fp) ? "End" : fread_word(fp);
	fMatch = FALSE;

	switch (UPPER(word[0])) {
	case '*':
	    fMatch = TRUE;
	    fread_to_eol(fp);
	    break;

	case 'A':
	    if (!str_cmp(word, "AffD")) {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_obj: unknown skill.", 0);
		else
		    paf->type = sn;

		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
		paf->modifier = fread_number(fp);
		paf->location = fread_number(fp);
		paf->bitvector = fread_number(fp);
		paf->next = obj->affected;
		obj->affected = paf;
		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word, "Affc")) {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_obj: unknown skill.", 0);
		else
		    paf->type = sn;

		paf->where = fread_number(fp);
		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
		paf->modifier = fread_number(fp);
		paf->location = fread_number(fp);
		paf->bitvector = fread_number(fp);
		paf->next = obj->affected;
		obj->affected = paf;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY("Cond", obj->condition, fread_number(fp));
	    KEY("Cost", obj->cost, fread_number(fp));
	    KEY("Clnd", obj->clone, fread_string(fp));
	    break;

	case 'D':
	    KEYS("Description", obj->description, fread_string(fp));
	    KEYS("Desc", obj->description, fread_string(fp));
	    break;

	case 'E':

	    if (!str_cmp(word, "Enchanted")) {
		obj->enchanted = TRUE;
		fMatch = TRUE;
		break;
	    }

	    KEY("ExtraFlags", obj->extra_flags, fread_number(fp));
	    KEY("ExtF", obj->extra_flags, fread_number(fp));

	    if (!str_cmp(word, "ExtraDescr") || !str_cmp(word, "ExDe")) {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword = fread_string(fp);
		ed->description = fread_string(fp);
		ed->next = obj->extra_descr;
		obj->extra_descr = ed;
		fMatch = TRUE;
	    }

	    if (!str_cmp(word, "End")) {
	      // if (!fNest || !fVnum || obj->pIndexData == NULL) {
	      if ( !fNest || ( fVnum && obj->pIndexData == NULL ) ){
		bug("Fread_obj: incomplete object.", 0);
		free_obj(obj);
		return;
	      } else
		{
		  if ( !fVnum )
		    {
		      free_obj( obj );
		      obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0
					   );
		    }

		    if (!new_format) {
			obj->next = object_list;
			object_list = obj;
			obj->pIndexData->count++;
		    }

		    if (!obj->pIndexData->new_format
			&& obj->item_type == ITEM_ARMOR
			&& obj->value[1] == 0) {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new) {
			int wear;

			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData, 0);
			obj->wear_loc = wear;
		    }
		    if (iNest == 0 || rgObjNest[iNest] == NULL)
			obj_to_char(obj, ch);
		    else
			obj_to_obj(obj, rgObjNest[iNest - 1]);
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY("ItemType", obj->item_type, fread_number(fp));
	    KEY("Ityp", obj->item_type, fread_number(fp));
	    break;

	case 'L':
	    KEY("Level", obj->level, fread_number(fp));
	    KEY("Lev", obj->level, fread_number(fp));
	    KEYS("Ldby", obj->iname, fread_string(fp));
	    break;

	case 'N':
	    KEYS("Name", obj->name, fread_string(fp));

	    if (!str_cmp(word, "Nest")) {
		iNest = fread_number(fp);
		if (iNest < 0 || iNest >= MAX_NEST) {
		    bug("Fread_obj: bad nest %d.", iNest);
		} else {
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

	case 'O':
	  /* KEYS("Owner", obj->owner, fread_string(fp)); */
	    if (!str_cmp(word, "Oldstyle")) {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;


	case 'S':
	    KEYS("ShortDescr", obj->short_descr, fread_string(fp));
	    KEYS("ShD", obj->short_descr, fread_string(fp));

	    if (!str_cmp(word, "Spell")) {
		int iValue;
		int sn;

		iValue = fread_number(fp);
		sn = skill_lookup(fread_word(fp));
		if (iValue < 0 || iValue > 3) {
		    bug("Fread_obj: bad iValue %d.", iValue);
		} else if (sn < 0) {
		    bug("Fread_obj: unknown skill.", 0);
		} else {
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY("Timer", obj->timer, fread_number(fp));
	    KEY("Time", obj->timer, fread_number(fp));
	    break;

	case 'V':
	    if (!str_cmp(word, "Values") || !str_cmp(word, "Vals")) {
		obj->value[0] = fread_number(fp);
		obj->value[1] = fread_number(fp);
		obj->value[2] = fread_number(fp);
		obj->value[3] = fread_number(fp);
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		    obj->value[0] = obj->pIndexData->value[0];
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Val")) {
		obj->value[0] = fread_number(fp);
		obj->value[1] = fread_number(fp);
		obj->value[2] = fread_number(fp);
		obj->value[3] = fread_number(fp);
		obj->value[4] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Vnum")) {
		int vnum;

		vnum = fread_number(fp);
		if ((obj->pIndexData = get_obj_index(vnum)) == NULL)
		    bug("Fread_obj: bad vnum %d.", vnum);
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY("WearFlags", obj->wear_flags, fread_number(fp));
	    KEY("WeaF", obj->wear_flags, fread_number(fp));
	    KEY("WearLoc", obj->wear_loc, fread_number(fp));
	    KEY("Wear", obj->wear_loc, fread_number(fp));
	    KEY("Weight", obj->weight, fread_number(fp));
	    KEY("Wt", obj->weight, fread_number(fp));
	    break;

	}

	if (!fMatch) {
	    bug("Fread_obj: no match.", 0);
	    fread_to_eol(fp);
	}
    }
}

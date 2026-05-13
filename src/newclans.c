/* *****

  New Clan Code, Extended Dance Mix! ^_^

  This file should have all the stuff for the new clan code that should go
  in a .c file.  PLEASE DON'T REFER TO THE CLAN-CONTROL STRUCTS HERE FROM
  OUTSIDE THIS FILE! I MIGHT RE-ARRANGE THOSE!  MAKE A FUNCTION AND USE IT
  INSTEAD.  (Some people...)

  This code is Suzuran's fault.  Blame me if it screws up.  Unless you
  modified it after I did, in which case you're on your own. :P

   ***** */

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

/***************************************************************************
 *       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
 *       ROM has been brought to you by the ROM consortium                  *
 *           Russ Taylor (rtaylor@pacinfo.com)                              *
 *           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
 *           Brian Moore (rom@rom.efn.org)                                  *
 *       By using this code, you have agreed to follow the terms of the     *
 *       ROM license, in the file Rom24/doc/rom.license                     *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"

/* DEFINES.  Put some more sane values here for active code! */
#define MAX_CLAN_MEMBER 25
#define MAX_CSKILL_NUMBER 5
/* MAX_CLANS is inherited from merc.h */
#define MAX_CLANS MAX_CLAN
#define MAX_CLAN_RANKS 40
#define MAX_CLAN_EQ 10
#define MAX_PLEDGE 25

/* Clan Flags - USE POWERS OF TWO, THIS IS A BITMAP */

#define CF_NONPK 1
#define CF_RP    2
#define CF_ESTABLISHED 4

/* Clan pledge flags */
#define CPF_PLEDGE 1

/* Player flags */
#define PF_COLDR   1

/* Command declarations */
DECLARE_DO_FUN(do_clanctl);
DECLARE_DO_FUN(do_clanldr);

/* Function declarations */
int add_clan(char *clan_name);
int del_clan(char *clan_name);
int lookup_clan(char *clan_name);
int clannie_lookup(int clan, char *name);
int clan_char(char *clan_name, char *name);
int declan_char(char *clan_name, char *name);
void update_player_clandata();
int is_leader(CHAR_DATA * ch);
int is_initiator(CHAR_DATA * ch);
int is_clannie(char *name);
int get_crank(CHAR_DATA * ch);
void set_crank(CHAR_DATA * ch, int newrank);
int clan_lookup(const char *name);
int pledge_lookup(const char *name, int clan);
int pledge_acl_lookup(const char *name, int clan);
int is_acl_entry(const char *name, int clan);
int new_pledge_entry(char *pname, int clan);
void note_to_char(char *name, char *text);

/* typedefs! */
typedef struct clan_data CLAN_DATA;
typedef struct clan_member_data CLAN_MEMBER_DATA;
typedef struct clan_skill_data CLAN_SKILL_DATA;
typedef struct clan_eq_data CLAN_EQ_DATA;
typedef struct clan_pledge_data CLAN_PLEDGE_DATA;

/* First we need structs we can use to describe a clan.  EVERYTHING
   about a clan should be able to be fit into these structs. */

struct clan_member_data {
    char *name;			/* Who is it? */
    char rank;			/* max 255 */
    int flags;			/* CM flags */
};

struct clan_skill_data {
    char *name;			/* Name */
};

struct clan_eq_data {
    char *name;
    sh_int vnum;
};

struct clan_pledge_data {
    char *name;			/* Who's the pledge? */
    int flags;			/* Special crap */
    int pcost;			/* Pledge cost incurred */
};

struct clan_data {
    char *name;			/* clan short name */
    char *lname;		/* long name */
    unsigned int flags;		/* flags, see below */
    int minlevel;		/* minimum level to join */
    int maxlevel;		/* Maximum level to join */
    char *leader;		/* Who's in charge? */
    char *ranktxt[MAX_CLAN_RANKS + 1];	/* Clan rank text */
    CLAN_MEMBER_DATA *member[MAX_CLAN_MEMBER + 1];	/* Who's in on this? */
    CLAN_SKILL_DATA *skill[MAX_CSKILL_NUMBER + 1];	/* Special clan tricks! */
    CLAN_EQ_DATA *eq[MAX_CLAN_EQ + 1];	/* Clan junk */
    CLAN_PLEDGE_DATA *pledge[MAX_PLEDGE + 1];	/* Pledges */
    char *infotext;		/* Text for CLAN INFO cmd */
    sh_int hall;		/* Clan hall vnum */
    sh_int droom;		/* Death respawn room */
    char *initiator;		/* Who created it? */
    int pledge_policy;		/* Pledge policy flag */
    int cost_to_join;
    long bank_balance;
};

static CLAN_DATA *clantable[MAX_CLANS + 1];

/* And now code. */

/* RT clan manipulation command */

void do_clanctl(CHAR_DATA * ch, char *argument)
{

    char arg1[MAX_INPUT_LENGTH];	/* what command */
    char arg2[MAX_INPUT_LENGTH];	/* what clan */
    char arg3[MAX_INPUT_LENGTH];	/* argument */
    char arg4[MAX_INPUT_LENGTH];	/* argument 2 */

    char buffer[MAX_INPUT_LENGTH];	/* temporary */

    /* See what the user wants */

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = ci_one_argument(argument, arg4);

    if (arg1[0] == '\0') {
	/* Self-documenting commands are a *GOOD* thing! */
	send_to_char("clanctl usage:\n\r", ch);
	send_to_char
	    ("clanctl <command> <clan short name> <arg> <arg2>\n\r", ch);
	send_to_char("commands are:\n\r", ch);
	send_to_char("add - Add a new clan\n\r", ch);
	send_to_char("del - Delete a clan\n\r", ch);
	send_to_char("flag - Toggle a clan flag\n\r", ch);
	send_to_char("set - Set a clan variable\n\r", ch);
	send_to_char("reload - Reload the clan table from disk\n\r", ch);
	send_to_char("save - Save the clan table to disk\n\r", ch);
	send_to_char("join - Join a player to a clan\n\r", ch);
	send_to_char("declan - Drop a player from a clan\n\r", ch);
	send_to_char("info - get clan info\n\r", ch);
	send_to_char("eq - edit clan eq table\n\r", ch);
	send_to_char("rtxt - edit rank text table\n\r", ch);
	send_to_char("skill - edit clan skill table\n\r", ch);
	send_to_char("establish - Set a clan as established\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "add")) {
	/* ADD CLAN */

	if (strlen(arg2) == 0) {
	    send_to_char("Clan name needed.\n\r", ch);
	    return;
	}

	if (add_clan(arg2) != 0) {
	    send_to_char("Clan successfully added\n\r", ch);
	    sprintf(buffer, "CLAN %s ADDED BY %s", arg2, ch->name);
	    log_string(buffer);
	    return;
	} else {
	    send_to_char("Out of clans, or other lossage.\n\r", ch);
	    return;
	}
    }

    if (!str_prefix(arg1, "del")) {
	/* DELETE CLAN */

	if (strlen(arg2) == 0) {
	    send_to_char("Clan name needed.\n\r", ch);
	    return;
	}

	if (del_clan(arg2) != 0) {
	    send_to_char("Clan successfully deleted\n\r", ch);
	    sprintf(buffer, "CLAN %s DELETED BY %s", arg2, ch->name);
	    log_string(buffer);
	    return;
	} else {
	    send_to_char("Clan doesn't exist, or other lossage.\n\r", ch);
	    return;
	}
    }

    if (!str_prefix(arg1, "flag")) {
	int x;

	/* FLAG CLAN */

	if (strlen(arg3) == 0) {
	    send_to_char("Possible flags are: nonpk rp\n\r", ch);
	    return;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\n\r", ch);
	    return;
	}

	x = lookup_clan(arg2);
	if (x == 0) {
	    send_to_char("Clan doesn't exist.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "nonpk")) {
	    clantable[x]->flags = clantable[x]->flags ^ CF_NONPK;
	    send_to_char("NONPK CLAN bit toggled.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "rp")) {
	    clantable[x]->flags = clantable[x]->flags ^ CF_RP;
	    send_to_char("RP CLAN bit toggled.\n\r", ch);
	    return;
	}

	send_to_char("Unknown flag! Type \"clanctl flag\" for help.\n\r",
		     ch);
	return;
    }

    if (!str_prefix(arg1, "set")) {
	/* SET CLAN VAR */
	int x;

	if (strlen(arg3) == 0) {
	    send_to_char
		("Vars are: longname minlevel leader droom infotxt hall\r\n",
		 ch);
	    return;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\r\n", ch);
	    return;
	}

	x = lookup_clan(arg2);
	if (x == 0) {
	    send_to_char("No such clan\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "longname")) {
	    if (clantable[x]->lname != NULL) {
		free(clantable[x]->lname);
	    }
	    clantable[x]->lname = strdup(arg4);
	    send_to_char("OK.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "minlevel")) {
	    clantable[x]->minlevel = atoi(arg4);
	    send_to_char("OK.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "leader")) {
	    if (clantable[x]->leader != NULL) {
		free(clantable[x]->leader);
	    }
	    clantable[x]->leader = strdup(arg4);
	    send_to_char("OK.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "droom")) {
	    clantable[x]->droom = atoi(arg4);
	    send_to_char("OK.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "hall")) {
	    clantable[x]->hall = atoi(arg4);
	    send_to_char("OK.\n\r", ch);
	    return;
	}

	if (!str_prefix(arg3, "infotxt")) {
	    if (strcmp(arg4, "+") == 0 && clantable[x]->infotext != NULL) {
		sprintf(buffer, "%s%s\n\r", clantable[x]->infotext,
			argument);
		free(clantable[x]->infotext);
		clantable[x]->infotext = strdup(buffer);
		send_to_char("OK.\n\r", ch);
		return;
	    } else {
		if (clantable[x] != NULL) {
		    free(clantable[x]->infotext);
		}
		sprintf(buffer, "%s %s\n\r", arg4, argument);	/* Avoid parser stupidity */
		clantable[x]->infotext = strdup(buffer);
		send_to_char("OK.\n\r", ch);
		return;
	    }
	}

	send_to_char
	    ("Unknown parameter - type \"clanctl set\" for a list\n\r",
	     ch);
	return;
    }

    if (!str_prefix(arg1, "reload")) {
	/* RELOAD CLANS */
	load_clan_table();
	update_player_clandata();
	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "save")) {
	/* SAVE CLANS */
	save_clan_table();
	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "join")) {
	/* JOIN CLAN */
	int x;
	CHAR_DATA *victim;

	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\n\r", ch);
	    return;
	}
	if (strlen(arg3) == 0) {
	    send_to_char("Need char name!\n\r", ch);
	    return;
	}

	if ((victim = get_char_world(ch, arg3)) == NULL) {
	    send_to_char("That person doesn't exist.\n\r", ch);
	    return;
	}

	/* Check for stupidities */

	if (IS_NPC(victim)) {
	    send_to_char("Clanning mobs is not allowed.\n\r", ch);
	    return;
	}

	x = clan_char(arg2, victim->name);

	if (x >= 0) {
	    send_to_char("OK.\n\r", ch);
	    return;
	}
	if (x == -1) {
	    send_to_char("No such clan\n\r", ch);
	    return;
	}
	if (x == -2) {
	    send_to_char("Too many clannies already!\n\r", ch);
	    return;
	}
	if (x == -3) {
	    send_to_char("Clannie must be PKset\n\r", ch);
	    return;
	}

	send_to_char
	    ("Unknown return from clan_char() - NOTIFY SUZURAN\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "declan")) {
	/* DECLAN */
	int x;
	CHAR_DATA *victim;

	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\n\r", ch);
	    return;
	}
	if (strlen(arg3) == 0) {
	    send_to_char("Need char name!\n\r", ch);
	    return;
	}

	if ((victim = get_char_world(ch, arg3)) == NULL) {
	    send_to_char("That person doesn't exist.\n\r", ch);
	    return;
	}

	if (is_clannie(victim->name) == 0) {
	    send_to_char("That person is not in a clan.\n\r", ch);
	    return;
	}

	x = declan_char(arg2, victim->name);

	if (x < 0) {
	    send_to_char("Error - No such clan, or target not in clan\n\r",
			 ch);
	}

	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "update")) {
	update_player_clandata();
	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "info")) {
	int x = 0, y = 0;

	/* CLAN INFO */
	if (strlen(arg2) == 0) {
	    /* LIST CLANS */
	    x = 0;
	    send_to_char("CLAN NAME/LONG NAME/STATUS\r\n", ch);
	    while (x < MAX_CLANS) {
		if (clantable[x] != NULL) {
		    sprintf(buffer, " %s / %s / ", clantable[x]->name,
			    clantable[x]->lname);
		    if (IS_SET(clantable[x]->flags, CF_ESTABLISHED)) {
			strcat(buffer, "ESTABLISHED\n\r");
		    } else {
			strcat(buffer, "PENDING\n\r");
		    }
		    send_to_char(buffer, ch);
		}
		x++;
	    }
	} else {
	    x = lookup_clan(arg2);
	    if (x == 0 || !(clantable[x]->flags & CF_ESTABLISHED)) {
		send_to_char("Nonexistent clan.\n\r", ch);
		return;
	    }

	    sprintf(buffer, "Clan name: %s - Long Name: %s\r\n",
		    clantable[x]->name, clantable[x]->lname);
	    send_to_char(buffer, ch);
	    sprintf(buffer, "Min level %d - Leader: %s\r\n",
		    clantable[x]->minlevel, clantable[x]->leader);
	    send_to_char(buffer, ch);
	    /*	    sprintf(buffer, "Cost %d to pledge\r\n",
		    clantable[x]->cost_to_join); */
	    send_to_char(buffer, ch);
	    sprintf(buffer, "Death room vnum %d, clan recall room %d\r\n",
		    clantable[x]->droom, clantable[x]->hall);
	    send_to_char(buffer, ch);
	    if (clantable[x]->flags & CF_NONPK) {
		send_to_char("This is a NON-PK clan\r\n", ch);
	    }
	    if (clantable[x]->flags & CF_RP) {
		send_to_char("This is a RP clan\r\n", ch);
	    }
	    sprintf(buffer, "Clan info: %s\r\n", clantable[x]->infotext);
	    page_to_char(buffer, ch);

	    send_to_char("Members:\r\n", ch);
	    y = 0;
	    while (y < MAX_CLAN_MEMBER) {
		if (clantable[x]->member[y] != 0) {
		    sprintf(buffer, "%s\r\n",
			    clantable[x]->member[y]->name);
		    send_to_char(buffer, ch);
		}
		y++;
	    }
	    y = 0;
	    send_to_char("Clan skills:\r\n", ch);
	    while (y < MAX_CSKILL_NUMBER) {
		if (clantable[x]->skill[y] != 0) {
		    sprintf(buffer, "%s\r\n",
			    clantable[x]->skill[y]->name);
		    send_to_char(buffer, ch);
		}
		y++;
	    }
	    y = 0;
	    send_to_char("Clan eq:\r\n", ch);
	    while (y < MAX_CLAN_EQ) {
		if (clantable[x]->eq[y] != 0) {
		    sprintf(buffer, "Name: %s - Vnum: %d\r\n",
			    clantable[x]->eq[y]->name,
			    clantable[x]->eq[y]->vnum);
		    send_to_char(buffer, ch);
		}
		y++;
	    }
	    return;
	}
	return;
    }

    if (!str_prefix(arg1, "eq")) {
	int x, snum, eq_vnum;

	if (strlen(arg3) == 0) {
	    send_to_char("Need slot number\r\n", ch);
	    return;
	}
	snum = atoi(arg3);

	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\r\n", ch);
	    return;
	}
	if (strlen(arg4) == 0) {
	    send_to_char
		("Usage: clanctl eq <clan> <number> <vnum> <eq name>\n\r",
		 ch);
	    return;
	}
	eq_vnum = atoi(arg4);

	if (strlen(argument) == 0) {
	    send_to_char("Need name for eq!\n\r", ch);
	    return;
	}

	x = lookup_clan(arg2);
	if (x == 0) {
	    send_to_char("No such clan\n\r", ch);
	    return;
	}

	if (snum > MAX_CLAN_EQ) {
	    send_to_char("EQ number too high\n\r", ch);
	    return;
	}

	if (clantable[x]->eq[snum] == NULL) {
	    clantable[x]->eq[snum] = calloc(16, 8);
	} else {
	    free(clantable[x]->eq[snum]);
	    clantable[x]->eq[snum] = calloc(16, 8);
	}

	clantable[x]->eq[snum]->name = strdup(argument);
	clantable[x]->eq[snum]->vnum = eq_vnum;
	send_to_char("OK.\n\r", ch);
	return;

    }

    if (!str_prefix(arg1, "skill")) {
	int x, snum;

	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\r\n", ch);
	    return;
	}

	if (strlen(arg3) == 0) {
	    send_to_char("Need slot number\r\n", ch);
	    return;
	}
	snum = atoi(arg3);

	if (snum > MAX_CSKILL_NUMBER) {
	    send_to_char("Number too high.\n\r", ch);
	    return;
	}

	if (strlen(arg4) == 0) {
	    send_to_char
		("Usage: clanctl skill <clan> <number> <skill name>\n\r",
		 ch);
	    return;
	}

	x = lookup_clan(arg2);
	if (x == 0) {
	    send_to_char("No such clan\n\r", ch);
	    return;
	}

	if (clantable[x]->skill[snum] != NULL) {
	    free(clantable[x]->skill[snum]);
	}
	clantable[x]->skill[snum] = calloc(512, 8);

	clantable[x]->skill[snum]->name = strdup(arg4);
	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "rtxt")) {
	int x, snum;

	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\r\n", ch);
	    return;
	}

	if (strlen(arg3) == 0) {
	    send_to_char("Need slot number\r\n", ch);
	    return;
	}
	snum = atoi(arg3);

	if (snum > MAX_CLAN_RANKS) {
	    send_to_char("Slot too high.\n\r", ch);
	    return;
	}

	if (strlen(arg4) == 0) {
	    send_to_char
		("Usage: clanctl rtxt <clan> <number> <rank name>\n\r",
		 ch); return;
	}

	x = lookup_clan(arg2);
	if (x == 0) {
	    send_to_char("No such clan\n\r", ch);
	    return;
	}

	if (clantable[x]->ranktxt[snum] != NULL) {
	    free(clantable[x]->ranktxt[snum]);
	}

	clantable[x]->ranktxt[snum] = strdup(arg4);
	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "establish")) {
	int x = 0;

	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\r\n", ch);
	    return;
	}

	x = lookup_clan(arg2);
	if (x == 0) {
	    send_to_char("No such clan\n\r", ch);
	    return;
	}

	/* Set validated flag */
	clantable[x]->flags |= CF_ESTABLISHED;

	/* Transfer ldrship */
	clantable[x]->leader = strdup(clantable[x]->initiator);

	/* Clan the leader */
	clan_char(clantable[x]->name, clantable[x]->leader);

	/* Save and update */
	save_clan_table();
	update_player_clandata();

	send_to_char("OK.\n\r", ch);
	return;
    }

    send_to_char("Unknown command - type \"clanctl\" for help.\n\r", ch);
    return;
}

int add_clan(char *clan_name)
{

    CLAN_DATA *clandata;
    int x = 1, y = 0;

    /* Return clan number, or 0 if I fail. */

    while (x < MAX_CLANS) {
	if (clantable[x] == NULL) {
	    break;
	}
	x++;
    }

    if (x == MAX_CLANS) {
	return (0);
    }

    /* ROM's memory manager kept doing stupid things with my data structs,
       so I use UNIX's instead. */

    /* The number 8192 here is a buffer size.  If you crash by overflowing
       the memory allocated for a clan (you can tell by bizarre crap getting
       written into the clan table, and other clan-related stupidity) raise
       this number.  If you make it too high, there's no ill effects other
       than the memory wastage.  I think. */

    clantable[x] = calloc(sizeof(clandata) + 8192, 8);

    /* Initialize EVERYTHING to sane values! */
    /* If you add stuff to the clan table, make SURE you initialize it
       here and free it below when clans get deleted! */

    clantable[x]->name = strdup(clan_name);
    clantable[x]->lname = strdup(" ");
    clantable[x]->flags = 0;
    clantable[x]->minlevel = 0;
    clantable[x]->cost_to_join = 100;
    clantable[x]->leader = strdup(" ");
    clantable[x]->hall = ROOM_VNUM_TEMPLE;
    clantable[x]->droom = ROOM_VNUM_ALTAR;
    clantable[x]->initiator = strdup(" ");

    y = 0;
    while (y < MAX_CLAN_EQ) {
	clantable[x]->eq[y] = NULL;
	y++;
    }

    y = 0;
    while (y < MAX_CLAN_RANKS) {
	clantable[x]->ranktxt[y] = NULL;
	y++;
    }

    y = 0;
    while (y < MAX_CLAN_MEMBER) {
	clantable[x]->member[y] = NULL;
	y++;
    }

    y = 0;
    while (y < MAX_CSKILL_NUMBER) {
	clantable[x]->skill[y] = NULL;
	y++;
    }

    y = 0;
    while (y < MAX_PLEDGE) {
	clantable[x]->pledge[y] = NULL;
	y++;
    }

    clantable[x]->infotext = strdup(" ");
    save_clan_table();
    return (x);
}

int del_clan(char *clan_name)
{

    int x, y;
    x = lookup_clan(clan_name);
    if (x != 0) {
	/* Clean up substructs */
	y = 0;
	while (y < MAX_CLAN_EQ) {
	    if (clantable[x]->eq[y] != NULL) {
		if (clantable[x]->eq[y]->name != NULL) {
		    free(clantable[x]->eq[y]->name);
		}
		free(clantable[x]->eq[y]);
	    }
	    y++;
	}
	y = 0;
	while (y < MAX_CLAN_RANKS) {
	    if (clantable[x]->ranktxt[y] != NULL) {
		free(clantable[x]->ranktxt[y]);
	    }
	    y++;
	}
	y = 0;
	while (y < MAX_CLAN_MEMBER) {
	    if (clantable[x]->member[y] != NULL) {
		free(clantable[x]->member[y]->name);
		free(clantable[x]->member[y]);
	    }
	    y++;
	}
	y = 0;
	while (y < MAX_CSKILL_NUMBER) {
	    if (clantable[x]->skill[y] != NULL) {
		free(clantable[x]->skill[y]->name);
		free(clantable[x]->skill[y]);
	    }
	    y++;
	}
	y = 0;
	while (y < MAX_PLEDGE) {
	    if (clantable[x]->pledge[y] != NULL) {
		free(clantable[x]->pledge[y]->name);
		free(clantable[x]->pledge[y]);
	    }
	    y++;
	}

	free(clantable[x]->name);
	free(clantable[x]->lname);
	free(clantable[x]->leader);
	free(clantable[x]->infotext);
	free(clantable[x]->initiator);

	free(clantable[x]);

	clantable[x] = NULL;

	save_clan_table();
	update_player_clandata();
	return (1);
    }

    return (0);
}

int lookup_clan(char *clan_name)
{

    int x = 0;

    /* This makes the compiler be strict about sequencing these
       if's.  Some optimizers will do silly things involving checking
       all the conditions at the same time, and since each is dependent
       on the first... *KABOOM* */

    while (x < (MAX_CLANS + 1)) {
	if (clantable[x] != NULL) {
	    if (clantable[x]->name != NULL) {
		if (strcasecmp(clantable[x]->name, clan_name) == 0) {
		    return (x);
		}
	    }
	}
	x++;
    }
    return (0);
}

int clan_char(char *clan_name, char *name)
{

    CLAN_MEMBER_DATA *memberdata;

    int x = 0, y = 0;

    x = lookup_clan(clan_name);
    if (x == 0) {
	return (-1);
    }

    y = 0;

    while (y <= MAX_CLAN_MEMBER) {
	y++;
	if (clantable[x]->member[y] == NULL) {
	    break;
	}
    }

    /* There used to be checks here, but that's now done by "clan pledge". */

    /* y points at clan member slot */

    clantable[x]->member[y] = calloc(sizeof(memberdata) + 1536, 8);

    clantable[x]->member[y]->name = strdup(name);
    clantable[x]->member[y]->rank = 0;
    clantable[x]->member[y]->flags = 0;

    save_clan_table();
    update_player_clandata();
    return (y);
}

int declan_char(char *clan_name, char *name)
{

    int x = 0, y = 0;

    x = lookup_clan(clan_name);

    if (x > MAX_CLANS) {
	bug("declan_char: STUPID CLAN NUMBER", 0);
	return 0;
    }

    if (x == 0) {
	return (-1);
    }

    y = 0;

    y = clannie_lookup(x, name);
    if (y > MAX_CLAN_MEMBER) {
	bug("declan_char: STUPID CLANNIE NUMBER", 0);
	return 0;
    }

    if (y == -1) {
	return (-1);
    }

    free(clantable[x]->member[y]->name);
    free(clantable[x]->member[y]);
    clantable[x]->member[y] = NULL;

    save_clan_table();
    update_player_clandata();
    return (0);
}

bool is_clan(CHAR_DATA * ch)
{
    if (ch->clan == 0) {
	return FALSE;
    } else {
	return TRUE;
    }
}

bool is_same_clan(CHAR_DATA * ch, CHAR_DATA * victim)
{
    if (!(is_clan(ch))) {
	return FALSE;
    } else {
	return (ch->clan == victim->clan);
    }
}

void do_guild(CHAR_DATA * ch, char *argument)
{

    char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int x;
    CHAR_DATA *victim;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!is_leader(ch) && !IS_IMP(ch)) {
	send_to_char("This command is for clan leaders only.\n\r", ch);
	return;
    }

    if (strlen(arg1) == 0 || strlen(arg2) == 0) {
	send_to_char("Usage: guild <char> <clan name>\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("That person doesn't exist.\n\r", ch);
	return;
    }

    /* Check for stupidities */

    if (IS_NPC(victim)) {
	send_to_char("Clanning mobs is not allowed.\n\r", ch);
	return;
    }

    /* These restrictions are from old clan code */

    if (IS_ADMIN(ch) && !IS_IMP(ch) && (victim != ch)) {
	send_to_char("You may only (un)guild yourself.\n\r", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("Whew! No guilding while charmed.\n\r", ch);
	return;
    }

    /* Declanning - also old code */
    if (!str_prefix(arg2, "none")) {
	if (!IS_ADMIN(ch) && (victim == ch)) {
	    send_to_char("You can't unguild yourself.\n\r", ch);
	    return;
	} else if (!IS_IMP(ch) && (ch->clan != victim->clan)) {
	    send_to_char("You can't unguild someone not in your clan!\n\r",
			 ch);
	    return;
	} else if (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim)) {
	    send_to_char("You can't unguild an immortal.\n\r", ch);
	    return;
	}

	if (!is_clannie(victim->name)) {
	    send_to_char("They are not in a clan.\n\r", ch);
	    return;
	}

	send_to_char("They are now clanless.\n\r", ch);
	send_to_char("You are no longer a member of a clan!\n\r", victim);

	/* REMOVE CLAN EQ/SKILLS HERE */

	x = 0;
	while (x < MAX_CSKILL_NUMBER) {
	    x++;
	    if (clantable[victim->clan]->skill[x] != NULL) {
		if (clantable[victim->clan]->skill[x]->name != NULL) {
		    group_remove(victim,
				 clantable[victim->clan]->skill[x]->name);
		}
	    }
	}

	sprintf(buf, "CSKILL: taken from %s.", victim->name);
	wiznet(buf, ch, NULL, WIZ_FLAGS, 0, get_trust(ch));

	/* Grab clan eq - From old code, with grammar corrected ^_^ */

	if (victim != ch) {
	    while ((obj = get_obj_list(ch, "-claneq-", victim->carrying))
		   != NULL) {

		if (obj->wear_loc != WEAR_NONE)
		    unequip_char(victim, obj);

		obj_from_char(obj);
		obj_to_char(obj, ch);

		sprintf(buf, "You seize %s from %s.\r\n", obj->short_descr,
			victim->name);

		send_to_char(buf, ch);
		sprintf(buf,
			"Abruptly, %s is taken from your possession.\r\n",
			obj->short_descr);
		send_to_char(buf, victim);
		sprintf(buf, "CLAN: %s seizes %s from %s.", ch->name,
			obj->short_descr, victim->name);
		wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
	    }
	}

	/* All done. */

	declan_char(clantable[victim->clan]->name, arg2);
	victim->clan = 0;
	return;
    }

    /* Not on existing clannies */

    if (is_clannie(victim->name)) {
	send_to_char("That person is already in someone else's clan!\n\r",
		     ch);
	return;
    }

    /* Also... */

    if (strcmp(arg2, clantable[ch->clan]->name) != 0) {
	send_to_char("Can't clan someone into someone else's clan!\n\r",
		     ch);
	return;
    }

    x = clan_char(arg2, victim->name);

    if (x >= 0) {
	/* THIS CODE IS ONLY RUN ON A SUCCESSFUL CLANNING */

	/* Apparently there are bonuses for getting clanned... This is
	   from old code.  Adjust if you don't like this. 

	   I didn't like it so I removed it - Suzuran */

	/*
	   victim->max_hit = victim->max_hit + 100;
	   victim->max_mana = victim->max_mana + 100;
	   victim->max_move = victim->max_move + 100;

	   victim->pcdata->perm_hit = victim->pcdata->perm_hit + 100;
	   victim->pcdata->perm_mana = victim->pcdata->perm_mana + 100;
	   victim->pcdata->perm_move = victim->pcdata->perm_move + 100;
	 */

	/* Notify them */

	sprintf(buf, "They are now a member of clan %s.\n\r",
		capitalize(clantable[victim->clan]->name));
	send_to_char(buf, ch);
	sprintf(buf, "You are now a member of clan %s.\n\r",
		capitalize(clantable[victim->clan]->name));
	send_to_char(buf, victim);

	/* All done */
	return;
    }
    if (x == -1) {
	send_to_char("No such clan\n\r", ch);
	return;
    }
    if (x == -2) {
	send_to_char("Too many clannies already!\n\r", ch);
	return;
    }
    if (x == -3) {
	send_to_char("Clannie must be PKset\n\r", ch);
	return;
    }

    sprintf(buf,
	    "Unknown return (%d) from clan_char() - NOTE SUZURAN!\n\r", x);
    send_to_char(buf, ch);
    return;
}

void do_clan(CHAR_DATA * ch, char *argument)
{

    /* clan <command> <params>

       Global Commands:     in_charge     list  initiate    info
       Clannie-only commands: recall who talk members deposit balance
     */

    char arg1[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int x = 0;

    argument = one_argument(argument, arg1);

    if (arg1[0] == 0) {
	if (is_clannie(ch->name)) {
	    send_to_char
		("Syntax: clan recall | who | talk | in_charge | list | members | info | deposit | balance\n\r",
		 ch);
	} else {
	    send_to_char
		("Syntax: clan in_charge | list | initiate | info\n\r",
		 ch);
	}
	return;
    }

    /* Sanitize everything */
    buf1[0] = 0;
    buf2[0] = 0;
    buf3[0] = 0;
    x = 0;

    if (!str_prefix(arg1, "in_charge")) {
	send_to_char("Clan name - Clan leader\n\r", ch);
	while (x < MAX_CLANS) {
	    x++;
	    if (clantable[x] != NULL && clantable[x]->leader != NULL
		&& clantable[x]->name != NULL
		&& clantable[x]->flags & CF_ESTABLISHED) {
		sprintf(buf1, "%s - %s\n\r", clantable[x]->name,
			clantable[x]->leader);
		send_to_char(buf1, ch);
	    }
	}
	return;
    }

    if (!str_prefix(arg1, "list")) {
	send_to_char("CLAN LIST\n\r", ch);
	while (x < MAX_CLANS) {
	    x++;
	    if (clantable[x] != NULL && clantable[x]->lname != NULL
		&& clantable[x]->flags & CF_ESTABLISHED) {
		sprintf(buf1, "%s\n\r", clantable[x]->lname);
		send_to_char(buf1, ch);
	    }
	}
	return;
    }

    if (!str_prefix(arg1, "initiate")) {
	/* RT clan initiate */
	int pm1 = 0, pm2 = 0, rppts_reqd = 0, x = 0;

	char arg2[MAX_STRING_LENGTH];
	char arg3[MAX_STRING_LENGTH];
	char arg4[MAX_STRING_LENGTH];
	char arg5[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	argument = one_argument(argument, arg4);
	argument = one_argument(argument, arg5);

	/* Punt on invalidity */
	if (is_clannie(ch->name)) {
	    send_to_char("You're already in a clan!\n\r", ch);
	    return;
	}

	if (is_initiator(ch)) {
	    send_to_char("You already initiated another clan.\n\r", ch);
	    return;
	}

	if (strlen(arg4) == 0) {
	    send_to_char
		("Usage: clan initiate <shortname> <pm1> <pm2> [nonpk]\n\r",
		 ch);
	    return;
	}

	if (clan_lookup(arg2) != 0) {
	    send_to_char("Shortname already in use.\n\r", ch);
	    return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
	    send_to_char("Not while charmed.\n\r", ch);
	    return;
	}

	/* Check payment and do it to it */
	pm1 = atoi(arg3);
	pm2 = atoi(arg4);
	if (strcasecmp(arg5, "nonpk") == 0) {
	    rppts_reqd = 160;
	} else {
	    rppts_reqd = 80;
	}

	/* Batanen found a feature! */
	if (rppts_reqd == 160 && ch->pcdata->pkset == TRUE) {
	    send_to_char("PKs can't start nonPK clans.\n\r", ch);
	    return;
	}
	if (rppts_reqd == 80 && ch->pcdata->pkset == FALSE) {
	    send_to_char("NonPKs can't start PK clans.\n\r", ch);
	    return;
	}

	if (pm1 > 3 || pm1 < 1) {
	    send_to_char("Illegal pm1.\n\r", ch);
	    return;
	}
	if (pm2 > 3 || pm2 < 1) {
	    send_to_char("Illegal pm2.\n\r", ch);
	    return;
	}
	if (pm1 == pm2) {
	    send_to_char("You must specify 2 different PMs.\n\r", ch);
	    return;
	}

	if (rppts_reqd == 160 && pm1 != 3 && pm2 != 3) {
	    send_to_char("NonPK clans MUST use RP points as a PM.\n\r",
			 ch);
	    return;
	}

	if ((pm1 == 1 || pm2 == 1) && ch->bank < 250000) {
	    send_to_char("You don't have enough gold in the bank.\n\r",
			 ch);
	    return;
	}

	if ((pm1 == 2 || pm2 == 2) && ch->questpoints < 3000) {
	    send_to_char("You don't have enough autoquest points.\n\r",
			 ch);
	    return;
	}

	if ((pm1 == 3 || pm2 == 3) && ch->rp_points < rppts_reqd) {
	    send_to_char("You don't have enough RP points.\n\r", ch);
	    return;
	}

	/* Do 'em */

	if ((pm1 == 1 || pm2 == 1)) {
	    ch->bank -= 250000;
	}

	if ((pm1 == 2 || pm2 == 2)) {
	    ch->questpoints -= 3000;
	}

	if ((pm1 == 3 || pm2 == 3)) {
	    ch->rp_points -= rppts_reqd;
	}

	/* Now create the clan. */
	x = add_clan(arg2);

	/* Set clan status */
	if (x < 1) {
	    send_to_char
		("CLAN ADD FAILED - NOTE SUZURAN WITH TIME/DATE.\n\r", ch);
	    if ((pm1 == 1 || pm2 == 1)) {
		ch->bank += 250000;
	    }
	    if ((pm1 == 2 || pm2 == 2)) {
		ch->questpoints += 3000;
	    }
	    if ((pm1 == 3 || pm2 == 3)) {
		ch->rp_points += rppts_reqd;
	    }
	    return;
	}
	if (rppts_reqd == 160) {
	    clantable[x]->flags |= CF_NONPK;
	}
	free(clantable[x]->initiator);
	clantable[x]->initiator = strdup(ch->name);
	send_to_char("Done!\n\r", ch);
	return;
    }

    /*    if (!str_prefix(arg1, "pledge")) {
	int x = 0, y = 0;

	char arg2[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg2);

	if (is_clannie(ch->name)) {
	    send_to_char("You're already in a clan!\n\r", ch);
	    return;
	}

	if (is_initiator(ch)) {
	    send_to_char("You already initiated another clan.\n\r", ch);
	    return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
	    send_to_char("Not while charmed.\n\r", ch);
	    return;
	}

	if (strlen(arg2) == 0) {
	    send_to_char("Need clan name!\n\r", ch);
	    return;
	}

	x = lookup_clan(arg2);

	if (x < 1) {
	    send_to_char("That clan doesn't exist! (Or other failure)\n\r",
			 ch);
	    return;
	}

	if ((clantable[x]->flags & CF_NONPK) && ch->pcdata->pkset != FALSE) {
	    send_to_char("PK people can't join non-PK clans.\n\r", ch);
	    return;
	}
	if ((clantable[x]->flags & CF_NONPK) == 0
	    && ch->pcdata->pkset != TRUE) {
	    send_to_char("NonPK people can't join PK clans.\n\r", ch);
	    return;
	}

	if (ch->level < clantable[x]->minlevel) {
	    send_to_char("You're not a high enough level.\n\r", ch);
	    return;
	}

	if (ch->bank < clantable[x]->cost_to_join) {
	    send_to_char
		("You don't have enough money in the bank to pledge.\n\r",
		 ch);
	    return;
	}

	if (is_acl_entry(ch->name, x) && clantable[x]->pledge_policy == 0) {
	    send_to_char("That clan has denied your pledge.\n\r", ch);
	    return;
	}
	if (!is_acl_entry(ch->name, x) && clantable[x]->pledge_policy == 1) {
	    send_to_char("That clan has denied your pledge.\n\r", ch);
	    return;
	}


	y = new_pledge_entry(ch->name, x);

	if (y == -1) {
	    send_to_char("Operand error.\n\r", ch);
	    return;
	}
	if (y == -2) {
	    send_to_char("Out of pledge table entries.\n\r", ch);
	    return;
	}

	clantable[x]->pledge[y]->flags |= CPF_PLEDGE;

	ch->bank = ch->bank - clantable[x]->cost_to_join;

	send_to_char("OK.", ch);
	return;
	} */


    if (!str_prefix(arg1, "info")) {
	int x = 0;
	/* int y = 0; */
	char buffer[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg2);

	/* CLAN INFO */
	if (strlen(arg2) == 0) {
	    /* LIST CLANS */
	    x = 0;
	    send_to_char("CLAN NAME/LONG NAME\r\n", ch);
	    while (x < MAX_CLANS) {
		if (clantable[x] != NULL) {
		    if (IS_SET(clantable[x]->flags, CF_ESTABLISHED)) {
			sprintf(buffer, " %s / %s\n\r", clantable[x]->name,
				clantable[x]->lname);
			send_to_char(buffer, ch);
		    }
		}
		x++;
	    }
	} else {
	    x = lookup_clan(arg2);
	    if (x == 0 || !(clantable[x]->flags & CF_ESTABLISHED)) {
		send_to_char("Nonexistent clan.\n\r", ch);
		return;
	    }

	    sprintf(buffer, "Clan name: %s - Long Name: %s\r\n",
		    clantable[x]->name, clantable[x]->lname);
	    send_to_char(buffer, ch);
	    sprintf(buffer, "Min level %d - Leader: %s\r\n",
		    clantable[x]->minlevel, clantable[x]->leader);
	    send_to_char(buffer, ch);
	    /*	    sprintf(buffer, "Cost %d to pledge\r\n",
		    clantable[x]->cost_to_join); */
	    send_to_char(buffer, ch);
	    if (clantable[x]->flags & CF_NONPK) {
		send_to_char("This is a NON-PK clan\r\n", ch);
	    }
	    if (clantable[x]->flags & CF_RP) {
		send_to_char("This is a RP clan\r\n", ch);
	    }
	    sprintf(buffer, "Clan info: %s\r\n", clantable[x]->infotext);
	    page_to_char(buffer, ch);

	    /* send_to_char("Clan skills:\r\n", ch);
	       while (y < MAX_CSKILL_NUMBER) {
	       if (clantable[x]->skill[y] != 0) {
	       sprintf(buffer, "%s\r\n",
	       clantable[x]->skill[y]->name);
	       send_to_char(buffer, ch);
	       }
	       y++;
	       }
	       y = 0;
	       send_to_char("Clan eq:\r\n", ch);
	       while (y < MAX_CLAN_EQ) {
	       if (clantable[x]->eq[y] != 0) {
	       sprintf(buffer, "Name: %s - Vnum: %d\r\n",
	       clantable[x]->eq[y]->name,
	       clantable[x]->eq[y]->vnum);
	       send_to_char(buffer, ch);
	       }
	       y++;
	       } */
	    return;
	}
	return;
    }


    if (!is_clannie(ch->name)) {
	send_to_char("Type \"clan\" for usage.\n\r", ch);
	return;
    }

    /* ALL COMMANDS PAST THIS LINE ARE FOR CLANNIES ONLY */

    if (!str_prefix(arg1, "recall")) {
	do_recall(ch, "clan");
	return;
    }

    if (!str_prefix(arg1, "who")) {
	send_to_char("Clan status:\n\r", ch);

	/* SEND: questbuf, incog, wizi, Ranktxt, AFK, KILLER, THIEF, name, ??? */

	/* Loop thru active descriptors */
	for (d = descriptor_list; d; d = d->next) {
	    if (d->connected == CON_PLAYING && d->character != NULL
		&& d->character->pcdata != NULL) {
		/* Char playing.  Same clan? */
		if (is_same_clan(ch, d->character)) {
		    /* Same clan.  Gen a string, and return it. */
		    /* Buf1 is the final output line, buf2 is temp */

		    /* clan rank */
		    if (clantable[d->character->clan]->ranktxt
			[get_crank(d->character)] == NULL) {
			sprintf(buf2, "[# %d] ", get_crank(d->character));
		    } else {
			sprintf(buf2, "[%s] ",
				clantable[d->character->clan]->
				ranktxt[get_crank(d->character)]);
		    }
		    strcat(buf1, buf2);
		    /* sex */
		    if (d->character->sex == SEX_NEUTRAL) {
			strcat(buf1, "{N} ");
		    }
		    if (d->character->sex == SEX_MALE) {
			strcat(buf1, "{M} ");
		    }
		    if (d->character->sex == SEX_FEMALE) {
			strcat(buf1, "{F} ");
		    }
		    /* Sneak, hide, invis */
		    if (IS_AFFECTED(d->character, AFF_SNEAK)) {
			strcat(buf1, "(S|");
		    } else {
			strcat(buf1, "( |");
		    }
		    if (IS_AFFECTED(d->character, AFF_HIDE)) {
			strcat(buf1, "H|");
		    } else {
			strcat(buf1, " |");
		    }
		    if (IS_AFFECTED(d->character, AFF_INVISIBLE)) {
			strcat(buf1, "I) ");
		    } else {
			strcat(buf1, " ) ");
		    }
		    /* name, and done */
		    sprintf(buf2, "%s\r\n",
			    capitalize(d->character->name));
		    strcat(buf1, buf2);
		    send_to_char(buf1, ch);
		    buf1[0] = 0;

		}
	    }
	}
	/* Done */
	return;
    }

    if (!str_prefix(arg1, "members")) {
	int y = 0;

	send_to_char("Clan Members:\r\n", ch);
	while (y < MAX_CLAN_MEMBER) {
	    if (clantable[ch->clan]->member[y] != 0) {
		sprintf(buf1, "%s\r\n",
			clantable[ch->clan]->member[y]->name);
		send_to_char(buf1, ch);
	    }
	    y++;
	}
	return;
    }

    if (!str_prefix(arg1, "talk")) {
	do_clantalk(ch, argument);
	return;
    }

    if (!str_prefix(arg1, "deposit")) {
	char arg2[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	long x = 0;

	if (strlen(arg2) == 0) {
	    send_to_char("Amount needed\n\r", ch);
	    return;
	}
	x = atoi(arg2);
	if (ch->gold < x) {
	    send_to_char("You don't have that much.\n\r", ch);
	    return;
	}
	if (x == 0) {
	    send_to_char("Cheapskate!\n\r", ch);
	    return;
	}

	clantable[ch->clan]->bank_balance += x;
	ch->gold -= x;

	/* Tattle */
	sprintf(buffer, "%s deposited %ld to clan account.\n\r", ch->name,
		x);
	note_to_char(clantable[ch->clan]->leader, buffer);

	return;
    }

    if (!str_prefix(arg1, "balance")) {
	char buffer[MAX_STRING_LENGTH];
	sprintf(buffer, "Clan bank balance is %ld\n\r",
		clantable[ch->clan]->bank_balance);
	send_to_char(buffer, ch);
	return;
    }

    send_to_char("Huh?\n\r", ch);
    return;
}

void do_cload(CHAR_DATA * ch, char *argument)
{

    /* Load clan eq */
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    int x = 0;

    one_argument(argument, arg1);

    pObjIndex = 0;

    if (!is_granted_name(ch, "cload")) {
	send_to_char("Huh?\n\r", ch);
	return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("Whew, No loading while charmed.\n\r", ch);
	return;
    }
    if (arg1[0] == '\0') {
	send_to_char("Syntax:  cload <victim>\n\r", ch);
	return;
    }

    if (IS_ADMIN(ch)) {
	victim = get_char_world(ch, arg1);
    } else {
	victim = get_char_room(ch, arg1);
    }

    if (victim == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    if ((victim->clan != ch->clan) && !IS_IMP(ch)) {
	send_to_char("ACK! You can't give a non-member clan eq!\n\r", ch);
	return;
    }
    if (!victim->clan) {
	sprintf(buf, "%s is a member of no clan.\n\r", victim->name);
	send_to_char(buf, ch);
	return;
    }
    if ((victim == ch) && IS_SET(ch->act, PLR_LEADER)) {
	pObjIndex = get_obj_index(12105);
	if (pObjIndex == NULL) {
	    send_to_char("Error making light.\n\r", ch);
	    return;
	}
	obj = create_object(pObjIndex, 0);
	obj_to_char(obj, ch);
	act("You have brought into existence $p.", ch, obj, NULL, TO_CHAR);
	act("$n has brought into existence $p.", ch, obj, NULL, TO_ROOM);
    }

    /* Here's the major change. */

    while (x < MAX_CLAN_EQ) {
	x++;
	if (clantable[victim->clan] != NULL) {
	    if (clantable[victim->clan]->eq[x] != NULL) {
		if (clantable[victim->clan]->eq[x]->name != NULL) {
		    pObjIndex =
			get_obj_index(clantable[victim->clan]->
				      eq[x]->vnum);
		    if (pObjIndex != NULL) {
			if ((obj = create_object(pObjIndex, 0)) != NULL) {
			    obj_to_char(obj, victim);
			    sprintf(buf, "CLOAD: $N to %s.", victim->name);
			    wiznet(buf, ch, NULL, WIZ_FLAGS, 0,
				   get_trust(ch));

			    if (victim != ch) {
				act("You give $p to $N.", ch, obj, victim,
				    TO_CHAR);
				act("$n gives you $p.", ch, obj, victim,
				    TO_VICT);
				act("$n gives $p to $N.", ch, obj, victim,
				    TO_NOTVICT);
			    } else {
				act("You give yourself $p.", ch, obj, NULL,
				    TO_CHAR);
				act("$n gives $mself $p.", ch, obj, victim,
				    TO_NOTVICT);
				sprintf(buf, "CLOAD: $N to %s.",
					victim->name);
				wiznet(buf, ch, NULL, WIZ_FLAGS, 0,
				       get_trust(ch));
			    }
			}
		    }
		}
	    }
	}
    }
    return;
}

void do_cskill(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int sn, x = 0;

    one_argument(argument, arg1);

    if (IS_NPC(ch))
	return;

    if (!is_granted_name(ch, "cskill")) {
	send_to_char("Huh?\n\r", ch);
	return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("You cannot use this command while charmed.\n\r", ch);
	return;
    }
    if (arg1[0] == '\0') {
	send_to_char("Syntax:  cskill <victim> grant\n\r", ch);
	return;
    }
    if ((victim = get_char_room(ch, arg1)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    if ((victim->clan != ch->clan) && get_trust(ch) < MAX_LEVEL) {
	send_to_char("ACK! They are not even in a clan!!!\n\r", ch);
	return;
    }

    while (x < MAX_CSKILL_NUMBER) {
	x++;
	if (clantable[victim->clan] != NULL) {
	    if (clantable[victim->clan]->skill[x] != NULL) {
		if (clantable[victim->clan]->skill[x]->name != NULL) {
		    group_add(victim,
			      clantable[victim->clan]->skill[x]->name,
			      FALSE);
		    sn =
			skill_lookup(clantable[victim->clan]->
				     skill[x]->name);
		    if (sn != 0) {
			victim->pcdata->learned[sn] = 100;
		    }
		}
	    }
	}
    }
    sprintf(buf, "CSKILL: $N to %s.", victim->name);
    wiznet(buf, ch, NULL, WIZ_FLAGS, 0, get_trust(ch));
    return;
}

void do_promote(CHAR_DATA * ch, char *argument)
{

    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int arg2int;
    CHAR_DATA *victim;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    arg2int = atoi(arg2);

    if (IS_NPC(ch)) {
	send_to_char
	    ("You Can't Do That On Television.  (Or as an NPC)\n\r", ch);
	return;
    }

    /* I don't have to worry about existing rank as it doesn't have
       anything to do with leadership */

    if ((arg1[0] == '\0') || (arg2[0] == '\0') || (arg2int < 0)
	|| (arg2int > MAX_CLAN_RANKS)) {
	send_to_char("Syntax: promote <char> <rank>\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg1)) == NULL) {
	send_to_char("They must be present to be promoted.\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char
	    ("Do not pass GO, do not collect $200, do not advance NPCs.\n\r",
	     ch);
	return;
    }

    if (victim->clan < 1) {
	send_to_char("They cannot be promoted.\n\r", ch);
	return;
    }

    /* Additional idiot check */

    if (!IS_IMMORTAL(ch) && !is_same_clan(ch, victim)) {
	send_to_char("Can't promote someone in someone else's clan!\n\r",
		     ch);
	return;
    }

    if (victim == ch) {
	if ((IS_IMMORTAL(ch)) || (get_trust(ch) >= 95)) {

	    /* Set clan rank */
	    set_crank(victim, arg2int);
	    send_to_char("Rank ceremony complete.\n\r", ch);
	    return;
	}
	send_to_char("You cannot promote yourself.\n\r", ch);
	return;
    }

    if ((IS_IMMORTAL(ch)) && (ch->level < 99) && (get_trust(ch) < 99)) {
	send_to_char("You can only promote yourself.\n\r", ch);
	return;
    }

    if ((ch->level < 99) && (get_trust(ch) < 99)) {
	if (ch->clan != victim->clan) {
	    send_to_char("They are not in your clan.\n\r", ch);
	    return;
	}
	if (get_crank(ch) <= arg2int) {
	    send_to_char("You cannot promote over your rank.\n\r", ch);
	    return;
	}
	if (get_crank(ch) < get_crank(victim)) {
	    send_to_char("You cannot promote someone higher than you.\n\r",
			 ch);
	    return;
	}

	set_crank(victim, arg2int);
	send_to_char("Rank ceremony complete.\n\r", ch);
	send_to_char("Rank ceremony complete.\n\r", victim);
	return;
    }

    if ((arg2int > MAX_CLAN_RANKS) && (!IS_IMMORTAL(victim))) {
	send_to_char("They are not immortal!\n\r", ch);
	return;
    }

    if (((arg2int > 9) && (!IS_SET(victim->act, PLR_LEADER)))
	&& (!IS_IMMORTAL(victim))) {
	send_to_char("They are not qualified to lead.\n\r", ch);
	return;
    }

    set_crank(victim, arg2int);

    send_to_char("Rank ceremony complete.\n\r", ch);
    send_to_char("Rank ceremony complete.\n\r", victim);
    return;
}

sh_int get_clan_hall(CHAR_DATA * ch)
{
    int y;

    /* Return vnum of luser's clan hall */
    y = ch->clan;
    if (y == 0) {
	return (ROOM_VNUM_TEMPLE);
    }

    return (clantable[y]->hall);
}

sh_int get_clan_droom(CHAR_DATA * ch)
{
    int y;

    /* Return vnum of luser's clan respawn point */
    y = ch->clan;
    if (y == 0) {
	return (ROOM_VNUM_ALTAR);
    }

    return (clantable[y]->droom);
}

bool clan_can_see(CHAR_DATA * ch, CHAR_DATA * victim)
{

    if (ch == victim)
	return TRUE;

    if (get_trust(ch) < victim->invis_level)
	return FALSE;

    if (get_trust(ch) < victim->incog_level
	&& ch->in_room != victim->in_room) return FALSE;

    return TRUE;
}

int clan_lookup(const char *name)
{
    int clan;

    for (clan = 0; clan < MAX_CLANS; clan++) {
	if (clantable[clan] != NULL && clantable[clan]->name != NULL) {
	    if (LOWER(name[0]) == LOWER(clantable[clan]->name[0])
		&& !str_prefix(name, clantable[clan]->name))
		return clan;
	}
    }

    return 0;
}

char *get_clan_longname(int clan)
{

    if (clan == 0) {		/* Punt! */
	return ("");
    }

    if (clan < 0 || clan > MAX_CLANS) {
	bug("ILLEGAL VALUE PASSED TO get_clan_longname", 0);
	return (" ");
    }

    if (clantable[clan] != NULL) {
	if (clantable[clan]->lname != NULL) {
	    return (clantable[clan]->lname);
	}
    }
    /* No clan? */
    return ("");

}

char *get_clan_name(int clan)
{

    /* Punt! */
    if (clan == 0) {
	return ("");
    }

    if (clan < 0 || clan > MAX_CLANS) {
	bug("get_clan_name: Stupid clan number", 0);
	return (" ");
    }

    if (clantable[clan] != NULL) {
	if (clantable[clan]->name != NULL) {
	    return (clantable[clan]->name);
	}
    }
    /* No clan? */
    bug("get_clan_name: No such clan!", 0);
    return (" ");

}

int clannie_lookup(int clan, char *name)
{

    int x = 0;

    if (clan < 0 || clan > MAX_CLANS) {
	return (-1);
    }

    if (clantable[clan] == NULL) {
	return (-1);
    }

    while (x <= MAX_CLAN_MEMBER) {
	if (clantable[clan]->member[x] != NULL) {
	    if (clantable[clan]->member[x]->name != NULL) {
		if (strcasecmp(clantable[clan]->member[x]->name, name) ==
		    0) {
		    break;
		}
	    }
	}
	x++;
    }

    if (x == MAX_CLAN_MEMBER) {
	return (-1);
    }

    /* Otherwise... */
    return (x);
}

char *clan_fread_string(FILE * fp)
{

    char buffer[MAX_STRING_LENGTH];
    int x = 0;
    char tmp = 0;
    int trim = 1;

    /* Trim leading spaces */

    while (x < MAX_STRING_LENGTH) {
	tmp = fgetc(fp);
	if (!(trim == 1 && tmp == ' ')) {
	    trim = 0;
	    if (tmp == '~') {
		buffer[x] = 0;
		fread_to_eol(fp);
		break;
	    }
	    buffer[x] = tmp;
	    x++;
	}
    }

    return (strdup(buffer));
}

int clan_fread_number(FILE * fp)
{

    char buffer[MAX_STRING_LENGTH];

    fgets(buffer, MAX_STRING_LENGTH, fp);
    return (atoi(buffer));
}

/* Messy macro */

#define KEY( literal, field, value )                            \
                                if ( !str_cmp( word, literal ) )        \
                                { \
			          field  = value; \
                                  fMatch = TRUE; \
                                break;}


void load_clan_table()
{

    FILE *fp;
    int x = 0, y = 0, snum = 0, eqnum = 0, rnum = 0, cpl = 0;
    bool fMatch;
    char *word;
    CLAN_DATA *clandata;

    if (!(fp = fopen("clantable.txt", "r"))) {
	bug("Can't read clantable.txt!", 0);
	return;
    }

    if (feof(fp)) {
	return;
    }

    x = 0;

    for (;;) {

	word = feof(fp) ? "End" : fread_word(fp);
	fMatch = FALSE;

	switch (UPPER(word[0])) {

	case '*':
	    fMatch = TRUE;
	    fread_to_eol(fp);
	    break;

	case '#':
	    fMatch = TRUE;
	    fread_to_eol(fp);
	    break;

	case 'B':
	    KEY("Bank", clantable[x]->bank_balance, clan_fread_number(fp));
	    break;

	case 'C':
	    if (!strcmp(word, "Cnxt")) {
		fread_to_eol(fp);

		/* Bump x, Alloc new clan, reset y. */
		x++;
		if (clantable[x] != NULL && clantable[x]->name != NULL) {
		    del_clan(clantable[x]->name);
		}
		clantable[x] = calloc(sizeof(clandata) + 8192, 8);

		clantable[x]->name = strdup(" ");
		clantable[x]->lname = strdup(" ");
		clantable[x]->flags = 0;
		clantable[x]->minlevel = 0;
		clantable[x]->cost_to_join = 100;
		clantable[x]->leader = strdup(" ");
		clantable[x]->hall = ROOM_VNUM_TEMPLE;
		clantable[x]->droom = ROOM_VNUM_ALTAR;
		clantable[x]->initiator = strdup(" ");

		y = 0;
		while (y < MAX_CLAN_EQ) {
		    clantable[x]->eq[y] = NULL;
		    y++;
		}

		y = 0;
		while (y < MAX_CLAN_RANKS) {
		    clantable[x]->ranktxt[y] = NULL;
		    y++;
		}

		y = 0;
		while (y < MAX_CLAN_MEMBER) {
		    clantable[x]->member[y] = NULL;
		    y++;
		}

		y = 0;
		while (y < MAX_CSKILL_NUMBER) {
		    clantable[x]->skill[y] = NULL;
		    y++;
		}

		y = 0;
		while (y < MAX_PLEDGE) {
		    clantable[x]->pledge[y] = NULL;
		    y++;
		}

		clantable[x]->infotext = strdup(" ");

		y = 0; rnum=0; snum = 0, eqnum = 0, rnum = 0, cpl = 0;
		fMatch = TRUE;
	    }
	    KEY("Ctj", clantable[x]->cost_to_join, clan_fread_number(fp));
	    break;

	case 'D':
	    KEY("Drm", clantable[x]->droom, clan_fread_number(fp));
	    break;

	case 'E':
	    KEY("Enam", clantable[x]->eq[eqnum]->name,
		clan_fread_string(fp));
	    KEY("Evnm", clantable[x]->eq[eqnum]->vnum,
		clan_fread_number(fp));
	    if (!strcmp(word, "Enxt")) {
		fread_to_eol(fp);
		fMatch = TRUE;
		eqnum++;
		clantable[x]->eq[eqnum] = calloc(16, 8);
	    }
	    if (!strcmp(word, "End")) {
		log_string("CLAN LOAD DONE");
		return;
	    }
	    break;

	case 'F':
	    KEY("Flgs", clantable[x]->flags, clan_fread_number(fp));
	    break;

	case 'H':
	    KEY("Hall", clantable[x]->hall, clan_fread_number(fp));
	    break;

	case 'I':
	    KEY("Info", clantable[x]->infotext, clan_fread_string(fp));
	    KEY("Intr", clantable[x]->initiator, clan_fread_string(fp));
	    break;

	case 'L':
	    KEY("Lnam", clantable[x]->lname, clan_fread_string(fp));
	    KEY("Ldr", clantable[x]->leader, clan_fread_string(fp));
	    break;

	case 'M':
	    KEY("Mnlv", clantable[x]->minlevel, clan_fread_number(fp));
	    KEY("Mnam", clantable[x]->member[y]->name,
		clan_fread_string(fp));
	    KEY("Mrnk", clantable[x]->member[y]->rank,
		clan_fread_number(fp));
	    KEY("Mflg", clantable[x]->member[y]->flags,
		clan_fread_number(fp));
	    if (!strcmp(word, "Mnxt")) {
		CLAN_MEMBER_DATA *memberdata;

		fread_to_eol(fp);
		/* Bump y, alloc new member data */
		y++;
		clantable[x]->member[y] =
		    calloc(sizeof(memberdata) + 1024, 8);
		fMatch = TRUE;
	    }
	    break;

	case 'N':
	    KEY("Name", clantable[x]->name, clan_fread_string(fp));
	    break;

	case 'R':
	    if (!strcmp(word, "Rnxt")) {
		fread_to_eol(fp);
		fMatch = TRUE;
		rnum++;
		clantable[x]->ranktxt[rnum] = calloc(MAX_STRING_LENGTH, 8);
	    }
	    KEY("Rtxt", clantable[x]->ranktxt[rnum],
		clan_fread_string(fp));
	    break;

	case 'S':
	    KEY("Snam", clantable[x]->skill[snum]->name,
		clan_fread_string(fp));
	    if (!strcmp(word, "Snxt")) {
		fread_to_eol(fp);
		fMatch = TRUE;
		/* Bump snum, alloc new skill */
		snum++;
		clantable[x]->skill[snum] = calloc(8, 8);
	    }
	    break;

	case 'P':
	    KEY("Pnam", clantable[x]->pledge[cpl]->name,
		clan_fread_string(fp));
	    KEY("Pcst", clantable[x]->pledge[cpl]->pcost,
		clan_fread_number(fp));
	    KEY("Pflg", clantable[x]->pledge[cpl]->flags,
		clan_fread_number(fp));
	    if (!strcmp(word, "Pnxt")) {
		CLAN_PLEDGE_DATA *pledgedata;

		fread_to_eol(fp);
		/* Bump cpl, alloc new pledge data */
		cpl++;
		clantable[x]->pledge[cpl] =
		    calloc(sizeof(pledgedata) + 512, 8);
		fMatch = TRUE;
	    }
	    break;

	}

	if (!fMatch) {
	    bug("load_clan_table: no match.", 0);
	    fread_to_eol(fp);
	}
    }


    return;
}

void save_clan_table()
{

    FILE *file;
    int x = 0;
    int y = 0;

    if (!(file = fopen("clantable.txt", "w"))) {
	bug("save_clan_table: can't open clantable.txt", 0);
	return;
    }

    fprintf(file, "#CLAN\n");

    while (x < MAX_CLANS) {
	if (clantable[x] != NULL) {
	    fprintf(file, "Cnxt 0\n");
	    smash_tilde(clantable[x]->name);
	    fprintf(file, "Name %s~\n", clantable[x]->name);
	    smash_tilde(clantable[x]->lname);
	    fprintf(file, "Lnam %s~\n", clantable[x]->lname);
	    fprintf(file, "Flgs %d\n", clantable[x]->flags);
	    fprintf(file, "Mnlv %d\n", clantable[x]->minlevel);
	    smash_tilde(clantable[x]->leader);
	    fprintf(file, "Ldr  %s~\n", clantable[x]->leader);
	    fprintf(file, "Intr %s~\n", clantable[x]->initiator);
	    smash_tilde(clantable[x]->infotext);
	    fprintf(file, "Info %s~\n", clantable[x]->infotext);
	    fprintf(file, "Hall %d\n", clantable[x]->hall);
	    fprintf(file, "Drm  %d\n", clantable[x]->droom);
	    fprintf(file, "Bank %ld\n", clantable[x]->bank_balance);
	    fprintf(file, "Ctj  %d\n", clantable[x]->cost_to_join);

	    /* Now dump members */
	    y = 0;
	    while (y < MAX_CLAN_MEMBER) {
		y++;
		if (clantable[x]->member[y] != NULL) {
		    if (clantable[x]->member[y]->name != NULL) {
			fprintf(file, "Mnxt 0\n");
			fprintf(file, "Mnam %s~\n",
				clantable[x]->member[y]->name);
			fprintf(file, "Mrnk %d\n",
				clantable[x]->member[y]->rank);
			fprintf(file, "Mflg %d\n",
				clantable[x]->member[y]->flags);
		    }
		}
	    }
	    /* Now dump pledges */
	    y = 0;
	    while (y < MAX_PLEDGE) {
		y++;
		if (clantable[x]->pledge[y] != NULL) {
		    if (clantable[x]->pledge[y]->name != NULL) {
			fprintf(file, "Pnxt 0\n");
			fprintf(file, "Pnam %s~\n",
				clantable[x]->pledge[y]->name);
			fprintf(file, "Pcst %d\n",
				clantable[x]->pledge[y]->pcost);
			fprintf(file, "Pflg %d\n",
				clantable[x]->pledge[y]->flags);
		    }
		}
	    }
	    /* Clan skills */
	    y = 0;
	    while (y < MAX_CSKILL_NUMBER) {
		y++;
		if (clantable[x]->skill[y] != NULL) {
		    if (clantable[x]->skill[y]->name != NULL) {
			fprintf(file, "Snxt 0\n");
			fprintf(file, "Snam %s~\n",
				clantable[x]->skill[y]->name);
		    }
		}
	    }
	    /* Clan EQ */
	    y = 0;
	    while (y < MAX_CLAN_EQ) {
		y++;
		if (clantable[x]->eq[y] != NULL) {
		    if (clantable[x]->eq[y]->name != NULL) {
			fprintf(file, "Enxt 0\n");
			fprintf(file, "Enam %s~\n",
				clantable[x]->eq[y]->name);
			fprintf(file, "Evnm %d\n",
				clantable[x]->eq[y]->vnum);
		    }
		}
	    }
	    y = 0;
	    while (y < MAX_CLAN_RANKS) {
		y++;
		if (clantable[x]->ranktxt[y] != NULL) {
		    fprintf(file, "Rnxt 0\n");
		    fprintf(file, "Rtxt %s~\n", clantable[x]->ranktxt[y]);
		}
	    }
	}
	x++;
    }

    fprintf(file, "End\n");
    fclose(file);
    return;
}

int is_clannie(char *name)
{

    /* Return clan number if name is the name of a clannie */

    int x = 0, y = 0;

    while (x < MAX_CLANS) {
	x++;
	y = 0;
	if (clantable[x] != NULL) {
	    while (y < MAX_CLAN_MEMBER) {
		y++;
		if (clantable[x]->member[y] != NULL) {
		    if (clantable[x]->member[y]->name != NULL) {
			if (strcasecmp(name, clantable[x]->member[y]->name)
			    == 0) {
			    return (x);
			}
		    }
		}
	    }
	}
    }
    return (0);
}

int is_leader(CHAR_DATA * ch)
{

    /* Return a 1 if char is leader of their clan */

    if (clantable[ch->clan] != NULL) {
	if (clantable[ch->clan]->leader != NULL) {
	    if (strcasecmp(clantable[ch->clan]->leader, ch->name) == 0) {
		return (1);
	    }
	}
    }
    return (0);

}

int is_initiator(CHAR_DATA * ch)
{

    int x = 0;

    /* Return x if char is initiator of a clan */
    /* ch->clan will be zero so we have to run on the assumption that
       they are the initiator of only one clan (this will be
       code-enforced */
    while (x < MAX_CLANS) {
	x++;
	if (clantable[x] != NULL) {
	    if (clantable[x]->initiator != NULL) {
		if (strcasecmp(clantable[x]->initiator, ch->name) == 0) {
		    return (x);
		}
	    }
	}
    }
    return (0);
}

void update_player_clandata()
{

    DESCRIPTOR_DATA *d;

    /* Update the clan data in the player data structs */

    /* NOTE - WITH A LARGE AMOUNT OF LOGGED IN PLAYERS, THIS CAN
       BE AN EXPENSIVE OPERATION (MEANING IT CAN TAKE A LOT OF TIME)
       IT PERFORMS A CLAN LOOKUP ON ALL LOGGED IN CHARS, AND A LEADER
       CHECK */

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING) {
	    /* Update clan member status */
	    d->character->clan = is_clannie(d->character->name);
	    /* Update clan leader status */
	    if (is_leader(d->character)) {
		SET_BIT(d->character->act, PLR_LEADER);
	    } else {
		REMOVE_BIT(d->character->act, PLR_LEADER);
	    }
	}
    }

    return;
}

int get_crank(CHAR_DATA * ch)
{

    /* Not that kind of crank! (Fscking drug addicts...) */

    int x = 0, y = 0;

    while (x < MAX_CLANS) {
	x++;
	y = 0;
	if (clantable[x] != NULL) {
	    while (y < MAX_CLAN_MEMBER) {
		y++;
		if (clantable[x]->member[y] != NULL) {
		    if (clantable[x]->member[y]->name != NULL) {
			if (strcasecmp
			    (ch->name, clantable[x]->member[y]->name) == 0) {
			    return (clantable[x]->member[y]->rank);
			}
		    }
		}
	    }
	}
    }
    return (0);
}

void set_crank(CHAR_DATA * ch, int newrank)
{

    /* Set clan rank */

    int x = 0, y = 0;
    if (newrank > MAX_CLAN_RANKS) {
	newrank = MAX_CLAN_RANKS;
    }

    while (x < MAX_CLANS) {
	x++;
	y = 0;
	if (clantable[x] != NULL) {
	    while (y < MAX_CLAN_MEMBER) {
		y++;
		if (clantable[x]->member[y] != NULL) {
		    if (clantable[x]->member[y]->name != NULL) {
			if (strcasecmp
			    (ch->name, clantable[x]->member[y]->name) == 0) {
			    clantable[x]->member[y]->rank = newrank;
			    save_clan_table();
			    return;
			}
		    }
		}
	    }
	}
    }
    return;
}

/* RT clan leader command */

void do_clanldr(CHAR_DATA * ch, char *argument)
{

    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    char buffer[MAX_INPUT_LENGTH];

    /* See what the luser wants. */

    argument = one_argument(argument, arg1);
    argument = ci_one_argument(argument, arg2);

    /* Bail if user is not allowed to use this command */
    if (!is_initiator(ch) && !is_leader(ch) && !IS_IMP(ch)) {
	send_to_char("Foo, you are a charlatan!\n\r", ch);
	return;
    }

    if (arg1[0] == '\0') {
	send_to_char("Syntax: clanldr <command> <arguments> ...\n\r", ch);
	send_to_char("Commands are:\n\r", ch);
	send_to_char("clan     -    Clan a pledge.\n\r", ch);
	send_to_char("declan   -    Declan someone.\n\r", ch);
	send_to_char("plist    -    Pledge list/status.\n\r", ch);
	send_to_char("pdel     -    Drop a pledge.\n\r", ch);
	send_to_char("info     -    Clan status information.\n\r", ch);
	send_to_char("rtxt     -    Edit rank text table.\n\r", ch);
	/* send_to_char("pcset    -    Pledge cost set.\n\r", ch); */
	send_to_char("lname    -    Set clan longname.\n\r", ch);
	send_to_char("minlvl   -    Set minimum level to join.\n\r", ch);
	send_to_char("infotxt  -    Edit the \"clan info\" text.\n\r", ch);
	send_to_char("cflag    -    Toggle clan member flags.\n\r", ch);
	send_to_char("addacl   -    Add an ACL entry.\n\r", ch);
	send_to_char("ppdef    -    Toggle pledge policy default.\n\r",
		     ch);
	send_to_char("sctj     -    Set cost-to-join\n\r", ch);
	send_to_char("promote  -    Set clannie's rank\n\r", ch);
	send_to_char
	    ("clan,declan,and cflag are only available to established clans.\n\r",
	     ch);
	send_to_char
	    ("lname and minlvl are only available before establishment.\n\r",
	     ch);
	return;
    }

    if (!str_prefix(arg1, "clan")) {
	/* CLAN CHAR */
	int x/*, y */;

	/* This second error is missing the period for a reason. */
	if (!is_clannie(ch->name)) {
	    send_to_char("Only for established clans.\n\r", ch);
	    return;
	}
	if (ch->clan == 0) {
	    send_to_char("Only for established clans\n\r", ch);
	    return;
	}

	if (strlen(arg2) == 0) {
	    send_to_char("Need char name!\n\r", ch);
	    return;
	}

	/* Make sure that char is on the pledge list */
	/* Fuck the pledge list!  031302SGM */
	/*	y = pledge_lookup(arg2, ch->clan);
	if (y == 0) {
	    send_to_char("That char hasn't pledged to your clan.\n\r", ch);
	    return;
	    } */

	x = clan_char(clantable[ch->clan]->name, arg2);

	if (x >= 0) {
	    send_to_char("OK.\n\r", ch);
	    /* Delete pledge entry */
	    /*	    free(clantable[ch->clan]->pledge[y]->name);
	    free(clantable[ch->clan]->pledge[y]);
	    clantable[ch->clan]->pledge[y] = NULL; */
	    return;
	}
	if (x == -1) {
	    send_to_char("No such clan\n\r", ch);
	    return;
	}
	if (x == -2) {
	    send_to_char("Too many clannies already!\n\r", ch);
	    return;
	}
	if (x == -3) {
	    send_to_char("Clannie must be PKset\n\r", ch);
	    return;
	}

	send_to_char
	    ("Unknown return from clan_char() - NOTIFY SUZURAN\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "declan")) {
	/* DECLAN */
	int x;

	if (ch->clan == 0 && !is_clannie(ch->name)) {
	    send_to_char("For established clans only.\n\r", ch);
	    return;
	}

	if (strlen(arg2) == 0) {
	    send_to_char("Need char name!\n\r", ch);
	    return;
	}

	if (is_clannie(arg2) != ch->clan) {
	    send_to_char("That person isn't in your clan.\n\r", ch);
	    return;
	}

	x = declan_char(clantable[ch->clan]->name, arg2);

	if (x < 0) {
	    send_to_char("Error - No such clan, or target not in clan\n\r",
			 ch);
	}

	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "plist")) {
	int x = 0, y = 0;

	send_to_char("Pledge list/status\n\r", ch);
	if (ch->clan == 0) {
	    x = is_initiator(ch);
	} else {
	    x = ch->clan;
	}
	while (y < MAX_PLEDGE) {
	    if (clantable[x]->pledge[y] != NULL
		&& clantable[x]->pledge[y]->name != NULL) {
		sprintf(buffer, "Name: %s - Pcost %d\n\r",
			clantable[x]->pledge[y]->name,
			clantable[x]->pledge[y]->pcost);
		send_to_char(buffer, ch);
		if ((clantable[x]->pledge[y]->flags & CPF_PLEDGE) == 0) {
		    send_to_char
			("This is an ACL entry and not a pledge.\n\r", ch);
		}
	    }
	    y++;
	}
	return;
    }

    if (!str_prefix(arg1, "pdel")) {
	int x = 0, y = 0;

	if (ch->clan == 0) {
	    x = is_initiator(ch);
	} else {
	    x = ch->clan;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need pledge name.\n\r", ch);
	    return;
	}

	y = pledge_acl_lookup(arg2, x);

	if (y == 0) {
	    send_to_char("Pledge not found.\n\r", ch);
	} else {
	    if (clantable[x]->pledge[y] != NULL) {
		free(clantable[x]->pledge[y]->name);
		free(clantable[x]->pledge[y]);
		clantable[x]->pledge[y] = NULL;
		send_to_char("OK.", ch);
	    }
	}
	return;
    }

    if (!str_prefix(arg1, "info")) {
	int x = 0, y = 0;

	/* CLAN INFO */
	if (ch->clan == 0) {
	    x = is_initiator(ch);
	} else {
	    x = ch->clan;
	}
	if (x == 0) {
	    send_to_char("Nonexistent clan.\n\r", ch);
	    return;
	}
	if (clantable[x] == NULL) {
	    send_to_char("Fatal error.\n\r", ch);
	    return;
	}

	sprintf(buffer, "Clan name: %s - Long Name: %s\r\n",
		clantable[x]->name, clantable[x]->lname);
	send_to_char(buffer, ch);
	sprintf(buffer, "Min level %d - Leader: %s\r\n",
		clantable[x]->minlevel, clantable[x]->leader);
	send_to_char(buffer, ch);
	sprintf(buffer, "Death room vnum %d, clan recall room %d\r\n",
		clantable[x]->droom, clantable[x]->hall);
	send_to_char(buffer, ch);
	if (clantable[x]->flags & CF_NONPK) {
	    send_to_char("This is a NON-PK clan\r\n", ch);
	}
	if (clantable[x]->flags & CF_RP) {
	    send_to_char("This is a RP clan\r\n", ch);
	}
	sprintf(buffer, "Clan info: %s\r\n", clantable[x]->infotext);
	page_to_char(buffer, ch);

	send_to_char("Members:\r\n", ch);
	y = 0;
	while (y < MAX_CLAN_MEMBER) {
	    if (clantable[x]->member[y] != 0) {
		sprintf(buffer, "%s\r\n", clantable[x]->member[y]->name);
		send_to_char(buffer, ch);
	    }
	    y++;
	}
	y = 0;
	send_to_char("Clan skills:\r\n", ch);
	while (y < MAX_CSKILL_NUMBER) {
	    if (clantable[x]->skill[y] != 0) {
		sprintf(buffer, "%s\r\n", clantable[x]->skill[y]->name);
		send_to_char(buffer, ch);
	    }
	    y++;
	}
	y = 0;
	send_to_char("Clan eq:\r\n", ch);
	while (y < MAX_CLAN_EQ) {
	    if (clantable[x]->eq[y] != 0) {
		sprintf(buffer, "Name: %s - Vnum: %d\r\n",
			clantable[x]->eq[y]->name,
			clantable[x]->eq[y]->vnum);
		send_to_char(buffer, ch);
	    }
	    y++;
	}
	return;
    }

    if (!str_prefix(arg1, "rtxt")) {
	int x, snum;

	argument = ci_one_argument(argument, arg3);

	if (strlen(arg3) == 0) {
	    send_to_char("Usage: clanldr rtxt <number> <rank name>\n\r",
			 ch);
	    return;
	}

	snum = atoi(arg2);

	if (snum > MAX_CLAN_RANKS || snum < 0) {
	    send_to_char("Bad slot number.\n\r", ch);
	    return;
	}

	if (ch->clan == 0) {
	    x = is_initiator(ch);
	} else {
	    x = ch->clan;
	}
	if (x == 0) {
	    send_to_char("No such clan\n\r", ch);
	    return;
	}

	if (clantable[x]->ranktxt[snum] != NULL) {
	    free(clantable[x]->ranktxt[snum]);
	}

	clantable[x]->ranktxt[snum] = strdup(arg3);
	send_to_char("OK.\n\r", ch);
	return;
    }

/*    if (!str_prefix(arg1, "pcset")) {
	int x = 0, y = 0;

    argument = ci_one_argument(argument, arg3);

	if (ch->clan == 0) {
	    send_to_char("Finalized clans only.\n\r", ch);
	    return;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need pledge name.\n\r", ch);
	    return;
	}
	if (strlen(arg3) == 0) {
	    send_to_char("Need a new pcost.\n\r", ch);
	    return;
	}
	x = ch->clan;

	y = pledge_lookup(arg2, x);

	if (y == 0) {
	    send_to_char("Pledge not found.\n\r", ch);
	} else {
	    if (clantable[x]->pledge[y] != NULL) {
		clantable[x]->pledge[y]->pcost = atoi(arg3); */
    /* Silently nail possible cheaters. ^_^ */
/*		if (clantable[x]->pledge[y]->pcost < 0) {
		    clantable[x]->pledge[y]->pcost = 0;
		}
		send_to_char("OK.", ch);
	    }
	}
	return; 

    } */
    if (!str_prefix(arg1, "lname")) {
	int x = 0;
	if (ch->clan != 0) {
	    send_to_char("Not for established clans.\n\r", ch);
	    return;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need new lname.\n\r", ch);
	    return;
	}
	x = is_initiator(ch);
	if (x == 0) {
	    send_to_char("Oops.", ch);
	    return;
	}
	if (clantable[x]->lname != NULL) {
	    free(clantable[x]->lname);
	}
	clantable[x]->lname = strdup(arg2);
	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "minlvl")) {
	int x = 0;
	if (ch->clan != 0) {
	    send_to_char("Not for established clans.\n\r", ch);
	    return;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need new minlevel.\n\r", ch);
	    return;
	}
	x = is_initiator(ch);
	if (x == 0) {
	    send_to_char("Oops.", ch);
	    return;
	}
	clantable[x]->minlevel = atoi(arg2);
	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "sctj")) {
	int x = 0, y = 0;
	if (ch->clan != 0) {
	    send_to_char("Not for established clans.\n\r", ch);
	    return;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need new cost.\n\r", ch);
	    return;
	}
	x = is_initiator(ch);
	if (x == 0) {
	    send_to_char("Oops.", ch);
	    return;
	}
	y = atoi(arg2);
	if (y < 100) {
	    y = 100;
	}
	if (y > 10000) {
	    y = 10000;
	}
	clantable[x]->cost_to_join = y;
	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "infotxt")) {
	int x = 0;
	if (ch->clan != 0) {
	    send_to_char("Not for established clans.\n\r", ch);
	    return;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need new string.\n\r", ch);
	    return;
	}
	x = is_initiator(ch);
	if (x == 0) {
	    send_to_char("Oops.", ch);
	    return;
	}

	if (strcmp(arg2, "+") == 0 && clantable[x]->infotext != NULL) {
	    sprintf(buffer, "%s%s\n\r", clantable[x]->infotext, argument);
	    free(clantable[x]->infotext);
	    clantable[x]->infotext = strdup(buffer);
	    send_to_char("OK.\n\r", ch);
	    return;
	} else {
	    if (clantable[x] != NULL) {
		free(clantable[x]->infotext);
	    }
	    sprintf(buffer, "%s %s\n\r", arg2, argument);	/* Avoid parser stupidity */
	    clantable[x]->infotext = strdup(buffer);
	    send_to_char("OK.\n\r", ch);
	    return;
	}
    }

    if (!str_prefix(arg1, "cflag")) {
	int x;
	/* FLAG CLANNIE */

	argument = ci_one_argument(argument, arg3);

	if (ch->clan == 0 && !is_clannie(ch->name)) {
	    send_to_char("For established clans only.\n\r", ch);
	    return;
	}

	if (strlen(arg3) == 0) {
	    send_to_char("Possible flags are: coldr\n\r", ch);
	    return;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need clannie name!\n\r", ch);
	    return;
	}

	x = is_clannie(arg2);
	if (x != ch->clan) {
	    send_to_char("That person isn't in your clan.\n\r", ch);
	    return;
	}

	x = clannie_lookup(ch->clan, arg2);

	if (!str_prefix(arg3, "coldr")) {
	    clantable[ch->clan]->member[x]->flags =
		clantable[ch->clan]->member[x]->flags ^ PF_COLDR;
	    send_to_char("CO-LEADER bit toggled.\n\r", ch);
	    return;
	}

	send_to_char("Unknown flag! Type \"clanldr flag\" for help.\n\r",
		     ch);
	return;
    }

    if (!str_prefix(arg1, "ppdef")) {
	int x = 0;
	if (ch->clan == 0) {
	    x = is_initiator(ch);
	} else {
	    x = ch->clan;
	}
	if (clantable[x]->pledge_policy == 0) {
	    clantable[x]->pledge_policy = 1;
	    send_to_char("Pledges now rejected by default.\n\r", ch);
	} else {
	    clantable[x]->pledge_policy = 0;
	    send_to_char("Pledges now accepted by default.\n\r", ch);
	}
	return;
    }

    if (!str_prefix(arg1, "addacl")) {
	int x = 0, y = 0;
	if (ch->clan == 0) {
	    x = is_initiator(ch);
	} else {
	    x = ch->clan;
	}
	if (strlen(arg2) == 0) {
	    send_to_char("Need player name.\n\r", ch);
	    return;
	}
	y = new_pledge_entry(arg2, x);

	if (y == -1) {
	    send_to_char("Operand error.\n\r", ch);
	    return;
	}
	if (y == -2) {
	    send_to_char("Out of pledge table entries.\n\r", ch);
	    return;
	}

	send_to_char("OK.\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "promote")) {
	int x = 0, y = 0;
	/* PROMOTE CLANNIE */

	argument = ci_one_argument(argument, arg3);

	if (ch->clan == 0 && !is_clannie(ch->name)) {
	    send_to_char("For established clans only.\n\r", ch);
	    return;
	}

	if (strlen(arg3) == 0) {
	    send_to_char("Need new rank.\n\r", ch);
	    return;
	}

	y = atoi(arg3);

	/* Desh found a bug! */
	if (y < 0 || y > MAX_CLAN_RANKS) {
	    send_to_char("Bogus rank, dude!\n\r", ch);
	    return;
	}

	if (strlen(arg2) == 0) {
	    send_to_char("Need clannie name!\n\r", ch);
	    return;
	}

	x = is_clannie(arg2);
	if (x != ch->clan) {
	    send_to_char("That person isn't in your clan.\n\r", ch);
	    return;
	}
	x = clannie_lookup(ch->clan, arg2);

	clantable[ch->clan]->member[x]->rank = y;

	send_to_char("Rank set.\n\r", ch);
	return;
    }

    send_to_char("Unknown command - type \"clanldr\" for help.\n\r", ch);
    return;
}

int pledge_lookup(const char *name, int clan)
{
    /* Return pledge slot number */
    int x = 0;

    if (clan == 0) {
	return (0);
    }
    while (x < MAX_PLEDGE) {
	x++;
	if (clantable[clan]->pledge[x] != NULL) {
	    if (clantable[clan]->pledge[x]->name != NULL) {
		if (strcasecmp(clantable[clan]->pledge[x]->name, name) ==
		    0) {
		    if ((clantable[clan]->pledge[x]->flags & CPF_PLEDGE) ==
			CPF_PLEDGE) {
			return (x);
		    }
		}
	    }
	}
    }
    return (0);
}

int pledge_acl_lookup(const char *name, int clan)
{
    /* Return pledge slot number */
    int x = 0;

    if (clan == 0) {
	return (0);
    }
    while (x < MAX_PLEDGE) {
	x++;
	if (clantable[clan]->pledge[x] != NULL) {
	    if (clantable[clan]->pledge[x]->name != NULL) {
		if (strcasecmp(clantable[clan]->pledge[x]->name, name) ==
		    0) {
		    return (x);
		}
	    }
	}
    }
    return (0);
}

int is_acl_entry(const char *name, int clan)
{
    /* Return pledge slot number */
    int x = 0;

    if (clan == 0) {
	return (0);
    }
    while (x < MAX_PLEDGE) {
	x++;
	if (clantable[clan]->pledge[x] != NULL) {
	    if (clantable[clan]->pledge[x]->name != NULL) {
		if (strcasecmp(clantable[clan]->pledge[x]->name, name) ==
		    0) {
		    return (x);
		}
	    }
	}
    }
    return (0);
}

int new_pledge_entry(char *pname, int clan)
{

    int x = 0;

    if (clan == 0 || pname == NULL || strlen(pname) == 0) {
	return (-1);
    }

    while (x < MAX_PLEDGE) {
	x++;
	if (clantable[clan]->pledge[x] == NULL) {
	    clantable[clan]->pledge[x] = calloc(512, 8);
	    clantable[clan]->pledge[x]->name = strdup(pname);
	    clantable[clan]->pledge[x]->pcost = 0;
	    clantable[clan]->pledge[x]->flags = 0;
	    return (x);
	}
    }
    return (-2);
}

int get_clan_banked(int clan)
{
    if (clantable[clan] != NULL) {
	return (clantable[clan]->bank_balance);
    }
    return (0);
}

int set_clan_banked(int clan, long balance)
{
    if (clantable[clan] != NULL) {
	clantable[clan]->bank_balance = balance;
	return (1);
    }
    return (0);
}

void note_to_char(char *name, char *text)
{

    extern NOTE_DATA *new_note();
    extern void append_note(NOTE_DATA * pnote);

    NOTE_DATA *pnote;
    char *strtime;

    pnote = new_note();
    strtime = ctime(&current_time);
    strtime[strlen(strtime) + 1] = '\0';

    pnote->next = NULL;
    pnote->sender = str_dup("CLAN SYSTEM");
    pnote->date = str_dup(strtime);
    pnote->date_stamp = current_time;
    pnote->to_list = str_dup(name);
    pnote->subject = str_dup("CLAN EVENT");
    pnote->text = str_dup(text);
    pnote->type = NOTE_NOTE;

/* Now send it */
    append_note(pnote);

}

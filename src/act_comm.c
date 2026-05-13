/*	This .h or .c file is part of a Rom 2.4b2 code written by
Russel Taylor.  It has been further enhanced and edited by Mical
and Kyler.  MOST bugs are removed from this release, and color has
been added.  It also has several new features, and many changes
from the original code.  There are no back doors, and few bugs
left in the code.

  Kyler and I ask that if you use this code base, you add
  that this code is greatly enhanced from Rom 2.4b2, along with
  Russel Taylor's name.
  
	Mical/Kyler
	
	  
		TC960905
		Rewrote the channel part using one generic function.
		The table tab_channel contains the channel data
		void channel( int , CHAR_DATA * , char * );
		int is an enumerate containing the names
        of the channels. Answer has to be the last !!!
*/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "bets.h"
#include <stdarg.h>

char *chibichibify(char *string);
extern char *color_table[];
extern AUCTION_DATA *auction_info;


/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_put);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_help);

/* TC961118 donation tab is in const.c */
extern unsigned int tab_donate[NUMBER_RACES][DON_LAST_ROOM];

/* TC960905 */
/* TG980801 - tab_channel abstracted out to tables.c, but can't 
 * seem to take out the enum dammit */
/* Dont forget: Answer has to be the last in list !!! */
/* The order of tab_channel in tables.c has to be the same as the enum !! */
enum e_channel
    { Gossip, Flame, Auction, Question, Music, Quote, Grats, Ooc, Answer };

void channel(int, CHAR_DATA *, char *);



/* RT code to delete yourself */
void do_delet(CHAR_DATA * ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",
		 ch);
}

void do_delete(CHAR_DATA * ch, char *argument)
{
   extern int is_clannie(char *name); 
   extern int declan_char(char *clan_name,char *name);

   char strsave[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;

    if (ch->pcdata->confirm_delete) {
	if (argument[0] != '\0') {
	    send_to_char("Delete status removed.\n\r", ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	} else {
	    sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(ch->name));
	    wiznet("$N turns $Mself into line noise.", ch, NULL, 0, 0, 0);
	    stop_fighting(ch, TRUE);

	    /* Declan the char - Suzuran */
	    if(is_clannie(ch->name)){
              if((declan_char(get_clan_name(ch->clan),ch->name)) < 0){
		send_to_char("Declan failed, if you care.\n",ch);
		}
	      }

	    do_quit(ch, "");
	    unlink(strsave);
	    return;
	}
    }

    if (argument[0] != '\0') {
	send_to_char("Just type delete. No argument.\n\r", ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r", ch);
    send_to_char("`!WARN`1i`!NG`A:`` this command is irreversible.\n\r",
		 ch);
    send_to_char
	("Typing delete with an argument will undo delete status.\n\r",
	 ch);
    ch->pcdata->confirm_delete = TRUE;

    wiznet("$N is contemplating deletion.", ch, NULL, 0, 0, get_trust(ch));
}


/* RT code to display channel status */
void do_channels(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int cnt;

    /* lists all channels and their status */
    send_to_char("   channel     status\n\r", ch);
    send_to_char("---------------------\n\r", ch);

    for (cnt = 0; cnt <= Answer; cnt++) {
	send_to_char(tab_channel[cnt].name, ch);
	if (!IS_SET(ch->comm, tab_channel[cnt].flag))
	    send_to_char("      `$ON``\n\r", ch);
	else
	    send_to_char("      `4OFF``\n\r", ch);
    }

    if (is_granted_name(ch, "immtalk") || is_granted_name(ch, ":")) {
	send_to_char("IMM channel    ", ch);
	if (!IS_SET(ch->comm, COMM_NOIMM))
	    send_to_char("`$ON``\n\r", ch);
	else
	    send_to_char("`4OFF``\n\r", ch);
    }

    if (is_granted_name(ch, "admint")) {
	send_to_char("ADMIN channel  ", ch);
	if (!IS_SET(ch->comm, COMM_NOADMIN))
	    send_to_char("`$ON``\n\r", ch);
	else
	    send_to_char("`4OFF``\n\r", ch);
    }

    if (is_granted_name(ch, "imptalk")) {
	send_to_char("IMP channel    ", ch);
	if (!IS_SET(ch->comm, COMM_NOIMP))
	    send_to_char("`$ON``\n\r", ch);
	else
	    send_to_char("`4OFF``\n\r", ch);
    }

    if (is_granted_name(ch, "wiztalk")) {
	send_to_char("WIZ channel    ", ch);
	if (!IS_SET(ch->comm, COMM_NOWIZ))
	    send_to_char("`$ON``\n\r", ch);
	else
	    send_to_char("`4OFF``\n\r", ch);
    }

    /* BO980707 - Quest Info */
    send_to_char("Qinfo          ", ch);
    if (!IS_SET(ch->comm, COMM_NOINFO))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Shouts         ", ch);
    if (!IS_SET(ch->comm, COMM_SHOUTSOFF))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Deaf (tells)   ", ch);
    if (!IS_SET(ch->comm, COMM_DEAF))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    send_to_char("Quiet mode     ", ch);
    if (IS_SET(ch->comm, COMM_QUIET))
	send_to_char("`$ON``\n\r", ch);
    else
	send_to_char("`4OFF``\n\r", ch);

    if (IS_SET(ch->comm, COMM_AFK))
	send_to_char("You are AFK.\n\r", ch);

    if (IS_SET(ch->comm, COMM_SNOOP_PROOF))
	send_to_char("You are immune to snooping.\n\r", ch);

    if (ch->lines != PAGELEN) {
	if (ch->lines) {
	    sprintf(buf, "You display %d lines of scroll.\n\r",
		    ch->lines + 2);
	    send_to_char(buf, ch);
	} else
	    send_to_char("Scroll buffering is off.\n\r", ch);
    }

    if (ch->prompt != NULL) {
	sprintf(buf, "Your current prompt is: %s\n\r", ch->prompt);
	send_to_char(buf, ch);
    }

    if (IS_SET(ch->comm, COMM_NOSHOUT))
	send_to_char("You cannot shout.\n\r", ch);

    if (IS_SET(ch->comm, COMM_NOTELL))
	send_to_char("You cannot use tell.\n\r", ch);

    if (IS_SET(ch->comm, COMM_NOCHANNELS))
	send_to_char("You cannot use channels.\n\r", ch);

    if (IS_SET(ch->comm, COMM_NOEMOTE))
	send_to_char("You cannot show emotions.\n\r", ch);
}



/* TC960905 */
void channel(int cnb, CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
	if (IS_SET(ch->comm, tab_channel[cnb].flag)) {
	    sprintf(buf, "`%c%s`` channel is now ON.\n\r",
		    tab_channel[cnb].color, tab_channel[cnb].name);
	    send_to_char(buf, ch);

	    REMOVE_BIT(ch->comm, tab_channel[cnb].flag);
	} else {
	    sprintf(buf, "`%c%s`` channel is now OFF.\n\r",
		    tab_channel[cnb].color, tab_channel[cnb].name);
	    send_to_char(buf, ch);

	    SET_BIT(ch->comm, tab_channel[cnb].flag);
	}
    } else {			/* message sent, turn flag on if it is off */
	if (IS_SET(ch->comm, COMM_QUIET)) {
	    send_to_char("You must turn off quiet mode first.\n\r", ch);
	    return;
	}

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
	    send_to_char
		("The gods have revoked your channel priviliges.\n\r", ch);
	    return;
	}

	if (IS_VALIDATION(ch)) {
	    send_to_char
		("You must be validated before using public channels.\n\r",
		 ch);
	    return;
	}

	REMOVE_BIT(ch->comm, tab_channel[cnb].flag);

	sprintf(buf, "You %s`` '`%c%s``'\n\r", tab_channel[cnb].text,
		tab_channel[cnb].color, argument);
	send_to_char(buf, ch);

	for (d = descriptor_list; d != NULL; d = d->next) {
	    CHAR_DATA *victim;

	    victim = d->original ? d->original : d->character;

	    if (d->connected == CON_PLAYING
		&& d->character != ch
		&& !IS_SET(victim->comm, tab_channel[cnb].flag)
		&& !IS_SET(victim->comm, COMM_QUIET)) {
		sprintf(buf, "$n %ss `7'`%c$t``'\n\r",
			tab_channel[cnb].text, tab_channel[cnb].color);
		act_new(buf, ch, argument, d->character, TO_VICT,
			POS_DEAD);
	    }
	}
    }
}




/* RT deaf blocks out all shouts */
void do_deaf(CHAR_DATA * ch, char *argument)
{
    if (IS_SET(ch->comm, COMM_DEAF)) {
	send_to_char("You can now hear tells again.\n\r", ch);
	REMOVE_BIT(ch->comm, COMM_DEAF);
    } else {
	send_to_char("From now on, you won't hear tells.\n\r", ch);
	SET_BIT(ch->comm, COMM_DEAF);
    }
}



/* RT quiet blocks out all communication */
void do_quiet(CHAR_DATA * ch, char *argument)
{
    if (IS_SET(ch->comm, COMM_QUIET)) {
	send_to_char("Quiet mode removed.\n\r", ch);
	REMOVE_BIT(ch->comm, COMM_QUIET);
    } else {
	send_to_char
	    ("From now on, you will only hear says and emotes.\n\r", ch);
	SET_BIT(ch->comm, COMM_QUIET);
    }
}



void do_afk(CHAR_DATA * ch, char *argument)
{
    if (IS_SET(ch->comm, COMM_AFK)) {
	send_to_char("AFK mode removed. Type 'replay' to see tells.\n\r",
		     ch);
	REMOVE_BIT(ch->comm, COMM_AFK);
    } else {
	send_to_char("You are now in AFK mode.\n\r", ch);
	SET_BIT(ch->comm, COMM_AFK);
    }
}



void do_replay(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch)) {
	send_to_char("You can't replay.\n\r", ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0') {
	send_to_char("You have no tells to replay.\n\r", ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer), ch);
    clear_buf(ch->pcdata->buffer);
}



/* RT auction rewritten in ROM style */
void do_auction(CHAR_DATA * ch, char *argument)
{
    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Auction: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    channel(Auction, ch, argument);
}

/* RT chat replaced with ROM gossip */
void do_gossip(CHAR_DATA * ch, char *argument)
{

if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Gossip: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }
 
    channel(Gossip,ch, argument);
}

void do_grats(CHAR_DATA * ch, char *argument)
{

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Grats: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    channel(Grats, ch, argument);
}

void do_quote(CHAR_DATA * ch, char *argument)
{

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Quote: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    channel(Quote, ch, argument);
}

void do_question(CHAR_DATA * ch, char *argument)
{

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Question: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    channel(Question, ch, argument);
}

/* RT answer channel - uses same line as questions */
void do_answer(CHAR_DATA * ch, char *argument)
{

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Answer: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    channel(Answer, ch, argument);
}

/* RT music channel */
void do_music(CHAR_DATA * ch, char *argument)
{

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Music: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    channel(Music, ch, argument);
}

void do_flame(CHAR_DATA * ch, char *argument)
{

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Flame: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    channel(Flame, ch, argument);
}

void do_ooc(CHAR_DATA * ch, char *argument)
{
    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: OOC: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    channel(Ooc, ch, argument);
}

/* clan channels */
void do_clantalk(CHAR_DATA * ch, char *argument)
{
    //char                          buf             [MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

/* Changed for new clans - Suzuran */
/*	if (!is_clan(ch) || clan_table[ch->clan].independent) {
		send_to_char("You aren't in a clan.\n\r",ch);
		return;
	} */

    if (!is_clan(ch)) {
	send_to_char("You aren't in a clan.\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Clan: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    if (argument[0] == '\0') {
	if (IS_SET(ch->comm, COMM_NOCLAN)) {
	    send_to_char("Clan channel is now ON\n\r", ch);
	    REMOVE_BIT(ch->comm, COMM_NOCLAN);
	} else {
	    send_to_char("Clan channel is now OFF\n\r", ch);
	    SET_BIT(ch->comm, COMM_NOCLAN);
	}
	return;
    }

    if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
	send_to_char("The gods have revoked your channel priviliges.\n\r",
		     ch);
	return;
    }

    REMOVE_BIT(ch->comm, COMM_NOCLAN);
    act_new("`8You c`&l`8an`&:`` `^$t``", ch, argument, NULL, TO_CHAR,
	    POS_DEAD);

    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->connected == CON_PLAYING
	    && d->character != ch && is_same_clan(ch, d->character)
	    && !IS_SET(d->character->comm, COMM_NOCLAN)
	    && !IS_SET(d->character->comm, COMM_QUIET))
	    act_new("`8$n c`&l`8ans`&: `^$t``", ch, argument, d->character,
		    TO_VICT, POS_DEAD);
    }

    return;
}

void do_immtalk(CHAR_DATA * ch, char *argument)
{
    //char                          buf             [MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
	if (IS_SET(ch->comm, COMM_NOWIZ)) {
	    send_to_char("Immortal channel is now ON.\n\r", ch);
	    REMOVE_BIT(ch->comm, COMM_NOWIZ);
	} else {
	    send_to_char("Immortal channel is now OFF\n\r", ch);
	    SET_BIT(ch->comm, COMM_NOWIZ);
	}
	return;
    }

    REMOVE_BIT(ch->comm, COMM_NOWIZ);
    act_new("$n on IMMTalk: `!$t``", ch, argument, NULL, TO_CHAR,
	    POS_DEAD);

    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->connected == CON_PLAYING && IS_IMMORTAL(d->character)
	    && !IS_SET(d->character->comm, COMM_NOWIZ))
	    act_new("$n on IMMTalk: `!$t``", ch, argument, d->character,
		    TO_VICT, POS_DEAD);
    }

    return;
}

void do_imptalk(CHAR_DATA * ch, char *argument)
{
    //char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
	if (IS_SET(ch->comm, COMM_NOIMP)) {
	    send_to_char("IMP channel is now ON\n\r", ch);
	    REMOVE_BIT(ch->comm, COMM_NOIMP);
	} else {
	    send_to_char("IMP channel is now OFF\n\r", ch);
	    SET_BIT(ch->comm, COMM_NOIMP);
	}

	return;
    }

    REMOVE_BIT(ch->comm, COMM_NOIMP);
    /*sprintf( buf, "`6$n on `^IMP`6Talk`A: `^%s``", argument ); */
    act_new("$n on `^IMP`6Talk`A: `^$t``", ch, argument, NULL, TO_CHAR,
	    POS_DEAD);

    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->connected == CON_PLAYING
	    && is_granted_name(d->character, "imptalk")
	    && !IS_SET(d->character->comm, COMM_NOIMP))
	    act_new("`6$n on `^IMP`6Talk`A: `^$t``", ch, argument,
		    d->character, TO_VICT, POS_DEAD);
    }

    return;
}

void do_whisper(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
	send_to_char("Syntax is: whisper <name> <message>\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Whisper: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     *
     */
    if ((victim = get_char_world(ch, arg)) == NULL
	|| (IS_NPC(victim) && victim->in_room != ch->in_room)
	|| (!IS_NPC(victim) && victim->in_room != ch->in_room)) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }

    if (victim->desc == NULL && !IS_NPC(victim)) {
	act("$N's mind is not connected, try again later.", ch, NULL,
	    victim, TO_CHAR);
	return;
    }

    if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) {
	act("$E is asleep.", ch, 0, victim, TO_CHAR);
	return;
    }

    act("You whisper '`5$t``' to $N.", ch, argument, victim, TO_CHAR);
    act_new("$n whispers '`5$t``' to you.", ch, argument, victim, TO_VICT,
	    POS_DEAD);
    victim->reply = ch;

    return;
}

void do_wiztalk(CHAR_DATA * ch, char *argument)
{
    //char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
	if (IS_SET(ch->comm, COMM_NOWIZ)) {
	    send_to_char("WIZARD channel is now ON\n\r", ch);
	    REMOVE_BIT(ch->comm, COMM_NOWIZ);
	} else {
	    send_to_char("WIZARD channel is now OFF\n\r", ch);
	    SET_BIT(ch->comm, COMM_NOWIZ);
	}

	return;
    }

    REMOVE_BIT(ch->comm, COMM_NOWIZ);
    //sprintf( buf, "$n on `&WIZ``Talk: `&%s``", argument );
    act_new("$n on `&WIZ``T`8a``lk: `&$t``", ch, argument, NULL, TO_CHAR,
	    POS_DEAD);

    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->connected == CON_PLAYING
	    && is_granted_name(d->character, "wiztalk")
	    && !IS_SET(d->character->comm, COMM_NOWIZ))
	    act_new("$n on `&WIZ``T`8a``lk: `&$t``", ch, argument,
		    d->character, TO_VICT, POS_DEAD);
    }

    return;
}

void do_admintalk(CHAR_DATA * ch, char *argument)
{
    //char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
	if (IS_SET(ch->comm, COMM_NOADMIN)) {
	    send_to_char("ADMIN channel is now ON\n\r", ch);
	    REMOVE_BIT(ch->comm, COMM_NOADMIN);
	} else {
	    send_to_char("ADMIN channel is now OFF\n\r", ch);
	    SET_BIT(ch->comm, COMM_NOADMIN);
	}

	return;
    }

    REMOVE_BIT(ch->comm, COMM_NOADMIN);
    //sprintf( buf, "$n on ADMINTalk: `6%s``", argument );
    act_new("$n on `%A`5D`%MIN``Talk: `%$t``", ch, argument, NULL, TO_CHAR,
	    POS_DEAD);
    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->connected == CON_PLAYING
	    && is_granted_name(d->character, "admint")
	    && !IS_SET(d->character->comm, COMM_NOADMIN))
	    act_new("$n on `%A`5D`%MIN``Talk: `%$t``", ch, argument,
		    d->character, TO_VICT, POS_DEAD);
    }

    return;
}

void do_say(CHAR_DATA * ch, char *argument)
{
    if (argument[0] == '\0') {
	send_to_char("Say what?\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Say: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	argument = makedrunk(argument, ch);

    if (!IS_NPC(ch) && (IS_SET(ch->act, PLR_SAYS))) {
	act("`6You say '`^$T`6' $t.``", ch,
	    race_talk_table[ch->race].race_talk, argument, TO_CHAR);
	act("`6$n says '`^$T`6' $t.``", ch,
	    race_talk_table[ch->race].race_talk, argument, TO_ROOM);
    } else {
	act("`6You say '`^$T`6'``", ch, NULL, argument, TO_CHAR);
	act("`6$n says '`^$T`6'``", ch, NULL, argument, TO_ROOM);
    }

    /* Added for mobprogs - Suzuran */

    if ( !IS_NPC(ch) )
      {
	CHAR_DATA *mob, *mob_next;
	for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
	  {
	    mob_next = mob->next_in_room;
	    if ( IS_NPC(mob) && HAS_TRIGGER( mob, TRIG_SPEECH )
		 &&   mob->position == mob->pIndexData->default_pos )
	      mp_act_trigger( argument, mob, ch, NULL, NULL, TRIG_SPEECH );
	  }
      }

    return;
}

void do_shout(CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (IS_VALIDATION(ch)) {
	send_to_char
	    ("You must be validated before using public channels.\n\r",
	     ch);
	return;
    }

    if (argument[0] == '\0') {
	if (IS_SET(ch->comm, COMM_SHOUTSOFF)) {
	    send_to_char("You can hear shouts again.\n\r", ch);
	    REMOVE_BIT(ch->comm, COMM_SHOUTSOFF);
	} else {
	    send_to_char("You will no longer hear shouts.\n\r", ch);
	    SET_BIT(ch->comm, COMM_SHOUTSOFF);
	}

	return;
    }

    if (IS_SET(ch->comm, COMM_NOSHOUT)) {
	send_to_char("You can't shout.\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Shout: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    REMOVE_BIT(ch->comm, COMM_SHOUTSOFF);
    WAIT_STATE(ch, 12);
    act("`1You shout '`!$T`1'``", ch, NULL, argument, TO_CHAR);

    for (d = descriptor_list; d != NULL; d = d->next) {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if (d->connected == CON_PLAYING
	    && d->character != ch && !IS_SET(victim->comm, COMM_SHOUTSOFF)
	    && !IS_SET(victim->comm, COMM_QUIET))
	    act("`1$n shouts '`!$t`1'``", ch, argument, d->character,
		TO_VICT);
    }

    return;
}



void do_tell(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm, COMM_DEAF)) {
	send_to_char("Your message didn't get through.\n\r", ch);
	return;
    } else if (IS_SET(ch->comm, COMM_QUIET)) {
	send_to_char("You must turn off quiet mode first.\n\r", ch);
	return;
    } else if (IS_SET(ch->comm, COMM_DEAF)) {
	send_to_char("You must turn off deaf mode first.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || argument[0] == '\0') {
	send_to_char("Tell whom what?\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: tell: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    /*
       * Can tell to PC's anywhere, but NPC's only in same room.
       * -- Furey
     */
    if ((victim = get_char_world(ch, arg)) == NULL
	|| (IS_NPC(victim) && victim->in_room != ch->in_room)) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    } else if (victim->desc == NULL && !IS_NPC(victim)) {
	act("$N seems to have misplaced $S link...try again later.", ch,
	    NULL, victim, TO_CHAR);
	sprintf(buf, "`@%s tells you '`t%s`@'``\n\r", PERS(ch, victim),
		argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer, buf);
	return;
    } else if (!(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL)
	       && !IS_AWAKE(victim)) {
	act("$E can't hear you.", ch, 0, victim, TO_CHAR);
	return;
    } else if ((IS_SET(victim->comm, COMM_QUIET)
		|| IS_SET(victim->comm, COMM_DEAF)) && !IS_IMMORTAL(ch)) {
	act("$E is not receiving tells.", ch, 0, victim, TO_CHAR);
	return;
    } else if (IS_SET(victim->comm, COMM_AFK)) {
	if (IS_NPC(victim)) {
	    act("$E is AFK, and not receiving tells.", ch, NULL, victim,
		TO_CHAR);
	    return;
	}

	act("$E is AFK, but your tell will go through when $E returns.",
	    ch, NULL, victim, TO_CHAR);
	act("$n just sent you a tell.", ch, NULL, victim, TO_VICT);
	sprintf(buf, "`@%s tells you '`t%s```@'``\n\r", PERS(ch, victim),
		argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer, buf);
	return;
    }

    act("`@You tell $N '`t$t```@'``", ch, argument, victim, TO_CHAR);
    act_new("`@$n tells you '`t$t```@'``", ch, argument, victim, TO_VICT,
	    POS_DEAD);
    victim->reply = ch;

    return;
}



void do_reply(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (IS_SET(ch->comm, COMM_NOTELL)) {
	send_to_char("Your message didn't get through.\n\r", ch);
	return;
    } else if ((victim = ch->reply) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    } else if (victim->desc == NULL && !IS_NPC(victim)) {
	act("$N seems to have misplaced $S link...try again later.", ch,
	    NULL, victim, TO_CHAR);
	sprintf(buf, "`@%s replies '`r%s```@'``\n\r", PERS(ch, victim),
		argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer, buf);
	return;
    } else if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) {
	act("$E can't hear you.", ch, 0, victim, TO_CHAR);
	return;
    } else if ((IS_SET(victim->comm, COMM_QUIET)
		|| IS_SET(victim->comm, COMM_DEAF)) && !IS_IMMORTAL(ch)
	       && !IS_IMMORTAL(victim)) {
	act_new("$E is not receiving tells.", ch, 0, victim, TO_CHAR,
		POS_DEAD);
	return;
    } else if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch)) {
	send_to_char("In your dreams, or what?\n\r", ch);
	return;
    } else if (IS_SET(victim->comm, COMM_AFK)) {
	if (IS_NPC(victim)) {
	    act_new("$E is AFK, and not receiving tells.", ch, NULL,
		    victim, TO_CHAR, POS_DEAD);
	    return;
	}

	act_new
	    ("$E is AFK, but your tell will go through when $E returns.",
	     ch, NULL, victim, TO_CHAR, POS_DEAD);
	sprintf(buf, "`@%s replies '`r%s```@'``\n\r", PERS(ch, victim),
		argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer, buf);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Reply: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    act_new("`@You reply to $N '`r$t```@'``", ch, argument, victim,
	    TO_CHAR, POS_DEAD);
    act_new("`@$n replies '`r$t```@'``", ch, argument, victim, TO_VICT,
	    POS_DEAD);
    victim->reply = ch;

    return;
}



void do_yell(CHAR_DATA * ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (IS_VALIDATION(ch)) {
	send_to_char
	    ("You must be validated before using public channels.\n\r",
	     ch);
	return;
    } else if (IS_SET(ch->comm, COMM_NOSHOUT)) {
	send_to_char("You can't yell.\n\r", ch);
	return;
    } else if (argument[0] == '\0') {
	send_to_char("Yell what?\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Yell: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    act("`!You yell '`1$t`!'``", ch, argument, NULL, TO_CHAR);

    for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->connected == CON_PLAYING
	    && d->character != ch
	    && d->character->in_room != NULL
	    && d->character->in_room->area == ch->in_room->area
	    && !IS_SET(d->character->comm, COMM_QUIET))
	    act("`!$n yells '`1$t`!'``", ch, argument, d->character,
		TO_VICT);
    }
    /* Why is this here?  
    if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
	mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );
    */
    return;
}


void do_emote(CHAR_DATA * ch, char *argument)
{
    if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
	send_to_char("You can't show your emotions.\n\r", ch);
	return;
    }

    if (argument[0] == '\0') {
	send_to_char("Emote what?\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Emote: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    /* Disable MOB trigger for emotes. - Suzuran*/

    MOBtrigger=FALSE;
    if (argument[0] == '\'') {
	act("$n$T", ch, NULL, argument, TO_ROOM);
	act("$n$T", ch, NULL, argument, TO_CHAR);
    } else {
	act("$n $T", ch, NULL, argument, TO_ROOM);
	act("$n $T", ch, NULL, argument, TO_CHAR);
    }
    MOBtrigger=TRUE;

    return;
}


void do_pmote(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *vch;
    char *letter, *name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;

    if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
	send_to_char("You can't show your emotions.\n\r", ch);
	return;
    } else if (argument[0] == '\0') {
	send_to_char("Pmote what?\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Pmote: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    /* Same for pmote - Suzuran */

    MOBtrigger = FALSE;
    if (argument[0] == '\'') {
	act("$n$t", ch, argument, NULL, TO_CHAR);
    } else {
	act("$n $t", ch, argument, NULL, TO_CHAR);
    }

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument, vch->name)) == NULL) {
	    if (argument[0] == '\'') {
		act("$N$t", vch, argument, ch, TO_CHAR);
	    } else {
		act("$N $t", vch, argument, ch, TO_CHAR);
	    }
	    continue;
	}

	strcpy(temp, argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
	last[0] = '\0';
	name = vch->name;

	for (; *letter != '\0'; letter++) {
	    if (*letter == '\'' && matches == strlen(vch->name)) {
		strcat(temp, "r");
		continue;
	    }

	    if (*letter == 's' && matches == strlen(vch->name)) {
		matches = 0;
		continue;
	    }

	    if (matches == strlen(vch->name))
		matches = 0;

	    if (*letter == *name) {
		matches++;
		name++;

		if (matches == strlen(vch->name)) {
		    strcat(temp, "you");
		    last[0] = '\0';
		    name = vch->name;
		    continue;
		}

		strncat(last, letter, 1);
		continue;
	    }

	    matches = 0;
	    strcat(temp, last);
	    strncat(temp, letter, 1);
	    last[0] = '\0';
	    name = vch->name;
	}

	if (argument[0] == '\'') {
	    act("$N$t", vch, temp, ch, TO_CHAR);
	} else {
	    act("$N $t", vch, temp, ch, TO_CHAR);
	}
    }

    MOBtrigger = TRUE;
    return;
}


/*
* All the posing stuff.
*/
struct pose_table_type {
    char *message[2 * MAX_CLASS];
};

const struct pose_table_type pose_table[] = {
    {{
      "You sizzle with energy.",
      "$n sizzles with energy.",
      "You feel very holy.",
      "$n looks very holy.",
      "You perform a small card trick.",
      "$n performs a small card trick.",
      "You show your bulging muscles.",
      "$n shows $s bulging muscles."}},

    {{
      "You turn into a butterfly, then return to your normal shape.",
      "$n turns into a butterfly, then returns to $s normal shape.",
      "You nonchalantly turn wine into water.",
      "$n nonchalantly turns wine into water.",
      "You wiggle your ears alternately.",
      "$n wiggles $s ears alternately.",
      "You crack nuts between your fingers.",
      "$n cracks nuts between $s fingers."}},

    {{
      "Blue sparks fly from your fingers.",
      "Blue sparks fly from $n's fingers.",
      "A halo appears over your head.",
      "A halo appears over $n's head.",
      "You nimbly tie yourself into a knot.",
      "$n nimbly ties $mself into a knot.",
      "You grizzle your teeth and look mean.",
      "$n grizzles $s teeth and looks mean."}},

    {{
      "Little red lights dance in your eyes.",
      "Little red lights dance in $n's eyes.",
      "You recite words of wisdom.",
      "$n recites words of wisdom.",
      "You juggle with daggers, apples, and eyeballs.",
      "$n juggles with daggers, apples, and eyeballs.",
      "You hit your head, and your eyes roll.",
      "$n hits $s head, and $s eyes roll."}},

    {{
      "A slimy green monster appears before you and bows.",
      "A slimy green monster appears before $n and bows.",
      "Deep in prayer, you levitate.",
      "Deep in prayer, $n levitates.",
      "You steal the underwear off every person in the room.",
      "Your underwear is gone!  $n stole it!",
      "Crunch, crunch -- you munch a bottle.",
      "Crunch, crunch -- $n munches a bottle."}},

    {{
      "You turn everybody into a little pink elephant.",
      "You are turned into a little pink elephant by $n.",
      "An angel consults you.",
      "An angel consults $n.",
      "The dice roll ... and you win again.",
      "The dice roll ... and $n wins again.",
      "... 98, 99, 100 ... you do pushups.",
      "... 98, 99, 100 ... $n does pushups."}},

    {{
      "A small ball of light dances on your fingertips.",
      "A small ball of light dances on $n's fingertips.",
      "Your body glows with an unearthly light.",
      "$n's body glows with an unearthly light.",
      "You count the money in everyone's pockets.",
      "Check your money, $n is counting it.",
      "Arnold Schwarzenegger admires your physique.",
      "Arnold Schwarzenegger admires $n's physique."}},

    {{
      "Smoke and fumes leak from your nostrils.",
      "Smoke and fumes leak from $n's nostrils.",
      "A spot light hits you.",
      "A spot light hits $n.",
      "You balance a pocket knife on your tongue.",
      "$n balances a pocket knife on your tongue.",
      "Watch your feet, you are juggling granite boulders.",
      "Watch your feet, $n is juggling granite boulders."}},

    {{
      "The light flickers as you rap in magical languages.",
      "The light flickers as $n raps in magical languages.",
      "Everyone levitates as you pray.",
      "You levitate as $n prays.",
      "You produce a coin from everyone's ear.",
      "$n produces a coin from your ear.",
      "Oomph!  You squeeze water out of a granite boulder.",
      "Oomph!  $n squeezes water out of a granite boulder."}},

    {{
      "Your head disappears.",
      "$n's head disappears.",
      "A cool breeze refreshes you.",
      "A cool breeze refreshes $n.",
      "You step behind your shadow.",
      "$n steps behind $s shadow.",
      "You pick your teeth with a spear.",
      "$n picks $s teeth with a spear."}},

    {{
      "A fire elemental singes your hair.",
      "A fire elemental singes $n's hair.",
      "The sun pierces through the clouds to illuminate you.",
      "The sun pierces through the clouds to illuminate $n.",
      "Your eyes dance with greed.",
      "$n's eyes dance with greed.",
      "Everyone is swept off their foot by your hug.",
      "You are swept off your feet by $n's hug."}},

    {{
      "The sky changes color to match your eyes.",
      "The sky changes color to match $n's eyes.",
      "The ocean parts before you.",
      "The ocean parts before $n.",
      "You deftly steal everyone's weapon.",
      "$n deftly steals your weapon.",
      "Your karate chop splits a tree.",
      "$n's karate chop splits a tree."}},

    {{
      "The stones dance to your command.",
      "The stones dance to $n's command.",
      "A thunder cloud kneels to you.",
      "A thunder cloud kneels to $n.",
      "The Grey Mouser buys you a beer.",
      "The Grey Mouser buys $n a beer.",
      "A strap of your armor breaks over your mighty thews.",
      "A strap of $n's armor breaks over $s mighty thews."}},

    {{
      "The heavens and grass change colour as you smile.",
      "The heavens and grass change colour as $n smiles.",
      "The Burning Man speaks to you.",
      "The Burning Man speaks to $n.",
      "Everyone's pocket explodes with your fireworks.",
      "Your pocket explodes with $n's fireworks.",
      "A boulder cracks at your frown.",
      "A boulder cracks at $n's frown."}},

    {{
      "Everyone's clothes are transparent, and you are laughing.",
      "Your clothes are transparent, and $n is laughing.",
      "An eye in a pyramid winks at you.",
      "An eye in a pyramid winks at $n.",
      "Everyone discovers your dagger a centimeter from their eye.",
      "You discover $n's dagger a centimeter from your eye.",
      "Mercenaries arrive to do your bidding.",
      "Mercenaries arrive to do $n's bidding."}},

    {{
      "A black hole swallows you.",
      "A black hole swallows $n.",
      "Valentine Michael Smith offers you a glass of water.",
      "Valentine Michael Smith offers $n a glass of water.",
      "Where did you go?",
      "Where did $n go?",
      "Four matched Percherons bring in your chariot.",
      "Four matched Percherons bring in $n's chariot."}},

    {{
      "The world shimmers in time with your whistling.",
      "The world shimmers in time with $n's whistling.",
      "The great god Mota gives you a staff.",
      "The great god Mota gives $n a staff.",
      "Click.",
      "Click.",
      "Atlas asks you to relieve him.",
      "Atlas asks $n to relieve him."}}
};



void do_pose(CHAR_DATA * ch, char *argument)
{
    int level;
    int pose;

    if (IS_NPC(ch))
	return;

    level =
	UMIN(ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1);
    pose = number_range(0, level);

    act(pose_table[pose].message[2 * ch->class + 0], ch, NULL, NULL,
	TO_CHAR);
    act(pose_table[pose].message[2 * ch->class + 1], ch, NULL, NULL,
	TO_ROOM);

    return;
}



void do_bug(CHAR_DATA * ch, char *argument)
{
    if (argument[0] == '\0') {
	send_to_char("We need an argument to log a bug!\n\r", ch);
	return;
    }

    append_file(ch, BUG_FILE, argument);
    send_to_char("Bug logged.\n\r", ch);
    return;
}

void do_typo(CHAR_DATA * ch, char *argument)
{
    if (argument[0] == '\0') {
	send_to_char("We need an argument to log a typo!\n\r", ch);
	return;
    }

    append_file(ch, TYPO_FILE, argument);
    send_to_char("Typo logged.\n\r", ch);
    return;
}

void do_rent(CHAR_DATA * ch, char *argument)
{
    send_to_char("There is no rent here.  Just save and quit.\n\r", ch);
    return;
}


void do_qui(CHAR_DATA * ch, char *argument)
{
    send_to_char("If you want to QUIT, you have to spell it out.\n\r", ch);
    return;
}



void do_quit(CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d, *d_next;
    char buf[MAX_STRING_LENGTH];
    int id;

    if (IS_NPC(ch))
	return;
    else if (ch->position == POS_FIGHTING) {
	send_to_char("No way! You are fighting.\n\r", ch);
	return;
    } else if (ch->position < POS_STUNNED) {
	send_to_char("You're not DEAD yet.\n\r", ch);
	return;
    }
	else if (auction_info->current_obj != NULL
		 && ((ch == auction_info->buyer)
		     || (ch == auction_info->current_obj->seller))) {
	send_to_char
	    ("Wait till you have sold/bought the item on auction.\n\r",
	     ch);
	return;
    } else if (ch->fight > 0 && !IS_IMMORTAL(ch)) {
	send_to_char
	    ("I think you'd better stick around a little longer.\n\r", ch);
	return;
    } else if (ch->quest != NULL && ch->in_room != ch->quest->in_room) {
	send_to_char
	    ("If you want your quest mob saved, you must be in the same room as it.\n\r",
	     ch);
	return;
    } else if (IS_SET(ch->act, PLR_KILLER) || IS_SET(ch->act, PLR_THIEF)) {
	send_to_char
	    ("Im sorry, Criminals are not allowed to leave the game.\n\r",
	     ch);
	return;
    }

    /* Added this for the bank stuff... --Vorlin */
	if (ch->pcdata->bank_timer > 0) {
	    ch->pcdata->bank_timer++;
	    send_to_char("You can't quit because you tried to abuse the bank bug.\n\r", ch);
	    send_to_char("In fact, I think you should have another tick added onto it.\n\r", ch);
	    sprintf(buf,"There, your wait is now %d minutes, roughly.\n\r", ch->pcdata->bank_timer);
	    send_to_char(buf, ch);
	    return;
	}

    if (IS_QUESTOR(ch))		/*EE960522 */
	REMOVE_BIT(ch->act, PLR_QUESTOR);

    if (IS_QUESTMST(ch)) {	/*EE960522 */
	for (victim = char_list; victim != NULL; victim = victim->next) {
	    if (IS_NPC(victim))
		continue;

	    if (IS_QUESTOR(victim)) {
		act("$N's quest has ceased to exist.", ch, NULL, victim,
		    TO_VICT);
		REMOVE_BIT(victim->act, PLR_QUESTOR);
	    }
	}

	send_to_char("You have stopped your quest.\n\r", ch);
    }

    send_to_char("Alas, all good things must come to an end.\n\r", ch);
    act("$n has left the game.", ch, NULL, NULL, TO_ROOM);
    sprintf(log_buf, "%s has quit.", ch->name);
    log_string(log_buf);
    wiznet("$N rejoins the real world.", ch, NULL, WIZ_LOGINS, 0,
	   get_trust(ch));

    /*
     * After extract_char the ch is no longer valid!
     */
    if (ch->level > 1) {
	save_char_obj(ch);
    } else {
	send_to_char("You can't save until level 2.\n\r", ch);
    }

    id = ch->id;
    d = ch->desc;
	/* C IS NOT BASIC, YOU CANNOT COPY A STRING WITH = ! */
    d->host =str_dup(ch->pcdata->socket);

    extract_char(ch, TRUE);

    if (d != NULL)
	close_socket(d);

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next) {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;

	if (tch && tch->id == id) {
	    log_string("Toast evil cheating bastards!");
	    extract_char(tch, TRUE);
	    close_socket(d);
	}
    }

    return;
}



void do_save(CHAR_DATA * ch, char *argument)
{
    if (IS_NPC(ch))
	return;
    if (ch->level == 1) {
	send_to_char("You must be level 2 before you can save.\n\r", ch);
	return;
    }

    save_char_obj(ch);
    send_to_char
	("`^Saving```6.```^ Remember that ```&A``n`&i``m`&e M``U`&D```^ has automatic saving`6.``\n\r",
	 ch);

    if (!IS_IMMORTAL(ch))
	WAIT_STATE(ch, 3 * PULSE_VIOLENCE);

    return;
}



void do_follow(CHAR_DATA * ch, char *argument)
{
    /* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Follow whom?\n\r", ch);
	return;
    } else if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    } else if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL) {
	act("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
	return;
    } else if (victim == ch) {
	if (ch->master == NULL) {
	    send_to_char("You already follow yourself.\n\r", ch);
	    return;
	}

	stop_follower(ch);
	return;
    } else if (!IS_NPC(victim) && IS_SET(victim->act, PLR_NOFOLLOW)
	       && !IS_IMMORTAL(ch)) {
	act("$N doesn't seem to want any followers.\n\r", ch, NULL, victim,
	    TO_CHAR);
	return;
    }

    REMOVE_BIT(ch->act, PLR_NOFOLLOW);

    if (ch->master != NULL)
	stop_follower(ch);

    add_follower(ch, victim);
    return;
}


void add_follower(CHAR_DATA * ch, CHAR_DATA * master)
{
    if (ch->master != NULL) {
	bug("Add_follower: non-null master.", 0);
	return;
    }

    ch->master = master;
    ch->leader = NULL;

    if (can_see(master, ch))
	act("$n now follows you.", ch, NULL, master, TO_VICT);

    act("You now follow $N.", ch, NULL, master, TO_CHAR);

    return;
}



void stop_follower(CHAR_DATA * ch)
{
    if (ch->master == NULL) {
	bug("Stop_follower: null master.", 0);
	return;
    }

    if (IS_AFFECTED(ch, AFF_CHARM)) {
	REMOVE_BIT(ch->affected_by, AFF_CHARM);
	affect_strip(ch, gsn_charm_person);
    }

    if (can_see(ch->master, ch) && ch->in_room != NULL) {
	act("$n stops following you.", ch, NULL, ch->master, TO_VICT);
	act("You stop following $N.", ch, NULL, ch->master, TO_CHAR);
    }

    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;

    return;
}

/* nukes charmed monsters and pets */
void nuke_pets(CHAR_DATA * ch)
{
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL) {
	stop_follower(pet);

	if (pet->in_room != NULL)
	    act("$N slowly fades away.", ch, NULL, pet, TO_NOTVICT);
	extract_char(pet, TRUE);
    }

    ch->pet = NULL;

    return;
}

void nuke_quest(CHAR_DATA * ch)
{
    CHAR_DATA *quest;

    if ((quest = ch->quest) != NULL) {
	if (quest->in_room != NULL)
	    act("$N slowly fades away.", ch, NULL, quest, TO_NOTVICT);
	extract_char(quest, TRUE);
    }

    ch->quest = NULL;

    return;
}


void die_follower(CHAR_DATA * ch)
{
    CHAR_DATA *fch;

    if (ch->master != NULL) {
	if (ch->master->pet == ch)
	    ch->master->pet = NULL;

	if (ch->master->horse == ch)
	    ch->master->horse = NULL;

	if (ch->master->quest == ch)
	    ch->master->quest = NULL;

	stop_follower(ch);
    }

    ch->leader = NULL;

    for (fch = char_list; fch != NULL; fch = fch->next) {
	if (fch->master == ch && !IS_SET(fch->act, ACT_HORSE))
	    stop_follower(fch);

	if (fch->leader == ch && !IS_SET(fch->act, ACT_HORSE))
	    fch->leader = fch;
    }

    return;
}



void do_order(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument(argument, arg);
    one_argument(argument, arg2);

    /* Don't allow charmed mob commands - Suzuran */
    if (!str_cmp(arg2, "delete") || !str_cmp(arg2,"mob")) {
	send_to_char("That will NOT be done.\n\r", ch);
	return;
    }

    if (!str_cmp(arg2, "save") || !str_cmp(arg2, "sav")) {
	send_to_char("That will NOT be done.\n\r", ch);
	return;
    }

    if (arg[0] == '\0' || argument[0] == '\0') {
	send_to_char("Order whom to do what?\n\r", ch);
	return;
    }

    /* Added this so demons can't force people to write notes */
    /* Vorlin, 8/23/2000 */

    if (!str_cmp(arg2, "note") || !str_cmp(arg2, "not")) {
	send_to_char("You can't do that.\n\r", ch);
	return;
    }


    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("You feel like taking, not giving, orders.\n\r", ch);
	return;
    }

    if (!str_cmp(arg, "all")) {
	fAll = TRUE;
	victim = NULL;
    } else {
	fAll = FALSE;

	if ((victim = get_char_room(ch, arg)) == NULL) {
	    send_to_char("They aren't here.\n\r", ch);
	    return;
	} else if (!IS_NPC(victim) && !str_cmp(arg, "all")
		   && (!str_cmp(arg2, "kill") || !str_cmp(arg2, "k")
		       || !str_cmp(arg2, "kil") || !str_cmp(arg2, "ki"))) {
	    send_to_char("He is your friend... not your weapon!\n\r", ch);
	    return;
	} else if (victim == ch) {
	    send_to_char("Aye aye, right away!\n\r", ch);
	    return;
	}
	    else
	    if (
		(!IS_AFFECTED(victim, AFF_CHARM) || (victim->master != ch)
		 || (IS_IMMORTAL(victim)
		     && (get_trust(ch) < get_trust(victim))))
		&& (!IS_SET(victim->act, ACT_HORSE)
		    && (victim->master != ch))) {
	    send_to_char("Do it yourself!\n\r", ch);
	    return;
	}
    }

    found = FALSE;

    for (och = ch->in_room->people; och != NULL; och = och_next) {
	och_next = och->next_in_room;

	if ((IS_AFFECTED(och, AFF_CHARM) && och->master == ch
	     && (fAll || och == victim)) || (ch->horse != NULL
					     && ch->horse == och)) {
	    found = TRUE;
	    sprintf(buf, "$n orders you to '%s'.", argument);
	    act(buf, ch, NULL, och, TO_VICT);
	    interpret(och, argument);
	}
    }

    if (found) {
	WAIT_STATE(ch, PULSE_VIOLENCE);
	send_to_char("Ok.\n\r", ch);
    } else
	send_to_char("You have no followers here.\n\r", ch);

    return;
}



void do_group(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf(buf, "%s's group:\n\r", PERS(leader, ch));
	send_to_char(buf, ch);

	for (gch = char_list; gch != NULL; gch = gch->next) {
	    if (is_same_group(gch, ch)) {
		/* TC960906  made "[%2d to [%3d   so Belgarion or I can group ;-) */
		sprintf(buf,
			"[%3d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
			gch->level,
			IS_NPC(gch) ? "Mob" : class_table[gch->
							  class].who_name,
			capitalize(PERS(gch, ch)), gch->hit, gch->max_hit,
			gch->mana, gch->max_mana, gch->move, gch->max_move,
			gch->exp);
		send_to_char(buf, ch);
	    }
	}

	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
	else if (ch->master != NULL
		 || (ch->leader != NULL && ch->leader != ch)) {
	send_to_char("But you are following someone else!\n\r", ch);
	return;
    } else if (victim->master != ch && ch != victim) {
	act("$N isn't following you.", ch, NULL, victim, TO_CHAR);
	return;
    } else if (IS_AFFECTED(victim, AFF_CHARM)) {
	send_to_char("You can't remove charmed mobs from your group.\n\r",
		     ch);
	return;
    } else if (IS_AFFECTED(ch, AFF_CHARM)) {
	act("You like your master too much to leave $m!", ch, NULL, victim,
	    TO_VICT);
	return;
    } else if (!IS_QUESTOR(ch) || !IS_QUESTOR(victim)) {	/*EE960617 */
	if (victim->level - ch->level > GROUP_LEVEL) {
	    send_to_char("They are to high of a level for your group.\n\r",
			 ch);
	    return;
	} else if (victim->level - ch->level < (0 - GROUP_LEVEL)) {
	    send_to_char("They are to low of a level for your group.\n\r",
			 ch);
	    return;
	}
    }

    if (is_same_group(victim, ch) && ch != victim) {
	victim->leader = NULL;
	act("$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT);
	act("$n removes you from $s group.", ch, NULL, victim, TO_VICT);
	act("You remove $N from your group.", ch, NULL, victim, TO_CHAR);
	return;
    }

    victim->leader = ch;
    act("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
    act("You join $n's group.", ch, NULL, victim, TO_VICT);
    act("$N joins your group.", ch, NULL, victim, TO_CHAR);
    return;
}



/*
* 'Split' originally by Gnort, God of Chaos.
*/
void do_split(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);

    if (arg1[0] == '\0') {
	send_to_char("Split how much?\n\r", ch);
	return;
    }

    amount_silver = atoi(arg1);

    if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

    if (amount_gold < 0 || amount_silver < 0) {
	send_to_char("Your group wouldn't like that.\n\r", ch);
	return;
    }

    if (amount_gold == 0 && amount_silver == 0) {
	send_to_char("You hand out zero coins, but no one notices.\n\r",
		     ch);
	return;
    }

    if (ch->gold < amount_gold || ch->silver < amount_silver) {
	send_to_char("You don't have that much to split.\n\r", ch);
	return;
    }

    members = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
	if (is_same_group(gch, ch) && !IS_AFFECTED(gch, AFF_CHARM))
	    members++;
    }

    if (members < 2) {
	send_to_char("Just keep it all.\n\r", ch);
	return;
    }

    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold = amount_gold / members;
    extra_gold = amount_gold % members;

    if (share_gold == 0 && share_silver == 0) {
	send_to_char("Don't even bother, cheapskate.\n\r", ch);
	return;
    }

    ch->silver -= amount_silver;
    ch->silver += share_silver + extra_silver;
    ch->gold -= amount_gold;
    ch->gold += share_gold + extra_gold;

    if (share_silver > 0) {
	sprintf(buf,
		"You split %d silver coins. Your share is %d silver.\n\r",
		amount_silver, share_silver + extra_silver);
	send_to_char(buf, ch);
    }

    if (share_gold > 0) {
	sprintf(buf, "You split %d gold coins. Your share is %d gold.\n\r",
		amount_gold, share_gold + extra_gold);
	send_to_char(buf, ch);
    }

    if (share_gold == 0) {
	sprintf(buf, "$n splits %d silver coins. Your share is %d silver.",
		amount_silver, share_silver);
    } else if (share_silver == 0) {
	sprintf(buf, "$n splits %d gold coins. Your share is %d gold.",
		amount_gold, share_gold);
    } else {
	sprintf(buf,
		"$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
		amount_silver, amount_gold, share_silver, share_gold);
    }

    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
	if (gch != ch && is_same_group(gch, ch)
	    && !IS_AFFECTED(gch, AFF_CHARM)) {
	    act(buf, ch, NULL, gch, TO_VICT);
	    gch->gold += share_gold;
	    gch->silver += share_silver;
	}
    }

    return;
}



void do_gtell(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    if (argument[0] == '\0') {
	send_to_char("Tell your group what?\n\r", ch);
	return;
    } else if (IS_SET(ch->comm, COMM_NOTELL)) {
	send_to_char("Your message didn't get through!\n\r", ch);
	return;
    }

    if(is_affected(ch,skill_lookup("chibichibification"))){
        char wnbuf[MAX_STRING_LENGTH];
        sprintf(wnbuf,"%s: Gtell: %s\n",ch->name,argument);
        wiznet(wnbuf,NULL,NULL,WIZ_CHIBI,0,95);
        argument = chibichibify(argument);
    }

    /*
       * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf(buf, "`5%s tells the group '`s%s```5'``\n\r",
	    !IS_NPC(ch) ? ch->name : capitalize(ch->short_descr),
	    argument);

    for (gch = char_list; gch != NULL; gch = gch->next) {
	if (is_same_group(gch, ch))
	    send_to_char(buf, gch);
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group(CHAR_DATA * ach, CHAR_DATA * bch)
{
    if (ach == NULL || bch == NULL)
	return FALSE;

    if (ach->leader != NULL)
	ach = ach->leader;
    if (bch->leader != NULL)
	bch = bch->leader;

    return ach == bch;
}



void do_touch(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    int health;
    chance = 0;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
	send_to_char("Who do you wish to heal?\n\r", ch);
	return;
    }
    if ((chance = get_skill(ch, gsn_touch_healing)) == 0 || (!IS_NPC(ch)
							     && ch->level <
							     skill_table
							     [gsn_touch_healing].skill_level
							     [ch->class]))
    {
	send_to_char("You do not know the arts of healing.\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    if (victim->position == POS_FIGHTING)
	chance += 30;

    if (ch->position == POS_FIGHTING)
	chance += 50;

    if (victim->position == POS_SLEEPING)
	chance -= 50;

    if (victim->max_hit > ch->max_hit)
	chance += 15;

    if (IS_AFFECTED(ch, AFF_BERSERK) || is_affected(ch, gsn_berserk)
	|| is_affected(ch, skill_lookup("frenzy")))
	chance += 10;

    if (get_curr_stat(ch, STAT_CON) < get_curr_stat(victim, STAT_CON))
	chance -= 30;

    if (IS_NPC(victim) || (IS_NPC(ch)))
	return;

    if (ch->level >= victim->level)
	chance += 25 + ch->level / 2;

    if (ch->mana < 30 || ch->move < (ch->level + victim->level / 2)) {
	send_to_char("You can't gather up enough energy.\n\r", ch);
	return;
    }
    if (number_percent() < chance) {
	ch->mana -= 30;
	ch->move -= (ch->level + victim->level) / 3;
	health = ch->move / 5 + 30;
	if (health >= victim->max_hit
	    || (health + victim->hit) >= victim->max_hit)
		victim->hit = victim->max_hit;
	else
	    victim->hit += health;

	affect_strip(victim, gsn_plague);
	affect_strip(victim, gsn_poison);
	affect_strip(victim, gsn_curse);
	act("You apply the art of healing to $M.", ch, NULL, victim,
	    TO_CHAR);
	act("$n helps you heal your wounds.", ch, NULL, victim, TO_VICT);
	act("$n heals $N with $s touch.", ch, NULL, victim, TO_NOTVICT);
	check_improve(ch, gsn_touch_healing, TRUE, 2);
    } else {
	ch->mana -= 30;
	ch->move -= ch->level / 2;
	act("You apply your healing to $M but fail.", ch, NULL, victim,
	    TO_CHAR);
	act("$n tries to heal you but fails.", ch, NULL, victim, TO_VICT);
	act("$n tried to heal $N with $s touch.", ch, NULL, victim,
	    TO_NOTVICT);
	check_improve(ch, gsn_touch_healing, FALSE, 2);
    }
    WAIT_STATE(ch, skill_table[gsn_touch_healing].beats);
}

void do_donate(CHAR_DATA * ch, char *argument)
{
    /*      OBJ_DATA *pit; */
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *original;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amount;
    int to_room;		/* TC961030  to wich room the object goes */

    argument = one_argument(argument, arg);

    if (IS_NPC(ch))
	return;

    if (arg[0] == '\0') {
	send_to_char("What would you like to donate?\n\r", ch);
	return;
    }
    original = ch->in_room;
    if (ch->position == POS_FIGHTING) {
	send_to_char("Better stop fighting first!\n\r", ch);
	return;
    }
    if ((obj = get_obj_carry(ch, arg)) == NULL) {
	send_to_char("You do not have that item!\n\r", ch);
	return;
    } else {
	if (!can_drop_obj(ch, obj)) {
	    send_to_char("You can't let go of it.\n\r", ch);
	    return;
	}
	if ((obj->item_type == ITEM_CORPSE_NPC) ||
	    (obj->item_type == ITEM_CORPSE_PC)) {
	    send_to_char("You cannot donate that!\n\r", ch);
	    return;
	}
	if (obj->timer > 0) {
	    send_to_char("You may only donate permanent items.\n\r", ch);
	    return;
	}
	/* TC961030 donate to rooms
	   Finger  + Hands    to  3377  (Torak)
	   Neck    + Torso    to  3375  (Chaldon)
	   Head    + Legs     to  3376  (Belar)
	   Feet    + Arms     to  3378  (Issa)
	   Shield  + Body     to  3379  (Nedra)
	   Waist   + Other    to  3373  (Mara)
	   Light   + Weapons  to  3374  (Aldur)

	   Worthless eq (value < 2) can't be donated anymore
	   as level 0 objects
	 */
	if (obj->cost < 2 || obj->level == 0) {
	    send_to_char("Noone would be interested in that.\n\r", ch);
	    return;
	}

	/*if (obj->level > 30)
	   {
	   send_to_char("That item is too powerful for you to donate.\n\r", ch);
	   return;
	   } */
	if (ch->in_room != get_room_index(ROOM_VNUM_ALTAR))
	    act("$n donates `&$p``.", ch, obj, NULL, TO_ROOM);
	act("You donate `&$p``.", ch, obj, NULL, TO_CHAR);

	if ((!IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	    (!IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	    IS_NEUTRAL(ch))
	    if (obj->cost > 2 && obj->level > 0) {
		amount = obj->level * 6;
		sprintf(buf,
			"You are rewarded %d `*silver`` for your generosity.\n\r",
			amount);
		send_to_char(buf, ch);
		ch->silver += amount;
	    }
	/* if */
	obj->cost = 2;		/* no multiple donate to gain gold */

	if (IS_SET(obj->wear_flags, ITEM_WEAR_FINGER))
	    to_room = tab_donate[ch->race][DON_FINGER];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_HANDS))
	    to_room = tab_donate[ch->race][DON_HAND];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_NECK))
	    to_room = tab_donate[ch->race][DON_NECK];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_ABOUT))
	    to_room = tab_donate[ch->race][DON_ABOUT];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_HEAD))
	    to_room = tab_donate[ch->race][DON_HEAD];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_LEGS))
	    to_room = tab_donate[ch->race][DON_LEGS];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_FEET))
	    to_room = tab_donate[ch->race][DON_FEET];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_ARMS))
	    to_room = tab_donate[ch->race][DON_ARMS];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
	    to_room = tab_donate[ch->race][DON_SHIELD];
	else if (IS_SET(obj->wear_flags, ITEM_WEAR_BODY))
	    to_room = tab_donate[ch->race][DON_BODY];
	else if (IS_SET(obj->wear_flags, ITEM_HOLD))
	    to_room = tab_donate[ch->race][DON_HOLD];
	else if (IS_SET(obj->wear_flags, ITEM_WIELD))
	    to_room = tab_donate[ch->race][DON_WIELD];
	else
	    /* waist, wrist and the rest */
	    to_room = tab_donate[ch->race][DON_OTHER];

	char_from_room(ch);
	/*   char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR)); 
	   pit = get_obj_list(ch, "pit", ch->in_room->contents);  */
	char_to_room(ch, get_room_index(to_room));
	obj_from_char(obj);
	/*   obj_to_obj(obj, pit); */
	obj_to_room(obj, get_room_index(to_room));
	char_from_room(ch);
	char_to_room(ch, original);
	return;
    }
}

/*EE960410*/

void do_pkset(CHAR_DATA * ch, char *argument)
{

    if (IS_NPC(ch))
	return;

    if (IS_VALIDATION(ch)) {
	send_to_char("You must be validated first before going PKset.\n\r",
		     ch);
	return;
    }

    if (ch->pcdata->pkset) {
	send_to_char("You cannot be more of a killer than you are.\n\r",
		     ch);
	return;
    }

    if (ch->pcdata->confirm_pkset) {
	if (argument[0] != '\0') {
	    send_to_char("Pkset status removed.\n\r", ch);
	    ch->pcdata->confirm_pkset = FALSE;
	    return;
	} else {
	    ch->pcdata->pkset = TRUE;
	    ch->pcdata->confirm_pkset = FALSE;
	    ch->max_hit = ch->max_hit + 100;
	    ch->max_mana = ch->max_mana + 100;
	    ch->max_move = ch->max_move + 100;

	    ch->pcdata->perm_hit = ch->pcdata->perm_hit + 100;
	    ch->pcdata->perm_mana = ch->pcdata->perm_mana + 100;
	    ch->pcdata->perm_move = ch->pcdata->perm_move + 100;
	    /* PKsetting slashes QP to one third. - KV020516 */
	    ch->questpoints /= 3;

	    send_to_char("You are now allowed to kill other players!\n\r",
			 ch);
	    wiznet("$N has chosen the path of a player killer.", ch, NULL,
		   0, 0, get_trust(ch));

	    return;
	}
    }

    if (argument[0] != '\0') {
	send_to_char("Just type pkset. No argument.\n\r", ch);
	return;
    }

    send_to_char("Type pkset again to confirm this command.\n\r", ch);
    send_to_char("`!WARNING`A:`` this command is irreversible.\n\r", ch);
    send_to_char
	("Typing pkset with an argument will undo pkset status.\n\r", ch);
    ch->pcdata->confirm_pkset = TRUE;
    wiznet("$N is contemplating being a player killer.", ch, NULL, 0, 0,
	   get_trust(ch));

}

void do_cls(CHAR_DATA * ch)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%s%s\n\r", color_table[18], color_table[19]);
    send_to_char(buf, ch);
    return;
}


void do_level(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (ch->level < LEVEL_HERO) {
	sprintf(buf,
		"You need to be level %d or higher to use this command.\n\r",
		LEVEL_HERO);
	send_to_char(buf, ch);
	return;
    }

    if (ch->level == LEVEL_HERO) {
	if (ch->rp_points >= 500) {
	    sprintf(buf,
		    "You have advanced a level! May you become even `&b`8e`&tt`8e`&r`` at `@R`#P``!\n\r");
	    send_to_char(buf, ch);
	    ch->level += 1;
	    ch->rp_points -= 500;
	    return;
	}
    } else if (ch->level == (LEVEL_HERO + 1)) {
	if (ch->rp_points >= 1500) {
	    sprintf(buf,
		    "You have advanced a level! May you become even `&b`8e`&tt`8e`&r`` at `@R`#P``!\n\r");
	    send_to_char(buf, ch);
	    ch->level += 1;
	    ch->rp_points -= 1500;
	    return;
	}
    } else if (ch->level == (LEVEL_HERO + 2)) {
	if (ch->rp_points >= 4500) {
	    sprintf(buf,
		    "You have advanced a level! May you become even `&b`8e`&tt`8e`&r`` at `@R`#P``!\n\r");
	    send_to_char(buf, ch);
	    ch->level += 1;
	    ch->rp_points -= 4500;
	    return;
	}
    } else {
	sprintf(buf,
		"You can advance no farther with `@R`#P`` po`&i``nts.\n\r");
	send_to_char(buf, ch);
	return;
    }

    return;
}

/* BO980707 - Quest info */

void info(CHAR_DATA * ch, int level, char *message, ...)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    va_list args;

    va_start(args, message);
    vsnprintf(buf, MAX_STRING_LENGTH, message, args);
    va_end(args);

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING &&
	    d->character != ch &&
	    get_trust(d->character) >= level &&
	    !IS_SET(d->character->comm, COMM_NOINFO))
		send_to_char(buf, d->character);
    }
}

/* BO980707 - Quest info */

void do_info(CHAR_DATA * ch, char *argument)
{
    if (IS_SET(ch->comm, COMM_NOINFO)) {
	send_to_char("Info channel is now ON.\n\r", ch);
	REMOVE_BIT(ch->comm, COMM_NOINFO);
    } else {
	send_to_char("Info channel is now OFF.\n\r", ch);
	SET_BIT(ch->comm, COMM_NOINFO);
    }
}

void do_rpconv(CHAR_DATA * ch, char *argument)
{

    if (ch->rp_points >= 100) {
	ch->rp_points -= 100;
	ch->questpoints += 700;
	send_to_char("Conversion complete.\n\r", ch);
	return;
    }
    send_to_char("You don't have enough RP points.  (Need 100)\n\r", ch);
    return;
}

void do_szversion(CHAR_DATA * ch, char *argument)
{

    send_to_char("Suzuran's Version Code Is: ", ch);
    send_to_char(SUZURAN_VERSION_CODE, ch);
    send_to_char("\n\r", ch);
    return;
}


/* Chibichibify -- SZ011115 */
char *chibichibify(char *string){

  char buf[MAX_STRING_LENGTH];
  char chibistr[10];
  int pos=0,x=0;
  char temp;

  buf[0]=0; pos=0; x=0;
  sprintf(chibistr,"`%%Chibi``");

  do{
    /* Is this a word? */
    temp = toupper(*string);
    if((temp >= 'A') && (temp <= 'Z')){
      /* Yes.  Eat the word, replace with 'Chibi' */
      if(x == 0){
        /* FIXME-CRASHBUG: BUFFER OVERFLOW HERE, FIX LATER */

        /* ONLY DO THIS IF WE HAVE ROOM */
        /*     if(pos < (MAX_STRING_LENGTH - 10)){ */
        strcpy(&buf[pos],chibistr);
        pos += strlen(chibistr);
        /*        } */
        x=1;
      }
      /* Not a word */
    }else{
      /* Not a word, straight copy it. */
      x=0; buf[pos++]=*string;
    }

  }
  while(*string++);

  buf[pos]=0;
  strcpy(string,buf);

  // printf("Chibichibify: pos %d, msg %s\n",pos,buf);
  return(string);

}


/* TC961108
   Thierry Coutelier coutel@pt.lu

   Ask mob system -> online quests

  1. Add to mobiles in area files.
       T  <q_number>
       <word_list>~
       <request list>~
       <action list>~

   <q_number> = running number of quest
         Enables checking if a player did ask this question
          and enables multiple quest per mob.
   <word list> = list of words that will enable the request check
      words are separrated by ';'.
   <request list> = gold <amount> | q_number <number> | item <vnum item> |
           align <alignemnt of player>  | level <level> | none
        list can be multiple separated with ';' 
   <action list> = list of action a mob will perform separated by ';'
     action may be most of the player commands:
       kill (with no parameter will kill the player)
       s e w n d u    directions
       emote <text>
       say <text>
      give <xxx> <to> (if <to> is empty player is target)
   Example:
     T 20301
     treasure; gold; money; silver~
     gold 500; item 20350~
     say I'm not a traitor; kill~
     T 20302
     fulrach; king; sendar~
     q_number 20305; 
     say He is a great king even if some think he's a peasant;
     say You should ask him about his lost son~

 2. Players commands
    ask <mob> <what>
    <mob> = any mob in room
       if mob has no T field he will answer that he knows nothing about 
          <what>
       <what> is a word or sentence (evry word will be checked)
    bribe <mob> <item> <question>
      if question is not given the item will stay in players inv.
      item = gold, silver or any item in players inv.

*/

/* Data structures */
/* Structure to add to CHAR_DATA */
/*  Saved or not ??? */

typedef struct Q_LIST {
    unsigned int q_number;	/* number of a success full q_number */
    Q_LIST *next;		/* pointer to next q_number */
} Q_LIST;

/* Structure for mobile */

typedef struct M_QLIST {
    unsigned int q_number;	/* number of quest */
    char *word_list;		/* allocated at load */
    char *require_list;		/* allocated at load */
    char *action_list;		/* allocated at load */
    int successes;		/* number of times quest has been successful */
    M_QLIST *next;
} M_QLIST;

/* Modifications in files 
interpret.c
 add of  do_q_ask( CHAR_DATA *ch,  char *argument );
 add of  do_q_bribe( CHAR_DATA *ch, char *argument );
db.c
 loading of T field
   m_qlist = ->m_qlist;
   while( m_qlist->next != NULL ) m_qlist = m_qlist->next;
   nmql = alloc_perm( sizeof( struct M_QLIST ));
   m_qlist->next = nmql;
   ->m_qlist->q_number = fread_number( );
   ->m_qlist->word_list = fread_string( );  
   ->m_qlist->word_list = fread_string( );
   ->m_qlist->word_list = fread_string( );
   ->m_qlist->next = NULL;

*/

void do_q_ask(CHAR_DATA * ch, char *argument)
{
/* check if mob in same room */
/* check if mob sees player */
/* check if word in list */
/* check id condition are fullfilled */
/* make mob execute commands */
/* add q_number to player */
/* increment successes of quest */
}				/* do_q_ask */

void do_q_bribe {
/* same as do_q_ask but one more condition and item will
					   be removed from players invent */ }
/* do_q_bribe *//* Needed for both */ boole mq_check_name(M_QLIST * mql,
							  char *argument)
{
}				/* mq_check_name */

boole mq_check_condition(M_QLIST * mql, char *argumnet)
{
}				/* mq_check_name */

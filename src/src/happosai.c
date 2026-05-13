/* Happosai Spec-proc - (C) SGM 1999.  Version 1. */

/* Suzuran did this, but it's Darkseid's fault.  Blame him for it. */

/* The ROM license requirements apply here. */

/* Question - How does Happy get a girl's panties off when
   she's standing?  Does he pick them up, or what?
   I guess I need to go watch the anime. ^_^ */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"

bool spec_happosai(CHAR_DATA *ch)
{

  CHAR_DATA *victim;
  int happy_counter = 0;
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  OBJ_DATA *obj;
  int attempt;
  
  if(ch->position == POS_STANDING){
    
    /* Search for a target. */
    
    for(victim=ch->in_room->people; victim != NULL; victim=victim->next_in_room){
      
      /* Victim must not be myself */
      if(ch == victim){continue;}
      
      /* Victim must be female. */
      if(victim->sex != SEX_FEMALE){ continue; }
      
      /* Punt if in safe room.  Note that if the IMM HAS MADE HAPPY A
	 SENTINEL MOB THIS WILL HANG HIM FOREVER IN A SAFE ROOM!
	 It will not hang ROM however.
	 (Subtle hint there.) */
      
      if(is_safe(ch,victim)){continue;}
      
      /* Now, check eq (waist only) */
      if((obj = get_eq_char(victim, WEAR_WAIST)) != NULL){
	/* obj->name is the name.  Is the string in there?
	   Note that this might be UNIX-specific. */
	if(is_name("panties",obj->name)){
	  /* Now, swipe that... */
	  /* Happosai is very good at what he does and never fails. */
	  /* This is mostly because I'm too lazy to write failure code. */
	  obj_from_char(obj);
	  obj_to_char(obj,ch);
	  /* Now, let people know... */
	  /* Someone with a little more imagination can come up with
	     better messages. */
	  act("$n sneaks up behind $N, swipes her panties, and takes off running!",ch,NULL,victim,TO_NOTVICT);
	  act("$n just swiped your panties!",ch,NULL,victim,TO_VICT);
	  act("You got $N's panties!  Run for it!",ch,NULL,victim,TO_CHAR);
	  /* And so we run for it... */
	  
	  /* Running from someone.  For 5 rooms, haul ass.
	     Happosai is very fast and so nobody can see him running.
	     You would have to run this as a state-machine (I.E. you would
	     need some form of a counter and would have to trigger running
	     every tick, I.E. you would need to hack ROM a little more than
	     my target imms have time for. */
	  
	  /* Pick a direction and go for it. */
	  
	  while(happy_counter < 6){
	    
	    was_in = ch->in_room;
	    for ( attempt = 0; attempt < 6; attempt++ )
	      {
		EXIT_DATA *pexit;
		int door;
		
		door = number_door( );
		if ( ( pexit = was_in->exit[door] ) == 0
		     ||   pexit->u1.to_room == NULL
		     ||   IS_SET(pexit->exit_info, EX_CLOSED)
		     || ( IS_NPC(ch)
			  &&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	          continue;
		
		move_char( ch, door, FALSE );
		if ( ( now_in = ch->in_room ) == was_in )
		  continue;
		
		ch->in_room = was_in;
		act( "$n streaks off, running like greased lightning!", ch, NULL, NULL, TO_ROOM );
		ch->in_room = now_in;
		
		happy_counter++;
	      }
            send_to_char( "Running failed?\n\r", ch );
            happy_counter++;
          }
	}
      }else{
	/* Didn't have panties */
	continue;
      }
      /* Done running */
      return(TRUE);
    }
    /* Done stealing */
  }
  /* Not sitting or done */
  return(FALSE);
}	   


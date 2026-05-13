void do_fry(CHAR_DATA * ch, char *argument)
{

/* Hacked a little - Suzuran */

    char strsave[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    /* Fetch argument */
    argument = one_argument(argument, arg1);

    /* Look for no parm */
    if (arg1[0] == 0) {
	send_to_char("Target, please?\n\r", ch);
	return;
    }

    if (IS_NPC(ch)) {
	/* Switched imm or some other stupidity */
	send_to_char("Only players can fry people.\n\r", ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL) {
	send_to_char("That player is not here.\n\r", ch);
	return;
    }

    if (victim == ch) {
	/* This message sucks!  I'll change it. */
	send_to_char("Use delete for that, it makes less mess.\n\r", ch);
	return;
    }

    /* Level restriction. */
    if (ch->level < 99 && victim->level > 20) {
	send_to_char
	    ("Only immortals level 99 or higher can delete 20s and above.\n\r",
	     ch);
	return;
    }

    /* Send messages, MANUALLY get rid of the character, and delete the
       pfile.  Forcing a quit command DOES NOT WORK! */

    sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(victim->name));
    wiznet("$N has just been FRIED! `1-`!TOASTY`1-``", victim, NULL, 0, 0,
	   0);

    /* Make sure not to screw up fighting */
    stop_fighting(victim, TRUE);

    /* Entertaining messages to the world - This is safer than forcing
       an echo command.  It's a part of gecho, to everyone not in the
       same room as the victim, and not to the fry-er. */

    for (d = descriptor_list; d; d = d->next) {
	if (d->connected == CON_PLAYING) {
	    if (d->character->in_room != victim->in_room
		&& d->character != ch) {
		send_to_char
		    ("A jagged streak of `&lightning`` flashes across the sky.\n\r",
		     d->character);
	    }
	}
    }

    /* To fry-ee */
    send_to_char
	("Out of nowhere a giant bolt of pure energy `&flashes`` out towards you. The bolt\n\r",
	 victim);
    send_to_char
	("engulfs your body in `$b`4l`$u`4e`` `!f`1l`!a`1m`!e`1s`A...``leaving only `Aashes`` where you stood!\n\r",
	 victim);

    /* To everyone else in the room with the victim */
    act
	("$N takes a massive bolt of `&lightning``!\n\rNothing is left but a puff of black smoke.",
	 victim, NULL, victim, TO_NOTVICT);

    /* And to the fry-er */
    send_to_char("Roasty-toasty!  Got any marshmallows?\n\r", ch);

    /* Get rid of the player or mob */
    if (IS_NPC(victim)) {
	extract_char(victim, TRUE);
	return;
    } else {
	/* Log this */
	sprintf(buf, "%s fried by %s.", victim->name, ch->name);
	log_string(buf);

	/* Lose the loser */
	d = victim->desc;
	extract_char(victim, TRUE);
	if (d != NULL) {
	    close_socket(d);
	}
    }

    /* and the pfile */
    unlink(strsave);

    /* All done. */
    return;
}

void do_stable(CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument(argument, arg);

    if ((victim = get_char_room(ch, arg)) == NULL) {
	send_to_char("They aren't there.\n\r", ch);
	return;
    }

    if (!IS_SET(ch->in_room->room_flags, ROOM_STABLE)) {
	send_to_char("But there's no stable here!\n\r", ch);
	return;
    }

    if (!IS_SET(victim->act, ACT_HORSE)) {
	send_to_char("You can't put that in a stable!\n\r", ch);
	return;
    }
    if (!IS_SET(victim->act, ACT_STABLED_HORSE)) {
	if (victim->master != ch) {
	    send_to_char("That's not your's to stable.\n\r", ch);
	    return;
	}

	if (victim->master == ch && ch->mount == IS_MOUNTED) {
	    send_to_char("You'll have to dismount first.\n\r", ch);
	    return;
	}

	SET_BIT(victim->act, ACT_STABLED_HORSE);
	sprintf(buf, "You stable %s.\n\r", victim->short_descr);
	send_to_char(buf, ch);
	sprintf(buf, "%s puts their %s in a stable.\n\r", ch->name,
		victim->short_descr);
	act(buf, ch, NULL, NULL, TO_ROOM);
    } else {
	if (victim->master != ch) {
	    send_to_char("That's not your's to unstable.\n\r", ch);
	    return;
	}

	REMOVE_BIT(victim->act, ACT_STABLED_HORSE);
	sprintf(buf, "You unstable %s.\n\r", victim->short_descr);
	send_to_char(buf, ch);
	sprintf(buf, "%s puts their %s in a stable.\n\r", ch->name,
		victim->short_descr);
	act(buf, ch, NULL, NULL, TO_ROOM);
    }
    return;
}

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"
#include "tables.h"
#include "recycle.h"

DECLARE_DO_FUN(do_autoauction);
DECLARE_DO_FUN(do_auction);

/* TG980801 - tab_channel abstracted out to tables.c, but can't 
 * seem to take out the enum dammit */
/* Dont forget: Answer has to be the last in list !!! */
/* The order of tab_channel in tables.c has to be the same as the enum !! */
enum e_channel
    { Gossip, Flame, Auction, Question, Music, Quote, Grats, Ooc, Answer };


/* TG980729 - redid do_auction for auctoauction */
void talk_auction(char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;


    sprintf(buf, "%s `8: `7'%s``'\n\r", tab_channel[Auction].text,
	    argument);

    for (d = descriptor_list; d != NULL; d = d->next) {

	original = d->original ? d->original : d->character;	/* if switched */

	if ((d->connected == CON_PLAYING)
	    && !IS_SET(original->comm, tab_channel[Auction].flag)
	    && !IS_SET(original->comm, COMM_QUIET)) {

	    act(buf, original, NULL, NULL, TO_CHAR);
	}
    }

}


/* put an item on auction, or see the stats on the current item or bet */
void do_autoauction(CHAR_DATA * ch, char *argument)
{
    OBJ_DATA *obj = NULL;
    AUCTION_LIST *pobj_list, *pobj_prev;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
	buf[MAX_STRING_LENGTH], arg3[MAX_INPUT_LENGTH];
    /*char log_buf[MAX_STRING_LENGTH]; */
    int ntoRemove = 0, count = 0, increment = 0;
    char *temparg = argument;	/* so in <mesg> option can just pass temparg instead of cat'ing 
				   * arg1 and argument back together */

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    pobj_list = pobj_prev = NULL;

    if (IS_NPC(ch))		/* NPC can be extracted at any time and thus can't auction! */
	return;

    if (arg1[0] == '\0') {
	if (!IS_IMMORTAL(ch)) {
	    send_to_char
		("Syntax: auction  <mesg> | off | show | list | remove <item number> |\n\r\
				item <item> [<min bid>] | <bet|bid> <amt>\n\r",
		 ch);
	    return;
	} else {
	    send_to_char
		("Syntax: auction  <mesg> | off | show | list | remove <item number> |\n\r\
				item <item> [<min bid>] | <bet|bid> <amt> | stop\n\r",
		 ch);
	    return;
	}
    } else if (!str_cmp(arg1, "show")) {
	if (auction_info->current_obj != NULL) {
	    /* show item data here */
	    if (auction_info->bet > 0)
		sprintf(buf, "Current bet on this item is %ld gold.\n\r",
			auction_info->bet);
	    else
		sprintf(buf,
			"No bets on this item have been received.\n\r");

	    send_to_char(buf, ch);

	    if (auction_info->current_obj->seller != ch)
		spell_identify(0, LEVEL_HERO - 1, ch,
			       auction_info->current_obj->item, TARGET_OBJ);	/* uuuh! */

	    return;
	} else {
	    send_to_char("There is nothing being auctioned now.\n\r", ch);
	    return;
	}
    } else if (!str_cmp(arg1, "off")) {
	if (IS_SET(ch->comm, tab_channel[Auction].flag)) {
	    sprintf(buf, "`%c%s`` channel is now `VON``.\n\r",
		    tab_channel[Auction].color, tab_channel[Auction].name);
	    send_to_char(buf, ch);
	    REMOVE_BIT(ch->comm, tab_channel[Auction].flag);
	    return;
	} else {
	    sprintf(buf, "`%c%s`` channel is now `4OFF``.\n\r",
		    tab_channel[Auction].color, tab_channel[Auction].name);
	    send_to_char(buf, ch);
	    SET_BIT(ch->comm, tab_channel[Auction].flag);
	    return;
	}
    } else if (IS_IMMORTAL(ch) && !str_cmp(arg1, "stop")) {
	if (auction_info->current_obj == NULL) {
	    send_to_char("There is no auction going on you can stop.\n\r",
			 ch);
	    return;
	} else {		/* stop the auction */
	    pobj_list = auction_info->current_obj;
	    sprintf(buf,
		    "Sale of %s has been stopped by God. Item confiscated.",
		    pobj_list->item->short_descr);
	    talk_auction(buf);
	    obj_to_char(pobj_list->item, ch);
	    auction_info->current_obj = auction_info->current_obj->next;
	    free_mem(pobj_list, sizeof(*pobj_list));

	    if (auction_info->buyer != NULL) {	/* return money to the buyer */
		auction_info->buyer->gold +=
		    auction_info->bet - auction_info->bank_bet;
		auction_info->buyer->bank += auction_info->bank_bet;
		send_to_char("Your money has been returned.\n\r",
			     auction_info->buyer);
	    }

	    /* resets the members of auction_info */
	    auction_info->bet = 0;
	    auction_info->bank_bet = 0;
	    auction_info->buyer = NULL;
	    auction_info->pulse = PULSE_AUCTION;
	    auction_info->going = 0;

	    if (auction_info->current_obj != NULL) {
		if (auction_info->current_obj->min_bid > 0) {
		    sprintf(buf,
			    "A new item has been received: %s.'\n\r`VM`8i`Vn B`8i`Vd `8: '`#%ld``",
			    auction_info->current_obj->item->short_descr,
			    auction_info->current_obj->min_bid);
		    talk_auction(buf);
		} else {
		    sprintf(buf, "A new item has been received: %s.",
			    auction_info->current_obj->item->short_descr);
		    talk_auction(buf);
		}
	    }
	    return;
	}
    } else if (!str_cmp(arg1, "list")) {
	BUFFER *output;

	output = new_buf();

	sprintf(buf, "`VA`8u`Vcti`8o`Vn `8L`Vi`8st`7.`&.`8.`V.``\n\r");
	add_buf(output, buf);
	sprintf(buf, "`8-------`7.`V-----`8-`&-`7-``\n\r");
	add_buf(output, buf);

	pobj_list = auction_info->current_obj;

	if (pobj_list != NULL)
	    pobj_list = pobj_list->next;

	while (pobj_list != NULL) {
	    if (pobj_list->min_bid > 0)
		sprintf(buf, "%s `8%d`@)`` %s\n\r    `VM`8B - `#%ld``\n\r",
			pobj_list->seller == ch ? "`^Û``" : " ", count++,
			pobj_list->item->short_descr, pobj_list->min_bid);
	    else
		sprintf(buf, "%s `8%d`@)`` %s\n\r",
			pobj_list->seller == ch ? "`^Û``" : " ", count++,
			pobj_list->item->short_descr);

	    add_buf(output, buf);
	    pobj_list = pobj_list->next;
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);
	return;
    }

    /* Rest of commands disabled by being charmed */
    if (IS_AFFECTED(ch, AFF_CHARM)) {
	send_to_char("You cannot use this command while charmed.\n\r", ch);
	return;
    }

    if (!str_cmp(arg1, "item")) {

	if (arg2[0] == '\0') {
	    send_to_char("What do you wish to auction?\n\r", ch);
	    return;
	}

	obj = get_obj_carry(ch, arg2);	/* does char have the item ? */

	if (obj == NULL) {
	    send_to_char("You aren't carrying that.\n\r", ch);
	    return;
	}

	if (obj->cost <= 2) {
	    send_to_char("That item isn't worth enough to auction.\n\r",
			 ch);
	    return;
	}

	if (IS_SET(obj->extra_flags, ITEM_NOAUCTION)) {
	    send_to_char("You `1can`!'`1t`` auction that.\n\r", ch);
	    return;
	}

	if (!can_drop_obj(ch, obj)) {
	    send_to_char("`#Ack`!!`` You can't let go of that item.\n\r",
			 ch);
	    return;
	}

	pobj_list = auction_info->current_obj;
	pobj_prev = NULL;

	if (pobj_list != NULL)
	    while ((pobj_list != NULL) && (count < MAX_AUCTION_ITEM)) {
		pobj_prev = pobj_list;
		pobj_list = pobj_list->next;
		count++;
	    }

	if (pobj_list == NULL) {
	    switch (obj->item_type) {

	    default:
		act("You cannot auction $Ts.", ch, NULL,
		    item_type_name(obj), TO_CHAR);
		return;
	    case ITEM_LIGHT:
	    case ITEM_SCROLL:
	    case ITEM_WAND:
	    case ITEM_STAFF:
	    case ITEM_WEAPON:
	    case ITEM_ARMOR:
	    case ITEM_POTION:
	    case ITEM_CLOTHING:
	    case ITEM_DRINK_CON:
	    case ITEM_PILL:
	    case ITEM_WARP_STONE:
	    case ITEM_GEM:
	    case ITEM_JEWELRY:
	    case ITEM_PISTOL:
	    case ITEM_SMG:
	    case ITEM_SHOTGUN:
	    case ITEM_RIFLE:
	    case ITEM_HEAVYGUN:
	    case ITEM_ENERGYGUN:
	    case ITEM_AMMO:
	    case ITEM_CLIP:
		obj_from_char(obj);
		pobj_list = alloc_mem(sizeof(*pobj_list));
		pobj_list->seller = ch;
		smash_bright_blue(obj->short_descr);
		pobj_list->item = obj;
		pobj_list->min_bid = (is_number(arg3)) ? atoi(arg3) : 0;
		pobj_list->next = NULL;

		if (pobj_prev == NULL)
		    auction_info->current_obj = pobj_list;
		else
		    pobj_prev->next = pobj_list;

		if (auction_info->current_obj == pobj_list) {
		    auction_info->bet = 0;
		    auction_info->bank_bet = 0;
		    auction_info->buyer = NULL;
		    auction_info->pulse = PULSE_AUCTION;
		    auction_info->going = 0;

		    if (auction_info->current_obj->min_bid > 0) {
			sprintf(buf,
				"A new item has been received: %s.'\n\r`VM`8i`Vn B`8i`Vd `8: '`#%ld``",
				obj->short_descr,
				auction_info->current_obj->min_bid);
			talk_auction(buf);
		    } else {
			sprintf(buf, "A new item has been received: %s.",
				obj->short_descr);
			talk_auction(buf);
		    }

		    return;
		} else {
		    send_to_char("Item recieved and put in list.\n\r", ch);
		    return;
		}
	    }			/* switch */
	} else {
	    send_to_char
		("Try again later - The list is full right now!\n\r", ch);
	    return;
	}
    } else if (!str_cmp(arg1, "remove")) {
	if (arg2[0] == '\0') {
	    send_to_char("Remove what item?\n\r", ch);
	    return;
	}

	if (is_number(arg2))
	    ntoRemove = atoi(arg2);
	else {
	    send_to_char
		("Ack! You must provide a numeric to remove an item from the list.\n\r",
		 ch);
	    return;
	}

	pobj_prev = auction_info->current_obj;	/* init this or remove 0 crashes */

	/* this starts you off on first item after the one being auctioned */
	if (auction_info->current_obj != NULL)
	    pobj_list = auction_info->current_obj->next;
	if (pobj_list == NULL) {
	    send_to_char("There are no items available to be removed.\n\r",
			 ch);
	    return;
	}

	/* find the item number you want */
	count = 0;
	while ((pobj_list != NULL) && (count < ntoRemove)) {
	    count++;
	    pobj_prev = pobj_list;
	    pobj_list = pobj_list->next;
	}

	/* i think this checks for infinite looping, but i forget now */
	if ((pobj_list == pobj_prev) && (pobj_prev != NULL)) {
	    bug("Autoauction:pobj_list == pobj_prev != NULL", 0);
	    return;
	}

	/*checks to see if the item is valid, and if so, if ch is owner */
	if (pobj_list != NULL) {
	    if (ch == pobj_list->seller) {
		/*extract item from list, place on char */
		act
		    ("The auctioneer appears before you to return $p to you.",
		     ch, pobj_list->item, NULL, TO_CHAR);
		act("The auctioneer appears before $n to return $p to $m.",
		    ch, pobj_list->item, NULL, TO_ROOM);
		obj_to_char(pobj_list->item, ch);

		pobj_prev->next = pobj_list->next;
		free_mem(pobj_list, sizeof(*pobj_list));
	    } else {
		send_to_char("That item isn't yours to extract!\n\r", ch);
		return;
	    }
	} else {
	    send_to_char("That item doesn't exist.\n\r", ch);
	    return;
	}

	return;
    } else if (!str_cmp(arg1, "bet") || !str_cmp(arg1, "bid")) {
	if (auction_info->current_obj != NULL) {
	    long newbet;

	    /* make - perhaps - a bet now */
	    if (arg2[0] == '\0') {
		send_to_char("Bet how much?\n\r", ch);
		return;
	    }

	    if (auction_info->current_obj->seller == ch) {
		send_to_char("You cannot bet on your own item.\n\r", ch);
		return;
	    }

	    if (is_number(arg2))
		newbet = atoi(arg2);
	    else {
		send_to_char("Ack! You must bet a number!\n\r", ch);
		return;
	    }

	    if(newbet == 0){
	      send_to_char("Now, that's not very nice... How about a nonzero bid?\n\r",ch);
	      return;
	    }

//            printf ("Bet: %d\n\r",newbet);


	    if (auction_info->bet > 0) {
		if (auction_info->bet >= 10) {
		    increment = auction_info->bet + auction_info->bet / 10;
		} else {
		    increment = 10;
		}
	    } else {
		increment = auction_info->current_obj->min_bid;
		if (increment == 0)
		    increment = 10;
	    }

	    if (newbet < increment) {
		sprintf(buf, "You must bid at least `&%d`7 for this item.\n\r", increment);
		send_to_char(buf, ch);
		return;
	    }

	    if (newbet > (ch->gold + ch->bank)) {
		send_to_char("You don't have that much money!\n\r", ch);
		return;
	    }

	    /* the actual bet is OK! */

	    /* return the gold to the last buyer, if one exists */
	    if (auction_info->buyer != NULL) {
		auction_info->buyer->bank += auction_info->bank_bet;
		auction_info->buyer->gold +=
		    auction_info->bet - auction_info->bank_bet;
	    }

	    if (newbet > ch->gold) {
		auction_info->bank_bet = newbet - ch->gold;
		ch->bank -= auction_info->bank_bet;
		ch->gold = 0;
	    } else {
		auction_info->bank_bet = 0;
		ch->gold -= newbet;	/* subtract the gold - important :) */
	    }

	    auction_info->buyer = ch;
	    auction_info->bet = newbet;
	    auction_info->going = 0;
	    auction_info->pulse = PULSE_AUCTION;	/* start the auction over again */

	    sprintf(buf, "A bet of %ld gold has been received on %s.\n\r",
		    newbet, auction_info->current_obj->item->short_descr);
	    talk_auction(buf);
	    return;


	} else {
	    send_to_char
		("There isn't anything being auctioned right now.\n\r",
		 ch);
	    return;
	}
    }

/* finally... speak your piece on auction channel */

    do_auction(ch, temparg);
    return;

}

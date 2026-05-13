CC      = gcc
PROF    = -O 
NOCRYPT =
C_FLAGS =  -ggdb -Wall $(PROF) $(NOCRYPT) 
L_FLAGS =  $(PROF)

O_FILES = act_comm.o act_enter.o act_info.o act_move.o act_obj.o act_wiz.o \
	alias.o auction.o ban.o comm.o const.o db.o db2.o effects.o fight.o \
	fight2.o flags.o handler.o healer.o interp.o note.o lookup.o magic.o \
	magic2.o music.o quest.o recycle.o save.o scan.o skills.o song.o \
	special.o tables.o update.o drunk2.o bets.o check.o chrskill1.o \
	newclans.o happosai.o mob_cmds.o mob_prog.o olc.o olc_act.o \
	olc_save.o bit.o mem.o string.o olc_mpcode.o gun.o

rom: $(O_FILES)
	rm -f dev
	$(CC) $(L_FLAGS) -o rom $(O_FILES) -lcrypt

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:

	rm -r *.o
	chmod 777 *

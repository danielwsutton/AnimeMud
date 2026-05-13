struct flag_type {
	char   *name;
	int		bit;
	bool	settable;
	int		level;
};

struct position_type {
	char   *name;
	char   *short_name;
};

struct sex_type {
	char   *name;
};

struct size_type {
	char   *name;
};

/* BO98???? - Tongues */
struct race_talk_type {
	char   *race_talk;
};

struct struct_channel {
	char   *name;			/* Name of channel used for do_channel and in channel	*/
	char	color;			/* signe char defining the color of the text			*/
	char   *text;			/* text chown. ex: You Gossip ... An s is added			*/
	bool	output_name;	/* Does nothing for the moment							*/
	int		flag;			/* COMM_NOxxx flag										*/
};

struct        bit_type
{
  const   struct  flag_type *     table;
  char *                          help;
};

/* game tables */
extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type		sex_table[];
extern	const	struct	size_type		size_table[];

/* flag tables */
extern	const	struct	flag_type		act_flags[];
extern	const	struct	flag_type		plr_flags[];
extern	const	struct	flag_type		affect_flags[];
extern	const	struct	flag_type		off_flags[];
extern	const	struct	flag_type		imm_flags[];
extern	const	struct	flag_type		form_flags[];
extern	const	struct	flag_type		part_flags[];
extern	const	struct	flag_type		comm_flags[];
extern	const	struct	flag_type		extra_flags[];
extern	const	struct	flag_type		wear_flags[];
extern	const	struct	flag_type		weapon_flags[];
extern	const	struct	flag_type		container_flags[];
extern	const	struct	flag_type		portal_flags[];
extern	const	struct	flag_type		room_flags[];
extern	const	struct	flag_type		exit_flags[];
extern        const   struct  flag_type       mprog_flags[];
extern        const   struct  flag_type       area_flags[];
extern        const   struct  flag_type       sector_flags[];
extern        const   struct  flag_type       door_resets[];
extern        const   struct  flag_type       wear_loc_strings[];
extern        const   struct  flag_type       wear_loc_flags[];
extern        const   struct  flag_type       res_flags[];
extern        const   struct  flag_type       imm_flags[];
extern        const   struct  flag_type       vuln_flags[];
extern        const   struct  flag_type       type_flags[];
extern        const   struct  flag_type       apply_flags[];
extern        const   struct  flag_type       sex_flags[];
extern        const   struct  flag_type       furniture_flags[];
extern        const   struct  flag_type       weapon_class[];
extern        const   struct  flag_type       apply_types[];
extern        const   struct  flag_type       weapon_type2[];
extern        const   struct  flag_type       apply_types[];
extern        const   struct  flag_type       size_flags[];
extern        const   struct  flag_type       position_flags[];
extern        const   struct  flag_type       ac_type[];
extern        const   struct  bit_type        bitvector_type[];


/* TG970913 */

/* TG980801 */ /* moved from act_comm.c to use in act_obj.c for autoauction */
extern  const	struct	struct_channel		tab_channel[];

/* BO98???? - Tongues */
extern  const   struct  race_talk_type  race_talk_table[MAX_PC_RACE];




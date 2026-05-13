/*

	This .h or .c file is part of a Rom 2.4b2 code written by
Russel Taylor.  It has been further enhanced and edited by Mical
and Kyler.  MOST bugs are removed from this release, and color has
been added.  It also has several new features, and many changes
from the original code.  There are no back doors, and few bugs
left in the code.

	Kyler and I ask that if you use this code base, you add
that this code is greatly enhanced from Rom 2.4b2, along with
Russel Taylor's name.

Mical/Kyler

*/

/* vals from db.c */
extern bool fBootDb;
extern int		newmobs;
extern int		newobjs;
extern MOB_INDEX_DATA 	* mob_index_hash          [MAX_KEY_HASH];
extern OBJ_INDEX_DATA 	* obj_index_hash          [MAX_KEY_HASH];
extern int		top_mob_index;
extern int		top_obj_index;
extern int  		top_affect;
extern int		top_ed; 
extern AREA_DATA 	* area_first;


/* from db2.c */
extern int	social_count;

/* conversion from db.h */
void	convert_mob(MOB_INDEX_DATA *mob);
void	convert_obj(OBJ_INDEX_DATA *obj);

/* macro for flag swapping */
#define GET_UNSET(flag1,flag2)	(~(flag1)&((flag1)|(flag2)))

/* Magic number for memory allocation */
#define MAGIC_NUM 25571214

/* func from db.c */
extern void assign_area_vnum( int vnum );                    /* OLC */

/* from db2.c */

void convert_mobile( MOB_INDEX_DATA *pMobIndex );            /* OLC ROM */
void convert_objects( void );                                /* OLC ROM */
void convert_object( OBJ_INDEX_DATA *pObjIndex );            /* OLC ROM */


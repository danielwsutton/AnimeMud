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

int	clan_lookup	args( (const char *name) );
int	position_lookup	args( (const char *name) );
int 	sex_lookup	args( (const char *name) );
int 	size_lookup	args( (const char *name) );
int	flag_lookup	args( (const char *, const struct flag_type *) );

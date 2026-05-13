
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

#ifndef _ANSI_
#define _ANSI_
/* ANSI control sequences, to be include */
#define ESCAPE      ""
#define ANSI_NORMAL "[0m"
#define ANSI_BOLD   "[1m"
#define ANSI_UNDERL "[4m"
#define ANSI_BLINK  "[5m"
#define ANSI_INVERS "[7m"

#define ANSI_BLACK  "[0;30m"
#define ANSI_RED    "[0;31m"
#define ANSI_GREEN  "[0;32m"
#define ANSI_YELLOW "[0;33m"
#define ANSI_BLUE   "[0;34m"
#define ANSI_PURPLE "[0;35m"
#define ANSI_CYAN   "[0;36m"
#define ANSI_WHITE  "[0;37m"
#define ANSI_BOLD_RED    "[1;31m"
#define ANSI_BOLD_GREEN  "[1;32m"
#define ANSI_BOLD_YELLOW "[1;33m"
#define ANSI_BOLD_BLUE   "[1;34m"
#define ANSI_BOLD_PURPLE "[1;35m"
#define ANSI_BOLD_CYAN   "[1;36m"
#define ANSI_BOLD_WHITE  "[1;37m"

#define ANSI_CLS    "[2J"
#define ANSI_HOME   "[1;1H"
#endif

char *color_table[]=
{
/*0*/  "[0;31m",/* 0x0 */
/*1*/  "[0;32m",
/*2*/  "[0;33m",
/*3*/  "[0;34m",
/*4*/  "[0;35m", 
/*5*/  "[0;36m", /*0x5*/
/*6*/  "[0;37m", /*0x6 white */ 
/*7*/  "[1;30m",
/*8*/  "[1;31m",
/*9*/  "[1;32m",
/*10*/  "[1;33m", 
/*11*/  "[1;34m", /*0xb*/
/*12*/  "[1;35m",
/*13*/  "[1;36m",
/*14*/  "[1;37m", /*0xe*/
/*15*/  "[5m",
/*16*/  "[7m",
/*17*/  "[4m",
/*18*/  "[2J",
/*19*/  "[1;1H",
/*20*/  "\a",
0
};
 
char *color_symbol[]=
{
 "1","2","3","4","5","6","7","8","!","@","#","$","%","^","&",0
}; 

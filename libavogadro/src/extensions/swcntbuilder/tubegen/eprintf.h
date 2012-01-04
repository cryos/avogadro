/* ===============================================================================
// eprintf.h													  �2001, J.T. Frey
// ===============================================================================
// Written:		J.T. Frey, 07.04.2001
// Purpose:		Extended formatted printing functions for C.  These functions add
//				additional codes to the standard C format string for the printf,
//				fprintf, and sprintf functions which allow you to embed ANSI
//				background, foreground, and font face directives in your strings.
//				The directives appear as '%@[directive]' codes.
//
//				Foreground colors:	Background colors:	Text faces:
//				=================================================================
//			    %@black;			%@Black;			%@bold;
//			    %@red;				%@Red;				%@normal;
//			    %@green;			%@Green;			%@italic;
//			    %@yellow;			%@Yellow;			%@underline;
//			    %@blue;				%@Blue;				%@blink;
//			    %@magenta;			%@Magenta;			%@rapidBlink;
//			    %@cyan;				%@Cyan;				%@reverse;
//			    %@white;			%@White;			%@invisible;
//
//				The %@reset; tag resets to default formatting.
//
// Last Mod:	n/a
*/

#ifndef __EPRINTF__
#define __EPRINTF__

// Removed for Avogadro:
// #include <stdc-includes.h>
//
// Replaced with:
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

    int eprintf(const char*,...);
    int efprintf(FILE*,const char*,...);
    int esprintf(char*,const char*,...);

#if defined(__cplusplus)
}
#endif

#endif /* __EPRINTF__  */

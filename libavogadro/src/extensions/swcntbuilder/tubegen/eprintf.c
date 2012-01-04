/* ===============================================================================
// eprintf.c													  �2001, J.T. Frey
// ===============================================================================
// Written:		J.T. Frey, 07.04.2001
// Purpose:		ANSI formatting additions to the printf, fprintf, and sprintf
//				functions.
//
// Last Mod:	n/a
*/

#include "eprintf.h"
#include <stdarg.h> // Avogadro edit: Needed for va_arg --DCL 2011/11/11

/*
 * NOTES:
 *
 * All of the formatting we're defining is done in terms of
 * numerical codes which are attached to an escape character
 * sequence.  The following data structure sets up a sorted
 * array of key/number pairs.  The number is the
 * corresponding ANSI code.  The directives defined in the
 * array are sorted such that a binary search can be done
 * based on the first character; the standard string compare
 * function is used to make the final decision.
 * These directives ARE CASE SENSITIVE!!!
 */

    struct keyedData {
      const char*		key;
      int				data;
    };
    
    #define				numPairs 25
    struct keyedData	pairs[] =		{
                                            { "Black",			40 },
                                            { "Blue",			44 },
                                            { "Cyan",			46 },
                                            { "Green",			42 },
                                            { "Magenta",		45 },
                                            { "Red",			41 },
                                            { "Yellow",			43 },
                                            { "White",			47 },
                                            { "black",			30 },
                                            { "blink",			 5 },
                                            { "blue",			34 },
                                            { "bold",			 1 },
                                            { "cyan",			36 },
                                            { "green",			32 },
                                            { "invisible",		 8 },
                                            { "italic",			 3 },
                                            { "magenta",		35 },
                                            { "normal",			 2 },
                                            { "rapidBlink",		 6 },
                                            { "red",			31 },
                                            { "reset",			 0 },
                                            { "reverse",		 7 },
                                            { "underline",		 4 },
                                            { "white",			37 },
                                            { "yellow",			33 }
                                        };

/**/

	/* ----------------------------------------------------------------------
	// * __fillFormattingBuffer									 [local only]
	// ----------------------------------------------------------------------
	// Written:		Jeff Frey, 07.04.2001
	// Purpose:		Translate the 'directive' into an escape sequence.  On
    //				exit the 'formatBuffer' contains the sequence and the
    //				function returns the number of characters in the format
    //				buffer.
	//
	// Last Mod:	n/a
    */
    
    int __fillFormattingBuffer(const char*,char*);

    int __fillFormattingBuffer(
      const char*		directive,
      char*				formatBuffer
    )
    {
      int				lo = 0,hi = numPairs - 1,target,v;
      int				didLoEqualHiAlready = 0,found = 0;
      
      while (lo <= hi) {
        target = (hi + lo) / 2;
        v = *directive - *(pairs[target].key);
        /*  Positive 'v' means directive is above this one;
         *  Negative means it should be below it;
         *  Zero means we matched the first character, so we
         *    do a standard string compare to see if we got it.
         */
        if (v < 0)
          hi = target + ((hi + lo) % 2);
        else if (v > 0)
          lo = target + ((hi + lo) % 2);
        else {
          v = strcmp(directive,pairs[target].key);
          if (v == 0) {
            found = 1;
            break;
          }
          if (v < 0)
            hi = target + ((hi + lo) % 2);
          else
            lo = target + ((hi + lo) % 2);
        }
        if (hi == lo) {
          if (didLoEqualHiAlready)
            break;
          else
            didLoEqualHiAlready = 1;
        }
      }
      
      if (found) {
        return sprintf(formatBuffer,"\033[%dm",pairs[target].data);
      } else {
        /*  Unrecognized...empty buffer!  */
        formatBuffer[0] = '\0';
        return 0;
      }
    }

/**/

	/* ----------------------------------------------------------------------
	// * __parseExtendedFormatString							 [local only]
	// ----------------------------------------------------------------------
	// Written:		Jeff Frey, 07.04.2001
	// Purpose:		Parse an entire format string as passed to one of the
    //				eprintf-type functions.  This routine scannd for our
    //				directives and the directives are replaced inline with
    //				escape sequences.  On exit, all entities which are of
    //				the form '%@..;' have been removed entirely or replaced
    //				with applicable escape sequences.
	//
	// Last Mod:	n/a
    */
    
    void __parseExtendedFormatString(char*);

    void __parseExtendedFormatString(
      char*				format
    )
    {
      char		formBuf[8];
      char*		p = format;
      int		wasEndOfString = 0;
      
      while (*p != '\0') {
        while ((*p != '\0') && (*p != '%')) p++;
        if (*p == '%') {
          /*  We found a percent sign; is it formatting or
           *  a regular old printf code?
           */
          if (*(++p) == '\0')
            break;
          if (*p == '@') {
            /*  It should be a formatting directive:  */
            char*	start = p + 1;
            char*	end = start;
            int		fillLen,prevLen = 3;
            
            /*  Isolate the formatting directive:  */
            while ((*end != '\0') && (*end != ';')) {
              end++;
              prevLen++;
            }
            if (*end)
              *end = '\0';
            else
              wasEndOfString = 1;
            
            /*  Fill the format buffer based on the directive:  */
            fillLen = __fillFormattingBuffer(start,formBuf);
            
            /*  Overwrite the directive with the format:  */
            start -= 2;
            if (fillLen > 0)
              memcpy(start,formBuf,fillLen);
            if (!wasEndOfString)
              memmove(start + fillLen,end + 1,strlen(end + 1) + 1);
            else
              *(start + fillLen) = '\0';
              
            /*  Move to the position following the now-formatted
             *  directive and we'll hit it next:
             */
            p = start + fillLen;
          }
        }
      }
    }

/**/

	/* ----------------------------------------------------------------------
	// * eprintf
	// ----------------------------------------------------------------------
	// Written:		Jeff Frey, 07.04.2001
	// Purpose:		Extended printf.  Makes a local copy of the format string
    //				which is then parsed for our ANSI directives and
    //				subsequently used with vprintf to do the actual output.
	//
	// Last Mod:	n/a
    */

    int eprintf(
      const char*		format,
      ...
    )
    {
      va_list			arguments;
      int				returnVal = -1,len;
      char*				formBuf;
      
      /*  Set us up to access the other arguments:  */
      va_start(arguments,format);
      
      if ((formBuf = (char*)(malloc(len = (strlen(format) + 1))))) {
        memcpy(formBuf,format,len);
        __parseExtendedFormatString(formBuf);
        returnVal = vprintf(formBuf,arguments);
        free(formBuf);
      }
      
      /*  End the argument list usage:  */
      va_end(arguments);
      return returnVal;
    }
    
/**/

	/* ----------------------------------------------------------------------
	// * efprintf
	// ----------------------------------------------------------------------
	// Written:		Jeff Frey, 07.04.2001
	// Purpose:		Extended fprintf.  Makes a local copy of the format
    //				string which is then parsed for our ANSI directives and
    //				subsequently used with vfprintf to do the actual output.
	//
	// Last Mod:	n/a
    */

    int efprintf(
      FILE*				stream,
      const char*		format,
      ...
    )
    {
      va_list			arguments;
      int				returnVal = -1,len;
      char*				formBuf;
      
      /*  Set us up to access the other arguments:  */
      va_start(arguments,format);
      
      if ((formBuf = (char*)(malloc(len = (strlen(format) + 1))))) {
        memcpy(formBuf,format,len);
        __parseExtendedFormatString(formBuf);
        returnVal = vfprintf(stream,formBuf,arguments);
        free(formBuf);
      }
      
      /*  End the argument list usage:  */
      va_end(arguments);
      return returnVal;
    }
    
/**/

	/* ----------------------------------------------------------------------
	// * esprintf
	// ----------------------------------------------------------------------
	// Written:		Jeff Frey, 07.04.2001
	// Purpose:		Extended sprintf.  Makes a local copy of the format
    //				string which is then parsed for our ANSI directives and
    //				subsequently used with vsprintf to do the actual output.
	//
	// Last Mod:	n/a
    */

    int esprintf(
      char*				buffer,
      const char*		format,
      ...
    )
    {
      va_list			arguments;
      int				returnVal = -1,len;
      char*				formBuf;
      
      /*  Set us up to access the other arguments:  */
      va_start(arguments,format);
      
      if ((formBuf = (char*)(malloc(len = (strlen(format) + 1))))) {
        memcpy(formBuf,format,len);
        __parseExtendedFormatString(formBuf);
        returnVal = vsprintf(buffer,formBuf,arguments);
        free(formBuf);
      }
      
      /*  End the argument list usage:  */
      va_end(arguments);
      return returnVal;
    }
    
/**/

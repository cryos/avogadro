// =============================================================
// ANSR.h																	 �1999-2002, J.T. Frey
// =============================================================
// Written:		J.T. Frey, 23.FEB.2002
// Purpose:		Atomic Number/Symbol Resolution class.
//
// Last Mod:	05.Feb.2003:	Removed all ostream::form calls and moved to purely
//													C++ stream operators.

#ifndef __ANSRDB__
#define __ANSRDB__

// Removed for Avogadro:
// #include <stdcxx-includes.h>
//
// Replaced with:
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

using namespace std;

//

	/////////////////////////////////////////////////////////
	// TElementSymbol - four-character entity, masquerading
	// as a simple 32-bit integer!  For any symbol/number
	// that we can't resolve, there's a constant, too:
	//
	typedef			uint32_t							TElementSymbol;
	
	#define     kANSRInvalidSymbol		(TElementSymbol)-1
	
//

	/////////////////////////////////////////////////////////
	// TElementInfo - data structure which contains an
	// element's information.
	//
	typedef struct TElementInfo {
		unsigned							atomicNumber;
	  TElementSymbol				chemSymbol;
		double								weight;
	} TElementInfo;
	
//

	/////////////////////////////////////////////////////////
	// ANSRDB - Atomic Number/Symbol Resolution Data Base
	//
	class ANSRDB {
	  public:
			ANSRDB();
			ANSRDB(const char* filepath);
			~ANSRDB();
      
      static ANSRDB* DefaultANSRDB()
        {
// Avogadro edit: This is hardcoded to a non-existant path, and is causing linker issues.
// Replaced with another nonexistent path.
//        extern char* defaultANSRDBPath;
		  char* defaultANSRDBPath = "";
          static ANSRDB* defaultANSRDBInstance = NULL;
          
          if (!defaultANSRDBInstance) {
            FILE*     fptr;
            
            if (fptr = fopen(defaultANSRDBPath,"r")) {
              fclose(fptr);
              defaultANSRDBInstance = new ANSRDB(defaultANSRDBPath);
              if (defaultANSRDBInstance->elementsInTable == 0) {
                delete defaultANSRDBInstance;
                defaultANSRDBInstance = NULL;
              }
            }
            if (!defaultANSRDBInstance)
              defaultANSRDBInstance = new ANSRDB();
          }
          return defaultANSRDBInstance;
        }
			
			static TElementSymbol MakeSymbolFromString(const char* string)
        {
          TElementSymbol	newSym = 0;
          unsigned        length = strlen(string);
        
          if (length) {
            char*         chars = (char*)&newSym;
            
            if (length > 3) length = 3;
            switch (length) {
              case 3:
                if (string[2] < 127 && isalpha(string[2]))
                  chars[2] = tolower(string[2]);
              case 2:
                if (string[1] < 127 && isalpha(string[1]))
                  chars[1] = tolower(string[1]);
              case 1:
                if (string[0] < 127 && isalpha(string[0]))
                  chars[0] = toupper(string[0]);
            }
          }
          return newSym;
        }
			
			TElementSymbol LookupSymbolForNumber(unsigned number);
			unsigned LookupNumberForSymbol(TElementSymbol symbol);
			TElementInfo* LookupElementInfoForNumber(unsigned number);
			TElementInfo* LookupElementInfoForSymbol(TElementSymbol symbol);
			
			void print(ostream& os);
			
		protected:
		  unsigned							elementsInTable;
		  TElementInfo*					elements;
		  unsigned*							lookupTable;
			
		private:
			int DidInitializeTables();
			int DidReadTableFromFile(const char* filepath);
	};

#endif

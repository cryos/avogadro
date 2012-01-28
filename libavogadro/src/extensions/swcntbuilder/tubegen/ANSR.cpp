// =============================================================
// ANSR.cpp                                ©1999-2002, J.T. Frey
// =============================================================
// Written:		J.T. Frey, 23.FEB.2002
// Purpose:		Atomic Number/Symbol Resolution class.
//
// Last Mod:	n/a

#include "ANSR.h"

// Added for Avogadro


//

  int __ANSRSymbolCmpBE(
    const void*     key,
    const void*     obj
  )
  {
    return (*((TElementSymbol*)key) - ((TElementInfo*)obj)->chemSymbol);
  }
  
  int __ANSRSymbolCmpLE(
    const void*     key,
    const void*     obj
  )
  {
    return strcmp((char*)key,(char*)(&((TElementInfo*)obj)->chemSymbol));
  }

//

#ifndef PKGDATADIR
# define PKGDATADIR "/usr/local/share/tubegen"
#endif

// Edit for avogadro on MSVC2008: This is causing linker errors, and is hardcoded
// to a non-existant path.
//  const char*       defaultANSRDBPath = PKGDATADIR "/periodic.table";


	unsigned					defaultTableSize = 36;
	TElementInfo			defaultTable_LittleEndian[] =	
                                {
                                  {13 , '\0\0lA' , 26.98},
                                  {18 , '\0\0rA' , 39.948},
                                  {33 , '\0\0sA' , 74.92},
                                  {5 , '\0\0\0B' , 10.81},
                                  {4 , '\0\0eB' , 9.012},
                                  {35 , '\0\0rB' , 79.904},
                                  {6 , '\0\0\0C' , 12.011},
                                  {20 , '\0\0aC' , 40.08},
                                  {17 , '\0\0lC' , 35.453},
                                  {27 , '\0\0oC' , 58.93},
                                  {24 , '\0\0rC' , 52},
                                  {29 , '\0\0uC' , 63.55},
                                  {9 , '\0\0\0F' , 18.998},
                                  {26 , '\0\0eF' , 55.85},
                                  {31 , '\0\0aG' , 69.72},
                                  {32 , '\0\0eG' , 72.61},
                                  {1 , '\0\0\0H' , 1.0079},
                                  {2 , '\0\0eH' , 4.0026},
                                  {19 , '\0\0\0K' , 39.098},
                                  {36 , '\0\0rK' , 83.8},
                                  {3 , '\0\0iL' , 6.941},
                                  {12 , '\0\0gM' , 24.31},
                                  {25 , '\0\0nM' , 54.94},
                                  {7 , '\0\0\0N' , 14.007},
                                  {11 , '\0\0aN' , 22.99},
                                  {10 , '\0\0eN' , 20.179},
                                  {28 , '\0\0iN' , 58.7},
                                  {8 , '\0\0\0O' , 15.999},
                                  {15 , '\0\0\0P' , 30.974},
                                  {16 , '\0\0\0S' , 32.066},
                                  {21 , '\0\0cS' , 44.96},
                                  {34 , '\0\0eS' , 78.96},
                                  {14 , '\0\0iS' , 28.09},
                                  {22 , '\0\0iT' , 47.88},
                                  {23 , '\0\0\0V' , 50.94},
                                  {30 , '\0\0nZ' , 65.39}
                                };
	TElementInfo			defaultTable_BigEndian[] =
                                {
                                  {13 , 'Al\0\0' , 26.98},
                                  {18 , 'Ar\0\0' , 39.948},
                                  {33 , 'As\0\0' , 74.92},
                                  {5 , 'B\0\0\0' , 10.81},
                                  {4 , 'Be\0\0' , 9.012},
                                  {35 , 'Br\0\0' , 79.904},
                                  {6 , 'C\0\0\0' , 12.011},
                                  {20 , 'Ca\0\0' , 40.08},
                                  {17 , 'Cl\0\0' , 35.453},
                                  {27 , 'Co\0\0' , 58.93},
                                  {24 , 'Cr\0\0' , 52},
                                  {29 , 'Cu\0\0' , 63.55},
                                  {9 , 'F\0\0\0' , 18.998},
                                  {26 , 'Fe\0\0' , 55.85},
                                  {31 , 'Ga\0\0' , 69.72},
                                  {32 , 'Ge\0\0' , 72.61},
                                  {1 , 'H\0\0\0' , 1.0079},
                                  {2 , 'He\0\0' , 4.0026},
                                  {19 , 'K\0\0\0' , 39.098},
                                  {36 , 'Kr\0\0' , 83.8},
                                  {3 , 'Li\0\0' , 6.941},
                                  {12 , 'Mg\0\0' , 24.31},
                                  {25 , 'Mn\0\0' , 54.94},
                                  {7 , 'N\0\0\0' , 14.007},
                                  {11 , 'Na\0\0' , 22.99},
                                  {10 , 'Ne\0\0' , 20.179},
                                  {28 , 'Ni\0\0' , 58.7},
                                  {8 , 'O\0\0\0' , 15.999},
                                  {15 , 'P\0\0\0' , 30.974},
                                  {16 , 'S\0\0\0' , 32.066},
                                  {21 , 'Sc\0\0' , 44.96},
                                  {34 , 'Se\0\0' , 78.96},
                                  {14 , 'Si\0\0' , 28.09},
                                  {22 , 'Ti\0\0' , 47.88},
                                  {23 , 'V\0\0\0' , 50.94},
                                  {30 , 'Zn\0\0' , 65.39}
                                };
	unsigned					defaultLookup[]	=	{ 16 , 17 , 20 , 4 , 3 , 6 , 23 , 27 , 12 , \
                                        25 , 24 , 21 , 0 , 32 , 28 , 29 , 8 , 1 , \
                                        18 , 7 , 30 , 33 , 34 , 10 , 22 , 13 , 9 , \
                                        26 , 11 , 35 , 14 , 15 , 2 , 31 , 5 , 19 };

//
	
	/////////////////////////////////////////////////////////
	// Default constructor.
	//
	ANSRDB::ANSRDB()
	{
    uint16_t    test = 0x1234;
    
		elementsInTable = defaultTableSize;
    if (*((char*)&test) == 0x34)
      elements = (TElementInfo*)&defaultTable_LittleEndian;
    else
      elements = (TElementInfo*)&defaultTable_BigEndian;
		lookupTable = (unsigned*)&defaultLookup;
	}
	
//

	/////////////////////////////////////////////////////////
	// File-based constructor.
	//
	ANSRDB::ANSRDB(
		const char*		filepath
	)
	{
		if (!DidReadTableFromFile(filepath)) {
      uint16_t    test = 0x1234;
      
		  elementsInTable = defaultTableSize;
      if (*((char*)&test) == 0x34)
        elements = (TElementInfo*)&defaultTable_LittleEndian;
      else
        elements = (TElementInfo*)&defaultTable_BigEndian;
		  lookupTable = (unsigned*)&defaultLookup;
		}
	}
	
//
	
	/////////////////////////////////////////////////////////
	// Default destructor.
	//
	ANSRDB::~ANSRDB()
	{
		if ((elements) && (elements != (TElementInfo*)&defaultTable_BigEndian) && \
        (elements != (TElementInfo*)&defaultTable_LittleEndian))
			free(elements);
		if ((lookupTable) && (lookupTable != (unsigned*)&defaultLookup))
			free(lookupTable);
	}
	
//

	/////////////////////////////////////////////////////////
	// Lookup the chemical symbol for the given atomic
	// number.
	//
	TElementSymbol
	ANSRDB::LookupSymbolForNumber(
		unsigned			number
	)
	{
		if ((number == 0) || (number > elementsInTable))
		  return kANSRInvalidSymbol;
		return elements[lookupTable[number - 1]].chemSymbol;
	}
	
//

	/////////////////////////////////////////////////////////
	// Lookup the atomic number for the given chemical
	// symbol.
	//
	unsigned
	ANSRDB::LookupNumberForSymbol(
		TElementSymbol		symbol
	)
	{
    TElementInfo*     found;
    uint16_t          test = 0x1234;
		
    if (*((char*)&test) == 0x34)
      found = (TElementInfo*)bsearch(&symbol,elements,elementsInTable,sizeof(TElementInfo),__ANSRSymbolCmpLE);
    else
      found = (TElementInfo*)bsearch(&symbol,elements,elementsInTable,sizeof(TElementInfo),__ANSRSymbolCmpBE);
    if (found)
      return found->atomicNumber;
		return kANSRInvalidSymbol;
	}
	
//

	/////////////////////////////////////////////////////////
	// Lookup the element info record for the given atomic
	// number.
	//
	TElementInfo*
	ANSRDB::LookupElementInfoForNumber(
		unsigned			number
	)
	{
		if ((number == 0) || (number > elementsInTable))
		  return NULL;
		return &elements[lookupTable[number - 1]];
	}
	
//

	/////////////////////////////////////////////////////////
	// Lookup the element info record for the given chemical
	// symbol.
	//
	TElementInfo*
	ANSRDB::LookupElementInfoForSymbol(
		TElementSymbol		symbol
	)
	{
    TElementInfo*     found;
    uint16_t          test = 0x1234;
    
    if (*((char*)&test) == 0x34)
      return (TElementInfo*)bsearch(&symbol,elements,elementsInTable,sizeof(TElementInfo),__ANSRSymbolCmpLE);
    return (TElementInfo*)bsearch(&symbol,elements,elementsInTable,sizeof(TElementInfo),__ANSRSymbolCmpBE);
	}
	
//

	/////////////////////////////////////////////////////////
	// Summarize the table(s) to the specified stream.
	//
	void
	ANSRDB::print(
		ostream&		os
	)
	{
		unsigned			i;
		
		for ( i = 0 ; i < elementsInTable ; i++ )
		  os << i+1 << " { " << elements[i].atomicNumber << " , " << \
				(char*)(&elements[i].chemSymbol) << " , " << elements[i].weight << " }" << endl;
	}

//
	
	/////////////////////////////////////////////////////////
	// Allocate memory for the tables.  Returns 1 when
	// allocation was successful, 0 otherwise.
	//
	int
	ANSRDB::DidInitializeTables()
	{
		if (elementsInTable > 0) {
			elements = (TElementInfo*)calloc(elementsInTable,sizeof(TElementInfo));
			if (elements) {
			  if (lookupTable = (unsigned*)calloc(elementsInTable,sizeof(unsigned)))
				  return 1;
				free(elements);
			}
		}
		return 0;
	}
	
//

	/////////////////////////////////////////////////////////
	// Attempt to read an ANSRDB from a file.  Returns 1
	// if successful, 0 otherwise.
	//
	int
	ANSRDB::DidReadTableFromFile(
		const char* 		filepath
	)
	{
		FILE*					stream;
		unsigned			i;
		char					buffer[8];
		
		if (filepath == NULL)
		  return 0; //  NULL string error
		if ((stream = fopen(filepath,"r")) == NULL)
		  return 0; //  Cannot open file error
		if (((fscanf(stream,"%u",&elementsInTable)) != 1) || (elementsInTable == 0)) {
		  fclose(stream);
		  return 0; //  Bad table size error
		}
		if (!DidInitializeTables()) {
		  fclose(stream);
			return 0; //  Memory allocation error
		}
		
		for ( i = 0 ; i < elementsInTable ; i++ ) {
		  //  Read an entry:
		  unsigned					tmpNum;
		  TElementSymbol		tmpSym = 0;
		  double						tmpWt;
		  unsigned					j,k;
		  TElementInfo			tmpEnt;
			int								err;
      uint16_t					test = 0x1234;
		  
		  if (( err = fscanf(stream,"%d %3s %lf",&tmpNum,buffer,&tmpWt) ) != 3) {
			  fprintf(stderr,"Error(%d @ %d): %d %s %lf\n",err,i,tmpNum,buffer,tmpWt);
				fclose(stream);
		    return 0;
			}
			tmpSym = ANSRDB::MakeSymbolFromString(buffer);
			
      if (*((char*)&test) == 0x34) {
				for ( j = 0 ; j < i ; j++ ) {
					//  The table is sorted alphabetically according to the
					//  symbols, so find where to insert this one:
					if ( strcmp((char*)(&(elements[j].chemSymbol)), (char*)(&tmpSym)) > 0 )
						break;
				}
			} else {
				for ( j = 0 ; j < i ; j++ ) {
					//  The table is sorted alphabetically according to the
					//  symbols, so find where to insert this one:
					if (elements[j].chemSymbol > tmpSym)
						break;
				}
			}
			//  Move everything FROM the insertion element TO the end
			//  of the table up one space if necessary:
		  if (j < i)
		    for ( k = i ; k > j ; k-- )
					elements[k] = elements[k - 1];
			//  Assign the new data:
		  elements[j].chemSymbol		= tmpSym;
		  elements[j].atomicNumber	= tmpNum;
		  elements[j].weight				= tmpWt;
		}
		//  Index the symbol table with respect to atomic number,
		//  so we can do quick lookups:
		for ( i = 0 ; i < elementsInTable ; i++ )
		  lookupTable[elements[i].atomicNumber - 1] = i;
			
		fclose(stream);
		
		//  Success!
		return 1;
	}

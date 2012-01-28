// ===============================================================================
// TubeGen.cpp                                               ©2001-2002, J.T. Frey
// ===============================================================================
// Written:   J.T. Frey, 07.26.2001
// Purpose:   Main program file for the TubeGen app.  This file contains the
//            routines which generate the atomic basis for a unit cell and the
//            k-point list for a unit cell.  Also contains the main program
//            which processes user commands.
//
// Last Mod:  20.FEB.2002: Switch to hexagonal unit cell; k-point generator is
//                         no longer necessary.
//            18.NOV.2002: Modifications to position generation loop [3.0.5]
//            03.DEC.2002: Cell expansion/contraction to preserve rolled C-C
//                         bond lengths added as an option [3.0.6]
//            14.DEC.2002: Further corrections to lattice relaxtion, introduction
//                         or TubuleBasis class [3.1]
//            28.OCT.2003: Added support for Gaussian 03 periodic boundary conditions
//                         and did some basic clean-up [3.1.1]
//            09.JUL.2004: Added support for PDB files [3.1.2]
//            02.Feb.2005: Added support for connectivity in PDB files and support
//                         for POV shape files [3.2]
//            17.Feb.2005: Added support for alternate elements in graphitic basis
//                         [3.3]
//            20.Feb.2005: Added support for MSI's .bgf format
//

//  Include some of our own headers:
#include "CrystalCell.h"
#include "CoreConst.h"

#include "ANSR.h"
#include "eprintf.h"
#include "TubuleBasis.h"

// Added for Avogadro
#include <fstream>
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

//

  //  The commands can only be this long:
  #define CMD_BUFFER_LEN    256
  
  //  For boolean stuff:
  #ifndef YES
    #define YES 1
  #endif
  #ifndef NO
    #define NO 0
  #endif
  
  #define     ANGSTROM_PER_BOHR         0.529

//

  // ----------------------------------------------------------------------
  // * mungeWhitespace
  // ----------------------------------------------------------------------
  // Updated:   Jeff Frey, 07.26.2001
  // Purpose:   Removes white space (space, tab, newline characters)
  //            from the head and tail of a string.
  //
  // Last Mod:  n/a
  
  void
  mungeWhitespace(
    char*   buffer
  )
  {
    char*   p1 = buffer;
    char*   p2 = buffer + strlen(buffer);
    size_t  l;
    
    while (isspace(*p1)) p1++;
    while (isspace(*(--p2)));
    l = p2 - p1 + 1;
    memmove(buffer,p1,l);
    buffer[l] = '\0';
  }
  
//

  // ----------------------------------------------------------------------
  // * main
  // ----------------------------------------------------------------------
  // Updated:   Jeff Frey, 07.26.2001
  // Purpose:   The entry point of the program.  This is one big long
  //            function which may be slightly bloated, but factoring it
  //            out into "chunks" and making the design a bit more
  //            modular would be of no true benefit.  Essentially, the
  //            program processes commands coming from a "comFile" while
  //            may be an actual file or may be the standard input stream.
  //
  // Last Mod:  08.DEC.2002:  Updated for version 3.1

  int main(int argc,const char * argv[])
  {
    char              command[CMD_BUFFER_LEN];
    FILE*             comFile = stdin;
    TubuleBasis       tube(3,3);
    CrystalCell*      theCell = NULL;
    int               printPrompt = YES;
    int               echoCommands = NO;
    unsigned          i = 1;
    TVector3D         v;
    
    //  If any argument is '--quiet' then we redirect stdout and stderr to
    //  /dev/null:
    while (i < argc) {
      if (strcmp(argv[i],"--quiet") == 0) {
        freopen("/dev/null","w",stdout);
        freopen("/dev/null","w",stderr);
        tube.SetOption_Verbose(false);
      }
      i++;
    }
    
    //  Make the tube object verbose now:
    tube.SetOption_Verbose(true);
    
    //  Are we interactive or not?
    if ((fseek(stdin,0,SEEK_END) == 0) && (ftell(stdin) > 0)) {
      rewind(stdin);
      echoCommands = YES;
    }
    
    //  Print out a header which shows version info, etc:
    eprintf("%@bold;TubeGen%@reset; - Carbon Nanotube Stucture Generator\n");
#if 0 // Removed for avogadro, causing build problems
    printf("Version " PACKAGE_VERSION " [%s %s]\n",__TIME__,__DATE__);
    printf("Copyright (c) 2000-2004, J. T. Frey\n\n");
#endif

    //  If the user specified a "script" file, open it; otherwise we stick
    //  to using stdin:
    if (argc > 1) {
      if ((comFile = fopen(argv[1],"r")) == NULL)
        comFile = stdin;
    }
    
    //  This loop runs as long as the quit/exit command is not given.  When
    //  that happens, a simple "break" statement pops us out!
    while(1) {
    
    
      
      //  Read the next command, drop the terminating newline
      //  character, drop any leading whitespace:
      if (printPrompt) {
        printf("> ");
        fflush(stdout);
      }
      
      fgets(command,CMD_BUFFER_LEN,comFile);
      if (feof(comFile)) break;
      
      if (command[strlen(command) - 1] == '\n')
        command[strlen(command) - 1] = '\0';
      mungeWhitespace(command);
        
      //  Watch for comments; the '#' sign at the head of a line
      //  indicates this!
      if (*command == '#') {
        printPrompt = NO;
        continue;
      }
      printPrompt = YES;
        
      //  Print the command if we're reading a script:
      if ((comFile != stdin) || (echoCommands))
        printf("%s\n",command);
      
      //  Scan the command:
      if ((*command == '?') || (strcmp(command,"help") == 0)) {
        //  Display help page
        printf("  Command Summary:\n");
        printf("    help                print this information\n");
        printf("    status              display state variables\n");
        printf("    set VARIABLE VALUE  reset a state variable\n");
        printf("    generate            create the cell and basis\n");
        printf("    ftranslate          apply a fractional translation to the atomic basis\n");
        printf("    ctranslate          apply a Cartesian translation to the atomic basis\n");
        printf("    print               print the cell to the display\n");
        printf("    save                save the cell to a file\n");
        printf("    exit                exit the program\n");
        printf("  Variables (default value appears first):\n");
        printf("    format              {");TubuleBasis::SummarizeFormatStrings();printf("}\n");
        printf("    units               {angstrom,bohr}\n");
        printf("    element{1,2}        # or symbol\n");
        printf("    bond                #\n");
        printf("    gutter              #,#,#\n");
        printf("    shape               {hexagonal,cubic,planar}\n");
        printf("    chirality           #,#\n");
        printf("    cell_count          #,#,#\n");
        printf("    relax_tube          {yes,no}\n");
      } else if ((strcmp(command,"exit") == 0) || (strcmp(command,"quit") == 0)) {
        //  Exit the program
        break;
      } else if (strcmp(command,"status") == 0) {
        TElementSymbol    symbol;
        unsigned          atomicNumber;
        
        //  Display state variables
        printf("  format            => %s\n",tube.StringForFormat());
        
        printf("  units             => ");
        switch(tube.GetOption_Units()) {
          case EUnits_Bohr:
            printf("bohr\n");
            break;
          case EUnits_Angstrom:
            printf("angstrom\n");
            break;
          default:
            printf("\n");
        }
        
        printf("  bond              => %lg\n",tube.Get_bond() * tube.GetUnitConversionFactor());
        
        printf("  element1          => ");
        atomicNumber = tube.Get_element(0);
        if ((symbol = (ANSRDB::DefaultANSRDB())->LookupSymbolForNumber(atomicNumber)) != kANSRInvalidSymbol)
          printf("%s\n",(char*)&symbol);
        else
          printf("%u\n",atomicNumber);
          
        printf("  element2          => ");
        atomicNumber = tube.Get_element(1);
        if ((symbol = (ANSRDB::DefaultANSRDB())->LookupSymbolForNumber(atomicNumber)) != kANSRInvalidSymbol)
          printf("%s\n",(char*)&symbol);
        else
          printf("%u\n",atomicNumber);
        
        v.x = tube.Get_gutter(EDirection_X) * tube.GetUnitConversionFactor();
        v.y = tube.Get_gutter(EDirection_Y) * tube.GetUnitConversionFactor();
        v.z = tube.Get_gutter(EDirection_Z) * tube.GetUnitConversionFactor();
        printf("  gutter            => %lg,%lg,%lg\n",v.x,v.y,v.z);
        
        printf("  shape             => ");
        switch(tube.GetOption_Lattice()) {
          case ELattice_Hexagonal:
            printf("hexagonal (rolled)\n");
            break;
          case ELattice_Cubic:
            printf("cubic (rolled)\n");
            break;
          case ELattice_Planar:
            printf("planar\n");
            break;
          default:
            printf("\n");
        }
        
        printf("  chirality         => %d,%d\n",tube.Get_n(),tube.Get_m());
        
        printf("  cell_count        => %d,%d,%d\n", \
            tube.Get_replication(EDirection_X), \
            tube.Get_replication(EDirection_Y), \
            tube.Get_replication(EDirection_Z)
        );
        
        printf("  relax_tube        => %s\n",((tube.GetOption_RelaxTubule())?("yes"):("no")));
        
        printf("    |-radius_conv   => %1.0lg\n",tube.GetOption_RadiusConv());
        printf("    |-error_conv    => %1.0lg\n",tube.GetOption_ErrorConv());
        printf("    |-gamma_conv    => %1.0lg\n",tube.GetOption_GammaConv());
      } else if (strstr(command,"set") == command) {
        //  Reset a state variable value
        char*   varName = command + 3;
        char*   end;
        char    saved;
        
        while ((*varName != '\0') && (!isalpha(*varName))) varName++;
        end = varName + 1;
        while ((*end != '\0') && ((isalpha(*end)) || (*end == '_'))) end++;
        saved = *end;
        *end = '\0';
        
////////////////////////////
        
        if (strcmp(varName,"radius_conv") == 0) {
          //  Radius convergence in tubule relaxation:
          double  tmp;
          
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isdigit(*varName))) varName++;
          if (sscanf(varName,"%lg",&tmp) == 1) {
            tube.SetOption_RadiusConv(tmp);
          } else
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
        }
        
////////////////////////////
        
        else if (strcmp(varName,"error_conv") == 0) {
          //  Error convergence in tubule relaxation:
          double  tmp;
          
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isdigit(*varName))) varName++;
          if (sscanf(varName,"%lg",&tmp) == 1) {
            tube.SetOption_ErrorConv(tmp);
          } else
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
        }
        
////////////////////////////
        
        else if (strcmp(varName,"gamma_conv") == 0) {
          //  Gamma convergence in tubule relaxation:
          double  tmp;
          
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isdigit(*varName))) varName++;
          if (sscanf(varName,"%lg",&tmp) == 1) {
            tube.SetOption_GammaConv(tmp);
          } else
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
        }
        
////////////////////////////
        
        else if (strcmp(varName,"format") == 0) {
          //  Exported file type:
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isalpha(*varName))) varName++;
          end = varName + 1;
          while ((*end != '\0') && ((isalnum(*end)) || (*end == '_'))) end++;
          
          if (!tube.SetOption_Format(varName))
            eprintf("  %@yellow;WARNING%@reset;:  Invalid output format: %s\n",varName);
        }
        
////////////////////////////
        
        else if (strcmp(varName,"units") == 0) {
          //  Units:
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isalpha(*varName))) varName++;
          end = varName + 1;
          while ((*end != '\0') && ((isalnum(*end)) || (*end == '_'))) end++;
          if (strcmp(varName,"bohr") == 0)
            tube.SetOption_Units(EUnits_Bohr);
          else if (strcmp(varName,"angstrom") == 0)
            tube.SetOption_Units(EUnits_Angstrom);
          else
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
        }
        
////////////////////////////
        
        else if (strcmp(varName,"bond") == 0) {
          //  Carbon-Carbon bond length:
          double  tmp;
          
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isdigit(*varName))) varName++;
          if (sscanf(varName,"%lg",&tmp) == 1) {
            tmp /= tube.GetUnitConversionFactor();
            tube.Set_bond(tmp);
          } else
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
        }
        
////////////////////////////
        
        else if (strstr(varName,"element") == varName) {
          //  Element:
          *end = saved;
          
          //  Find out which element we're looking at:
          varName += 7;
          if (*varName == '1' || *varName == '2') {
            unsigned          which = *varName - '1';
            TElementSymbol    symbol;
            unsigned          atomicNumber = 0;
            
            varName++;
            while ((*varName != '\0') && (!isalnum(*varName))) varName++;
            
            //  Try to turn the remainder of the string into a symbol:
            if ((symbol = ANSRDB::MakeSymbolFromString(varName)) != kANSRInvalidSymbol) {
              if ((atomicNumber = (ANSRDB::DefaultANSRDB())->LookupNumberForSymbol(symbol)) == kANSRInvalidSymbol)
                atomicNumber = 0;
            }
            if (!atomicNumber)
              sscanf(varName,"%u",&atomicNumber);
            
            //  If we have something non-zero in 'atomicNumber' we got an
            //  element!
            if (atomicNumber)
              tube.Set_element(which,atomicNumber);
            else
              eprintf("  %@yellow;WARNING%@reset;:  '%s' is not a valid element\n",varName);
          } else
            eprintf("  %@yellow;WARNING%@reset;:  Only indices of {1,2} are allowed for the 'element#' variable\n");
        }
        
////////////////////////////
        
        else if (strcmp(varName,"gutter") == 0) {
          //  Unit cell padding (gutter):
          double  x,y,z;
          int   c;
          
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isdigit(*varName))) varName++;
          c = sscanf(varName,"%lg,%lg,%lg",&x,&y,&z);
          if (c >= 1) {
            x /= tube.GetUnitConversionFactor();
            tube.Set_gutter(EDirection_X,x);
          }
          if (c >= 2) {
            y /= tube.GetUnitConversionFactor();
            tube.Set_gutter(EDirection_Y,y);
          }
          if (c >= 3) {
            z /= tube.GetUnitConversionFactor();
            tube.Set_gutter(EDirection_Z,z);
          }
          if (c < 1)
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
        }
        
////////////////////////////
        
        else if (strcmp(varName,"shape") == 0) {
          //  Lattice shape:
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isalpha(*varName))) varName++;
          end = varName + 1;
          while ((*end != '\0') && ((isalnum(*end)) || (*end == '_'))) end++;
          if (strcmp(varName,"hexagonal") == 0)
            tube.SetOption_Lattice(ELattice_Hexagonal);
          else if (strcmp(varName,"cubic") == 0)
            tube.SetOption_Lattice(ELattice_Cubic);
          else if (strcmp(varName,"planar") == 0)
            tube.SetOption_Lattice(ELattice_Planar);
          else
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
        }
        
////////////////////////////
        
        else if (strcmp(varName,"chirality") == 0) {
          //  Chirality parameters (n,m):
          int   tn,tm;
          int   c;
          
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isdigit(*varName))) varName++;
          c = sscanf(varName,"%u,%u",&tn,&tm);
          if (c == 1) {
            tube.Set_nandm(tn,tn);
          }
          if (c >= 2) {
            tube.Set_nandm(tn,tm);
          }
          if (c < 1)
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
        }
        
////////////////////////////
        
        else if (strcmp(varName,"cell_count") == 0) {
          //  How many cells to generate in each direction:
          unsigned  x,y,z;
          int   c;
          
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isdigit(*varName))) varName++;
          x = y = z = 0;
          c = sscanf(varName,"%u,%u,%u",&x,&y,&z);
          if (c < 1)
            eprintf("  %@yellow;WARNING%@reset;:  Invalid parameter value; variable not reset.\n");
          else {
            if (c >= 1)
              tube.Set_replication(EDirection_X,x);
            if (c >= 2)
              tube.Set_replication(EDirection_Y,y);
            if (c >= 3)
              tube.Set_replication(EDirection_Z,z);
          }
        }
        
////////////////////////////
        
        else if (strcmp(varName,"relax_tube") == 0) {
          //  Use expansion ratios in rolled cell:
          *end = saved;
          varName = end;
          while ((*varName != '\0') && (!isalpha(*varName))) varName++;
          end = varName + 1;
          while ((*end != '\0') && ((isalnum(*end)) || (*end == '_'))) end++;
          if (strcmp(varName,"yes") == 0)
            tube.SetOption_RelaxTubule(true);
          else if (strcmp(varName,"no") == 0)
            tube.SetOption_RelaxTubule(false);
          else
            eprintf("  %@yellow;WARNING%@reset;:  Please specify yes or no.\n");
        } else
          eprintf("  %@yellow;WARNING%@reset;:  Unknown variable.\n");
      }
      
      else if (strcmp(command,"generate") == 0) {
        //  Generate a unit cell; if one already exists, we overwrite it:
        if (theCell)
          delete theCell;
        theCell = tube.GenerateCrystalCell();
      }
      
      else if (strcmp(command,"print") == 0) {
        if (theCell)
          theCell->print(cout);
        else
          eprintf("  %@red;ERROR%@reset;:  No cell generated yet.\n");
      }
      
      else if (strstr(command,"save") == command) {
        //  Save a unit cell
        ofstream*   file = NULL;
        char*       fName = command + 4;
        char*       end;
        char        saved;
        
        if (theCell == NULL) {
          eprintf("  %@red;ERROR%@reset;:  No cell generated yet.\n");
        } else {
          while ((*fName != '\0') && (isspace(*fName))) fName++;
          if (*fName == '\0') {
            char    filename[32];
            int     charcnt = 0;
            
            switch (tube.GetOption_Lattice()) {
              case ELattice_Hexagonal:
              case ELattice_Cubic:
                charcnt = snprintf(filename,sizeof(filename),"%02d%02dr.",tube.Get_n(),tube.Get_m());
                break;
              case ELattice_Planar:
                charcnt = snprintf(filename,sizeof(filename),"%02d%02df.",tube.Get_n(),tube.Get_m());
                break;
              default:
                charcnt = snprintf(filename,sizeof(filename),"tubegen_out.");
                break;
            }
            snprintf(filename + charcnt,sizeof(filename) - charcnt,"%s",tube.FileExtensionsForFormat());
            file = new ofstream(filename);
          } else {
            end = fName + 1;
            while ((*end != '\0') && (!isspace(*end))) end++;
            *end = '\0';
            file = new ofstream(fName);
          }
          
          if (file) {
            tube.WriteInputFile(*file,theCell);
            delete file;
          }
        }
      }
      
      else if (strstr(command,"ftranslate") == command) {
        //  Apply a fractional translation:
        TVector3D   delta;
        int         c;
        char*       varName = command + 9;
        
        while ((*varName != '\0') && (!isdigit(*varName)) && (*varName != '-') && (*varName != '.') && (*varName != '+'))
          varName++;
        if (theCell) {
          c = sscanf(varName,"%lg,%lg,%lg",&delta.x,&delta.y,&delta.z);
          if (c == 3) {
            if ((delta.x > -1.0) && (delta.x < 1.0) && \
                (delta.y > -1.0) && (delta.y < 1.0) && \
                (delta.z > -1.0) && (delta.z < 1.0))
              theCell->ApplyFractionalTranslation(delta);
            else
              eprintf("  %@yellow;WARNING%@reset;:  Invalid translation vector; all values must be in (-1,1) in fractional coordinates.\n");
          } else
            eprintf("  %@yellow;WARNING%@reset;:  Invalid translation vector; transformation not done.\n");
        } else
          eprintf("  %@red;ERROR%@reset;:  No cell generated yet.\n");
      }
      
      else if (strstr(command,"ctranslate") == command) {
        //  Apply a cartesian translation:
        TVector3D   delta;
        int         c;
        char*       varName = command + 9;
        
        while ((*varName != '\0') && (!isdigit(*varName)) && (*varName != '-') && (*varName != '.') && (*varName != '+'))
          varName++;
        if (theCell) {
          c = sscanf(varName,"%lg,%lg,%lg",&delta.x,&delta.y,&delta.z);
          if (c == 3) {
            TVector3D   ctv = theCell->GetCellTranslationVector();
            
            //  If projection of the translation onto the cell translation vector
            //  is -1.0 < p < 1.0 then it's a vector within the proper displacement
            //  bounds:
            if ( fabs(Vector3D_Dot(&ctv,&delta) / Vector3D_Dot(&ctv,&ctv)) < 1.0)
              theCell->ApplyCartesianTranslation(delta);
            else
              eprintf("  %@yellow;WARNING%@reset;:  Invalid Cartesian translation vector.\n");
          } else
            eprintf("  %@yellow;WARNING%@reset;:  Invalid Cartesian translation vector.\n");
        } else
          eprintf("  %@red;ERROR%@reset;:  No cell generated yet.\n");
      }
      
      else if (*command != '\0')
        //  Command not understood
        eprintf("  %@red;ERROR%@reset;:  %s\n",command);
    }
    
    return 0;
  }


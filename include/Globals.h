//
// Global variables for the ROOT analyzer
//
// Name: Globals.h
//
// $Id$
//

// Run parameters

extern int  gRunNumber;
extern bool gIsRunning;
extern bool gIsPedestalsRun;
extern bool gIsOffline;
//extern int gUserMod;
//extern int gUserChan;

// Output files

//extern TFile* gOutputFile;
extern TFile* treeFile;
//extern FILE* ASCIIYY1;
// ODB access

#include "VirtualOdb.h"

extern VirtualOdb* gOdb;

// end


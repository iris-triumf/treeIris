// treeIris
// Analysis software for the IRIS experiment @TRIUMF
// based on
// ROOT analyzer
// by K.Olchanski

#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <signal.h>
#include <string>

#include "TMidasEvent.h"
//#include "TMidasFile.h"
#include "midasio.h"
//#include "XmlOdb.h"
#include "mvodb.h"

#include <TTimer.h>
#include <TFile.h>
#include <TTree.h>
#include "Globals.h"

#include "HandleMesytec.h"
#include "HandleV1190.h"
#include "HandleSTAT.h"
#include "HandleScaler.h"
#include "HandleV1740.h"

#include "IScaler.cxx"
#include "IDet.cxx"
#include "ITdc.cxx"

// Global Variables
int  gRunNumber = 0;
int  gEventNumber = 0;
int  gFileNumber = 0;
bool gIsPedestalsRun = false;
int  gEventCutoff = 0;
bool gUseTdc = false;
bool gUseRaw = false;
bool gUseDccp = false;
bool gRandomise = false; //Randomise ADC values within the channel when calibrating.
std::string gOutputFile = "";
std::string gCalibFile = ""; //File with energy calibrations.
std::string gTimeFile = "";  //File with time calibrations.

char mesbkname[][5] = {"ICA_", "SD2A", "SD1A", "YDA_", "SUA_", "YUA_"};
char tdcbkname[][5] = {"ICT_", "SD2T", "SD1T", "YDT_", "SUT_", "YUT_"};
char stabkname[][5] = {"VTST", "VTSA", "VTRT", "VTRA"};
char scalbkname[][5] = {"SCAD", "SCAR", "SCAS",""}; //scalers

TFile* treeFile = NULL;
TTree *tree = NULL;
//VirtualOdb* gOdb = NULL;
MVOdb *gOdb = NULL;

IDet detec; // calibrated variables from detectors, to be passed to HandlePhysics
IDet *pdet = &detec;
ITdc timeArray;
ITdc *ptdc = &timeArray;
IScaler scal;
IScaler *pscaler = &scal;

double GetTimeSec() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec + 0.000001*tv.tv_usec;
}

//--------------------------------------------------------------
void startRun(int run,int time)
{
	gRunNumber = run;
	//gIsPedestalsRun = gOdb->odbReadBool("/experiment/edit on start/Pedestals run");
	gOdb->RB("/experiment/edit on start/Pedestals run",&gIsPedestalsRun);
	printf("Begin run: %d, pedestal run: %d\n", gRunNumber, gIsPedestalsRun);
		
	if(treeFile!=NULL)
	{
	  	treeFile->Write();
	  	treeFile->Close();
	  	treeFile=NULL;
	}  
	
	char filename[1024];
	
	sprintf(filename, "%s", gOutputFile.data());
	if(gFileNumber==0) treeFile = new TFile(filename,"RECREATE");
	else  treeFile = new TFile(filename,"UPDATE");
	printf("Writing data to %s.\n",gOutputFile.data());
	
	HandleBOR_Mesytec(run, gFileNumber, time, pdet, gCalibFile);
	if(gUseTdc) HandleBOR_V1190(run, gFileNumber, gTimeFile, ptdc);
  HandleBOR_V1740(gFileNumber);
	HandleBOR_Scaler(run, gFileNumber, time, pscaler); 
	HandleBOR_STAT(run, time);
	if(gFileNumber==0){
		tree->Branch("Run",&gRunNumber,"Run/I");
		tree->Branch("Event",&gEventNumber,"Event/I");
	}
	else{
		tree->SetBranchAddress("Run",&gRunNumber);
		tree->SetBranchAddress("Event",&gEventNumber);
	}
	//gFileNumber++;
	printf("Started run.\n");
}

void endRun(int run,int time)
{
	gRunNumber = run;
	
	HandleEOR_Mesytec(run, time);
	if(gUseTdc) HandleEOR_V1190(run, time);
	HandleEOR_Scaler(run, time); 
	HandleEOR_STAT(run, time);
  HandleEOR_V1740();
 
	if (treeFile)
    {
      	tree->AutoSave();
      	treeFile->Close();
      	treeFile = NULL;
    }

  	printf("End of run %d\n",run);
}

//--------------------------------------------------------------
void HandleMidasEvent(TMidasEvent& event)
{
  	void *ptr;
  	int m;
  	int eventId = event.GetEventId();
  
  	// clear values here!
	clearDetectors();
  pdet->Clear();
  ptdc->Clear();
  	if ((eventId == 1)) {
    	//pdet->Clear();
    	//ptdc->Clear();
    	m=0;
    	while (mesbkname[m][0]) { 
    	  	int size = event.LocateBank(NULL, mesbkname[m], &ptr);
    	  	if (ptr && size) { 
				HandleMesytec(event, ptr, size, m, pdet, gCalibFile, gUseRaw); 
 			} 
    	  	m++;
 		}
		if(gUseTdc){
			m=0;
  			while (tdcbkname[m][0]) {
        //printf("HandleMidasEvent(): Handling bank: %s\n",tdcbkname[m]);
				int size = event.LocateBank(NULL, tdcbkname[m], &ptr);
				if (ptr && size) {
            //printf("Ok, the bank has size: %d\n",size);
			  		HandleV1190(event, ptr, size, m, ptdc); 
				}
				m++;
  			}
		}
    //XXX: Experiment with wfd.
    int size = event.LocateBank(NULL, "D740", &ptr);
    if (ptr && size) HandleV1740(event, ptr, size); 
  }
 	else if ((eventId == 3)) { // Scaler modules
      	m=0;
    	while (scalbkname[m][0]) { 
      		int size = event.LocateBank(NULL, scalbkname[m], &ptr);
      		if (ptr && size) { 
				HandleScaler(event, ptr, size, m, pscaler); 
      		} 
      		m++;
    	}
  	}

	if(treeFile) tree->Fill();
#if 0
  // Stat banks
  int size =  event.LocateBank(NULL, stabkname[3], &ptr);
  printf("bank %s:%d\n", tdcbkname[3], size);              
  if (ptr && size) {
    HandleSTAT(event, ptr, size, 0);
  }
#endif

}

//--------------------------------------------------------------
int ProcessMidasFile(const char*fname)
{
	char dcfname[100] = "dccp://";
   	strcat(dcfname,fname); // added dccp:// in front of fname, there is seg fault
  	//TMidasFile f;
  	TMReaderInterface *reader = TMNewReader(fname);
  	//bool tryOpen = false;
  	
	//if(gUseDccp) tryOpen = f.Open(dcfname);
	//else tryOpen = f.Open(fname);
 
  	//if (!tryOpen)
  	if(reader->fError)
   {
     printf("Cannot open input file \"%s\"\n",dcfname);
     delete reader;
     return -1;
   }

	while(1){
    	TMidasEvent event;
    	//if (!f.Read(&event))
    	if (!TMReadEvent(reader, &event))
		break;

    	int eventId = event.GetEventId();
    	if ((eventId & 0xFFFF) == 0x8000)
		{
			// begin run
			printf("---- BEGIN RUN ---- \n");
			// Load ODB contents from the ODB XML file
			if (gOdb) delete gOdb;
			//gOdb = new XmlOdb(event.GetData(),event.GetDataSize());
			gOdb = MakeXmlBufferOdb(event.GetData(),event.GetDataSize());
			startRun(event.GetSerialNumber(),0);
		}
    	else if ((eventId & 0xFFFF) == 0x8001)
		{
			printf("---- END RUN ---- \n");
			// end run
		}
    	else
		{
			event.SetBankList();
			HandleMidasEvent(event);
      //if(gEventNumber>10) break;
		}
    	//if(gEventNumber>100) break;
    	if((gEventNumber%100)==0)
		{
			printf("Processing event %d of run %d\r",gEventNumber,gRunNumber);
		}
    	gEventNumber++;
    	if ((gEventCutoff!=0)&&(gEventNumber>=gEventCutoff))
		{
			printf("Reached event %d, exiting loop.\n",gEventNumber);
			break;
		}
	}
  	//f.Close();
  	reader->Close();
   delete reader;
   reader = NULL;
  	endRun(gRunNumber,0);

  	return 0;
}

void help()
{
	printf("\nUsage:\n");
	printf("\n./bin/treeIris [-h] [-c][-o] [-eMaxEvents] [file1 file2 ...]\n");
	printf("\n");
	printf("\t-h: print this help message\n");
	printf("\t-o/-o=/--output=: Path of output file.\n");
	printf("\t-c/-c=/--config=: Path of main configuration file.\n");
	printf("\t-e: Number of events to read from input data files\n");
	printf("\t-tdc[=ARG]: Write TDC data to root file. The optional [ARG] is\n");
  printf("\t\tthe path to a file containing the time calibrations.\n");
	printf("\t-raw: Write raw ADC data to root file.\n");
	printf("\t-dccp: Use dccp file system.\n");
	printf("\t-rndm: Randomise ADC values within the channel when calibrating.\n");
	printf("\n");
	exit(1);
}

// Main function call

int main(int argc, char *argv[])
{
	setbuf(stdout,NULL);
	setbuf(stderr,NULL);
	
	signal(SIGILL,  SIG_DFL);
	signal(SIGBUS,  SIG_DFL);
	signal(SIGSEGV, SIG_DFL);

  
	std::vector<std::string> args;
	for (int i=0; i<argc; i++)
	{
		if (strcmp(argv[i],"-h")==0) help(); // does not return
	    args.push_back(argv[i]);
	}
	
	bool have_output= false;

 	for (unsigned int i=1; i<args.size(); i++) // loop over the commandline options
 	{
       	const char* arg = args[i].c_str();
	   
        if (strncmp(arg,"-e",2)==0)  // Event cutoff flag (only applicable in offline mode)
	 		gEventCutoff = atoi(arg+2);
       	else if (strncmp(arg,"-o=",3)==0){  // Output file 
	 		gOutputFile = arg+3;
			have_output=true;
		}
       	else if (strncmp(arg,"-o",2)==0){  // Output file 
	 		gOutputFile = arg+2;
			have_output=true;
		}
      	else if (strncmp(arg,"--output=",9)==0){  // Output file 
	 		gOutputFile = arg+9;
			have_output=true;
		}
		else if (strncmp(arg,"-c=",3)==0){  // Calibration file 
	 		gCalibFile = arg+3;
		}
       	else if (strncmp(arg,"-c",2)==0){  // Calibration file 
	 		gCalibFile = arg+2;
		}
      	else if (strncmp(arg,"--config=",9)==0){  // Calibration file 
	 		gCalibFile = arg+9;
		}
    else if (strncmp(arg,"-tdc",4)==0){
      //printf("argument = %s\n",arg);
      gUseTdc=true;
      if(strncmp(arg,"-tdc=",5)==0){
        gTimeFile = arg+5; //optional calibration file.
        //printf("gTimeFile = %s\n",gTimeFile.c_str());
      }
    }
    else if (strcmp(arg,"-raw")==0)
      gUseRaw=true;
    else if (strcmp(arg,"-dccp")==0)
	 	  gUseDccp=true;
    else if (strcmp(arg,"-h")==0)
      help(); // does not return
    else if(strcmp(arg,"-rndm")==0) gRandomise = true;
    else if (arg[0] == '-')
	 		help(); // does not return
	}
	
	for (unsigned int i=1; i<args.size(); i++)
    {
       	const char* arg = args[i].c_str();

      	if (arg[0] != '-')  
	 	{  
			if(have_output==true){	   
				ProcessMidasFile(arg);
			}
			else{
				printf("No output file specified!\n");
				printf("Use -o=/path/to/your/output-file!\n");
	 		}
     	}
	}

   	return 0;
}
//end

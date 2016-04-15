//
// ROOT analyzer
//
// K.Olchanski
//
// $Id: analyzer.cxx 64 2008-12-22 03:14:11Z olchansk $
//

#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <signal.h>

#include "TMidasOnline.h"
#include "TMidasEvent.h"
#include "TMidasFile.h"
#include "XmlOdb.h"

#include "string.h"

//#include <TROOT.h>
#include "TParticle.h"
#include <TSystem.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TTimer.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TTree.h>
#include <TString.h>
#include "Globals.h"

#include "HandleMesytec.h"
#include "HandleV1190.h"
#include "HandleSTAT.h"
#include "HandleScaler.h"
#include "HandlePHYSICS.h"
#include "eloss.h"

// Global Variables
int  gRunNumber = 0;
bool gIsRunning = false;
bool gIsPedestalsRun = false;
bool gIsOffline = false;
int  gEventCutoff = 0;
bool gUseTdc = true;
std::string gOutputFile = "";
std::string gCalibFile = "";

char mesbkname[][5] = {"ICA_", "SD2A", "SD1A", "YDA_","SUA_",""};
//char tdcbkname[][5] = {"YDT_", "", ""};
char tdcbkname[][5] = {"ICT_", "YDT_", "YUT_", "SUT_" ,"SD2T" ,"SD1T"};
char stabkname[][5] = {"VTST", "VTSA", "VTRT", "VTRA"};
char scalbkname[][5] = {"SCAD", "SCAR", "SCAS",""}; //scalers

TDirectory* gOnlineHistDir = NULL;
// TFile* gOutputFile = NULL;
TFile* treeFile = NULL;
TTree *tree = NULL;
TEvent *IrisEvent = NULL;
VirtualOdb* gOdb = NULL;
FILE* ASCIIYY1 = NULL;
FILE* ASCIICsI1 = NULL;
FILE* ASCIICsI2 = NULL;
FILE* ASCIIIC = NULL;
FILE* ASCIISd1 = NULL;
FILE* ASCIISd2 = NULL;

IDet detec; // calibrated variables from detectors, to be passed to HandlePhysics
IDet *pdet = &detec;
ITdc timeArray;
ITdc *ptdc = &timeArray;
//tdc_t ptdc;

double GetTimeSec() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec + 0.000001*tv.tv_usec;
}

//
//--------------------------------------------------------------
class MyPeriodic : public TTimer
{
public:
  typedef void (*TimerHandler)(void);

  int          fPeriod_msec;
  TimerHandler fHandler;
  double       fLastTime;

  MyPeriodic(int period_msec,TimerHandler handler)
  {
    assert(handler != NULL);
    fPeriod_msec = period_msec;
    fHandler  = handler;
    fLastTime = GetTimeSec();
    Start(period_msec,kTRUE);
  }

  Bool_t Notify()
  {
    double t = GetTimeSec();
    //printf("timer notify, period %f should be %f!\n",t-fLastTime,fPeriod_msec*0.001);

    if (t - fLastTime >= 0.9*fPeriod_msec*0.001)
      {
	//printf("timer: call handler %p\n",fHandler);
	if (fHandler)
	  (*fHandler)();

	fLastTime = t;
      }

    Reset();
    return kTRUE;
  }

  ~MyPeriodic()
  {
    TurnOff();
  }
};

//
//--------------------------------------------------------------
void startRun(int transition,int run,int time)
{
	gIsRunning = true;
	gRunNumber = run;
	gIsPedestalsRun = gOdb->odbReadBool("/experiment/edit on start/Pedestals run");
	printf("Begin run: %d, pedestal run: %d\n", gRunNumber, gIsPedestalsRun);
	  
	// if(gOutputFile!=NULL)
	// {
	//   	gOutputFile->Write();
	//   	gOutputFile->Close();
	//   	gOutputFile=NULL;
	// }  
	
	if(treeFile!=NULL)
	{
	  	treeFile->Write();
	  	treeFile->Close();
	  	treeFile=NULL;
	}  
	
	char filename[1024];
	// sprintf(filename, "output%05d.root", run);
	// gOutputFile = new TFile(filename,"RECREATE");
	
	// sprintf(filename, "tree%05d.root", run);
	sprintf(filename, "%s", gOutputFile.data());
	treeFile = new TFile(filename,"RECREATE");
	printf("Writing data to %s.\n",gOutputFile.data());
	
	HandleBOR_Mesytec(run, time, pdet, gCalibFile);
	if(gUseTdc) HandleBOR_V1190(run, time, ptdc);
	HandleBOR_PHYSICS(run,time, pdet, gCalibFile); 
	HandleBOR_Scaler(run,time); 
	HandleBOR_STAT(run, time);
}

void endRun(int transition,int run,int time)
{
	gIsRunning = false;
	gRunNumber = run;
	
	HandleEOR_Mesytec(run, time);
	if(gUseTdc) HandleEOR_V1190(run, time);
	HandleEOR_PHYSICS(run,time);
	HandleEOR_Scaler(run, time); 
	HandleEOR_STAT(run, time);

#ifdef OLD_SERVER
  	if (gManaHistosFolder)	gManaHistosFolder->Clear();
#endif

  	// if (gOutputFile)
    // {
    //  	gOutputFile->Write();      
	// 	gOutputFile->Close();
    //   	gOutputFile = NULL;
    // }
  
	if (treeFile)
    {
      	tree->AutoSave();
      	//treeFile->Write();
      	treeFile->Close();
      	treeFile = NULL;
    }

  	printf("End of run %d\n",run);
}

#include <TH1D.h>

//
//--------------------------------------------------------------
// void HandleSample(int ichan, void* ptr, int wsize)
// {
//   uint16_t *samples = (uint16_t*) ptr;
//   int numSamples = wsize;
// 
//   if (numSamples != 512)
//     return;
// 
//   char name[256];
//   sprintf(name, "channel%d", ichan);
// 
//   if (gOutputFile)
//     gOutputFile->cd();
// 
//   TH1D* samplePlot = (TH1D*)gDirectory->Get(name);
// 
//   if (!samplePlot)
//     {
//       printf("Create [%s]\n", name);
//       samplePlot = new TH1D(name, name, numSamples, 0, numSamples);
//       //samplePlot->SetMinimum(0);
// #ifdef OLD_SERVER
//       if (gManaHistosFolder)
//         gManaHistosFolder->Add(samplePlot);
// #endif
//     }
// 
//   for(int ti=0; ti<numSamples; ti++)
//     samplePlot->SetBinContent(ti, samples[ti]);
// }

//
//--------------------------------------------------------------
void HandleMidasEvent(TMidasEvent& event)
{
  	void *ptr;
  	int m;
  	int eventId = event.GetEventId();
  	//uint32_t timeStamp = event.GetTimeStamp();
  	//uint32_t serialN = event.GetSerialNumber();
  
  	// clear values here!
	clearDetectors();
  	if ((eventId == 1)) {
    //
    // Mesytec modules
    pdet->Clear();
  	IrisEvent->Clear();
    m=0;
    while (mesbkname[m][0]) { 
      	int size = event.LocateBank(NULL, mesbkname[m], &ptr);
      	//  if (m==2)  printf("ADC bank %s: first: %d %d\n", mesbkname[m], mesbkname[m][0], size); // loop until ascii code for first character of the bank name is empty
      	if (ptr && size) { 
			HandleMesytec(event, ptr, size, m, pdet, gCalibFile); 
 		} 
      	m++;
 	}
	if(gUseTdc){
		m=0;
  		while (tdcbkname[m][0]) {
			int size = event.LocateBank(NULL, tdcbkname[m], &ptr);
			// printf("TDC bank %s:%d\n", tdcbkname[m], size);
			if (ptr && size) {
		  		HandleV1190(event, ptr, ptdc, size, m); 
			}
			m++;
  		}
	}
	HandlePHYSICS(pdet);

  	}
 
//  else if ((eventId == 2)) {    
//      //
//      // V1190 modules 
//      m=0;
//      while (tdcbkname[m][0]) {
//	int size = event.LocateBank(NULL, tdcbkname[m], &ptr);
//	// printf("TDC bank %s:%d\n", tdcbkname[m], size);
//	if (ptr && size) {
//	  HandleV1190(event, ptr, ptdc, size, m); 
//	}
//	m++;
//      }
//  }

 	else if ((eventId == 3)) { // Scaler modules
      	m=0;
    	while (scalbkname[m][0]) { 
      		int size = event.LocateBank(NULL, scalbkname[m], &ptr);
      		//  if (m==2)  printf("ADC bank %s: first: %d %d\n", mesbkname[m], mesbkname[m][0], size); // loop until ascii code for first character of the bank name is empty
      		if (ptr && size) { 
				HandleScaler(event, ptr, size, m); 
      		} 
      		m++;
    	}
  	}
	//HandlePHYSICS(pdet);

#if 0
  // Stat banks
  int size =  event.LocateBank(NULL, stabkname[3], &ptr);
  printf("bank %s:%d\n", tdcbkname[3], size);              
  if (ptr && size) {
    HandleSTAT(event, ptr, size, 0);
  }
#endif

}

//
//--------------------------------------------------------------
void eventHandler(const void*pheader,const void*pdata,int size)
{
  TMidasEvent event;
  memcpy(event.GetEventHeader(), pheader, sizeof(TMidas_EVENT_HEADER));
  event.SetData(size, (char*)pdata);
  event.SetBankList();
  HandleMidasEvent(event);
}

//
//--------------------------------------------------------------
int ProcessMidasFile(TApplication*app,const char*fname)
{
	char dcfname[100] = "dccp://";
   	strcat(dcfname,fname); // added dccp:// in front of fname, there is seg fault
  	TMidasFile f;
  	bool tryOpen = f.Open(dcfname);
 
  	if (!tryOpen)
    {
    	printf("Cannot open input file \"%s\"\n",dcfname);
      	return -1;
    }

  	int i=0;
  	while (1)
    {
      	TMidasEvent event;
      	if (!f.Read(&event))
		break;

      	int eventId = event.GetEventId();

      	if ((eventId & 0xFFFF) == 0x8000)
		{
	  		// begin run
	  		printf("---- BEGIN RUN ---- \n");
	  		//event.Print();

	  		//char buf[256];
	  		//memset(buf,0,sizeof(buf));
	  		//memcpy(buf,event.GetData(),255);
	  		//printf("buf is [%s]\n",buf);
	  		//
	  		// Load ODB contents from the ODB XML file
	  		if (gOdb) delete gOdb;
	  		gOdb = new XmlOdb(event.GetData(),event.GetDataSize());
	  		startRun(0,event.GetSerialNumber(),0);
		}
      	else if ((eventId & 0xFFFF) == 0x8001)
		{
	  		printf("---- END RUN ---- \n");
	  		// end run
	  		//event.Print();
		}
      	else
		{
	  		//printf("case 3\n");	  
	  		event.SetBankList();
	  		//event.Print();
	  		HandleMidasEvent(event);
		}
      
      	if((i%100)==0)
		{
	  		//resetClock2time();
	   		printf("Processing event %d\r",i);
	  		//SISperiodic();
	  		//StepThroughSISBuffer();
		}
       	i++;
      	if ((gEventCutoff!=0)&&(i>=gEventCutoff))
		{
	  		printf("Reached event %d, exiting loop.\n",i);
	  		break;
		}
    }
  
  	f.Close();
  	endRun(0,gRunNumber,0);

  	// start the ROOT GUI event loop
  	//  app->Run(kTRUE);

  	return 0;
}

#ifdef HAVE_MIDAS

void MidasPollHandler()
{
  if (!(TMidasOnline::instance()->poll(0)))
    gSystem->ExitLoop();
}

int ProcessMidasOnline(TApplication*app, const char* hostname, const char* exptname)
{
   TMidasOnline *midas = TMidasOnline::instance();

   int err = midas->connect(hostname, exptname, "anaIris");
   if (err != 0)
     {
       fprintf(stderr,"Cannot connect to MIDAS, error %d\n", err);
       return -1;
     }

   gOdb = midas;

   midas->setTransitionHandlers(startRun,endRun,NULL,NULL);
   midas->registerTransitions();

   /* reqister event requests */

   midas->setEventHandler(eventHandler);
   midas->eventRequest("SYSTEM",-1,-1,(1<<1));

   /* fill present run parameters */

   gRunNumber = gOdb->odbReadInt("/runinfo/Run number");

   if ((gOdb->odbReadInt("/runinfo/State") == 3))
     startRun(0,gRunNumber,0);

   printf("Startup: run %d, is running: %d, is pedestals run: %d\n",gRunNumber,gIsRunning,gIsPedestalsRun);
   
   MyPeriodic tm(100,MidasPollHandler);
   //MyPeriodic th(1000,SISperiodic);
   //MyPeriodic tn(1000,StepThroughSISBuffer);
   //MyPeriodic to(1000,Scalerperiodic);

   /*---- start main loop ----*/

   //loop_online();
   app->Run(kTRUE);

   /* disconnect from experiment */
   midas->disconnect();

   return 0;
}

#endif

// #include <TGMenu.h>
// 
// class MainWindow: public TGMainFrame {
// 
// private:
//   TGPopupMenu*		menuFile;
//   //TGPopupMenu* 		menuControls;
//   TGMenuBar*		menuBar;
//   TGLayoutHints*	menuBarLayout;
//   TGLayoutHints*	menuBarItemLayout;
//   
// public:
//   MainWindow(const TGWindow*w,int s1,int s2);
//   virtual ~MainWindow(); // Closing the control window closes the whole program
//   virtual void CloseWindow();
//   
//   Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
// };
// 
// #define M_FILE_EXIT 0
// 
// Bool_t MainWindow::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
// {
//     // printf("GUI Message %d %d %d\n",(int)msg,(int)parm1,(int)parm2);
//     switch (GET_MSG(msg))
//       {
//       default:
// 	break;
//       case kC_COMMAND:
// 	switch (GET_SUBMSG(msg))
// 	  {
// 	  default:
// 	    break;
// 	  case kCM_MENU:
// 	    switch (parm1)
// 	      {
// 	      default:
// 		break;
// 	      case M_FILE_EXIT:
// 	        if(gIsRunning)
//     		   endRun(0,gRunNumber,0);
// 		gSystem->ExitLoop();
// 		break;
// 	      }
// 	    break;
// 	  }
// 	break;
//       }
// 
//     return kTRUE;
// }
// 
// MainWindow::MainWindow(const TGWindow*w,int s1,int s2) // ctor
//     : TGMainFrame(w,s1,s2)
// {
// 	//SetCleanup(kDeepCleanup);
// 	
// 	SetWindowName("ROOT Analyzer Control");
// 	
// 	// layout the gui
// 	menuFile = new TGPopupMenu(gClient->GetRoot());
// 	menuFile->AddEntry("Exit", M_FILE_EXIT);
// 	
// 	menuBarItemLayout = new TGLayoutHints(kLHintsTop|kLHintsLeft, 0, 4, 0, 0);
// 	
// 	menuFile->Associate(this);
// 	//menuControls->Associate(this);
// 	
// 	menuBar = new TGMenuBar(this, 1, 1, kRaisedFrame);
// 	menuBar->AddPopup("&File",     menuFile,     menuBarItemLayout);
// 	//menuBar->AddPopup("&Controls", menuControls, menuBarItemLayout);
// 	menuBar->Layout();
// 	
// 	menuBarLayout = new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX);
// 	AddFrame(menuBar,menuBarLayout);
// 	
// 	MapSubwindows(); 
// 	Layout();
// 	MapWindow();
// }
// 
// MainWindow::~MainWindow()
// {
//     delete menuFile;
//     //delete menuControls;
//     delete menuBar;
//     delete menuBarLayout;
//     delete menuBarItemLayout;
// }
// 
// void MainWindow::CloseWindow()
// {
//     if(gIsRunning)
//     	endRun(0,gRunNumber,0);
//     gSystem->ExitLoop();
// }

static bool gEnableShowMem = false;

int ShowMem(const char* label)
{
  if (!gEnableShowMem)
    return 0;

  FILE* fp = fopen("/proc/self/statm","r");
  if (!fp)
    return 0;

  int mem = 0;
  fscanf(fp,"%d",&mem);
  fclose(fp);

  if (label)
    printf("memory at %s is %d\n", label, mem);

  return mem;
}

void help()
{
	printf("\nUsage:\n");
	printf("\n./bin/anaIris [-h] [-c] [-Hhostname] [-Eexptname] [-eMaxEvents] [-P9091] [-p9090] [-m] [-g] [file1 file2 ...]\n");
	printf("\n");
	printf("\t-h: print this help message\n");
	printf("\t-o/-o=/--output=: Path of output file.\n");
	printf("\t-c/-c=/--config=: Path of main configuration file.\n");
	printf("\t-T: test mode - start and serve a test histogram\n");
	printf("\t-Hhostname: connect to MIDAS experiment on given host\n");
	printf("\t-Eexptname: connect to this MIDAS experiment\n");
	printf("\t-P: Start the TNetDirectory server on specified tcp port (for use with roody -Plocalhost:9091)\n");
	printf("\t-p: Start the old midas histogram server on specified tcp port (for use with roody -Hlocalhost:9090)\n");
	printf("\t-e: Number of events to read from input data files\n");
	printf("\t-m: Enable memory leak debugging\n");
	printf("\t-g: Enable graphics display when processing data files\n");
	printf("\t-nt: Don't read TDCs.\n");
	printf("\n");
	printf("Example1: analyze online data: ./bin/anaIris -P9091\n");
	printf("Example2: analyze existing data: ./bin/anaIris /data/alpha/current/run00500.mid -c=config.txt\n");
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
	
	TApplication *app = new TApplication("anaIris", &argc, argv);
	
	if(gROOT->IsBatch()) {
		printf("Cannot run in batch mode\n");
	 	return 1;
	}
	
	bool have_output= false;
	bool have_calib= false;
	bool forceEnableGraphics = false;
	bool testMode = false;
	int  oldTcpPort = 0;
	int  tcpPort = 0;
	const char* hostname = NULL;
	const char* exptname = NULL;

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
			have_calib=true;
		}
       	else if (strncmp(arg,"-c",2)==0){  // Calibration file 
	 		gCalibFile = arg+2;
			have_calib=true;
		}
      	else if (strncmp(arg,"--config=",9)==0){  // Calibration file 
	 		gCalibFile = arg+9;
			have_calib=true;
		}
       	else if (strncmp(arg,"-m",2)==0) // Enable memory debugging
	 		gEnableShowMem = true;
       	else if (strncmp(arg,"-p",2)==0) // Set the histogram server port
	 		oldTcpPort = atoi(arg+2);
       	else if (strncmp(arg,"-P",2)==0) // Set the histogram server port
	 		tcpPort = atoi(arg+2);
       	else if (strcmp(arg,"-T")==0)
	 		testMode = true;
       	else if (strcmp(arg,"-nt")==0)
	 		gUseTdc=false;
       	else if (strcmp(arg,"-g")==0)
	 		forceEnableGraphics = true;
       	else if (strncmp(arg,"-H",2)==0)
	 		hostname = strdup(arg+2);
       	else if (strncmp(arg,"-E",2)==0)
	 		exptname = strdup(arg+2);
       	else if (strcmp(arg,"-h")==0)
	 		help(); // does not return
       	else if (arg[0] == '-')
	 		help(); // does not return
	}
    
	//MainWindow mainWindow(gClient->GetRoot(), 200, 300);
	
	gROOT->cd();
	gOnlineHistDir = new TDirectory("anaIris", "anaIris online plots");

	#ifdef OLD_SERVER
	   	if (oldTcpPort)	StartMidasServer(oldTcpPort);
	#else
	   	if (oldTcpPort)	fprintf(stderr,"ERROR: No support for the old midas server!\n");
	#endif
	   
	if (tcpPort) fprintf(stderr,"ERROR: No support for the TNetDirectory server!\n");
		 
	gIsOffline = false;

	for (unsigned int i=1; i<args.size(); i++)
    {
       	const char* arg = args[i].c_str();

      	if (arg[0] != '-')  
	 	{  
	   		gIsOffline = true;
	   		//gEnableGraphics = false;
	   		//gEnableGraphics |= forceEnableGraphics;
			if(have_output==true&&have_calib==true){	   
				ProcessMidasFile(app,arg);
			}
			else{
				if(have_output==false){
					printf("No output file specified!\n");
					printf("Use -o=/path/to/your/output-file!\n");
				}
				if(have_calib==false){ 
					printf("No calibration file specified!\n");
					printf("Use -c=/path/to/your/calib-file!\n");
				}
	 		}
     	}
	}

   	if (testMode)
    {
    	gOnlineHistDir->cd();
    	TH1D* hh = new TH1D("test", "test", 100, 0, 100);
    	hh->Fill(1);
    	hh->Fill(10);
    	hh->Fill(50);

    	app->Run(kTRUE);
    	return 0;
   	}

	// if we processed some data files,
	// do not go into online mode.
	if (gIsOffline)
	  return 0;
	    
	gIsOffline = false;
	//gEnableGraphics = true;
	#ifdef HAVE_MIDAS
   	ProcessMidasOnline(app, hostname, exptname);
	#endif
   
   	return 0;
}

//end

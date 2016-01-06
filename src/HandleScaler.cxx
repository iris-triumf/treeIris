//
// ROOT analyzer
//
// DAQ Scaler handling
//
// $Id$
//

/// \mainpage
///
/// \section intro_sec Introduction
///
///
/// \section features_sec Features
///  
///   state = gOdb->odbReadInt("/runinfo/State");

#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <signal.h>

//#include "midasServer.h"
#include "TMidasEvent.h"
#include <TApplication.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2.h>
#include <TNtuple.h>
#include <TFolder.h>
#include <TFile.h>
#include <TMath.h>

#include "Globals.h"

#include "HandleScaler.h"
// published var for HandlePHYSICS
int gScalernitems;

extern TTree *tree;

scal_t scaler;
const int Nchannels = 32;
TH1D * hScalerD[Nchannels] = {NULL};
TH1D * hScalerR[Nchannels] = {NULL};
TH1D * hScalerS[Nchannels] = {NULL};

void HandleScaler(TMidasEvent& event, void* ptr, int nitems, int MYLABEL)
{
  uint32_t *data;
  int    i, debug = 0, debug1 = 0; 
  int eventId = event.GetEventId();

  data = (uint32_t *) ptr;

  // if (hScaler[0] == NULL)
  //    HandleBOR_Scaler(gRunNumber, 0);
    
    // publishing var for HandlePHYSICS
    gScalernitems = nitems;

    // Loop over all the banks

    if (nitems != 0) {
      if (debug) printf(" Scaler EvtId: %d Evt#:%d nitems:%d\n", eventId, event.GetSerialNumber(), nitems); 
          if (nitems > Nchannels) return;
	  
	  if (MYLABEL==0){	  
      for (i=0 ; i<nitems ; i++) {
	if (debug1) printf("items:%d - data[%d]: %d / 0x%x\n", Nchannels, i, data[i], data[i]);
	
       	  //hScalerD[i]->Fill((float)data[i] , 1.); 
	
	
      } //for
	  } //MYLABEL==0

	  if (MYLABEL==1){	  
      for (i=0 ; i<nitems ; i++) {
	if (debug1) printf("items:%d - data[%d]: %d / 0x%x\n", Nchannels, i, data[i], data[i]);
	
       	  //hScalerR[i]->Fill((float)data[i] , 1.); 
		  scaler.ICRate = data[30];
	  scaler.ScintRate = data[29];
	  scaler.AccepTrigRate = data[26];
	  scaler.FreeTrigRate = data[31];
	
      } //for
	  } //MYLABEL==0

	  if (MYLABEL==2){	  
      for (i=0 ; i<nitems ; i++) {
	if (debug1) printf("items:%d - data[%d]: %d / 0x%x\n", Nchannels, i, data[i], data[i]);
	
       	  //hScalerS[i]->Fill((float)data[i] , 1.); 
	  scaler.ICSum = data[30];
	  scaler.ScintSum = data[29];
	  scaler.AccepTrigSum = data[26];
	  scaler.FreeTrigSum = data[31];
      } //for
      
	  } //MYLABEL==0

      } // nitems != 0
     
    scaler.EventNumber = eventId;
    

};

//---------------------------------------------------------------------------------
void HandleBOR_Scaler(int run, int time)
{
  printf(" in Scaler BOR\n");
//   char label[32], sig[32];
// 
//   // Booking 
// 
// 
//   hScalerD[0] = (TH1D*)gDirectory->Get("scalerD00");
//   if (hScalerD[0] == 0) {
//     if (gOutputFile) {
//  gOutputFile->cd();
// 
//       // Make an SCALER directory and cd to it.
//       TDirectory* scaler_dir = gOutputFile->mkdir("scaler");      
//       scaler_dir->cd();
// 
//       for (int channel=0;channel<Nchannels;channel++) {
// 	sprintf(label, "scalerD%02d", channel);
// 	sprintf(sig, "scalerD%03d", channel);
// 	hScalerD[channel] = new TH1D(label, sig, 4000, 0, 100000);
//       }
// 
//     for (int channel=0;channel<Nchannels;channel++) {
// 	sprintf(label, "scalerR%02d", channel);
// 	sprintf(sig, "scalerR%03d", channel);
// 	hScalerR[channel] = new TH1D(label, sig, 4000, 0, 100000);
//       }
// 
//     for (int channel=0;channel<Nchannels;channel++) {
// 	sprintf(label, "scalerS%02d", channel);
// 	sprintf(sig, "scalerS%03d", channel);
// 	hScalerS[channel] = new TH1D(label, sig, 4000, 0, 1000000000);
//       }
//       printf(" in Mesytec BOR... Booking histos Done ....\n");
//     }
//   }
  



tree->Branch("scaler",&(scaler.ICSum),"EventNumber/I:ICSum:ScintSum:AccepTrigSum:FreeTrigSum:ICRate/D:ScintRate:AccepTrigRate:FreeTrigRate");
}


void HandleEOR_Scaler(int run, int time)
{
  printf(" in Scaler EOR\n");
}

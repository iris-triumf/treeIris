// HandleMesytec.h

#ifndef HandleMesytec_H
#define HandleMesytec_H
#include "TMidasEvent.h"
#include "TEvent.h"

//Extern
//extern int gMesytecnitems;


int clearDetectors();

//class det_t {
// public:
//  Double_t TYdEnergy;
//  Double_t TYdEnergy2;
//  Double_t TCsI1Energy;
//  Double_t TCsI2Energy;
//  Double_t TCsIEnergy;
//  Double_t TCsI1Energy2;
//  Double_t TCsI2Energy2;
//  Double_t TCsIEnergy2;
//  Double_t TCsI1ADC;
//  Double_t TCsI2ADC;
//  Double_t TICEnergy;
//  Double_t TSd2rEnergy;
//  Double_t TSd2sEnergy;
//  Double_t TSd1rEnergy;
//  Double_t TSd1sEnergy;
//  Double_t TSd2rEnergy2;
//  Double_t TSd2sEnergy2;
//  Double_t TSd2rEnergyCal;
//  Double_t TSd1rEnergy2;
//  Double_t TSd1sEnergy2;
//  Double_t SSB;
//  Double_t TSdETot;
//  Double_t TSdTheta;
//  Double_t TSdThetaCM;
//  Double_t TSdPhi;
//  Double_t TYdTheta;// Yd theta angle
//  Double_t TQv; // Q-value from S3
//  Double_t QValue; //Q-value from YY1
//  Double_t TBE; //Beam energy
//  Double_t TIC00;
//  Double_t TIC01;
//  Double_t TIC02;
//  Double_t TIC03;
//  Double_t TIC04;
//  Double_t TIC05;
//  Double_t TIC06;
//  Double_t TIC07;
//  Double_t TIC08;
//  Double_t TIC09;
//  Double_t TIC10;
//  Double_t TIC11;
//  Double_t TIC12;
//  Double_t TIC13;
//  Double_t TIC14;
//  Double_t TIC15;
//  Int_t    TYdChannel;
//  Int_t    TYdChannel2;
//  Int_t    TYdNo;
//  Int_t    TYdRing;
//  Int_t    TCsI1Channel;
//  Int_t    TCsI2Channel;
//  Int_t    TCsI1Channel2;
//  Int_t    TCsI2Channel2;
//  Int_t    TCsIChannel;
//  Int_t    TICChannel;
//  Int_t    TSd2rChannel;
//  Int_t    TSd2sChannel;
//  Int_t    TSd1rChannel;
//  Int_t    TSd1sChannel;
//  Int_t    TSd2rChannel2;
//  Int_t    TSd2sChannel2;
//  Int_t    TSd1rChannel2;
//  Int_t    TSd1sChannel2;
//
//  void Clear();
//
// };

void  HandleMesytec(TMidasEvent& event, void* ptr, int wsize, int MYLABEL, IDet* pdet, TString CalibFile);
void  HandleBOR_Mesytec(int run, int time, IDet* pdet, TString CalibFile);
void  HandleEOR_Mesytec(int run, int time);


#endif
// end

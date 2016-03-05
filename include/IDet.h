// IDet.h

#ifndef IDet_H
#define IDet_H
#include <vector>
#include <TObject.h>
#include <TClass.h>

class IDet : public TObject {
 	public:
  		IDet(); //! Create
		virtual ~IDet() {} //! 
		
		Int_t TYdMul;
		std::vector<Int_t> TYdChannel;
		std::vector<Double_t> TYdEnergy;
		std::vector<Int_t> TYdNo;
		std::vector<Int_t> TYdRing;
		std::vector<Double_t> TYdTheta;// Yd theta angle

		Int_t TCsI1Mul;
		std::vector<Int_t> TCsI1Channel;
		std::vector<Double_t> TCsI1Energy;
		std::vector<Double_t> TCsI1ADC;
		Int_t TCsI2Mul;
		std::vector<Int_t> TCsI2Channel;
		std::vector<Double_t> TCsI2Energy;
		std::vector<Double_t> TCsI2ADC;
		Int_t TCsIMul;
		std::vector<Int_t> TCsIChannel;
		std::vector<Double_t> TCsIEnergy;

		Double_t SSB;
		
		Int_t TICChannel;
		Double_t TICEnergy;
		Double_t TIC[16];

		Int_t TSd1rMul;
		std::vector<Int_t> TSd1rChannel;
		std::vector<Double_t> TSd1rEnergy;
		std::vector<Double_t> TSd1rADC;
		
		Int_t TSd1sMul;
		std::vector<Int_t> TSd1sChannel;
		std::vector<Double_t> TSd1sEnergy;
		std::vector<Double_t> TSd1sADC;
		
		Int_t TSd2rMul;
		std::vector<Int_t> TSd2rChannel;
		std::vector<Double_t> TSd2rEnergy;
		std::vector<Double_t> TSd2rADC;
		Double_t TSd2rEnergyCal;
		
		Int_t TSd2sMul;
		std::vector<Int_t> TSd2sChannel;
		std::vector<Double_t> TSd2sEnergy;
		std::vector<Double_t> TSd2sADC;
		
		Double_t TSdETot;
		std::vector<Double_t> TSdTheta;
		Double_t TSdThetaCM;
		std::vector<Double_t> TSdPhi;

//		Double_t TQv1; // Q-value from S3
//		Double_t TQv2; // Q-value from S3
//		Double_t QValue1; //Q-value from YY1 and CsI1
//		Double_t QValue2; //Q-value from YY1 and CsI2
		Double_t QValue; //Q-value from YY1 and CsI2
		Double_t TBE; //Beam energy
	
	void Clear();
	ClassDef(IDet,1)
};

#endif
// end

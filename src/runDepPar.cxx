//Run dependent parameters in BOR AS
#include "runDepPar.h"
#include <math.h>
//#include "TGraph.h"

//TGraph* h2Th = new TGraph("$TREEIRIS/calib-files/H2Thickness.dat","%lg %lg");

// void setRunDepPar(TString filename, beam_t *beam, target_t *target, ejectile_t *light_ej, ejectile_t *heavy_ej)
void setRunDepPar(TString filename, runDepPar_t *runDepPar)
{
	runDepPar->ICmin=0;
	runDepPar->ICmax=0;
//	heavy_ej->A = -1;
//	heavy_ej->Z = -1;
//	heavy_ej->mass = -1;
//	light_ej->A = -1;
//	light_ej->Z = -1;
//	light_ej->mass = -1;

//	double value =200;
//	double *EBAC = &value;
 	
	char buffer[256];
	FILE * parFile;
	parFile=fopen(filename,"r");

	if (parFile == NULL) {
		perror ("Error opening file");
		//fprintf(pwFile,"Error opening file");
   	}	
	printf("Reading config file '%s'\n",filename.Data());
	
	while (!feof(parFile))
	{
		if (!fgets(buffer,256,parFile)) break;
		printf("%s",buffer);
		
		char* val=strchr(buffer,'=');
		if (!val) printf("Missing = in input parFile, line: '%s'\n",buffer);
		*val=0;	
		
		// trim param name
		char* trm=val-1;
		while (*trm<=32) *(trm--)=0;
	
		val++;
		if (*val==0) printf("Value missing for parameter %s",buffer);
		
		// parse float parameter (if any)
		float v;
		char cval[256];	
		std::string strval;
		sscanf(val,"%f",&v);
		sscanf(val,"%s",cval);
		strval=cval;

		if (strcmp(buffer,"EBAC")==0){
			runDepPar->EBAC = v;
			runDepPar->energy = v;
		}
//		if (strcmp(buffer,"BMASS")==0)		beam->mass = v;
//		if (strcmp(buffer,"BA")==0)			beam->A = v;
//		if (strcmp(buffer,"BZ")==0)			beam->Z = v;
		if (strcmp(buffer,"ICMIN")==0)		runDepPar->ICmin = v;
		if (strcmp(buffer,"ICMAX")==0)		runDepPar->ICmax = v;
		if (strcmp(buffer,"A")==0)			runDepPar->nA = strval;
		if (strcmp(buffer,"a")==0)			runDepPar->na = strval;
		if (strcmp(buffer,"B")==0)			runDepPar->nB = strval;
		if (strcmp(buffer,"b")==0)			runDepPar->nb = strval;
// 		if (strcmp(buffer,"TA")==0)			target->A = v;
// 		if (strcmp(buffer,"TZ")==0)			target->Z = v;
// 		if (strcmp(buffer,"TMASS")==0)		target->mass = v;
// 		if (strcmp(buffer,"TTH")==0)		target->thickness = v;
// 		if (strcmp(buffer,"HEA")==0)		heavy_ej->A = v;
// 		if (strcmp(buffer,"HEZ")==0)		heavy_ej->Z = v;
// 		if (strcmp(buffer,"HEMASS")==0)		heavy_ej->mass = v;
// 		if (strcmp(buffer,"LEA")==0)		light_ej->A = v;
// 		if (strcmp(buffer,"LEZ")==0)		light_ej->Z = v;
// 		if (strcmp(buffer,"LEMASS")==0)		light_ej->mass = v;
	}
	fclose(parFile);

//	if(heavy_ej->A==-1) heavy_ej->A=beam->A;	
//	if(heavy_ej->Z==-1) heavy_ej->Z=beam->Z;	
//	if(heavy_ej->mass==-1) heavy_ej->mass=beam->mass;	
//	if(light_ej->A==-1) light_ej->A=target->A;	
//	if(light_ej->Z==-1) light_ej->Z=target->Z;	
//	if(light_ej->mass==-1) light_ej->mass=target->mass;	

 //	runDepPar->EBAC = *EBAC;
//	runDepPar->energy = *EBAC;
//	runDepPar->momentum = sqrt(beam->energy*beam->energy+2.*beam->energy*beam->mass);//beam momentum
//	runDepPar->beta = beam->momentum/(beam->energy + beam->mass + target->mass);
//	runDepPar->gamma = 1./sqrt(1.-beam->beta*beam->beta);
}

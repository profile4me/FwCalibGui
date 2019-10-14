
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TPRegexp.h>

#include <haddef.h>
#include <hades.h>
#include "hruntimedb.h"
#include "htask.h"
#include "hwalltaskset.h"
#include "hevent.h"
#include "hcategory.h"
#include "hdatasource.h"
#include <hdst.h>

using namespace std;

TPRegexp re("^cat");
const char 		*LIB_PATH 		= "/cvmfs/hades.gsi.de/install/5.34.34/hydra2-5.1a/lib";
Int_t 			mdcMods[6][4] 	= { 	{1,1,1,1},
    									{1,1,1,1},
    									{1,1,1,1},
    									{1,1,1,1},
    									{1,1,1,1},
    									{1,1,1,1} };
const char* 	beamtime 		= "mar19";
const char* 	paramSource 	= "root";
const char* 	rootParFile 	= "/cvmfs/hades.gsi.de/param/real/mar19/allParam_Mar19_gen0_06062019.root";
const char* 	paramRelease 	= "MAR19";
const char* 	inFile		 	= "/lustre/nyx/hades/raw2/mar19/default/tsm/062/be1906202103902.hld";
const char* 	outFile		 	= "./dst/be1906202103902.root";
const Int_t 	maxEvents 		= 100000;

void loadMods() {
	vector<const char*> libs;
	libs.push_back("libStart.so");
	libs.push_back("libRich.so");
	libs.push_back("libMdc.so");
	libs.push_back("libTof.so");
	libs.push_back("libShower.so");
	libs.push_back("libEmc.so");
	libs.push_back("libRpc.so");
	libs.push_back("libWall.so");
	libs.push_back("libPionTracker.so");
	libs.push_back("libMdcTrackD.so");
	libs.push_back("libMdcTrackG.so");
	libs.push_back("libParticle.so");
	for (auto&& i: libs) gSystem->Load(Form("%s/%s",LIB_PATH,i));
}

map<string,Cat_t> *getCats() {
	map<string,Cat_t> *res = new map<string,Cat_t>;
	TCollection *col= gROOT->GetListOfGlobals();
	for (auto&& i: *col) {
		if (re.Match(i->GetName())) {
			(*res)[i->GetName()] = *(Cat_t*)gROOT->ProcessLineFast(Form("&%s",i->GetName()));
		}
	}
	return res;
}

int main(int argc, char const *argv[])
{
	if (argc!=2) {
		printf("Wrong number of args!!\n");
		return 1;
	}

	const char* inFile = argv[1];
	TString tempStr = gSystem->BaseName(inFile);
	printf("Hld BaseName: %s\n",tempStr.Data());
	// tempStr.ReplaceAll(".hld",".root");
	TString outFile = Form("dst_mar19/%s",tempStr.Data());

	loadMods();
	map<string,Cat_t> *notPersistCats = getCats();
	(*notPersistCats)["catWallRaw"] = -1;

	new Hades;
    gHades->setTreeBufferSize(1000);
    gHades->makeCounter(5000);
    HDst::setupSpectrometer(beamtime,mdcMods,"wall");
    HDst::setupParameterSources(paramSource,"",rootParFile,paramRelease);  
    HDst::setDataSource(0,"",inFile); // Int_t sourceType,TString inDir,TString inFile,Int_t refId, TString eventbuilder"
    HDst::setupUnpackers(beamtime,"wall");
    HTaskSet *masterTaskSet = gHades->getTaskSet("all");
    HTask *wallTasks = (new HWallTaskSet())->make("real");
    masterTaskSet->add(wallTasks);
    
    gHades->init();
    HEvent *event = gHades->getCurrentEvent();
    for (auto& p: *notPersistCats) {
    	HCategory *cat = event->getCategory(p.second);
    	if (cat) cat->setPersistency(kFALSE);
    }
    // outFile = "./dst/be1906202103902.root";
    gHades->setOutputFile((Text_t*)outFile.Data(),"RECREATE","Test",2);
    gHades->makeTree();

    gHades->eventLoop(maxEvents);
    printf("SUCCESS_MARK\n");
    delete gHades;
	return 0;
}
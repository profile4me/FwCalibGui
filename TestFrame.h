#ifndef __TestFrame__
#define __TestFrame__

#include <map>
#include <string>
#include <fstream>

#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TFile.h>
#include <TPRegexp.h>
#include <TMath.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TApplication.h>
#include "TGListTreePersistent.h"
#include <TGFrame.h>
#include <TGButton.h>
const int FILES_PER_PERIOD = 5;

class TimePeriod;

class FileLists {
	static TPRegexp 					fnMask;
	static const char 					*HLD_TXT;
	
	TimePeriod 							*tp;
	std::map<std::string,std::string> 	dst[FILES_PER_PERIOD];
	std::vector<std::string>		 	hld[FILES_PER_PERIOD];
	bool 								filledFlags[FILES_PER_PERIOD];
	//output
	TList 								*outDst;
	TList 								*outHld;

	bool hasAlready(int n, const char *fname) {
		auto it = dst[n].find(fname);
		return it!=dst[n].end();
	}
public:
	static const char 					*DST_DIR_KRONOS;
	static const char 					*DST_DIR_LXPOOL;
	static const char 					*TASK_TXT;
	static const char 					*PID_TXT;

	FileLists(TimePeriod *tp);
	void exploreDst(TSystemDirectory *d=new TSystemDirectory("root",DST_DIR_KRONOS));
	void exploreHld();
	void preliminaryLists();
	int dstSize(int n);
	TList *getDst();
	TList *getHld();
};


class TimePeriod : public TNamed {
	static TPRegexp tsMask;
	int bCoords[3];
	int eCoords[3];
	int bTimeStamp;
	int eTimeStamp;
	std::vector<int> *subRanges;
	
	int getTimeStamp(const char *fname) {
		// Info("getTimeStamp()", "fname: %s",fname);
		if (!tsMask.Match(fname)) return 0;
		// Info("getTimeStamp()", "Matched!!!");
		return ( (TObjString*)tsMask.MatchS(fname)->At(1) )->String().Atoi(); 	
	}

	void formSubPeriods() {
		int nSubPeriods = FILES_PER_PERIOD;
		int cur = bTimeStamp;
		subRanges->push_back(cur);
		while (cur<eTimeStamp) {
			int stepSize = TMath::Ceil((eTimeStamp-cur+1)*1.0/nSubPeriods--);
			subRanges->push_back(cur+=stepSize);
		}
		if (cur==eTimeStamp) subRanges->push_back(eTimeStamp+1); 
		
		printf("SubRanges: [");
		for (int i=0; i<subRanges->size(); i++) printf("%d, ",subRanges->at(i));
		printf("]\n");
	}

public:
	TimePeriod() {
		for (int i=0; i<3; i++) {
			bCoords[i]=eCoords[i]=0;
		}	
		bTimeStamp = eTimeStamp = 0;
		subRanges =0;
	}
	TimePeriod(TString beg, TString end) {
		TObjArray *bArr=beg.Tokenize(":");
		TObjArray *eArr=end.Tokenize(":");
		for (int i=0; i<3; i++) {
			bCoords[i]=( (TObjString*)bArr->At(i) )->GetString().Atoi();
			eCoords[i]=( (TObjString*)eArr->At(i) )->GetString().Atoi();
		}
		SetName(Form("[%d:%02d:%02d - %d:%02d:%02d]",bDay(),bHour(),bMin(),eDay(),eHour(),eMin()));
		bTimeStamp = bCoords[0]*10000 + bCoords[1]*100 + bCoords[2];
		eTimeStamp = eCoords[0]*10000 + eCoords[1]*100 + eCoords[2];
		subRanges = new std::vector<int>;
		formSubPeriods();
	}
	int bDay() 	const { return bCoords[0]; } 
	int bHour() const { return bCoords[1]; }
	int bMin() 	const { return bCoords[2]; }
	int eDay() 	const { return eCoords[0]; }
	int eHour() const { return eCoords[1]; }
	int eMin() 	const { return eCoords[2]; }
      // type==0  - run 
      // type==1  - days group
      // type==2  - day
      // type==3  - hours group
      // type==4  - hour
      // type==5  - minutes group
      // type==6  - minute
	int getType() {
		if (bMin()==eMin() && bHour() == eHour() && bDay() == eDay()) return 6;
		else if (bHour() == eHour() && bDay() == eDay()) {
			if (bMin()==0 && eMin()==59) return 4;
			else return 5;
		} else if (bDay() == eDay()) {
			if (bMin()!=0 || eMin()!=59) return 5;
			else if (bHour()!=0 || eHour()!=23) return 3;
			else return 2;
		}
		else {
			if (bMin()!=0 || eMin()!=59) return 5;
			else if (bHour()!=0 || eHour()!=23) return 3;
			else return 1;
		}
	}
	bool isLayIn(int ts) {
		return (bTimeStamp <= ts && ts <= eTimeStamp);
	}
	bool isLayIn(const char *fname) {
		return isLayIn(getTimeStamp(fname));
	}
	int subRangeIndex(int ts) {
		int i=0;
		for (; i<subRanges->size(); i++) {
			if (ts<subRanges->at(i)) break;
		}
		return (i==subRanges->size()) ? -1 : i-1;
	}
	int subRangeIndex(const char *fname) {
		int ts = getTimeStamp(fname);
		int ind = subRangeIndex(ts);
		// Info("subRangeIndex()", "ts: %d \t ind: %d", ts,ind);
		return ind;
	}
	const char *string() const {
		return Form("[%d:%02d:%02d-%d:%02d:%02d]", bDay(),bHour(),bMin(), eDay(),eHour(),eMin());
	}
	ClassDef(TimePeriod,1);
};

class TreeItem : public TGListTreePersistentItemStd {
	TimePeriod *tp;
	//toSkip: 		-10
	//dstNotEnough:	0
	//dstEnough:	1
	int status;
	int id;
	TList *dstList;
	TList *hldList;
public:
	static int globId;
	TreeItem() {
		tp = 0;
		status =0;
		id =0;
		dstList = hldList = 0;
	}
	TreeItem(const char *name) : TGListTreePersistentItemStd(gClient,name) {
		tp = 0;
		status = -10;  //if this constructor was used then it is not end node (not involved in processing. Only for nice tree view) 
		id = -1;
		dstList = hldList =  new TList;
	}
	TreeItem(TimePeriod *tp) : TGListTreePersistentItemStd(gClient,tp->string()) {
		this->tp = tp;
      	status = 0;		// start status value (not enough dst)
		setText();
		dstList = hldList =  new TList;
    }
    void setText() {
    	const char *prefix;
    	switch (tp->getType()) {
         	case 0: prefix = Form("Run"); break;
         	case 1: prefix = Form( "Days group [day%d - day%d]", tp->bDay(),tp->eDay()); break;
         	case 2: prefix = Form("Day %d",tp->bDay()); break;
         	case 3: prefix = Form("Hours group [day%d %02d:00 - day%d %02d:00]",tp->bDay(),tp->bHour(),tp->eDay(),tp->eHour()); break;
         	case 4: prefix = Form("Hour %d",tp->bHour()); break;
         	case 5: prefix = Form("Minutes group [day%d %02d:%02d - day%d %02d:%02d]",tp->bDay(),tp->bHour(),tp->bMin(),tp->eDay(),tp->eHour(),tp->eMin()); break;
         	case 6: prefix = Form("Minute %d",tp->bMin()); break;
      	}
      	const char *postfix;
      	switch (status) {
      		case 0: 	postfix = "not enough dst"; break;
      		case 1: 	postfix = "ready for filling"; break;
      		case -1:	postfix = "pended"; break;
      		case -2:	postfix = "processing"; break;
      		case -3:	postfix = "Dst generated"; break;
      		default: 	postfix = "";
      	}
      	SetText(Form("%s %s", prefix,postfix));
    }
	void checkFiles() {
		if (status>=-3 && status<1) {	//not enough dst 
			FileLists *fl=new FileLists(tp);
			dstList = fl->getDst();
			hldList = fl->getHld();
			
			Info("checkFiles()", "Formed dstList: -------------------");
			dstList->Print();
			Info("checkFiles()", "-----------------------------------");
			Info("checkFiles()", "Formed hldList: -------------------");
			hldList->Print();
			Info("checkFiles()", "-----------------------------------");
	
			if (hldList->GetEntries()==0) {
				status = 1;
				setText();
				id = -1;
				return;
			}
			id=globId++;
			ofstream oS(FileLists::TASK_TXT, ofstream::app);		
			oS << Form("#%04d\n",id);
			for (auto* i: *hldList) {
				oS << i->GetName() << "\n";
				Info("checkFiles()","Another line appended into %s",FileLists::TASK_TXT);
			}
			oS.close();

		}
	}
	void update() {
		if (id >= 0) {
			ifstream iS(FileLists::PID_TXT);
			std::string str;
			while (getline(iS,str)) {
				TString tStr(str);
				if ( ((TObjString*)tStr.Tokenize('\t')->At(0))->String().Atoi() == id ) {
					status=-1; //job array for timePeriod at least pended  
					
					TSystemDirectory logDir("logDir",Form("%s/log",FileLists::DST_DIR_LXPOOL));
					TPRegexp slurmOutReg(Form("slurm-%s.*.out",tStr.Tokenize('\t')->At(1)->GetName()));
					Info("update()", "Searching \'%s\' in dir \'%s\'", slurmOutReg.GetPattern().Data(), logDir.GetTitle());
					TList fl;
					for (auto* o: *logDir.GetListOfFiles()) {
						printf("\t%s\n",o->GetName());
						if (slurmOutReg.Match(o->GetName())) fl.Add(o);
					}		
					if (fl.GetEntries()) {
						status = -2; //job array for this timePeriod at least running
						bool notAllEnded=false;
						for (auto* o: fl) {
							ifstream iS(Form("%s/%s",logDir.GetTitle(),o->GetName()));
							std::string str;
							bool thisEnded=false;
							while (getline(iS,str)) {
								if (!str.compare("SUCCESS_MARK")) {
									thisEnded = true;
									break;
								}
							}
							if (!thisEnded) {
								notAllEnded=true;
								break;
							}
						}
						if (!notAllEnded) status = -3; //job array successfully finished
					}
					setText();
				}
			}
			iS.close();
		}
	}
	ClassDef(TreeItem,1);
};

class TestFrame : public TGMainFrame {
	static const char *VIEW_FILE_NAME;
	static bool checkingState;
	TGCanvas *view;
	TGListTreePersistent *container;
	TFile *ioF;
	TreeItem *rootItem;
	TGTextButton *btnCheckFiles;
	TGTextButton *btnMakeDst;
	TGTextButton *btnUpdate;
	TGHorizontalFrame *hFrame;
public:
	TestFrame();
	TreeItem *readRootItem();
	void AddItem(TreeItem *parent, TreeItem *item);
	void CloseWindow();
	void checkFiles(TreeItem *item = NULL);
	//SLOTS
	void update(TreeItem *item = NULL);
	void checkFilesWrap();
	void handleClick(TGListTreePersistentItem *triggeredItem, int btn);
	void makeDst();
	ClassDef(TestFrame,0);
};

#endif
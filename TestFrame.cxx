#include "TestFrame.h"
ClassImp(TimePeriod);
ClassImp(TestFrame);
ClassImp(TreeItem);

TPRegexp TimePeriod::tsMask(".*be19([[:digit:]]{7})[[:digit:]]{4}[.][[:alpha:]]{3,4}$");

using namespace std;
//********************************* FileLists REALIZATION *****************************************
//**************************************************************************************************
TPRegexp FileLists::fnMask("be[[:digit:]]{13}");
const char 	*FileLists::HLD_TXT = "/u/dborisen/newDir/hld.list";
const char 	*FileLists::DST_DIR_KRONOS = "../dstProduction/dst_mar19";     // if build on kronos node
const char 	*FileLists::DST_DIR_LXPOOL = "/u/dborisen/lustre/user/dborisen/dstProduction/dst_mar19";
const char 	*FileLists::TASK_TXT = "/u/dborisen/lustre/user/dborisen/dstProduction/task.list";
const char 	*FileLists::PID_TXT = "/u/dborisen/lustre/user/dborisen/dstProduction/pids.list";

FileLists::FileLists(TimePeriod *tp) {
	printf("Getting file lists for TimePeriod: %s ...\n", tp->string());

	this->tp=tp;
	for (int i=0; i<FILES_PER_PERIOD; i++) filledFlags[i]=false;
	outDst = new TList;
	outHld = new TList;

	exploreDst();
	exploreHld();
	bool exitFlag;
	int counter=0;
	for (int i=0; i<FILES_PER_PERIOD; i++) {
		for (int j=0; j<FILES_PER_PERIOD; j++) {
			if (dst[j].size()>i) {
				map<string,string>::iterator it = dst[j].begin();
				advance(it,i);
				outDst->Add( new TNamed((*it).second.data(),"") );
				counter++;
			} else if (hld[j].size()) {
				outHld->Add( new TNamed(hld[j].back().data(),"") );
				hld[j].pop_back();
				counter++;
			}
			if (counter==FILES_PER_PERIOD) {
				exitFlag=true;
				break;
			}
		}
		if (exitFlag) break;
	}
}
void FileLists::exploreDst(TSystemDirectory *d) {
	printf("\tExploring dsts in dir: %s ...\n", d->GetTitle());
	TList *temp = d->GetListOfFiles();
	temp->Remove(temp->FindObject("."));
	temp->Remove(temp->FindObject(".."));
	for (TObject *o : *temp) {
		if ( ((TSystemFile*)o)->IsFolder() ) exploreDst((TSystemDirectory*)o);
		else {
			int ind = tp->subRangeIndex(o->GetName());
			if (ind<0 || filledFlags[ind]) continue;  
			dst[ind][fnMask.MatchS(o->GetName())->At(0)->GetName()] = Form("%s/%s",o->GetTitle(),o->GetName());
			printf("dst[%d][%s] = %s\n", ind,fnMask.MatchS(o->GetName())->At(0)->GetName(),Form("%s/%s",o->GetTitle(),o->GetName()));
			if (dst[ind].size()==FILES_PER_PERIOD) filledFlags[ind]=true;
		}
	}
}
void FileLists::exploreHld() {
	printf("\tExploring hld...\n");
	ifstream iS(HLD_TXT);
	string str;
	while (getline(iS, str)) {
		int ind = tp->subRangeIndex(str.data());
		// if (ind>=0) printf("Ind: %d\n",ind);
		if (ind<0 || filledFlags[ind] || hasAlready(ind, fnMask.MatchS(str.data())->At(0)->GetName())) continue;
		// printf("Ind: %d\n",ind);
		hld[ind].push_back(str);
		if (dst[ind].size()+hld[ind].size()==FILES_PER_PERIOD) filledFlags[ind] = true;
	}
}
void FileLists::preliminaryLists() {
	printf("--------------------------\n");
	for (int i=0; i<FILES_PER_PERIOD; i++) printf("subPeriod %d: %d %d\n", i,dst[i].size(),hld[i].size());
	printf("--------------------------\n");
}
int FileLists::dstSize(int n) {
	if (n>=0 && n<FILES_PER_PERIOD) return dst[n].size();
	return -1;
}
TList *FileLists::getDst() { return outDst; }
TList *FileLists::getHld() { return outHld; }
//*************************************************************************************************




//********************************* TestFrame REALIZATION *****************************************
//*************************************************************************************************
int TreeItem::globId = 0;
bool TestFrame::checkingState = 0;
const char *TestFrame::VIEW_FILE_NAME = "view.root";

TestFrame::TestFrame() : TGMainFrame(gClient->GetRoot(),200,200),ioF(0) {
	view = new TGCanvas(this,200,200);
	container = new TGListTreePersistent(view,0);
	//BUTTON BLOCK
	hFrame = new TGHorizontalFrame(this,1,20, kFixedHeight);
	btnCheckFiles = new TGTextButton(hFrame,"checkFiles");
	btnMakeDst = new TGTextButton(hFrame,"makeDst");
	btnUpdate = new TGTextButton(hFrame,"update");
	hFrame->AddFrame(btnCheckFiles, new TGLayoutHints(kLHintsCenterX));
	hFrame->AddFrame(btnMakeDst, new TGLayoutHints(kLHintsCenterX));
	hFrame->AddFrame(btnUpdate, new TGLayoutHints(kLHintsCenterX));

	SetLayoutManager(new TGVerticalLayout(this));
	AddFrame(view, new TGLayoutHints(kLHintsExpandX|kLHintsTop));
	AddFrame(hFrame, new TGLayoutHints(kLHintsExpandX|kLHintsBottom));
	Layout();

	MapSubwindows();
	MapWindow();
	readRootItem();
	Connect(container, "Clicked(TGListTreePersistentItem*,int)", "TestFrame", this, "handleClick(TGListTreePersistentItem*,int)");
	Connect(btnCheckFiles, "Clicked()", "TestFrame", this, "checkFilesWrap()");
	Connect(btnMakeDst, "Clicked()", "TestFrame", this, "makeDst()");
	Connect(btnUpdate, "Clicked()", "TestFrame", this, "update()");
}

TreeItem *TestFrame::readRootItem() {
	TString fName(VIEW_FILE_NAME);
	int fMode = gSystem->FindFile(".", fName) ? 1 : 0;
	Info("readRootItem()", "fMode: %d",fMode);
	ioF = new TFile(VIEW_FILE_NAME, "update");
	rootItem = (TreeItem*)ioF->Get("rootItem");
	if (!rootItem) {
		rootItem = new TreeItem("mar19");
		container->AddItem(0,rootItem);
		container->AddItem(rootItem, new TreeItem(new TimePeriod("62:00:00","62:23:59")));
		container->AddItem(rootItem, new TreeItem(new TimePeriod("63:00:00","63:23:59")));
	} else container->AddItem(0,rootItem);
}
void TestFrame::AddItem(TreeItem *parent, TreeItem *item) {
	container->AddItem(parent, item);
}
void TestFrame::CloseWindow() {
	ioF->cd();
	rootItem->Write("", TObject::kOverwrite);
	ioF->Close();
	gApplication->Terminate(0);
}
void TestFrame::handleClick(TGListTreePersistentItem *triggeredItem, int btn) {
	// if (btn==3) container->AddItem(triggeredItem, new TreeItem("subItem"));
}
void TestFrame::checkFilesWrap() {
	Info("","Wrapper BEGIN");
	if (checkingState) return;
	checkingState = 1;
	TreeItem::globId=0;

	ofstream oS(FileLists::TASK_TXT, ofstream::out);		
	oS.close();
	Info("checkFilesWrap()", "%s is cleaned!", FileLists::TASK_TXT);

	checkFiles();

	checkingState = 0;
	Info("","Wrapper END");
}
void TestFrame::checkFiles(TreeItem *item ) {
	
	if (!item) item=rootItem;
	while (item) {
		item->checkFiles();
		if ((TreeItem*)item->GetFirstChild())  checkFiles((TreeItem*)item->GetFirstChild());
		item = (TreeItem*)item->GetNextSibling();
	}
}
void TestFrame::makeDst() {
	gSystem->Exec("ssh kronos.hpc '/lustre/nyx/hades/user/dborisen/TreeList/send.sh'");
}
void TestFrame::update(TreeItem *item) {
	if (!item) item=rootItem;
	while (item) {
		item->update();
		if ((TreeItem*)item->GetFirstChild())  update((TreeItem*)item->GetFirstChild());
		item = (TreeItem*)item->GetNextSibling();
	}
}

//*************************************************************************************************



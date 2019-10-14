#include <stdio.h>
#include "TGListTreePersistent.h"
#include <TGFrame.h>
#include <TFile.h>
#include "TestFrame.h"

int main(int argc, char  *argv[])
{
	TApplication app("app",&argc,argv);
	TestFrame frame;
	
	app.Run();
	return 0;
}
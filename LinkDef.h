#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#if defined(ROOT_TGListTreePersistent)  && !defined(__TestFrame__)
	#pragma link C++ class TGListTreePersistentItem+;
	#pragma link C++ class TGListTreePersistentItemStd+;
	#pragma link C++ class TGListTreePersistent;
#endif

#ifdef __TestFrame__
	#pragma link C++ class TimePeriod+;
	#pragma link C++ class TreeItem+;
	#pragma link C++ class TestFrame;
#endif	

#endif
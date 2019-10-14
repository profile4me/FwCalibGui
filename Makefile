BUILD_DIR = ./build
CC = g++ `root-config --glibs --cflags` -I. -I${BUILD_DIR} -g -std=c++11
HYDRA_ASSETS = -I${HADDIR}/include -L${HADDIR}/lib -lAlignment -lQA -lParticle -lKalman -lPionTracker -lMdcTrackG -lMdcTrackD -lOra -lWall -lShowerUtil -lShower -lTof -lMdc -lRich -lEmc -lRpc -lStart -lTools -lHydra -lDst

all: analysisDST app

########## TGListTreePersistent ######################################################
${BUILD_DIR}/TGListTreePersistentDict.cxx: TGListTreePersistent.h LinkDef.h
	rootcint -f ${BUILD_DIR}/TGListTreePersistentDict.cxx -c -p TGListTreePersistent.h LinkDef.h

${BUILD_DIR}/TGListTreePersistentDict.cxx.o: ${BUILD_DIR}/TGListTreePersistentDict.cxx 
	${CC}  -o ${BUILD_DIR}/TGListTreePersistentDict.cxx.o -c ${BUILD_DIR}/TGListTreePersistentDict.cxx 

${BUILD_DIR}/TGListTreePersistent.cxx.o: TGListTreePersistent.cxx TGListTreePersistent.h
	${CC}  -o ${BUILD_DIR}/TGListTreePersistent.cxx.o -c TGListTreePersistent.cxx
####################################################################################

########## TestFrame ######################################################
${BUILD_DIR}/TestFrameDict.cxx: TestFrame.h LinkDef.h
	rootcint -f ${BUILD_DIR}/TestFrameDict.cxx -c -p TestFrame.h LinkDef.h

${BUILD_DIR}/TestFrameDict.cxx.o: ${BUILD_DIR}/TestFrameDict.cxx 
	${CC} -o ${BUILD_DIR}/TestFrameDict.cxx.o -c ${BUILD_DIR}/TestFrameDict.cxx

${BUILD_DIR}/TestFrame.cxx.o: TestFrame.cxx TestFrame.h
	${CC} -o ${BUILD_DIR}/TestFrame.cxx.o -c TestFrame.cxx
##########################################################################



obj: ${BUILD_DIR}/TGListTreePersistentDict.cxx.o ${BUILD_DIR}/TGListTreePersistent.cxx.o ${BUILD_DIR}/TestFrameDict.cxx.o ${BUILD_DIR}/TestFrame.cxx.o

app: main.cc TestFrame.cxx TestFrame.h obj 
	${CC} -o app main.cc ${BUILD_DIR}/*.o

analysisDST: analysisDST.cc 
	${CC} ${HYDRA_ASSETS} -o analysisDST analysisDST.cc
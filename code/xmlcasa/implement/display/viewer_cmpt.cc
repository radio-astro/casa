
/***
 * Framework independent implementation file for viewer...
 *
 * Implement the viewer component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <xmlcasa/display/viewer_cmpt.h>
        
#include <display/Display/StandAloneDisplayApp.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtViewer/QtApp.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableLock.h>
#include <QMutex>
#include <xmlcasa/display/qtlistner.qo.h>

//extern qtListner *pythonThread;

using namespace std;

namespace casac {

viewer::viewer() : v(0), dpg(0), qdd(0)
{

}

viewer::~viewer()
{
}

bool
viewer::show(const std::string& filename, const std::string& datatype, const std::string& displaytype)
{
	casa::Table::relinquishAutoLocks(true);
	/*
	pthread_attr_t attr;
	pthread_t theThread[1], stuff(0);
	pthread_attr_init(&attr);
	pthread_create(&theThread[0], &attr, doviewer, (void *)stuff);
	if(!fork()){
		execl("/opt/casa/darwin/apps/casaviewer.app/Contents/MacOS/casaviewer",
		      "/opt/casa/darwin/apps/casaviewer.app/Contents/MacOS/casaviewer", 
		      filename.c_str(),
		      datatype.c_str(),
		      displaytype.c_str(),
		      (char *)0);
		std::cerr << "big problems" << std::endl;
	}
	*/
	/*
        pythonThread->signal_showviewer();
	//::qtListner *anybodyHome = new ::qtListner;
	//anybodyHome->start();
	*/

        INITIALIZE_PGPLOT
	//casa::QtViewer *v;
        //casa::QtDisplayPanelGui *dpg;
	//casa::QtDisplayData *qdd(0);

        if(!v){
        	v = new casa::QtViewer;
	        dpg = new casa::QtDisplayPanelGui(v);
	}
        if(filename.length())
	   qdd = v->createDD(filename, datatype, displaytype);
	if(qdd){
           casa::Record opts;
	   opts.define("axislabelswitch", casa::True);
	   qdd->setOptions(opts);
	}
        dpg->show();
	v->showDataManager();


	std::cerr << filename << std::endl;
	std::cerr << datatype << std::endl;
	std::cerr << displaytype << std::endl;
        return false;
    // TODO : IMPLEMENT ME HERE !
}

} // casac namespace


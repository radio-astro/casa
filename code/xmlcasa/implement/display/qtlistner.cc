#include <iostream>
#include <Python.h>
#include <CCM_Python/Py.h>
#include <xmlcasa/display/qtlistner.qo.h>

qtListner::qtListner(int argc, char **argv_mod, char *path){
   CCM_Python::Py::init( argc, argv_mod, path );
}

void qtListner::signal_showviewer()
{
	emit showviewer();
}

void qtListner::run()
{
   CCM_Python::Py py;
   py.evalFile( "qtcasapy.py" );
}


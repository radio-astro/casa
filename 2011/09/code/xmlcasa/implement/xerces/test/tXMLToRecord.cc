/*
*/
#include <xmlcasa/xerces/stdcasaXMLUtil.h>
#include <xmlcasa/utils/stdBaseInterface.h>
#include <string>
#include <vector>
#include <xmlcasa/record.h>
#include <iostream> 
//
using namespace std;
using namespace casac;
//
int main (int argc, char* args[]) { 
	stdcasaXMLUtil myXMLUtil;
	record outRec;
	string xmlfile = args[1];
	   // Parse the XML task/tool file to get the parameter set
	myXMLUtil.readXMLFile(outRec, xmlfile);
	dumpRecord(cerr, outRec);
	return 0; 
} 

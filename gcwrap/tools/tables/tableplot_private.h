//Privates for table_cmpt

casa::TablePlot *TP;

casacore::Vector<casacore::String> TabNames;
casacore::Vector<casacore::Table> TABS;
casacore::Vector<casacore::Table> SelTABS;
casacore::Vector<casacore::Table*> TABSptr;
casacore::Int nTabs;

casacore::Int adbg;
casacore::Record tempplotoption;
casacore::Vector<casacore::String> temptabnames;
casacore::Vector<casacore::String> tempdatastr;
casacore::Vector<casacore::String> tempiteraxes;
casacore::Vector<casacore::String> templabels;
casacore::Int alive;

casa::TPResetCallBack *resetter_p;

PyThreadState *myInterpreter;
casacore::LogIO *itsLog;


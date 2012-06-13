//Privates for table_cmpt

casa::TablePlot *TP;

casa::Vector<casa::String> TabNames;
casa::Vector<casa::Table> TABS;
casa::Vector<casa::Table> SelTABS;
casa::Vector<casa::Table*> TABSptr;
casa::Int nTabs;

casa::Int adbg;
casa::Record tempplotoption;
casa::Vector<casa::String> temptabnames;
casa::Vector<casa::String> tempdatastr;
casa::Vector<casa::String> tempiteraxes;
casa::Vector<casa::String> templabels;
casa::Int alive;

casa::TPResetCallBack *resetter_p;

PyThreadState *myInterpreter;
casa::LogIO *itsLog;


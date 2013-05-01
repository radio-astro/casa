//Privates for table_cmpt
//
//  These are internal variables the connect us to the objects
//
casa::LogIO      *itsLog;
casa::TableProxy *itsTable;
//
// Private constructor so we can make components on the fly
//
table(casa::TableProxy *myTable);
//
casa::TableLock *getLockOptions(casac::record &lockoptions);

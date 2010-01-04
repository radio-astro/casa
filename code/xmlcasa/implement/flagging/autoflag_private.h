casa::LogIO *itsLog;
casa::MeasurementSet *itsMS;
casa::Record *agents;
casa::Record *opts;
casa::Record *itsSelect;
casa::RedFlagger *itsFlagger;
int agentCount;
bool need2AddSelect;

bool initSelectRecord();
bool addAgent(casa::RecordInterface &);

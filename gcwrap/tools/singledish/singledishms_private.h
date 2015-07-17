casa::LogIO *itsLog;
casa::SingleDishMS *itsSd;

// raise AipsError if SingleDishMS instance is not yet generated.
void assert_valid_ms();
// generate a record to pass SinglDishMS::setAverage
casa::Record get_time_averaging_record(const bool& timeaverage, const string& timebin, const string& timespan);

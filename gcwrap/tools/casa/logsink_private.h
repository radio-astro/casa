
void installLogFilter();
casacore::LogMessage::Priority getLogLevel(const std::string &level);

casacore::LogSinkInterface *thelogsink;
casacore::LogOrigin *itsorigin;
casacore::String *taskname;
casacore::String processor_name;
std::string logname;
casacore::LogMessage::Priority logLevel;
std::vector<std::string> filterMsgList;
bool globalsink;

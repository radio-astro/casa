
void installLogFilter();
casa::LogMessage::Priority getLogLevel(const std::string &level);

casa::LogSinkInterface *thelogsink;
casa::LogOrigin *itsorigin;
casa::String *taskname;
casa::String processor_name;
std::string logname;
casa::LogMessage::Priority logLevel;
std::vector<std::string> filterMsgList;
bool globalsink;

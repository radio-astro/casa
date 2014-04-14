#include <iostream>

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/helpers/exception.h>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>

#include <casa_sakura/SakuraUtils.h>

namespace log4cxx {  
class CasaLoggerAppender : public AppenderSkeleton
{
public:
  DECLARE_LOG4CXX_OBJECT(CasaLoggerAppender)
  BEGIN_LOG4CXX_CAST_MAP()
  LOG4CXX_CAST_ENTRY(CasaLoggerAppender)
  LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
  END_LOG4CXX_CAST_MAP()

  CasaLoggerAppender()
    : AppenderSkeleton(),
      casa_logger_(),
      origin_("sakura", "")
  {}

  CasaLoggerAppender(const LayoutPtr &layout)
    : AppenderSkeleton(layout),
      casa_logger_(),
      origin_("sakura", "")
  {}

  virtual ~CasaLoggerAppender() {}

  void close() {}

  bool requiresLayout() const {return true;}

protected:
  void append(const spi::LoggingEventPtr &event, helpers::Pool &)
  {
    // set origin
    const spi::LocationInfo &location = event->getLocationInformation();
    origin_.functionName(location.getMethodName());
    origin_.sourceLocation(casa::SourceLocation::canonicalize(location.getFileName(), location.getLineNumber()));
    casa_logger_.origin(origin_);

    // set loglevel
    const LevelPtr &level = event->getLevel();
    switch (level->toInt()) {
    case Level::FATAL_INT:
      casa_logger_ << casa::LogIO::EXCEPTION;
      break;
    case Level::ERROR_INT:
      casa_logger_ << casa::LogIO::SEVERE;
      break;
    case Level::WARN_INT:
      casa_logger_ << casa::LogIO::WARN;
      break;
    case Level::INFO_INT:
      casa_logger_ << casa::LogIO::NORMAL;
      break;
    case Level::DEBUG_INT:
    case Level::TRACE_INT:
      casa_logger_ << casa::LogIO::DEBUGGING;
      break;
    default:
      casa_logger_ << casa::LogIO::NORMAL;
      break;
    }

    // output message
    const LogString &rendered_message = event->getRenderedMessage();
    casa_logger_ << rendered_message << casa::LogIO::POST;
  }

private:
  casa::LogIO casa_logger_;
  casa::LogOrigin origin_;
};

} // namespace log4cxx

using namespace log4cxx;
IMPLEMENT_LOG4CXX_OBJECT(CasaLoggerAppender);

namespace casa {
Bool SakuraUtils::is_initialized_ = False;
  
Bool SakuraUtils::InitializeSakura(const std::string &level) {
  // return if already initialized
  if (is_initialized_) {
    return True;
  }
  
  // Setup log4cxx
  int result = 0;
  try {
    // configure logger
    log4cxx::helpers::Properties properties;
    properties.setProperty("log4j.rootLogger", "ALL, A1");
    properties.setProperty("log4j.appender.A1", "org.apache.log4j.CasaLoggerAppender");
    properties.setProperty("log4j.appender.A1.layout", "org.apache.log4j.PatternLayout");
    log4cxx::PropertyConfigurator::configure(properties);
  }
  catch (log4cxx::helpers::Exception &) {
    result = 1;
  }

  // set logging level for sakura
  SetLogLevel(level);
  
  LogIO logger(LogOrigin("SakuraUtils", "InitializeSakura", WHERE));
  if (result == 0) {
    log4cxx::LoggerPtr log4cxx_logger = log4cxx::Logger::getRootLogger();
    logger << LogIO::DEBUGGING << "Succeeded to initialize sakura logger: " << log4cxx_logger->getClass().getName() << " with " << log4cxx_logger->getAllAppenders()[0]->getClass().getName() << LogIO::POST;
  }
  else {
    logger << LogIO::WARN << "Failed to configure logger for sakura" << LogIO::POST;
  }
  logger << LogIO::DEBUGGING << "Initializing Sakura...";
  is_initialized_ = False;
  LIBSAKURA_SYMBOL(Status) status = LIBSAKURA_SYMBOL(Initialize)(NULL, NULL);
  if (status == LIBSAKURA_SYMBOL(Status_kOK)) {
    logger << LogIO::DEBUGGING << "SUCCESS!" << LogIO::POST;
    is_initialized_ = True;
  }
  else {
    logger << LogIO::DEBUGGING << "FAILED!" << LogIO::POST;
  }
  return is_initialized_;
}

void SakuraUtils::CleanUpSakura() {
  if (is_initialized_) {
    LogIO logger(LogOrigin("SakuraUtils", "CleanUpSakura", WHERE));
    logger << LogIO::DEBUGGING << "Cleaning up Sakura..." << LogIO::POST;
    LIBSAKURA_SYMBOL(CleanUp)();
    is_initialized_ = False;
  }
}

Bool SakuraUtils::IsSakuraInitialized() {
  LogIO logger(LogOrigin("SakuraUtils", "IsSakuraInitialized", WHERE));
  logger << LogIO::DEBUGGING << "sakura is "
	 << ((is_initialized_) ? "" : "not ") << "initialized." << LogIO::POST;
  return is_initialized_;
}
  
void SakuraUtils::SetLogLevel(const std::string &loglevel)
{
  String level_string = String(loglevel);
  level_string.upcase();
  log4cxx::LoggerPtr log4cxx_logger = log4cxx::Logger::getRootLogger();
  if (level_string == "EXCEPTION") {
    log4cxx_logger->setLevel(log4cxx::Level::getFatal());
  }
  else if (level_string == "SEVERE") {
    log4cxx_logger->setLevel(log4cxx::Level::getError());
  }
  else if (level_string == "WARN") {
    log4cxx_logger->setLevel(log4cxx::Level::getWarn());
  }
  else if (level_string == "INFO") {
    log4cxx_logger->setLevel(log4cxx::Level::getInfo());
  }
  else if (level_string == "DEBUG") {
    log4cxx_logger->setLevel(log4cxx::Level::getDebug());
  }
}
}  // End of casa namespace.

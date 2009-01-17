#include "Logger.h"

using namespace asap;

std::string Logger::log_ = std::string("");

Logger::Logger()
{
  enableLog();
}

Logger::Logger(bool enabled)
{
  enabled_ = enabled;
}
Logger::~Logger()
{
}

void Logger::pushLog(const std::string& s, bool newline ) const
{
  if (enabled_) {
    log_+=s;
    if ( newline ) log_+="\n";
  };
}
std::string Logger::popLog() const
{
  std::string out;
  if (enabled_) out=log_;log_="";
  return out;
}
void Logger::enableLog()
{
  enabled_ = true;
}
void Logger::disableLog()
{
  enabled_ = false;
}

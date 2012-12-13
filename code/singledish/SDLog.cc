#include <singledish/SDLog.h>

using namespace asap;

std::string SDLog::log_ = std::string("");

SDLog::SDLog() 
{ 
  enableLog();
}

SDLog::SDLog(bool enabled)
{ 
  enabled_ = enabled;
}

SDLog::~SDLog()
{};

void SDLog::pushLog(const std::string& s) const
{ 
  if (enabled_) {
    log_+=s;log_+="\n";
  }; 
}
std::string SDLog::popLog() const
{
  std::string out; 
  if (enabled_) out=log_;log_="";
  return out;
}
void SDLog::enableLog()
{ 
  enabled_ = true;
}
void SDLog::disableLog()
{
  enabled_ = false;
}

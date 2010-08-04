//
// C++ Interface: AsapLogSink
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPLOGSINK_H
#define ASAPLOGSINK_H

#include <casa/Logging/MemoryLogSink.h>
#include <string>

namespace asap {
/**
  * This class provides the a casa::LogSink implementation for asap
  * It should be used to catch all casa logMessage and replace the globalSink.
  * It provides an overloaded postLocally which accepts strings so it can also
  * be used from python.
  * @brief The ASAP logging class
  * @author Malte Marquarding
  * @date $Date: $
  * @version
  */
class AsapLogSink : public casa::MemoryLogSink {
public:
  /**
   * Default Constructor
   **/
  AsapLogSink() {;} ;

  virtual ~AsapLogSink() {;};

  virtual void postMessage(const std::string& msg,
			   const std::string& location="",
                           const std::string& priority="INFO");

  std::string popMessages();

private:

};

void setAsapSink(AsapLogSink& sink);

} // namespace

#endif

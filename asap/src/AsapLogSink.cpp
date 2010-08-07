//
// C++ Implementation: AsapLogSink
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/iostream.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/NullLogSink.h>
#include "AsapLogSink.h"

using namespace casa;

namespace asap {

  void AsapLogSink::postMessage(const std::string& msg,
				const std::string& location,
                                const std::string& priority)
  {
    LogMessage::Priority p;
    if (priority == "INFO") {
      p = LogMessage::NORMAL;
    } else if (priority == "WARN") {
      p = LogMessage::WARN;
    } else if (priority == "ERROR") {
      p = LogMessage::SEVERE;
    }
    LogMessage message(msg, LogOrigin(location), p);

    MemoryLogSink::postLocally(message);
  }

  std::string AsapLogSink::popMessages()
  {
    ostringstream oss;
    for (uInt i=0; i < nelements(); ++i) {
      std::string p = getPriority(i);
      if (p != "INFO") {
        oss << p << ": ";
      }
      oss << getMessage(i) << endl;
    }
    clearLocally();
    return String(oss);
  }

  void setAsapSink(AsapLogSink& sink)
  {
    LogSinkInterface* s = &sink;
    LogSink().globalSink(s);
  }

};

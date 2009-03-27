//
// C++ Interface: Logger
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPLOGGER_H
#define ASAPLOGGER_H

#include <string>
#include <iostream>

namespace asap {
/**
  * This class provides the logging within asap. All other classes which need to log
  * should inherit from this.
  * @brief The ASAP logging class
  * @author Malte Marquarding
  * @date $Date: 2007-04-25 22:55:17 -0600 (Wed, 25 Apr 2007) $
  * @version
  */
class Logger {
public:
  /**
   * Default Constructor
   **/
  Logger();
  
  /**
   * Constructor with switch to enable/disable logging
   * @param[in] enabled indicating the deafult state
   */  
  explicit Logger(bool enabled);

  /*
   * Destructor
   */
  virtual ~Logger();
  /**
   * push another message into the logger
   * @param[in] s the message
   * @param[in] newline whether to add a newline character at the end
   */  
  void pushLog(const std::string& s, bool newline=true) const;
  /**
   * pop the message form the logger
   * @return the log message string
   */  
  std::string popLog() const;
  /**
   * enable logging
   */
  virtual void enableLog();
  /**
   * disable logging
   */
  virtual void disableLog();
  
private:
  static std::string log_;
  bool enabled_;
};

} // namespace

#endif


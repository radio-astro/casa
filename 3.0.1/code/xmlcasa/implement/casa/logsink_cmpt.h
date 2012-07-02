
/***
 * Framework independent header file for logsink...
 *
 * Implement the logsink component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _logsink_cmpt__H__
#define _logsink_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>

#include <xmlcasa/casa/logsink_forward.h>

// put includes here

namespace casac {

/**
 * logsink component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class logsink
{
  private:

	

  public:

    logsink();
    virtual ~logsink();

    bool origin(const std::string& fromwhere);

    bool filter(const std::string& level);

    bool post(const std::string& message, const std::string& priority = "NORMAL", const std::string &origin="");

    bool postLocally(const std::string& message, const std::string& priority = "NORMAL", const std::string &origin="");

    std::string version();

    std::string localId();

    std::string id();


    bool setglobal(const bool isglobal = true);

    bool setlogfile(const std::string& filename = "casapy.log");

    bool showconsole(const bool onconsole = false);

    std::string logfile();

    private:

#include <xmlcasa/casa/logsink_private.h>

};

} // casac namespace
#endif


//#---------------------------------------------------------------------------
//# PKSmsg.h: Message handling for the PKSIO classes.
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 2008-2009, Australia Telescope National Facility, CSIRO
//#
//# This file is part of livedata.
//#
//# livedata is free software: you can redistribute it and/or modify it under
//# the terms of the GNU General Public License as published by the Free
//# Software Foundation, either version 3 of the License, or (at your option)
//# any later version.
//#
//# livedata is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with livedata.  If not, see <http://www.gnu.org/licenses/>.
//#
//# Correspondence concerning livedata may be directed to:
//#        Internet email: mcalabre@atnf.csiro.au
//#        Postal address: Dr. Mark Calabretta
//#                        Australia Telescope National Facility, CSIRO
//#                        PO Box 76
//#                        Epping NSW 1710
//#                        AUSTRALIA
//#
//# http://www.atnf.csiro.au/computing/software/livedata.html
//# $Id: PKSmsg.h,v 1.3 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2008/09/18, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSMSG_H
#define ATNF_PKSMSG_H

#include <casa/stdio.h>

using namespace std;

// <summary>
// Message handling for the PKSIO classes.
// </summary>

class PKSmsg
{
  public:
    // Constructor.
    PKSmsg();

    // Destructor.
    virtual ~PKSmsg();

    // Set message disposition.  If fd is non-zero messages will be written
    // to that file descriptor, else stored for retrieval by getMsg().
    virtual int setMsg(
        FILE *fd = 0x0);

    // Log a message.
    virtual void logMsg(const char *msg = 0x0);

    // Get a message string, or 0x0 if there is none.  The null-terminated
    // message string may contain embedded newline characters and will have
    // a trailing newline.
    const char *getMsg();

    // Get the next group of messages by type: ERROR, WARNING, or otherwise.
    // The null-terminated message string may contain embedded newline
    // characters but will NOT have a trailing newline.  Call this repeatedly
    // to unwind the message stack (otherwise messages may be lost).
    enum msgType {NORMAL, WARNING, ERROR};
    const char *getMsg(msgType &type);

    // Clear the message buffer.
    void clearMsg(void);

  protected:
    // Initialize messaging.
    void initMsg();

  private:
    // For messaging.
    char *cMsgBuff, *cMsgIdx;
    int   cMsgLen, cNMsg;
    FILE *cMsgFD;
};

#endif

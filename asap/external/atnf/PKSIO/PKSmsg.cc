//#---------------------------------------------------------------------------
//# PKSmsg.cc: Message handling for the PKSIO classes.
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
//# $Id: PKSmsg.cc,v 1.3 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2008/09/18, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/PKSmsg.h>

#include <casa/stdio.h>

#include <string>
#include <cstring>

//------------------------------------------------------------- PKSmsg::PKSmsg

// Default constructor.

PKSmsg::PKSmsg()
{
  initMsg();
}

void PKSmsg::initMsg()
{
  cMsgFD = stderr;
  cMsgLen = 0;
  cMsgBuff = 0x0;
  cMsgIdx  = cMsgBuff;
  cNMsg = 0;
}

//------------------------------------------------------------ PKSmsg::~PKSmsg

// Destructor.

PKSmsg::~PKSmsg()
{
  delete [] cMsgBuff;
  cMsgBuff = 0x0;
  cMsgIdx  = cMsgBuff;
}

//------------------------------------------------------------- PKSmsg::setMsg

// Set message disposition.  If fd is non-zero messages will be written
// to that file descriptor, else stored for retrieval by getMsg().

int PKSmsg::setMsg(FILE *fd)
{
  cMsgFD = fd;

  if (cMsgFD == 0x0) {
    clearMsg();

  } else {
    delete [] cMsgBuff;
    cMsgLen  = 0;
    cMsgBuff = 0x0;
    cMsgIdx  = cMsgBuff;
  }

  return 0;
}

//------------------------------------------------------------- PKSmsg::logMsg

// Log a message.

void PKSmsg::logMsg(const char *msg)
{
  if (msg) {
    if (cMsgFD) {
      fprintf(cMsgFD, "%s\n", msg);

    } else {
      // Expand the message buffer if necessary.
      if (!cMsgBuff) {
        cMsgLen = 128;
        cMsgBuff = new char[cMsgLen];
        cMsgIdx  = cMsgBuff;
        clearMsg();
      }

      int used = strlen(cMsgBuff);
      int free = cMsgLen - (used + 1);
      int xtra = 1 + strlen(msg);
      if (free < xtra) {
        if (xtra < 128) xtra = 128;
        cMsgLen += xtra;
        char *newBuff = new char[cMsgLen];
        strcpy(newBuff, cMsgBuff);
        delete [] cMsgBuff;
        cMsgBuff = newBuff;
        cMsgIdx  = cMsgBuff;
      }

      sprintf(cMsgBuff+used, "%s%s", ((cNMsg++)?"\n":""), msg);
    }
  }
}

//------------------------------------------------------------- PKSmsg::getMsg

// Get a message string, or 0x0 if there is none.

const char *PKSmsg::getMsg()
{
  if (cMsgBuff && *cMsgBuff) {
    cMsgIdx = cMsgBuff;
    return cMsgBuff;
  }

  return 0x0;
}


// Get the next group of messages by type: ERROR, WARNING, or otherwise.

const char *PKSmsg::getMsg(msgType &type)
{
  if (cMsgIdx && *cMsgIdx) {
    if (strncmp(cMsgIdx, "ERROR", 5) == 0) {
      type = ERROR;
    } else if (strncmp(cMsgIdx, "WARNING", 7) == 0) {
      type = WARNING;
    } else {
      type = NORMAL;
    }

    // Gather multi-line messages.
    char *cp = cMsgIdx;
    while ((cMsgIdx = strchr(cMsgIdx, '\n'))) {
      cMsgIdx++;

      if (type == ERROR) {
        if (strncmp(cMsgIdx, "ERROR", 5) == 0 ||
            strncmp(cMsgIdx, "     ", 5) == 0) {
          continue;
        }

      } else if (type == WARNING) {
        if (strncmp(cMsgIdx, "WARNING", 7) == 0 ||
            strncmp(cMsgIdx, "       ", 7) == 0) {
          continue;
        }

      } else {
        if (strncmp(cMsgIdx, "ERROR", 5) ||
            strncmp(cMsgIdx, "WARNING", 7)) {
          continue;
        }
      }

      *(cMsgIdx-1) = '\0';
      break;
    }

    return cp;
  }

  return 0x0;
}

//----------------------------------------------------------- PKSmsg::clearMsg

// Clear the message buffer.

void PKSmsg::clearMsg(void)
{
  if (cMsgBuff) *cMsgBuff = '\0';
  cMsgIdx = cMsgBuff;
  cNMsg = 0;
}

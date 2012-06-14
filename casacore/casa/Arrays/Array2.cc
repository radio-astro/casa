//# Array2.cc: Template Arrays with slices, logical operations, and arithmetic
//# Copyright (C) 1993,1994,1995,1999
//# Associated Universities, Inc. Washington DC, USA.
//# 
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//# 
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//# 
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//# 
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: Array2.cc 20739 2009-09-29 01:15:15Z Malte.Marquarding $

#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/ArrayError.h>
#include <casa/Exceptions/CasaErrorTools.h>

// Include needed for debugging 120423; remove evenutally
#include <sstream>
#include <sys/time.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Begin Array Life Cycle debug code

ArrayLifecycleTracker::Filter ArrayLifecycleTracker::filter_p = NULL;
Bool ArrayLifecycleTracker::enable_p = False;
Int ArrayLifecycleTracker::min_p = 0;
FILE * ArrayLifecycleTracker::logFile_p = NULL;
ArrayLifecycleTracker::Tagger ArrayLifecycleTracker::tagger_p = NULL;

void
ArrayLifecycleTracker::addComment (const String & comment, const String & filename, int lineNumber)
{
    if (! enable_p){
        return;
    }

    fprintf (logFile_p, "Comment: t=%9.3f: '%s' (%s:%d)\n", now(), comment.c_str(), filename.c_str(), lineNumber);
}

void
ArrayLifecycleTracker::enable (Bool enableIt, Int min)
{
    if (enable_p != enableIt){
        if (enableIt){
            logFile_p = fopen ("array-events.log", "w");
        }
        else{
            fclose (logFile_p);
        }
    }

    enable_p = enableIt;

    if (min >= 0){
        min_p = min;
    }
}

double
ArrayLifecycleTracker::now ()
{
    struct timeval tv;
    gettimeofday (& tv, NULL);
    double t = tv.tv_sec % 86400 + tv.tv_usec * 1e-6;

    return t;
}

void
ArrayLifecycleTracker::setFilter (Filter filter)
{
    filter_p = filter;
}

void
ArrayLifecycleTracker::setTagger (Tagger tagger)
{
    tagger_p = tagger;
}

void
ArrayLifecycleTracker::track (void * object, const String & event, const IPosition & shapeNew,
                              void * storageNew, uInt elementSize, const String & signature)
{
    track (object, event, shapeNew, storageNew, IPosition (), 0, elementSize, signature);
}

void
ArrayLifecycleTracker::track (void * object, const String & event, const IPosition & shapeNew,
                              void * storageNew, const IPosition & shapeOld, void * storageOld,
                              uInt elementSize, const String & signature)
{
    if (! enable_p){
        return;
    }

    Bool filteredIn = True;
    Bool doStackTrace = False;

    if (filter_p != NULL){
        std::pair<Bool,Bool> filterFlags = filter_p (object, event, shapeNew, storageNew, shapeOld,
                                                     storageOld, elementSize, signature);

        filteredIn = filterFlags.first;
        doStackTrace = filterFlags.second;
    }

    if (! filteredIn){
        return;
    }

    int sizeNew = shapeNew.product() * elementSize;
    int sizeOld = shapeOld.product() * elementSize;

    if (sizeNew < min_p && sizeOld < min_p){
        return;
    }

    ostringstream os1;
    os1 << shapeNew;
    String shapeNewStr = os1.str();

    ostringstream os2;
    if (shapeOld.nelements() == 0){
        os2 << "[]";
    }
    else{
        os2 << shapeOld;
    }
    String shapeOldStr = os2.str();

    double t = now();

    string tag;
    if (tagger_p != NULL){
        tag = tagger_p ();
    }

    fprintf (logFile_p,
             "ArrayEvent: t=%9.3f: this=%p; e=%s; size=%d; shape=%s; storage=%p, "
             "old (size=%d, shape=%s. storage=%p) %s {%s}\n",
             t, object, event.c_str(), sizeNew, shapeNewStr.c_str(), storageNew,
             sizeOld, shapeOldStr.c_str(), storageOld, signature.c_str(), tag.c_str());

    if (doStackTrace){
        String stackTrace = AipsError::generateStackTrace ();
        stackTrace = CasaErrorTools::replaceStackAddresses (stackTrace);
        fprintf (logFile_p, "%s\n", stackTrace.c_str());
    }
}


// End Array Life Cycle debug code

// This probably isn't of interest to normal users. It returns the "volume" of
// an array (i.e. "nelements").
size_t ArrayVolume (uInt Ndim, const ssize_t *Shape)
{
    uInt i;
    if (aips_debug) {
	for (i=0; i < Ndim; i++)
	    if (Shape[i] < 0)
		throw(ArrayError("::ArrayVolume - negative shape"));
    }
    if (Ndim == 0)
	return 0;
    size_t total=1;
    for(i=0; i < Ndim; i++)
	total *= Shape[i];

    return total;
}

// This probably isn't of interest to normal users. Given a decimated
// array with a non-zero origin, what is the linear index into storage.
// Here we assume that the Shape is the original length, i.e. has INC
// in it.
size_t ArrayIndexOffset (uInt Ndim, const ssize_t *Shape,
                         const ssize_t *Origin, const ssize_t *Inc,
                         const IPosition &Index)
{
    uInt i;
    if (aips_debug) {
	for (i=0; i < Ndim; i++)
	    if (Index(i) < Origin[i] || Index(i) > (Origin[i] + Shape[i] - 1) ||
		Shape[i] < 0 || Inc[i] < 1)
		throw(ArrayError("::ArrayIndexOffset - negative shape or inc"
				 "<1 or out-of-bounds index"));
    }
    size_t offset = (Index(0) - Origin[0])*Inc[0];
    for (i=1; i < Ndim; i++)
	offset += (Index(i) - Origin[i])*Inc[i]*ArrayVolume(i, Shape);

    return offset;
}

size_t ArrayIndexOffset (uInt Ndim, const ssize_t *Shape,
                         const ssize_t *Inc, const IPosition &Index)
{
    uInt i;
    if (aips_debug) {
	for (i=0; i < Ndim; i++)
	    if (Index(i) < 0 || Index(i) >= Shape[i] ||
		Shape[i] < 0 || Inc[i] < 1)
		throw(ArrayError("::ArrayIndexOffset - negative shape or inc"
				 "<1 or out-of-bounds index"));
    }
    size_t offset = Index(0)*Inc[0];
    for (i=1; i < Ndim; i++)
	offset += Index(i)*Inc[i]*ArrayVolume(i, Shape);

    return offset;
}

} //# NAMESPACE CASA - END


// """
// Example python 3 script for processing the output of the array life cycle
// event log.
//
// Processes a log of array life cycle events generated by instrumenting
// the casa::Array object.  This creates a log file containing array
// events, stack traces and comments.  That log file should be ran
// through addLineNumberToTrace.py and then c++filt:
//
//     cat array-events.log | addLineNumberToTrace.py | c++filt > array-events.txt
//
// To use this file then do:
//
// import parseLog as pl
// lp = pl.LogParser ("array-events.txt", 1) # print summary every 1 second
// lp.processEvents (onChangeOnly=True) # print a summary only when storage being monitored changes
//
// This produces output along the lines of the excerpt below.  The summary lines all start by giving
// the time to the nearest second, the number of bytes being monitored, the number of arrays (bogus below)
// and the number of distinct blocks of storage (i.e., if two arrays share a piece of storage then
// there will be only one count in nDistinct).  Following the summary header line is a list of the
// distinct arrays in existence.  Each line starts with the count, the shape of the array, the element
// size of the array in bytes and then a list of the stack trace indices for where these arrays were
// created.  Shown below is the stack trace having index 153.  At the moment the stack trace shown
// stops at any method in casac::imager.  This can be changed if another area is being monitored
// and is needed to avoid displaying the large number of stack entries that are internal to
// the python implementation and generally not of interest.
//
//
// t=83511; nBytes=652.3 MB; nArrays=    0 nDistinct=    9
//   1 x (3000, 3000, 1, 1):4 [81]
//   2 x (3000, 3000, 2, 1):8 [11, 14]
//   2 x (3000, 3000):4 [7, 9]
//   4 x (3000, 3000, 2, 1):4 [10, 12, 13, 15]
// Comment: t=83530.653: 'Begin Major Cycle' (/home/orion/casa/active/code/synthesis/implement/MeasurementComponents/MFCleanImageSkyModel.cc:329)
//
// t=83530; nBytes=618.0 MB; nArrays=    0 nDistinct=    8
//   2 x (3000, 3000, 2, 1):8 [11, 14]
//   2 x (3000, 3000):4 [7, 9]
//   4 x (3000, 3000, 2, 1):4 [10, 12, 13, 15]
//
// ...
//
// 153:: Storage trace for 0x2aab07879010 created t=85135 [3000, 3000, 2, 1]:8
// ... casa::Array<std::complex<float> >::Array @ Array.tcc:113
// ... casa::RecordRep::createDataField @ RecordRep.cc:390
// ... casa::RecordRep::addDataField @ RecordRep.cc:175
// ... casa::Record::addDataField @ Record.cc:205
// ... casa::RecordInterface::defineField @ RecordInterface.cc:186
// ... casa::RecordInterface::define @ RecordInterface.cc:289
// ... casa::ImageInterface<std::complex<float> >::toRecord @ ImageInterface.tcc:321
// ... casa::FTMachine::toRecord @ FTMachine.cc:952
// ... casa::GridFT::toRecord @ GridFT.cc:1247
// ... casa::CubeSkyEquation::gradientsChiSquared @ CubeSkyEquation.cc:807
// ... casa::ImageSkyModel::makeNewtonRaphsonStep @ ImageSkyModel.cc:278
// ... casa::MFCleanImageSkyModel::solve @ MFCleanImageSkyModel.cc:730
// ... casa::SkyEquation::solveSkyModel @ SkyEquation.cc:816
// ... casa::Imager::clean @ Imager.cc:3877
// ... casa::Imager::iClean @ Imager.cc:6717
// ... casac::imager::clean @ imager_cmpt.cc:264
//
// ...
//
// """
//
// import cgi
// import functools
// import operator
// import re
// import sys
//
// class LogParser:
//
//     identity = lambda x: x
//     fromHex = lambda s: 0 if s=="(nil)" else int (s, 16)
//     toFloat = lambda x: float (x)
//     toInt = lambda x: int (x)
//
//     parsers = [ ("time", toFloat), ("object", fromHex), ("event", identity),
//                 ("size", toInt), ("shape", identity), ("oldSize", toInt),
//                 ("oldShape", identity), ("signature", identity), ("storage", fromHex),
//                 ("oldStorage", fromHex)]
//
//     Add = 1
//     Delete = -1
//     DeleteOld = -2
//     Nothing = 0
//
//     eventProcessors = {
//      "ctor" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.Add, returnStorage=True),
//      "ctorCpyDel":
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.DeleteOld, returnStorage=False),
//      "ctorCpy" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.Nothing, returnStorage=False),
//      "dtorDel" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Delete, storageAction=LogParser.Delete, returnStorage=False),
//      "dtorNod" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Delete, storageAction=LogParser.Nothing, returnStorage=False),
//      "refDel" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.DeleteOld, returnStorage=True),
//      "ref" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.Nothing, returnStorage=False),
//      "op=Del" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.Add, returnStorage=True),
//      "op=" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.Nothing, returnStorage=False),
//      "resizeDel" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.DeleteOld, returnStorage=True),
//      "resize" :
//      lambda s,l: s.processNewStorage (l, objectAction=LogParser.Add, storageAction=LogParser.Nothing, returnStorage=False)}
//
//     def __init__ (self, filename, interval = 1, debug=False):
//         self.__asHtml = False
//         self.__debug = debug
//         self.__interval = interval
//         self.__filename = filename
//         self.__onChangeOnly = False
//         self.reset()
//
//     def reset (self):
//         self.__time = 0
//         self.__nBytes = 0
//         self.__nBytesOld = -1
//         self.__nArraysDistinct = 0
//         self.__nArraysDistinctOld = -1
//         self.__objects = set()
//         self.__storage = dict ()
//         self.__storageTrace = []
//         self.__lastEvent = "*NONE*"
//         self.__lastEventLine = "*NONE*"
//         self.__lastEventLineNumber = -1
//         self.__lineNumber = -1
//         self.__nIgnored = 0
//         self.__nProcessed = 0
//         self.__nComments = 0
//         self.__nUnmatched = 0
//         self.__stackTrace = []
//
//     def captureStackTrace (self, line, lastStorage, starting=False):
//
//         lineIsTraceStart = re.match (r"^\|\|\> Stack trace.*$", line) is not None
//         capturingStack = len (self.__stackTrace) > 0
//
//         if not lineIsTraceStart:
//             if not capturingStack:
//                 return False, lastStorage
//         else:
//             if capturingStack:
//                 sys.stdout.flush()
//                 assert False, "Nested stack traces???\n{0}".format (self.__stackTrace)
//             else:
//                 self.__stackTrace = []
//
//         self.__stackTrace.append (line)
//         stillCapturing = re.match ("^.*casac::imager::.*$", line) is None
//
//         assert (len (self.__stackTrace) < 100)
//
//         if not stillCapturing:
//
//             # replace relevant entry with one containing a stack strace
//
//             replaceStackTrace = True
//
//             if lastStorage == 0:
//                 pass # ignore things storage at address 0x0
//             elif lastStorage not in self.__storage:
//
//                 if self.__lastEvent not in {"ref", "dtorDel"}:
//
//                     s = "--> Stack Trace without eligible storage location 0x{4} at {1}; previous event ({3}@{2})\n {0}."
//                     print (s.format (self.__lastEventLine, self.__lineNumber, self.__lastEventLineNumber, self.__lastEvent,
//                                      lastStorage))
//
//             elif len (self.__storage [lastStorage]) == 2:
//
//                 # Add a stack strace to this storage entry if this entry doesn't
//                 # already have a trace.  Unless there's an error in the logic the
//                 # newer trace is probably because the stack was dumped twice for
//                 # the same operation.
//
//                 shape, size = self.__storage [lastStorage]
//
//                 self.__storageTrace.append ((lastStorage, self.__time, shape, size, self.__stackTrace))
//                 self.__storage [lastStorage] = (shape, size, len (self.__storageTrace) - 1)
//                 lastStorage = None
//
//             self.__stackTrace = []
//
//         return (stillCapturing, lastStorage)
//
//     @classmethod
//     def ignoreSearch (cls, pattern):
//         return lambda line, reMatch: re.search (pattern, line) is not None
//
//     @classmethod
//     def ignoreSearchNot (cls, pattern):
//         return lambda line, reMatch: re.search (pattern, line) is None
//
//     def setDebug (self, debug=True):
//         self.__debug = debug
//
//     def matchLine (self, line):
//
//         pattern = ( r"ArrayEvent: t=(?P<time>[0-9.]+): this=0x(?P<object>[0-9a-f]+); e=(?P<event>[^;]+);"
//                     r" size=(?P<size>[0-9]+); shape=(?P<shape>\[[0-9, ]*\]); storage=(0x)?(?P<storage>[0-9a-f]+|\(nil\)), "
//                     r"old \(size=(?P<oldSize>[0-9]+), "
//                     r"shape=(?P<oldShape>\[[0-9, ]*\]). storage=(0x)?(?P<oldStorage>[0-9a-f]+|\(nil\))\)"
//                     r" (?P<signature>[^{]*)"
//                     r"[{]vm=(?P<vm>[0-9.]+) MB, res=(?P<res>[0-9.]+) MB[}].*$")
//
//         m = re.match (pattern, line)
//
//         return m
//
//
//     def cleanStackTrace (self, stackTrace, showArgs=False, showDeltaPc=False, showPc=False,
//                          showFile=True, showLine=True, trimFile=True, endPattern="casac::imager"):
//
//         pattern = (r"^[^(]+[(]"                    # skip over everything until a "("
//                    r"(?P<name>[^(]+)"              # grab the method name (everything until antoher "(")
//                    r"(?P<args>[(][^)]*[)])"        # grab method args (everything until a ")")
//                    r"[+](?P<deltaPc>0x[0-9a-f]+)"  # grab address offset relative to method
//                    r"[^[]+[[][+]?"                 # grab address or address relative to shared object
//                    r"(?P<pc>0x[0-9a-f]+)\]"
//                    r"( *[(](?P<filename>[^:]+):(?P<line>[0-9]+)[)])?" # optionally grab (file:line)
//                    r".*$")                         # ignore rest of line
//         ignoredNames = ["casa::AipsError::generateStackTrace", "casa::ArrayLifecycleTracker::track",
//                         "casa::ArrayLifecycleTracker::track"]
//
//         cleanedTrace = []
//
//         for entry in stackTrace:
//
//             m = re.match (pattern, entry)
//
//             line = ""
//
//             if m is not None and not m.group ("name") in ignoredNames:
//
//                 line = m.group ("name")
//
//                 if showArgs:
//                     line += m.group ("args")
//
//                 if showDeltaPc:
//                     line += m.group ("deltaPc")
//
//                 if showPc:
//                     line += m.group ("pc")
//
//                 if showFile:
//                     filename = m.group ("filename")
//                     if filename is not None:
//                         if trimFile:
//                             filename = filename.split ("/")[-1]
//                         line += " @ {0}".format (filename)
//
//                 if showLine:
//                     lineNumber = m.group ("line")
//                     if lineNumber is not None:
//                         line += ":{0}".format (lineNumber)
//
//                 cleanedTrace.append (line)
//
//             if re.match (endPattern, line):
//                 break; # last item to process so exit loop
//
//             #else:
//             #    print ("No match:'{0}'".format (entry))
//
//         return cleanedTrace
//
//     def printStorageTrace (self):
//
//         for i in range (len(self.__storageTrace)):
//
//             storage, t, shape, size, stack = self.__storageTrace [i]
//             stack = self.cleanStackTrace (stack)
//             splits = shape.rstrip("]").strip("[").split(",")
//             splits = tuple ([int(i) for i in splits])
//             nElements = functools.reduce (operator.mul, splits)
//             elementSize = int(round (size / nElements)) # in bytes
//
//             picturePrefix = "\n{2:2d}"
//             if self.__asHtml:
//                 picturePrefix = "<p/><a id=st{2}>{2:2d}</a>"
//
//             picture = picturePrefix + ":: Storage trace for 0x{0:x} created t={1} {3}:{4}"
//             print (picture.format (storage, t, i, shape, elementSize))
//
//             for j in stack:
//                 traceLine = j if not self.__asHtml else cgi.escape (j)
//                 picture = "" if not self.__asHtml else "<br/>..." + "... {0}"
//                 print (picture.format (traceLine))
//
//     def printSummary (self, match):
//
//         result = dict()
//
//         for s in sorted (self.__storage.values()):
//             shape = s[0]
//             size = s[1]
//             index = s[2] if len (s) == 3 else -1
//             splits = shape.rstrip("]").strip("[").split(",")
//             shape = tuple ([int(i) for i in splits])
//             nElements = functools.reduce (operator.mul, shape)
//             elementSize = int(round (size / nElements)) # in bytes
//             key = (shape, elementSize)
//             if key in result:
//                 oldCount, oldIndices = result [key]
//                 result [key] = (oldCount + 1, oldIndices + [index])
//             else:
//                 result [key] = (1, [index])
//
//         nBytes = self.totalStorage ()
//
//         if self.__onChangeOnly and nBytes == self.__nBytesOld and self.__nArraysDistinct == self.__nArraysDistinctOld:
//             return
//
//         ### print (self.__onChangeOnly, self.__nBytes, self.__nBytesOld, self.__nArrays, self.__nArraysOld)
//
//         nBytes = self.totalStorage ()
//
//         delta = nBytes - self.__nBytesOld
//         self.__nBytesOld = nBytes
//         self.__nArraysDistinctOld = self.__nArraysDistinct
//
//         picture = "t={0:5d}; mem={1:6.1f} MB; dMem={2:6.1f} MB; nDistinct={3:5d}; vm={4:6s}; rss={5:6s}"
//         if self.__asHtml:
//             picture = "<p/>" + picture
//
//         outputString = picture.format (self.__time,
//                                        nBytes / (1024*1024),
//                                        delta / (1024 * 1024),
//                                        len (self.__storage),
//                                        match.group ("vm"),
//                                        match.group ("res"))
//
//         if self.__asHtml:
//             outputString = outputString.replace (" ", "&nbsp;")
//
//         print (outputString)
//
//         picture = "{0:3d} x {1}:{2} {3}"
//         if self.__asHtml:
//             picture = "<br/>&nbsp;&nbsp;&nbsp; " + picture
//
//         for i in result.items():
//             if self.__asHtml:
//                 refs = "[" + ", ".join (["<a href=#st{0}>{0}</a>".format (j) for j in i[1][1]]) + "]"
//                 print (picture.format (i[1][0], i[0][0], i[0][1], refs))
//             else:
//                 print (picture.format (i[1][0], i[0][0], i[0][1], i[1][1]))
//
//     def processNewStorage (self, lineGroups, objectAction=Nothing,
//                            storageAction=Nothing, returnStorage = False):
//
//         if objectAction == LogParser.Delete:
//             self.__objects.discard (lineGroups ["object"])
//         else:
//             self.__objects.add (lineGroups ["object"])
//
//         if storageAction == LogParser.Add:
//             self.__storage [lineGroups ["storage"]] = (lineGroups ["shape"], lineGroups ["size"])
//             self.__nBytes += lineGroups ["size"]
//         elif storageAction == LogParser.Delete:
//             self.__storage.pop (lineGroups ["storage"], None)
//             self.__nBytes -= lineGroups ["size"]
//             self.__nArraysDistinct -= 1
//         elif storageAction == LogParser.DeleteOld:
//             self.__storage.pop (lineGroups ["oldStorage"], None)
//             self.__nBytes += lineGroups ["size"] - lineGroups ["oldSize"]
//             self.__nArraysDistinct -= 1
//
//         return lineGroups ["storage"] if returnStorage else None
//
//     def processEvent (self, lineGroups):
//
//         processor = LogParser.eventProcessors [lineGroups ["event"]]
//         return processor (self, lineGroups)
//
//     def processFile (self, ignoreLine=None, onChangeOnly=False, asHtml=False):
//
//         self.__onChangeOnly = onChangeOnly
//         self.__asHtml = asHtml
//
//         self.reset()
//
//         if ignoreLine is None:
//             ignoreLine = lambda l, m: False
//
//         if self.__asHtml:
//             print ("<tt>")
//
//         capturingStackTrace = False
//         self.__lineNumber = 0
//         lastStorage = None
//
//         for line in open (self.__filename):
//
//             try:
//
//                 self.__lineNumber += 1
//
//                 capturingStackTrace, lastStorage = self.captureStackTrace (line, lastStorage)
//
//                 if not capturingStackTrace:
//
//                     m = self.matchLine (line)
//
//                     if m is not None and not ignoreLine (line, m):
//                         self.__nProcessed += 1
//                         lastStorage = self.processLine (m)
//                         self.__lastEvent = m.group ("event")
//                         self.__lastEventLine = line
//                         self.__lastEventLineNumber = self.__lineNumber
//                     elif re.match ("^Comment:.*$", line):
//                         self.__nComments += 1
//                         print (line)
//                     else:
//                         self.__nUnmatched += 1
//
//             except Exception as e:
//
//                 print ("While processing line {0}:\n{1}".format (self.__lineNumber, line))
//                 raise
//
//         self.printStorageTrace ()
//
//         picture = "--- Summary: nLines={0}, nMatched={1}, nComments={2}, nUnmatched={3}"
//         if self.__asHtml:
//             picture = "<br/>" + picture
//
//         print (picture.format (self.__lineNumber, self.__nProcessed, self.__nComments, self.__nUnmatched))
//
//         if self.__asHtml:
//             print ("</tt>")
//
//     def processLine (self, match):
//
//         result = dict()
//
//         for p in LogParser.parsers:
//             name = p[0]
//             result [name] = p[1] (match.group(name))
//
//         if result ["time"] >= self.__time + self.__interval:
//
//             # flush results
//
//             self.printSummary (match)
//
//             self.__time = int (result ["time"])
//             self.__time = self.__time - self.__time % self.__interval
//
//         event = result ["event"]
//
//         if (self.__debug):
//             print ("e={0:7s}; sz={1:7.1f}<--{4:7.1f} MB; in={2:5}; o=0x{3:016x}".format (result["event"],
//                                                                        result["size"] / (1024*1024),
//                                                                        result ["object"] in self.__objects,
//                                                                        result ["object"],
//                                                                        result["oldSize"] / (1024*1024)))
//
//         lastStorage = self.processEvent (result)
//
//         return lastStorage
//
//     def totalStorage (self):
//
//         nBytes = 0
//
//         for s in self.__storage.values():
//
//             nBytes += s[1]
//
//         return nBytes
//
//
// def testTraceCleaner ():
//
//     sampleStackTrace = [
// r"||> Stack trace (use c++filt to demangle):",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libcasa_casa.so(casa::AipsError::generateStackTrace()+0x24) [+0x1f5a02] (/home/orion/casa/active/casacore/casa/Exceptions/Error2.cc:123)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libcasa_casa.so(casa::ArrayLifecycleTracker::track(void*, casa::String const&, casa::IPosition const&, void*, casa::IPosition const&, void*, unsigned int, casa::String const&)+0x334) [+0x1773f4] (/home/orion/casa/active/casacore/casa/Arrays/Array2.cc:156)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libcasa_casa.so(casa::ArrayLifecycleTracker::track(void*, casa::String const&, casa::IPosition const&, void*, unsigned int, casa::String const&)+0x67) [+0x1775cd] (/home/orion/casa/active/casacore/casa/Arrays/Array2.cc:99)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libxmlcasa.so.3.5.0(casa::Array<float>::Array(casa::IPosition const&)+0x161) [+0x3762b9] (/home/orion/casa/active/linux_64b/Debug/include/casacore/casa/Arrays/Array.tcc:113)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libxmlcasa.so.3.5.0(casa::ArrayLattice<float>::ArrayLattice(casa::IPosition const&)+0x3d) [+0x3defa9] (/home/orion/casa/active/linux_64b/Debug/include/casacore/lattices/Lattices/ArrayLattice.tcc:48)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libxmlcasa.so.3.5.0(casa::TempLattice<float>::init(casa::TiledShape const&, double)+0x3b0) [+0x3e1962] (/home/orion/casa/active/linux_64b/Debug/include/casacore/lattices/Lattices/TempLattice.tcc:125)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libxmlcasa.so.3.5.0(casa::TempLattice<float>::TempLattice(casa::TiledShape const&, int)+0x6f) [+0x3e1a21] (/home/orion/casa/active/linux_64b/Debug/include/casacore/lattices/Lattices/TempLattice.tcc:54)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libdisplay.so.3.5.0(casa::TempImage<float>::TempImage(casa::TiledShape const&, casa::CoordinateSystem const&, int)+0x8f) [+0x7945e1] (/home/orion/casa/active/linux_64b/Debug/include/casacore/images/Images/TempImage.tcc:67)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libsynthesis.so.3.5.0(casa::Imager::makePBImage(casa::PBMath&, casa::ImageInterface<float>&)+0x28f) [+0xf081ef] (/home/orion/casa/active/code/synthesis/implement/MeasurementEquations/Imager2.cc:4093)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libsynthesis.so.3.5.0(casa::Imager::makePBImage(casa::CoordinateSystem const&, casa::PBMath&, casa::String const&)+0x88) [+0xf08d66] (/home/orion/casa/active/code/synthesis/implement/MeasurementEquations/Imager2.cc:4062)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libsynthesis.so.3.5.0(casa::Imager::makePBImage(casa::CoordinateSystem const&, casa::String const&, casa::String const&, bool, double)+0x52b) [+0xf09619] (/home/orion/casa/active/code/synthesis/implement/MeasurementEquations/Imager2.cc:4037)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libsynthesis.so.3.5.0(casa::Imager::makeimage(casa::String const&, casa::String const&, casa::String const&, bool)+0x8c8) [+0xeb3e0e] (/home/orion/casa/active/code/synthesis/implement/MeasurementEquations/Imager.cc:2957)",
// r"/home/orion/casa/active/linux_64b/Debug/lib/libxmlcasa.so.3.5.0(casac::imager::makeimage(std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, bool, bool)+0xbc) [+0x54f990] (/home/orion/casa/active/code/xmlcasa/implement/synthesis/imager_cmpt.cc:612)",
// r"/home/orion/casa/active/linux_64b/Debug/../Debug/python/2.6/casac.so(CCM_Local::casac::CCM_Session_imager::CCM_imager_impl::makeimage(std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, bool, bool)+0x4f) [+0x57e3ed] (/home/orion/casa/active/code/build/Debug/xmlcasa/impl/imager_impl.cc:386)",
// r"/home/orion/casa/active/linux_64b/Debug/../Debug/python/2.6/casac.so(CCM_Local::casac::CCM_Session_imager::imager::makeimage(std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, bool, bool)+0x5e) [+0x575fba] (/home/orion/casa/active/code/build/Debug/xmlcasa/CCM_Local/casac/CCM_Session_imager/imager_gen.cc:277)",
// r"/home/orion/casa/active/linux_64b/Debug/../Debug/python/2.6/casac.so [0x2b498eb2bcea]",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x4b20) [+0x3d762d0110] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3965)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x466f) [+0x3d762cfc5f] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3846)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x545e) [+0x3d762d0a4e] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3836)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0 [0x2b498327152c]",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyObject_Call+0x68) [+0x3d762437a8] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Objects/abstract.c:2492)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0 [0x2b4983259afd]",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyObject_Call+0x68) [+0x3d762437a8] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Objects/abstract.c:2492)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0 [0x2b498325a6fb]",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyObject_Call+0x68) [+0x3d762437a8] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Objects/abstract.c:2492)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x200a) [+0x3d762cd5fa] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3968)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCode+0x32) [+0x3d762d1b62] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:547)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0 [0x2b49832f32a2]",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyRun_FileExFlags+0x96) [+0x3d762eb376] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/pythonrun.c:1325)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0 [0x2b49832d1a4f]",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x5949) [+0x3d762d0f39] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3750)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCode+0x32) [+0x3d762d1b62] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:547)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x56ac) [+0x3d762d0c9c] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:4445)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x466f) [+0x3d762cfc5f] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3846)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x466f) [+0x3d762cfc5f] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3846)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x545e) [+0x3d762d0a4e] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3836)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x545e) [+0x3d762d0a4e] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3836)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x466f) [+0x3d762cfc5f] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3846)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalFrameEx+0x466f) [+0x3d762cfc5f] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3846)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCodeEx+0x945) [+0x3d762d1b25] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:3000)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyEval_EvalCode+0x32) [+0x3d762d1b62] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/ceval.c:547)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0 [0x2b49832f32a2]",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyRun_FileExFlags+0x96) [+0x3d762eb376] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/pythonrun.c:1325)",
// r"/usr/lib64/casapy/lib/libpython2.6.so.1.0(PyRun_SimpleFileExFlags+0x1f7) [+0x3d762ec897] (/var/rpmbuild/BUILD/casapy-python/Python-2.6.5/Python/pythonrun.c:935)",
// r"/usr/lib64/casapy/lib/libccmtools_local.so.0.5.5(CCM_Python::Py::evalFile(char const*)+0x4e) [+0x19a1e] (??:0)",
// r"casapy(main+0x8e6) [0x401eaa]",
// r"/lib64/libc.so.6(__libc_start_main+0xf4) [+0x3d7521d9b4] (??:0)",
// r"casapy(__gxx_personality_v0+0x109) [0x4014a9]"
//     ]
//
//     lp = LogParser ("array-events.tmp", 1)
//     s = lp.cleanStackTrace (sampleStackTrace)
//     for i in s:
//         print (i)
//
// def doTesting ():
//     testTraceCleaner()
//
//     lp = LogParser ("array-events.tmp", 1)
//     m = lp.matchLine (r"ArrayEvent: t=71246.947: this=0x7fff17d1ba80; e=ctorCpy; size=941892; shape=[235473]; storage=0x17b8c9e0, old (size=0, shape=[]. storage=(nil)) casa::Array<T>::Array(const casa::Array<T>&) [with T = unsigned int] {vm=1388.6 MB, res=187.3 MB}")
//     if m is None:
//         print ("No match ???")
//     else:
//         print (m.groups())
//
// #    lp.processFile ()
//
//
//
// if __name__ == "__main__":
//
//     doTesting ()
//
//
//
//
//

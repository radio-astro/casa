#*******************************************************************************
# ALMA - Atacama Large Millimeter Array
# Copyright (c) NAOJ - National Astronomical Observatory of Japan, 2011
# (in the framework of the ALMA collaboration).
# All rights reserved.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#*******************************************************************************
#
# $Revision: 1.29.2.2.2.1 $
# $Date: 2012/09/19 20:58:59 $
# $Author: ldavis $
#
#import casac
from casac import casac
import os
import sys
import numpy
from decimal import *
from taskinit import gentools

from asap.scantable import is_scantable,is_ms

class SDLogger:
    """
    Logger class that wraps casalogger
    """
    #loghome=casac.homefinder.find_home_by_name('logsinkHome')
    
    def __init__( self, level=2, consolelevel=3, origin='PIPELINEHeuristics', fileout='casapy.log', Global=False ):
        """
        level -- log level for logger output
        consolelevel -- log level for console output ( must be <= level )
        origin -- origin of the message
        fileout -- output file name
        Global -- whether this logger is global or not
        """
        import os

        # show log
        self.showLog = True

        # global or not
        self.isGlobal = Global

        # set origin
        self.origin = origin
        
        # create logger
        self.logger = None
        self.create()

        # set log level
        self.LogLevel = level
        self.ConsoleLevel = consolelevel
        self.setLogLevel( level, consolelevel )
        self.ConsoleOut = True

        # set logfile
        self.fileOut = None
        self.setFileOut( fileout )
        self.fileOut = fileout

    def __del__(self):
        #del self.loghome
	pass

    def disableConsoleOutput( self ):
        """
        Disable console output
        """
        self.ConsoleOut = False

    def enableConsoleOutput( self ):
        """
        Enable console output
        """
        self.ColsoleOut = True

    def isConsoleOutputEnabled( self ):
        """
        """
        return self.ConsoleOut

    def setLogLevel( self, level, consolelevel=3 ):
        """
        Set log level.
        """
        self.LogLevel = level
        self.ConsoleLevel = consolelevel
        if level == 0:
            self.logger.filter( 'ERROR' )
        elif level == 1:
            self.logger.filter( 'WARN' )
        elif level == 2:
            self.logger.filter( 'INFO' )
        elif level == 3:
            self.logger.filter( 'DEBUG2' )
        else:
            self.logger.filter( 'DEBUG' )

    def __del__( self ):
        """
        """
        self.logger.setlogfile( 'casapy.log' )
        self.logger.filter( 'INFO' )
        self.logger.origin( 'casa' )

    def setFileOut( self, fileout ):
        """
        Set output file name.
        """
        if type(fileout) != str:
            self.showLog = False
            return
        
##         if self.fileOut != None and fileout == self.fileOut:
##             self.setLogLevel( self.LogLevel )
##             return
        
        import os
        self.showLog = True
        if not os.path.exists( fileout ) or os.path.isfile( fileout ):
            del self.logger
            self.create()
            self.logger.setlogfile( fileout )
        else:
            self.logger.post('fileout is not a regular file. set logfile casapy.log.','WARN','Logger::setFileOut')
            del self.logger
            self.create()
            self.logger.setlogfile('casapy.log')
        self.setLogLevel( self.LogLevel, self.ConsoleLevel )
        self.fileOut = fileout

    def LogMessage( self, Category='TITLE', Origin='', Msg='...', ConsoleOut=True ):
        """
        Write logging message.
        """
        if not self.showLog:
            return 
        
        CATEGORY = Category.upper()
        if CATEGORY == 'TITLE':
            Level = 0
            Priority = 'INFO'
            # Heading = '\nTITLE: '
            Heading = ''
        elif CATEGORY == 'ERROR':
            Level = 0
            Priority = 'ERROR'
            Heading = '\nERROR: '
        elif CATEGORY == 'WARNING':
            Level = 1
            Priority = 'WARN'
            Heading = 'WARNING: '
        elif CATEGORY == 'TIMER':
            Level = 1
            Priority = 'INFO'
            Heading = ''
        elif CATEGORY == 'PROGRESS':
            Level = 1
            Priority = 'INFO'
            Heading = 'PROGRESS: '
        elif CATEGORY == 'INFO':
            Level = 2
            Priority = 'INFO'
            Heading = 'INFO: '
        elif CATEGORY == 'DEBUG':
            Level = 3
            Priority = 'DEBUG2'
            Heading = 'DEBUG: '
        else:
            Level = 4
            Priority = 'DEBUG'
            Heading = 'OTHER: '

        if Level > self.LogLevel: return

        if self.ConsoleOut and ConsoleOut and Level <= self.ConsoleLevel:
            self.logger.showconsole(True)
        #    print '%s%s' % (Heading, Msg)
        #    sys.stdout.flush()
            
        self.logger.post( '%s'%(Msg),Priority,Origin )
        self.logger.showconsole(False)

    def create( self ):
        """
        Create logger
        """
        if self.isGlobal:
            from taskinit import casalog
            self.logger = casalog
            self.logger.origin(self.origin)
        else:
            #self.logger = self.loghome.create()
            self.logger = casac.logsink()
            self.logger.origin(self.origin)
    
        
        

def LogMessage(Category='Title', LogLevel=2, FileOut=False, ConsoleOut=True, Msg='...'):
    """
    Category: Title, Error, Warning, Timer, Progress, Info, Debug, Other
    FileOut: False: no output, 'FileName': output to file
    ConsoleOut: True: print on console, False: not print on console
    LogLevel: Show message if the level higher than LogLevel
       LogLevel=0: Title, Error
       LogLevel=1: Warning, Timer, Progress
       LogLevel=2: Info
       LogLevel=3: Debug
       LogLevel=4: Other      
    Msg: 'Message'
    """
    CATEGORY = Category.upper()
    if CATEGORY == 'TITLE':
        Level = 0
        #Heading = '\nTITLE: '
        Heading = ''
    elif CATEGORY == 'ERROR':
        Level = 0
        Heading = '\nERROR: '
    elif CATEGORY == 'WARNING':
        Level = 1
        Heading = 'WARNING: '
    elif CATEGORY == 'TIMER':
        Level = 1
        Heading = ''
    elif CATEGORY == 'PROGRESS':
        Level = 1
        Heading = 'PROGRESS: '
    elif CATEGORY == 'INFO':
        Level = 2
        Heading = 'INFO: '
    elif CATEGORY == 'DEBUG':
        Level = 3
        Heading = 'DEBUG: '
    else:
        Level = 4
        Heading = 'OTHER: '

    if Level > LogLevel: return

    if ConsoleOut:
        print '%s%s' % (Heading, Msg)
        sys.stdout.flush()
    if FileOut != False:
        print >> FileOut, '%s%s' %(Heading, Msg)
        FileOut.flush()

    return


def FreqWindow2Channel(Abcissa, SpectrumWindow):
    """
    Convert Frequency Window (GHz) to Channel Window
    Input:
      Abcissa: [0]:channel [1]:Frequency(GHz) [2]:Velocity(Km/s) [3]:Wavelength(mm)
      SpectrumWindow: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
          if one velocity is specified (e.g., [CF, Vel]), read as [CenterFreq, -Vel, +Vel]
    Output:
      SpWin: [[CenterChannel, FullWidthChannel, True],,,]
      SpWinRange: [[channel0, channel1],,,,]
    """
    SpWin = []
    SpWinRange = []
    for n in range(len(SpectrumWindow)):
        # Check if the specified Freq. is within the range
        if SpectrumWindow[n][0] < max(Abcissa[1][0], Abcissa[1][-1]) and \
           SpectrumWindow[n][0] > min(Abcissa[1][0], Abcissa[1][-1]):
            # Convert Velocity to Freq.
            if len(SpectrumWindow[n]) == 2:
               MaxVel = abs(SpectrumWindow[n][1])
               MinVel = abs(SpectrumWindow[n][1]) * -1.0
            else:
               MaxVel = SpectrumWindow[n][2]
               MinVel = SpectrumWindow[n][1]
            MinFreq = SpectrumWindow[n][0] * (1.0 - MinVel / 299792.458)
            MaxFreq = SpectrumWindow[n][0] * (1.0 - MaxVel / 299792.458)
            # Convert Freq. to Channel
            DiffMinFreq = abs(MinFreq)
            DiffMaxFreq = abs(MaxFreq)
            MinChan = Abcissa[0][0]
            MaxChan = Abcissa[0][0]
            for i in range(len(Abcissa[0])):
                if abs(Abcissa[1][i] - MinFreq) < DiffMinFreq:
                    DiffMinFreq = abs(Abcissa[1][i] - MinFreq)
                    MinChan = i
                if abs(Abcissa[1][i] - MaxFreq) < DiffMaxFreq:
                    DiffMaxFreq = abs(Abcissa[1][i] - MaxFreq)
                    MaxChan = i
            if MinChan > MaxChan:
                tmpChan = MaxChan
                MaxChan = MinChan
                MinChan = tmpChan
            if MinChan > Abcissa[0][0]: MinChan -= 1
            if MaxChan < Abcissa[0][-1]: MaxChan += 1
            SpWinRange.append([int(MinChan), int(MaxChan)])
            SpWin.append([int((MinChan + MaxChan)/2.0 + 0.5), int(MaxChan - MinChan + 1), True])
    return(SpWin, SpWinRange)


def Channel2FreqWindow(Abcissa, ChannelWindow):
    """
    Convert Channel Window to Frequency Window (GHz)
    Input:
      Abcissa: [0]:channel [1]:Frequency(GHz) [2]:Velocity(Km/s) [3]:Wavelength(mm)
      ChannelWindow: [[min Channel, max Channel],[minC,maxC],,,]
    Output:
      SpectrumWindow: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
    """
    SpectrumWindow = []
    for n in range(len(ChannelWindow)):
        # Check if the Channel Window is within the range
        if ChannelWindow[n][0] >= min(Abcissa[0][0], Abcissa[0][-1]) and \
           ChannelWindow[n][1] < max(Abcissa[0][0], Abcissa[0][-1]):
            # Convert Channel to Frequency
            CenterChannel = (ChannelWindow[n][0] + ChannelWindow[n][1])/2.0
            Weight = CenterChannel - int(CenterChannel)
            CenterFreq = Abcissa[1][int(CenterChannel)]*(1.0-Weight) + Abcissa[1][int(CenterChannel)+1]*Weight
            # Convert Channel Width to Velocity
            Weight = ChannelWindow[n][0] - int(ChannelWindow[n][0])
            Freq0 = Abcissa[1][int(ChannelWindow[n][0])]*(1.0-Weight) + Abcissa[1][int(ChannelWindow[n][0])+1]*Weight
            Weight = ChannelWindow[n][1] - int(ChannelWindow[n][1])
            Freq1 = Abcissa[1][int(ChannelWindow[n][1])]*(1.0-Weight) + Abcissa[1][int(ChannelWindow[n][1])+1]*Weight
            Vel0 = 299792.458*(CenterFreq/max(Freq0,Freq1) - 1.0)
            Vel1 = 299792.458*(CenterFreq/min(Freq0,Freq1) - 1.0)
            SpectrumWindow.append([CenterFreq, min(Vel0,Vel1), max(Vel0,Vel1)])
    return(SpectrumWindow)


def PlotCheck(Flag, CurrentIteration, TotalIteration):
    """
    Flag: True, False, or 'Last'
    """
    if Flag != False:
        if Flag != 'Last' or CurrentIteration == TotalIteration: return True
    return False


####
# ProgressTimer
#
# Show the progress bar on the console if LogLevel is lower than or equal to 2.
#
####
class ProgressTimer:
    def __init__(self, length=80, maxCount=80, LogLevel=2):
        """
        Constructor:
            length: length of the progress bar (default 80 characters)
        """
        self.currentLevel = 0
        self.maxCount = maxCount
        self.curCount = 0
        self.scale = float(length)/float(maxCount)
        self.LogLevel = LogLevel
        if self.LogLevel <= 2:
            print '\n|' + '='*((length-8)/2) + ' 100% ' + '='*((length-8)/2) + '|'

    def __del__(self):
        if self.LogLevel <= 2:
            print '\n'

    def count(self, increment=1):
        if self.LogLevel <= 2:
            self.curCount += increment
            newLevel = int(self.curCount * self.scale)
            if newLevel != self.currentLevel:
                print '\b' + '*' * (newLevel - self.currentLevel),
                sys.stdout.flush()
                self.currentLevel = newLevel


####
# osfdatautils
#
# This is a utility package that includes several functions that execute
# necessary modifications and manipulations on the original data and
# create datasets to run on heuristics pipeline.
#
# Usage:
#
#   CASA <1>: import SDTool as SDT
#   CASA <2>: o=SDT.osfdatautils(filename='input_filename',format='data_format')
#   CASA <3>: o.domod()
#
# format is either 'ASDM' or 'MS2' depending on the data format of
# input file. The class may work with MS2 input, but it is originally
# defined to process ASDM.
#
# 2009/11/12 Takeshi Nakazato  Created
# 2010/01/08 TN                Renumbering SCAN_NUMBER is not needed any more
# 2010/05/13 TN                Changes for 3.0.1 or later
#                              (backward incompatible)
#                                 - splitant just calls asap.splitant
#                                 - renamesrc updated to see SRCTYPE column to
#                                   identify scan intents
# 2010/05/19 TN                Updates to import and use osfdatautils
#                              in StagesInterface
# 2010/05/21 TN                Removed several workarounds that are needed
#                              CASA 3.0 or earlier
# 2010/05/28 TN                Moved to SDTool.py
####

class osfdatautils:
    def __init__(self,indir='./',filename='',format='ASDM'):
        """
        Constructor.
        
        filename --- input data.
        format   --- format of input data ('ASDM' or 'MS2')
        """
        # local tool
        #self._tbtool = casac.homefinder.find_home_by_name('tableHome')
        #self._tb = self._tbtool.create()
        self._tb = gentools(['tb'])[0]
        
        self.format=format
        self.indir=indir
        self.fileorg=filename.rstrip('/')
        self.format = format
        if self.format=='ASDM':
            self.filems=self.fileorg+'.ms'
        elif self.format=='MS2':
            self.filems=self.indir+'/'+self.fileorg
        self.filemod=''
        splitname=self.filems.split('.')
        if splitname[-1]=='ms' or splitname[-1]=='MS':
            sep='.'
            splitname.insert(-1,'mod')
            self.filemod=sep.join(splitname)
        else:
            self.filemod=filename+'.mod.ms'
        self.filesep=[]
        self.antid=None
        self.stateid=None
        self.scannum=None
        self.obsmode=None
        self.numant=0
        self.ismodified=False
        self.haveRef=None
        self.numpol=0

    def __del__( self ):
        """
        """
        #del self._tbtool
        del self._tb

    def importasdm(self,outdir=''):
        """
        do importasdm
        """
        import os
        from tasks import importasdm
        if len(outdir) > 1 and outdir[-1] != '/':
            outdir=outdir+'/'
        if not os.path.exists(outdir+self.filems):
            #importasdm(asdm=self.indir+self.fileorg,vis=outdir+self.filems,corr_mode="all",srt="all",time_sampling="all",ocorr_mode="ao",compression=False,asis="",wvr_corrected_data="both",verbose=False,useversion='v2',showversion=False)
            importasdm(asdm=self.indir+self.fileorg,vis=outdir+self.filems,corr_mode="all",srt="all",time_sampling="all",ocorr_mode="ao",compression=False,asis="",wvr_corrected_data="both",verbose=False,showversion=False)
##         if os.path.exists(outdir+self.filems+'.ms'):
##             os.system( 'mv %s %s' %(self.filems+'.ms',self.filems) )
##             os.system( 'mv %s.flagversions %s.flagversions' %(self.filems+'.ms',self.filems) )
        self.filems=outdir+self.filems
        self.haveRef=self.haveReference()

    def splitant(self, outprefix='', overwrite=False):
        """
        Split Measurement set by antenna name, save data as a scantables,
        and return a list of filename.
        Notice this method can only be available from CASA. 
        Prameter
        outprefix:   the prefix of output scantable name.
                     the names of output scantable will be
                     outprefix.antenna1, outprefix.antenna2, ....
                     If not specified, outprefix = filename is assumed.
        overwrite    If the file should be overwritten if it exists.
                     The default False is to return with warning
                     without writing the output. USE WITH CARE.             
        """
        import os
        from asap import splitant
        if self.ismodified:
            filename=self.filemod
            prefix=fielname.rstrip('.mod.ms')
        else:
            filename=self.filems
            prefix=filename.rstrip('.ms')
        if ( outprefix == '' ):
            outprefix=prefix
        if not overwrite:
            expnames=self.getsplitname(outprefix=outprefix)
            allexists=True
            for iant in xrange(len(expnames)):
                if not os.path.exists(expnames[iant]):
                    allexists=False
            if allexists:
                self.filesep=expnames
                return
        self.filesep=splitant(filename=filename,outprefix=outprefix,overwrite=True)

    def getsplitname(self,outprefix=''):
        """
        Return expected filenames for split Scantables.
        """
        if self.ismodified:
            filename=self.filemod
            prefix=filename.rstrip('.mod.ms')
        else:
            filename=self.filems
            prefix=filename.rstrip('.ms')
        if ( outprefix == '' ):
            outprefix=prefix
        self._tb.open(filename)
        #anttab=self._tb.getkeyword('ANTENNA').split()[-1]
        anttab=self._tb.getkeyword('ANTENNA').lstrip('Table: ')
        self._tb.close()
        self._tb.open(anttab)
        antnames=self._tb.getcol('NAME')
        self._tb.close()
        expnames=[]
        for iant in xrange(len(antnames)):
            expnames.append( outprefix+'.'+antnames[iant]+'.asap' )
        return expnames

    def getmsmeta(self):
        """
        get metadata from MAIN table and STATE table of MS data.
        """
        if self.ismodified:
            filename=self.filemod
        else:
            filename=self.filems
        self._tb.open(filename)
        #stattab=self._tb.getkeyword('STATE').split()[-1]
        stattab=self._tb.getkeyword('STATE').lstrip('Table: ')
        self.antid=self._tb.getcol('ANTENNA1')
        self.stateid=self._tb.getcol('STATE_ID')
        self.scannum=self._tb.getcol('SCAN_NUMBER')
        colnames=self._tb.colnames()
        if 'FLOAT_DATA' in colnames:
            sp=self._tb.getcell('FLOAT_DATA',0)
        else:
            sp=self._tb.getcell('DATA',0)
        self.numpol=sp.shape[0]
        del sp
        del colnames
        self._tb.close()
        self._tb.open(stattab)
        self.obsmode=self._tb.getcol('OBS_MODE')
        self._tb.close()
        uniqueids=numpy.unique(self.antid)
        self.numant=len(uniqueids)

    def getasapname(self):
        """
        Just get created Scantable name.
        """
        if len(self.filesep)==0:
            if self.ismodified:
                filename=self.filemod
            else:
                filename=self.filems
            prefix=filename.rstrip('ms').rstrip('MS')
            self._tb.open(filename)
            #anttab=self._tb.getkeyword('ANTENNA').split()[-1]
            anttab=self._tb.getkeyword('ANTENNA').lstrip('Table: ')
            self._tb.close()
            self._tb.open(anttab)
            antnames=self._tb.getcol('NAME')
            self._tb.close()
            for antid in range(len(antnames)):
                outname=prefix+antnames[antid]+'.asap'
                self.filesep.append(outname)

    def fillRestFreq(self):
        """
        Fill RESTFREQUENCY column in MOLECULES table of Scantable.
        """
        import xml.dom.minidom as DOM
        import string
        if self.numant==0:
            self.getmsmeta()
        if len(self.filesep)==0:
            self.getasapname()
        rf=[]
        if self.format == 'ASDM':
            spwname='%s/%s/SpectralWindow.xml'%(self.indir,self.fileorg)
            dom3=DOM.parse(spwname)
            rootnode=dom3.getElementsByTagName('refFreq')
            for inode in range(len(rootnode)):
                sval=rootnode[inode].childNodes[0].data.encode('UTF-8')
                val=string.atof(sval)
                rf.append([val])
        else:
            # workaround when original ASDM is not provided
            # refer reference frequency of SPECTRAL_WINDOW table for MS
            self._tb.open(self.filems)
            #ddtab=self._tb.getkeyword('DATA_DESCRIPTION').split()[-1]
            #spwtab=self._tb.getkeyword('SPECTRAL_WINDOW').split()[-1]
            ddtab=self._tb.getkeyword('DATA_DESCRIPTION').lstrip('Table: ')
            spwtab=self._tb.getkeyword('SPECTRAL_WINDOW').lstrip('Table: ')
            datadescid=self._tb.getcell('DATA_DESC_ID',0)
            self._tb.close()
            self._tb.open(ddtab)
            spwid=self._tb.getcell('SPECTRAL_WINDOW_ID',datadescid)
            self._tb.close()
            self._tb.open(spwtab)
            val=self._tb.getcell('REF_FREQUENCY',spwid)
            rf.append([val])
        for iant in range(self.numant):
            self._tb.open(self.filesep[iant])
            #moltab=self._tb.getkeyword('MOLECULES').split()[-1]
            moltab=self._tb.getkeyword('MOLECULES').lstrip('Table: ')
            self._tb.close()
            self._tb.open(moltab,nomodify=False)
            nrow=self._tb.nrows()
            for irow in range(nrow):
                restfreq=self._tb.getcell('RESTFREQUENCY',irow)
                if ( len(restfreq) == 1 ) and ( restfreq[0] == 0.0 ):
                    self._tb.putcell('RESTFREQUENCY',irow,rf[irow])
        self._tb.flush()
        self._tb.close()
            
    def haveReference(self):
        """
        """
        self._tb.open(self.filems)
        #stattab=self._tb.getkeyword('STATE').split()[-1]
        stattab=self._tb.getkeyword('STATE').lstrip('Table: ')
        self._tb.close()
        self._tb.open(stattab)
        obsmode=self._tb.getcol('OBS_MODE')
        self._tb.close()
        haveref=False
        for irow in range(len(obsmode)):
            if obsmode[irow].find('OFF') != -1:
                haveref=True
                break
        return haveref

    def renumIFNO(self):
        """
        Set IFNO to be same as FREQ_ID
        """
        if self.numant==0:
            self.getmsmeta()
        if len(self.filesep)==0:
            self.getasapname()
        for iant in range(self.numant):
            self._tb.open(self.filesep[iant],nomodify=False)
            ifnocol=self._tb.getcol('IFNO')
            freqidcol=self._tb.getcol('FREQ_ID')
            if ifnocol[0]!=0 and freqidcol[0]==0:
                self._tb.putcol('IFNO',freqidcol)
                self._tb.flush()
            self._tb.close()

    def removePointingRef(self):
        """
        Remove path to POINTING table.
        """
        if self.numant==0:
            self.getmsmeta()
        if len(self.filesep)==0:
            self.getasapname()
        for iant in range(self.numant):
            self._tb.open(self.filesep[iant],nomodify=False)
            if 'POINTING' in self._tb.keywordnames():
                self._tb.removekeyword('POINTING')
                self._tb.flush()
            self._tb.close()

    def modifyobsmode(self):
        # modify separator in OBS_MODE string if it is '#'
        self._tb.open(self.filems)
        #statetab=self._tb.getkeyword('STATE').split()[-1]
        statetab=self._tb.getkeyword('STATE').lstrip('Table: ')
        self._tb.close()
        self._tb.open(statetab,nomodify=False)
        obsmode=self._tb.getcol('OBS_MODE')
        for irow in xrange(self._tb.nrows()):
            obsmode[irow]=obsmode[irow].replace('#','.')
        self._tb.putcol('OBS_MODE',obsmode)
        self._tb.close()

    def domod(self,outdir='',outprefix='',overwrite=True):
        """
        Do all necessary procedures and modifications sequentially.
        """
        # initial setup
        if len(outdir) > 1 and outdir[-1] != '/':
            outdir=outdir+'/'
        
        # import ASDM
        if self.format == 'ASDM':
            print 'importasdm'
            self.importasdm(outdir=outdir)

        # modify OBS_MODE string ('#'->'.')
        # this is really necessary for calibration
        print 'modifyobsmode'
        self.modifyobsmode()
        
        # split MS by antenna 
        print 'splitant'
        self.splitant(outprefix=outdir+outprefix,overwrite=overwrite)

        # fill RESTFREQUENCY
        print 'fillrestfreq'
        self.fillRestFreq()

        # renumber IFNO
        print 'renumIFNO'
        self.renumIFNO()

        # remove reference to POINTING table
        print 'removePointingRef'
        self.removePointingRef()

        # return filenames created
        return self.filesep

        

def changetag(filename,outname=None):
    """
    Change tag from SRCNAME to SRCTYPE along with the following rule:
    
    SRCNAME          SRCTYPE
    _pson, _ps        0
    _psoff, _psr      1
    _ps_calon        10
    _psr_calon       11
    _wobon            0
    _woboff           1
    _nod              2
    _nod_calon       12
    _sky              7
    _hot              8
    _cold             9
    _fson. _fs        3          
    _fsoff, _fsr      4
    _fs_calon        13
    _fsr_calon       14
    _fslo            20
    _fshi            30
    _fslo_off        21
    _fshi_off        31
    _fslo_sky        27
    _fshi_sky        37
    _fslo_hot        28
    _fshi_hot        38
    _fslo_cold       29
    _fshi_cold       39
    (none)           99

    Input data (filename) must be Scantable.
    """
    from asap._asap import srctype as stp
    import string
    if outname==None:
        self._tb.open(filename,nomodify=False)
        tborg=self._tb.copy(filename.rstrip('/')+'.org',deep=True,valuecopy=True)
        tborg.close()
    else:
        self._tb.open(filename)
        tbout=self._tb.copy(outname,deep=True,valuecopy=True)
        tbout.close()
        self._tb.close()
    self._tb.open(outname,nomodify=False)
    srcname=self._tb.getcol('SRCNAME')
    srctype=self._tb.getcol('SRCTYPE')
    srctype[:]=0
    for irow in range(self._tb.nrows()):
        ss=srcname[irow].split('_')
        if ss[-1]=='ps' or ss[-1]=='pson' or ss[-1]=='wobon':
            srctype[irow]=stp.pson
            srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='psr' or ss[-1]=='psoff' or ss[-1]=='woboff':
            srctype[irow]=stp.psoff
            srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='nod':
            srctype[irow]=stp.nod
            srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='fs' or ss[-1]=='fson':
            srctype[irow]=stp.fson
            srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='fsr' or ss[-1]=='fsoff':
            srctype[irow]=stp.fsoff
            srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='sky':
            if ss[-2]=='fslo':
                srctype[irow]=stp.flosky
                srcname[irow]=string.join(ss[:-2],'_')
            elif ss[-2]=='fshi':
                srctype[irow]=stp.fhisky
                srcname[irow]=string.join(ss[:-2],'_')
            else:
                srctype[irow]=stp.sky
                srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='hot':
            if ss[-2]=='fslo':
                srctype[irow]=stp.flohot
                srcname[irow]=string.join(ss[:-2],'_')
            elif ss[-2]=='fshi':
                srctype[irow]=stp.fhihot
                srcname[irow]=string.join(ss[:-2],'_')
            else:
                srctype[irow]=stp.hot
                srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='cold':
            if ss[-2]=='fslo':
                srctype[irow]=stp.flocold
                srcname[irow]=string.join(ss[:-2],'_')
            elif ss[-2]=='fshi':
                srctype[irow]=stp.fhicold
                srcname[irow]=string.join(ss[:-2],'_')
            else:
                srctype[irow]=stp.cold
                srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='calon':
            if ss[-2]=='ps':
                srctype[irow]=stp.poncal
                srcname[irow]=string.join(ss[:-2],'_')
            elif ss[-2]=='psr':
                srctype[irow]=stp.poffcal
                srcname[irow]=string.join(ss[:-2],'_')
            elif ss[-2]=='nod':
                srctype[irow]=stp.nodcal
                srcname[irow]=string.join(ss[:-2],'_')
            elif ss[-2]=='fs':
                srctype[irow]=stp.foncal
                srcname[irow]=string.join(ss[:-2],'_')
            elif ss[-2]=='fsr':
                srctype[irow]=stp.foffcal
                srcname[irow]=string.join(ss[:-2],'_')
            else:
                srctype[irow]=stp.notype
        elif ss[-1]=='fslo':
            srctype[irow]=stp.fslo
            srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='fshi':
            srctype[irow]=stp.fshi
            srcname[irow]=string.join(ss[:-1],'_')
        elif ss[-1]=='off':
            if ss[-2]=='fslo':
                srctype[irow]=stp.flooff
                srcname[irow]=string.join(ss[:-2],'_')
            elif ss[-2]=='fshi':
                srctype[irow]=stp.fhioff
                srcname[irow]=string.join(ss[:-2],'_')
            else:
                srctype[irow]=stp.notype
        else:
            srctype[irow]=stp.notype
    self._tb.putcol('SRCTYPE',srctype)
    self._tb.putcol('SRCNAME',srcname)
    self._tb.flush()
    self._tb.close()

###
#
# is_asdm
#
# Check if given file is ASDM or not.
#
# Inputs:
#
#    filename --- file name
#
###
def is_asdm(filename):
    """Is the given file an ASDM?

    Parameters:

        filename: the name of the file/directory to test

    """
    if ( os.path.isdir(filename)
         and os.path.exists(filename+'/ASDM.xml') ):
        return True
    else:
        return False

###
#
# decorators
#
###
import functools
import time
def dec_stages_interface_logging( func ):
    @functools.wraps( func )
    def wrapper( *args, **kw ):
        # args[0] must be StagesInterface class instance
        obj = args[0]

        funcname = func.__name__
        origin = funcname + '()'
        ProcStartTime = time.time()
        obj.LogMessage( 'PROGRESS', Origin=origin, Msg=funcname+' Start: %s'%time.ctime(ProcStartTime) )

        # execute func
        try:
            message = 'Processing %s ...'%(funcname)
            obj.frontPage.postStatus(message,'blue')
            ### actual processing ###
            retval = func( *args, **kw )
            #########################
            message = message.replace('Processing','Finished')
            obj.frontPage.postStatus(message,'blue')
        except Exception, e:
            import traceback
            obj.setFileOut(obj.GlobalLogDir+'/'+obj.SD_PIPELINE_LOG)
            tbstr=__removeTraceFromWrapper( traceback.format_exc() )
            obj.LogMessage( 'ERROR', Origin=origin, Msg=tbstr )
            obj.status[0] = 'Failed at %s'%(funcname)
            obj.status[1] = 'red'
            #obj.setFileOut(obj.LogDir+'/'+obj.SD_PIPELINE_LOG)
            obj.cleanup()
            raise e
        finally:
            ProcEndTime = time.time()
            elapsedTime=ProcEndTime-ProcStartTime
            if obj.profilePage is not None:
                obj.profilePage.addTime(funcname,elapsedTime)
            obj.LogMessage( 'PROGRESS', Origin=origin, Msg=funcname+' End: %s (ElapsedTime=%.1f sec)'%(time.ctime(ProcEndTime),(elapsedTime)) )
            

        return retval
    return wrapper

def dec_stages_interface_logfile( func ):
    @functools.wraps( func )
    def wrapper( *args, **kw ):
        # args[0] must be StagesInterface class instance
        obj = args[0]

        #set logfile
        #obj.setFileOut( obj.LogDir+'/'+obj.SD_PIPELINE_LOG )
        obj.setFileOut( obj.GlobalLogDir+'/'+obj.SD_PIPELINE_LOG )

        funcname = func.__name__
        origin = funcname + '()'

        #execute func
        retval = func( *args, **kw )

        #set back logfile
        #obj.setFileOut( obj.LogDir+'/'+obj.SD_PIPELINE_LOG )

        return retval
    return wrapper
        
def dec_engines_logging( func ):
    @functools.wraps( func )
    def wrapper( *args, **kw ):
        # args[0] must be SDEngine class instance
        obj = args[0]
        
        funcname = func.__name__
        origin = funcname + '()'
        ProcStartTime = time.time()
        obj.LogMessage( 'TITLE', Origin=origin, Msg='<'*15+'  '+funcname+'  '+'>'*15 )
        obj.LogMessage( 'PROGRESS', Origin=origin, Msg=funcname+' Start: %s'%time.ctime(ProcStartTime) )
        
        # execute func
        try:
            retval = func( *args, **kw )
        except Exception, e:
            if 'LogFile' in kw.keys():
                obj.setFileOut( kw['LogFile'] )
            else:
                obj.setFileOut( False )
            import traceback
            tbstr=__removeTraceFromWrapper( traceback.format_exc() )
            obj.LogMessage( 'ERROR', Origin=origin, Msg=tbstr )
            raise e
        finally:
            ProcEndTime = time.time()
            obj.LogMessage( 'PROGRESS', Origin=origin, Msg=funcname+' End: %s (ElapsedTime=%.1f sec)'%(time.ctime(ProcEndTime),(ProcEndTime-ProcStartTime)) )
            
        return retval
    return wrapper

def dec_engines_logfile( func ):
    @functools.wraps( func )
    def wrapper( *args, **kw ):
        # args[0] must be SDEngine class instance
        obj = args[0]

        # set logfile
        #print kw
        if 'LogFile' in kw.keys():
            obj.setFileOut( kw['LogFile'] )
        else:
            obj.setFileOut( False )

        #execute func
        retval = func( *args, **kw )

        #set back logfile
        # 2012/03/08 TN
        # There is an assumption that all methods are called via 
        # StagesInterface instance
        import inspect
        stack = inspect.stack()
        for i in xrange(len(stack)):
            frame = stack[i][0]
            arginfo = inspect.getargvalues(frame)
            if 'self' in arginfo.locals:
                o = arginfo.locals['self']
                #obj.LogMessage('DEBUG','TEST','hasattr(SD_PIPELINE_LOG)=%s'%(hasattr(o,'SD_PIPELINE_LOG')))
                if hasattr(o,'SD_PIPELINE_LOG'):
                    o.setFileOut(o.GlobalLogDir+'/'+o.SD_PIPELINE_LOG)
                break

        return retval
    return wrapper

def __getExceptionTypeString( e ):
    etype=str(type(e))
    typestr=etype[etype.find('\'')+1:etype.rfind('\'')].split('.')
    if len(typestr) > 1:
        typestr=typestr[1]
    else:
        typestr=typestr[0]
    return typestr

def __removeTraceFromWrapper( s ):
    import re
    import string
    wexists=True
    matchstr='.*SDTool\.py.*in wrapper.*'
    while wexists:
        ss=s.split('\n')
        wexists=False
        for i in xrange(len(ss)):
            if re.match(matchstr,ss[i]) is not None:
                ss.pop(i)
                ss.pop(i)
                wexists=True
                break
        s=string.join(ss,'\n')
    return s

def getHeuristicsVersion():
    import string
    import datetime
    src=['SDCalibration.py',
         'SDDataProductName.py',
         'SDEngine.py',
         'SDFlagRule.py',
         'SDHtmlProduct.py',
         'SDImaging.py',
         'SDjava2html.py',
         'SDpipelineControl.py',
         'SDPipelineStagesInterface.py',
         'SDPlotter.py',
         'SDTool.py']
    hpath=None
    hver='Undefined'
    for p in sys.path:
        #print 'p=%s'%(p)
        subp=['','heuristics','hsd','heuristics/hsd']
        found=False
        for sp in subp:
            path=p+'/'+sp+'/'+src[0]
            if os.path.exists(path):
                hpath=p+'/'+sp
                #print '...found at %s'%(path)
                found=True
                break
            #else:
                #print '...not found'
        if found:
            break
    if hpath is None:
        #print 'hpath is not found'
        return hver
    else:
        #print 'hpath=%s'%(hpath)
        d0=datetime.datetime(1900,1,1)
        for s in src:
            f=open(hpath+'/'+s,'r')
            d=''
            iter=0
            while d.find('# $Date:') == -1 or iter > 100:
                d=f.readline()
            f.close()
            #print 'd=%s'%(d)
            spd=d.split()
            if iter <= 100 and len(spd)>4:
                dd=string.join(spd[2:4])
                #print 'dd=%s'%(dd)
                (y,m,d,H,M,S)=__toymd(dd)
                dt=datetime.datetime(y,m,d,H,M,S)
                if dt > d0:
                    d0=dt
                    #print 'dt=%s'%(dt)
                    hver=dd
        if hver != 'Undefined':
            hver=__reformat(hver)
        return hver
            

def __toymd( s ):
    """
    s: string YYYY/MM/DD HH:MM:SS
    """
    (ymd,hms)=s.split()
    ymd=ymd.split('/')
    hms=hms.split(':')
    y=int(ymd[0])
    m=int(ymd[1])
    d=int(ymd[2])
    H=int(hms[0])
    M=int(hms[1])
    S=int(hms[2])
    return (y,m,d,H,M,S)

def __reformat( s ):
    """
    s: YYYY/MM/DD HH:MM:SS
    """
    import time
    news=time.asctime(time.strptime(s,'%Y/%m/%d %H:%M:%S'))
    news+=' UTC (latest commit date)'
    return news


####
#
# DataSummaryBase
# 
# Return data property as Python dictionary
#
# This is base class which is independent of data format.
# Result returned by the instance of this base class might be odd.
# Inherited classes that are proper for data format must be used.
#
# Inheritances:
#
#    ASDMDataSummary      --- works on ASDM
#    MSDataSummary        --- works on MS
#    ScantableDataSummary --- works on Scantable 
#
#
# Data Property Structure:
#
#     { 'name': '<data file name>',
#       'type': 'ASDM'|'MS2'|'ASAP',
#       'pol': { ID: { 'type': 'stokes'|'linear'|'circular'|'linpol'
#                      'str': [<list of string representation of correlation types>]
#                       'corr': [<list of MS-type correlation type enumeration>]
#                       'polno': [<list of ASAP-type polarization index>]
#                       'spwid': [<list of associated spectral window>] }
#                ... },
#       'spw': { ID: { 'type': 'SP'|'TP'|'WVR'
#                      'intent': 'TARGET'|'CALIB'|...|'NONE' (intent of spw)
#                      'polid': [<list of associated polarization setup>]
#                      'frame': 'TOPO'|'LSRK'|... (frequency reference frame) 
#                      'freq': [<freq_min>,<freq_max>]
#                      'bw': <bandwidth>
#                      'nchan': <nchan> },
#                ... },
#       'filltsys': True|False (whether need to run filltsys or not),
#       'spwtsys': [<list of spws for Tsys calibration>],
#       'spwspec': [<list of target spws for filltsys script>],
#       'observer': '<observer>',
#       'date': '<date string (start - end)>',
#       'source': { 'name': '<source name>',
#                   'dir': [<source direction>] },
#       'antenna': { 'name': '<antenna name>' } } 
###
class DataSummaryBase:
    def __init__( self, filename ):
        self.name = filename
        self.format = 'NONE'
        self.summary = {}
        self.pols = []
        self.poltypeDic = {}
        self.polidDic = {}
        self.polnoDic = {}
        self.polstrDic = {}
        self.sassocDic = {}
        self.spws = []
        self.nchans = {}
        self.spwtypeDic = {}
        self.intentDic = {}
        self.passocDic = {}
        self.sourceDic = {}
        self.frameDic = {}
        self.freqDic = {}
        self.bwDic = {}
        self.bbnoDic = {}
        self.antennaDic = {}
        self.who = 'ANYBODY'
        self.when = [0.0,0.0]

    def getSummary( self ):
        return self.summary

    def fillSummary( self ):
        self.summary['name'] = self.name
        self.summary['type'] = self.format
        self.summary['pol'] = self.__pol()
        self.summary['spw'] = self.__spw()
        (tspw,cspw,notsys) = self.__compileAssoc()
        self.summary['filltsys'] = notsys #self.__needFillTsys()
        self.summary['spwtsys'] = cspw #self.__spwTsys()
        self.summary['spwspec'] = tspw #self.__spwSpec()
        self.summary['observer'] = self.who
        self.summary['date'] = self.when
        self.summary['source'] = self.sourceDic
        self.summary['antenna'] = self.antennaDic

    def __pol( self ):
        ret = {}
        for pol in self.pols:
            d={}
            d['type'] = self.__test(self.poltypeDic,pol)
            d['str'] = self.__test(self.polstrDic,pol,[])
            d['corr'] = self.__test(self.polidDic,pol,[])
            d['polno'] = self.__test(self.polnoDic,pol,[])
            d['spwid'] = self.__test(self.sassocDic,pol,[])
            ret[pol] = d
        return ret
        
    def __spw( self ):
        ret = {}
        for spw in self.spws:
            d = {}
            d['type'] = self.__test(self.spwtypeDic,spw)
            d['intent'] = self.__test(self.intentDic,spw)
            d['polid'] = self.__test(self.passocDic,spw,[])
            d['frame'] = self.__test(self.frameDic,spw)
            d['freq'] = self.__test(self.freqDic,spw,0.0)
            d['bw'] = self.__test(self.bwDic,spw,0.0)
            d['nchan'] = self.__test(self.nchans,spw,0)
            ret[spw]=d
        return ret

    def __test( self, dic, key, default='NONE' ):
        if dic.has_key(key):
            return dic[key]
        else:
            return default

    def __compileAssoc( self ):
        target = self.__spwSpec()
        calib = self.__spwTsys()
        spwspec = []
        spwtsys = []
        for tspw in target:
            # skip if Tsys is already interpolated
            tintent = self.intentDic[tspw]
            if tintent.find('TSYS') != -1:
                continue

            # check association
            tfreq = self.freqDic[tspw]
            tbbno = self.bbnoDic[tspw]
            for cspw in calib:
                cfreq = self.freqDic[cspw]
                cbbno = self.bbnoDic[cspw]
                # condition 1: calibration spw must cover whole freq.
                #              range of target spw
                # tfreq  |------|
                # cfreq |----------|
                #
                # condition 2: baseband number must be same
                if tfreq[0] >= cfreq[0] and tfreq[1] <= cfreq[1] and tbbno == cbbno:
                    spwspec.append(tspw)
                    spwtsys.append(cspw)

        if len(spwspec) > 0:
            noTsys = True
        else:
            noTsys = False

        return (spwspec,spwtsys,noTsys)

    def __needFillTsys( self ):
        v = self.intentDic.values()
        if 'TARGET:TSYS' in v:
            return False
        elif 'CALIB:TSYS' in v:
            return True
        else:
            return False

    def __spwTsys( self ):
        ret = []
        for spw in self.spws:
            if self.intentDic[spw] == 'CALIB:TSYS':
                ret.append( spw )
        return ret 
        
    def __spwSpec( self ):
        ret = []
        for spw in self.spws:
            if self.intentDic[spw].find('TARGET') is 0 and self.spwtypeDic[spw] == 'SP':
                ret.append(spw)
        return ret

    def __spwtype( self ):
        for i in xrange(len(self.spws)):
            spw = self.spws[i]
            if self.nchans[i] > 1:
                if self.nchans[i] == 4:
                    self.spwtypeDic[spw] = 'WVR'
                else:
                    self.spwtypeDic[spw] = 'SP'
            elif self.nchans[i] == 1:
                self.spwtypeDic[spw] = 'TP'
            else:
                self.spwtypeDic[spw] = 'NONE'

    def __processSource( self, name, dir ):
        nsrc = len(name)
        self.sourceDic.clear()
        for i in xrange(nsrc):
            d={}
            d['name'] = name[i]
            d['dir'] = dir[i]
            self.sourceDic[i] = d

    def __processAntenna( self, id, name ):
        self.antennaDic.clear()
        for i in id:
            d={}
            d['name'] = str(name[i])
            self.antennaDic[i] = d

###
#
# ASDMDataSummary
#
# Return data property as Python dictionary.
# Works on ASDM.
#
###
class ASDMDataSummary( DataSummaryBase ):
    def __init__( self, filename ):
        DataSummaryBase.__init__( self, filename )
        self.format = 'ASDM'
        self.mfrMap = { 0: 'LABREST',
                        1: 'LSRD',
                        2: 'LSRK',
                        3: 'BARY',
                        4: 'REST',
                        5: 'GEO',
                        6: 'GALACTO',
                        7: 'TOPO' }
        self.polMap = { 'Undefined': ['Undefined', 0, 0],
                        'I': ['stokes', 1, 0],
                        'Q': ['stokes', 2, 1],
                        'U': ['stokes', 3, 2],
                        'V': ['stokes', 4, 3],
                        'RR': ['circular', 5, 0],
                        'RL': ['circular', 6, 2],
                        'LR': ['circular', 7, 3],
                        'LL': ['circular', 8, 1],
                        'XX': ['linear',  9, 0],
                        'XY': ['linear', 10, 2],
                        'YX': ['linear', 11, 3],
                        'YY': ['linear', 12, 1],
                        'Ptotal': ['linpol', 28, 0],
                        'Plinear': ['linpol', 29, 1],
                        'PFtotal': ['linpol', 30, 2],
                        'PFlinear': ['linpol', 31, 3],
                        'Pangle': ['linpol', 32, 4] }

    def fillSummary( self ):
        self.__process()
        DataSummaryBase.fillSummary( self )
        
    def __processExecBlock( self ):
        # process only first row so far
        (tab,rows,nrow) = self.__tableandrow('ExecBlock')
        self.who = str(self.__getdata(rows[0],'observerName')).strip()
        startTime = self.__getdata(rows[0],'startTime')
        endTime = self.__getdata(rows[0],'endTime')
        self.when[0] = self.__fromMJD( startTime )
        self.when[1] = self.__fromMJD( endTime )

    def __process( self ):

        # initialization of spwids and nchans
        self.__processSpw()

        # Polarization table
        self.__processPolarization()

        # process ExecBlock table
        self.__processExecBlock()

        # process Antenna table
        self.__processAntenna()
        
        # process Source table
        self.__processSource()

        # Scan table
        (tab,rows,nrow) = self.__tableandrow('Scan')
        mdic = {}
        for i in xrange(nrow):
            scanNumber = self.__getdata(rows[i],'scanNumber')
            scan = int(scanNumber)
            scanIntent = self.__getdata(rows[i],'scanIntent')
            firstIntent = scanIntent.split()[2]
            mdic[scan] = firstIntent

        # DataDescription table
        ddrows = self.__processDataDesc()

        # SysCal table
        spwTsys = []
        if os.path.exists(self.name+'/SysCal.xml'):
            (tab,rows,nrow) = self.__tableandrow('SysCal')
            for i in xrange(nrow):
                spwtag = self.__getdata(rows[i],'spectralWindowId')
                spwid = self.__tagvalue(spwtag,'SpectralWindow')
                if spwid not in spwTsys:
                    spwTsys.append( spwid )

        # loop on spw
        for spw in self.spws:
            ddlist = []
            for ddid in ddrows.keys():
                ddrow = ddrows[ddid]
                if ddrow[0] == spw:
                    ddlist.append(ddid)
            if len(ddlist) == 0:
                self.intentDic[spw] = 'NONE'
            else:

                # ConfigDescription table
                (tab,rows,nrow) = self.__tableandrow('ConfigDescription')
                cdlist = []
                for i in xrange(nrow):
                    ddtag = self.__getdata(rows[i],'dataDescriptionId')
                    ddtags = self.__tolist(ddtag)
                    ddids = [-1 for j in xrange(len(ddtags))]
                    for j in xrange(len(ddtags)):
                        ddids[j] = int( self.__tagvalue(ddtags[j],'DataDescription') )
                    for j in xrange(len(ddlist)):
                        if ddlist[j] in ddids:
                            cdtag = self.__getdata(rows[i],'configDescriptionId')
                            cdlist.append( self.__tagvalue(cdtag,'ConfigDescription') )
                            break

                # if no corresponding ConfigDescriptionId set 'NONE'
                if len(cdlist) == 0:
                    self.intentDic[spw] = 'NONE'
        
                # now process Main table
                (tab,rows,nrow) = self.__tableandrow('Main')
                scans = []
                for i in xrange(nrow):
                    cdtag = self.__getdata(rows[i],'configDescriptionId')
                    cdid = self.__tagvalue(cdtag,'ConfigDescription')
                    if cdid in cdlist:
                        scan = int( self.__getdata(rows[i],'scanNumber') )
                        if scan not in scans:
                            scans.append(scan)
                iscal = numpy.zeros( len(scans), dtype=bool )
                for i in xrange(len(scans)):
                    iscal[i] = mdic[scans[i]].find('CALIBRATE_') != -1
                if all(iscal):
                    self.intentDic[spw] = 'CALIB'
                else:
                    self.intentDic[spw] = 'TARGET'
                if spw in spwTsys:
                    self.intentDic[spw] += ':TSYS'

    def __processAntenna( self ):
        (tab,rows,nrow) = self.__tableandrow('Antenna')
        antId = [-1 for i in xrange(nrow)]
        antName = ['' for i in xrange(nrow)]
        for i in xrange(nrow):
            antTag = self.__getdata(rows[i],'antennaId')
            antId[i] = self.__tagvalue(antTag,'Antenna')
            antName[i] = str(self.__getdata(rows[i],'name'))
        self._DataSummaryBase__processAntenna( antId, antName )

    def __processDataDesc( self ):
        (tab,rows,nrow) = self.__tableandrow('DataDescription')
        ddrows = {}
        for i in xrange(nrow):
            spwtag = self.__getdata(rows[i],'spectralWindowId')
            spwid = self.__tagvalue(spwtag,'SpectralWindow')
            poltag = self.__getdata(rows[i],'polOrHoloId')
            polid = self.__tagvalue(poltag,'Polarization')
            ddtag = self.__getdata(rows[i],'dataDescriptionId')
            ddid = self.__tagvalue(ddtag,'DataDescription')
            ddrows[ddid] = [spwid,polid]
        for spw in self.spws:
            self.passocDic[spw]=[]
        for pol in self.pols:
            self.sassocDic[pol]=[]
        for row in ddrows.values():
            self.passocDic[row[0]].append(row[1])
            self.sassocDic[row[1]].append(row[0])
        return ddrows

    def __processPolarization( self ):
        (tab,rows,nrow) = self.__tableandrow('Polarization')
        for i in xrange(nrow):
            poltag = self.__getdata(rows[i],'polarizationId')
            polid = int(self.__tagvalue(poltag,'Polarization'))
            self.pols.append(polid)
            corrTypeStr = self.__getdata(rows[i],'corrType')
            polStr = self.__tolist(corrTypeStr)
            self.polstrDic[polid] = polStr
            self.poltypeDic[polid] = self.polMap[polStr[0]][0]
            self.polidDic[polid] = [self.polMap[j][1] for j in polStr]
            self.polnoDic[polid] = [self.polMap[j][2] for j in polStr]

    def __processSource( self ):
        (tab,rows,nrow) = self.__tableandrow('Source')
        names = []
        dirs = []
        for i in xrange(nrow):
            name = str( self.__getdata(rows[i],'sourceName') )
            if name not in names:
                names.append( name )
                sdir = self.__getdata(rows[i],'direction')
                ldir = self.__tolist( sdir )
                dir = numpy.zeros( 2, float )
                for i in xrange(2):
                    dir[i] = float(ldir[i]) * 180.0/numpy.pi
                dirs.append(dir)
        self._DataSummaryBase__processSource( names, dirs )
                
    def __processSpw( self ):
        (tab,rows,nrow) = self.__tableandrow('SpectralWindow')
        self.spws = numpy.zeros(nrow,dtype=int) 
        for i in xrange(nrow):
            spwtag = self.__getdata(rows[i],'spectralWindowId')
            spw = int( spwtag.strip().strip('SpectralWindow_') )
            self.spws[i] = spw
            numChan = self.__getdata(rows[i],'numChan')
            self.nchans[spw] = int(numChan)
            bw = self.__getdata(rows[i],'totBandwidth')
            self.bwDic[spw] = float(bw)
            edge = [0.0,0.0]
            chFreqStartStr = self.__getdata(rows[i],'chanFreqStart')
            if chFreqStartStr is False:
                strRep = self.__getdata(rows[i],'chanFreqArray')
                chFreqArray = self.__tolist(strRep)
                freqStart = float(chFreqArray[0])
                freqEnd = float(chFreqArray[-1])
                strRep = self.__getdata(rows[i],'chanWidthArray')
                chWidthArray = self.__tolist(strRep)
                widthStart = float(chWidthArray[0])
                widthEnd = float(chWidthArray[-1])
                netsb = self.__getdata(rows[i],'netSideband')
                if netsb == 'LSB':
                    widthStart *= -1.0
                    widthEnd *= -1.0
                    edge[0] = freqEnd + 0.5 * widthEnd
                    edge[1] = freqStart - 0.5 * widthStart
                else:
                    edge[0] = freqStart - 0.5 * widthStart
                    edge[1] = freqEnd + 0.5 * widthEnd
            else:
                chFreqStepStr = self.__getdata(rows[i],'chanFreqStep')
                chFreqStart = float(chFreqStartStr)
                chFreqStep = float(chFreqStepStr)
                chFreqEnd = chFreqStart + (self.nchans[spw]-1) * chFreqStep
                chWidthStr = self.__getdata(rows[i],'chanWidth')
                chWidth = float(chWidthStr)
                if chFreqStart > chFreqEnd:
                    edge[0] = chFreqEnd - 0.5 * chWidth
                    edge[1] = chFreqStart + 0.5 * chWidth
                else:
                    edge[0] = chFreqStart - 0.5 * chWidth
                    edge[1] = chFreqEnd + 0.5 * chWidth
            self.freqDic[spw] = edge
            mfr = self.__getdata(rows[i],'measFreqRef')
            if mfr is False:
                mfr = 7
            self.frameDic[spw] = self.mfrMap[mfr]
            bbname = self.__getdata(rows[i],'basebandName')
            if bbname == 'NOBB':
                bbno = -1
            else:
                bbno = int( bbname.split('_')[-1] )
            self.bbnoDic[spw] = bbno
        self._DataSummaryBase__spwtype()

    def __tableandrow( self, tablename ):
        tab = self.__gettable(tablename)
        rows = tab.getElementsByTagName('row')
        nrow = len(rows)
        return (tab,rows,nrow)

    def __gettable( self, tablename ):
        import xml.dom.minidom as DOM
        dom = DOM.parse(self.name+'/'+tablename+'.xml')
        tab = dom.getElementsByTagName(tablename+'Table')[0]
        return tab

    def __getdata( self, node, tagname ):
        elems = node.getElementsByTagName(tagname)
        if len(elems) > 0:
            elem = elems[0]
            return elem.lastChild.data
        else:
            return False

    def __tolist( self, data ):
        l = data.split()
        ndim = int(l[0])
        nelem = []
        idx = 1
        for i in xrange(1,ndim+1):
            nelem.append(int(l[i]))
            idx += 1
        if ndim == 1:
            ret = ['' for i in xrange(nelem[0])]
            for i in xrange(nelem[0]):
                ret[i] = str(l[idx])
                idx += 1
            return ret

    def __tagvalue( self, tag, tagtype ):
        return int( tag.strip().strip(tagtype+'_') )

    def __fromMJD( self, t ):
        """
        MJD [nanosec] string to date string
        """
        dsec = float(t) * 1.0e-9
        return fromMJD( dsec ) 

###
#
# MSDataSummary
#
# Return data property as Python dictionary.
# Works on MS.
#
###
class MSDataSummary( DataSummaryBase ):
    #tbtool = casac.homefinder.find_home_by_name('tableHome')
    def __init__( self, filename ):
        DataSummaryBase.__init__( self, filename )
        self.format = 'MS2'
        self.mfrMap = { 0: 'REST',
                        1: 'LSRK',
                        2: 'LSRD',
                        3: 'BARY',
                        4: 'GEO',
                        5: 'TOPO',
                        6: 'GALACTO',
                        7: 'LGROUP',
                        8: 'CMB' }
        self.polMap = {  0: ['Undefined', 'Undefined', 0],
                         1: ['stokes', 'I', 0],
                         2: ['stokes', 'Q', 1],
                         3: ['stokes', 'U', 2],
                         4: ['stokes', 'V', 3],
                         5: ['circular', 'RR', 0],
                         6: ['circular', 'RL', 2],
                         7: ['circular', 'LR', 3],
                         8: ['circular', 'LL', 1],
                         9: ['linear', 'XX', 0],
                        10: ['linear', 'XY', 2],
                        11: ['linear', 'YX', 3],
                        12: ['linear', 'YY', 1],
                        28: ['linpol', 'Ptotal', 0],
                        29: ['linpol', 'Plinear', 1],
                        30: ['linpol', 'PFtotal', 2],
                        31: ['linpol', 'PFlinear', 3],
                        32: ['linpol', 'Pangle', 4] }
        self.__process()

    def fillSummary( self ):
        self.__process()
        DataSummaryBase.fillSummary( self )

    def __processObservation( self, tab ):
        # process only first row so far
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]
        tb.open(tab)
        self.who = tb.getcell('OBSERVER',0)
        timerange = tb.getcell('TIME_RANGE',0)
        self.when[0] = fromMJD( timerange[0] )
        self.when[1] = fromMJD( timerange[1] )
        tb.close()
        
    def __process( self ):
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]
        
        # access to subtables
        tb.open(self.name)
        #ddname = tb.getkeyword('DATA_DESCRIPTION').split()[-1]
        #statename = tb.getkeyword('STATE').split()[-1]
        #polname = tb.getkeyword('POLARIZATION').split()[-1]
        #scname = tb.getkeyword('SYSCAL').split()[-1]
        #srcname = tb.getkeyword('SOURCE').split()[-1]
        #spwname = tb.getkeyword('SPECTRAL_WINDOW').split()[-1]
        #obsname = tb.getkeyword('OBSERVATION').split()[-1]
        #antname = tb.getkeyword('ANTENNA').split()[-1]
        ddname = tb.getkeyword('DATA_DESCRIPTION').lstrip('Table: ')
        statename = tb.getkeyword('STATE').lstrip('Table: ')
        polname = tb.getkeyword('POLARIZATION').lstrip('Table: ')
        scname = tb.getkeyword('SYSCAL').lstrip('Table: ')
        srcname = tb.getkeyword('SOURCE').lstrip('Table: ')
        spwname = tb.getkeyword('SPECTRAL_WINDOW').lstrip('Table: ')
        obsname = tb.getkeyword('OBSERVATION').lstrip('Table: ')
        antname = tb.getkeyword('ANTENNA').lstrip('Table: ')
        tb.close()

        # process OBSERVATION table
        self.__processObservation( obsname )

        # process ANTENNA table
        self.__processAntenna( antname )

        # process SPECTRAL_WINDOW table
        self.__processSpw( spwname )

        # process SOURCE table
        self.__processSource( srcname )

        # POLARIZATION subtable
        self.__processPolarization( polname )

        # STATE subtable
        tb.open(statename)
        obsmodes = tb.getcol('OBS_MODE')
        tb.close()

        # SYSCAL subtable
        tb.open(scname)
        spwTsys = numpy.unique( tb.getcol('SPECTRAL_WINDOW_ID') )
        tb.close()

        # DATA_DESCRIPTION subtable
        ddrows = self.__processDataDesc( ddname )

        # loop on spws
        for spw in self.spws:
            ddlist = []
            for ddid in xrange(len(ddrows)):
                ddrow = ddrows[ddid]
                if ddrow[0] == spw:
                    ddlist.append(ddid)
            if len(ddlist) == 0:
                self.intentDic[spw] = 'NONE'
            else:
                # now process MAIN table
                tb.open(self.name)
                tsel = tb.query( 'DATA_DESC_ID IN %s'%(ddlist) )
                stateids = numpy.unique( tsel.getcol('STATE_ID') )
                tsel.close()
                tb.close()
                iscal = numpy.zeros( len(stateids), dtype=bool )
                for i in xrange(len(stateids)):
                    iscal[i] = obsmodes[stateids[i]].split(',')[0].upper().find('CAL') != -1
                if all(iscal):
                    self.intentDic[spw] = 'CALIB'
                else:
                    self.intentDic[spw] = 'TARGET'
                if spw in spwTsys:
                    self.intentDic[spw] += ':TSYS'

    def __processAntenna( self, tab ):
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]
        tb.open(tab)
        namecol = tb.getcol('NAME')
        nrow = tb.nrows()
        tb.close()
        antId = range(nrow)
        antName = [namecol[i] for i in xrange(nrow)]
        self._DataSummaryBase__processAntenna( antId, antName )
            
    def __processPolarization( self, tab ):
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]
        tb.open(tab)
        self.pols = range(tb.nrows())
        for i in xrange(tb.nrows()):
            corrType = tb.getcell('CORR_TYPE',i)
            self.poltypeDic[i] = self.polMap[corrType[0]][0]
            self.polidDic[i] = corrType
            self.polstrDic[i] = [self.polMap[j][1] for j in corrType]
            self.polnoDic[i] = [self.polMap[j][2] for j in corrType]
        tb.close()
                    
    def __processDataDesc( self, tab ):
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]
        tb.open(tab)
        spwids = tb.getcol('SPECTRAL_WINDOW_ID')
        polids = tb.getcol('POLARIZATION_ID')
        ddrows = []
        for ddid in xrange(tb.nrows()):
            ddrows.append([spwids[ddid],polids[ddid]])
        tb.close()
        for spw in self.spws:
            self.passocDic[spw]=[]
        for pol in self.pols:
            self.sassocDic[pol]=[]
        for row in ddrows:
            self.passocDic[row[0]].append(row[1])
            self.sassocDic[row[1]].append(row[0])
        return ddrows 

    def __processSpw( self, tab ):
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]
        tb.open(tab)
        self.spws = range( tb.nrows() )
        nchan = tb.getcol('NUM_CHAN')
        bw = tb.getcol('TOTAL_BANDWIDTH')
        isbbexists = 'BBC_NO' in tb.colnames()
        if isbbexists:
            bbno = tb.getcol('BBC_NO')
        else:
            bbno = numpy.zeros( tb.nrows(), dtype=int )
        for i in xrange(len(self.spws)):
            spw = self.spws[i]
            self.nchans[spw] = nchan[i]
            self.bwDic[spw] = bw[i]
            chfreq = tb.getcell('CHAN_FREQ',i)
            chwidth = tb.getcell('CHAN_WIDTH',i)
            if nchan[i] == 1:
                edge = [chfreq[0]-0.5*chwidth[0],
                        chfreq[0]+0.5*chwidth[0]]
            else:
                if chfreq[0] > chfreq[1]:
                    chwidth *= -1.0
                edge = [chfreq[0]-0.5*chwidth[0],
                        chfreq[-1]+0.5*chwidth[-1]]
                if edge[0] > edge[1]:
                    tmp = edge[0]
                    edge[0] = edge[1]
                    edge[1] = tmp
            self.freqDic[spw] = edge
            mfr = tb.getcell('MEAS_FREQ_REF',0)
            self.frameDic[spw] = self.mfrMap[mfr]
            self.bbnoDic[spw] = bbno[i]
        tb.close()
        self._DataSummaryBase__spwtype()


    def __processSource( self, name ):
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]
        tb.open(name)
        srccol = tb.getcol('NAME')
        nrow = tb.nrows()
        srcList = numpy.unique(srccol)
        numList = len(srcList)
        dir = [[] for i in xrange(numList)]
        for i in xrange(numList):
            idx = 0
            while ( idx < nrow and srccol[idx] != srcList[i] ):
                idx += 1
            dir[i] = tb.getcell('DIRECTION',idx) * 180.0/numpy.pi
        tb.close()
        self._DataSummaryBase__processSource( srcList, dir )

###
#
# ScantableDataSummary
#
# Return data property as Python dictionary.
# Works on Scantable.
#
###
class ScantableDataSummary( DataSummaryBase ):
    #tbtool = casac.homefinder.find_home_by_name('tableHome')
    def __init__( self, filename ):
        DataSummaryBase.__init__( self, filename )
        self.format = 'ASAP'
        self.polMap = { 'linear': { 0: ['XX',  9],
                                    1: ['YY', 12],
                                    2: ['XY', 10],
                                    3: ['YX', 11] },
                        'circular': { 0: ['RR', 5],
                                      1: ['LL', 8],
                                      2: ['RL', 6],
                                      3: ['LR', 7] },
                        'stokes': { 0: ['I', 1],
                                    1: ['Q', 2],
                                    2: ['U', 3],
                                    3: ['V', 4] },
                        'linpol': { 0: ['Ptotal',   28],
                                    1: ['Plinear',  29],
                                    2: ['PFtotal',  30],
                                    3: ['PFlinear', 31],
                                    4: ['Pangle',   32] } }
        
    def fillSummary( self ):
        self.__process()
        DataSummaryBase.fillSummary( self )

    def __process( self ):
        self.__spwchans()
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]

        tb.open(self.name)
        self.who = tb.getkeyword('Observer')
        timecol = tb.getcol('TIME')
        self.when[0] = fromMJD( timecol.min(), 'day' )
        self.when[1] = fromMJD( timecol.max(), 'day' )
        self.__processSource( tb )
        self.__processSpw( tb )
        self.__processPolarization( tb )
        self.__processAntenna( tb )
        tb.close()

    def __processAntenna( self, tb ):
        antname = tb.getkeyword('AntennaName')
        if antname.find('//') != -1:
            antname = antname.split('//')[-1]
        if antname.find('@') != -1:
            antname = antname.split('@')[0]
        antId = [0]
        antName = [antname]
        self._DataSummaryBase__processAntenna( antId, antName )

    def __processPolarization( self, tb ):
        poltype = tb.getkeyword('POLTYPE')
        num = 0
        for spw in self.spws:
            self.passocDic[spw] = []
            tsel = tb.query('IFNO==%s'%(spw))
            pols = numpy.unique(tsel.getcol('POLNO'))
            idx = -1
            for key in self.polnoDic.keys():
                ref = self.polnoDic[key]
                if len(ref) == len(pols) and all(ref==pols):
                    idx = key
                    break
            if idx == -1:
                self.pols.append(num)
                self.polnoDic[num] = pols
                self.poltypeDic[num] = poltype
                self.polstrDic[num] = [self.polMap[poltype][i][0] for i in pols]
                self.polidDic[num] = [self.polMap[poltype][i][1] for i in pols]
                self.sassocDic[num] = [spw]
                self.passocDic[spw].append(num)
                num += 1
            else:
                if idx not in self.passocDic[spw]:
                    self.passocDic[spw].append(idx)
                if spw not in self.sassocDic[idx]:
                    self.sassocDic[idx].append(spw)
            
    def __processSource( self, tb ):
        srccol = tb.getcol('SRCNAME')
        nrow = tb.nrows()
        srcList = numpy.unique(srccol)
        numList = len(srcList)
        dir = [[] for i in xrange(numList)]
        for i in xrange(numList):
            idx = 0
            while ( idx < nrow and srccol[idx] != srcList[i] ):
                idx += 1
            dir[i] = tb.getcell('SRCDIRECTION',idx) * 180.0/numpy.pi
        self._DataSummaryBase__processSource( srcList, dir )

    def __processSpw( self, tb ):
        from asap import srctype as st
        self.intentDic.clear()
        #ftab = tb.getkeyword('FREQUENCIES').split()[-1]
        ftab = tb.getkeyword('FREQUENCIES').lstrip('Table: ')
        for spw in self.spws:
            tsel = tb.query( 'IFNO==%s'%(spw) )
            tsyschan = len( tsel.getcell('TSYS',0) )

            # spectral spec
            freqid = tsel.getcell('FREQ_ID',0)
            self.__processFrequencies( ftab, spw, freqid )

            # scan intent
            srctypes = numpy.unique( tsel.getcol('SRCTYPE') )
            tsel.close()
            if ( (st.pson in srctypes)
                 or (st.fson in srctypes)
                 or (st.fslo in srctypes)
                 or (st.nod in srctypes) ):
                self.intentDic[spw] = 'TARGET'
            else:
                self.intentDic[spw] = 'CALIB'
            if self.spwtypeDic[spw] == 'SP' and self.nchans[spw] == tsyschan:
                # if nchan (!=1) for TSYS is same as SPECTRA,
                # it might be TSYS calibratoin
                self.intentDic[spw] += ':TSYS'

            # bbno is always 0
            self.bbnoDic[spw] = 0

    def __processFrequencies( self, tab, spw, freqid ):
        #tb = self.tbtool.create()
        tb = gentools(['tb'])[0]
        tb.open( tab )
        self.frameDic[spw] = tb.getkeyword('FRAME')
        tsel = tb.query('ID==%s'%(freqid))
        if tsel.nrows() != 0:
            refpix = tsel.getcell('REFPIX',0)
            refval = tsel.getcell('REFVAL',0)
            incr = tsel.getcell('INCREMENT',0)
            if self.nchans[spw] == 1:
                self.bwDic[spw] = abs(incr)
                edge=[0.0,0.0]
                edge[0] = refval-0.5*incr
                edge[1] = refval+0.5*incr
                if edge[0] > edge[1]:
                    tmp = edge[0]
                    edge[0] = edge[1]
                    edge[1] = tmp
                self.freqDic[spw] = edge
            else:
                edge = [0.0,0.0]
                edge[0] = refval - (refpix+0.5) * incr
                edge[1] = refval + (self.nchans[spw]-refpix-0.5) * incr
                if edge[0] > edge[1]:
                    tmp = edge[0]
                    edge[0] = edge[1]
                    edge[1] = tmp
                self.freqDic[spw] = edge
                self.bwDic[spw] = edge[1] - edge[0]
        tsel.close()
        tb.close()
        

    def __spwchans( self ):
        from asap import scantable
        s = scantable(self.name,False)
        self.spws = s.getifnos()
        for spw in self.spws:
            self.nchans[spw] = s.nchan(spw)
        del s
        self._DataSummaryBase__spwtype()

def fromMJD( t, unit='sec' ):
    """
    MJD ---> date string

    t: MJD
    unit: sec or day
    """
    if unit == 'sec':
        mjd = t
    elif unit == 'day':
        mjd = t * 86400.0
    else:
        mjd = 0.0
    import time
    import datetime
    mjdzero=datetime.datetime(1858,11,17,0,0,0)
    zt=time.gmtime(0.0)
    timezero=datetime.datetime(zt.tm_year,zt.tm_mon,zt.tm_mon,zt.tm_mday,zt.tm_hour,zt.tm_min,zt.tm_sec)
    dtd = timezero-mjdzero
    dtsec=mjd-(float(dtd.days)*86400.0+float(dtd.seconds)+float(dtd.microseconds)*1.0e-6)
    mjdstr=time.asctime(time.gmtime(dtsec))+' UTC'
    return mjdstr 

###
#
# getSummary( filename )
#
# Return data property as Python dictionary.
# This is high level function that identifies data format of
# input file, creates appropriate instance depending on
# given data format, and finally return data property.
#
# Inputs:
#
#    filename --- data file name
#
###
def getSummary( filename ):
    obj = None
    if not os.path.exists( filename ):
        print '%s: file doesn\'t exist'%(filename)
        return obj
    elif is_asdm( filename ):
        obj = ASDMDataSummary( filename )
    elif is_ms( filename ):
        obj = MSDataSummary( filename )
    elif is_scantable( filename ):
        obj = ScantableDataSummary( filename )
    else:
        print '%s: Unsupported data format'%(filename)
        return obj
    obj.fillSummary()
    return obj.getSummary()

###
# Tool to obtain beam size from antenna name/diameter and
# observing frequency.
#
###
def beamsize(a, f):
    """
    calculate beam size in arcsec.
    returned value is rounded, like 32.0 for 32.199992 or
    9.8 for 9.783333331.
    
    a: antenna name (string) or diameter in metre (float)
    f: observing frequency in GHz
    """
    accuratesize = accuratebeamsize( a, f )
    #print 'accurate size=',accuratesize
    return rounding( accuratesize )
    
def accuratebeamsize(a, f):
    """
    calculate beam size in arcsec.

    a: antenna name (string) or diameter in metre (float)
    f: observing frequency in GHz
    """
    factor = 1.2
    if type(a) == str:
        # see lookup table to get antenna diameter from its name
        d = antennadiameter( a )
        accurateSize = accuratebeamsize( d, f )
    else:
        # beam size = lambda / D
        #           = (c * pi / 180.0 * 3600.0) / (nu * D) [arcsec]
        #           = 6.188e4 / {(nu [GHz]) * (D [m])} [arcsec] 
        #accurateSize = 6.188e4 / f / a
        # to multiply factor 1.2
        accurateSize = factor * 6.188e4 / f / a
    return accurateSize

def rounding(v):
    """
    round values

    v: float or int
    """
    s = '%e'%(v)
    p = int(s.split('e')[1])
    dstr = digitstr(p)
    #print dstr
    #ret = Decimal(s).quantize(Decimal(dstr),rounding=ROUND_HALF_UP)
    ret = Decimal(s).quantize(Decimal(dstr),rounding=ROUND_UP)
    return float(ret)

def digitstr(i):
    if i > 0:
        return '1.'
    else:
        import string
        ret = '.'+string.join(['0' for j in xrange(abs(i))],'')+'1'
        return ret
              
def antennadiameter(name):
    """
    get antenna diameter in metre from its name

    name: antenna name
    """
    lookuptable = { 'DV[0-5][0-9]': 12.0,
                    'DA[0-5][0-9]': 12.0,
                    'PM0[1-4]': 12.0,
                    'CM[0-1][0-9]': 7.0,
                    'APEX': 12.0,
                    'AP-': 12.0,
                    'NRO': 45.0,
                    'ASTE': 10.0,
                    'MRT': 30.0,
                    'IRAM30m': 30.0,
                    'Effelsberg': 100.0,
                    'GBT': 104.9,
                    'SMT': 10.0,
                    'HHT': 10.0,
                    # from asap/src/STAttr.cpp
                    'MOPRA': 22.0,
                    'PKS': 64.0,
                    'TIDBINBILLA': 70.0,
                    'CEDUNA': 30.0,
                    'HOBART': 26.0 }
    import re
    d=0.0
    for (key,item) in lookuptable.items():
        if re.match( key, name ) is not None:
            #print 'matched %s'%(key)
            d = item
            break
    if d == 0.0:
        raise Exception('No data in lookup table: %s'%(name))
    return d
    

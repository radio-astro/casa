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
# $Revision: 1.51.2.3.2.22 $
# $Date: 2012/12/14 09:23:17 $
# $Author: tnakazat $
#
from SDTool import SDLogger, is_scantable, dec_engines_logging, dec_engines_logfile, ProgressTimer
import SDDataTable as DT
import asap as sd
import numpy as NP
import numpy.fft as FFTP
import numpy.linalg as LA
import time
import math
import SDPlotter as SDP
import pylab as PL
import matplotlib.pyplot as PLT
#import SDTool as SDT
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import FuncFormatter, MultipleLocator, AutoLocator
#from heuristics.shared.hlinefinder import hlinefinder
#import casac
from casac import casac
import sys
import os
#import heuristics.hif.linefinder2 as hlinefinder
from taskinit import gentools

Rad2Deg = 180. / 3.141592653
POLYNOMIAL_FIT = 'ASAP'
#POLYNOMIAL_FIT = 'SIMPLE'
#POLYNOMIAL_FIT = 'CLIPPING'
SPLINE_FIT = 'ASAP'
#SPLINE_FIT = 'HEURISTICS'
#LINEFINDER = 'ASAP'
LINEFINDER = 'HEURISTICS'
CLIP_CYCLE = 1
INITFFTORDER = 2
HighLowFreqPartition = 9
MaxDominantFreq = 15
MAX_FREQ = 9
MAX_WINDOW_FRAGMENTATION = 3
MIN_CHANNELS = 512
NoData = -32767.0
CLUSTER_WHITEN = 1.0
CONV_FACTOR_LINEFINDER_THRE = 3.0

MAX_NhPanel = 5
MAX_NvPanel = 5
#MAX_NhPanelFit = 3
MAX_NhPanelFit = 4
MAX_NvPanelFit = 5
#NChannelMap = 12
NChannelMap = 15

#DPIDetail = 120
DPIDetail = 130
DPISummary = 90

DT_ROW = 0
DT_SCAN = 1
DT_IF = 2
DT_POL = 3
DT_BEAM = 4
DT_DATE = 5
DT_TIME = 6
DT_ELAPSED = 7
DT_EXPT = 8
DT_RA = 9
DT_DEC = 10
DT_AZ = 11
DT_EL = 12
DT_NCHAN = 13
DT_TSYS = 14
DT_TARGET = 15
DT_STAT = 16
DT_FLAG = 17
DT_PFLAG = 18
DT_SFLAG = 19
DT_NMASK = 20
DT_MASKLIST = 21
DT_NOCHANGE = 22
DT_ANT = 23

class SDEngine( SDLogger ):
    #USE_CASA_TABLE=False
    USE_CASA_TABLE=True
    
    def __init__( self, casaversion=0, casarevision=0, LogLevel=2, ConsoleLevel=3, LogFile=False ):
        """
        """
        SDLogger.__init__( self, level=LogLevel, consolelevel=ConsoleLevel, origin='SDEngine', fileout=LogFile, Global=True )

        # local tool
        #self._tbtool = casac.homefinder.find_home_by_name('tableHome')
        #self._tb = self._tbtool.create()
        #self._qatool = casac.homefinder.find_home_by_name('quantaHome')
        #self._qa = self._qatool.create()
	# Go to gentools
        self._qa = casac.quanta()
        self._tb = gentools(['tb'])[0]
        
        # 2011/10/23 GK for multiple antenna input
        if self.USE_CASA_TABLE:
            self.DataTable=DT.DataTableImpl()
        else:
            self.DataTable={}
        self.Row2ID={}
        self.Abcissa=None
        self.DetectSignal=None
        #self.listall=[]
        self.listall={}
        self.casaversion=casaversion
        self.casarevision=casarevision
##         self.ResultTable=None
##         self.PosDict=None
##         self.PosGap=None
##         self.TimeTable=None
##         self.TimeGap=None
        #self.stagesLogFile=None


    def __del__( self ):
        """
        """
        #del self._tbtool
        del self._tb
        #del self._qatool
        del self._qa


    # add contFile for the continuum output 2010/10/25 GK
    #def ReadData(self, filename, outFile, rawFile, TableOut, SummaryOut, LogFile, recipe=None):
    @dec_engines_logfile
    @dec_engines_logging
    def ReadData(self, D_filename, D_outFile, D_rawFile, D_contFile, D_TableOut, D_SummaryOut, LogFile=False, recipe=None, productTable=None):
        """
        Read header table to make new table file for SDpipeline
        Table format:
         ID,  Row, Scan,  IF, Pol, Beam, Date,  Time, ElapsedTime, Exptime,
         int, int,  int, int, int,  int,  str, float,       float,   float,
                                                sec,         sec,     sec,
        
           RA,   DEC,    Az,    El, nchan,  Tsys, TargetName
        float, float, float, float,   int, float, str
          deg,   deg,   deg,   deg
          
        TableOut: name of the output ascii table file
        SummaryOut: name of the output summary text
        return: [dict]: [DataTable for Ant0, DataTable for Ant1, ...]
          DataTable: [Row, Scan, IF, Pol, Beam, Date, Time, ElapsedTime,
                        0     1   2    3     4     5     6            7
                      Exptime, RA, DEC, Az, El, nchan, Tsys, TargetName, 
                            8   9   10  11  12     13    14          15
                      Statistics, Flags, PermanentFlags, SummaryFlag, Nmask, MaskList, NoChange, Ant]
                              16,    17,             18,          19,    20,       21,       22,  23
          Statistics: DataTable[ID][16] =
                      [LowFreqRMS, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, ExpectedRMS, ExpectedRMS]
                                0       1       2           3           4            5            6
          Flags: DataTable[ID][17] =
                      [LowFrRMSFlag, PostFitRMSFlag, PreFitRMSFlag, PostFitRMSdiff, PreFitRMSdiff, PostFitExpRMSFlag, PreFitExpRMSFlag]
                                  0               1              2               3              4                  5                 6
          PermanentFlags: DataTable[ID][18] =
                      [WeatherFlag, TsysFlag, UserFlag]
                                 0         1         2
         flag = 1: valid data,  flag = 0: flag out
        'LowFreqRMS, NewRMS, ...' fields were added only into the dictionary.
        OutPut:
         SpStorage: Array where all raw and reduced spectra are stored
         Abcissa: is array storing channel information
           Abcissa[0]: row number
           Abcissa[1]: Frequency (GHz)
           Abcissa[2]: LSRK velocity (km/s)
           Abcissa[3]: Wavelength (mm)
        """
        origin = 'ReadData()'
        
        import time
        stime = time.ctime()
        import sys
        import os
        import re

        if self.USE_CASA_TABLE:
            self.LogMessage('INFO',Origin=origin,Msg='Using CASA table style DataTable')
            self.LogMessage('INFO',Origin=origin,Msg='colnames=%s'%(self.DataTable.colnames()))
        else:
            self.LogMessage('INFO',Origin=origin,Msg='Using dict style DataTable')
    
        # ASDM data
        p=sys.path
        olderversion=False
        # 2009/8/18 Memory -> Disk only in DataPreparation.py
        tmpParam = sd.rcParams['scantable.storage']
        sd.rcParams['scantable.storage'] = 'disk'

        # 2011/10/24 GK input format is Dictionary
        rawFileList = []
        for vAnt in D_filename.keys():
            filename = D_filename[vAnt]
            outFile = D_outFile[vAnt]
            rawFile = D_rawFile[vAnt]
            if productTable is None:
                outTbl = D_outFile[vAnt]+'.product.tbl'
            else:
                outTbl = productTable[vAnt]
            if len(D_contFile) > 0: contFile = D_contFile[vAnt]
            else: contFile = None
            TableOut = D_TableOut[vAnt]
            SummaryOut = D_SummaryOut[vAnt]
            print 'vAnt', vAnt
            print 'filename', filename
            print 'outFile', outFile
            print 'rawFile', rawFile

            s = sd.scantable(filename, average=False)

            outfile = open(SummaryOut, 'w')
            print >> outfile, '<html><body>'
            print >> outfile, '<h3>Input data file: %s</h3>' % rawFile
            if recipe != None:
                print >> outfile, '<h3>Recipe file: %s</h3>' % recipe
            print >> outfile, '<img src="./RADEC.png">'
            print >> outfile, '<p>Figure 1: The figure shows individual telescope pointings (blue dots) and telescope slew (green line) along with the first pointing position with a beam pattern (red circle) and last pointing position (red dot).</p>'
            print >> outfile, '<HR>'
            print >> outfile, '<img src="./AzEl.png">'
            print >> outfile, '<p>Figure 2: Upper Panel: Elevation is plotted against  Time (UT). Vertical cyan lines (if any) represent larger gaps of observing time.  Lower Panel: Azimuth position is plotted against Time (UT). Vertical green lines (if any) show the time when the pointing changed largely.</p>'
            print >> outfile, '<HR>'
            print >> outfile, '<img src="./Weather.png">'
            print >> outfile, '<p>Figure 3: Upper Panel: Weather (Temperature(degC) and Humidity(%)) and Tsys (green) plot versus Time (MJD).  Lower Panel: Weather (Pressure(hPa) and Wind speed(m/s)) and Tsys (green) plot versus Time (MJD). Tsys are arbitrarily scaled to fit in the figure</p>'
            print >> outfile, '<HR>'
            print >> outfile, '<img src="./WVR.png">'
            print >> outfile, '<p>WVR readings are plotted above if the WVR data is available. Otherwise, the link would be broken.</p>'
            print >> outfile, '<HR>'
            print >> outfile, '<code>'
            ### 2011/10/18 TN
            ### _summary() becomes void function after 3.3
            #tmp = s._summary()
            if self.casarevision > 16563:
                s._summary( 'SDEngine.ReadData.tmp' )
                tmpfile = open( 'SDEngine.ReadData.tmp', 'r' )
                tmp = tmpfile.read()
                tmpfile.close()
                os.system('rm -rf SDEngine.ReadData.tmp' )
            else:
                tmp = s._summary()
            print >> outfile, tmp.replace('\n', '<br>\n')
            print >> outfile, '</code>'
            print >> outfile, '</body></html>'
            del tmp
            outfile.close()

            # If the format is either sdfits or rpf, save it to asap format
            if rawFile[-5:].upper().find('FIT') != -1 or rawFile[-5:].upper().find('RPF') != -1:
                tmpfilename = filename+'.tmpms'
                s.save(tmpfilename, format='asap')
                filename = tmpfilename

            #s.set_unit('channel')
            if os.path.isdir(outFile):
                self.LogMessage('INFO',Origin=origin,Msg='File %s exists. Deleting...' % outFile)
                os.system('rm -rf %s' % outFile)
            self.LogMessage('INFO',Origin=origin,Msg='Creating file %s' % outFile)
            s.save(outFile, format='asap')
            self.LogMessage('INFO',Origin=origin,Msg='File %s was created' % outFile)
            if contFile != None:
                if os.path.isdir(contFile):
                    self.LogMessage('INFO',Origin=origin,Msg='File %s exists. Deleting...' % contFile)
                    os.system('rm -rf %s' % contFile)
                self.LogMessage('INFO',Origin=origin,Msg='Creating file %s' % contFile)
                s.save(contFile, format='asap')
                self.LogMessage('INFO',Origin=origin,Msg='File %s was created' % contFile)
            nrow = s.nrow()
            npol = s.npol()
            nbeam = s.nbeam()
            nif = s.nif()
            # 2009/10/19 nchan for scantable is not correctly set
            #nchan = s.nchan()
            # 2010/2/10 TN
            # from 3.1 rcParams['verbose'] is disabled
            # instead, we have sd.asaplog.enable()/disable() methods
            vorg=sd.rcParams['verbose']
            sd.rcParams['verbose']=False
            if self.casaversion > 302:
                sd.asaplog.disable()
            Tsys = s.get_tsys()
            sd.rcParams['verbose']=vorg
            if self.casaversion > 302:
                sd.asaplog.enable()
##             if format.upper() == 'MS':
            if s.get_azimuth()[0] == 0: s.recalc_azel()
            self._tb.open(filename)
            Texpt = self._tb.getcol('INTERVAL')
##             Ttime = self._tb.getcol('TIME_CENTROID')
            # ASAP doesn't know the rows for cal are included in s.nrow()
            # nrow = len(Ttime)
            Tscan = self._tb.getcol('SCANNO')
            Tif = self._tb.getcol('IFNO')
            Tpol = self._tb.getcol('POLNO')
            Tbeam = self._tb.getcol('BEAMNO')
            # 2009/10/19 nchan for scantable is not correctly set
            NchanArray = NP.zeros(nrow, NP.int)
            for row in range(nrow):
                tmp = self._tb.getcell('SPECTRA', row)
                NchanArray[row] = len(tmp)
            del tmp
            self._tb.close()
        
            # 2011/10/23 GK List of DataTable for multiple antennas
            #self.DataTable = {}
            # Save file name to be able to load the special setup needed for the
            # flagging based on the expected RMS.
            # 2011/10/23 GK List of DataTable for multiple antennas
            #if self.DataTable.has_key('FileName') == False: self.DataTable['FileName']  = ['']
            if self.USE_CASA_TABLE:
                self.DataTable.putkeyword('FileName',[''])
            else:
                if self.DataTable.has_key('FileName') == False:
                    self.DataTable['FileName']  = ['']

            #self.DataTable['FileName'] = rawFile
            #self.DataTable['FileName'].append(rawFile)

            #if 'FileName' in self.DataTable.tb.keywordnames():
            #    l = self.DataTable.getkeyword('FileName').tolist()
            #    self.DataTable.putkeyword('FileName',l+[rawFile])
            #else:
            #    self.DataTable.putkeyword('FileName',[rawFile])

            rawFileList.append(rawFile)
            self.Row2ID[vAnt] = {}
            outfile = open(TableOut, 'w')
            outfile.write("!ID,Row,Scan,IF,Pol,Beam,Date,MJD,ElapsedTime,ExpTime,RA,Dec,Az,El,Nchan,Tsys,TargetName,AntennaID\n")

            # 2011/10/23 GK List of DataTable for multiple antennas
            ID = len(self.DataTable)-1
            #ID = len(self.DataTable)
            self.LogMessage('INFO',Msg='ID=%s'%(ID))
            #ID = 0
            ROWs = []
            IDs = []
            # 2009/7/16 to speed-up, get values as a list
            # 2011/11/8 get_direction -> get_directionval
            Sdir = s.get_directionval()
            #Sdir = s.get_direction()
            if ( sd.__version__ == '2.1.1' ):
                Stim = s.get_time()
            else:
                Stim = s.get_time(-1, True)
            Ssrc = s.get_sourcename()
            Saz = s.get_azimuth()
            Sel = s.get_elevation()

            # 2012/08/31 Temporary
            if os.path.isdir(outTbl):
                os.system('rm -rf %s' % outTbl)
            TBL = createExportTable(outTbl, nrow)
            #TBL = gentools(['tb'])[0]
            #TBL.open(self.TableOut, nomodify=False)
            #TBL.addrows(nrow)
            if self.USE_CASA_TABLE:
            #if False:
                self.DataTable.addrows( nrow )
                # column based storing
                intArr = NP.arange(nrow,dtype=int)
                self.DataTable.putcol('ROW',intArr,startrow=ID)
                self.DataTable.putcol('SCAN',Tscan,startrow=ID)
                self.DataTable.putcol('IF',Tif,startrow=ID)
                self.DataTable.putcol('POL',Tpol,startrow=ID)
                self.DataTable.putcol('BEAM',Tbeam,startrow=ID)
                self.DataTable.putcol('EXPOSURE',Texpt,startrow=ID)
                dirNP = NP.array(Sdir,dtype=float) * Rad2Deg
                self.DataTable.putcol('RA',dirNP[:,0],startrow=ID)
                self.DataTable.putcol('DEC',dirNP[:,1],startrow=ID)
                azNP = NP.array(Saz,dtype=float) * Rad2Deg
                self.DataTable.putcol('AZ',azNP,startrow=ID)
                elNP = NP.array(Sel,dtype=float) * Rad2Deg
                self.DataTable.putcol('EL',elNP,startrow=ID)
                self.DataTable.putcol('NCHAN',NchanArray,startrow=ID)
                self.DataTable.putcol('TSYS',Tsys,startrow=ID)
                self.DataTable.putcol('TARGET',Ssrc,startrow=ID)
                intArr[:] = 1
                self.DataTable.putcol('FLAG_SUMMARY',intArr,startrow=ID)
                intArr[:] = 0
                self.DataTable.putcol('NMASK',intArr,startrow=ID)
                intArr[:] = -1
                self.DataTable.putcol('NOCHANGE',intArr,startrow=ID)
                intArr[:] = vAnt
                self.DataTable.putcol('ANTENNA',intArr,startrow=ID)
                # row base storing
                stats = [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0]
                flags = [1, 1, 1, 1, 1, 1, 1]
                pflags = [1, 1, 1]
                masklist = [[0,0]]
                for x in range(nrow):
                    Ttime = Stim[x]
                    sDate = ("%4d-%02d-%02d" % (Ttime.year, Ttime.month, Ttime.day))
                    # Calculate MJD
                    sTime = ("%4d/%02d/%02d/%02d:%02d:%.1f" % (Ttime.year, Ttime.month, Ttime.day, Ttime.hour, Ttime.minute, Ttime.second))
                    qTime = self._qa.quantity(sTime)
                    MJD = qTime['value']
                    if x == 0: MJD0 = MJD
                    self.DataTable.putcell('DATE',ID,sDate)
                    self.DataTable.putcell('TIME',ID,MJD)
                    self.DataTable.putcell('ELAPSED',ID,(MJD-MJD0)*86400.0)
                    self.DataTable.putcell('STATISTICS',ID,stats)
                    self.DataTable.putcell('FLAG',ID,flags)
                    self.DataTable.putcell('FLAG_PERMANENT',ID,pflags)
                    self.DataTable.putcell('MASKLIST',ID,masklist)
                    if Ssrc[x].find('_calon') < 0:
                        outfile.write("%d,%d,%d,%d,%d,%d,%s,%.8f,%.3f,%.3f,%.8f,%.8f,%.3f,%.3f,%d,%f,%s,%d\n" % \
                                 (ID, x, Tscan[x], Tif[x], Tpol[x], Tbeam[x], \
                                 sDate, MJD, (MJD - MJD0) * 86400., Texpt[x], \
                                  dirNP[x,0],dirNP[x,1], \
                                  azNP[x], elNP[x], \
                                 NchanArray[x], Tsys[x], Ssrc[x], vAnt))

                        TBL.putcol('Row', int(x), int(x), 1, 1)
                        TBL.putcol('Ant', vAnt, int(x), 1, 1)
                        ROWs.append(int(x))
                        IDs.append(int(ID))
                        self.Row2ID[vAnt][int(x)] = int(ID)
                        ID += 1
            else:
                ###
                ###self.DataTable.addrows( nrow )
                for x in range(nrow):
                    if x % 10000 == 0: self.LogMessage('INFO',Origin=origin,Msg='Reading Meta-data %s'%(x))
                    RAdeg = Sdir[x][0] * Rad2Deg
                    DECdeg = Sdir[x][1] * Rad2Deg
                    #if ( sd.__version__ == '2.1.1' ):
                    #    Ttime = s.get_time(x)
                    #else:
                    #    Ttime = s.get_time(x,True)
                    Ttime = Stim[x]
                    sDate = ("%4d-%02d-%02d" % (Ttime.year, Ttime.month, Ttime.day))
                    # Calculate MJD
                    sTime = ("%4d/%02d/%02d/%02d:%02d:%.1f" % (Ttime.year, Ttime.month, Ttime.day, Ttime.hour, Ttime.minute, Ttime.second))
                    qTime = self._qa.quantity(sTime)
                    MJD = qTime['value']

                    if x == 0: MJD0 = MJD
                    sName = Ssrc[x]
                    # If it is not a cal data
                    if sName.find('_calon') < 0:
                        outfile.write("%d,%d,%d,%d,%d,%d,%s,%.8f,%.3f,%.3f,%.8f,%.8f,%.3f,%.3f,%d,%f,%s,%d\n" % \
                                (ID, x, Tscan[x], Tif[x], Tpol[x], Tbeam[x], \
                                 sDate, MJD, (MJD - MJD0) * 86400., Texpt[x], \
                                 RAdeg, DECdeg, \
                                 Saz[x]*Rad2Deg, Sel[x]*Rad2Deg, \
                                 NchanArray[x], Tsys[x], sName, vAnt))
                        self.DataTable[ID] = [int(x), Tscan[x], Tif[x], Tpol[x], Tbeam[x], \
                                 sDate, MJD, (MJD - MJD0) * 86400., Texpt[x], \
                                 RAdeg, DECdeg, \
                                 Saz[x]*Rad2Deg, Sel[x]*Rad2Deg, \
                                 NchanArray[x], Tsys[x], sName, \
                                 [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0], \
                                 [1, 1, 1, 1, 1, 1, 1], \
                                 [1, 1, 1], \
                                 1, 0, [], False, vAnt]
                        # 2012/08/31 for CASA Table
                        TBL.putcol('Row', int(x), int(x), 1, 1)
                        TBL.putcol('Ant', vAnt, int(x), 1, 1)
                        ROWs.append(int(x))
                        IDs.append(int(ID))
                        self.Row2ID[vAnt][int(x)] = int(ID)

                        ID += 1
##             for x in range(nrow):
##                 if x % 10000 == 0: self.LogMessage('INFO',Origin=origin,Msg='Reading Meta-data %s'%(x))
##                 RAdeg = Sdir[x][0] * Rad2Deg
##                 DECdeg = Sdir[x][1] * Rad2Deg
##                 #if ( sd.__version__ == '2.1.1' ):
##                 #    Ttime = s.get_time(x)
##                 #else:
##                 #    Ttime = s.get_time(x,True)
##                 Ttime = Stim[x]
##                 sDate = ("%4d-%02d-%02d" % (Ttime.year, Ttime.month, Ttime.day))
##                 # Calculate MJD
##                 sTime = ("%4d/%02d/%02d/%02d:%02d:%.1f" % (Ttime.year, Ttime.month, Ttime.day, Ttime.hour, Ttime.minute, Ttime.second))
##                 qTime = self._qa.quantity(sTime)
##                 MJD = qTime['value']

##                 if x == 0: MJD0 = MJD
##                 sName = Ssrc[x]
##                 # If it is not a cal data
##                 if sName.find('_calon') < 0:
##                     outfile.write("%d,%d,%d,%d,%d,%d,%s,%.8f,%.3f,%.3f,%.8f,%.8f,%.3f,%.3f,%d,%f,%s,%d\n" % \
##                             (ID, x, Tscan[x], Tif[x], Tpol[x], Tbeam[x], \
##                              sDate, MJD, (MJD - MJD0) * 86400., Texpt[x], \
##                              RAdeg, DECdeg, \
##                              Saz[x]*Rad2Deg, Sel[x]*Rad2Deg, \
##                              NchanArray[x], Tsys[x], sName, vAnt))
##                     self.DataTable[ID] = [int(x), Tscan[x], Tif[x], Tpol[x], Tbeam[x], \
##                              sDate, MJD, (MJD - MJD0) * 86400., Texpt[x], \
##                              RAdeg, DECdeg, \
##                              Saz[x]*Rad2Deg, Sel[x]*Rad2Deg, \
##                              NchanArray[x], Tsys[x], sName, \
##                              [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0], \
##                              [1, 1, 1, 1, 1, 1, 1], \
##                              [1, 1, 1], \
##                              1, 0, [], False, vAnt]
##                     # 2012/08/31 for CASA Table
##                     TBL.putcol('Row', int(x), int(x), 1, 1)
##                     TBL.putcol('Ant', vAnt, int(x), 1, 1)
##                     ROWs.append(int(x))
##                     IDs.append(int(ID))
##                     self.Row2ID[vAnt][int(x)] = int(ID)
        
##                     ID += 1
            #print x, time.ctime()
            TBL.close()
            outfile.close()
            listall = [IDs, ROWs, list(s.getscannos()), list(s.getifnos()), list(s.getpolnos()), list(s.getbeamnos())]
            # 2011/10/24 listall should be a dictionary in the future....
            #self.listall = listall[:]
            #self.listall.append(listall)
            self.listall[vAnt] = listall
            del Sdir, Stim, Ssrc, Saz, Sel, Tscan, Tif, Tpol, Tbeam, listall

            # Get telescope name
            TelName=s.get_antennaname()

            # Get Abcissa info (Frequency) for every IF
            # Abcissa[IF] [0]:channel [1]:Frequency(GHz) [2]:Velocity(Km/s) [3]:Wavelength(mm)
            self.Abcissa = {}
            # 2011/10/23 GK for multiple antenna
            #for IF in self.listall[3]:
            for IF in self.listall[vAnt][3]:
                print 'IF=', IF
                # Select one IF
                sel = s.get_selection()
                sel.set_ifs(IF)
                s.set_selection(sel)
                self.Abcissa[IF] = []
                # Set Abcissa info
                s.set_unit('channel')
                self.Abcissa[IF].append(NP.array(s.get_abcissa()[0]))
                s.set_unit('GHz')
                #RestFreq = s.get_restfreqs()
                RestFreq = 0.0
                #temporary fix
                if type(s.get_restfreqs())==dict:
                    RestFreqs = s.get_restfreqs().values()[0]
                    if ( len(RestFreqs) != 0 ):
                        RestFreq = RestFreqs[0]/1.0e9
                    else:
                        #RestFreq = self.Abcissa[IF][0][0]
                        RestFreq = s._getabcissa()[0]
                    #RestFreq = s.get_restfreqs().values()[0][0]/1.0e9
                    #RestFreq = s.get_restfreqs()[IF][0]/1.0e9
                    #RestFreq = s.get_restfreqs()[0][0]/1.0e9
                else:
                    #RestFreq = s.get_restfreqs()[IF]/1.0e9
                    RestFreq = s.get_restfreqs()[0]/1.0e9
                self.LogMessage('INFO',Origin=origin,Msg='RestFreq = %s'%(RestFreq))

                # 2010/06/25 TN
                # Abcissa is always absolute frequency value in new datasets.
                #
                #if re.match('APEX$', TelName) or re.match('^SMT', TelName) or re.match('^HHT', TelName) or re.match('^IRAM_30M',TelName) or re.match('^MRT',TelName) or re.match('^EFFELSBERG',TelName):
                #    self.Abcissa[IF].append(NP.array(s.get_abcissa()[0]) + RestFreq)
                #else:
                #    self.Abcissa[IF].append(NP.array(s.get_abcissa()[0]))
                self.Abcissa[IF].append(NP.array(s.get_abcissa()[0]))
                #is this correct?
                #original
                ##self.Abcissa[IF].append(((self.Abcissa[IF][1] / RestFreq) - 1.0) * 299792.458)
                #modified version by TT
                self.Abcissa[IF].append((1.0 - (self.Abcissa[IF][1] / RestFreq)) * 299792.458)
                #Abcissa[3] = 299792.458e+6 / Abcissa[1]
                self.Abcissa[IF].append(299.792458 / self.Abcissa[IF][1])
                #Abcissa[1] = Abcissa[1] / 1.0e+9
                s.set_unit('channel')
            del s

        # add list of rawfiles
        if self.USE_CASA_TABLE:
            self.DataTable.putkeyword('FileName',rawFileList)
        else:
            self.DataTable['FileName'] = rawFileList

        # 2009/8/18 Memory -> Disk only in DataPreparation.py
        sd.rcParams['scantable.storage'] = tmpParam

        #if self.USE_CASA_TABLE:
        #   self.DataTable.exportdata('ThisIsExportedDataTable.tbl',overwrite=True)

        return self.listall


    # Added by TT 
    # renumerate SCANNO so that different scan number for each row
    def RenumASAPData(self, filename):
        """
        renumerate SCANNO so that different scan number for each row
        """
        origin = 'RenumASAPData()' 
        
        # renumerate ASAP scantable
        import os.path
        filename = os.path.expandvars(filename)
        filename = os.path.expanduser(filename)
        if os.path.isdir(filename) \
                        and os.path.exists(filename+'/table.info'):
            # crude check if asap table
            if is_scantable(filename):
                isasapf = True
                tabname = filename
            else:
                isasapf = False
                tabname = filename+'.ASAP'

        # 2009/8/18 Memory -> Disk only in DataPreparation.py
        #print tabname
        tmpParam = sd.rcParams['scantable.storage']
        sd.rcParams['scantable.storage'] = 'disk'

        s = sd.scantable(filename, average=False)
        nr = s.nrow()
        scnos = s.getscannos()
        if not isasapf:
            s.save(tabname)
        self._tb.open(tabname, nomodify=False)
        scol = self._tb.getcol('SCANNO')
        newscol = NP.arange(len(scol)).tolist()
        ok = self._tb.putcol('SCANNO',newscol)
        self._tb.flush()
        self._tb.close()
        sd.rcParams['scantable.storage'] = tmpParam
        del s    


    # Modified by TT; use of lists-> (numpy) arrays 
    @dec_engines_logfile
    @dec_engines_logging
    def WriteNewMSData(self, ReferenceFile, MSout, SpStorage, Table, outform='ASAP', LogLevel=2, LogFile=False):
        """
        Write result in MS format
        
        ReferenceFile: reference file from which header and tables are copied
        MSout: output file with MS format
        SpStorage: Numarray of processed spectra.  SpStorage[nrows][Spectrum]
        Table format:
           [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]
                    ......
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]]
             0   1    2  3  4   5    6                  7
        """
        origin = 'WriteNewMSData()'
        
        import os
        import re
        OutRows = len(Table)

        # 2009/8/18 Memory -> Disk only in DataPreparation.py
        tmpParam = sd.rcParams['scantable.storage']
        sd.rcParams['scantable.storage'] = 'disk'

        s = sd.scantable(ReferenceFile, average=False)
        ifnos=s.getifnos()
        nrow = s.nrow()
        
        #if fluxunit is unset, set the default unit, K
        funit=s.get_fluxunit()
        if funit == '':
            s.set_fluxunit('K')
        #get rest frequency
        #temporary fix
        if type(s.get_restfreqs())==dict:
            restfs = s.get_restfreqs().values()[0]
            if ( len(restfs) != 0 ):
                restf = restfs[0]/1.0e9
            else:
                unitorg = s.get_unit()
                s.set_unit('Hz')
                restf = s._getabcissa(0)[0]
                s.set_unit( unitorg )
            #restf = s.get_restfreqs()[0][0]
        else:
            restf = s.get_restfreqs()[0]
        TelName=s.get_antennaname()
        #use temporary file
        #s.save(MSout, format='ASAP', overwrite=True)
        s.save(MSout.rstrip('/')+'.tmp', format='ASAP', overwrite=True)
        #s.save(MSout, format=outform, overwrite=True)
        del s
        #n = int((OutRows - 1) / nrow) + 1
        #if n <= 1:
        #    s.save(MSout, format='ASAP', overwrite=True)
        #    del s
        #else:
        #    t = sd.merge([s]*n)
        #    nrow *= n
        #    t.save(MSout, format='ASAP', overwrite=True)
        #    del s, t

        # fix and fill in some missing info
        # assume frequencies are  relative as in APEX and SMT data
        # Comment out since frequencies are always absolute value 2010/11/3 TN
##         if re.match('^GBT', TelName) or re.match('^DV', TelName) or re.match('DA', TelName):
##             pass # do nothing
##         else:
##             #use temporary file
##             #self._tb.open(MSout+'/FREQUENCIES', nomodify=False)
##             self._tb.open(MSout.rstrip('/')+'.tmp/FREQUENCIES', nomodify=False)
##             refval = NP.array(self._tb.getcol('REFVAL')) + restf
##             self._tb.putcol('REFVAL',refval)
##             freqframe = self._tb.getkeyword('FRAME')
##             self._tb.flush()
##             self._tb.close()
##             # insert frequency reference info in main table
##             #use temporary file
##             #self._tb.open(MSout, nomodify=False)
##             self._tb.open(MSout.rstrip('/')+'.tmp', nomodify=False)
##             self._tb.putkeyword('FreqRefFrame',freqframe)
##             self._tb.putkeyword('FreqRefVal',refval[0])
##             self._tb.flush()
##             self._tb.close()

        #use temporary file
        #self._tb.open(MSout, nomodify=False)
        self._tb.open(MSout.rstrip('/')+'.tmp', nomodify=False)
        freqidCol = self._tb.getcol('FREQ_ID')
        ifnoCol = self._tb.getcol('IFNO')

        # check IFNO-FREQ_ID relationship
        freqid={}
        for ifid in ifnos:
            idx=NP.where(ifnoCol==ifid)[0][0]
            freqid[ifid]=freqidCol[idx]            

        del freqidCol, ifnoCol

        # 2012/04/03 TN
        # optimize a process to fill table
        #  - minimize disk access
        #    - use getcell for column with single value
        #    - use putcol instead of putcell
        #  - share numpy array as much as possible

        Ref_time = self._tb.getcell('TIME',0)
        Ref_tsys = self._tb.getcell('TSYS',0)[0]
        #Ref_interval = self._tb.getcell('INTERVAL',0)
        Src_name = self._tb.getcell('SRCNAME',0)
        Fld_name = self._tb.getcell('FIELDNAME',0)
        srcpm = self._tb.getcell('SRCPROPERMOTION', 0)
        srcdir = self._tb.getcell('SRCDIRECTION', 0)
        scanrate = self._tb.getcell('SCANRATE', 0)

        #print 'OutRows=', OutRows, 'nrow=', nrow
        if OutRows < nrow:
            self._tb.removerows(range(OutRows, nrow))
        elif OutRows > nrow:
        #    print 'ERROR'
        #    sys.exit()
            self._tb.addrows(OutRows - nrow)
        #print 'self._tb.nrows()=', self._tb.nrows()

        # shared numpy array for int
        scalarIntCol = NP.zeros(OutRows,dtype='int32')

        # BEAMNO
        self._tb.putcol('BEAMNO',scalarIntCol)

        # SCANNO
        for row in xrange(OutRows):
            scalarIntCol[row] += row
        self._tb.putcol('SCANNO',scalarIntCol)

        # IFNO
        for row in xrange(OutRows):
            scalarIntCol[row] = Table[row][0]
        self._tb.putcol('IFNO',scalarIntCol)

        # FREQ_ID
        for row in xrange(OutRows):
            scalarIntCol[row] = freqid[Table[row][0]]
        self._tb.putcol('FREQ_ID',scalarIntCol)

        # POLNO
        for row in xrange(OutRows):
            scalarIntCol[row] = Table[row][1]
        self._tb.putcol('POLNO',scalarIntCol)

        del scalarIntCol

        # shared numpy array for float
        scalarFloatCol = NP.ones(OutRows,dtype='float64') * 0.1        

        # INTERVAL
        self._tb.putcol('INTERVAL',scalarFloatCol)

        # TIME
        for row in xrange(OutRows):
            scalarFloatCol[row] = Ref_time + float(row/86400.)
        self._tb.putcol('TIME', scalarFloatCol)

        # TSYS
        for row in xrange(OutRows):
            scalarFloatCol[row] = Ref_tsys
        scalarFloatCol.shape = (1,OutRows)
        self._tb.putcol('TSYS',scalarFloatCol)

        del scalarFloatCol

        # shared numpy array for two-element float array column
        directionCol = NP.zeros((2,OutRows),dtype='float64')

        # DIRECTION
        for row in xrange(OutRows):
            directionCol[0][row] = Table[row][4] / Rad2Deg
            directionCol[1][row] = Table[row][5] / Rad2Deg
        self._tb.putcol('DIRECTION',directionCol)

        # SRCPROPERMOTION
        for row in xrange(OutRows):
            directionCol[:,row] = srcpm
        self._tb.putcol('SRCPROPERMOTION',directionCol)        

        # SRCDIRECTION 
        for row in xrange(OutRows):
            directionCol[:,row] = srcdir
        self._tb.putcol('SRCDIRECTION',directionCol)

        # SCANRATE 
        for row in xrange(OutRows):
            directionCol[:,row] = scanrate
        self._tb.putcol('SCANRATE',directionCol)

        del directionCol

        # SRCNAME and FIELDNAME
        self._tb.putcol('SRCNAME',NP.array([Src_name]*OutRows))
        self._tb.putcol('FIELDNAME',NP.array([Fld_name]*OutRows))
        
        # SPECTRA and FLAGTRA
        self._tb.putcol('SPECTRA',SpStorage.transpose())
        self._tb.putcol('FLAGTRA',NP.zeros((SpStorage.shape[1],OutRows),dtype=NP.int32))
        self._tb.putkeyword('FreqRefFrame', 'LSRK')
        self._tb.putkeyword('nChan', SpStorage.shape[1] )

        self._tb.ok()
        self._tb.close()

        #use temporary file
        #s = sd.scantable(MSout, average=False)
        s = sd.scantable(MSout.rstrip('/')+'.tmp', average=False)
        #for row in range(OutRows):
        #    s._setspectrum(SpStorage[row], row)
        #s.recalc_azel()

        if outform=='MS2':
            savemsg='Write Re-Gridded Spectra to Disk as %s (MS format)'
        elif outform=='ASAP':
            savemsg='Write Re-Gridded Spectra to Disk as %s (ASAP format)'
        else:
            outform='ASAP'

        self.LogMessage('INFO', Origin=origin, Msg=savemsg % MSout)
        s.save(MSout, format=outform, overwrite=True)
        # for APEX data (2009/4/24)
        if ( outform=='MS2' ):
            self._tb.open(MSout+'/OBSERVATION',nomodify=False)
            telname=self._tb.getcol('TELESCOPE_NAME')
            for i in range(len(telname)):
                if ( telname[i].find('APEX-12m') != -1 ):
                    telname[i] = 'ALMA'
            self._tb.putcol('TELESCOPE_NAME',telname)
            self._tb.flush()
            self._tb.close()
        #
        #delete temporary file
        del s

        # 2009/8/18 Memory -> Disk only in DataPreparation.py
        sd.rcParams['scantable.storage'] = tmpParam
        os.system('rm -rf '+MSout.rstrip('/')+'.tmp')


    @dec_engines_logfile
    @dec_engines_logging
    def GroupByPosition(self, rows, vAnt, CombineRadius, AllowanceRadius, LogLevel=2, LogFile=False):
        """
        Grouping by RA/DEC position
            rows: rows to be processed
            CombineRadius: inside CombineRadius will be grouped together
            AllowanceRadius: inside AllowanceRadius are assumed to be the same position
        Return(PosDict, PosGap):
            PosDict: [[row1, row2,..., rowN],[ID1, ID2,..., IDN]] if not referenced before
                     [[-1, rowK],[IDK]] if has a reference to rowK
            PosGap: [rowX1, rowX2,...,rowXN]
        """
        origin = 'GroupByPosition()'

        self.LogMessage('INFO', Origin=origin, Msg='Grouping by Position...')

        # store Original ID, RA, DEC to ArrayID, ArrayRA, ArrayDEC
        Nrows = len(rows)
        Idx = []
        for row in rows: Idx.append(self.Row2ID[vAnt][row])

        if self.USE_CASA_TABLE:
            ArrayRA = NP.take(self.DataTable.getcol('RA'),Idx)
            ArrayDEC = NP.take(self.DataTable.getcol('DEC'),Idx)
            ArrayROW = NP.take(self.DataTable.getcol('ROW'),Idx)
        else:
            ArrayRA = NP.zeros(Nrows, dtype=NP.float64)
            ArrayDEC = NP.zeros(Nrows, dtype=NP.float64)
            ArrayROW = NP.zeros(Nrows, dtype=NP.int)
            for index in xrange(Nrows):
                ArrayROW[index] = self.DataTable[Idx[index]][DT_ROW]
                ArrayRA[index] = self.DataTable[Idx[index]][DT_RA]
                ArrayDEC[index] = self.DataTable[Idx[index]][DT_DEC]

##         ArrayRA = NP.zeros(Nrows, dtype=NP.float64)
##         ArrayDEC = NP.zeros(Nrows, dtype=NP.float64)
##         ArrayROW = NP.zeros(Nrows, dtype=NP.int)
##         for index in xrange(Nrows):
##             ArrayROW[index] = self.DataTable[Idx[index]][DT_ROW]
##             ArrayRA[index] = self.DataTable[Idx[index]][DT_RA]
##             ArrayDEC[index] = self.DataTable[Idx[index]][DT_DEC]

        ThresholdR = CombineRadius * CombineRadius
        ThresholdAR = AllowanceRadius * AllowanceRadius

        # 2009/2/10 Quicker Method
        PosDict = {}
        SelectDict = {}
        MinRA = ArrayRA.min()
        MaxRA = ArrayRA.max()
        MinDEC = ArrayDEC.min()
        MaxDEC = ArrayDEC.max()
        # Calculate the lattice position (sRA, sDEC) for each pointings
        # Create list of pointings (dictionary) for each lattice position
        for x in xrange(Nrows):
           sRA = int((ArrayRA[x] - MinRA) / CombineRadius / 2.0)
           sDEC = int((ArrayDEC[x] - MinDEC) / CombineRadius / 2.0)
           if not SelectDict.has_key(sRA): SelectDict[sRA] = {}
           if not SelectDict[sRA].has_key(sDEC): SelectDict[sRA][sDEC] = [[ArrayROW[x]],[x]]
           else:
               SelectDict[sRA][sDEC][0].append(ArrayROW[x])
               SelectDict[sRA][sDEC][1].append(Idx[x])
               #SelectDict[sRA][sDEC][1].append(x)

        # Create PosDict
        # make a list of spectra inside each lattice grid
        # store the list in PosDict[i] where 'i' is the smallest row number in the list
        # Other spectra have a reference to 'i'
        self.LogMessage('DEBUG', Origin=origin, Msg='SelectDict.keys() : %s' % SelectDict.keys())
        for sRA in SelectDict.keys():
            self.LogMessage('DEBUG', Origin=origin, Msg='len(SelectDict[%s].keys()) : %s' % (sRA, len(SelectDict[sRA].keys())))
            for sDEC in SelectDict[sRA].keys():
                PosDict[SelectDict[sRA][sDEC][0][0]] = SelectDict[sRA][sDEC]
                if len(SelectDict[sRA][sDEC][0]) != 1:
                    for x in SelectDict[sRA][sDEC][0][1:]:
                        PosDict[x] = [[-1,SelectDict[sRA][sDEC][0][0]],[SelectDict[sRA][sDEC][1][0]]]
        del SelectDict

        # Calculate thresholds to determine gaps
        DeltaP = NP.sqrt((ArrayRA[1:] - ArrayRA[:-1])**2.0 + (ArrayDEC[1:] - ArrayDEC[:-1])**2.0)
        DeltaQ = NP.take(DeltaP, NP.nonzero(DeltaP > ThresholdAR)[0])
        if len(DeltaQ) != 0:
            ThresholdP = NP.median(DeltaQ) * 10.0
        else:
            ThresholdP = 0.0
        self.LogMessage('INFO', Origin=origin, Msg='threshold:%s deg' % ThresholdP)
        #del ArrayROW, ArrayDEC, ArrayRA

        # List rows which distance from the previous position is larger than the threshold
        PosGap = []
        for x in range(1, Nrows):
            if DeltaP[x - 1] > ThresholdP:
                PosGap.append(Idx[x])
                #PosGap.append(self.DataTable[Idx[x]][DT_ROW])
                #self.LogMessage('INFO', Origin=origin, Msg='Position Gap %s deg at row=%d' % (DeltaP[x-1], self.DataTable[Idx[x]][DT_ROW]))
                self.LogMessage('INFO', Origin=origin, Msg='Position Gap %s deg at row=%d' % (DeltaP[x-1], ArrayROW[x]))

        del ArrayROW, ArrayDEC, ArrayRA

        # Print information
        if len(PosGap) == 0:
            PosGapMsg = 'Found no position gap'
        else:
            PosGapMsg = 'Found %d position gap(s)' % len(PosGap)
        self.LogMessage('INFO', Origin=origin, Msg=PosGapMsg)

        #print '\nPosGap', PosGap
        #print '\nPosDict', PosDict
        return (PosDict, PosGap)
        # PosGap is index
        # PosDict[row][0]: row, PosDict[row][1]: index



    @dec_engines_logfile
    @dec_engines_logging
    def GroupByTime(self, basedata, rows, vAnt, LogLevel=2, LogFile=False):
        """
        Grouping by time sequence
            basedata: thresholds are determined by basadata (list of rows)
            rows: thresholds are applied to rows
                basedata and rows are the same list in most of the cases
                basedata contains all spectra and rows are selections in other case
        Return (TimeTable, TimeGap)
            TimeTable: [[[[row0,..,rowN],[index0,..,indexN]],[[rowXX,..,rowXXX],[indexXX..]],..,[[,,]]], [[],[],[]]]]
            TimeTable[0]: separated by small gaps (for process5 and 7)
            TimeTable[1]: separated by large gaps (for process5 and 7)
            TimeGap: [[rowX1, rowX2,...,rowXN], [rowY1, rowY2,...,rowYN]]
            TimeGap[0]: small gap (for plot)
            TimeGap[1]: large gap (for plot)
        """
        origin = 'GroupByTime()'

        self.LogMessage('INFO', Origin=origin, Msg='Grouping by Time...')

        # Read time stamp for all basedata (list of rows) to determine thresholds
        if self.USE_CASA_TABLE:
            ArrayTime = NP.take(self.DataTable.getcol('ELAPSED'),basedata)
        else:
            ArrayTime = NP.zeros(len(basedata), dtype=NP.float64)
            for Index in xrange(len(basedata)):
                ArrayTime[Index] = self.DataTable[basedata[Index]][DT_ELAPSED]
##         ArrayTime = NP.zeros(len(basedata), dtype=NP.float64)
##         for Index in xrange(len(basedata)):
##             ArrayTime[Index] = self.DataTable[basedata[Index]][DT_ELAPSED]

        # 2009/2/5 adapted for multi beam which assumes to have identical time stamps
        # identical time stamps are rejected before determining thresholds
        # DeltaT: difference from the previous time stamp
        DeltaT = ArrayTime[1:] - ArrayTime[:-1]
        DeltaT1 = NP.take(DeltaT, NP.nonzero(DeltaT)[0])
        Threshold1 = NP.median(DeltaT1) * 5.0
        DeltaT2 = NP.take(DeltaT1, NP.nonzero(DeltaT1 > Threshold1)[0])
        if len(DeltaT2) > 0:
            Threshold2 = NP.median(DeltaT2) * 5.0
        else:
            Threshold2 = Threshold1

        # print information
        self.LogMessage('INFO', Origin=origin, Msg='Threshold1 = %s sec' % Threshold1)
        self.LogMessage('INFO', Origin=origin, Msg='Threshold2 = %s sec' % Threshold2)
        self.LogMessage('INFO', Origin=origin, Msg='MaxDeltaT = %s sec' % DeltaT1.max())
        self.LogMessage('INFO', Origin=origin, Msg='MinDeltaT = %s sec' % DeltaT1.min())
        del DeltaT2, DeltaT1, DeltaT, ArrayTime

        # Read time stamp for rows
        Idx = []
        for row in rows: Idx.append(self.Row2ID[vAnt][row])

        if self.USE_CASA_TABLE:
            ArrayTime = NP.take(self.DataTable.getcol('ELAPSED'),Idx)
        else:
            ArrayTime = NP.zeros(len(Idx), dtype=NP.float64)
            for Index in xrange(len(Idx)):
                ArrayTime[Index] = self.DataTable[Idx[Index]][DT_ELAPSED]
##         ArrayTime = NP.zeros(len(Idx), dtype=NP.float64)
##         for Index in xrange(len(Idx)):
##             ArrayTime[Index] = self.DataTable[Idx[Index]][DT_ELAPSED]

        # DeltaT: difference from the previous time stamp
        DeltaT = ArrayTime[1:] - ArrayTime[:-1]
        del ArrayTime

        tROW = None
        if self.USE_CASA_TABLE:
            tROW = self.DataTable.getcol('ROW')

        TimeTable = [[],[]]
        #SubTable1 = [self.DataTable[Idx[0]][DT_ROW]]
        #SubTable2 = [self.DataTable[Idx[0]][DT_ROW]]
        if self.USE_CASA_TABLE:
            SubTable1 = [tROW[Idx[0]]]
            SubTable2 = [tROW[Idx[0]]]
        else:
            SubTable1 = [self.DataTable[Idx[0]][DT_ROW]]
            SubTable2 = [self.DataTable[Idx[0]][DT_ROW]]
        IdxTable1 = [Idx[0]]
        IdxTable2 = [Idx[0]]
        TimeGap = [[],[]]

        # Detect small and large time gaps
        for index in xrange(len(Idx)-1):
            indexp1 = index + 1
            if DeltaT[index] <= Threshold1:
                if self.USE_CASA_TABLE:
                    SubTable1.append(tROW[Idx[indexp1]])
                else:
                    SubTable1.append(self.DataTable[Idx[indexp1]][DT_ROW])
                #SubTable1.append(self.DataTable[Idx[indexp1]][DT_ROW])
                IdxTable1.append(Idx[indexp1])
            else:
                TimeTable[0].append([SubTable1, IdxTable1])
                if self.USE_CASA_TABLE:
                    SubTable1 = [tROW[Idx[indexp1]]]
                else:                
                    SubTable1 = [self.DataTable[Idx[indexp1]][DT_ROW]]
                #SubTable1 = [self.DataTable[Idx[indexp1]][DT_ROW]]
                IdxTable1 = [Idx[indexp1]]
                TimeGap[0].append(Idx[indexp1])
                if self.USE_CASA_TABLE:
                    self.LogMessage('INFO', Origin=origin, Msg='Small Time Gap: %s sec at row=%d' % (DeltaT[index], tROW[Idx[indexp1]]))
                else:
                    self.LogMessage('INFO', Origin=origin, Msg='Small Time Gap: %s sec at row=%d' % (DeltaT[index], self.DataTable[Idx[indexp1]][DT_ROW]))
                #self.LogMessage('INFO', Origin=origin, Msg='Small Time Gap: %s sec at row=%d' % (DeltaT[index], self.DataTable[Idx[indexp1]][DT_ROW]))
            if DeltaT[index] <= Threshold2:
                if self.USE_CASA_TABLE:
                    SubTable2.append(tROW[Idx[indexp1]])
                else:
                    SubTable2.append(self.DataTable[Idx[indexp1]][DT_ROW])
                #SubTable2.append(self.DataTable[Idx[indexp1]][DT_ROW])
                IdxTable2.append(Idx[indexp1])
            else:
                TimeTable[1].append([SubTable2, IdxTable2])
                if self.USE_CASA_TABLE:
                    SubTable2 = [tROW[Idx[indexp1]]]
                else:
                    SubTable2 = [self.DataTable[Idx[indexp1]][DT_ROW]]
                #SubTable2 = [self.DataTable[Idx[indexp1]][DT_ROW]]
                IdxTable2 = [Idx[indexp1]]
                TimeGap[1].append(Idx[indexp1])
                if self.USE_CASA_TABLE:
                    self.LogMessage('INFO', Origin=origin, Msg='Large Time Gap: %s sec at row=%d' % (DeltaT[index], tROW[Idx[indexp1]]))
                else:
                    self.LogMessage('INFO', Origin=origin, Msg='Large Time Gap: %s sec at row=%d' % (DeltaT[index], self.DataTable[Idx[indexp1]][DT_ROW]))
                #self.LogMessage('INFO', Origin=origin, Msg='Large Time Gap: %s sec at row=%d' % (DeltaT[index], self.DataTable[Idx[indexp1]][DT_ROW]))

        if len(SubTable1) > 0: TimeTable[0].append([SubTable1, IdxTable1])
        if len(SubTable2) > 0: TimeTable[1].append([SubTable2, IdxTable2])
        del SubTable1, SubTable2, DeltaT, IdxTable1, IdxTable2

        # print information
        if len(TimeGap[0])==0: 
            TimeGapMsg = 'Found no time gap'
            self.LogMessage('INFO', Origin=origin, Msg=TimeGapMsg)
        else:
            TimeGapMsg1 = 'Found %d small time gap(s)' % len(TimeGap[0])
            TimeGapMsg2 = 'Found %d large time gap(s)' % len(TimeGap[1])
            self.LogMessage('INFO', Origin=origin, Msg=TimeGapMsg1)
            self.LogMessage('INFO', Origin=origin, Msg=TimeGapMsg2)

        #print '\nTimeGap', TimeGap
        #print '\nTimeTable', TimeTable
        return (TimeTable, TimeGap)
        # TimeGap is index
        # TimeTable[][0] is row, TimeTable[][1] is index



    @dec_engines_logfile
    @dec_engines_logging
    def MergeGapTables(self, TimeGap, TimeTable, PosGap, LogLevel=2, LogFile=False):
        """
        Merge TimeGap Table and PosGap Table. PosGap is merged into TimeGap Table[0]: Small gap
            TimeTable, TimeGap: output from GroupByTime()
            PosGap: output from GroupByPosition()
        Return (TimeTable, TimeGap)
            TimeTable[0]: separated by small gaps (for process5 and 7)
            TimeTable[1]: separated by large gaps (for process5 and 7)
            format: [[[row1,...,rowN],[index1,...,indexN]],[[row2,...,rowM],[index2,...,indexM]]]
        """
        origin = 'MergeGapTables()'

        self.LogMessage('INFO', Origin=origin, Msg='Merging Position and Time Gap tables...')

        tROW = None
        tBEAM = None
        if self.USE_CASA_TABLE:
            tROW = self.DataTable.getcol('ROW')
            tBEAM = self.DataTable.getcol('BEAM')

        idxs = []
        for i in xrange(len(TimeTable[0])):
            idxs += TimeTable[0][i][1]
        IDX = list(NP.sort(NP.array(idxs)))
        tmpGap = list(NP.sort(NP.array(TimeGap[0] + PosGap)))
        NewGap = []
        if len( tmpGap ) != 0: 
            t = n = tmpGap[0]
            for n in tmpGap[1:]:
                if t != n and t in IDX:
                    NewGap.append(t)
                    t = n
            if n in IDX:
                NewGap.append(n)
        TimeGap[0] = NewGap

        SubTable1 = []
        SubTable2 = []
        TimeTable[0] = []
        for index in range(len(IDX)):
            n = IDX[index]
            if n in TimeGap[0]:
                TimeTable[0].append([SubTable1, SubTable2])
                if self.USE_CASA_TABLE:
                    SubTable1 = [tROW[n]]
                    self.LogMessage('INFO', Origin=origin, Msg='Small Time Gap at row=%d' % tROW[n])
                else:
                    SubTable1 = [self.DataTable[n][DT_ROW]]                    
                    self.LogMessage('INFO', Origin=origin, Msg='Small Time Gap at row=%d' % self.DataTable[n][DT_ROW])
                #SubTable1 = [self.DataTable[n][DT_ROW]]
                SubTable2 = [n]
                #self.LogMessage('INFO', Origin=origin, Msg='Small Time Gap at row=%d' % self.DataTable[n][DT_ROW])
            else:
                if self.USE_CASA_TABLE:
                    SubTable1.append(tROW[n])
                else:
                    SubTable1.append(self.DataTable[n][DT_ROW])
                #SubTable1.append(self.DataTable[n][DT_ROW])
                SubTable2.append(n)
        if len(SubTable1) > 0: TimeTable[0].append([SubTable1, SubTable2])

        # 2009/2/6 Divide TimeTable in accordance with the Beam
        TimeTable2 = TimeTable[:]
        TimeTable = [[],[]]
        for i in range(len(TimeTable2)):
            for index in range(len(TimeTable2[i])):
                rows = TimeTable2[i][index][0]
                idxs = TimeTable2[i][index][1]
                BeamDict = {}
                for index2 in range(len(rows)):
                    row = rows[index2]
                    idx = idxs[index2]
                    if self.USE_CASA_TABLE:
                        if BeamDict.has_key(tBEAM[row]):
                            BeamDict[tBEAM[row]][0].append(row)
                            BeamDict[tBEAM[row]][1].append(idx)
                        else:
                            BeamDict[tBEAM[row]] = [[row],[idx]]
                    else:
                        if BeamDict.has_key(self.DataTable[row][DT_BEAM]):
                            BeamDict[self.DataTable[row][DT_BEAM]][0].append(row)
                            BeamDict[self.DataTable[row][DT_BEAM]][1].append(idx)
                        else:
                            BeamDict[self.DataTable[row][DT_BEAM]] = [[row],[idx]]
##                     if BeamDict.has_key(self.DataTable[row][DT_BEAM]):
##                         BeamDict[self.DataTable[row][DT_BEAM]][0].append(row)
##                         BeamDict[self.DataTable[row][DT_BEAM]][1].append(idx)
##                     else:
##                         BeamDict[self.DataTable[row][DT_BEAM]] = [[row],[idx]]
                BeamList = BeamDict.values()
                for beam in BeamList:
                    TimeTable[i].append(beam)

        #print TimeTable[0]
        del BeamDict, BeamList, TimeTable2

        self.LogMessage('DEBUG', Origin=origin, Msg='TimeTable = %s' % (TimeTable))

        #print '\nTimeGap', TimeGap
        #print '\nTimeTable', TimeTable
        return(TimeTable, TimeGap)



    @dec_engines_logfile
    @dec_engines_logging
    def ObsPatternAnalysis(self, PosDict, rows, vAnt, LogLevel=2, LogFile=False):
        """
        Analyze pointing pattern from PosDict
        Return (ret)
            ret: 'RASTER', 'SINGLE-POINT', or 'MULTI-POINT'
        # PosDict[row][0]: row, PosDict[][1]: index
        """
        origin = 'ObsPatternAnalysis()'

        self.LogMessage('INFO', Origin=origin, Msg='Analyze Scan Pattern by Positions...')

        nPos = 0
        for row in rows:
            if PosDict[row][0][0] != -1:
                nPos += 1
            if nPos == 0: nPos = 1
        self.LogMessage('DEBUG', Origin=origin, Msg='Number of Spectra: %d,   Number of independent position > %d' % (len(rows), nPos))
        #if nPos > math.sqrt(len(rows)) or nPos > 10: ret = 'RASTER'
        if nPos > math.sqrt(len(rows)) or nPos > 3: ret = 'RASTER'
        elif nPos == 1: ret = 'SINGLE-POINT'
        else: ret = 'MULTI-POINT'
        self.LogMessage('INFO', Origin=origin, Msg='Pattern is %s' % (ret))
        return ret


    def FreqWindow2Channel(self, vIF, SpectrumWindow):
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
        origin = 'FreqWindow2Channel()'

        Abcissa = self.Abcissa[vIF]
        
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
                DiffMinFreq = MinFreq
                DiffMaxFreq = MaxFreq
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



    def Channel2FreqWindow(self, vIF, ChannelWindow):
        """
        Convert Channel Window to Frequency Window (GHz)
        Input:
          Abcissa: [0]:channel [1]:Frequency(GHz) [2]:Velocity(Km/s) [3]:Wavelength(mm)
          ChannelWindow: [[min Channel, max Channel],[minC,maxC],,,]
        Output:
          SpectrumWindow: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
        """
        origin = 'Channel2FreqWindow()'

        Abcissa = self.Abcissa[vIF]
        
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



    @dec_engines_logfile
    @dec_engines_logging
    def Process3(self, DataIn, ResultTable, PosDict, rows, NCHAN, LineFinderRule, SpWinRange=[], edge=(0, 0), BroadComponent=True, LogLevel=2, LogFile=False):
        """
        The process finds emission lines and determines protection regions for baselinefit
        'dummyscan' is a dummy scantable in order to use ASAP linefinder class
        """
        origin = 'Process3()'

        tRA = None
        tDEC = None
        if self.USE_CASA_TABLE:
            tRA = self.DataTable.getcol('RA')
            tDEC = self.DataTable.getcol('DEC')
        
        self.DetectSignal = {}
        # Pre-Defined Spectrum Window
        if len(SpWinRange) != 0:
            self.LogMessage('INFO', Origin=origin, Msg='Skip Process3')
            if self.USE_CASA_TABLE:
                for idx in rows:
                    row = rows[idx]
                    self.DetectSignal[row] = [tROW[row], tDEC[row], SpWinRange]
                    for row in range(len(self.DataTable)):
                        self.DataTable.putcell('MASKLIST',row,SpWinRange)
            else:
                for idx in rows:
                    row = rows[idx]
                    self.DetectSignal[row] = [self.DataTable[row][DT_RA], self.DataTable[row][DT_DEC], SpWinRange]
                    for row in range(len(self.DataTable)):
                        self.DataTable[row][DT_MASKLIST] = SpWinRange
##             for idx in rows:
##                 row = rows[idx]
##                 self.DetectSignal[row] = [self.DataTable[row][DT_RA], self.DataTable[row][DT_DEC], SpWinRange]
##             for row in range(len(self.DataTable)):
##                 self.DataTable[row][DT_MASKLIST] = SpWinRange
            return self.DetectSignal

        NROW = len(rows)
        # create storage
        SpOned = NP.zeros(NCHAN, dtype=NP.float64)

        self.LogMessage('INFO', Origin=origin, Msg='Search regions for protection against the background subtraction...')
        self.LogMessage('INFO', Origin=origin, Msg='Processing %d spectra...' % NROW)

        # read data to SpStorage
        if DataIn != None:
            self._tb.open(DataIn)

        # Set edge mask region
        (EdgeL, EdgeR) = parseEdge(edge)
        Nedge = EdgeR + EdgeL
        if Nedge >= NCHAN:
            self.LogMessage('ERROR', Origin=origin, Msg='Error: Edge masked region too large...')
            return False

        #2010/6/9 Max FWHM found to be too large!
        if LineFinderRule['MaxFWHM'] > ((NCHAN-Nedge)/2):
            MaxFWHM = int((NCHAN-Nedge)/2)
            LineFinderRule['MaxFWHM'] = MaxFWHM
        else:
            MaxFWHM = int(LineFinderRule['MaxFWHM'])
        MinFWHM = int(LineFinderRule['MinFWHM'])
        Threshold = LineFinderRule['Threshold']        

        # 2011/05/17 TN
        # Switch to use either ASAP linefinder or John's linefinder
        if LINEFINDER == 'ASAP':
            #from heuristics.shared.hlinefinder import hlinefinder
            from pipeline.h.heuristics.hlinefinder import hlinefinder
            LF = hlinefinder(nchan=NCHAN)
            ### 2011/05/23 for linefinder2
            Thre = [Threshold, Threshold * math.sqrt(2)]
            if BroadComponent: (Start, Binning) = (0, 1)
            else: (Start, Binning) = (1, 1)
        elif LINEFINDER == 'HEURISTICS':
            #from heuristics.shared.linefinder2 import hlinefinder
            from pipeline.h.heuristics.linefinder2 import hlinefinder
            LF = hlinefinder()
            ### 2011/05/23 for linefinder2
            #Thre = [Threshold * CONV_FACTOR_LINEFINDER_THRE, Threshold * math.sqrt(2) * CONV_FACTOR_LINEFINDER_THRE]
            ### 2012/02/29 for Broad Line detection by Binning
            ### Only 'Thre' is effective for Heuristics Linefinder. BoxSize, AvgLimit, and MinFWHM are ignored
            #Thre = [Threshold * CONV_FACTOR_LINEFINDER_THRE, Threshold * CONV_FACTOR_LINEFINDER_THRE]
            Thre = [Threshold * CONV_FACTOR_LINEFINDER_THRE, Threshold * CONV_FACTOR_LINEFINDER_THRE, Threshold * CONV_FACTOR_LINEFINDER_THRE]
            if BroadComponent: (Start, Binning) = (0, 5)
            else: (Start, Binning) = (1, 3)
        
        # try 2010/10/27
        #BoxSize = [min(2.0*MaxFWHM/(NCHAN - Nedge), 0.5, (NCHAN - Nedge)/float(NCHAN)*0.9), min(max(MaxFWHM/(NCHAN - Nedge)/4.0, 0.1), (NCHAN - Nedge)/float(NCHAN)/2.0)]
        BoxSize = [min(2.0*MaxFWHM/(NCHAN - Nedge), 0.5, (NCHAN - Nedge)/float(NCHAN)*0.9), min(max(MaxFWHM/(NCHAN - Nedge)/4.0, 0.1), (NCHAN - Nedge)/float(NCHAN)/2.0), min(max(MaxFWHM/(NCHAN - Nedge)/4.0, 0.1), (NCHAN - Nedge)/float(NCHAN)/2.0)]
        ### 2011/05/23 for linefinder2
        #Thre = [Threshold, Threshold * math.sqrt(2)]
        #Thre = [Threshold, Threshold * math.sqrt(2), Threshold * 2]
        #AvgLimit = [MinFWHM * 16, MinFWHM * 4]
        AvgLimit = [MinFWHM * 16, MinFWHM * 4, MinFWHM]
        Processed = []
        Pending = []


        # Create progress timer
        Timer = ProgressTimer(80, NROW, LogLevel)
        for index in range(NROW):
            row = rows[index]
            if len(ResultTable) == 0:
                if self.USE_CASA_TABLE:
                    ProtectRegion = [tRA[row], tDEC[row], []]
                else:
                    ProtectRegion = [self.DataTable[row][DT_RA], self.DataTable[row][DT_DEC], []]
                #ProtectRegion = [self.DataTable[row][DT_RA], self.DataTable[row][DT_DEC], []]
            else:
                ProtectRegion = [ResultTable[row][4], ResultTable[row][5], []]
                #               [RA,                DEC,                []]
            if len(ResultTable) == 0 and PosDict[row][0][0] == -1:
                self.LogMessage('DEBUG', Origin=origin, Msg='Row %d: Skipped - Reference spectrum' % row)
                # Reference to other spectrum
                X = int(PosDict[row][0][1])
                if X in Processed:
                    ### 2011/05/13 Stop the copy of the detected signal from reference
                    self.DetectSignal[row] = self.DetectSignal[X]
                    #ProtectRegion[2].append([-1, -1])
                    #self.DetectSignal[row] = ProtectRegion
                    ### 2011/05/13 end
                    Processed.append(row)
                    # Countup progress timer
                    Timer.count()
                    continue
                else:
                    Pending.append(row)
                    ### 2011/05/13 Stop the copy of the detected signal from reference
                    #ProtectRegion[2].append([-1, -1])
                    #self.DetectSignal[row] = ProtectRegion
                    ### 2011/05/13 end
                    #STEP = STEP - 1
                    continue
            elif len(ResultTable) != 0 and ResultTable[index][6] == 0:
                self.LogMessage('DEBUG', Origin=origin, Msg='Row %d: No spectrum' % row)
                # No spectrum
                ProtectRegion[2].append([-1, -1])
                self.DetectSignal[row] = ProtectRegion
                Processed.append(row)
                # Countup progress timer
                Timer.count()
            else:
                ProcStartTime = time.time()
                self.LogMessage('DEBUG', Origin=origin, Msg='Start Row %d' % (row))
                # 2011/03/24 TN
                # use hlinefinder instead of sd.linefinder
                #scan._setspectrum(NP.array(SpStorage[index],dtype=NP.float64))
                ### 2011/05/26 not use SpStorage
                SpOned = self._tb.getcell('SPECTRA', row)
                #LF.set_spectrum(NP.array(SpStorage[index],dtype=NP.float64))
                ### 2012/02/29 GK for broad line detection
                ###LF.set_spectrum(SpOned)
                # Countup progress timer
                Timer.count()
                
                # Try to detect broader component and narrow component separately
                for y in range(Start, len(Thre)):
                #for y in range(Start, 2):
                    ### 2012/02/29 GK for broad line detection
                    ### y=0: Bin=25,  y=1: Bin=5 or 3,  y=2: Bin=1
                    Bin = Binning ** (2-y)
                    if Bin != 1 and NCHAN/Bin < 50: continue
                    LF.set_spectrum(SpBinning(SpOned, Bin))
                    self.LogMessage('DEBUG', Origin=origin, Msg='line detection parameters: ')
                    self.LogMessage('DEBUG', Origin=origin, Msg='threshold (S/N per channel)=%.1f,' % Thre[y] \
                                   + 'min_nchan for detection=%d, running mean box size (in fraction of spectrum)=%s, ' % (MinFWHM, BoxSize[y]) \
                                   + 'upper limit for averaging=%s channels, edges to be dropped=[%s, %s]' % (AvgLimit[y], EdgeL, EdgeR) )
                    LF.set_options(threshold=Thre[y], box_size=BoxSize[y], min_nchan=MinFWHM, avg_limit=AvgLimit[y], tweak=True)
                    #nlines = LF.find_lines(edge=(EdgeL, EdgeR))
                    nlines = LF.find_lines(edge=(int((EdgeL+Bin-1)/Bin), int((EdgeR+Bin-1)/Bin)))
                    ### Debug TT
                    ###self.LogMessage('INFO', Origin=origin, Msg='NLINES=%s, EdgeL=%s, EdgeR=%s' % (nlines, EdgeL, EdgeR))
                    # No-line is detected
                    if (nlines == 0):
                       if len(ProtectRegion[2]) == 0: ProtectRegion[2].append([-1, -1])
                    # Single line is detected
                    elif (nlines == 1):
                        Ranges = getWidenLineList(int(NCHAN/Bin), 1, 1, LF.get_ranges(False))
                        ### 2012/02/29 GK for broad line detection
                        (Ranges[0], Ranges[1]) = (Ranges[0]*Bin+int(Bin/2), Ranges[1]*Bin+int(Bin/2))
                        Width = Ranges[1] - Ranges[0] + 1
                        ### 2011/05/16 allowance was moved to clustering analysis
                        allowance = int(Width/5)
                        #allowance = int(Width/10)
                        #Debug (TT)
                        self.LogMessage('DEBUG', Origin=origin, Msg='Ranges=%s, Width=%s, allowance=%s' % (Ranges, Width, allowance))
                        ### 2011/05/16 allowance was moved to clustering analysis
                        #if Width >= MinFWHM and Width <= MaxFWHM and \
                        #   Ranges[0] > (EdgeL + allowance + 1) and \
                        #   Ranges[1] < (NCHAN - 2 - allowance - EdgeR):
                        #    ProtectRegion[2].append([Ranges[0] - allowance, Ranges[1] + allowance])
                        if Width >= MinFWHM and Width <= MaxFWHM and \
                           Ranges[0] > EdgeL and \
                           Ranges[1] < (NCHAN - 1 - EdgeR):
                            if len(ProtectRegion[2]) != 0 and ProtectRegion[2][0] == [-1, -1]:
                                ProtectRegion[2][0] = [Ranges[0], Ranges[1]]
                            else: ProtectRegion[2].append([Ranges[0], Ranges[1]])
                        elif len(ProtectRegion[2]) == 0: ProtectRegion[2].append([-1, -1])
                    # Multipule lines (candidates) are detected
                    else:
                        linestat = []
                        Ranges = getWidenLineList(int(NCHAN/Bin), 1, 1, LF.get_ranges(False))
                        #for y in range(nlines):
                        for y in range(int(len(Ranges)/2)):
                            ### 2012/02/29 GK for broad line detection
                            (Ranges[y*2], Ranges[y*2+1]) = (Ranges[y*2]*Bin+int(Bin/2), Ranges[y*2+1]*Bin+int(Bin/2))
                            Width = Ranges[y*2+1] - Ranges[y*2] + 1
                            ### 2011/05/16 allowance was moved to clustering analysis
                            allowance = int(Width/5)
                            #allowance = int(Width/10)
                            #Debug (GK)
                            self.LogMessage('DEBUG', Origin=origin, Msg='Ranges=%s, Width=%s, allowance=%s' % (Ranges, Width, allowance))
                            ### 2011/05/16 allowance was moved to clustering analysis
                            #if Width >= MinFWHM and Width <= MaxFWHM and \
                            #   Ranges[y*2] > (EdgeL + allowance + 1) and \
                            #   Ranges[y*2+1] < (NCHAN - 2 - allowance - EdgeR):
                            #    linestat.append((Ranges[y*2] - allowance, Ranges[y*2+1] + allowance, SpStorage[index][Ranges[y*2]:Ranges[y*2+1]].max() - SpStorage[index][Ranges[y*2]:Ranges[y*2+1]].min()))
                            #if Width >= MinFWHM and Width <= MaxFWHM and \
                            if Width > MinFWHM and Width < MaxFWHM and \
                               Ranges[y*2] > EdgeL and \
                               Ranges[y*2+1] < (NCHAN - 1 - EdgeR):
                                ### 2011/05/26 not use SpStorage
                                #linestat.append((Ranges[y*2], Ranges[y*2+1], SpStorage[index][Ranges[y*2]:Ranges[y*2+1]].max() - SpStorage[index][Ranges[y*2]:Ranges[y*2+1]].min()))
                                linestat.append((Ranges[y*2], Ranges[y*2+1], SpOned[Ranges[y*2]:Ranges[y*2+1]].max() - SpOned[Ranges[y*2]:Ranges[y*2+1]].min()))
                        # No candidate lines are left
                        if len(linestat) == 0:
                            if len(ProtectRegion[2]) == 0: ProtectRegion[2].append([-1, -1])
                        # More than or equal to one line are left
                        else:
                            if len(ProtectRegion[2]) == 1 and ProtectRegion[2][0] == [-1, -1]:
                                ProtectRegion[2] = []
                            for y in range(len(linestat)):
                                # Store line if max intensity exceeds 1/30 of the strongest one
                                #if linestat[y][2] > Threshold:
                                #    ProtectRegion[2].append([linestat[y][0], linestat[y][1]])
                                ProtectRegion[2].append([linestat[y][0], linestat[y][1]])
                            # 2007/09/01 Merge lines into one if two lines are close
                            flag = True
                            for y in range(len(linestat) - 1):
                                if (linestat[y+1][0] - linestat[y][1]) < (min((linestat[y][1]-linestat[y][0]),(linestat[y+1][1]-linestat[y+1][0]))/4.0):
                                    if flag == True:
                                        if linestat[y][1] < linestat[y+1][1] and linestat[y][0] < linestat[y+1][0] and (linestat[y+1][1] - linestat[y][0]) < MaxFWHM:
                                            ProtectRegion[2].append([linestat[y][0], linestat[y+1][1]])
                                            Line0 = linestat[y][0]
                                        else: continue
                                    else:
                                        if (linestat[y+1][1] - Line0) < MaxFWHM:
                                            ProtectRegion[2].pop()
                                            ProtectRegion[2].append([Line0, linestat[y+1][1]])
                                        else:
                                            flag = True
                                            continue
                                    flag = False
                                else: flag = True
                self.DetectSignal[row] = ProtectRegion
                Processed.append(row)
                ProcEndTime = time.time()
                self.LogMessage('INFO', Origin=origin, Msg='Channel ranges of detected lines for Row %s: %s' % (row, self.DetectSignal[row][2]))

                self.LogMessage('DEBUG', Origin=origin, Msg='End Row %d: Elapsed Time=%.1f sec' % (row, (ProcEndTime - ProcStartTime)) )
        self._tb.close()
        del SpOned

        if len(Pending) != 0 and len(ResultTable) == 0:
            ProcStartTime = time.time()
            self.LogMessage('DEBUG', Origin=origin, Msg="Process pending list: %s" % (Pending))
            # Deal with pending list
            for row in Pending:
                if not PosDict[row][0][1] in Processed:
                    # Reference Not ready
                    self.LogMessage('WARNING', Origin=origin, Msg="Wrong reference detected!!!")
                else:
                    # Copy referece detections
                    ### 2011/05/13 Stop the copy of the detected signal from reference
                    self.DetectSignal[row] = self.DetectSignal[PosDict[row][0][1]]
                    ### 2011/05/13 end
                    Processed.append(row)
                # Countup progress timer
                Timer.count()

            ProcEndTime = time.time()
            self.LogMessage('INFO', Origin=origin, Msg="Pending list process End: Elapsed time = %.1f sec" % (ProcEndTime - ProcStartTime) )

        del Timer

        return self.DetectSignal



    @dec_engines_logfile
    @dec_engines_logging
    def Process4(self, ResultTable, vIF, idxList, SpWin, Pattern, GridSpaceRA, GridSpaceDEC, ITER, Nsigma=3.0, Xorder=-1, Yorder=-1, BroadComponent=False, ClusterRule=None, LineFinderRule=None, LogLevel=2, LogFile=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
        """
        2D fit line characteristics calculated in Process3
        Sigma clipping iterations will be applied if Nsigma is positive
        order < 0 : automatic determination of fitting order (max = 5)
        SpWin: Pre-Defined Spectrum Window
         DetectSignal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannel2, LineEndChannel2],
                                         [LineStartChannelN, LineEndChannelN]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannelN, LineEndChannelN]]]}

        Lines: output parameter
           [LineCenter, LineWidth, Validity]  OK: Validity = True; NG: Validity = False
        """
        origin = 'Process4()'

        import numpy
        import scipy.cluster.vq as VQ

        #import SDpipelineControl as SDC
        #reload(SDC)
        Valid = ClusterRule['ThresholdValid']
        Marginal = ClusterRule['ThresholdMarginal']
        Questionable = ClusterRule['ThresholdQuestionable']

        #2010/6/9 Delete global parameter Min/MaxFWHM
        MinFWHM = LineFinderRule['MinFWHM']
        MaxFWHM = LineFinderRule['MaxFWHM']

        Abcissa = self.Abcissa[vIF]
        
        # for Pre-Defined Spectrum Window
        if len(SpWin) != 0:
            self.LogMessage('INFO', Origin=origin, Msg='Skip Process4')
            return SpWin

        self.LogMessage('INFO', Origin=origin, Msg='2D fit the line characteristics...')

        #tMASKLIST = None
        #tNOCHANGE = None
        tRA = None
        tDEC = None
        tSFLAG = None
        if self.USE_CASA_TABLE:
            tRA = self.DataTable.getcol('RA')
            tDEC = self.DataTable.getcol('DEC')
            tSFLAG = self.DataTable.getcol('FLAG_SUMMARY')
        
        TotalLines = 0
        RMS0 = 0.0
        Xorder0 = Xorder
        Yorder0 = Yorder
        # Dictionary for final output
        RealSignal = {}
        if self.USE_CASA_TABLE:
            nChan = self.DataTable.getcell('NCHAN',idxList[0])
        else:
            nChan = self.DataTable[idxList[0]][DT_NCHAN]
        #nChan = self.DataTable[idxList[0]][DT_NCHAN]
        Lines = []

        # First cycle
        if len(ResultTable) == 0:
            ROWS = idxList
            if Pattern.upper() == 'SINGLE-POINT' or Pattern.upper() == 'MULTI-POINT':
                if self.USE_CASA_TABLE:
                    for row in idxList:
                        tMASKLIST = self.DataTable.getcell('MASKLIST',row)
                        tNOCHANGE = self.DataTable.getcell('NOCHANGE',row)
                        self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (tMASKLIST, self.DetectSignal[row][2], tNOCHANGE))
                        self.DataTable.putcell('MASKLIST',row,self.DetectSignal[row][2])
                        self.DataTable.putcell('NOCHANGE',row,False)
                else:
                    for row in idxList:
                        self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (self.DataTable[row][DT_MASKLIST], self.DetectSignal[row][2], self.DataTable[row][DT_NOCHANGE]))
                        self.DataTable[row][DT_MASKLIST] = self.DetectSignal[row][2]
                        self.DataTable[row][DT_NOCHANGE] = False
##                 for row in idxList:
##                     self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (self.DataTable[row][DT_MASKLIST], self.DetectSignal[row][2], self.DataTable[row][DT_NOCHANGE]))
##                     self.DataTable[row][DT_MASKLIST] = self.DetectSignal[row][2]
##                     self.DataTable[row][DT_NOCHANGE] = False
                return Lines

        # Iteration case
        else:
            ROWS = range(len(ResultTable))
            if Pattern.upper() == 'SINGLE-POINT' or Pattern.upper() == 'MULTI-POINT':
                if self.USE_CASA_TABLE:
                    for row in idxList:
                        RealSignal[row] = self.DetectSignal[0]
                        tMASKLIST = self.DataTable.getcell('MASKLIST',row)
                        tNOCHANGE = self.DataTable.getcell('NOCHANGE',row)
                        self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (tMASKLIST, self.DetectSignal[0][2], tNOCHANGE))
                        if tMASKLIST == self.DetectSignal[0][2]:
                            if type(tNOCHANGE) != int:
                                self.DataTable.putcell('NOCHANGE',row,ITER - 1)
                        else:
                            self.DataTable.putcell('MASKLIST',row,self.DetectSignal[0][2])
                            self.DataTable.putcell('NOCHANGE',row,False)
                else:
                    for row in idxList:
                        RealSignal[row] = self.DetectSignal[0]
                        self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (self.DataTable[row][DT_MASKLIST], self.DetectSignal[0][2], self.DataTable[row][DT_NOCHANGE]))
                        if self.DataTable[row][DT_MASKLIST] == self.DetectSignal[0][2]:
                            if type(self.DataTable[row][DT_NOCHANGE]) != int:
                                self.DataTable[row][DT_NOCHANGE] = ITER - 1
                        else:
                            self.DataTable[row][DT_MASKLIST] = self.DetectSignal[0][2]
                            self.DataTable[row][DT_NOCHANGE] = False
##                 for row in idxList:
##                     RealSignal[row] = self.DetectSignal[0]
##                     self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (self.DataTable[row][DT_MASKLIST], self.DetectSignal[0][2], self.DataTable[row][DT_NOCHANGE]))
##                     if self.DataTable[row][DT_MASKLIST] == self.DetectSignal[0][2]:
##                         if type(self.DataTable[row][DT_NOCHANGE]) != int:
##                             self.DataTable[row][DT_NOCHANGE] = ITER - 1
##                     else:
##                         self.DataTable[row][DT_MASKLIST] = self.DetectSignal[0][2]
##                         self.DataTable[row][DT_NOCHANGE] = False
                return Lines

        # RASTER CASE
        # Read data from Table to generate ID -> RA, DEC conversion table
        Region = []
        dummy = []
        flag = 1
        Npos = 0
        MaxLines = 0
        ### 2011/05/13 Calculate median line width
        Width = []
        for row in ROWS:
            if len(self.DetectSignal[row][2]) > MaxLines: MaxLines = len(self.DetectSignal[row][2])
            if self.DetectSignal[row][2][0][0] != -1: Npos += 1
            for line in self.DetectSignal[row][2]:
                # Check statistics flag added by G.K. 2008/1/17
                # Bug fix 2008/5/29
                if self.USE_CASA_TABLE:
                    if (line[0] != line[1]) and ((len(ResultTable) == 0 and tSFLAG[row] == 1) or len(ResultTable) != 0):
                        Region.append([row, line[0], line[1], self.DetectSignal[row][0], self.DetectSignal[row][1], flag])
                        ### 2011/05/17 make cluster insensitive to the line width
                        dummy.append([float(line[1] - line[0]) / CLUSTER_WHITEN, (line[0] + line[1]) / 2.0])
                        #dummy.append([float(line[1] - line[0]), (line[0] + line[1]) / 2.0])
                        ### 2011/05/13 Calculate median line width
                        Width.append(float(line[1] - line[0]) / CLUSTER_WHITEN)
                else:
                    if (line[0] != line[1]) and ((len(ResultTable) == 0 and self.DataTable[row][DT_SFLAG] == 1) or len(ResultTable) != 0):
                        Region.append([row, line[0], line[1], self.DetectSignal[row][0], self.DetectSignal[row][1], flag])
                        ### 2011/05/17 make cluster insensitive to the line width
                        dummy.append([float(line[1] - line[0]) / CLUSTER_WHITEN, (line[0] + line[1]) / 2.0])
                        #dummy.append([float(line[1] - line[0]), (line[0] + line[1]) / 2.0])
                        ### 2011/05/13 Calculate median line width
                        Width.append(float(line[1] - line[0]) / CLUSTER_WHITEN)
##                 if (line[0] != line[1]) and ((len(ResultTable) == 0 and self.DataTable[row][DT_SFLAG] == 1) or len(ResultTable) != 0):
##                     Region.append([row, line[0], line[1], self.DetectSignal[row][0], self.DetectSignal[row][1], flag])
##                     ### 2011/05/17 make cluster insensitive to the line width
##                     dummy.append([float(line[1] - line[0]) / CLUSTER_WHITEN, (line[0] + line[1]) / 2.0])
##                     #dummy.append([float(line[1] - line[0]), (line[0] + line[1]) / 2.0])
##                     ### 2011/05/13 Calculate median line width
##                     Width.append(float(line[1] - line[0]) / CLUSTER_WHITEN)
        # Region2:[Width, Center]
        Region2 = NP.array(dummy)
        ### 2011/05/13 Calculate median line width
        MedianWidth = NP.median(NP.array(Width))
        #self.LogMessage('DEBUG', Origin=origin, Msg='dummy = %s' % dummy)
        #self.LogMessage('DEBUG', Origin=origin, Msg='Width = %s' % Width)
        del dummy, Width
        self.LogMessage('DEBUG', Origin=origin, Msg='MaxLines = %s' % MaxLines)
        self.LogMessage('DEBUG', Origin=origin, Msg='Npos = %s' % Npos)
        self.LogMessage('DEBUG', Origin=origin, Msg='MedianWidth = %s' % MedianWidth)
        # 2010/6/9 for non-detection
        if Npos == 0: return Lines

        # 2008/9/20 Dec Effect was corrected
        if self.USE_CASA_TABLE:
            DecCorrection = 1.0 / math.cos(tDEC[0] / 180.0 * 3.141592653)
        else:
            DecCorrection = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)
        #DecCorrection = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)
        GridSpaceRA *= DecCorrection
        # Calculate Parameters for Gridding
        if self.USE_CASA_TABLE:
            PosList = NP.array([NP.take(tRA,idxList),
                                NP.take(tDEC,idxList)])
        else:
            dummy = [[],[]]
            for row in idxList:
                dummy[0].append(self.DataTable[row][DT_RA])
                dummy[1].append(self.DataTable[row][DT_DEC])
            PosList = NP.array(dummy)
            del dummy
##         dummy = [[],[]]
##         for row in idxList:
##             dummy[0].append(self.DataTable[row][DT_RA])
##             dummy[1].append(self.DataTable[row][DT_DEC])
##         PosList = NP.array(dummy)
##         del dummy

        wra = PosList[0].max() - PosList[0].min()
        wdec = PosList[1].max() - PosList[1].min()
        cra = PosList[0].min() + wra/2.0
        cdec = PosList[1].min() + wdec/2.0
        # 2010/6/11 +1.0 -> +1.01: if wra is n x GridSpaceRA (n is a integer), int(wra/GridSpaceRA) is not n in some cases because of the lack of accuracy.
        nra = 2 * (int((wra/2.0 - GridSpaceRA/2.0)/GridSpaceRA) + 1) + 1
        ndec = 2 * (int((wdec/2.0 - GridSpaceDEC/2.0)/GridSpaceDEC) + 1) + 1
        x0 = cra - GridSpaceRA/2.0 - GridSpaceRA*(nra-1)/2.0
        y0 = cdec - GridSpaceDEC/2.0 - GridSpaceDEC*(ndec-1)/2.0
        self.LogMessage('DEBUG', Origin=origin, Msg='Grid = %d x %d\n' % (nra, ndec))

        # K-mean Clustering Analysis with LineWidth and LineCenter
        # Max number of protect regions are SDC.SDParam['Cluster']['MaxCluster'] (Max lines)
        ProcStartTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='K-mean Clustering Analaysis Start')
        #MaxCluster = int(min(ClusterRule['MaxCluster'], max(MaxLines + 1, (Npos ** 0.5)/2)))
        MaxCluster = int(ClusterRule['MaxCluster'])
        self.LogMessage('INFO', Origin=origin, Msg='Maximum number of clusters (MaxCluster) = %s' % MaxCluster)
        # Whiten is no more necessary 2007/2/12
        # whitened = VQ.whiten(Region2)
        TmpList = []
        # Determin the optimum number of clusters
        BestScore = -1.0
        # 2010/6/15 Plot the score along the number of the clusters
        ListNcluster = []
        ListScore = []
        ListBestScore = []
        #
        for Ncluster in range(1, MaxCluster + 1):
            tmpScore=[]
            # Fix the random seed 2008/5/23
            numpy.random.seed((1234,567))
            # Try multiple times to supress random selection effect 2007/09/04
            for Multi in range(min(Ncluster+1, 10)):
                #codebook, diff = VQ.kmeans(whitened, Ncluster)
                #codebook, diff = VQ.kmeans(Region2, Ncluster)
                codebook, diff = VQ.kmeans(Region2, Ncluster, iter=50)
                NclusterNew = 0
                # Do iteration until no merging of clusters to be found
                while(NclusterNew != len(codebook)):
                    NclusterNew = len(codebook)
                    category, distance = VQ.vq(Region2, codebook)
                    #self.LogMessage('INFO', Origin=origin, Msg='Cluster Category&Distance %s, distance = %s' % (category, distance))
                    for x in range(NclusterNew - 1, -1, -1):
                        # Remove a cluster without any members
                        if sum(NP.equal(category, x) * 1.0) == 0:
                            NclusterNew -= 1
                            tmp = list(codebook)
                            del tmp[x]
                            codebook = numpy.array(tmp)

                    # Clear Flag
                    for i in range(len(Region)): Region[i][5] = 1
                    # Nsigma clipping/flagging with cluster distance (set flag to 0)
                    Outlier = 0.0
                    MaxDistance = []
                    for Nc in range(NclusterNew):
                        ValueList = distance[(category == Nc).nonzero()[0]]
                        Stddev = ValueList.std()
                        ### 2011/05/17 Strict the threshold
                        Threshold = ValueList.mean() + Stddev * Nsigma
                        #Threshold = Stddev * Nsigma
                        self.LogMessage('DEBUG', Origin=origin, Msg='Cluster Clipping Threshold = %s, Stddev = %s' % (Threshold, Stddev))
                        ### 2011/05/17 clipping iteration
                        #ValueList = ValueList.take((ValueList < Threshold).nonzero())[0]
                        #Stddev = ValueList.std()
                        #Threshold = Stddev * Nsigma
                        del ValueList
                        #self.LogMessage('DEBUG', Origin=origin, Msg='Cluster Clipping Threshold = %s, Stddev = %s' % (Threshold, Stddev))
                        for i in ((distance * (category == Nc)) > Threshold).nonzero()[0]:
                            Region[i][5] = 0
                            Outlier += 1.0
                        MaxDistance.append(max(distance * ((distance < Threshold) * (category == Nc))))
                        self.LogMessage('LONG', Origin=origin, Msg='Region = %s' % Region)
                    MemberRate = (len(Region) - Outlier)/float(len(Region))
                    self.LogMessage('DEBUG', Origin=origin, Msg='MemberRate = %f' % MemberRate)

                    # Calculate Cluster Characteristics
                    Lines = []
                    for NN in range(NclusterNew):
                        LineCenterList = []
                        LineWidthList = []
                        for x in range(len(category)):
                            if category[x] == NN and Region[x][5] != 0:
                                LineCenterList.append(Region2[x][1])
                                LineWidthList.append(Region2[x][0])
                        Lines.append([NP.median(NP.array(LineCenterList)), NP.median(NP.array(LineWidthList)), True, MaxDistance[NN]])
                    self.LogMessage('DEBUG', Origin=origin, Msg='Lines = %s' % Lines)

                    # Rating
                    ### 2011/05/16 modified for line-width variation
                    #Score = ((distance+MedianWidth/2.0) * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) * (1.1 - MemberRate)
                    #Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) * (((1.0 - MemberRate)**0.5 + 1.0)**2.0)
                    ### 2011/05/12 modified for (distance==0)
                    Score = ((distance * numpy.transpose(numpy.array(Region))[5]).mean() + MedianWidth/2.0) * (NclusterNew+ 1.0/NclusterNew) * (((1.0 - MemberRate)**0.5 + 1.0)**2.0)
                    # 2007/09/10 More sensitive to the number of lines clipped out
                    #Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) * (((1.0 - MemberRate)**0.5 + 1.0)**2.0)
                    #Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) / MemberRate**2.0
                    # 2010/6/15 Plot the score along the number of the clusters
                    ListNcluster.append(Ncluster)
                    ListScore.append(Score)
                    tmpScore.append(Score)
                    self.LogMessage('DEBUG', Origin=origin, Msg='NclusterNew = %d, Score = %f' % (NclusterNew, Score))
                    if BestScore < 0:
                        BestNcluster = 1
                        BestScore = Score
                        BestCategory = category.copy()
                        BestCodebook = codebook.copy()
                        BestRegion = Region[:]
                        BestLines = Lines[:]
                    elif Score < BestScore:
                        BestNcluster = NclusterNew
                        BestScore = Score
                        BestCategory = category.copy()
                        BestCodebook = codebook.copy()
                        BestRegion = Region[:]
                        BestLines = Lines[:]
                
                TmpList.append([NclusterNew, Score, codebook])
            ListBestScore.append(min(tmpScore))
            self.LogMessage('INFO', Origin=origin, Msg='Ncluster = %d, BestScore = %f' % (NclusterNew, min(tmpScore)))
            if len(ListBestScore) > 3 and \
               ListBestScore[-4] <= ListBestScore[-3] and \
               ListBestScore[-4] <= ListBestScore[-2] and \
               ListBestScore[-4] <= ListBestScore[-1]:
                self.LogMessage('INFO', Origin=origin, Msg='Determined the Number of Clusters to be %d' % (BestNcluster))
                break

        Ncluster = BestNcluster
        Region = BestRegion
        category = BestCategory[:]
        Lines = []
        for x in range(Ncluster): Lines.append([BestCodebook[x][1], BestCodebook[x][0], True])
        #self.LogMessage('DEBUG', Origin=origin, Msg='Final: Ncluster = %s, Score = %s, Category = %s, CodeBook = %s, Lines = %s' % (Ncluster, BestScore, category, BestCodebook, Lines))
        ### 2011/05/17 Scaling back the line width
        for x in range(len(Region2)): Region2[x][0] *= CLUSTER_WHITEN
        # 2010/6/15 Plot the score along the number of the clusters
        SDP.ShowClusterScore(ListNcluster, ListScore, ShowPlot, FigFileDir, FigFileRoot)
        SDP.ShowClusterInChannelSpace(Region2, BestLines, CLUSTER_WHITEN, ShowPlot, FigFileDir, FigFileRoot)
        ProcEndTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Final: Ncluster = %s, Score = %s, Lines = %s' % (Ncluster, BestScore, Lines))
        self.LogMessage('INFO', Origin=origin, Msg='K-mean Cluster Analaysis End: Elapsed time = %.1f sec' % (ProcEndTime - ProcStartTime))

        # Sort Lines and Category by LineCenter: Lines[][0]
        LineCenter = []
        for Nc in range(Ncluster): LineCenter.append(BestLines[Nc][0])
        LineIndex = NP.argsort(NP.array(LineCenter))
        for Nc in range(Ncluster):
            Lines[Nc] = BestLines[LineIndex[Nc]]
            ### 2011/05/17 Scaling back the line width
            Lines[Nc][1] *= CLUSTER_WHITEN
        LineIndex2 = NP.argsort(LineIndex)
        for i in range(len(BestCategory)): category[i] = LineIndex2[BestCategory[i]]

        # Create Grid Parameter Space (Ncluster * nra * ndec)
        GridCluster = NP.zeros((Ncluster, nra, ndec), dtype=NP.float32)
        GridMember = NP.zeros((nra, ndec))

        ######## Clustering: Detection Stage ########
        ProcStartTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Clustering: Detection Stage Start')
        # Set Cluster on the Plane
        for row in ROWS:
            # Check statistics flag added by G.K. 2008/1/17
            # Bug fix 2008/5/29
            #if DataTable[row][DT_SFLAG] == 1:
            if self.USE_CASA_TABLE:
                if ((len(ResultTable) == 0 and tSFLAG[row] == 1) or len(ResultTable) != 0):
                    #print 'nra, ndec, row', nra, ndec, row
                    #print 'GridMember', int((self.DetectSignal[row][0] - x0)/GridSpaceRA)
                    #print 'GridMember', int((self.DetectSignal[row][1] - y0)/GridSpaceDEC)
                    GridMember[int((self.DetectSignal[row][0] - x0)/GridSpaceRA)][int((self.DetectSignal[row][1] - y0)/GridSpaceDEC)] += 1
            else:
                if ((len(ResultTable) == 0 and self.DataTable[row][DT_SFLAG] == 1) or len(ResultTable) != 0):
                    #print 'nra, ndec, row', nra, ndec, row
                    #print 'GridMember', int((self.DetectSignal[row][0] - x0)/GridSpaceRA)
                    #print 'GridMember', int((self.DetectSignal[row][1] - y0)/GridSpaceDEC)
                    GridMember[int((self.DetectSignal[row][0] - x0)/GridSpaceRA)][int((self.DetectSignal[row][1] - y0)/GridSpaceDEC)] += 1
##             if ((len(ResultTable) == 0 and self.DataTable[row][DT_SFLAG] == 1) or len(ResultTable) != 0):
##                 #print 'nra, ndec, row', nra, ndec, row
##                 #print 'GridMember', int((self.DetectSignal[row][0] - x0)/GridSpaceRA)
##                 #print 'GridMember', int((self.DetectSignal[row][1] - y0)/GridSpaceDEC)
##                 GridMember[int((self.DetectSignal[row][0] - x0)/GridSpaceRA)][int((self.DetectSignal[row][1] - y0)/GridSpaceDEC)] += 1
        for i in range(len(category)):
            if Region[i][5] == 1:
                try:
                    GridCluster[category[i]][int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] += 1.0
                except IndexError:
                    pass
        SDP.ShowCluster(GridCluster, [1.5, 0.5], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'detection', ShowPlot, FigFileDir, FigFileRoot)
        ProcEndTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Clustering: Detection Stage End: Elapsed time = %.1f sec' % (ProcEndTime - ProcStartTime))

        ######## Clustering: Validation Stage ########
        ProcStartTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Clustering: Validation Stage Start')
        # Create Space for storing the list of spectrum (ID) in the Grid
        Grid2SpectrumID = []
        for x in range(nra):
            Grid2SpectrumID.append([])
            for y in range(ndec):
                Grid2SpectrumID[x].append([])
        for i in range(len(idxList)):
            Grid2SpectrumID[int((PosList[0][i] - x0)/GridSpaceRA)][int((PosList[1][i] - y0)/GridSpaceDEC)].append(idxList[i])
        self.LogMessage('LONG', Origin=origin, Msg='Grid2SpectrumID = %s' % Grid2SpectrumID)
        # Effective if number of spectrum which contains feature belongs to the cluster is greater or equal to the half number of spectrum in the Grid
        for Nc in range(Ncluster):
            self.LogMessage('DEBUG', Origin=origin, Msg='Before: GridCluster[%s] = %s' % (Nc, GridCluster[Nc]))
            for x in range(nra):
                for y in range(ndec):
                    if GridMember[x][y] == 0: GridCluster[Nc][x][y] = 0.0
                    # if a single spectrum is inside the grid and has a feature belongs to the cluster, validity is set to 0.5 (for the initial stage) or 1.0 (iteration case).
                    elif GridMember[x][y] == 1 and GridCluster[Nc][x][y] > 0.9:
                        if ITER == 0: GridCluster[Nc][x][y] = 0.5
                        else: GridCluster[Nc][x][y] = 1.0
                    # if the size of population is enough large, validate it as a special case 2007/09/05
                    elif ITER == 0:
                        GridCluster[Nc][x][y] = max(min(GridCluster[Nc][x][y] / GridMember[x][y]**0.5 - 1.0, 3.0), GridCluster[Nc][x][y] / float(GridMember[x][y]))
                    else: GridCluster[Nc][x][y] = min(GridCluster[Nc][x][y] / GridMember[x][y]**0.5, 3.0)
                    #else: GridCluster[Nc][x][y] = max(min(GridCluster[Nc][x][y] / GridMember[x][y]**0.5 - 1.0, 3.0), GridCluster[Nc][x][y] / float(GridMember[x][y]))
                    # normarize validity
                    #else: GridCluster[Nc][x][y] /= float(GridMember[x][y])

            if ((GridCluster[Nc] > Questionable)*1).sum() == 0: Lines[Nc][2] = False
            self.LogMessage('DEBUG', Origin=origin, Msg='After:  GridCluster[%s] = %s' % (Nc, GridCluster[Nc]))
        SDP.ShowCluster(GridCluster, [Valid, Marginal, Questionable], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'validation', ShowPlot, FigFileDir, FigFileRoot)

        ProcEndTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Clustering: Validation Stage End: Elapsed time = %.1f sec' % (ProcEndTime - ProcStartTime))
        ######## Clustering: Smoothing Stage ########
        # Rating:  [0.0, 0.4, 0.5, 0.4, 0.0]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.5, 1.0, 6.0, 1.0, 0.5]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.0, 0.4, 0.5, 0.4, 0.0]
        # Rating = 1.0 / (Dx**2 + Dy**2)**(0.5) : if (Dx, Dy) == (0, 0) rating = 6.0

        ProcStartTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Clustering: Smoothing Stage Start')

        for Nc in range(Ncluster):
            if Lines[Nc][2] != False:
                GridScore = NP.zeros((2, nra, ndec), dtype=NP.float32)
                for x in range(nra):
                    for y in range(ndec):
                        for dx in [-2, -1, 0, 1, 2]:
                           for dy in [-2, -1, 0, 1, 2]:
                               if (abs(dx) + abs(dy)) <= 3:
                                   (nx, ny) = (x + dx, y + dy)
                                   if 0 <= nx < nra and 0 <= ny < ndec:
                                       if dx == 0 and dy == 0: Rating = 6.0
                                       else: Rating = 1.0 / (dx**2.0 + dy**2.0) ** 0.5
                                       GridScore[0][x][y] += Rating * GridCluster[Nc][nx][ny]
                                       GridScore[1][x][y] += Rating
                self.LogMessage('DEBUG', Origin=origin, Msg='Score :  GridScore[%s][0] = %s' % (Nc, GridScore[0]))
                self.LogMessage('DEBUG', Origin=origin, Msg='Rating:  GridScore[%s][1] = %s' % (Nc, GridScore[1]))
                GridCluster[Nc] = GridScore[0] / GridScore[1]
                del GridScore
            if ((GridCluster[Nc] > Questionable)*1).sum() < 0.1: Lines[Nc][2] = False
        SDP.ShowCluster(GridCluster, [Valid, Marginal, Questionable], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'smoothing', ShowPlot, FigFileDir, FigFileRoot)

        ProcEndTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Clustering: Smoothing Stage End: Elapsed time = %.1f sec' % (ProcEndTime - ProcStartTime))

        ######## Clustering: Final Stage ########
        ProcStartTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Clustering: Final Stage Start')

        HalfGrid = (GridSpaceRA ** 2 + GridSpaceDEC ** 2) ** 0.5 / 2.0
        # Clean isolated grids
        for Nc in range(Ncluster):
            #print '\nNc=', Nc
            if Lines[Nc][2] != False:
                Plane = (GridCluster[Nc] > Marginal) * 1
                if Plane.sum() == 0:
                    Lines[Nc][2] = False
                    #print 'Lines[Nc][2] -> False'
                    continue
                Original = GridCluster[Nc].copy()
                # Clear GridCluster Nc-th plane
                GridCluster[Nc] *= 0.0
                Nmember = []
                Realmember = []
                MemberList = []
                NsubCluster = 0
                for x in range(nra):
                    for y in range(ndec):
                        if Plane[x][y] == 1:
                            Plane[x][y] = 2
                            SearchList = [(x, y)]
                            M = 1
                            if Original[x][y] > Valid: MM = 1
                            #if Original[x][y] > Marginal: MM = 1
                            else: MM = 0
                            MemberList.append([(x, y)])
                            while(len(SearchList) != 0):
                                cx, cy = SearchList[0]
                                for dx in [-1, 0, 1]:
                                    for dy in [-1, 0, 1]:
                                        (nx, ny) = (cx + dx, cy + dy)
                                        if 0 <= nx < nra and 0 <= ny < ndec and Plane[nx][ny] == 1:
                                            Plane[nx][ny] = 2
                                            SearchList.append((nx, ny))
                                            M += 1
                                            if Original[nx][ny] > Valid: MM += 1
                                            #if Original[nx][ny] > Marginal: MM += 1
                                            MemberList[NsubCluster].append((nx, ny))
                                del SearchList[0]
                            Nmember.append(M)
                            Realmember.append(MM)
                            NsubCluster += 1

                self.LogMessage('DEBUG', Origin=origin, Msg='Nmember = %s' % Nmember)
                self.LogMessage('DEBUG', Origin=origin, Msg='MemberList = %s' % MemberList)
                # If no members left, skip to next cluster
                if len(Nmember) == 0: continue
                # Threshold is set to half the number of the largest cluster in the plane
                #Threshold = max(Nmember) / 2.0
                Threshold = min(max(Realmember) / 2.0, 3)
                for n in range(NsubCluster -1, -1, -1):
                    # isolated cluster made from single spectrum should be omitted
                    if Nmember[n] == 1:
                        (x, y) = MemberList[n][0]
                        if GridMember[x][y] <= 1:
                            Nmember[n] = 0
                            #print '\nHere Nmember[%d] = 1' % n
                    # Sub-Cluster whose member below the threshold is cleaned
                    if Nmember[n] < Threshold:
                        #print '\nNmember[%d]=%d, Threshold=%f' % (n, Nmember[n], Threshold)
                        for (x, y) in MemberList[n]:
                            Plane[x][y] == 0
                        del Nmember[n]
                        del MemberList[n]
                self.LogMessage('DEBUG', Origin=origin, Msg='Cluster Member = %s' % Nmember)

                # Blur each SubCluster with the radius of sqrt(Nmember/Pi) * ratio
                ratio = ClusterRule['BlurRatio']
                # Set-up SubCluster
                for n in range(len(Nmember)):
                    SubPlane = NP.zeros((nra, ndec), dtype=NP.float32)
                    xlist = []
                    ylist = []
                    for (x, y) in MemberList[n]:
                        SubPlane[x][y] = Original[x][y]
                        xlist.append(x)
                        ylist.append(y)
                    # Calculate Blur radius
                    #Blur = int((Realmember[n] / 3.141592653) ** 0.5 * ratio + 0.5)
                    BlurF = (Realmember[n] / 3.141592653) ** 0.5 * ratio + 1.5
                    Blur = int(BlurF)
                    self.LogMessage('DEBUG', Origin=origin, Msg='Blur = %d' % Blur)
                    # Set-up kernel for convolution
                    # caution: if nra < (Blur*2+1) and ndec < (Blur*2+1)
                    #  => dimension of SPC.convolve2d(Sub,kernel) gets not (nra,ndec) but (Blur*2+1,Blur*2+1)
                    if nra < (Blur * 2 + 1) and ndec < (Blur * 2 + 1): Blur = int((max(nra, ndec) - 1) / 2)
                    kernel = NP.zeros((Blur * 2 + 1, Blur * 2 + 1),dtype=int)
                    for x in range(Blur * 2 + 1):
                        for y in range(Blur * 2 + 1):
                            if math.sqrt((Blur - x)**2.0 + (Blur - y)**2.0) <= BlurF:
                                kernel[x][y] = 1
                    BlurPlane = (convolve2d(SubPlane, kernel) > Marginal) * 1
                    ValidPlane = (SubPlane > Valid) * 1
                    self.LogMessage('DEBUG', Origin=origin, Msg='kernel.shape = %s' % list(kernel.shape))
                    self.LogMessage('DEBUG', Origin=origin, Msg='GridCluster.shape = %s' % list(GridCluster.shape))
                    self.LogMessage('DEBUG', Origin=origin, Msg='Plane.shape = %s' % list(Plane.shape))
                    self.LogMessage('DEBUG', Origin=origin, Msg='SubPlane.shape = %s' % list(SubPlane.shape))
                    self.LogMessage('DEBUG', Origin=origin, Msg='BlurPlane.shape = %s' % list(BlurPlane.shape))
                    for x in range(len(Plane)):
                        self.LogMessage('DEBUG', Origin=origin, Msg=' %d : %s' % (x, list(Plane[x])))
                    for x in range(len(BlurPlane)):
                        self.LogMessage('DEBUG', Origin=origin, Msg=' %d : %s' % (x, list(BlurPlane[x])))
                    for x in range(len(ValidPlane)):
                        self.LogMessage('DEBUG', Origin=origin, Msg=' %d : %s' % (x, list(ValidPlane[x])))

                    # 2D fit for each Plane
                    # Use the data for fit if GridCluster[Nc][x][y] > Valid
                    # Not use for fit but apply the value at the border if GridCluster[Nc][x][y] > Marginal

                    # 2009/9/10 duplication of the position was taken into account (BroadComponent=True)
                    Bfactor = 1.0
                    if BroadComponent: Bfactor = 2.0
                    # Determine fitting order if not specified
                    #if Xorder < 0: Xorder0 = min(((NP.sum(ValidPlane, axis=0) > 0.5)*1).sum() - 1, 5)
                    #if Yorder < 0: Yorder0 = min(((NP.sum(ValidPlane, axis=1) > 0.5)*1).sum() - 1, 5)
                    if Xorder < 0: Xorder0 = int(min(((NP.sum(ValidPlane, axis=0) > 0.5)*1).sum()/Bfactor - 1, 5))
                    if Yorder < 0: Yorder0 = int(min(((NP.sum(ValidPlane, axis=1) > 0.5)*1).sum()/Bfactor - 1, 5))
                    #if Xorder < 0: Xorder0 = min(max(max(xlist) - min(xlist), 0), 5)
                    #if Yorder < 0: Yorder0 = min(max(max(ylist) - min(ylist), 0), 5)
                    self.LogMessage('DEBUG', Origin=origin, Msg='(X,Y)order = (%d, %d)' % (Xorder0, Yorder0))

                    # clear Flag
                    for i in range(len(category)): Region[i][5] = 1




                    if Xorder0 < 0 or Yorder0 < 0:
                        SingularMatrix = True
                        ExceptionLinAlg = False
                    else:
                        SingularMatrix = False
                        ExceptionLinAlg = True
                    while ExceptionLinAlg:
                        FitData = []
                        ### 2011/05/15 One parameter (Width, Center) for each spectra
                        #Region format:([row, line[0], line[1], self.DetectSignal[row][0], self.DetectSignal[row][1], flag])
                        dummy = []
                        for i in range(len(category)):
                            if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > Valid:
                                dummy.append((Region[i][0], Region[i][1], Region[i][2], Region[i][3], Region[i][4]))
                        i = 0
                        (Lrow, Lmin, Lmax, LRA, LDEC) = (dummy[i][0], dummy[i][1], dummy[i][2], dummy[i][3], dummy[i][4])
                        while(1):
                            i = i + 1
                            if i == len(dummy):
                                FitData.append([Lmin, Lmax, LRA, LDEC, 1])
                                break
                            elif Lrow == dummy[i][0]:
                                if Lmin > dummy[i][1]: Lmin = dummy[i][1]
                                if Lmax < dummy[i][2]: Lmax = dummy[i][2]
                            else:
                                FitData.append([Lmin, Lmax, LRA, LDEC, 1])
                                (Lrow, Lmin, Lmax, LRA, LDEC) = (dummy[i][0], dummy[i][1], dummy[i][2], dummy[i][3], dummy[i][4])
                        del dummy
                        for iteration in range(3):
                            self.LogMessage('DEBUG', Origin=origin, Msg='2D Fit Iteration = %d' % iteration)

                            ### Commented out three lines 2011/05/15
                            # FitData format: [Width, Center, RA, DEC]
                            #for i in range(len(category)):
                            #    if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > Valid:
                            #        FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))
                            if len(FitData) == 0 or SingularMatrix: break
                            self.LogMessage('DEBUG', Origin=origin, Msg='FitData = %s' % FitData)

                            # make arrays for coefficient calculation
                            # Matrix    MM x A = B  ->  A = MM^-1 x B
                            M0 = NP.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), dtype=NP.float64)
                            M1 = NP.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), dtype=NP.float64)
                            B0 = NP.zeros((Xorder0 + 1) * (Yorder0 + 1), dtype=NP.float64)
                            B1 = NP.zeros((Xorder0 + 1) * (Yorder0 + 1), dtype=NP.float64)
                            MM0 = NP.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], dtype=NP.float64)
                            MM1 = NP.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], dtype=NP.float64)
                            for (Width, Center, x, y, flag) in FitData:
                                if flag == 1:
                                    for k in range(Yorder0 * 2 + 1):
                                        for j in range(Xorder0 * 2 + 1):
                                            M0[j + k * (Xorder0 * 2 + 1)] += math.pow(x, j) * math.pow(y, k)
                                    for k in range(Yorder0 + 1):
                                        for j in range(Xorder0 + 1):
                                            B0[j + k * (Xorder0 + 1)] += math.pow(x, j) * math.pow(y, k) * Center
                                    for k in range(Yorder0 * 2 + 1):
                                        for j in range(Xorder0 * 2 + 1):
                                            M1[j + k * (Xorder0 * 2 + 1)] += math.pow(x, j) * math.pow(y, k)
                                    for k in range(Yorder0 + 1):
                                        for j in range(Xorder0 + 1):
                                            B1[j + k * (Xorder0 + 1)] += math.pow(x, j) * math.pow(y, k) * Width

                            # make Matrix MM0,MM1 and calculate A0,A1
                            for K in range((Xorder0 + 1) * (Yorder0 + 1)):
                                k0 = K % (Xorder0 + 1)
                                k1 = int(K / (Xorder0 + 1))
                                for J in range((Xorder0 + 1) * (Yorder0 + 1)):
                                    j0 = J % (Xorder0 + 1)
                                    j1 = int(J / (Xorder0 + 1))
                                    MM0[J, K] = M0[j0 + k0 + (j1 + k1) * (Xorder0 * 2 + 1)]
                                    MM1[J, K] = M1[j0 + k0 + (j1 + k1) * (Xorder0 * 2 + 1)]
                            self.LogMessage('DEBUG', Origin=origin, Msg='MM0 = %s' % MM0)
                            self.LogMessage('DEBUG', Origin=origin, Msg='B0 = %s' % B0)
                            ExceptionLinAlg = False
                            try:
                                A0 = LA.solve(MM0, B0)
                                A1 = LA.solve(MM1, B1)
                            #except LinAlgError:
                            except:
                                if Xorder0 != 0 or Yorder0 != 0:
                                    ExceptionLinAlg = True
                                    self.LogMessage('DEBUG', Origin=origin, Msg='Xorder0,Yorder0 = %s,%s' % (Xorder0, Yorder0))
                                    Xorder0 = max(Xorder0 - 1, 0)
                                    Yorder0 = max(Yorder0 - 1, 0)
                                    self.LogMessage('DEBUG', Origin=origin, Msg='Exception Raised: Xorder0,Yorder0 = %s,%s' % (Xorder0, Yorder0))
                                else:
                                    SingularMatrix = True
                                    self.LogMessage('DEBUG', Origin=origin, Msg='SingularMatrix = True')
                                break

                            self.LogMessage('DEBUG', Origin=origin, Msg='A0 = %s' % A0)
                            self.LogMessage('DEBUG', Origin=origin, Msg='M[0] = %s' % M0[0])

                            # Calculate Sigma
                            # Sigma should be calculated in the upper stage
                            # Fit0: Center or Lmax, Fit1: Width or Lmin
                            Diff = []
                            Number = []
                            for (Width, Center, x, y, flag) in FitData:
                                if flag == 1:
                                    Fit0 = Fit1 = 0.0
                                    for k in range(Yorder0 + 1):
                                        for j in range(Xorder0 + 1):
                                            Coef = math.pow(x, j) * math.pow(y, k)
                                            Fit0 += Coef * A0[j + k * (Xorder0 + 1)]
                                            Fit1 += Coef * A1[j + k * (Xorder0 + 1)]
                                    Diff.append(((Fit0 - Center)**2.0 + (Fit1 - Width)**2.0)**0.5)
                            if len(Diff) > 1: Threshold = NP.array(Diff).mean() + NP.array(Diff).std() * Nsigma
                            #if len(Diff) > 1: Threshold = NP.array(Diff).std() * Nsigma
                            else: Threshold *= 2.0
                            self.LogMessage('DEBUG', Origin=origin, Msg='Diff = %s' % Diff)
                            self.LogMessage('DEBUG', Origin=origin, Msg='2D Fit Threshold = %s' % Threshold)

                            # Sigma Clip
                            NFlagged = 0
                            Number = 0
                            ### 2011/05/15
                            for i in range(len(FitData)):
                                #self.LogMessage('DEBUG', Origin=origin, Msg='i = %s' % i)
                                (Width, Center, x, y, flag) = FitData[i]
                                #self.LogMessage('DEBUG', Origin=origin, Msg='Width, Center, x, y, flag = %s, %s, %s, %s, %s' % (Width, Center, x, y, flag))
                                Number += 1
                                Fit0 = Fit1 = 0.0
                                for k in range(Yorder0 + 1):
                                    for j in range(Xorder0 + 1):
                                        Coef = math.pow(x, j) * math.pow(y, k)
                                        Fit0 += Coef * A0[j + k * (Xorder0 + 1)]
                                        Fit1 += Coef * A1[j + k * (Xorder0 + 1)]
                                if ((Fit0 - Center)**2.0 + (Fit1 - Width)**2.0)**0.5 <= Threshold:
                                    FitData[i][4] = 1
                                else:
                                    FitData[i][4] = 0
                                    NFlagged += 1

                            self.LogMessage('DEBUG', Origin=origin, Msg='2D Fit Flagged/All = (%s, %s)' % (NFlagged, Number))
                            #2009/10/15 compare the number of the remainder and fitting order
                            if (Number - NFlagged) <= max(Xorder0, Yorder0) or Number == NFlagged:
                                SingularMatrix = True
                                self.LogMessage('DEBUG', Origin=origin, Msg='SingularMatrix = True')
                                break
                    # Iteration End
                    ### 2011/05/15 Fitting is no longer (Width, Center) but (minChan, maxChan)

                    # FitData: [(Width, Center, RA, DEC)]
                    if not SingularMatrix:
                        FitData = []
                        for i in range(len(category)):
                            if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > Valid:
                                FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))
                        if len(FitData) == 0: continue

                        # Calculate Fit for each position
                        for x in range(nra):
                            for y in range(ndec):
                                if ValidPlane[x][y] == 1:
                                    for ID in Grid2SpectrumID[x][y]:
                                        ### 2011/05/15 (Width, Center) -> (minChan, maxChan)
                                        Chan0 = Chan1 = 0.0
                                        PID = idxList.index(ID)
                                        for k in range(Yorder0 + 1):
                                            for j in range(Xorder0 + 1):
                                                Coef = math.pow(PosList[0][PID], j) * math.pow(PosList[1][PID], k)
                                                Chan1 += Coef * A0[j + k * (Xorder0 + 1)]
                                                Chan0 += Coef * A1[j + k * (Xorder0 + 1)]
                                        Fit0 = (Chan0 + Chan1) / 2.0
                                        Fit1 = (Chan1 - Chan0) + 1.0
                                        self.LogMessage('DEBUG', Origin=origin, Msg='Fit0, Fit1 = %s, %s' % (Fit0, Fit1))
                                        if (Fit1 >= MinFWHM) and (Fit1 <= MaxFWHM):
                                            # Allowance = Fit1 / 2.0 * 1.3
                                            # To keep broad line region, make allowance larger
                                            ### 2011/05/16 allowance + 0.5 ->  2.5 for sharp line
                                            ### 2011/05/16 factor 1.5 -> 2.0 for broad line
                                            #Allowance = min(Fit1 / 2.0 * 2.0, MaxFWHM / 2.0)
                                            ### 2011/11/22 Allowance is too narrow for new line finding algorithm
                                            Allowance = min(Fit1 + 5.0, MaxFWHM / 2.0)
                                            ### 2011/10/21 left side mask exceeded nChan
                                            Protect = [min(max(int(Fit0 - Allowance), 0), nChan - 1), min(int(Fit0 + Allowance), nChan - 1)]
                                            #Allowance = Fit1 / 2.0 * 1.5
                                            #Protect = [max(int(Fit0 - Allowance - 0.5), 0), min(int(Fit0 + Allowance + 0.5), nChan - 1)]
                                            self.LogMessage('DEBUG', Origin=origin, Msg='0 Allowance = %s Protect = %s' % (Allowance, Protect))
                                            if RealSignal.has_key(ID):
                                                tmplist = RealSignal[ID][2]
                                                tmplist.append(Protect)
                                                RealSignal[ID][2] = tmplist
                                            else:
                                                if self.USE_CASA_TABLE:
                                                    RealSignal[ID] = [tRA[ID], tDEC[ID] ,[Protect]]
                                                else:
                                                    RealSignal[ID] = [self.DataTable[ID][DT_RA], self.DataTable[ID][DT_DEC] ,[Protect]]
                                                #RealSignal[ID] = [self.DataTable[ID][DT_RA], self.DataTable[ID][DT_DEC] ,[Protect]]
                                elif BlurPlane[x][y] == 1:
                                    # in Blur Plane, Fit is not extrapolated, but use the nearest value in Valid Plane
                                    # Search the nearest Valid Grid
                                    Nearest = []
                                    Dist2 = []
                                    for xx in range(nra):
                                        for yy in range(ndec):
                                            if ValidPlane[xx][yy] == 1:
                                                Dist2.append(((xx - x)*GridSpaceRA)**2 + ((yy - y)*GridSpaceDEC)**2)
                                                Nearest.append([xx, yy])
                                    ID = NP.argmin(NP.array(Dist2))
                                    (RA0, DEC0) = (x0 + GridSpaceRA * (x + 0.5), y0 + GridSpaceDEC * (y + 0.5))
                                    (RA1, DEC1) = (x0 + GridSpaceRA * (Nearest[ID][0] + 0.5), y0 + GridSpaceDEC * (Nearest[ID][1] + 0.5))

                                    # Setup the position near the border
                                    RA2 = RA1 - (RA1 - RA0) * HalfGrid / (Dist2[ID] ** 0.5)
                                    DEC2 = DEC1 - (DEC1 - DEC0) * HalfGrid / (Dist2[ID] ** 0.5)
                                    self.LogMessage('DEBUG', Origin=origin, Msg='[X,Y],[XX,YY] = [%d,%d],%s' % (x,y,Nearest[ID]))
                                    self.LogMessage('DEBUG', Origin=origin, Msg='(RA0,DEC0),(RA1,DEC1),(RA2,DEC2) = (%.5f,%.5f),(%.5f,%.5f),(%.5f,%.5f)' % (RA0,DEC0,RA1,DEC1,RA2,DEC2))
                                    # Calculate Fit and apply same value to all the spectra in the Blur Grid
                                    ### 2011/05/15 (Width, Center) -> (minChan, maxChan)
                                    Chan0 = Chan1 = 0.0
                                    for k in range(Yorder0 + 1):
                                        for j in range(Xorder0 + 1):
                                            # Border case
                                            #Coef = math.pow(RA2, j) * math.pow(DEC2, k)
                                            # Center case
                                            Coef = math.pow(RA1, j) * math.pow(DEC1, k)
                                            Chan1 += Coef * A0[j + k * (Xorder0 + 1)]
                                            Chan0 += Coef * A1[j + k * (Xorder0 + 1)]
                                    Fit0 = (Chan0 + Chan1) / 2.0
                                    Fit1 = (Chan1 - Chan0)
                                    self.LogMessage('DEBUG', Origin=origin, Msg='Fit0, Fit1 = %s, %s' % (Fit0, Fit1))
                                    if (Fit1 >= MinFWHM) and (Fit1 <= MaxFWHM):
                                        #Allowance = Fit1 / 2.0 * 1.3
                                        # To keep broad line region, make allowance larger
                                        ### 2011/05/16 allowance + 0.5 ->  2.5 for sharp line
                                        ### 2011/05/16 factor 1.5 -> 2.0 for broad line
                                        #Allowance = min(Fit1 / 2.0 * 2.0, MaxFWHM / 2.0)
                                        ### 2011/11/22 Allowance is too narrow for new line finding algorithm
                                        Allowance = min(Fit1 + 5.0, MaxFWHM / 2.0)
                                        ### 2011/10/21 left side mask exceeded nChan
                                        Protect = [min(max(int(Fit0 - Allowance), 0), nChan - 1), min(int(Fit0 + Allowance), nChan - 1)]
                                        #Allowance = Fit1 / 2.0 * 1.5
                                        #Protect = [max(int(Fit0 - Allowance + 0.5), 0), min(int(Fit0 + Allowance + 0.5), nChan - 1)]

                                        self.LogMessage('DEBUG', Origin=origin, Msg='1 Allowance = %s Protect = %s' % (Allowance, Protect))
                                        for ID in Grid2SpectrumID[x][y]:
                                            if RealSignal.has_key(ID):
                                                tmplist = RealSignal[ID][2]
                                                tmplist.append(Protect)
                                                RealSignal[ID][2] = tmplist
                                            else:
                                                if self.USE_CASA_TABLE:
                                                    RealSignal[ID] = [tRA[ID], tDEC[ID] ,[Protect]]
                                                else:
                                                    RealSignal[ID] = [self.DataTable[ID][DT_RA], self.DataTable[ID][DT_DEC] ,[Protect]]
                                                #RealSignal[ID] = [self.DataTable[ID][DT_RA], self.DataTable[ID][DT_DEC] ,[Protect]]
                                    else: continue
                    # for Plot
                    if not SingularMatrix: GridCluster[Nc] += BlurPlane
                if ((GridCluster[Nc] > 0.5)*1).sum() < Questionable: Lines[Nc][2] = False
                for x in range(nra):
                    for y in range(ndec):
                        if GridCluster[Nc][x][y] > 0.5: GridCluster[Nc][x][y] = 1.0
                        if Original[x][y] > Valid: GridCluster[Nc][x][y] = 2.0
        SDP.ShowCluster(GridCluster, [1.5, 0.5, 0.5, 0.5], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'regions', ShowPlot, FigFileDir, FigFileRoot)
        ProcEndTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Clustering: Final Stage End: Elapsed time = %.1f sec' % (ProcEndTime - ProcStartTime))

        # Merge masks if possible
        for row in idxList:
            if RealSignal.has_key(row):
                if len(RealSignal[row][2]) != 1:
                    Region = NP.ones(nChan + 2, dtype=int)
                    for [chan0, chan1] in RealSignal[row][2]:
                        Region[chan0 + 1:chan1 + 1] = 0
                    dummy = (Region[1:] - Region[:-1]).nonzero()
                    RealSignal[row][2] = []
                    for y in range(0, len(dummy[0]), 2):
                        RealSignal[row][2].append([dummy[0][y], dummy[0][y + 1]])
            else:
                if self.USE_CASA_TABLE:
                    RealSignal[row] = [tRA[row], tDEC[row], [[-1, -1]]]
                else:
                    RealSignal[row] = [self.DataTable[row][DT_RA], self.DataTable[row][DT_DEC], [[-1, -1]]]
                #RealSignal[row] = [self.DataTable[row][DT_RA], self.DataTable[row][DT_DEC], [[-1, -1]]]
            if self.USE_CASA_TABLE:
                tMASKLIST = self.DataTable.getcell('MASKLIST',row)
                tNOCHANGE = self.DataTable.getcell('NOCHANGE',row)
                self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, RealSignal = %s' % (tMASKLIST, RealSignal[row][2]))
                if tMASKLIST == RealSignal[row][2]:
                    if type(tNOCHANGE) != int:
                        self.DataTable.putcell('NOCHANGE',row,ITER - 1)
                else:
                    self.DataTable.putcell('MASKLIST',row,RealSignal[row][2])
                    self.DataTable.putcell('NOCHANGE',row,False)
            else:
                self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, RealSignal = %s' % (self.DataTable[row][DT_MASKLIST], RealSignal[row][2]))
                if self.DataTable[row][DT_MASKLIST] == RealSignal[row][2]:
                    if type(self.DataTable[row][DT_NOCHANGE]) != int:
                        self.DataTable[row][DT_NOCHANGE] = ITER - 1
                else:
                    self.DataTable[row][DT_MASKLIST] = RealSignal[row][2]
                    self.DataTable[row][DT_NOCHANGE] = False
##             self.LogMessage('DEBUG', Origin=origin, Msg='DataTable = %s, RealSignal = %s' % (self.DataTable[row][DT_MASKLIST], RealSignal[row][2]))
##             if self.DataTable[row][DT_MASKLIST] == RealSignal[row][2]:
##                 if type(self.DataTable[row][DT_NOCHANGE]) != int:
##                     self.DataTable[row][DT_NOCHANGE] = ITER - 1
##             else:
##                 self.DataTable[row][DT_MASKLIST] = RealSignal[row][2]
##                 self.DataTable[row][DT_NOCHANGE] = False

        del GridCluster, RealSignal

        return Lines


    @dec_engines_logfile
    @dec_engines_logging
    def Process5(self, DataIn, TimeTableList, Idx, NCHAN, edge=(0, 0), FitOrderRule=None, LogLevel=2, LogFile=False):
        """
        Determine Baseline-Fitting order for each time-bin
         DataTable[Idx][21] = [[LineStartChannel1, LineEndChannel1],
                               [LineStartChannel2, LineEndChannel2],
                               [LineStartChannelN, LineEndChannelN]]
         RealSignal = {ID1,[RA,DEC,[[LineStartChannel1, LineEndChannel1],
                                    [LineStartChannel2, LineEndChannel2],
                                    [LineStartChannelN, LineEndChannelN]]],
                       IDn,[RA,DEC,[[LineStartChannel1, LineEndChannel1],
                                    [LineStartChannelN, LineEndChannelN]]]}
        """
        origin = 'Process5()'
        
        # NROW is only necessary for messages
        NROW = len(Idx)
        output = []

        self.LogMessage('INFO', Origin=origin, Msg='Determine Baseline-Fitting order for each time-bin...')
        self.LogMessage('INFO', Origin=origin, Msg='Applied time bin: %s' % FitOrderRule['ApplicableDuration'])
        self.LogMessage('INFO', Origin=origin, Msg='Processing %d spectra...' % NROW)

        # 2012/03/06 Fixed order case: FitOrderRule['PolynomialOrder'] = int
        if type(FitOrderRule['PolynomialOrder']) == int:
            fixedOrder = FitOrderRule['PolynomialOrder']
            self.LogMessage('INFO', Origin=origin, Msg='Baseline-Fitting order was fixed to be %d' % fixedOrder)
            for idx in Idx:
                output.append([idx, fixedOrder])
            return dict(output)

        self.LogMessage('INFO', Origin=origin, Msg='Baseline-Fitting order was automatically determined')
        # Select time gap list: 'subscan': large gap; 'raster': small gap
        if FitOrderRule['ApplicableDuration'] == 'subscan':
            TimeTable = TimeTableList[1]
        else:
            TimeTable = TimeTableList[0]

        # Set edge mask region
        (EdgeL, EdgeR) = parseEdge(edge)
        EffectiveNCHAN = NCHAN - EdgeR - EdgeL

        TGroup = TimeTable

        # for getting spectra
        SpOned = NP.zeros(NCHAN, dtype=NP.float32)
        self._tb.open(DataIn)

        # Create progress timer
        Timer = ProgressTimer(80, NROW, LogLevel)
        for y in range(len(TGroup)):
            SpFFT = []
            for index in range(len(TGroup[y][0])):
                row = TGroup[y][0][index]
                idx = TGroup[y][1][index]
                # Countup progress timer
                Timer.count()
                # Array to store spectrum
                SpOned = self._tb.getcell('SPECTRA', row)
                mask = NP.ones(NCHAN)
                mask[:EdgeL] = 0
                if EdgeR > 0: mask[-EdgeR:] = 0
                if self.USE_CASA_TABLE:
                    masklist = self.DataTable.getcell('MASKLIST',idx)
                else:
                    masklist = self.DataTable[idx][DT_MASKLIST]
                for line in masklist:
                #for line in self.DataTable[idx][DT_MASKLIST]:
                    if line[0] != -1:
                        mask[line[0]:line[1]] = 0
                ave = (SpOned * mask).sum() / float(mask.sum())
                SpOned = (SpOned - ave) * mask
                del mask

                # Apply FFT to the spectra
                SpFFT.append(NP.abs(FFTP.rfft(SpOned)))
            #print 'len(SpFFT) =', len(SpFFT)
            # Average seems to be better than median
            #Power = NML.median(SpFFT)
            Power = NP.average(SpFFT,axis=0)
            N = int(MaxDominantFreq * EffectiveNCHAN / 2048.0)
            if N==0:
                N=1
            # 2007/09/01 Absolute value of power should be taken into account
            # If the power is low, it should be ignored
            # Normalize the power
            Power2 = Power / Power.mean()
            MaxPower = Power2[:N].max()
            if MaxPower < 3.0: Order = 1.0
            elif MaxPower < 5.0: Order = 1.5
            elif MaxPower < 10.0: Order = 2.0
            else:
                flag = False
                for i in range(N, -1, -1):
                    if Power2[i] > 10.0: break
                    if Power2[i] > 5.0: flag = True
                if flag == True: Order = float(max(2.0, i)) + 0.5
                else: Order = float(max(2.0, i))

            self.LogMessage('DEBUG', Origin=origin, Msg='Power= %s' % (Power2[:N+1]))
            for idx in TGroup[y][1]:
                output.append([idx, Order])
                if self.USE_CASA_TABLE:
                    self.LogMessage('DEBUG', Origin=origin, Msg='Time bin = %s, Number of nodes determined from Power spectrum = %s' % (self.DataTable.getcell('ROW',idx), Order))
                else:
                    self.LogMessage('DEBUG', Origin=origin, Msg='Time bin = %s, Number of nodes determined from Power spectrum = %s' % (self.DataTable[idx][DT_ROW], Order))
                #self.LogMessage('DEBUG', Origin=origin, Msg='Time bin = %s, Number of nodes determined from Power spectrum = %s' % (self.DataTable[idx][DT_ROW], Order))
                

        del SpOned, Timer
        self._tb.close()
        #print 'TGroup', TGroup
        #print 'output', output
        return dict(output)


    @dec_engines_logfile
    @dec_engines_logging
    def Process6(self, DataIn, DataOut, DataCont, Idx, NCHAN, DictFitOrder, FitFunc, vIF, TimeGapList=[[],[]], edge=(0, 0), FitOrderRule=None, LogLevel=2, LogFile=False, FigFileDir=False, FigFileRoot=False, productTable=None):
        """
        if DictFitOrder is an integer number rather than a dictionary, then fitting order is fixed to be the number
        """

        origin = 'Process6()'

        # MaxPolynomialOrder = 'none' or any integer
        MaxPolynomialOrder = FitOrderRule['MaxPolynomialOrder']
        TimeGap = TimeGapList[1]
        PosGap = TimeGapList[0]
        tbName = DataOut+'.baseline.table'
        if productTable is None:
            outTbl = DataOut+'.product.tbl'
        else:
            outTbl = productTable

        # clean-up baseline table (text)
        if os.path.exists(tbName):
            os.system('rm '+tbName)

        # 2010/6/12 interactive mode was deleted
        #if stepbystep: showevery = 1
        #if ShowRMS: SDP.ShowResult(mode='init', title='Statistics Plot: Baseline RMS for Raw and Processed data')

        # 2009/10/29 Create dummy scantable
        # Bugs in scantable: sometimes 'RuntimeError' occurs
        for index in Idx:
            try:
##                 dummyscan = sd.scantable(DataIn, average=False).get_row(self.DataTable[index][DT_ROW], insitu=False).copy()
                # make memory table
                tmpname = 'heuristics.temporary.table'
                if self.USE_CASA_TABLE:
                    sd.scantable(DataIn, average=False).get_row(self.DataTable.getcell('ROW',index), insitu=False).save(tmpname,overwrite=True)
                else:
                    sd.scantable(DataIn, average=False).get_row(self.DataTable[index][DT_ROW], insitu=False).save(tmpname,overwrite=True)
                #sd.scantable(DataIn, average=False).get_row(self.DataTable[index][DT_ROW], insitu=False).save(tmpname,overwrite=True)
                sd.rcParams['scantable.storage']='memory'
                dummyscan = sd.scantable(tmpname,False).copy()
                sd.rcParams['scantable.storage']='disk'
                os.system('rm -rf %s'%(tmpname))
                break
            except RuntimeError:
                print 'RuntimeError occured at row = %d: retry....' % self.DataTable[index][DT_ROW]
                continue

        ########## Baseline fit for each spectrum ##########
        # Initialize variables
        Rows = []
        NROW = len(Idx)
        SpOned = NP.zeros(NCHAN, dtype=NP.float32)
        FitResult = NP.zeros(NCHAN, dtype=NP.float32)
        #tbtool = casac.homefinder.find_home_by_name('tableHome')
        #tbIn = tbtool.create()
        #tbOut = tbtool.create()
        tbIn,tbOut,tbCont,tbTBL = gentools(['tb','tb','tb','tb'])
        tbIn.open(DataIn)
        tbOut.open(DataOut, nomodify=False)

        # for Continuum Observation 2010/10/25 GK
        if DataCont != None:
            #tbCont = tbtool.create()
            tbCont.open(DataCont, nomodify=False)
            Continuum = NP.zeros(NCHAN, dtype=NP.float32)

        # Set edge mask region
        (EdgeL, EdgeR) = parseEdge(edge)

        self.LogMessage('INFO', Origin=origin, Msg='Baseline Fit: background subtraction...')
        self.LogMessage('INFO', Origin=origin, Msg='Processing %d spectra...' % NROW)

        # Create progress timer
        Timer = ProgressTimer(80, NROW, LogLevel)
        # for multi-section polynomial fit
        Nwin = []
        #for row in rows:
        
        tROW = None
        if self.USE_CASA_TABLE:
            #tROW = NP.take(self.DataTable.getcol('ROW'),Idx)
            tROW = self.DataTable.getcol('ROW')
            
        for idx in Idx:
            if self.USE_CASA_TABLE:
                row = int(tROW[idx])
            else:
                row = self.DataTable[idx][DT_ROW]
            # Countup progress timer
            Timer.count()
            self.LogMessage('DEBUG', Origin=origin, Msg='=' * 30 + 'Processing at row = %s' % row  + '=' * 30)
            Rows.append(row)
            if self.USE_CASA_TABLE:
                tNOCHANGE = self.DataTable.getcell('NOCHANGE',idx)
            else:
                tNOCHANGE = self.DataTable[idx][DT_NOCHANGE]
            self.LogMessage('DEBUG', Origin=origin, Msg='row = %s, Flag = %s' % (row, tNOCHANGE))
            #self.LogMessage('DEBUG', Origin=origin, Msg='row = %s, Flag = %s' % (row, self.DataTable[idx][DT_NOCHANGE]))
            if type(tNOCHANGE) != int:
            #if type(self.DataTable[idx][DT_NOCHANGE]) != int:
                SpOned = tbIn.getcell('SPECTRA', row)
                SpOned[:EdgeL] = 0
                if EdgeR > 0: SpOned[-EdgeR:] = 0
                dummyscan._setspectrum(NP.array(SpOned, dtype=NP.float64))
                # 2012/03/06 DictFitOrder can be a integer rather than a dict
                if type(DictFitOrder) == dict:
                    #polyorder = int(DictFitOrder[row] * 3.0)
                    #polyorder = int((DictFitOrder[row] + 1) * 2.0 + 0.5)
                    polyorder = int(DictFitOrder[idx] * 2.0 + 1.5)
                    # 2008/9/23 to limit maximum polyorder by user input
                    if MaxPolynomialOrder != 'none':
                        polyorder = min(MaxPolynomialOrder, polyorder)
                else: polyorder = DictFitOrder

                maxwidth = 1
                if self.USE_CASA_TABLE:
                    tMASKLIST = self.DataTable.getcell('MASKLIST',idx)
                else:
                    tMASKLIST = self.DataTable[idx][DT_MASKLIST]
                for [Chan0, Chan1] in tMASKLIST:
                #for [Chan0, Chan1] in self.DataTable[idx][DT_MASKLIST]:
                    if Chan1 - Chan0 >= maxwidth: maxwidth = int((Chan1 - Chan0 + 1) / 1.4)
                # allowance in Process3 is 1/5: (1 + 1/5 + 1/5)^(-1) = (5/7)^(-1) = 7/5 = 1.4
                maxpolyorder = int((NCHAN - EdgeL - EdgeR) / maxwidth + 1)

                self.LogMessage('DEBUG', Origin=origin, Msg='Masked Region from previous processes = %s' % tMASKLIST)
                #self.LogMessage('DEBUG', Origin=origin, Msg='Masked Region from previous processes = %s' % self.DataTable[idx][DT_MASKLIST])
                self.LogMessage('DEBUG', Origin=origin, Msg='edge parameters= (%s,%s)' % (EdgeL, EdgeR))
                self.LogMessage('DEBUG', Origin=origin, Msg='Polynomial order = %d  Max Polynomial order = %d' % (polyorder, maxpolyorder))
                (FitResult, nmask, win_polyorder, fragment, nwindow) = self.CalcBaselineFit(dummyscan, idx, min(polyorder, maxpolyorder), NCHAN, 0, FitFunc, (EdgeL, EdgeR), blfile=tbName, LogLevel=LogLevel, LogFile=LogFile)
                Nwin.append(nwindow)
                tbOut.putcell('SPECTRA', row, FitResult)
                #self._tb.putcell('SPECTRA', row, FitResult)
                # for Continuum Observation 2010/10/25 GK
                if DataCont != None:
                    # try Case 1
                    #Continuum[index] = SpStorage[index] - FitResult
                    # try Case 2
                    Continuum = SpOned.copy()
                    for [Chan0, Chan1] in tMASKLIST:
                    #for [Chan0, Chan1] in self.DataTable[idx][DT_MASKLIST]:
                        Continuum[Chan0:Chan1] = Continuum[Chan0:Chan1] - FitResult[Chan0:Chan1]
                    Continuum[:EdgeL] = 0.0
                    if EdgeR > 0: Continuum[-EdgeR:] = 0.0
                    tbCont.putcell('SPECTRA', row, Continuum)
                    del Continuum
                #self.DataTable[idx][DT_STAT][1] = NewRMS
                #self.DataTable[idx][DT_STAT][2] = OldRMS
            else: continue
        tbOut.flush()
        tbOut.close()
        tbIn.close()
        # Store Continuum Data 2010/10/25 GK
        if DataCont != None:
            tbCont.flush()
            tbCont.close()

        # Read text table and store values into casa table
        #StartTime = time.time()
        if os.path.exists(tbName):
            TableTxt = open(tbName, 'r')
            # CASA3.4
            i = 0
            Pscan, Pbeam, Pif, Ppol, Pcycle, Prms, Psec, Pprm = [], [], [], [], [], [], [], []
            if FitFunc.upper() == 'SPLINE':
                fitFunc = 'spline'
                while 1:
                    line = TableTxt.readline()
                    if line == '': break
                    elif line.find('p0') != -1:
                        ParamSW = line.replace(' ','').replace('[','').replace(']','=').replace(',','=').split('=')
                        if len(Psec) == i:
                            Psec.append([[[int(ParamSW[0])], [int(ParamSW[1])]]])
                            Pprm.append([[[float(ParamSW[3])], [float(ParamSW[5])], [float(ParamSW[7])], [float(ParamSW[9])]]])
                        else:
                            Psec[i].append([[int(ParamSW[0])], [int(ParamSW[1])]])
                            Pprm[i].append([[float(ParamSW[3])], [float(ParamSW[5])], [float(ParamSW[7])], [float(ParamSW[9])]])
                    elif line[:5] == '-----': i+=1
                    elif line.find('Scan') != -1:
                        ParamSW = line.replace(' ','').replace('[','@').replace(']','@').split('@')
                        Pscan.append(int(ParamSW[1]))
                        Pbeam.append(int(ParamSW[3]))
                        Pif.append(int(ParamSW[5]))
                        Ppol.append(int(ParamSW[7]))
                        Pcycle.append(int(ParamSW[9]))
                    #elif line.find('rms') != -1:
                    #    Prms.append([[float(line.replace(' ','').split('=')[1])]])
            else:
                fitFunc = 'polynomial'
                nwin0 = Nwin[0]
                flag = True
                while 1:
                    line = TableTxt.readline()
                    if line == '': break
                    elif line.find('Fitter') != -1:
                        ParamSW = line.replace(' ','').replace('[','').replace(']','').replace(',','=').split('=')
                        if flag: Psec.append([])
                        PP = [[float(ParamSW[1])],[float(ParamSW[-1])]]
                        Psec[i].append(PP)
                    elif line.find('p0') != -1:
                        ParamSW = line.replace(' ','').replace(',','=').split('=')
                        if flag: Pprm.append([])
                        PP = []
                        for j in range(1, len(ParamSW), 2):
                            PP.append([float(ParamSW[j])])
                        Pprm[i].append(PP)
                    elif line[:5] == '-----':
                        nwin0 -= 1
                        if nwin0 == 0:
                            i+=1
                            if i < len(Nwin): nwin0 = Nwin[i]
                            #else: print 'Nwin index over size!!!'
                            flag = True
                        else: flag = False
                    elif line.find('Scan') != -1 and flag:
                        ParamSW = line.replace(' ','').replace('[','@').replace(']','@').split('@')
                        Pscan.append(int(ParamSW[1]))
                        Pbeam.append(int(ParamSW[3]))
                        Pif.append(int(ParamSW[5]))
                        Ppol.append(int(ParamSW[7]))
                        Pcycle.append(int(ParamSW[9]))
                    #elif line.find('rms') != -1 and flag:
                    #    Prms.append([[float(line.replace(' ','').split('=')[1])]])
            TableTxt.close()
            os.system('rm '+tbName)
            #print len(Idx), len(Pscan), len(Pbeam), len(Pif), len(Ppol), len(Pcycle), len(Psec), len(Pprm)

            tbTBL.open(outTbl, nomodify=False)
            #tbTBL.putcol('Statistics',Prms)
            i = 0
            for idx in Idx:
                if self.USE_CASA_TABLE:
                    tNOCHANGE = self.DataTable.getcell('NOCHANGE',idx)
                    tMASKLIST = self.DataTable.getcell('MASKLIST',idx)
                else:
                    tNOCHANGE = self.DataTable[idx][DT_NOCHANGE]
                    tMASKLIST = self.DataTable[idx][DT_MASKLIST]
                if type(tNOCHANGE) != int:
                #if type(self.DataTable[idx][DT_NOCHANGE]) != int:
                    if self.USE_CASA_TABLE:
                        row = tROW[idx]
                    else:
                        row = self.DataTable[idx][DT_ROW]
                    #tbTBL.putcol('Statistics',Prms[i],row,1,1)
                    #print Psec[i], type(Psec[i][0][0][0])
                    tbTBL.putcol('Sections',Psec[i],row,1,1)
                    tbTBL.putcol('SectionCoefficients',Pprm[i],row,1,1)
                    #print [self.DataTable[idx][DT_MASKLIST]], type(self.DataTable[idx][DT_MASKLIST][0][0])
                    if len(tMASKLIST) != 0:
                    #if len(self.DataTable[idx][DT_MASKLIST]) != 0:
                        Mask = []
                        for [m0, m1] in tMASKLIST:
                        #for [m0, m1] in self.DataTable[idx][DT_MASKLIST]:
                            Mask.append([[int(m0)],[int(m1)]])
                    else: Mask = [[[0],[0]]]
                    #print Mask, idx, i, len(Idx)
                    tbTBL.putcol('LineMask',Mask,row,1,1)
                    tbTBL.putcol('FitFunc',fitFunc,row,1,1)
                    i+= 1
            tbTBL.close()
            #EndTime = time.time()
            #print 'ProcessTime:', EndTime-StartTime
            #print len(Pscan), len(Pbeam), len(Pif), len(Ppol), len(Pcycle), len(Prms), len(Psec), len(Pprm)
            #for i in range(5):
            #    print Pscan[i], Pbeam[i], Pif[i], Ppol[i], Pcycle[i], Prms[i], Psec[i], Pprm[i] 
        del dummyscan, SpOned, FitResult, Timer
        return


#    @dec_engines_logging
    def CalcBaselineFit(self, scan, idx, polyorder, nchan, modification, FitFunc, edge=(0, 0), blfile='',  LogLevel=2, LogFile=False):

        origin = 'CalcBaselineFit()'

        if self.USE_CASA_TABLE:
            masklist = self.DataTable.getcell('MASKLIST',idx)
        else:
            masklist = self.DataTable[idx][DT_MASKLIST]
        #masklist = self.DataTable[idx][DT_MASKLIST]
        self.LogMessage('DEBUG', Origin=origin, Msg='masklist = %s' % masklist)

        # Initialize plot for debugging
        ###if ShowPlot: SDP.DrawDebugFit(mode='INIT')

        data = NP.array(scan._getspectrum(0), dtype=NP.float32)
        # set edge mask
        (edgeL, edgeR) = parseEdge(edge)
        NCHAN = nchan
        NCHANwoEdge = NCHAN - edgeL - edgeR
        data[:edgeL] = 0.0
        if edgeR > 0: data[-edgeR:] = 0.0

        # Create mask for line protection
        mask = NP.ones(NCHAN, dtype=int)
        if type(masklist) == list:
            for [m0, m1] in masklist:
                mask[m0:m1] = 0
        else: return False
        if edgeR > 0: Nmask = int(NCHANwoEdge - NP.sum(mask[edgeL:-edgeR] * 1.0))
        else: Nmask = int(NCHANwoEdge - NP.sum(mask[edgeL:] * 1.0))
        Nnomask = NCHANwoEdge - Nmask

        resultdata = []
        self.LogMessage('DEBUG', Origin=origin, Msg='NCHANwoEdge, Nmask, diff = %s, %s, %s' % (NCHANwoEdge, Nmask, NCHANwoEdge-Nmask))
        # The following line is here because win_polyorder should be returned to the parent (but not necessary to spline fit)
        (fragment, nwindow, win_polyorder) = self.CalcFragmentation(polyorder, NCHANwoEdge - Nmask, 0)

        # 2010/11/1 CubicSplineFit
        if FitFunc.upper() == 'SPLINE':
            mask[:edgeL] = 0
            if edgeR > 0: mask[-edgeR:] = 0
            NewOrder = max(int(min(polyorder*(NCHANwoEdge-Nmask)/float(NCHANwoEdge)+0.5, Nnomask/10)), 1)
            # 2011/3/9 TN
            # use ASAP's cubic spline fitting function
            self.LogMessage('DEBUG', Origin=origin, Msg='Cubic Spline Fit: Number of Sections = %d' % NewOrder)
            self.LogMessage('DEBUG',Origin=origin,Msg='Using ASAP cspline_baseline')
            ### 2011/05/17 add clipniter for the clipping cycle
            outdata = NP.array(scan.cspline_baseline(insitu=False,mask=mask,npiece=NewOrder,clipthresh=5.0,clipniter=CLIP_CYCLE,blfile=blfile)._getspectrum(0))
        else:
            self.LogMessage('DEBUG', Origin=origin, Msg='fragment, nwindow, win_polyorder = %s, %s, %s' % (fragment, nwindow, win_polyorder))
            self.LogMessage('DEBUG', Origin=origin, Msg='Number of subdivided segments= %s' % nwindow)
            for WIN in range(nwindow):
                EdgeL = int(WIN * NCHANwoEdge / (fragment * 2) + edgeL)
                EdgeR = NCHAN - edgeR - int(NCHANwoEdge * (nwindow - 1 - WIN) / (fragment * 2))
                # Check EdgeL and EdgeR is inside mask region or not
                (NewEdgeL, NewEdgeR) = (EdgeL, EdgeR)
                masked = 0
                for [m0, m1] in masklist:
                    # All masked
                    if m0 <= EdgeL and EdgeR <= m1:
                        NewEdgeL = m0 - (EdgeR - EdgeL) / 2
                        NewEdgeR = m1 + (EdgeR - EdgeL) / 2
                        masked += (EdgeR - EdgeL)
                    # mask inside windows
                    elif EdgeL < m0 and m1 < EdgeR:
                        masked += (m1 - m0)
                        if m1 <= (EdgeL + EdgeR) / 2:
                            NewEdgeL = NewEdgeL - (m1 - m0)
                        elif (EdgeL + EdgeR) / 2 <= m0:
                            NewEdgeR = NewEdgeR + (m1 - m0)
                        else:
                            NewEdgeL = NewEdgeL - ((EdgeL + EdgeR) / 2 - m0)
                            NewEdgeR = NewEdgeR + (m1 - (EdgeL + EdgeR) / 2)
                    # Left edge inside mask
                    elif m0 <= EdgeL and EdgeL < m1:
                        masked += (m1 - EdgeL)
                        if m1 <= (EdgeL + EdgeR) / 2:
                            NewEdgeL = NewEdgeL - (m1 - m0)
                        else:
                            NewEdgeL = m0 - (EdgeR - EdgeL) / 2
                            NewEdgeR = NewEdgeR + (m1 - (EdgeL + EdgeR) / 2)
                    # Right edge inside mask
                    elif m0 < EdgeR and EdgeR <= m1:
                        masked += (EdgeR - m0)
                        if (EdgeL + EdgeR) / 2 <= m0:
                            NewEdgeR = NewEdgeR + (m1 - m0)
                        else:
                            NewEdgeL = NewEdgeL - ((EdgeL + EdgeR) / 2 - m0)
                            NewEdgeR = m1 + (EdgeR - EdgeL) / 2
                    (EdgeL, EdgeR) = (NewEdgeL, NewEdgeR)
    
                EdgeL = max(NewEdgeL, edgeL)
                EdgeR = min(NewEdgeR, NCHAN - edgeR)

                # Calculate positions for combining fragmented spectrum
                WIN_EDGE_IGNORE_L = int(NCHANwoEdge / (fragment * win_polyorder))
                WIN_EDGE_IGNORE_R = WIN_EDGE_IGNORE_L
                PosL0 = int(WIN * NCHANwoEdge / (fragment * 2)) + NCHANwoEdge / fragment / win_polyorder + edgeL
                PosL1 = int((WIN + 1) * NCHANwoEdge / (fragment * 2)) -1 - NCHANwoEdge / fragment / win_polyorder + edgeL
                DeltaL = float(PosL1 - PosL0)
                PosR0 = int((WIN + 1) * NCHANwoEdge / (fragment * 2)) + NCHANwoEdge / fragment / win_polyorder + edgeL
                PosR1 = int((WIN + 2) * NCHANwoEdge / (fragment * 2)) -1 - NCHANwoEdge / fragment / win_polyorder + edgeL
                DeltaR = float(PosR1 - PosR0)
                if WIN == 0:
                    WIN_EDGE_IGNORE_L = 0
                    PosL0 = edgeL
                    PosL1 = edgeL
                    DeltaL = 1.0
                if WIN == (nwindow - 1):
                    WIN_EDGE_IGNORE_R = 0
                    PosR0 = NCHAN - edgeR
                    PosR1 = NCHAN - edgeR
                    DeltaR = 1.0
                self.LogMessage('DEBUG', Origin=origin, Msg='PosL0, PosL1, PosR0, PosR1 = %s, %s, %s, %s' % (PosL0, PosL1, PosR0, PosR1))
                NNmask = float((PosR1 - PosL0) - mask[PosL0:PosR1].sum())
                dorder = int(max(1, ((PosR1 - PosL0 - NNmask*0.5) * win_polyorder / (PosR1 - PosL0) + 0.5)))
                self.LogMessage('DEBUG', Origin=origin, Msg='Revised edgemask = %s:%s  Adjust polyorder = %s' % (EdgeL, EdgeR, dorder))
                self.LogMessage('DEBUG', Origin=origin, Msg='Segment %d: Revised edgemask = %s:%s  Adjust polyorder used in individual fit= %s' % (WIN, EdgeL, EdgeR, dorder))
                FitStartTime = time.time()
                self.LogMessage('DEBUG', Origin=origin, Msg='Fitting Start')

                # Shift scan and mask to set unmasked region starting at zero
                # Shift size is EdgeL: EdgeL -> 0
                if POLYNOMIAL_FIT == 'ASAP':
                    #print "start ASAP fitting..."
                    tmpscan = scan.copy()
                    edgemask = tmpscan.create_mask([0, EdgeR - EdgeL])
                    # 0 and (EdgeR-EdgeL) are included in the fitting range
                    tmpscan._setspectrum(NP.concatenate((data[EdgeL:EdgeR], NP.zeros(NCHAN+EdgeL-EdgeR, dtype=NP.float64))))
                    tmpmask = NP.concatenate((mask[EdgeL:EdgeR], NP.zeros(NCHAN+EdgeL-EdgeR, dtype=int)))
                    # use linear fit
                    if self.casaversion < 310:
                        tmpfit0 = tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask),uselin=True,insitu=False)._getspectrum(0)
                    else:
                        tmpfit0 = tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask),insitu=False,blfile=blfile)._getspectrum(0)
                    tmpfit = NP.array(tmpfit0, dtype=NP.float32)[:EdgeR - EdgeL]
                    del tmpscan, tmpmask, edgemask, tmpfit0
                else:
                    if POLYNOMIAL_FIT == 'CLIP':
                        tmpfit = PolynomialFitClip(data[EdgeL:EdgeR], mask[EdgeL:EdgeR], order=dorder, iter=CLIP_CYCLE, hsigma=5.0, lsigma=5.0, subtract=True, LogLevel=LogLevel, LogFile=LogFile)
                    else:
                        tmpfit = PolynomialFit(data[EdgeL:EdgeR], mask[EdgeL:EdgeR], order=dorder, subtract=True)

                # Restore scan to the original position
                # 0 -> EdgeL
                resultdata.append(list(NP.concatenate((NP.zeros(EdgeL), tmpfit, NP.zeros(NCHAN - EdgeR)))))
                del tmpfit
                FitEndTime = time.time()
                self.LogMessage('DEBUG', Origin=origin, Msg='Fitting End: Elapsed Time= %.1f' % (FitEndTime-FitStartTime) )

                # window function: f(x) = -2x^3 + 2x^2 (0 <= x <= 1)
                for i in range(NCHAN):
                    if i < PosL0:
                        resultdata[WIN][i] = 0.0
                    elif i <= PosL1:
                        x = (i - PosL0) / DeltaL
                        resultdata[WIN][i] *= (-2.0 * x ** 3.0 + 3.0 * x ** 2.0)
                    elif i > PosR1:
                        resultdata[WIN][i] = 0.0
                    elif i >= PosR0:
                        x = (i - PosR0) / DeltaR
                        resultdata[WIN][i] *= (2.0 * x ** 3.0 - 3.0 * x ** 2.0 + 1.0)

                # Plot for verification use only : START
                ###if ShowPlot:
                ###    plotdata =  data - resultdata[WIN]
                ###    for i in range(NCHAN):
                ###        if i < PosL0:
                ###            plotdata[i] = 0.0
                ###        elif i <= PosL1:
                ###            x = (i - PosL0) / DeltaL
                ###            if x == 0 or x == 1:
                ###                plotdata[i] = 0.0
                ###            else:
                ###                plotdata[i] = data[i] - resultdata[WIN][i] / (-2.0 * x ** 3.0 + 3.0 * x ** 2.0)
                ###        elif i > PosR1:
                ###            plotdata[i] = 0.0
                ###        elif i >= PosR0:
                ###            x = (i - PosR0) / DeltaR
                ###            if x == 0 or x == 1:
                ###                plotdata[i] = 0.0
                ###            else:
                ###                plotdata[i] = data[i] - resultdata[WIN][i] / (2.0 * x ** 3.0 - 3.0 * x ** 2.0 + 1.0)
                ###    SDP.DrawDebugFit('FIRST', range(NCHAN), plotdata)
                # Plot for verification : END
            outdata = NP.sum(resultdata, axis=0)

        ###ProcStartTime = time.time()
        ###self.LogMessage('DEBUG', Origin=origin, Msg='RMS before/after fitting calculation Start')
        # Calculate RMS after/before fitting
        ###MaskedData = outdata * mask
        ###StddevMasked = MaskedData.std()
        ###MeanMasked = MaskedData.mean()
        # 2009/9/1 Case that all channels are masked out
        ###if NCHANwoEdge == Nmask: 
        ###    NewRMS = 0.0
        ###else:
        ###    NewRMS = math.sqrt(NCHANwoEdge * StddevMasked ** 2 / (NCHANwoEdge - Nmask) - \
        ###                NCHANwoEdge * Nmask * MeanMasked ** 2 / ((NCHANwoEdge - Nmask) ** 2))
        ###MaskedData = data * mask
        ###StddevMasked = MaskedData.std()
        ###MeanMasked = MaskedData.mean()

        ###if NCHANwoEdge == Nmask: 
        ###    OldRMS = 0.0
        ###else:
        ###    OldRMS = math.sqrt(abs(NCHANwoEdge * StddevMasked ** 2 / (NCHANwoEdge - Nmask) - \
        ###                NCHANwoEdge * Nmask * MeanMasked ** 2 / ((NCHANwoEdge - Nmask) ** 2)))
        ###self.LogMessage('DEBUG', Origin=origin, Msg='Pre-fit RMS= %.2f, Post-fit RMS= %.2f' % (OldRMS, NewRMS) )
        ###ProcEndTime = time.time()
        ###self.LogMessage('DEBUG', Origin=origin, Msg='RMS before/after fitting calculation End: Elapsed time = %.1f sec' % (ProcEndTime - ProcStartTime) )

        # Plot for verification use only : START
        ###if ShowPlot:
        ###    plotdata = data - NP.sum(resultdata)
        ###    Ymin = min(plotdata) - (max(plotdata) - min(plotdata)) * 0.15
        ###    Ymax = max(plotdata) + (max(plotdata) - min(plotdata)) * 0.15
        ###    SDP.DrawDebugFit('SECOND', range(NCHAN), plotdata, Ymin, Ymax)
        # Plot for verification : END

        ### 2012/03/02 Masked Edge should be zero
        outdata[:edgeL] = 0.0
        if edgeR > 0: outdata[-edgeR:] = 0.0
        ###return (outdata, NewRMS, OldRMS, Nmask, win_polyorder, fragment, nwindow)
        return (outdata, Nmask, win_polyorder, fragment, nwindow)


    @dec_engines_logfile
    @dec_engines_logging
    def Process7(self, DataIn, DataOut, vIF, rows, vAnt, NCHAN, TimeGapList=[[],[]], TimeTableList=[[],[]], Iteration=5, interactive=True, edge=(0,0), UserFlag=[], FlagRule=None, LogLevel=2, LogFile=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False,rawFileIdx=0, productTable=None):

        origin = 'Process7()'

        NROW = len(rows)

        ThreNewRMS = FlagRule['RmsPostFitFlag']['Threshold']
        ThreOldRMS = FlagRule['RmsPreFitFlag']['Threshold']
        ThreNewDiff = FlagRule['RunMeanPostFitFlag']['Threshold']
        ThreOldDiff = FlagRule['RunMeanPreFitFlag']['Threshold']
        ThreTsys = FlagRule['TsysFlag']['Threshold']
        ThreExpectedRMSPreFit = FlagRule['RmsExpectedPreFitFlag']['Threshold']
        ThreExpectedRMSPostFit = FlagRule['RmsExpectedPostFitFlag']['Threshold']
        Nmean = FlagRule['RunMeanPreFitFlag']['Nmean']
        Threshold = [ThreNewRMS, ThreOldRMS, ThreNewDiff, ThreOldDiff, ThreTsys]

        if productTable is None:
            outTbl = DataOut+'.product.tbl'
        else:
            outTbl = productTable

        # Select time gap list: 'subscan': large gap; 'raster': small gap
        if FlagRule['Flagging']['ApplicableDuration'] == "subscan":
            TimeTable = TimeTableList[1]
        else:
            TimeTable = TimeTableList[0]
        TimeGap = TimeGapList[1]
        PosGap = TimeGapList[0]

        self.LogMessage('INFO', Origin=origin, Msg='Applied time bin for the running mean calculation: %s' % FlagRule['Flagging']['ApplicableDuration'])

        # Calculate RMS and Diff from running mean
        ProcStartTime = time.time()
        data = self.calcStatistics(DataIn, DataOut, rows, NCHAN, Nmean, TimeTable, edge, LogLevel=LogLevel, LogFile=LogFile)
        ProcEndTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='RMS and diff caluculation End: Elapse time = %.1f sec' % (ProcEndTime - ProcStartTime))

        ProcStartTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Apply flags Start')
        self.LogMessage('INFO', Origin=origin, Msg='Thresholds: (post-fit, pre-fit, post-fit diff, pre-fit diff, Tsys) = (%s)*RMS' % Threshold)
        # Compare statistics with thresholds to set flag
        tmpdata = NP.transpose(data)
        Ndata = len(tmpdata[0])
        for cycle in range(Iteration + 1):
            threshold = []
            for x in range(5):
                Unflag = int(NP.sum(tmpdata[x + 6] * 1.0))
                FlaggedData = tmpdata[x + 1] * tmpdata[x + 6]
                StddevFlagged = FlaggedData.std()
                if StddevFlagged == 0: StddevFlagged = tmpdata[x + 1][0] / 100.0
                MeanFlagged = FlaggedData.mean()
                AVE = MeanFlagged / float(Unflag) * float(Ndata)
                RMS = math.sqrt(Ndata * StddevFlagged ** 2 / Unflag - \
                                Ndata * (Ndata - Unflag) * MeanFlagged ** 2 / (Unflag ** 2))
                ThreP = AVE + RMS * Threshold[x]
                #ThreM = AVE - RMS * Threshold[x]
                if x == 4:
                    # Tsys case
                    ThreM = 0.0
                else: ThreM = -1.0
                threshold.append([ThreM, ThreP])
                for y in range(len(tmpdata[0])):
                    if ThreM < tmpdata[x + 1][y] <= ThreP: tmpdata[x + 6][y] = 1
                    else: tmpdata[x + 6][y] = 0
        N = 0
        for row in rows:
            if self.USE_CASA_TABLE:
                flags = self.DataTable.getcell('FLAG',self.Row2ID[vAnt][row])
                pflags = self.DataTable.getcell('FLAG_PERMANENT',self.Row2ID[vAnt][row])
            else:
                flags = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG]
                pflags = self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG]
            #flags = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG]
            #pflags = self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG]
            #self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][1] = tmpdata[6][N]
            #self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][2] = tmpdata[7][N]
            #self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][3] = tmpdata[8][N]
            #self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][4] = tmpdata[9][N]
            #self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][1] = tmpdata[10][N]
            flags[1] = tmpdata[6][N]
            flags[2] = tmpdata[7][N]
            flags[3] = tmpdata[8][N]
            flags[4] = tmpdata[9][N]
            pflags[1] = tmpdata[10][N]
            if self.USE_CASA_TABLE:
                self.DataTable.putcell('FLAG',self.Row2ID[vAnt][row],flags)
                self.DataTable.putcell('FLAG_PERMANENT',self.Row2ID[vAnt][row],pflags)
            else:
                self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG] = flags
                self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG] = pflags
            #self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG] = flags
            #self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG] = pflags
            N += 1

        # flag by Expected RMS
        self.flagExpectedRMS(vIF, rows, vAnt, FlagRule=FlagRule, LogLevel=LogLevel, LogFile=LogFile, rawFileIdx=rawFileIdx)
        ProcEndTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Apply flags End: Elapse time = %.1f sec' % (ProcEndTime - ProcStartTime))

        ProcStartTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Plot flagging Start')
        # Plot statistics
        # Store data for plotting
        FlaggedRowsCategory = [[],[],[],[],[],[],[],[],[]]
        FlaggedRows = []
        PermanentFlag = []
        NPpdata = NP.zeros((7,NROW), NP.float)
        NPpflag = NP.zeros((7,NROW), NP.int)
        NPprows = NP.zeros((2,NROW), NP.int)
        N = 0
        for row in rows:
            # Update User Flag 2008/6/4
            try:
                Index = UserFlag.index(row)
                if self.USE_CASA_TABLE:
                    tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',self.Row2ID[vAnt][row])
                    tPFLAG[2] = 0
                    self.DataTable.putcell('FLAG_PERMANENT',self.Row2ID[vAnt][row],tPFLAG)
                else:
                    self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][2] = 0
                #self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][2] = 0
            except ValueError:
                if self.USE_CASA_TABLE:
                    tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',self.Row2ID[vAnt][row])
                    tPFLAG[2] = 1
                    self.DataTable.putcell('FLAG_PERMANENT',self.Row2ID[vAnt][row],tPFLAG)
                else:
                    self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][2] = 1
                #self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][2] = 1
            # Check every flags to create summary flag
            if self.USE_CASA_TABLE:
                tFLAG = self.DataTable.getcell('FLAG',self.Row2ID[vAnt][row])
                tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',self.Row2ID[vAnt][row])
                tTSYS = self.DataTable.getcell('TSYS',self.Row2ID[vAnt][row])
                tSTAT = self.DataTable.getcell('STATISTICS',self.Row2ID[vAnt][row])
            else:
                tFLAG = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG]
                tPFLAG = self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG]
                tTSYS = self.DataTable[self.Row2ID[vAnt][row]][DT_TSYS]
                tSTAT = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT]
            Flag = 1
            if (tPFLAG == 0 and FlagRule['WeatherFlag']['isActive']) or \
               (tPFLAG == 0 and FlagRule['TsysFlag']['isActive']) or \
               (tPFLAG == 0 and FlagRule['UserFlag']['isActive']):
#            if (self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][0] == 0 and FlagRule['WeatherFlag']['isActive']) or \
#               (self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][1] == 0 and FlagRule['TsysFlag']['isActive']) or \
#               (self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][2] == 0 and FlagRule['UserFlag']['isActive']):
                Flag = 0
            PermanentFlag.append(Flag)
            if Flag == 0 or \
               (tFLAG[1] == 0 and FlagRule['RmsPostFitFlag']['isActive']) or \
               (tFLAG[2] == 0 and FlagRule['RmsPreFitFlag']['isActive']) or \
               (tFLAG[3] == 0 and FlagRule['RunMeanPostFitFlag']['isActive']) or \
               (tFLAG[4] == 0 and FlagRule['RunMeanPreFitFlag']['isActive']) or \
               (tFLAG[5] == 0 and FlagRule['RmsExpectedPostFitFlag']['isActive']) or \
               (tFLAG[6] == 0 and FlagRule['RmsExpectedPreFitFlag']['isActive']):
#            if Flag == 0 or \
#               (self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][1] == 0 and FlagRule['RmsPostFitFlag']['isActive']) or \
#               (self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][2] == 0 and FlagRule['RmsPreFitFlag']['isActive']) or \
#               (self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][3] == 0 and FlagRule['RunMeanPostFitFlag']['isActive']) or \
#               (self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][4] == 0 and FlagRule['RunMeanPreFitFlag']['isActive']) or \
#               (self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][5] == 0 and FlagRule['RmsExpectedPostFitFlag']['isActive']) or \
#               (self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][6] == 0 and FlagRule['RmsExpectedPreFitFlag']['isActive']):
                if self.USE_CASA_TABLE:
                    self.DataTable.putcell('FLAG_SUMMARY',self.Row2ID[vAnt][row],0)
                else:
                    self.DataTable[self.Row2ID[vAnt][row]][DT_SFLAG] = 0
                #self.DataTable[self.Row2ID[vAnt][row]][DT_SFLAG] = 0
                FlaggedRows.append(row)
            else:
                if self.USE_CASA_TABLE:
                    self.DataTable.putcell('FLAG_SUMMARY',self.Row2ID[vAnt][row],1)
                else:
                    self.DataTable[self.Row2ID[vAnt][row]][DT_SFLAG] = 1
                #self.DataTable[self.Row2ID[vAnt][row]][DT_SFLAG] = 1
            # Tsys flag
            NPpdata[0][N] = tTSYS
            NPpflag[0][N] = tPFLAG[1]
            #NPpdata[0][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_TSYS]
            #NPpflag[0][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][1]
            NPprows[0][N] = row
            Valid = True
            if tPFLAG[1] == 0:
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][1] == 0:
                FlaggedRowsCategory[0].append(row)
                Valid = False
            # Weather flag
            if tPFLAG[0] == 0:
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][0] == 0:
                FlaggedRowsCategory[1].append(row)
                Valid = False
            # User flag
            if tPFLAG[2] == 0:
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_PFLAG][2] == 0:
                FlaggedRowsCategory[2].append(row)
                Valid = False

            # commented out not to remove permanent flagged data to show
            #if Valid:
            NPprows[1][N] = row
            # RMS flag before baseline fit
            NPpdata[1][N] = tSTAT[2]
            NPpflag[1][N] = tFLAG[2]
            if tFLAG[2] == 0:
            #NPpdata[1][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][2]
            #NPpflag[1][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][2]
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][2] == 0:
                FlaggedRowsCategory[4].append(row)
            # RMS flag after baseline fit
            NPpdata[2][N] = tSTAT[1]
            NPpflag[2][N] = tFLAG[1]
            if tFLAG[1] == 0:
            #NPpdata[2][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][1]
            #NPpflag[2][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][1]
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][1] == 0:
                FlaggedRowsCategory[3].append(row)
            # Running mean flag before baseline fit
            NPpdata[3][N] = tSTAT[4]
            NPpflag[3][N] = tFLAG[4]
            if tFLAG[4] == 0:
            #NPpdata[3][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][4]
            #NPpflag[3][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][4]
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][4] == 0:
                FlaggedRowsCategory[6].append(row)
            # Running mean flag after baseline fit
            NPpdata[4][N] = tSTAT[3]
            NPpflag[4][N] = tFLAG[3]
            if tFLAG[3] == 0:
            #NPpdata[4][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][3]
            #NPpflag[4][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][3]
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][3] == 0:
                FlaggedRowsCategory[5].append(row)
            # Expected RMS flag before baseline fit
            NPpdata[5][N] = tSTAT[6]
            NPpflag[5][N] = tFLAG[6]
            if tFLAG[6] == 0:
            #NPpdata[5][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][6]
            #NPpflag[5][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][6]
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][6] == 0:
                FlaggedRowsCategory[8].append(row)
            # Expected RMS flag after baseline fit
            NPpdata[6][N] = tSTAT[5]
            NPpflag[6][N] = tFLAG[5]
            if tFLAG[5] == 0:
            #NPpdata[6][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][5]
            #NPpflag[6][N] = self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][5]
            #if self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][5] == 0:
                FlaggedRowsCategory[7].append(row)
            N += 1
        # data store finished

        # Tsys flag
        PlotData = {'row': NPprows[0], 'data': NPpdata[0], 'flag': NPpflag[0], \
                    'thre': [threshold[4][1], 0.0], \
                    'gap': [PosGap, TimeGap], \
                            'title': "Tsys (K)\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['TsysFlag']['Threshold'], \
                    'xlabel': "row (spectrum)", \
                    'ylabel': "Tsys (K)", \
                    'permanentflag': PermanentFlag, \
                    'isActive': FlagRule['TsysFlag']['isActive'], \
                    'threType': "line"}
        SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_0')

        # RMS flag before baseline fit
        PlotData['row'] = NPprows[1]
        PlotData['data'] = NPpdata[1]
        PlotData['flag'] = NPpflag[1]
        PlotData['thre'] = [threshold[1][1]]
        PlotData['title'] = "Baseline RMS (K) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['RmsPreFitFlag']['Threshold']
        PlotData['ylabel'] = "Baseline RMS (K)"
        PlotData['isActive'] = FlagRule['RmsPreFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_1')

        # RMS flag after baseline fit
        PlotData['data'] = NPpdata[2]
        PlotData['flag'] = NPpflag[2]
        PlotData['thre'] = [threshold[0][1]]
        PlotData['title'] = "Baseline RMS (K) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % FlagRule['RmsPostFitFlag']['Threshold']
        PlotData['isActive'] = FlagRule['RmsPostFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_2')

        # Running mean flag before baseline fit
        PlotData['data'] = NPpdata[3]
        PlotData['flag'] = NPpflag[3]
        PlotData['thre'] = [threshold[3][1]]
        PlotData['title'] = "RMS (K) for Baseline Deviation from the running mean (Nmean=%d) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (FlagRule['RunMeanPreFitFlag']['Nmean'], FlagRule['RunMeanPreFitFlag']['Threshold'])
        PlotData['isActive'] = FlagRule['RunMeanPreFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_3')

        # Running mean flag after baseline fit
        PlotData['data'] = NPpdata[4]
        PlotData['flag'] = NPpflag[4]
        PlotData['thre'] = [threshold[2][1]]
        PlotData['title'] = "RMS (K) for Baseline Deviation from the running mean (Nmean=%d) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (FlagRule['RunMeanPostFitFlag']['Nmean'], FlagRule['RunMeanPostFitFlag']['Threshold'])
        PlotData['isActive'] = FlagRule['RunMeanPostFitFlag']['isActive']
        SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_4')

        # Expected RMS flag before baseline fit
        PlotData['data'] = NPpdata[1]
        PlotData['flag'] = NPpflag[5]
        PlotData['thre'] = [NPpdata[5]]
        PlotData['title'] = "Baseline RMS (K) compared with the expected RMS calculated from Tsys before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f, Red H-line(s): out of vertical scale limit(s)" % ThreExpectedRMSPreFit
        PlotData['isActive'] = FlagRule['RmsExpectedPreFitFlag']['isActive']
        PlotData['threType'] = "plot"
        SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_5')

        # Expected RMS flag after baseline fit
        PlotData['data'] = NPpdata[2]
        PlotData['flag'] = NPpflag[6]
        PlotData['thre'] = [NPpdata[6]]
        PlotData['title'] = "Baseline RMS (K) compared with the expected RMS calculated from Tsys after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f" % ThreExpectedRMSPostFit
        PlotData['isActive'] = FlagRule['RmsExpectedPostFitFlag']['isActive']
        PlotData['threType'] = "plot"
        SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_6')

        # Create Flagging Summary Page
        if FigFileDir != False:
            Filename = FigFileDir+FigFileRoot+'.html'
            if os.access(Filename, os.F_OK): os.remove(Filename)
            Out = open(Filename, 'w')
            print >> Out, '<html>\n<head>\n<style>'
            print >> Out, '.ttl{font-size:20px;font-weight:bold;}'
            print >> Out, '.stt{font-size:18px;font-weight:bold;color:white;background-color:navy;}'
            print >> Out, '.stp{font-size:18px;font-weight:bold;color:black;background-color:gray;}'
            print >> Out, '.stc{font-size:16px;font-weight:normal;}'
            print >> Out, '</style>\n</head>\n<body>'
            print >> Out, '<p class="ttl">Flagging Status</p>'
            print >> Out, '<table border="1">'
            print >> Out, '<tr align="center" class="stt"><th>&nbsp</th><th>isActive?</th><th>SigmaThreshold<th>Flagged spectra</th><th>Flagged ratio(%)</th></tr>'
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('User', FlagRule['UserFlag']['isActive'], FlagRule['UserFlag']['Threshold'], len(FlaggedRowsCategory[2]), len(FlaggedRowsCategory[2])*100.0/NROW)
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Weather', FlagRule['WeatherFlag']['isActive'], FlagRule['WeatherFlag']['Threshold'], len(FlaggedRowsCategory[1]), len(FlaggedRowsCategory[1])*100.0/NROW)
            print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Tsys', FlagRule['TsysFlag']['isActive'], FlagRule['TsysFlag']['Threshold'], len(FlaggedRowsCategory[0]), len(FlaggedRowsCategory[0])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('RMS baseline (pre-fit)', FlagRule['RmsPreFitFlag']['isActive'], FlagRule['RmsPreFitFlag']['Threshold'], len(FlaggedRowsCategory[4]), len(FlaggedRowsCategory[4])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('RMS baseline (post-fit)', FlagRule['RmsPostFitFlag']['isActive'], FlagRule['RmsPostFitFlag']['Threshold'], len(FlaggedRowsCategory[3]), len(FlaggedRowsCategory[3])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Running Mean (pre-fit)', FlagRule['RunMeanPreFitFlag']['isActive'], FlagRule['RunMeanPreFitFlag']['Threshold'], len(FlaggedRowsCategory[6]), len(FlaggedRowsCategory[6])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Running Mean (post-fit)', FlagRule['RunMeanPostFitFlag']['isActive'], FlagRule['RunMeanPostFitFlag']['Threshold'], len(FlaggedRowsCategory[5]), len(FlaggedRowsCategory[5])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Expected RMS (pre-fit)', FlagRule['RmsExpectedPreFitFlag']['isActive'], FlagRule['RmsExpectedPreFitFlag']['Threshold'], len(FlaggedRowsCategory[8]), len(FlaggedRowsCategory[8])*100.0/NROW)
            print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Expected RMS (post-fit)', FlagRule['RmsExpectedPostFitFlag']['isActive'], FlagRule['RmsExpectedPostFitFlag']['Threshold'], len(FlaggedRowsCategory[7]), len(FlaggedRowsCategory[7])*100.0/NROW)
            print >> Out, '<tr align="center" class="stt"><th>%s</th><th>%s</th><th>%s</th><th>%s</th><th>%.1f</th></tr>' % ('Total Flagged', '-', '-', len(FlaggedRows), len(FlaggedRows)*100.0/NROW)
            print >> Out, '<tr><td colspan=4>%s</td></tr>' % ("Note: flags in grey background are permanent, <br> which are not reverted or changed during the iteration cycles.") 
            print >> Out, '</table>\n</body>\n</html>'
            Out.close()


        if len(FlaggedRowsCategory[2]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by User =%s ' % FlaggedRowsCategory[2])
        if len(FlaggedRowsCategory[1]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by Weather =%s ' % FlaggedRowsCategory[1])
        if len(FlaggedRowsCategory[0]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by Tsys =%s ' % FlaggedRowsCategory[0])
        if len(FlaggedRowsCategory[4]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by the baseline fluctuation (pre-fit) =%s ' % FlaggedRowsCategory[4])
        if len(FlaggedRowsCategory[3]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by the baseline fluctuation (post-fit) =%s ' % FlaggedRowsCategory[3])
        if len(FlaggedRowsCategory[6]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by the difference from running mean (pre-fit) =%s ' % FlaggedRowsCategory[6])
        if len(FlaggedRowsCategory[5]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by the difference from running mean (post-fit) =%s ' % FlaggedRowsCategory[5])
        if len(FlaggedRowsCategory[8]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by the expected RMS (pre-fit) =%s ' % FlaggedRowsCategory[8])
        if len(FlaggedRowsCategory[7]) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Flagged rows by the expected RMS (post-fit) =%s ' % FlaggedRowsCategory[7])
        if len(FlaggedRows) > 0:
            self.LogMessage('INFO', Origin=origin, Msg='Final Flagged rows by all active categories =%s ' % FlaggedRows)

        ProcEndTime = time.time()
        self.LogMessage('INFO', Origin=origin, Msg='Plot flags End: Elapsed time = %.1f sec' % (ProcEndTime - ProcStartTime) )

        del tmpdata, threshold, NPpdata, NPpflag, NPprows, PlotData, FlaggedRows, FlaggedRowsCategory

        # 2012/09/01 for Table Output
        #StartTime = time.time()
        tbTBL = gentools(['tb'])[0]
        tbTBL.open(outTbl, nomodify=False)
        for row in rows:
            ID = self.Row2ID[vAnt][row]
            if self.USE_CASA_TABLE:
                tflaglist = self.DataTable.getcell('FLAG',ID)[1:7]
                tpflaglist = self.DataTable.getcell('FLAG_PERMANENT',ID)[:3]
                tstatisticslist = self.DataTable.getcell('STATISTICS',ID)[1:7]
                flaglist, pflaglist, statisticslist = [], [], []
                for i in range(6):
                    flaglist.append([tflaglist[i]])
                for i in range(3):
                    pflaglist.append([tpflaglist[i]])
                for i in range(6):
                    statisticslist.append([tstatisticslist[i]])
            else:
                flaglist, pflaglist, statisticslist = [], [], []
                for i in range(1,7):
                    flaglist.append([self.DataTable[ID][DT_FLAG][i]])
                for i in range(3):
                    pflaglist.append([self.DataTable[ID][DT_PFLAG][i]])
                for i in range(1,7):
                    statisticslist.append([self.DataTable[ID][DT_STAT][i]])
            #for i in range(1,7):
            #    flaglist.append([self.DataTable[ID][DT_FLAG][i]])
            #for i in range(3):
            #    pflaglist.append([self.DataTable[ID][DT_PFLAG][i]])
            #for i in range(1,7):
            #    statisticslist.append([self.DataTable[ID][DT_STAT][i]])
            tbTBL.putcol('StatisticsFlags',flaglist,row,1,1)
            tbTBL.putcol('PermanentFlags',pflaglist,row,1,1)
            tbTBL.putcol('Statistics',statisticslist,row,1,1)
            if self.USE_CASA_TABLE:
                tbTBL.putcol('SummaryFlag',bool(self.DataTable.getcell('FLAG_SUMMARY',ID)),row,1,1)
            else:
                tbTBL.putcol('SummaryFlag',bool(self.DataTable[ID][DT_SFLAG]),row,1,1)
            #tbTBL.putcol('SummaryFlag',bool(self.DataTable[ID][DT_SFLAG]),row,1,1)
        tbTBL.close()
        #EndTime = time.time()
        #print 'ProcessTime:', EndTime-StartTime
        return


    @dec_engines_logfile
    @dec_engines_logging
    def calcStatistics(self, DataIn, DataOut, rows, NCHAN, Nmean, TimeTable, edge, LogLevel=2, LogFile=False):

        origin = 'calcStatistics()'

        # Calculate RMS and Diff from running mean
        NROW = len(rows)
        (edgeL, edgeR) = parseEdge(edge)

        self.LogMessage('INFO', Origin=origin, Msg='Calculate RMS and Diff from running mean for Pre/Post fit...')
        self.LogMessage('INFO', Origin=origin, Msg='Processing %d spectra...' % NROW)
        self.LogMessage('INFO', Origin=origin, Msg='Nchan for running mean=%s' % Nmean)
        data = []
        flag = 1

        ProcStartTime = time.time()

        self.LogMessage('INFO', Origin=origin, Msg='RMS and diff caluculation Start')

        #tbtool = casac.homefinder.find_home_by_name('tableHome')
        #tbIn = tbtool.create()
        #tbOut = tbtool.create()
        tbIn,tbOut = gentools(['tb','tb'])
        tbIn.open(DataIn)
        tbOut.open(DataOut)

        # Create progress timer
        Timer = ProgressTimer(80, NROW, LogLevel)
        for chunks in TimeTable:
            # chunks[0]: row, chunks[1]: index
            chunk = chunks[0]
            self.LogMessage('DEBUG', Origin=origin, Msg='Before Fit: Processing spectra = %s' % chunk)
            self.LogMessage('DEBUG', Origin=origin, Msg='chunks[0]= %s' % chunks[0])
            nrow = len(chunks[0])
            START = 0
            ### 2011/05/26 shrink the size of data on memory
            SpIn = NP.zeros((nrow, NCHAN), dtype=NP.float32)
            SpOut = NP.zeros((nrow, NCHAN), dtype=NP.float32)
            for index in range(len(chunks[0])):
                SpIn[index] = tbIn.getcell('SPECTRA', chunks[0][index])
                SpOut[index] = tbOut.getcell('SPECTRA', chunks[0][index])
            SpIn[index][:edgeL] = 0
            SpOut[index][:edgeL] = 0
            if edgeR > 0:
                SpIn[index][-edgeR:] = 0
                SpOut[index][-edgeR:] = 0
            ### loading of the data for one chunk is done

            for index in range(len(chunks[0])):
                row = chunks[0][index]
                idx = chunks[1][index]
                # Countup progress timer
                Timer.count()
                START += 1
                mask = NP.ones(NCHAN, NP.int)
                if self.USE_CASA_TABLE:    
                    for [m0, m1] in self.DataTable.getcell('MASKLIST',idx): mask[m0:m1] = 0
                else:
                    for [m0, m1] in self.DataTable[idx][DT_MASKLIST]: mask[m0:m1] = 0
                #for [m0, m1] in self.DataTable[idx][DT_MASKLIST]: mask[m0:m1] = 0
                mask[:edgeL] = 0
                if edgeR > 0: mask[-edgeR:] = 0
                Nmask = int(NCHAN - NP.sum(mask * 1.0))

                # Calculate RMS
                ### 2011/05/26 shrink the size of data on memory
                MaskedData = SpIn[index] * mask
                StddevMasked = MaskedData.std()
                MeanMasked = MaskedData.mean()
                OldRMS = math.sqrt(abs(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                                NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2)))
                #self.DataTable[idx][DT_STAT][2] = OldRMS
                if self.USE_CASA_TABLE:
                    stats = self.DataTable.getcell('STATISTICS',idx)
                else:
                    stats = self.DataTable[idx][DT_STAT]
                #stats = self.DataTable[idx][DT_STAT]
                stats[2] = OldRMS

                MaskedData = SpOut[index] * mask
                StddevMasked = MaskedData.std()
                MeanMasked = MaskedData.mean()
                NewRMS = math.sqrt(abs(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                                NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2)))
                #self.DataTable[idx][DT_STAT][1] = NewRMS
                stats[1] = NewRMS

                # Calculate Diff from the running mean
                ### 2011/05/26 shrink the size of data on memory
                ### modified to calculate Old and New statistics in a single cycle
                if nrow == 1:
                    OldRMSdiff = 0.0
                    #self.DataTable[idx][DT_STAT][4] = OldRMSdiff
                    stats[4] = OldRMSdiff
                    NewRMSdiff = 0.0
                    #self.DataTable[idx][DT_STAT][3] = NewRMSdiff
                    stats[3] = NewRMSdiff
                else:
                    if START == 1:
                        Rmask = NP.zeros(NCHAN, NP.int)
                        RdataOld0 = NP.zeros(NCHAN, NP.float64)
                        RdataNew0 = NP.zeros(NCHAN, NP.float64)
                        NR = 0
                        for x in range(1, min(Nmean + 1, nrow)):
                            NR += 1
                            RdataOld0 += SpIn[x]
                            RdataNew0 += SpOut[x]
                            mask0 = NP.ones(NCHAN, NP.int)
                            if self.USE_CASA_TABLE:
                                for [m0, m1] in self.DataTable.getcell('MASKLIST',chunks[0][x]): mask0[m0:m1] = 0
                            else:
                                for [m0, m1] in self.DataTable[chunks[0][x]][DT_MASKLIST]: mask0[m0:m1] = 0
                            #for [m0, m1] in self.DataTable[chunks[0][x]][DT_MASKLIST]: mask0[m0:m1] = 0
                            Rmask += mask0
                    elif START > (nrow - Nmean):
                        NR -= 1
                        RdataOld0 -= SpIn[index]
                        RdataNew0 -= SpOut[index]
                        Rmask -= mask
                    else:
                        RdataOld0 -= (SpIn[index] - SpIn[START + Nmean - 1])
                        RdataNew0 -= (SpOut[index] - SpOut[START + Nmean - 1])
                        mask0 = NP.ones(NCHAN, NP.int)
                        if self.USE_CASA_TABLE:
                            for [m0, m1] in self.DataTable.getcell('MASKLIST',chunks[0][START + Nmean - 1]): mask0[m0:m1] = 0
                        else:
                            for [m0, m1] in self.DataTable[chunks[0][START + Nmean - 1]][DT_MASKLIST]: mask0[m0:m1] = 0
                        #for [m0, m1] in self.DataTable[chunks[0][START + Nmean - 1]][DT_MASKLIST]: mask0[m0:m1] = 0
                        Rmask += (mask0 - mask)
                    if START == 1:
                        Lmask = NP.zeros(NCHAN, NP.int)
                        LdataOld0 = NP.zeros(NCHAN, NP.float64)
                        LdataNew0 = NP.zeros(NCHAN, NP.float64)
                        NL = 0
                    elif START <= (Nmean + 1):
                        NL += 1
                        LdataOld0 += SpIn[START - 2]
                        LdataNew0 += SpOut[START - 2]
                        mask0 = NP.ones(NCHAN, NP.int)
                        if self.USE_CASA_TABLE:
                            for [m0, m1] in self.DataTable.getcell('MASKLIST',chunks[0][START - 2]): mask0[m0:m1] = 0
                        else:
                            for [m0, m1] in self.DataTable[chunks[0][START - 2]][DT_MASKLIST]: mask0[m0:m1] = 0
                        #for [m0, m1] in self.DataTable[chunks[0][START - 2]][DT_MASKLIST]: mask0[m0:m1] = 0
                        Lmask += mask0
                    else:
                        LdataOld0 += (SpIn[START - 2] - SpIn[START - 2 - Nmean])
                        LdataNew0 += (SpOut[START - 2] - SpOut[START - 2 - Nmean])
                        mask0 = NP.ones(NCHAN, NP.int)
                        if self.USE_CASA_TABLE:
                            for [m0, m1] in self.DataTable.getcell('MASKLIST',chunks[0][START - 2]): mask0[m0:m1] = 0
                        else:
                            for [m0, m1] in self.DataTable[chunks[0][START - 2]][DT_MASKLIST]: mask0[m0:m1] = 0
                        #for [m0, m1] in self.DataTable[chunks[0][START - 2]][DT_MASKLIST]: mask0[m0:m1] = 0
                        Lmask += mask0
                        mask0 = NP.ones(NCHAN, NP.int)
                        if self.USE_CASA_TABLE:
                            for [m0, m1] in self.DataTable.getcell('MASKLIST',chunks[0][START - 2 - Nmean]): mask0[m0:m1] = 0
                        else:
                            for [m0, m1] in self.DataTable[chunks[0][START - 2 - Nmean]][DT_MASKLIST]: mask0[m0:m1] = 0
                        #for [m0, m1] in self.DataTable[chunks[0][START - 2 - Nmean]][DT_MASKLIST]: mask0[m0:m1] = 0
                        Lmask -= mask0

                    diffOld0 = (LdataOld0 + RdataOld0) / float(NL + NR) - SpIn[index]
                    diffNew0 = (LdataNew0 + RdataNew0) / float(NL + NR) - SpOut[index]
                    mask0 = (Rmask + Lmask + mask) / (NL + NR + 1)

                    # Calculate RMS
                    MaskedDataOld = diffOld0 * mask0
                    StddevMasked = MaskedDataOld.std()
                    MeanMasked = MaskedDataOld.mean()
                    #print row, StddevMasked, MeanMasked, NCHAN-Nmask
                    OldRMSdiff = math.sqrt(abs((NCHAN * StddevMasked ** 2 - Nmask * MeanMasked ** 2 )/ (NCHAN - Nmask)))
                    #self.DataTable[idx][DT_STAT][4] = OldRMSdiff
                    stats[4] = OldRMSdiff
                    MaskedDataNew = diffNew0 * mask0
                    StddevMasked = MaskedDataNew.std()
                    MeanMasked = MaskedData.mean()
                    #print row, StddevMasked, MeanMasked, NCHAN-Nmask
                    NewRMSdiff = math.sqrt(abs((NCHAN * StddevMasked ** 2 - Nmask * MeanMasked ** 2 )/ (NCHAN - Nmask)))
                    #self.DataTable[idx][DT_STAT][3] = NewRMSdiff
                    stats[3] = NewRMSdiff
                if self.USE_CASA_TABLE:
                    self.DataTable.putcell('STATISTICS',idx,stats)
                    self.DataTable.putcell('NMASK',idx,Nmask)
                else:
                    self.DataTable[idx][DT_STAT] = stats
                    self.DataTable[idx][DT_NMASK] = Nmask
                #self.DataTable[idx][DT_STAT] = stats
                #self.DataTable[idx][DT_NMASK] = Nmask
                self.LogMessage('INFO', Origin=origin, Msg='Row=%d, pre-fit RMS= %.2f pre-fit diff RMS= %.2f' % (row, OldRMS, OldRMSdiff))
                self.LogMessage('INFO', Origin=origin, Msg='Row=%d, post-fit RMS= %.2f post-fit diff RMS= %.2f' % (row, NewRMS, NewRMSdiff))
                if self.USE_CASA_TABLE:
                    data.append([row, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, self.DataTable.getcell('TSYS',idx), flag, flag, flag, flag, flag, Nmask])
                else:
                    data.append([row, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, self.DataTable[idx][DT_TSYS], flag, flag, flag, flag, flag, Nmask])
                #data.append([row, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, self.DataTable[idx][DT_TSYS], flag, flag, flag, flag, flag, Nmask])
            del SpIn, SpOut
        return data


    @dec_engines_logfile
    @dec_engines_logging
    def flagExpectedRMS(self, vIF, rows, vAnt, FlagRule=None, LogLevel=2, LogFile=False, rawFileIdx=0):
        # FLagging based on expected RMS
        # TODO: Include in normal flags scheme

        # The expected RMS according to the radiometer formula sometimes needs
        # special scaling factors to account for meta data conventions (e.g.
        # whether Tsys is given for DSB or SSB mode) and for backend specific
        # setups (e.g. correlator, AOS, etc. noise scaling). These factors are
        # not saved in the data sets' meta data. Thus we have to read them from
        # a special file. TODO: This needs to be changed for ALMA later on.

        origin = 'flagExpectedRMS()'

        try:
            fd = open('%s.exp_rms_factors' % (self.DataTable['FileName'][rawFileIdx]), 'r')
            sc_fact_list = fd.readlines()
            fd.close()
            sc_fact_dict = {}
            for sc_fact in sc_fact_list:
                sc_fact_key, sc_fact_value = sc_fact.replace('\n','').split()
                sc_fact_dict[sc_fact_key] = float(sc_fact_value)
            tsys_fact = sc_fact_dict['tsys_fact']
            nebw_fact = sc_fact_dict['nebw_fact']
            integ_time_fact = sc_fact_dict['integ_time_fact']
            self.LogMessage('INFO', Origin=origin, Msg="Using scaling factors tsys_fact=%f, nebw_fact=%f and integ_time_fact=%f for flagging based on expected RMS." % (tsys_fact, nebw_fact, integ_time_fact))
        except:
            self.LogMessage('WARNING', Origin=origin, Msg="Cannot read scaling factors for flagging based on expected RMS. Using 1.0.")
            tsys_fact = 1.0
            nebw_fact = 1.0
            integ_time_fact = 1.0

        # TODO: Make threshold a parameter
        # This needs to be quite strict to catch the ripples in the bad Orion
        # data. Maybe this is due to underestimating the total integration time.
        # Check again later.
        # 2008/10/31 divided the category into two
        #ThreExpectedRMS = FlagRule['RmsExpectedFlag']['Threshold']
        ThreExpectedRMSPreFit = FlagRule['RmsExpectedPreFitFlag']['Threshold']
        ThreExpectedRMSPostFit = FlagRule['RmsExpectedPostFitFlag']['Threshold']
        #ThreExpectedRMS = 1.333

        # The noise equivalent bandwidth is proportional to the channel width
        # but may need a scaling factor. This factor was read above.
        Abcissa = self.Abcissa[vIF]
        noiseEquivBW = abs(Abcissa[1][1]-Abcissa[1][0]) * 1e9 * nebw_fact
        #noiseEquivBW = abs(self.Abcissa[vIF][1][1]-self.Abcissa[vIF][1][0]) * 1e9 * nebw_fact

        tEXPT = None
        tTSYS = None
        if self.USE_CASA_TABLE:
            tEXPT = self.DataTable.getcol('EXPOSURE')
            tTSYS = self.DataTable.getcol('TSYS')

        for row in rows:
            # The HHT and APEX test data show the "on" time only in the CLASS
            # header. To get the total time, at least a factor of 2 is needed,
            # for OTFs and rasters with several on per off even higher, but this
            # cannot be automatically determined due to lacking meta data. We
            # thus use a manually supplied scaling factor.
            #integTimeSec = self.DataTable[row][DT_EXPT] * integ_time_fact
            if self.USE_CASA_TABLE:
                integTimeSec = tEXPT[self.Row2ID[vAnt][row]] * integ_time_fact
            else:
                integTimeSec = self.DataTable[self.Row2ID[vAnt][row]][DT_EXPT] * integ_time_fact
            #integTimeSec = self.DataTable[self.Row2ID[vAnt][row]][DT_EXPT] * integ_time_fact
            # The Tsys value can be saved for DSB or SSB mode. A scaling factor
            # may be needed. This factor was read above.
            #currentTsys = self.DataTable[row][DT_TSYS] * tsys_fact
            if self.USE_CASA_TABLE:
                currentTsys = tTSYS[self.Row2ID[vAnt][row]] * tsys_fact
            else:
                currentTsys = self.DataTable[self.Row2ID[vAnt][row]][DT_TSYS] * tsys_fact
            #currentTsys = self.DataTable[self.Row2ID[vAnt][row]][DT_TSYS] * tsys_fact
            if ((noiseEquivBW * integTimeSec) > 0.0):
                expectedRMS = currentTsys / math.sqrt(noiseEquivBW * integTimeSec)
                # 2008/10/31
                # Comparison with both pre- and post-BaselineFit RMS
                if self.USE_CASA_TABLE:
                    stats = self.DataTable.getcell('STATISTICS',self.Row2ID[vAnt][row])
                else:
                    stats = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT]
                #stats = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT]
                #PostFitRMS = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][1]
                #PreFitRMS = self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][2]
                PostFitRMS = stats[1]
                PreFitRMS = stats[2]
                self.LogMessage('DEBUG_DM',Origin=origin,Msg='DEBUG_DM: Row: %d Expected RMS: %f PostFit RMS: %f PreFit RMS: %f' % (row, expectedRMS, PostFitRMS, PreFitRMS))
                #self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][5] = expectedRMS * ThreExpectedRMSPostFit
                #self.DataTable[self.Row2ID[vAnt][row]][DT_STAT][6] = expectedRMS * ThreExpectedRMSPreFit
                stats[5] = expectedRMS * ThreExpectedRMSPostFit
                stats[6] = expectedRMS * ThreExpectedRMSPreFit
                if self.USE_CASA_TABLE:
                    self.DataTable.putcell('STATISTICS',self.Row2ID[vAnt][row],stats)
                else:
                    self.DataTable[self.Row2ID[vAnt][row]][DT_STAT] = stats
                #self.DataTable[self.Row2ID[vAnt][row]][DT_STAT] = stats
                if self.USE_CASA_TABLE:
                    flags = self.DataTable.getcell('FLAG',row)
                else:
                    flags = self.DataTable[row][DT_FLAG]
                #flags = self.DataTable[row][DT_FLAG]
                if (PostFitRMS > ThreExpectedRMSPostFit * expectedRMS):
                    #self.DataTable[row][DT_FLAG][5] = 0
                    flags[5] = 0
                else:
                    #self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][5] = 1
                    flags[5] = 1
                if (PreFitRMS > ThreExpectedRMSPreFit * expectedRMS):
                    #self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][6] = 0
                    flags[6] = 0
                else:
                    #self.DataTable[self.Row2ID[vAnt][row]][DT_FLAG][6] = 1
                    flags[6] = 1
                if self.USE_CASA_TABLE:
                    self.DataTable.putcell('FLAG',row,flags)
                else:
                    self.DataTable[row][DT_FLAG] = flags
                #self.DataTable[row][DT_FLAG] = flags


    def CalcFragmentation(self, polyorder, nchan, modification=0):
        polyorder += modification
        # Number of fitting window is determined according to the polynomial order
        fragment = int(min(polyorder / MAX_FREQ + 1, max(nchan / MIN_CHANNELS, 1)))
        if fragment > MAX_WINDOW_FRAGMENTATION: fragment = MAX_WINDOW_FRAGMENTATION
        nwindow = fragment * 2 - 1
        win_polyorder = min(int(polyorder / fragment) + fragment - 1, MAX_FREQ)
        return (fragment, nwindow, win_polyorder)


    @dec_engines_logfile
    @dec_engines_logging
    def GroupForGrid(self, Idx, vIF, vPOL, CombineRadius, Allowance, GridSpacing, ObsPattern, LogLevel=2, LogFile=False):
        """
        Gridding by RA/DEC position
        """
        origin = 'GroupForGrid'
        
        # Re-Gridding

        NROW = len(Idx)
        self.LogMessage('INFO', Origin=origin, Msg='ObsPattern = %s' % ObsPattern)
        self.LogMessage('INFO', Origin=origin, Msg='Processing %d spectra...' % NROW)

        GridTable = []
        # 2008/09/20 Spacing should be identical between RA and DEC direction
        # Curvature has not been taken account
        if self.USE_CASA_TABLE:
            DecCorrection = 1.0 / math.cos(self.DataTable.getcell('DEC',0) / 180.0 * 3.141592653)
        else:
            DecCorrection = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)
        #DecCorrection = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)
        #### GridSpacingRA = GridSpacing * DecCorrection
        if type(Idx[0]) == list:
            IdxTmp = Idx[0][:]
            for i in range(len(Idx)-1):
                IdxTmp += Idx[i+1]
        Idx = IdxTmp

        if self.USE_CASA_TABLE:
            NpRAs = NP.take(self.DataTable.getcol('RA'),Idx)
            NpDECs = NP.take(self.DataTable.getcol('DEC'),Idx)
            NpRMSs = NP.take(self.DataTable.getcol('STATISTICS'),Idx,axis=1)[1,:]
            NpROWs = NP.take(self.DataTable.getcol('ROW'),Idx)
            NpIDXs = NP.array(Idx)
            NpANTs = NP.take(self.DataTable.getcol('ANTENNA'),Idx)
        else:
            NpRAs = NP.zeros(len(Idx), dtype=NP.float64)
            NpDECs = NP.zeros(len(Idx), dtype=NP.float64)
            NpRMSs = NP.zeros(len(Idx), dtype=NP.float64)
            NpROWs = NP.zeros(len(Idx), dtype=NP.int)
            NpIDXs = NP.zeros(len(Idx), dtype=NP.int)
            NpANTs = NP.zeros(len(Idx), dtype=NP.int)
            #for row in rows:
            for index in range(len(Idx)):
                idx = Idx[index]
                NpRAs[index] = self.DataTable[idx][DT_RA]
                NpDECs[index] = self.DataTable[idx][DT_DEC]
                NpRMSs[index] = self.DataTable[idx][DT_STAT][1]
                NpROWs[index] = self.DataTable[idx][DT_ROW]
                NpIDXs[index] = idx
                NpANTs[index] = self.DataTable[idx][DT_ANT]
#        NpRAs = NP.zeros(len(Idx), dtype=NP.float64)
#        NpDECs = NP.zeros(len(Idx), dtype=NP.float64)
#        NpRMSs = NP.zeros(len(Idx), dtype=NP.float64)
#        NpROWs = NP.zeros(len(Idx), dtype=NP.int)
#        NpIDXs = NP.zeros(len(Idx), dtype=NP.int)
#        NpANTs = NP.zeros(len(Idx), dtype=NP.int)
#        #for row in rows:
#        for index in range(len(Idx)):
#            idx = Idx[index]
#            NpRAs[index] = self.DataTable[idx][DT_RA]
#            NpDECs[index] = self.DataTable[idx][DT_DEC]
#            NpRMSs[index] = self.DataTable[idx][DT_STAT][1]
#            NpROWs[index] = self.DataTable[idx][DT_ROW]
#            NpIDXs[index] = idx
#            NpANTs[index] = self.DataTable[idx][DT_ANT]

        if ObsPattern.upper() == 'RASTER':
            ThresholdR = CombineRadius * CombineRadius
            MinRA = NpRAs.min()
            MaxRA = NpRAs.max()
            MinDEC = NpDECs.min()
            MaxDEC = NpDECs.max()
            # Check if the distribution crosses over the RA=0
            if MinRA < 10 and MaxRA > 350:
                NpRAs = NpRAs + NP.less_equal(NpRAs, 180) * 360.0
                MinRA = NpRAs.min()
                MaxRA = NpRAs.max()
            #NGridRA = int((max(RAs) - MinRA) / (GridSpacing * DecCorrection)) + 1
            #NGridDEC = int((max(DECs) - MinDEC) / GridSpacing) + 1
            # (RAcenter, DECcenter) to be the center of the grid
            NGridRA = int((MaxRA - MinRA) / (GridSpacing * DecCorrection)) + 1
            NGridDEC = int((MaxDEC - MinDEC) / GridSpacing) + 1
            MinRA = (MinRA + MaxRA) / 2.0 - (NGridRA - 1) / 2.0 * GridSpacing * DecCorrection
            MinDEC = (MinDEC + MaxDEC) / 2.0 - (NGridDEC - 1) / 2.0 * GridSpacing

            for y in range(NGridDEC):
                if NROW > 10000: print 'Progress:', y, '/', NGridDEC, '(', NGridRA, ')', ' : ', time.ctime()
                DEC = MinDEC + GridSpacing * y
                DeltaDEC = NpDECs - DEC
                SelectD = NP.nonzero(NP.less_equal(DeltaDEC, CombineRadius) * NP.greater_equal(DeltaDEC, -CombineRadius))[0]
                sDeltaDEC = NP.take(DeltaDEC, SelectD)
                sRA = NP.take(NpRAs, SelectD)
                sROW = NP.take(NpROWs, SelectD)
                sIDX = NP.take(NpIDXs, SelectD)
                sRMS = NP.take(NpRMSs, SelectD)
                sANT = NP.take(NpANTs, SelectD)
                self.LogMessage('DEBUG', Origin=origin, Msg='Combine Spectra: %s' % len(sRMS))
                for x in range(NGridRA):
                    RA = MinRA + GridSpacing * DecCorrection * x
                    sDeltaRA = (sRA - RA) / DecCorrection
                    Delta = sDeltaDEC * sDeltaDEC + sDeltaRA * sDeltaRA
                    #Select = NP.less_equal(Delta, ThresholdR)
                    SelectR = NP.nonzero(NP.less_equal(Delta, ThresholdR))[0]
                    if len(SelectR > 0):
                        ssROW = NP.take(sROW, SelectR)
                        ssRMS = NP.take(sRMS, SelectR)
                        ssIDX = NP.take(sIDX, SelectR)
                        ssANT = NP.take(sANT, SelectR)
                        ssDelta = NP.sqrt(NP.take(Delta, SelectR))
                        line = [vIF, vPOL, x, y, RA, DEC, NP.transpose([ssROW, ssDelta, ssRMS, ssIDX, ssANT])]
                    else:
                        line = [vIF, vPOL, x, y, RA, DEC, []]
                    GridTable.append(line)
                    self.LogMessage('LONG', Origin=origin, Msg="GridTable: %s" % line)
            del DEC, DeltaDEC, SelectD, sDeltaDEC, sRA, sROW, sIDX, sRMS, sANT, sDeltaRA, RA, Delta, line

        elif ObsPattern.upper() == 'SINGLE-POINT':
            NGridRA = 1
            NGridDEC = 1
            CenterRA = NpRAs.mean()
            CenterDEC = NpDECs.mean()
            #CenterRA = (NP.array(RAs)).mean()
            #CenterDEC = (NP.array(DECs)).mean()
            line = [vIF, vPOL, 0, 0, CenterRA, CenterDEC, []]
            for x in range(len(Idx)):
                Delta = math.sqrt((NpRAs[x] - CenterRA) ** 2.0 + (NpDECs[x] - CenterDEC) ** 2.0)
                #Delta = math.sqrt((RAs[x] - CenterRA) ** 2.0 + (DECs[x] - CenterDEC) ** 2.0)
                if Delta <= Allowance:
                    line[6].append([Idx[x], Delta, NpRMSs[x], NpIDXs[x], NpANTs[x]])
                    #line[6].append([rows[x], Delta, RMSs[x], IDXs[x]])
            GridTable.append(line)
            self.LogMessage('DEBUG', Origin=origin, Msg="GridTable: %s" % line)

        elif ObsPattern.upper() == 'MULTI-POINT':
            NGridRA = 0
            NGridDEC = 1
            Flag = NP.ones(len(Idx))
            while Flag.sum() > 0:
                for x in range(len(Idx)):
                    if Flag[x] == 1:
                        RA = NpRAs[x]
                        DEC = NpDECs[x]
                        #RA = RAs[x]
                        #DEC = DECs[x]
                        RAtmp = [RA]
                        DECtmp = [DEC]
                        for y in range(x + 1, len(Idx)):
                            if Flag[y] == 1:
                                Delta = math.sqrt((RA - NpRAs[y]) ** 2.0 + (DEC - NpDECs[y]) ** 2.0)
                                #Delta = math.sqrt((RA - RAs[y]) ** 2.0 + (DEC - DECs[y]) ** 2.0)
                                if Delta <= Allowance:
                                    RAtmp.append(NpRAs[y])
                                    DECtmp.append(NpDECs[y])
                                    #RAtmp.append(RAs[y])
                                    #DECtmp.append(DECs[y])
                        CenterRA = (NP.array(RAtmp)).mean()
                        CenterDEC = (NP.array(DECtmp)).mean()
                        line = [vIF, vPOL, 0, NGridRA, CenterRA, CenterDEC, []]
                        NGridRA += 1
                        for x in range(len(Idx)):
                            if Flag[x] == 1:
                                Delta = math.sqrt((NpRAs[x] - CenterRA) ** 2.0 + (NpDECs[x] - CenterDEC) ** 2.0)
                                #Delta = math.sqrt((RAs[x] - CenterRA) ** 2.0 + (DECs[x] - CenterDEC) ** 2.0)
                                if Delta <= Allowance:
                                    line[6].append([Idx[x], Delta, NpRMSs[x], NpIDXs[x], NpANTs[x]])
                                    #line[6].append([rows[x], Delta, RMSs[x], IDXs[x]])
                                    Flag[x] = 0
                        GridTable.append(line)
                        self.LogMessage('DEBUG', Origin=origin, Msg="GridTable: %s" % line)
            del Flag
        else:
            self.LogMessage('ERROR', Origin=origin, Msg='Error: ObsPattern not defined...')
            return False

        del NpRAs, NpDECs, NpRMSs, NpROWs, NpIDXs, NpANTs

        self.LogMessage('INFO', Origin=origin, Msg='NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))
        return GridTable

    @dec_engines_logfile
    @dec_engines_logging
    def Process8(self, DataIn, DataOut, IDX, NCHAN, GridTable, CombineRadius, GridRule, LogLevel=2, LogFile=False):
        """
        The process does re-map and combine spectrum for each position
        GridTable format:
          [[IF,POL,0,0,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]
           [IF,POL,0,1,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]
                        ......
           [IF,POL,M,N,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]]
         where row0,row1,...,rowN should be combined to one for better S/N spectra
               'r' is a distance from grid position
        'weight' can be 'CONST', 'GAUSS', or 'LINEAR'
        'clip' can be 'none' or 'minmaxreject' 
        'rms_weight' is either True or False. If True, NewRMS is used for additional weight
          Number of spectra output is len(GridTable)
        OutputTable format:
           [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
                    ......
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]]

        DataOut is not used 2010/10/25 GK
        """
        origin = 'Process8()'
        
        # create storage
        # 2011/11/12 DataIn and rowsSel are [list]
        NSpIn = len(IDX[0])
        for i in range(len(IDX)-1): NSpIn += len(IDX[i+1])
        SpStorage = NP.zeros((NSpIn, NCHAN), dtype=NP.float32)
        #SpStorage = NP.zeros((len(IDX), NCHAN), dtype=NP.float32)

        if NCHAN != 1:
            clip = GridRule['Clipping']
            rms_weight = GridRule['WeightRMS']
            tsys_weight = GridRule['WeightTsysExptime']
        else:
            clip = 'none'
            rms_weight = False
            tsys_weight = True
        weight = GridRule['WeightDistance']

        NROW = len(GridTable)
        self.LogMessage('INFO', Origin=origin, Msg='Accumulate nearby spectrum for each Grid position...')
        self.LogMessage('INFO', Origin=origin, Msg='Processing %d spectra...' % (NROW))
        OutputTable = []
        # create storage for output
        StorageOut = NP.ones((NROW, NCHAN), dtype=NP.float32) * NoData
        ID = 0

        tROW = None
        tSFLAG = None
        tTSYS = None
        tEXPT = None
        if self.USE_CASA_TABLE:
            tROW = self.DataTable.getcol('ROW')
            tSFLAG = self.DataTable.getcol('FLAG_SUMMARY')
            tTSYS = self.DataTable.getcol('TSYS')
            tEXPT = self.DataTable.getcol('EXPOSURE')

        # 2011/11/12 DataIn and rowsSel are [list]
        IDX2StorageID = {}
        StorageID = 0
        for i in range(len(DataIn)):
            # read data to SpStorage
            self._tb.open(DataIn[i])
            for x in range(len(IDX[i])):
                #SpStorage[x] = self._tb.getcell('SPECTRA', self.DataTable[IDX[x]][DT_ROW])
                if self.USE_CASA_TABLE:
                    SpStorage[StorageID] = self._tb.getcell('SPECTRA', tROW[IDX[i][x]])
                else:
                    SpStorage[StorageID] = self._tb.getcell('SPECTRA', self.DataTable[IDX[i][x]][DT_ROW])
                #SpStorage[StorageID] = self._tb.getcell('SPECTRA', self.DataTable[IDX[i][x]][DT_ROW])
                IDX2StorageID[IDX[i][x]] = StorageID
                #IDX2StorageID[IDX[x]] = x
                StorageID += 1
            self.LogMessage('DEBUG', Origin=origin, Msg='Data Stored in SpStorage')
            self._tb.close()

        # Create progress timer
        Timer = ProgressTimer(80, NROW, LogLevel)
        for [IF, POL, X, Y, RAcent, DECcent, RowDelta] in GridTable:
            rowlist = []
            indexlist = []
            deltalist = []
            rmslist = []
            flagged = 0
            for [row, delta, rms, index, ant] in RowDelta:
                # Check Summary Flag
                if self.USE_CASA_TABLE:
                    if tSFLAG[int(index)] == 1:
                    #if self.DataTable[row][DT_SFLAG] == 1:
                        rowlist.append(int(row))
                        indexlist.append(IDX2StorageID[int(index)])
                        deltalist.append(delta)
                        rmslist.append(rms)
                    else: flagged += 1
                else:
                    if self.DataTable[index][DT_SFLAG] == 1:
                    #if self.DataTable[row][DT_SFLAG] == 1:
                        rowlist.append(row)
                        indexlist.append(IDX2StorageID[index])
                        deltalist.append(delta)
                        rmslist.append(rms)
                    else: flagged += 1
                #if self.DataTable[index][DT_SFLAG] == 1:
                ##if self.DataTable[row][DT_SFLAG] == 1:
                #    rowlist.append(row)
                #    indexlist.append(IDX2StorageID[index])
                #    deltalist.append(delta)
                #    rmslist.append(rms)
                #else: flagged += 1
            if len(rowlist) == 0:
                # No valid Spectra at the position
                RMS = 0.0
                pass
            elif len(rowlist) == 1:
                # One valid Spectrum at the position
                #StorageOut[ID] = SpStorage[rowlist[0]]
                StorageOut[ID] = SpStorage[indexlist[0]]
                RMS = rmslist[0]
            else:
                # More than one valid Spectra at the position
                #data = SpStorage[rowlist].copy()
                data = SpStorage[indexlist].copy()
                #print data
                #w = NP.ones(NP.shape(data), NP.float64)
                #weightlist = NP.ones(len(rowlist), NP.float64)
                w = NP.ones(NP.shape(data), NP.float32)
                weightlist = NP.ones(len(rowlist), NP.float32)
                # Clipping
                if clip.upper() == 'MINMAXREJECT' and len(rowlist) > 2:
                    w[NP.argmin(data, axis=0), range(len(data[0]))] = 0.0
                    w[NP.argmax(data, axis=0), range(len(data[0]))] = 0.0
                # Weight by RMS
                # Weight = 1/(RMS**2)
                if rms_weight == True:
                    for m in range(len(rowlist)):
                        if rmslist[m] != 0.0:
                            w[m] /= (rmslist[m]**2)
                            weightlist[m] /= (rmslist[m]**2)
                        else:
                            w[m] *= 0.0
                            weightlist[m] *= 0.0
                # Weight by Exptime & Tsys
                # RMS = n * Tsys/sqrt(Exptime)
                # Weight = 1/(RMS**2) = (Exptime/(Tsys**2))
                if tsys_weight == True:
                    for m in range(len(rowlist)):
                        # 2008/9/21 Bug fix
                        if self.USE_CASA_TABLE:
                            if tTSYS[rowlist[m]] > 0.5:
                                w[m] *= (tEXPT[rowlist[m]]/(tTSYS[rowlist[m]]**2))
                                weightlist[m] *= (tEXPT[rowlist[m]]/(tTSYS[rowlist[m]]**2))
                            #if self.DataTable[m][DT_TSYS] > 0.5:
                            #    w[m] *= (self.DataTable[m][DT_EXPT]/(self.DataTable[m][DT_TSYS]**2))
                            #    weightlist[m] *= (self.DataTable[m][DT_EXPT]/(self.DataTable[m][DT_TSYS]**2))
                            else:
                                w[m] *= 0.0
                                weightlist[m] = 0.0
                        else:
                            if self.DataTable[rowlist[m]][DT_TSYS] > 0.5:
                                w[m] *= (self.DataTable[rowlist[m]][DT_EXPT]/(self.DataTable[rowlist[m]][DT_TSYS]**2))
                                weightlist[m] *= (self.DataTable[rowlist[m]][DT_EXPT]/(self.DataTable[rowlist[m]][DT_TSYS]**2))
                            #if self.DataTable[m][DT_TSYS] > 0.5:
                            #    w[m] *= (self.DataTable[m][DT_EXPT]/(self.DataTable[m][DT_TSYS]**2))
                            #    weightlist[m] *= (self.DataTable[m][DT_EXPT]/(self.DataTable[m][DT_TSYS]**2))
                            else:
                                w[m] *= 0.0
                                weightlist[m] = 0.0
                        #if self.DataTable[rowlist[m]][DT_TSYS] > 0.5:
                        #    w[m] *= (self.DataTable[rowlist[m]][DT_EXPT]/(self.DataTable[rowlist[m]][DT_TSYS]**2))
                        #    weightlist[m] *= (self.DataTable[rowlist[m]][DT_EXPT]/(self.DataTable[rowlist[m]][DT_TSYS]**2))
                        ##if self.DataTable[m][DT_TSYS] > 0.5:
                        ##    w[m] *= (self.DataTable[m][DT_EXPT]/(self.DataTable[m][DT_TSYS]**2))
                        ##    weightlist[m] *= (self.DataTable[m][DT_EXPT]/(self.DataTable[m][DT_TSYS]**2))
                        #else:
                        #    w[m] *= 0.0
                        #    weightlist[m] = 0.0
                # Weight by radius
                if weight.upper() == 'GAUSS':
                    # weight = exp(-ln2*((r/hwhm)**2))
                    for m in range(len(rowlist)):
                        w[m] *= (math.exp(-0.69314718055994529*((deltalist[m]/CombineRadius)**2)))
                        weightlist[m] *= (math.exp(-0.69314718055994529*((deltalist[m]/CombineRadius)**2)))
                elif weight.upper() == 'LINEAR':
                    # weight = 0.5 + (hwhm - r)/2/hwhm = 1.0 - r/2/hwhm
                    for m in range(len(rowlist)):
                        w[m] *= (1.0 - deltalist[m]/2.0/CombineRadius)
                        weightlist[m] *= (1.0 - deltalist[m]/2.0/CombineRadius)
                # Combine Spectra
                if w.sum() != 0: StorageOut[ID] = (NP.sum(data * w, axis=0) / NP.sum(w,axis=0))
                # Calculate RMS of the spectrum
                r0 = 0.0
                r1 = 0.0
                for m in range(len(rowlist)):
                    r0 += (rmslist[m] * weightlist[m]) ** 2
                    r1 += weightlist[m]
                RMS = (r0**0.5) / r1
                del data, w, weightlist

            OutputTable.append([IF, POL, X, Y, RAcent, DECcent, len(rowlist), flagged, RMS])
            ID += 1
            del rowlist, indexlist, deltalist, rmslist

            # Countup progress timer
            Timer.count()

        del SpStorage, Timer, IDX2StorageID

        return (StorageOut, OutputTable)

    @dec_engines_logfile
    @dec_engines_logging
    def SimpleGridding(self, DataIn, DataOut, NCHAN, GridTable, LogLevel=2, LogFile=False):
        """
        The process does re-map and combine spectrum for each position
        GridTable format:
          [[IF,POL,0,0,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]
           [IF,POL,0,1,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]
                        ......
           [IF,POL,M,N,RAcent,DECcent,[[row0,r0,RMS0,index0,ant0],[row1,r1,RMS1,index1,ant1],..,[rowN,rN,RMSN,indexN,antn]]]]
         where row0,row1,...,rowN should be combined to one for better S/N spectra
               'r' is a distance from grid position
          Number of spectra output is len(GridTable)
        OutputTable format:
           [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
                    ......
            [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]]

        """
        origin = 'SimpleGridding()'
        
        #tbtool = casac.homefinder.find_home_by_name('tableHome')
        #tbIn = tbtool.create()
        tbIn = gentools(['tb'])[0]

        NROW = len(GridTable)
        self.LogMessage('INFO', Origin=origin, Msg='SimpleGridding() start...')
        self.LogMessage('INFO', Origin=origin, Msg='Processing %d spectra...' % (NROW))
        # create storage for output
        StorageOut = NP.zeros((NROW, NCHAN), dtype=NP.float32)
        StorageWeight = NP.zeros((NROW), dtype=NP.float32)
        StorageNumSp = NP.zeros((NROW), dtype=NP.int)
        StorageNumFlag = NP.zeros((NROW), dtype=NP.int)
        StorageNoData = NP.ones((NCHAN), dtype=NP.float32) * NoData
        OutputTable = []

        tTSYS = None
        tEXPT = None
        tROW = None
        tSFLAG = None
        if self.USE_CASA_TABLE:
            tROW = self.DataTable.getcol('ROW')
            tTSYS = self.DataTable.getcol('TSYS')
            tEXPT = self.DataTable.getcol('EXPOSURE')
            tSFLAG = self.DataTable.getcol('FLAG_SUMMARY')
            
        # Create progress timer
        Timer = ProgressTimer(80, NROW*len(DataIn), LogLevel)
        # loop for antennas
        for AntID in DataIn.keys():
            tbIn.open(DataIn[AntID])
            # loop for all ROWs in GridTable
            for ROW in range(NROW):
                [IF, POL, X, Y, RAcent, DECcent, RowDelta] = GridTable[ROW]
                for [row, delta, rms, index, ant] in RowDelta:
                    if ant == AntID:
                        # Check Summary Flag: 1: valid, 0: flagged
                        if self.USE_CASA_TABLE:
                            if tSFLAG[index] == 1:
                                Sp = NP.array(tbIn.getcell('SPECTRA', tROW[index]))
                                # Weight for Tsys and ExpTime only
                                if tTSYS[index] > 0.5 and tEXPT[index] > 0.0:
                                    Weight = tEXPT[index]/(tTSYS[index]**2.0)
                                else: Weight = 1.0
                                StorageOut[ROW] += Sp * Weight
                                StorageWeight[ROW] += Weight
                                StorageNumSp[ROW] += 1
                            else: StorageNumFlag[ROW] += 1
                        else:
                            if self.DataTable[index][DT_SFLAG] == 1:
                                Sp = NP.array(tbIn.getcell('SPECTRA', self.DataTable[index][DT_ROW]))
                                # Weight for Tsys and ExpTime only
                                if self.DataTable[index][DT_TSYS] > 0.5 and self.DataTable[index][DT_EXPT] > 0.0:
                                    Weight = self.DataTable[index][DT_EXPT]/(self.DataTable[index][DT_TSYS]**2.0)
                                else: Weight = 1.0
                                StorageOut[ROW] += Sp * Weight
                                StorageWeight[ROW] += Weight
                                StorageNumSp[ROW] += 1
                            else: StorageNumFlag[ROW] += 1
##                         if self.DataTable[index][DT_SFLAG] == 1:
##                             Sp = NP.array(tbIn.getcell('SPECTRA', self.DataTable[index][DT_ROW]))
##                             # Weight for Tsys and ExpTime only
##                             if self.DataTable[index][DT_TSYS] > 0.5 and self.DataTable[index][DT_EXPT] > 0.0:
##                                 Weight = self.DataTable[index][DT_EXPT]/(self.DataTable[index][DT_TSYS]**2.0)
##                             else: Weight = 1.0
##                             StorageOut[ROW] += Sp * Weight
##                             StorageWeight[ROW] += Weight
##                             StorageNumSp[ROW] += 1
##                         else: StorageNumFlag[ROW] += 1
                # Countup progress timer
                Timer.count()
            tbIn.close()

        # Calculate Tsys-ExpTime weighted average
        # RMS = n * Tsys/sqrt(Exptime)
        # Weight = 1/(RMS**2) = (Exptime/(Tsys**2))
        for ROW in range(NROW):
            [IF, POL, X, Y, RAcent, DECcent, RowDelta] = GridTable[ROW]
            if StorageNumSp[ROW] == 0:
                StorageOut[ROW] = StorageNoData
                RMS = 0.0
            else:
                StorageOut[ROW] /= StorageWeight[ROW]
                RMS = 1.0
            OutputTable.append([IF, POL, X, Y, RAcent, DECcent, StorageNumSp[ROW], StorageNumFlag[ROW], RMS])

        del StorageWeight, StorageNumSp, StorageNumFlag, StorageNoData, Timer
        return (StorageOut, OutputTable)

    @dec_engines_logfile
    @dec_engines_logging
    def BaselineSubAtRasterEdge(self, DataIn, DataOut, TimeTableList, rows, FitOrderRule=None, SwitchingRule=None, LogLevel=2, LogFile=False):
        """
        Baseline calculated by the edge of the raster scan was Subtracted from the rest of the spectra
        """
        origin = 'BaselineSubAtRasterEdge()'
 
        NROW = len(rows)
        if self.USE_CASA_TABLE:
            NCHAN = self.DataTable.getcell('NCHAN',rows[0])
        else:
            NCHAN = self.DataTable[rows[0]][DT_NCHAN]
        #NCHAN = self.DataTable[rows[0]][DT_NCHAN]
        # create storage
        ### 2011/05/26 not use SpStorage
        #SpStorage = NP.zeros((NROW, NCHAN), dtype=NP.float32)
        SpOned = NP.zeros(NCHAN, dtype=NP.float32)

        self.LogMessage('INFO', Origin=origin, Msg='Applied time bin: %s' % FitOrderRule['ApplicableDuration'])
        self.LogMessage('INFO', Origin=origin, Msg='Calibration Formula: %s' % SwitchingRule['CalibrationFormula'])
        self.LogMessage('INFO', Origin=origin, Msg='Processing %d spectra...' % NROW)

        ### 2011/05/26 not use SpStorage
        # read data to SpStorage
        #self._tb.open(DataIn)
        ReverseTable = NP.zeros(max(rows)+1, NP.int)
        for x in range(NROW):
            #SpStorage[x] = self._tb.getcell('SPECTRA', rows[x])
            ReverseTable[rows[x]] = x
        #self.LogMessage('DEBUG', Origin=origin, Msg='Data Stored in SpStorage')
        #self._tb.close()
        #self._tb.open(DataOut, nomodify=False)

        #tbtool = casac.homefinder.find_home_by_name('tableHome')
        #tbIn = tbtool.create()
        #tbOut = tbtool.create()
        tbIn,tbOut = gentools(['tb','tb'])
        tbIn.open(DataIn)
        tbOut.open(DataOut, nomodify=False)

        # Select time gap list: 'subscan': large gap; 'raster': small gap
        if FitOrderRule['ApplicableDuration'] == 'subscan':
            TimeTable = TimeTableList[1]
        else:
            TimeTable = TimeTableList[0]

        GroupList = TimeTable
        self.LogMessage('LONG', Origin=origin, Msg='GroupList:%s' % GroupList)

        tELAPSED = None
        if self.USE_CASA_TABLE:
            tELAPSED = self.DataTable.getcol('ELAPSED')

        for chunks in GroupList:
            chunk = chunks[0]
            self.LogMessage('DEBUG', Origin=origin, Msg='Processing spectra = %s' % chunk)

            ### 2011/05/17 to improve S/N for Off-spectrum
            NumOffSpectra = SwitchingRule['NumOffSpectra'] 
            if NumOffSpectra > 2:
                if len(chunk) > 6:
                    T0 = T1 = 0.0
                    TmpStorage0 = NP.zeros((NumOffSpectra, NCHAN), dtype=NP.float32)
                    TmpStorage1 = NP.zeros((NumOffSpectra, NCHAN), dtype=NP.float32)
                    for i in range(NumOffSpectra):
                        if self.USE_CASA_TABLE:
                            T0 += tELAPSED[chunk[i]]
                            T1 += tELAPSED[chunk[-(i+1)]]
                        else:
                            T0 += self.DataTable[chunk[i]][DT_ELAPSED]
                            T1 += self.DataTable[chunk[-(i+1)]][DT_ELAPSED]
                        #T0 += self.DataTable[chunk[i]][DT_ELAPSED]
                        #T1 += self.DataTable[chunk[-(i+1)]][DT_ELAPSED]
                        ### 2011/05/26 not use SpStorage
                        TmpStorage0[i] = tbIn.getcell('SPECTRA', chunk[i])
                        TmpStorage1[i] = tbOut.getcell('SPECTRA', chunk[-(i+1)])
                        # 2010/11/2 Apply flag (User Flag) to the spectra at the edge of the Raster
                        # 2011/05/17 moved here but not necessary
                        #self.DataTable[chunk[i]][DT_PFLAG][2] = 0
                        #self.DataTable[chunk[-(i+1)]][DT_PFLAG][2] = 0
                    T0 /= float(NumOffSpectra)
                    T1 /= float(NumOffSpectra)
                    if T0 == T1: T1 = T0 + 1.0
                    # MinMax Rejection
                    w0 = NP.ones(NP.shape(TmpStorage0), NP.float32)
                    w1 = NP.ones(NP.shape(TmpStorage1), NP.float32)
                    w0[NP.argmin(TmpStorage0, axis=0), range(NCHAN)] = 0.0
                    w0[NP.argmax(TmpStorage0, axis=0), range(NCHAN)] = 0.0
                    w1[NP.argmin(TmpStorage1, axis=0), range(NCHAN)] = 0.0
                    w1[NP.argmax(TmpStorage1, axis=0), range(NCHAN)] = 0.0
                    Sp0 = NP.sum(TmpStorage0 * w0, axis=0) / float(NumOffSpectra - 2)
                    Sp1 = NP.sum(TmpStorage1 * w1, axis=0) / float(NumOffSpectra - 2)
                else: NumOffSpectra = 2
            if NumOffSpectra == 2:
                if len(chunk) > 4:
                    if self.USE_CASA_TABLE:
                        T0 = (tELAPSED[chunk[0]] + tELAPSED[chunk[1]]) / 2.0
                        T1 = (tELAPSED[chunk[-1]] + tELAPSED[chunk[-2]]) / 2.0
                    else:
                        T0 = (self.DataTable[chunk[0]][DT_ELAPSED] + self.DataTable[chunk[1]][DT_ELAPSED]) / 2.0
                        T1 = (self.DataTable[chunk[-1]][DT_ELAPSED] + self.DataTable[chunk[-2]][DT_ELAPSED]) / 2.0
                    #T0 = (self.DataTable[chunk[0]][DT_ELAPSED] + self.DataTable[chunk[1]][DT_ELAPSED]) / 2.0
                    #T1 = (self.DataTable[chunk[-1]][DT_ELAPSED] + self.DataTable[chunk[-2]][DT_ELAPSED]) / 2.0
                    if T0 == T1: T1 = T0 + 1.0
                    ### 2011/05/26 not use SpStorage
                    Sp0 = (tbIn.getcell('SPECTRA', chunk[0]) + tbIn.getcell('SPECTRA', chunk[1])) / 2.0
                    Sp1 = (tbIn.getcell('SPECTRA', chunk[-1]) + tbIn.getcell('SPECTRA', chunk[-2])) / 2.0
                    # 2010/11/2 Apply flag (User Flag) to the spectra at the edge of the Raster
                    # 2011/05/17 moved here
                    if self.USE_CASA_TABLE:
                        tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',chunk[0])
                        tPFLAG[2] = 0
                        self.DataTable.putcell('FLAG_PERMANENT',chunk[0],tPFLAG)
                        tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',chunk[1])
                        tPFLAG[2] = 0
                        self.DataTable.putcell('FLAG_PERMANENT',chunk[1],tPFLAG)
                        tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',chunk[-1])
                        tPFLAG[2] = 0
                        self.DataTable.putcell('FLAG_PERMANENT',chunk[-1],tPFLAG)
                        tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',chunk[-2])
                        tPFLAG[2] = 0
                        self.DataTable.putcell('FLAG_PERMANENT',chunk[-2],tPFLAG)
                    else:
                        self.DataTable[chunk[0]][DT_PFLAG][2] = 0
                        self.DataTable[chunk[1]][DT_PFLAG][2] = 0
                        self.DataTable[chunk[-1]][DT_PFLAG][2] = 0
                        self.DataTable[chunk[-2]][DT_PFLAG][2] = 0
                    #self.DataTable[chunk[0]][DT_PFLAG][2] = 0
                    #self.DataTable[chunk[1]][DT_PFLAG][2] = 0
                    #self.DataTable[chunk[-1]][DT_PFLAG][2] = 0
                    #self.DataTable[chunk[-2]][DT_PFLAG][2] = 0
                else: NumOffSpectra = 1
            if NumOffSpectra == 1:
                if self.USE_CASA_TABLE:
                    T0 = tELAPSED[chunk[0]]
                    T1 = tELAPSED[chunk[-1]]
                else:
                    T0 = self.DataTable[chunk[0]][DT_ELAPSED]
                    T1 = self.DataTable[chunk[-1]][DT_ELAPSED]
                #T0 = self.DataTable[chunk[0]][DT_ELAPSED]
                #T1 = self.DataTable[chunk[-1]][DT_ELAPSED]
                if T0 == T1: T1 = T0 + 1.0
                ### 2011/05/26 not use SpStorage
                Sp0 = tbIn.getcell('SPECTRA', chunk[0])
                Sp1 = tbIn.getcell('SPECTRA', chunk[-1])
                # 2010/11/2 Apply flag (User Flag) to the spectra at the edge of the Raster
                # 2011/05/17 moved here
                if self.USE_CASA_TABLE:
                    tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',chunk[0])
                    tPFLAG[2] = 0
                    self.DataTable.putcell('FLAG_PERMANENT',chunk[0],tPFLAG)
                    tPFLAG = self.DataTable.getcell('FLAG_PERMANENT',chunk[-1])
                    tPFLAG[2] = 0
                    self.DataTable.putcell('FLAG_PERMANENT',chunk[-1],tPFLAG)
                else:
                    self.DataTable[chunk[0]][DT_PFLAG][2] = 0
                    self.DataTable[chunk[-1]][DT_PFLAG][2] = 0
                #self.DataTable[chunk[0]][DT_PFLAG][2] = 0
                #self.DataTable[chunk[-1]][DT_PFLAG][2] = 0

            if SwitchingRule['CalibrationFormula'].upper() == 'ON-OFF': 
                for row in chunk:
                    if self.USE_CASA_TABLE:
                        T = tELAPSED[row]
                    else:
                        T = self.DataTable[row][DT_ELAPSED]
                    #T = self.DataTable[row][DT_ELAPSED]
                    Sp = tbIn.getcell('SPECTRA', row) - \
                         (Sp0 * (T1 - T) + Sp1 * (T - T0)) / (T1 - T0)
                    tbOut.putcell('SPECTRA', int(row), Sp)
            else: 
                for row in chunk:
                    if self.USE_CASA_TABLE:
                        T = tELAPSED[row]
                    else:
                        T = self.DataTable[row][DT_ELAPSED]
                    #T = self.DataTable[row][DT_ELAPSED]
                    SpOff = (Sp0 * (T1 - T) + Sp1 * (T - T0)) / (T1 - T0)
                    Sp = (tbIn.getcell('SPECTRA', row) - SpOff) / SpOff
                    tbOut.putcell('SPECTRA', int(row), Sp)

        tbIn.close()
        tbOut.flush()
        tbOut.close()


    def selectGroupData( self, vAnt, vIF, vPOL, ROWbase, ROW, SCANbase, SCAN ):
        """
        """
        #print '\nvAnt', vAnt
        #print '\nvIF', vIF
        #print '\nvPOL', vPOL
        #print '\nvROW', ROW
        #print '\nvSCANbase', SCANbase
        #print '\nvSCAN', SCAN, '\n'
        #print '\nself.listall', self.listall, '\n'
        #print '\nlen(ROWbase,ROW,SCANbase,SCAN)', len(ROWbase), len(ROW), len(SCANbase), len(SCAN)
        #print 'len(self.DataTable)', len(self.DataTable)
        #print 'ROW[0],[-1]', ROW[0], ROW[-1]
        #print 'ROWbase[0],[-1]', ROWbase[0], ROWbase[-1]
        #print 'SCANbase[0],[-1]', SCANbase[0], SCANbase[-1]
        #print 'SCAN[0],[-1]', SCAN[0], SCAN[-1]
        #print 'len, self.listall[vAnt][0][0],[-1]', len(self.listall[vAnt][0]), self.listall[vAnt][0][0], self.listall[vAnt][0][-1], '\n'
        rowsSel = []
        rowsidSel = []
        basedataSel = []
        basedataidSel = []
        ROWbase.sort()
        ROW.sort()
        indexROWb = 0
        indexROW = 0
        indexSCANb = 0
        indexSCAN = 0
        Counter = 0
        if self.USE_CASA_TABLE:
            tIF = self.DataTable.getcol('IF')
            tPOL = self.DataTable.getcol('POL')
            tANT = self.DataTable.getcol('ANTENNA')
            tROW = self.DataTable.getcol('ROW')
            for ID in self.listall[vAnt][0]:
                if Counter % 10000 == 0: print 'Processing ID:', ID, time.ctime()
                if tIF[ID] == vIF and tPOL[ID] == vPOL and tANT[ID] == vAnt:
                    row = tROW[ID]
                    if row in ROWbase[indexROWb:]:
                    #if ID in ROWbase[indexROWb:]:
                        while row != ROWbase[indexROWb]: indexROWb += 1
                        #while ID != ROWbase[indexROWb]: indexROWb += 1
                        #if self.DataTable[ID][DT_SCAN] in SCANbase[indexSCANb:]:
                        #    while self.DataTable[ID][DT_SCAN] != SCANbase[indexSCANb]: indexSCANb += 1
                        basedataSel.append(row)
                        #basedataSel.append(self.DataTable[ID][DT_ROW])
                        basedataidSel.append(ID)
                    if row in ROW[indexROW:]:
                    #if ID in ROW[indexROW:]:
                        while row != ROW[indexROW]: indexROW += 1
                        #while ID != ROW[indexROW]: indexROW += 1
                        #if self.DataTable[ID][DT_SCAN] in SCAN[indexSCAN:]:
                        #    while self.DataTable[ID][DT_SCAN] != SCAN[indexSCAN]: indexSCAN += 1
                        rowsSel.append(row)
                        #rowsSel.append(self.DataTable[ID][DT_ROW])
                        rowsidSel.append(ID)
                Counter += 1
        else:
            for ID in self.listall[vAnt][0]:
                if Counter % 10000 == 0: print 'Processing ID:', ID, time.ctime()
                if self.DataTable[ID][DT_IF] == vIF and self.DataTable[ID][DT_POL] == vPOL and self.DataTable[ID][DT_ANT] == vAnt:
                    row = self.DataTable[ID][DT_ROW]
                    if row in ROWbase[indexROWb:]:
                    #if ID in ROWbase[indexROWb:]:
                        while row != ROWbase[indexROWb]: indexROWb += 1
                        #while ID != ROWbase[indexROWb]: indexROWb += 1
                        #if self.DataTable[ID][DT_SCAN] in SCANbase[indexSCANb:]:
                        #    while self.DataTable[ID][DT_SCAN] != SCANbase[indexSCANb]: indexSCANb += 1
                        basedataSel.append(row)
                        #basedataSel.append(self.DataTable[ID][DT_ROW])
                        basedataidSel.append(ID)
                    if row in ROW[indexROW:]:
                    #if ID in ROW[indexROW:]:
                        while row != ROW[indexROW]: indexROW += 1
                        #while ID != ROW[indexROW]: indexROW += 1
                        #if self.DataTable[ID][DT_SCAN] in SCAN[indexSCAN:]:
                        #    while self.DataTable[ID][DT_SCAN] != SCAN[indexSCAN]: indexSCAN += 1
                        rowsSel.append(row)
                        #rowsSel.append(self.DataTable[ID][DT_ROW])
                        rowsidSel.append(ID)
                Counter += 1
##         for ID in self.listall[vAnt][0]:
##             if Counter % 10000 == 0: print 'Processing ID:', ID, time.ctime()
##             if self.DataTable[ID][DT_IF] == vIF and self.DataTable[ID][DT_POL] == vPOL and self.DataTable[ID][DT_ANT] == vAnt:
##                 row = self.DataTable[ID][DT_ROW]
##                 if row in ROWbase[indexROWb:]:
##                 #if ID in ROWbase[indexROWb:]:
##                     while row != ROWbase[indexROWb]: indexROWb += 1
##                     #while ID != ROWbase[indexROWb]: indexROWb += 1
##                     #if self.DataTable[ID][DT_SCAN] in SCANbase[indexSCANb:]:
##                     #    while self.DataTable[ID][DT_SCAN] != SCANbase[indexSCANb]: indexSCANb += 1
##                     basedataSel.append(row)
##                     #basedataSel.append(self.DataTable[ID][DT_ROW])
##                     basedataidSel.append(ID)
##                 if row in ROW[indexROW:]:
##                 #if ID in ROW[indexROW:]:
##                     while row != ROW[indexROW]: indexROW += 1
##                     #while ID != ROW[indexROW]: indexROW += 1
##                     #if self.DataTable[ID][DT_SCAN] in SCAN[indexSCAN:]:
##                     #    while self.DataTable[ID][DT_SCAN] != SCAN[indexSCAN]: indexSCAN += 1
##                     rowsSel.append(row)
##                     #rowsSel.append(self.DataTable[ID][DT_ROW])
##                     rowsidSel.append(ID)
##             Counter += 1

        #print '\nbasedataSel', basedataSel
        #print '\nbasedataidSel', basedataidSel
        #print '\nrowsSel', rowsSel
        #print '\nrowsidSel', rowsidSel, '\n'
       
        return (basedataSel, basedataidSel, rowsSel, rowsidSel)

    

    def isTP( self, rowsSel ):
        """
        Check if the current IF is total power (1ch) or not.
        """
        isTP = None
        if self.USE_CASA_TBLE:
            if self.DataTable.getcell('NCHAN',rowsSel) == 1:
                isTP = True
            else:
                isTP = False
        else:
            if self.DataTable[rowsSel][DT_NCHAN] == 1:
                isTP = True
            else:
                isTP = False
        #if self.DataTable[rowsSel][DT_NCHAN] == 1:
        #    isTP = True
        #else:
        #    isTP = False
            
        return isTP



    ### Plotter ###
    def DrawRADEC(self, rows, vAnt, connect=True, circle=[], ObsPattern=False, ShowPlot=True, FigFile=False):
        """
        Draw loci of the telescope pointing
        xaxis: extension header keyword for RA
        yaxis: extension header keyword for DEC
        connect: connect points if True
        """
        origin = 'DrawRADEC()'

        if ShowPlot == False and FigFile == False: return

        tRA = None
        tDEC = None
        tNCHAN = None
        if self.USE_CASA_TABLE:
            tRA = self.DataTable.getcol('RA')
            tDEC = self.DataTable.getcol('DEC')
            tNCHAN = self.DataTable.getcol('NCHAN')
        
        # Extract RA and DEC
        RA = []
        DEC = []
        if self.USE_CASA_TABLE:
            for row in rows:
                ID = self.Row2ID[vAnt][row]
                # remove TP data
                if tNCHAN[ID] > 1:
                    RA.append(tRA[ID])
                    DEC.append(tDEC[ID])
        else:
            for row in rows:
                ID = self.Row2ID[vAnt][row]
                # remove TP data
                if self.DataTable[ID][DT_NCHAN] > 1:
                    RA.append(self.DataTable[ID][DT_RA])
                    DEC.append(self.DataTable[ID][DT_DEC])
##         for row in rows:
##             ID = self.Row2ID[vAnt][row]
##             # remove TP data
##             if self.DataTable[ID][DT_NCHAN] > 1:
##                 RA.append(self.DataTable[ID][DT_RA])
##                 DEC.append(self.DataTable[ID][DT_DEC])
        span = max(max(RA) - min(RA), max(DEC) - min(DEC))
        xmax = min(RA) - span / 10.0
        xmin = max(RA) + span / 10.0
        ymax = max(DEC) + span / 10.0
        ymin = min(DEC) - span / 10.0
        (RAlocator, DEClocator, RAformatter, DECformatter) = SDP.RADEClabel(span)

        # 2008/9/20 DEC Effect
        if self.USE_CASA_TABLE:
            Aspect = 1.0 / math.cos(tDEC[0] / 180.0 * 3.141592653)
        else:
            Aspect = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)
        #Aspect = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)

        # Plotting routine
        if ShowPlot: PL.ion()
        PL.figure(SDP.MATPLOTLIB_FIGURE_ID[1])
        if ShowPlot: PL.ioff()
        if connect is True: Mark = 'g-o'
        else: Mark = 'bo'
        PL.cla()
        PL.clf()
        a = PL.axes([0.15, 0.2, 0.7, 0.7])
        # 2008/9/20 DEC Effect
        a.set_aspect(Aspect)
        #a.set_aspect('equal')
        PL.xlabel('RA')
        PL.ylabel('Dec')
        if ObsPattern == False:
            PL.title('Telescope Pointing on the Sky')
        else:
            PL.title('Telescope Pointing on the Sky\nPointing Pattern = %s' % ObsPattern)
        PL.plot(RA, DEC, Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
        a.xaxis.set_major_formatter(RAformatter)
        a.yaxis.set_major_formatter(DECformatter)
        a.xaxis.set_major_locator(RAlocator)
        a.yaxis.set_major_locator(DEClocator)
        xlabels = a.get_xticklabels()
        PL.setp(xlabels, 'rotation', SDP.RArotation, fontsize=8)
        ylabels = a.get_yticklabels()
        PL.setp(ylabels, 'rotation', SDP.DECrotation, fontsize=8)
        # plot starting position with beam and end position 
        if len(circle) != 0:
            for R in circle:
                Mark = 'r-'
                x = []
                y = []
                for t in range(50):
                    # 2008/9/20 DEC Effect
                    x.append(RA[0] + R * math.sin(t * 0.13)  * Aspect)
                    #x.append(RA[0] + R * math.sin(t * 0.13))
                    y.append(DEC[0] + R * math.cos(t * 0.13))
                PL.plot(x, y, Mark)
                #Mark = 'm-'
                Mark = 'ro'
                x = []
                y = []
                x.append(RA[-1])
                y.append(DEC[-1])
                PL.plot(x, y, Mark, markersize=4, markeredgecolor='r', markerfacecolor='r')
        PL.axis([xmin, xmax, ymin, ymax])
        if ShowPlot != False: PL.draw()
        if FigFile != False: PL.savefig(FigFile, format='png', dpi=SDP.DPISummary)

        del RA, DEC, x, y
        return


    def DrawMultiSpectraNavigator(self, vIF, vPOL, iteration, Table, rows, Spacing, ObsPattern=False, ShowPlot=False, FigFileDir=False, FigFileRoot=False):
        """
        Draw loci of the telescope pointing
        xaxis: extension header keyword for RA
        yaxis: extension header keyword for DEC
        connect: connect points if True
        Table: [[IF, POL, X, Y, RAcent, DECcent, CombinedSP, flaggedSP, RMS]
                           ...
                [IF, POL, X, Y, RAcent, DECcent, CombinedSP, flaggedSP, RMS]]
        """
        origin = 'DrawMultiSpectraNavigator()'

        if FigFileDir == False: return
        if ObsPattern.upper() != 'RASTER' or len(Table) <= 1: return

        # Extract RA and DEC
        if self.USE_CASA_TABLE:
            RA = NP.take(self.DataTable.getcol('RA'),rows)
            DEC = NP.take(self.DataTable.getcol('DEC'),rows)
        else:
            RA = []
            DEC = []
            for ID in rows:
                RA.append(self.DataTable[ID][DT_RA])
                DEC.append(self.DataTable[ID][DT_DEC])
##         RA = []
##         DEC = []
##         for ID in rows:
##             RA.append(self.DataTable[ID][DT_RA])
##             DEC.append(self.DataTable[ID][DT_DEC])
        span = max(max(RA) - min(RA), max(DEC) - min(DEC))
        xmax = min(RA) - span / 10.0
        xmin = max(RA) + span / 10.0
        ymax = max(DEC) + span / 10.0
        ymin = min(DEC) - span / 10.0
        (RAlocator, DEClocator, RAformatter, DECformatter) = SDP.RADEClabel(span)

        # 2008/9/20 DEC Effect
        if self.USE_CASA_TABLE:
            Aspect = 1.0 / math.cos(self.DataTable.getcell('DEC',0) / 180.0 * 3.141592653)
        else:
            Aspect = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)
        #Aspect = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)

        # Plotting routine
        if ShowPlot: PL.ion()
        PL.figure(SDP.MATPLOTLIB_FIGURE_ID[1])
        if ShowPlot: PL.ioff()
        Mark = 'bo'
        PL.cla()
        PL.clf()
        a = PL.axes([0.15, 0.2, 0.7, 0.7])

        # Plot  Rectangles
        (Xmin, Xmax) = (Table[0][2], Table[0][2])
        (Ymin, Ymax) = (Table[0][3], Table[0][3])
        (RAupperleft, DECupperleft) = (Table[0][4], Table[0][5])
        for idx in range(1, len(Table)):
            if Xmin > Table[idx][2]: Xmin = Table[idx][2]
            elif Xmax < Table[idx][2]:
                Xmax = Table[idx][2]
                RAupperleft = Table[idx][4]
            if Ymin > Table[idx][3]: Ymin = Table[idx][3]
            elif Ymax < Table[idx][3]:
                Ymax = Table[idx][3]
                DECupperleft = Table[idx][5]
        NhPanel = MAX_NhPanel
        NvPanel = MAX_NvPanel
        NH = int((Xmax - Xmin) / NhPanel + 1)
        NV = int((Ymax - Ymin) / NvPanel + 1)
        RAupperleft += Spacing/3.0
        DECupperleft += Spacing/3.0
        page = 0

        pngname = FigFileRoot+'_Thumb.png'
        htmlname = FigFileRoot+'_Thumb.html'
        ClickableMap = open( FigFileDir+htmlname, 'w' )
        print >> ClickableMap, '<HTML>\n<IMG SRC="./%s" USEMAP="#%s">\n<MAP NAME="%s">'%(pngname,pngname,pngname)

        xmax = RAupperleft-Spacing*Aspect*NhPanel*NH - span/20.0
        ymin = DECupperleft-Spacing*NvPanel*NV - span/20.0

        for y in range(NV):
            for x in range(NH):
                #ax = PL.gca()
                Px0 = RAupperleft-Spacing*Aspect*NhPanel*x
                Pxw = Spacing*Aspect*(NhPanel-1/3.0)
                Px1 = Px0 - Pxw
                Py0 = DECupperleft-Spacing*NvPanel*y
                Pyw = Spacing*(NvPanel-1/3.0)
                Py1 = Py0 - Pyw
                pixmin=247+int((Px0-xmin)/(xmax-xmin)*545)
                piymin=78+int((ymax-Py0)/(ymax-ymin)*545)
                pixmax=247+int((Px1-xmin)/(xmax-xmin)*545)
                piymax=78+int((ymax-Py1)/(ymax-ymin)*545)
                rect = PL.Rectangle((Px0, Py0), -Pxw, -Pyw, facecolor='#eeeeee')
                print >> ClickableMap, '<AREA SHAPE="rect" COORDS="%s,%s,%s,%s" HREF="./%s_%s.png">'%(pixmin,piymin,pixmax,piymax,FigFileRoot,page)
                #rect.set_transform(ax.transAxes)
                PL.gca().add_patch(rect)
                #ax.add_patch(rect)
                #ax.text(0.5*(Px0+Px1), 0.5*(Py0+Py1), 'Page %d' % page, horizontalalignment='center', verticalalignment='center', transform=ax.transAxes, color='g')
                PL.text(0.5*(Px0+Px1), 0.5*(Py0+Py1), 'Page %d' % page, horizontalalignment='center', verticalalignment='center', color='g', size=max(5, 12-max(NH, NV)))
                page += 1

        # 2008/9/20 DEC Effect
        a.set_aspect(Aspect)
        # Plot Pointing
        PL.xlabel('RA')
        PL.ylabel('Dec')
        PL.title('Navigator for Multi Spectra')
        PL.plot(RA, DEC, Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
        a.xaxis.set_major_formatter(RAformatter)
        a.yaxis.set_major_formatter(DECformatter)
        a.xaxis.set_major_locator(RAlocator)
        a.yaxis.set_major_locator(DEClocator)
        xlabels = a.get_xticklabels()
        PL.setp(xlabels, 'rotation', SDP.RArotation, fontsize=8)
        ylabels = a.get_yticklabels()
        PL.setp(ylabels, 'rotation', SDP.DECrotation, fontsize=8)
        # moved before the (NV,NH) loop as those are needed for clickable map
        # 2010/11/11 TN
        #xmax = RAupperleft-Spacing*Aspect*NhPanel*NH - span/20.0
        #ymin = DECupperleft-Spacing*NvPanel*NV - span/20.0
        PL.axis([xmin, xmax, ymin, ymax])
        
        PL.savefig(FigFileDir+pngname, format='png', dpi=DPIDetail)
        
        print >> ClickableMap, '</MAP>\n</HTML>'
        ClickableMap.close()

        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: MultiSpectra'
            print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
        #print >> BrowserFile, FigFileRoot+'_Thumb.png'
        print >> BrowserFile, htmlname
        BrowserFile.close()

        del RA, DEC
        return


    def DrawAzEl(self, TimeGapList, rows, vAnt, ShowPlot=True, FigFile=False):
        """
        Plot Az El v.s. Time
        Table: DataTable
        TimeGapList: [[rowX1, rowX2,...,rowXN],[rowX1, rowX2,...,rowXN]]
        """
        origin = 'DrawAZEL()'
        
        # Select large time gap (equivalent to one set of raster scan)
        TimeGap = TimeGapList[1]

        # if DoStack is true plot will be stacked with different dates.
        DoStack = True
        if ShowPlot == False and FigFile == False: return
        # Extract Az, El, and MJD
        Az = []
        AzArr = []
        El = []
        ElArr = []
        MJD = []
        MJDArr = []
        TGap = []
        PGap = []

        tTIME = None
        tAZ = None
        tEL = None
        if self.USE_CASA_TABLE:
            tTIME = self.DataTable.getcol('TIME')
            tAZ = self.DataTable.getcol('AZ')
            tEL = self.DataTable.getcol('EL')
        
            for gap in TimeGap:
                if gap > self.Row2ID[vAnt][rows[-1]]: break
                if gap == 0: continue
                TGap.append((tTIME[gap - 1] + tTIME[gap]) / 2.)
            for gap in TimeGap:
                if gap > self.Row2ID[vAnt][rows[-1]]: break
                if gap == 0: continue
                PGap.append((tTIME[gap - 1] + tTIME[gap]) / 2.)
            for row in rows:
                ID = self.Row2ID[vAnt][row]
                Az.append(tAZ[ID])
                El.append(tEL[ID])
                MJD.append(tTIME[ID])
        else:
            for gap in TimeGap:
                if gap > self.Row2ID[vAnt][rows[-1]]: break
                if gap == 0: continue
                TGap.append((self.DataTable[gap - 1][DT_TIME] + self.DataTable[gap][DT_TIME]) / 2.)
            for gap in TimeGap:
                if gap > self.Row2ID[vAnt][rows[-1]]: break
                if gap == 0: continue
                PGap.append((self.DataTable[gap - 1][DT_TIME] + self.DataTable[gap][DT_TIME]) / 2.)
            for row in rows:
                ID = self.Row2ID[vAnt][row]
                Az.append(self.DataTable[ID][DT_AZ])
                El.append(self.DataTable[ID][DT_EL])
                MJD.append(self.DataTable[ID][DT_TIME])
##         for gap in TimeGap:
##             if gap > self.Row2ID[vAnt][rows[-1]]: break
##             if gap == 0: continue
##             TGap.append((self.DataTable[gap - 1][DT_TIME] + self.DataTable[gap][DT_TIME]) / 2.)
##         for gap in TimeGap:
##             if gap > self.Row2ID[vAnt][rows[-1]]: break
##             if gap == 0: continue
##             PGap.append((self.DataTable[gap - 1][DT_TIME] + self.DataTable[gap][DT_TIME]) / 2.)
##         for row in rows:
##             ID = self.Row2ID[vAnt][row]
##             Az.append(self.DataTable[ID][DT_AZ])
##             El.append(self.DataTable[ID][DT_EL])
##             MJD.append(self.DataTable[ID][DT_TIME])
        MJDmin = NP.array(MJD).min()
        MJDmax = NP.array(MJD).max()
        Extend = (MJDmax - MJDmin) * 0.05
        MJDmin -= Extend
        MJDmax += Extend
        ELmin = min(El)
        if min(Az) < 0:
            for row in range(len(Az)):
                if Az[row] < 0: Az[row] += 360.0

        if DoStack:
            # find number of days 
            ndays = 0
            for n in range(len(MJD)):
                if n == 0:
                    ndays +=1
                    MJDArr.append([])
                    AzArr.append([])
                    ElArr.append([])
                    MJDArr[0].append(MJD[0])
                    AzArr[0].append(Az[0])
                    ElArr[0].append(El[0])
                else:
                    delt = int(MJD[n]) - int(MJD[n-1])
                    if delt >= 1:
                        ndays += 1
                        MJDArr.append([])
                        AzArr.append([])
                        ElArr.append([])

                    MJDArr[ndays-1].append(MJD[n])
                    AzArr[ndays-1].append(Az[n])
                    ElArr[ndays-1].append(El[n])

        # Plotting routine
        if ShowPlot: PL.ion()
        PL.figure(SDP.MATPLOTLIB_FIGURE_ID[2])
        if ShowPlot: PL.ioff()
        PL.cla()
        PL.clf()

        if DoStack:
            markercolorbase = ['b', 'm', 'y', 'k', 'r']
            m=NP.ceil(ndays*1.0/len(markercolorbase))
            markercolors = markercolorbase*int(m)
            markerbase = ['o','x','^','s','v']
            markers = []
            for mrk in markerbase:
                for i in range(len(markercolorbase)):
                    if len(markers)<len(markercolors): 
                        markers.append(markercolors[i]+mrk)

            PL.axes([0.1, 0.55, 0.8, 0.35])
            PL.ylabel('Elevation (deg)')
            PL.title('Elevation Plot v.s. Time (with Detected large Gaps)')
            PL.xlabel('Time (UT)')
            for nd in range(ndays):
                UTdata = (NP.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
                if nd == 0:
                    UTmin = min(UTdata)
                    UTmax = max(UTdata)
                else:
                    if min(UTdata) < UTmin: UTmin = min(UTdata)
                    if max(UTdata) > UTmax: UTmax = max(UTdata)

                #date = self._qa.quantity(MJDArr[nd][0],'d')
                date = self._qa.quantity(str(MJDArr[nd][0])+'d')
                (datelab,rest) = self._qa.time(date,form='dmy')[0].split('/')  
                #PL.plot(UTdata, ElArr[nd], 'bo', markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)

                PL.plot(UTdata, ElArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                PL.legend(prop=FontProperties(size='smaller'),markerscale=1.0,numpoints=1)
                for Time in TGap:
                    if int(Time) == int(MJDArr[nd][0]):
                        modTime = (Time - int(Time))*24
                        PL.axvline(x=modTime, linewidth=0.5, color='c',label='_nolegend_')
            Extend = (UTmax - UTmin) * 0.05
            UTmin -= Extend
            UTmax += Extend
            if ELmin < 0: PL.axis([UTmin, UTmax, -90, 90])
            else: PL.axis([UTmin, UTmax, 0, 90])

            PL.axes([0.1, 0.1, 0.8, 0.35])
            PL.ylabel('Azimuth (deg)')
            PL.title('Azimuth Plot v.s. Time (with Detected large Gaps)')
            PL.xlabel('Time (UT)')
            for nd in range(ndays):
                UTdata = (NP.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
                date = self._qa.quantity(str(MJDArr[nd][0])+'d')
                (datelab,rest) = self._qa.time(date,form='dmy')[0].split('/')  
                #PL.plot(UTdata, AzArr[nd], 'bo', markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                PL.plot(UTdata, AzArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
                PL.legend(prop=FontProperties(size='smaller'),markerscale=0.8,numpoints=1)
                for Time in PGap:
                    if int(Time) == int(MJDArr[nd][0]):
                        modTime = (Time - int(Time))*24
                        PL.axvline(x=modTime, linewidth=0.5, color='g', label='_nolegend_')

            PL.axis([UTmin, UTmax, 0, 360])
        else:
            PL.axes([0.1, 0.55, 0.8, 0.35])
            PL.ylabel('Elevation (deg)')
            PL.title('Elevation Plot v.s. Time (with Detected large Gaps)')
            PL.xlabel('MJD (Day)')
            for Time in TGap:
                PL.axvline(x=Time, linewidth=0.5, color='c')
            PL.plot(MJD, El, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
            if ELmin < 0: PL.axis([MJDmin, MJDmax, -90, 90])
            else: PL.axis([MJDmin, MJDmax, 0, 90])

            PL.axes([0.1, 0.1, 0.8, 0.35])
            PL.ylabel('Azimuth (deg)')
            PL.title('Azimuth Plot v.s. Time (with Detected large Gaps)')
            PL.xlabel('MJD (Day)')
            for Time in PGap:
                PL.axvline(x=Time, linewidth=0.5, color='g')
            PL.plot(MJD, Az, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
            PL.axis([MJDmin, MJDmax, 0, 360])

        if ShowPlot != False: PL.draw()
        if FigFile != False: PL.savefig(FigFile, format='png', dpi=SDP.DPISummary)

        del MJD, Az, El, TGap, PGap
        return


    def DrawWeather(self, WeatherDic, rows, vAnt, ShowPlot=True, FigFile=False):
        """
        Plot Weather information and Tsys v.s. Time
        Table: DataTable
        """
        origin = 'DrawWeather()'
        
        if ShowPlot == False and FigFile == False: return

        # Plotting routine
        if ShowPlot: PL.ion()
        Fig = PL.figure(SDP.MATPLOTLIB_FIGURE_ID[2])
        if ShowPlot: PL.ioff()
        PL.clf()

        # get Weather info from the table
        #print WeatherDic['TIME']
        #print WeatherDic['TEMPERATURE']
        #print WeatherDic['PRESSURE']
        #print WeatherDic['REL_HUMIDITY']
        #print WeatherDic['WIND_SPEED']

        # Extract MJD and Tsys
        if self.USE_CASA_TABLE:
            MJD = NP.take(self.DataTable.getcol('TIME'),rows)
            Tsys = NP.take(self.DataTable.getcol('TSYS'),rows)
        else:
            MJD = []
            Tsys = []
            for row in rows:
                ID = self.Row2ID[vAnt][row]
                MJD.append(self.DataTable[ID][DT_TIME])
                Tsys.append(self.DataTable[ID][DT_TSYS])
##         MJD = []
##         Tsys = []
##         for row in rows:
##             ID = self.Row2ID[vAnt][row]
##             MJD.append(self.DataTable[ID][DT_TIME])
##             Tsys.append(self.DataTable[ID][DT_TSYS])

        # Convert MJD sec to MJD date for WeatherDic
        # K -> degC
        for i in range(len(WeatherDic['TIME'])):
            WeatherDic['TIME'][i] = WeatherDic['TIME'][i]/3600./24.
            WeatherDic['TEMPERATURE'][i] = WeatherDic['TEMPERATURE'][i] - 273.16

        MJDmin = NP.array(MJD).min()
        MJDmax = NP.array(MJD).max()
        Tsysmax = NP.array(Tsys).max()
        if Tsysmax > 0:
            for i in range(len(Tsys)):
                Tsys[i] = Tsys[i] * 100.0 / Tsysmax
        else:
            for i in range(len(Tsys)):
                Tsys[i] = 0.0
        Tempmin = WeatherDic['TEMPERATURE'].min() - 3.0
        Tempmax = WeatherDic['TEMPERATURE'].max() + 2.0
        dTemp = Tempmax - Tempmin
        Humimin = 0.0
        Humimax = min(WeatherDic['REL_HUMIDITY'].max() + 5.0, 100.0)
        Presmin = WeatherDic['PRESSURE'].min() - 2.0
        Presmax = WeatherDic['PRESSURE'].max() + 1.0
        dPres = Presmax - Presmin
        Windmin = 0.0
        Windmax = WeatherDic['WIND_SPEED'].max() + 5
        dTsys = max(Tsys) - min(Tsys)
        if dTsys < 0.1:
            (Tsysmin, Tsysmax) = (min(Tsys) - 1.0, max(Tsys) + 1.0)
        else:
            (Tsysmin, Tsysmax) = (min(Tsys) - dTsys*0.1, max(Tsys) + dTsys*0.1)
        dTsys = Tsysmax - Tsysmin
        # Scale Tsys to fit in the Weather plotting window
        # Overplot on the Temperature plot and Pressure plot
        TsysT = Tsys[:]
        TsysP = Tsys[:]
        for i in range(len(TsysT)):
            TsysT[i] = Tempmin + dTemp * (Tsys[i] - Tsysmin) / dTsys
            TsysP[i] = Presmin + dPres * (Tsys[i] - Tsysmin) / dTsys
        
        # Plot Temperature (degC)
        Ax1 = Fig.add_subplot(211)
        Ax1.plot(MJD, TsysT, 'go', markersize=3, markeredgecolor='g', markerfacecolor='g')
        if len(WeatherDic['TIME']) == 1:
            Ax1.axhline(y = WeatherDic['TEMPERATURE'][0])
        else:
            Ax1.plot(WeatherDic['TIME'], WeatherDic['TEMPERATURE'], 'r-')
        Ax1.axis([MJDmin, MJDmax, Tempmin, Tempmax])
        Ax1.set_xlabel('MJD')
        Ax1.set_ylabel('Temperature (degC)', color='r')
        Ax1.set_title('Weather (Temperature & Humidity) and Tsys(green) versus MJD')
        for tl in Ax1.get_yticklabels():
            tl.set_color('r')

        # Plot Humidity (%)
        Ax2 = Ax1.twinx()
        if len(WeatherDic['TIME']) == 1:
            Ax2.axhline(y = WeatherDic['REL_HUMIDITY'][0])
        else:
            Ax2.plot(WeatherDic['TIME'], WeatherDic['REL_HUMIDITY'], 'b-')
        Ax2.axis([MJDmin, MJDmax, Humimin, Humimax])
        Ax2.set_ylabel('Humidity (%)', color='b')
        for tl in Ax2.get_yticklabels():
            tl.set_color('b')

        # Plot Pressure (hPa)
        Ax1 = Fig.add_subplot(212)
        Ax1.plot(MJD, TsysP, 'go', markersize=3, markeredgecolor='g', markerfacecolor='g')
        if len(WeatherDic['TIME']) == 1:
            Ax1.axhline(y = WeatherDic['PRESSURE'][0])
        else:
            Ax1.plot(WeatherDic['TIME'], WeatherDic['PRESSURE'], 'r-')
        Ax1.axis([MJDmin, MJDmax, Presmin, Presmax])
        Ax1.set_xlabel('MJD')
        Ax1.set_ylabel('Pressure (hPa)', color='r')
        Ax1.set_title('Weather (Pressure & Wind Speed) and Tsys(green) versus MJD')
        for tl in Ax1.get_yticklabels():
            tl.set_color('r')

        # Plot Wind speed (m/s)
        Ax2 = Ax1.twinx()
        if len(WeatherDic['TIME']) == 1:
            Ax2.axhline(y = WeatherDic['WIND_SPEED'][0])
        else:
            Ax2.plot(WeatherDic['TIME'], WeatherDic['WIND_SPEED'], 'b-')
        Ax2.axis([MJDmin, MJDmax, Windmin, Windmax])
        Ax2.set_ylabel('Wind Speed (m/s)', color='b')
        for tl in Ax2.get_yticklabels():
            tl.set_color('b')

        if ShowPlot != False: PL.draw()
        if FigFile != False: PL.savefig(FigFile, format='png', dpi=SDP.DPISummary)

        del MJD, Tsys, TsysT, TsysP
        return


    def DrawWVR(self, WVRdata, WVRFreq, ShowPlot=True, FigFile=False):
        """
        Plot WVR values v.s. Time
        """
        origin = 'DrawWVR()'
        
        if ShowPlot == False and FigFile == False: return

        # Plotting routine
        if ShowPlot: PL.ion()
        Fig = PL.figure(SDP.MATPLOTLIB_FIGURE_ID[2])
        if ShowPlot: PL.ioff()
        PL.clf()

        # Convert MJD sec to MJD date for WVRdata
        WVRdata[0] = WVRdata[0]/3600.0/24.0

        # Convert WVRFreq in Hz to GHz
        WVRFreq = WVRFreq / 1.0e9

        # Plot WVR data
        Ax1 = Fig.add_subplot(111)
        if len(WVRdata[0]) == 1:
            Ax1.axhline(y = WVRdata[1][0], color='r', label='%.2fGHz'%WVRFreq[0])
            Ax1.axhline(y = WVRdata[2][0], color='g', label='%.2fGHz'%WVRFreq[1])
            Ax1.axhline(y = WVRdata[3][0], color='b', label='%.2fGHz'%WVRFreq[2])
            Ax1.axhline(y = WVRdata[4][0], color='c', label='%.2fGHz'%WVRFreq[3])
        else:
            Ax1.plot(WVRdata[0], WVRdata[1], 'ro', markersize=3, markeredgecolor='r', markerfacecolor='r', label='%.2fGHz'%WVRFreq[0])
            Ax1.plot(WVRdata[0], WVRdata[2], 'go', markersize=3, markeredgecolor='g', markerfacecolor='g', label='%.2fGHz'%WVRFreq[1])
            Ax1.plot(WVRdata[0], WVRdata[3], 'bo', markersize=3, markeredgecolor='b', markerfacecolor='b', label='%.2fGHz'%WVRFreq[2])
            Ax1.plot(WVRdata[0], WVRdata[4], 'co', markersize=3, markeredgecolor='c', markerfacecolor='c', label='%.2fGHz'%WVRFreq[3])
        Ax1.legend(loc=0,numpoints=1,prop=FontProperties(size='smaller'))
        Ax1.set_xlabel('MJD')
        Ax1.set_ylabel('WVR reading')
        Ax1.set_title('WVR reading versus MJD')

        if ShowPlot != False: PL.draw()
        if FigFile != False: PL.savefig(FigFile, format='png', dpi=SDP.DPISummary)
        return


    @dec_engines_logfile
    @dec_engines_logging
    def DrawFitSpectrum(self, DataIn, DataOut, Idx, NCHAN, vIF, edge=(0,0), HandleFlag=False, LogLevel=2, LogFile=False, FigFileDir=False, FigFileRoot=False):

        origin = 'DrawFitSpectrum()'
        self.setFileOut( LogFile )
##         Abcissa = self.Abcissa[vIF]

        NROW = len(Idx)
        # Variables for Panel
        TickSizeList = [12, 12, 10, 8, 6, 5, 5, 5, 5, 5, 5]
        if ((NROW-1) / (MAX_NhPanelFit*MAX_NvPanelFit)+1) > 1:
            (NhPanel, NvPanel) = (MAX_NhPanelFit, MAX_NvPanelFit)
        elif NROW == 1: (NhPanel, NvPanel) = (1, 1)
        elif NROW == 2: (NhPanel, NvPanel) = (1, 2)
        elif NROW <= 4: (NhPanel, NvPanel) = (2, 2)
        elif NROW <= 6: (NhPanel, NvPanel) = (2, 3)
        elif NROW <= 9: (NhPanel, NvPanel) = (3, 3)
        elif NROW <=12: (NhPanel, NvPanel) = (3, 4)
        elif NROW <=15: (NhPanel, NvPanel) = (3, 5)
        else: (NhPanel, NvPanel) = (MAX_NhPanelFit, MAX_NvPanelFit)
        NSpFit = NhPanel * NvPanel

        alist = [None] * NSpFit * 3
    
        # fitparam: no use since 2010/6/12

        tROW = None
        tSFLAG = None
        tSTAT = None
        if self.USE_CASA_TABLE:
            tROW = self.DataTable.getcol('ROW')
            tSFLAG = self.DataTable.getcol('FLAG_SUMMARY')
            tSTAT = self.DataTable.getcol('STATISTICS')

        # Set edge mask region (not used?)
        (EdgeL, EdgeR) = parseEdge(edge)

        #tbtool = casac.homefinder.find_home_by_name('tableHome')
##         tbIn = tbtool.create()
        s = sd.scantable(DataIn, average=False)
        unitorg = s.get_unit()
        s.set_unit('GHz')
        if self.USE_CASA_TABLE:
            Abcissa = NP.array( s._getabcissa(tROW[Idx[0]]) )
        else:
            Abcissa = NP.array( s._getabcissa(self.DataTable[Idx[0]][DT_ROW]) )
        #Abcissa = NP.array( s._getabcissa(self.DataTable[Idx[0]][DT_ROW]) )
        s.set_unit(unitorg)
        #tbOut = tbtool.create()
        tbOut = gentools(['tb'])[0]
##         tbIn.open(DataIn)
        tbOut.open(DataOut)

        # Setup Plot range, fontsize, ticks
##         Xrange = [min(Abcissa[1][0], Abcissa[1][-1]), max(Abcissa[1][0], Abcissa[1][-1])]
        Xrange = [min(Abcissa[0], Abcissa[-1]), max(Abcissa[0], Abcissa[-1])]
        #TickSize = 12 - NhPanel * 2
        TickSize = TickSizeList[NhPanel]
        xtick = abs(Xrange[1] - Xrange[0]) / 3.0
        Order = int(math.floor(math.log10(xtick)))
        NewTick = int(xtick / (10**Order) + 1) * (10**Order)
        FreqLocator = MultipleLocator(NewTick)
        if Order < 0: FMT = '%%.%df' % (-Order)
        else: FMT = '%.2f'
        Format = PL.FormatStrFormatter(FMT)

        # Main loop to plot all spectra (raw + reduced)
        counter = 0
        Npanel = 0
        #for row in rows:
        for index in xrange(len(Idx)):
            idx = Idx[index]
            if self.USE_CASA_TABLE:
                row = tROW[idx]
            else:
                row = self.DataTable[idx][DT_ROW]
            #print index, idx, row, self.DataTable[row][DT_NMASK], self.DataTable[row][DT_MASKLIST]
##             SpIn = tbIn.getcell('SPECTRA', row)
            SpIn = NP.array(s._getspectrum(row))
            SpOut = tbOut.getcell('SPECTRA', row)
            if self.USE_CASA_TABLE:
                NoChange = self.DataTable.getcell('NOCHANGE',idx)
            else:
                NoChange = self.DataTable[idx][DT_NOCHANGE]
            NSp = counter % NSpFit
            if NSp == 0:
                if os.access(FigFileDir+'listofplots.txt', os.F_OK):
                    BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
                else:
                    BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
                    print >> BrowserFile, 'TITLE: BF_Fitting'
                    print >> BrowserFile, 'FIELDS: FIT IF POL Iteration Page'
                    print >> BrowserFile, 'COMMENT: Baseline Fit'
                print >> BrowserFile, FigFileRoot+'_%s.png' % Npanel
                BrowserFile.close()

            YMIN = min(SpIn.min(), SpOut.min())
            YMAX = max(SpIn.max(), SpOut.max())
            #YMIN = min(SpStorageIn[index].min(), SpStorageOut[index].min())
            #YMAX = max(SpStorageIn[index].max(), SpStorageOut[index].max())
            Yrange = [YMIN-(YMAX-YMIN)/10.0, YMAX+(YMAX-YMIN)/10.0]
            Mask = []

            if self.USE_CASA_TABLE:
                tMASKLIST = self.DataTable.getcell('MASKLIST',idx)
            else:
                tMASKLIST = self.DataTable[idx][DT_MASKLIST]

            for xx in range(len(tMASKLIST)):
##                 Mask.append([Abcissa[1][int(self.DataTable[idx][DT_MASKLIST][xx][0])], Abcissa[1][int(self.DataTable[idx][DT_MASKLIST][xx][1])]])
                Mask.append([Abcissa[int(tMASKLIST[xx][0])], Abcissa[int(tMASKLIST[xx][1])]])
            #for xx in range(len(self.DataTable[idx][DT_MASKLIST])):
##          #       Mask.append([Abcissa[1][int(self.DataTable[idx][DT_MASKLIST][xx][0])], Abcissa[1][int(self.DataTable[idx][DT_MASKLIST][xx][1])]])
            #    Mask.append([Abcissa[int(self.DataTable[idx][DT_MASKLIST][xx][0])], Abcissa[int(self.DataTable[idx][DT_MASKLIST][xx][1])]])

            if type(NoChange) != int:
                TitleColor = 'k'
                AddTitle = ''
            else:
                TitleColor = 'g'
                AddTitle = 'No Change since Cycle%d' % NoChange
    
            fitdata = SpIn - SpOut
            #fitdata = SpStorageIn[index] - SpStorageOut[index]
            if self.USE_CASA_TABLE:
                if HandleFlag and tSFLAG[idx] == 0: BackgroundColor = '#ff8888'
                else: BackgroundColor = 'w'
            else:
                if HandleFlag and self.DataTable[idx][DT_SFLAG] == 0: BackgroundColor = '#ff8888'
                else: BackgroundColor = 'w'
            #if HandleFlag and self.DataTable[idx][DT_SFLAG] == 0: BackgroundColor = '#ff8888'
            #else: BackgroundColor = 'w'
    
            if NhPanel == 1 and NvPanel == 1:
                PL.cla()
                PL.clf()
                PL.subplots_adjust(hspace=0.3)
                PL.subplot(121)
                PL.xlabel('Channel', size=10)
                #PL.ylabel('Flux Density', size=10)
                PL.ylabel('Intensity (K)', size=10)
                PL.title('%s\nRaw and Fit data : row = %d' % (AddTitle, row), size=10, color=TitleColor)
##                 PL.plot(Abcissa[1], SpIn, color='b', linestyle='-', linewidth=0.2)
                PL.plot(Abcissa, SpIn, color='b', linestyle='-', linewidth=0.2)
                #PL.plot(Abcissa[1], SpStorageIn[index], color='b', linestyle='-', linewidth=0.2)
##                 PL.plot(Abcissa[1], fitdata, color='r', linestyle='-', linewidth=0.8)
                PL.plot(Abcissa, fitdata, color='r', linestyle='-', linewidth=0.8)
                for x in range(len(Mask)):
                    PL.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.8)
                    PL.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.8)
                PL.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
                if self.USE_CASA_TABLE:
                    statistics = 'Pre-Fit RMS=%.2f, Post-Fit RMS=%.2f' % (tSTAT[2][idx], tSTAT[1][idx])
                else:
                    statistics = 'Pre-Fit RMS=%.2f, Post-Fit RMS=%.2f' % (self.DataTable[idx][DT_STAT][2], self.DataTable[idx][DT_STAT][1])
                #statistics = 'Pre-Fit RMS=%.2f, Post-Fit RMS=%.2f' % (self.DataTable[idx][DT_STAT][2], self.DataTable[idx][DT_STAT][1])
                PL.figtext(0.05, 0.015, statistics, size=10)
                #PL.figtext(0.05, 0.015, fitparam + statistics, size=10)

                PL.subplot(122)
                PL.xlabel('Channel', size=10)
                #PL.ylabel('Flux Density', size=10)
                PL.ylabel('Intensity (arbitrary)', size=10)
                PL.title('%s\nReduced data : row = %d' % (AddTitle, row), size=10, color=TitleColor)
##                 PL.plot(Abcissa[1], SpOut, color='b', linestyle='-', linewidth=0.2)
                PL.plot(Abcissa, SpOut, color='b', linestyle='-', linewidth=0.2)
                #PL.plot(Abcissa[1], SpStorageOut[index], color='b', linestyle='-', linewidth=0.2)
                for x in range(len(Mask)):
                    PL.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.8)
                    PL.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.8)
                PL.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
            else:
                if index == 0:#NSp == 0:
                    PL.cla()
                    PL.clf()
                x = NSp % NhPanel
                # UpperLeft: 0 -> Upper Right to increase -> then one line down
                #y = NvPanel - 1 - int(NSp / NhPanel)
                #x00 = 1.0 / float(NhPanel) * (x + 0.1 + 0.05)
                #x01 = 1.0 / float(NhPanel) * 0.4
                #x10 = 1.0 / float(NhPanel) * (x + 0.5 + 0.05)
                #x11 = 1.0 / float(NhPanel) * 0.4
                #y0 = 1.0 / float(NvPanel) * (y + 0.1)
                #y1 = 1.0 / float(NvPanel) * 0.8

##                 a0 = PL.axes([x00, y0, x01, y1], axisbg=BackgroundColor)
                pindex = index % NSpFit
                baseid = 3*pindex
                if alist[baseid] is None:
                    y = NvPanel - 1 - int(NSp / NhPanel)
                    x00 = 1.0 / float(NhPanel) * (x + 0.1 + 0.05)
                    x01 = 1.0 / float(NhPanel) * 0.4
                    x10 = 1.0 / float(NhPanel) * (x + 0.5 + 0.05)
                    x11 = 1.0 / float(NhPanel) * 0.4
                    y0 = 1.0 / float(NvPanel) * (y + 0.1)
                    y1 = 1.0 / float(NvPanel) * 0.8
                    alist[baseid] = PL.axes([x00, y0, x01, y1])
                    alist[baseid+1] = PL.axes([x10, y0, x11, y1])
                    alist[baseid+2] = PL.axes([(x00-x01/5.0), y0-0.125/float(NvPanel), x01/10.0, y1/10.0])
                a0 = alist[baseid]
                a0.set_axis_bgcolor(BackgroundColor)
                PL.gcf().sca(a0)
                PL.cla()
        #        PL.xlabel('Channel', size=TickSize)
                PL.ylabel('Intensity (arbitrary)', size=TickSize)
                PL.title('Fit: row = %d' % row, size=TickSize, color=TitleColor)
##                 PL.plot(Abcissa[1], SpIn, color='b', linestyle='-', linewidth=0.2)
                PL.plot(Abcissa, SpIn, color='b', linestyle='-', linewidth=0.2)
                #PL.plot(Abcissa[1], SpStorageIn[index], color='b', linestyle='-', linewidth=0.2)
##                 PL.plot(Abcissa[1], fitdata, color='r', linestyle='-', linewidth=0.8)
                PL.plot(Abcissa, fitdata, color='r', linestyle='-', linewidth=0.8)
                a0.xaxis.set_major_formatter(Format)
                a0.xaxis.set_major_locator(FreqLocator)
                for x in range(len(Mask)):
                    PL.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.2)
                    PL.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.2)
                #PL.xticks(size=TickSize)
                for t in a0.get_xticklabels():
                    t.set_fontsize((TickSize-1))
                PL.yticks(size=TickSize)
                PL.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
        #        PL.figtext(0.05, 0.015, fitparam + statistics, size=10)

##                 a1 = PL.axes([x10, y0, x11, y1], axisbg=BackgroundColor)
                a1 = alist[baseid+1]
                a1.set_axis_bgcolor(BackgroundColor)
                PL.gcf().sca(a1)
                PL.cla()
        #        PL.xlabel('Channel', size=TickSize)
                if type(NoChange) == int: PL.title(AddTitle, size=TickSize, color=TitleColor)
                else: PL.title('Reduced: row = %d' % row, size=TickSize, color=TitleColor)
##                 PL.plot(Abcissa[1], SpOut, color='b', linestyle='-', linewidth=0.2)
                PL.plot(Abcissa, SpOut, color='b', linestyle='-', linewidth=0.2)
           #PL.plot(Abcissa[1], SpStorageOut[index], color='b', linestyle='-', linewidth=0.2)
                a1.xaxis.set_major_formatter(Format)
                a1.xaxis.set_major_locator(FreqLocator)
                for x in range(len(Mask)):
                    PL.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.2)
                    PL.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.2)
                PL.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
                #PL.xticks(size=TickSize)
                #newlabs = []
                for t in a1.get_xticklabels():
                    #tt = t.get_text()
                    #newlabs.append(tt)
                    t.set_fontsize((TickSize-1))
                #PL.text(1,0,' [GHz]', size=(TickSize-1), va='bottom',transform=a1.transAxes)
                #PL.text(1,0,' [GHz]', size=(TickSize-1), ha='center',va='top',transform=a1.transAxes)
                #newlabs[len(newlabs)-1]=newlabs[len(newlabs)-1]+'GHz'
                #a1.set_xticklabels(newlabs)
                a1.yaxis.set_major_locator(PL.NullLocator())
                #a2 = PL.axes([(x10+x11-x11/15.0), y0-0.15/float(NvPanel), x11/10.0, y1/10.0])
                #a2 = PL.axes([(x10+x11-x11/10.0), y0-0.125/float(NvPanel), x11/10.0, y1/10.0])
##                 a2 = PL.axes([(x00-x01/5.0), y0-0.125/float(NvPanel), x01/10.0, y1/10.0])
                a2 = alist[baseid+2]
                PL.gcf().sca(a2)
                PL.cla()
                a2.set_axis_off()
                PL.text(0,0.5,' (GHz)', size=(TickSize-1) ,transform=a2.transAxes)

            counter += 1
            if counter % NSpFit == 0 or idx == Idx[-1]:
                plotted = counter % NSpFit
                if plotted > 0:
                    for ipanel in xrange(plotted*3, NSpFit*3):
                        alist[ipanel].clear()
                        alist[ipanel].set_axis_off()
                    PL.draw()
                PL.savefig(FigFileDir+FigFileRoot+'_%s.png' % Npanel, format='png', dpi=DPIDetail)
                Npanel += 1

        del SpIn, SpOut
        del s
        tbOut.close()
        return
    
    
    @dec_engines_logfile
    def DrawFitSpectrumThumbnail(self, rows, vIF, vPOL, iteration, LogLevel=2, LogFile=False, FigFileDir=False, FigFileRoot=False):
        """
        Clickable Map: UpperLeft: (129+81*x, 135+58*y), Width: (70, 46)
        """

        origin = 'DrawFitSpectrumThumbnail()'

        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: BF_Fitting'
            print >> BrowserFile, 'FIELDS: FIT IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Baseline Fit'

        NROW = len(rows)
        row = 0
        page = 0
        for PAGE in range(1+int((NROW - 1)/(MAX_NhPanelFit*MAX_NvPanelFit*100))):
            Fname = FigFileRoot+'_Thumb%s' % PAGE
            #print >> BrowserFile, FigFileRoot+'_Thumb%s.png' % PAGE
            print >> BrowserFile, Fname+'.html'
            ClickableMap = open(FigFileDir+Fname+'.html', 'w')
            print >> ClickableMap, '<HTML>\n<IMG SRC="./%s.png" USEMAP="#%s">\n<map name="%s">' % (Fname, Fname, Fname)
            PL.cla()
            PL.clf()
            a1=PL.subplot(11, 1, 1)
            a1.set_axis_off()
            PL.text(0.5, 0.5, 'Thumbnail Navigator for the Baseline Fit.      Iteration:%d   IF:%d   POL:%d   row: %d - %d' % (iteration, vIF, vPOL, rows[PAGE*MAX_NhPanelFit*MAX_NvPanelFit*100], rows[min(NROW, (PAGE+1)*MAX_NhPanelFit*MAX_NvPanelFit*100)-1]), horizontalalignment='center', verticalalignment='center', size=12)
            for y in range(10):
                for x in range(10):
                    a1=PL.subplot(11, 10, 11+x+10*y)
                    a1.yaxis.set_major_locator(PL.NullLocator())
                    a1.xaxis.set_major_locator(PL.NullLocator())
                    PL.title('p.%d: %d-%d' % (page, rows[row], rows[min(row+MAX_NhPanelFit*MAX_NvPanelFit-1, NROW-1)]), size=5)
                    for yy in range(MAX_NvPanelFit):
                        ypos = [yy-0.4, yy-0.4, yy+0.4, yy+0.4]
                        for xx in range(MAX_NhPanelFit):
                            xpos = [xx-0.4, xx+0.4, xx+0.4, xx-0.4]
                            if self.DataTable[rows[row]][DT_SFLAG] == 0: color = 'r'
                            else: color = 'w'
                            row += 1
                            a1.fill(xpos, ypos, color)
                            if row == NROW: break
                        if row == NROW: break
                    PL.axis([-0.5, MAX_NhPanelFit-0.5, MAX_NvPanelFit-0.5, -0.5])
                    print >> ClickableMap, '<area shape="rect" coords="%d,%d,%d,%d" href="./%s_%d.png">' % (129+81*x, 135+58*y, 199+81*x, 181+58*y, FigFileRoot, page)
                    if row == NROW: break
                    else: page += 1
                if row == NROW: break
            PL.savefig(FigFileDir+FigFileRoot+'_Thumb%s.png' % PAGE, format='png', dpi=DPIDetail)
            print >> ClickableMap, '</map>\n</HTML>'
            ClickableMap.close()
        BrowserFile.close()
        return


    @dec_engines_logfile
    def makeSimpleGridTable(self, IndexList, GridSpacingRA, GridSpacingDEC, nPlane=3, LogLevel=2, LogFile=False, FigFileDir=False, FigFileRoot=False):
        """
        Calculate Parameters for SimpleGridding by RA/DEC positions
        """
        origin = 'calcSimpleGridParam'
        
        #print '\nDEBUG:len(self.DataTable)', len(self.DataTable)
        #print 'DEBUG:len(IndexList), IndexList[0], IndexList[-1]', len(IndexList), IndexList[0], IndexList[-1], '\n'

        tRA = None
        tDEC = None
        tIF = None
        tPOL = None
        tROW = None
        tANT = None
        tSTAT = None
        if self.USE_CASA_TABLE:
            tROW = self.DataTable.getcol('ROW')
            tIF = self.DataTable.getcol('IF')
            tPOL = self.DataTable.getcol('POL')
            tANT = self.DataTable.getcol('ANTENNA')
            tRA = self.DataTable.getcol('RA')
            tDEC = self.DataTable.getcol('DEC')
            tSTAT = self.DataTable.getcol('STATISTICS')

        # Curvature has not been taken account
        if self.USE_CASA_TABLE:
            DecCorrection = 1.0 / math.cos(tDEC[0] / 180.0 * 3.141592653)
        else:
            DecCorrection = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)
        #DecCorrection = 1.0 / math.cos(self.DataTable[0][DT_DEC] / 180.0 * 3.141592653)
        #### GridSpacingRA = GridSpacing * DecCorrection

        if self.USE_CASA_TABLE:
            NpRAs = NP.take(tRA,IndexList)
            NpDECs = NP.take(tDEC,IndexList)
        else:
            NpRAs = NP.zeros(len(IndexList), dtype=NP.float64)
            NpDECs = NP.zeros(len(IndexList), dtype=NP.float64)
            for index in range(len(IndexList)):
                NpRAs[index] = self.DataTable[IndexList[index]][DT_RA]
                NpDECs[index] = self.DataTable[IndexList[index]][DT_DEC]
##         NpRAs = NP.zeros(len(IndexList), dtype=NP.float64)
##         NpDECs = NP.zeros(len(IndexList), dtype=NP.float64)
##         for index in range(len(IndexList)):
##             NpRAs[index] = self.DataTable[IndexList[index]][DT_RA]
##             NpDECs[index] = self.DataTable[IndexList[index]][DT_DEC]

        MinRA = NpRAs.min()
        MaxRA = NpRAs.max()
        MinDEC = NpDECs.min()
        MaxDEC = NpDECs.max()
        # Check if the distribution crosses over the RA=0
        if MinRA < 10 and MaxRA > 350:
            NpRAs = NpRAs + NP.less_equal(NpRAs, 180) * 360.0
            MinRA = NpRAs.min()
            MaxRA = NpRAs.max()
        NGridRA = int(int((MaxRA - MinRA + GridSpacingRA * DecCorrection)  / (2.0 * GridSpacingRA * DecCorrection)) * 2 + 1)
        NGridDEC = int(int((MaxDEC - MinDEC + GridSpacingDEC) / (2.0 * GridSpacingDEC)) * 2 + 1)
        MinRA = (MinRA + MaxRA - NGridRA * GridSpacingRA * DecCorrection) / 2.0
        MinDEC = (MinDEC + MaxDEC - NGridDEC * GridSpacingDEC) / 2.0

        # Calculate Grid index for each position
        IndexRA = NP.array((NpRAs - MinRA) / (GridSpacingRA * DecCorrection), dtype=NP.int)
        IndexDEC = NP.array((NpDECs - MinDEC) / GridSpacingDEC, dtype=NP.int)
        # Counter for distributing spectrum into several planes (nPlane)
        NpCounter = NP.zeros((NGridRA, NGridDEC), dtype=NP.int)

        # Make lists to store indexes for combine spectrum
        CombineList = []
        for p in range(nPlane):
            CombineList.append([])
            for x in range(NGridRA):
                CombineList[p].append([])
                for y in range(NGridDEC):
                    CombineList[p][x].append([])

        # Store indexes
        for index in range(len(IndexList)):
            CombineList[NpCounter[IndexRA[index]][IndexDEC[index]] % nPlane][IndexRA[index]][IndexDEC[index]].append(IndexList[index])
            NpCounter[IndexRA[index]][IndexDEC[index]] += 1
        del IndexRA, IndexDEC, NpCounter

        # Create GridTable for output
        GridTable = []
        # vIF, vPOL: dummy (not necessary)
        if self.USE_CASA_TABLE:
            vIF, vPOL = tIF[IndexList[0]], tPOL[IndexList[0]]
        else:
            vIF, vPOL = self.DataTable[IndexList[0]][DT_IF], self.DataTable[IndexList[0]][DT_POL]
        #vIF, vPOL = self.DataTable[IndexList[0]][DT_IF], self.DataTable[IndexList[0]][DT_POL]
        for y in range(NGridDEC):
            DEC = MinDEC + GridSpacingDEC * (y + 0.5)
            #self.LogMessage('DEBUG'  Origin=origin, Msg='Combine Spectra: %s' % len(sRMS))
            for x in range(NGridRA):
                RA = MinRA + GridSpacingRA * DecCorrection * (x + 0.5)
                #sDeltaRA = (sRA - RA) / DecCorrection
                #Delta = sDeltaDEC * sDeltaDEC + sDeltaRA * sDeltaRA
                for p in range(nPlane):
                    line = [vIF, vPOL, x, y, RA, DEC, []]
                    if self.USE_CASA_TABLE:
                        for index in CombineList[p][x][y]:
                            Delta = (((tRA[index] - RA) * DecCorrection) ** 2.0 + \
                                     (tDEC[index] - DEC) ** 2.0) ** 0.5
                            line[6].append([tROW[index], Delta, tSTAT[0][index], index, tANT[index]])
                    else:
                        for index in CombineList[p][x][y]:
                            Delta = (((self.DataTable[index][DT_RA] - RA) * DecCorrection) ** 2.0 + \
                                     (self.DataTable[index][DT_DEC] - DEC) ** 2.0) ** 0.5
                            line[6].append([self.DataTable[index][DT_ROW], Delta, self.DataTable[index][DT_STAT][0], index, self.DataTable[index][DT_ANT]])
##                     for index in CombineList[p][x][y]:
##                         Delta = (((self.DataTable[index][DT_RA] - RA) * DecCorrection) ** 2.0 + \
##                                   (self.DataTable[index][DT_DEC] - DEC) ** 2.0) ** 0.5
##                         line[6].append([self.DataTable[index][DT_ROW], Delta, self.DataTable[index][DT_STAT][0], index, self.DataTable[index][DT_ANT]])
                    GridTable.append(line)
                    self.LogMessage('LONG', Origin=origin, Msg="GridTable: %s" % line)

        del NpRAs, NpDECs, CombineList

        self.LogMessage('INFO', Origin=origin, Msg='NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))
        return GridTable


    def combineGriddedData(self, filelist):
        """
        combine the data listed in 'filelist'
        number of channels and number of rows should be the same to each other
        """
        origin = 'combineGriddedData'
        nData = len(filelist)
        if nData < 2:
            self.LogMessage('INFO', Origin=origin, Msg='Number of input files is less than 2: quitting...')
            return False
        data = []
        RAD2Deg = 180.0/math.pi
        # open all data
        for filename in filelist:
            data.append(sd.scantable(filename, average=False))
        # check if the data set can be combined
        (nRow, nChan) = (data[0].nrow(), data[0].nchan())
        for i in range(1, nData):
            if data[i].nrow() != nRow or data[i].nchan() != nChan:
                self.LogMessage('INFO', Origin=origin, Msg='Data size mismatch for (nRow,nChan):(%s,%s) and (%s,%s)' % (nRow,nChan,data[i].nrow(),data[i].nchan()))
                return False
        # create array
        Table = []
        StorageOut = NP.zeros((nRow, nChan), NP.float)
        nullData = NP.ones(nChan, NP.float) * NoData
        Weight = NP.zeros(nData, NP.float)
        Sp = NP.zeros((nData, nChan), NP.float)
        # combine data
        for row in range(nRow):
            for i in range(nData):
                tmp = NP.array(data[i]._getspectrum(row), NP.float)
                self.LogMessage('INFO', Origin=origin, Msg='row:%d data:%d Len:%d' % (row, i, len(tmp)))
                Sp[i] = NP.array(data[i]._getspectrum(row), NP.float)
                Tsys = data[i].get_tsys(row)
                if Sp[i][0] < (NoData+1) or Tsys == 0: Weight[i] =  0.0
                else: Weight[i] = 1.0/(Tsys**2.0)
            if Weight.sum() == 0: StorageOut[row] = nullData.copy()
            else: StorageOut[row] = (Sp.transpose()*Weight).sum(axis=1)/Weight.sum()
            (RArad, DECrad) = data[0].get_directionval(row)
            Table.append([data[0].getif(row), data[0].getpol(row), 0, 0, RArad*RAD2Deg%360, DECrad*RAD2Deg, 1, 0])
        # close all data
        for i in range(nData-1,-1,-1): del data[i]
        return (StorageOut, Table)


def convolve2d( data, kernel, mode='nearest', cval=0.0 ):
    """
    2d convolution function.

    mode = 'nearest'  use nearest pixel value for pixels beyond the edge
           'constant' use cval for pixels beyond the edge
    """
    (ndx,ndy) = data.shape
    (nkx,nky) = kernel.shape
    edgex = int( 0.5 * ( nkx - 1 ) )
    edgey = int( 0.5 * ( nky - 1 ) )
    dummy = NP.ones( (ndx+2*edgex,ndy+2*edgey), dtype=NP.float64 ) * cval
    dummy[edgex:ndx+edgex,edgey:ndy+edgey] = data
    if ( mode == 'nearest' ):
        dummy[0:edgex,0:edgey] = data[0][0]
        dummy[0:edgex,edgey+ndy:] = data[0][ndy-1]
        dummy[edgex+ndx:,0:edgey] = data[ndx-1][0]
        dummy[edgex+ndx:,edgey+ndy:] = data[ndx-1][ndy-1]
        for i in xrange(ndx):
            dummy[i+edgex,0:edgey] = data[i][0]
            dummy[i+edgex,edgey+ndy:] = data[i][ndy-1]
        for i in xrange(ndy):
            dummy[0:edgex,i+edgey] = data[0][i]
            dummy[edgex+ndx:,i+edgey] = data[ndx-1][i]
    cdata = NP.zeros( (ndx,ndy), dtype=NP.float64 ) 
    for ix in xrange(ndx):
        for iy in xrange(ndy):
            for jx in xrange( nkx ):
                for jy in xrange( nky ):
                    idx = ix + jx
                    idy = iy + jy
                    val = dummy[idx][idy]
                    cdata[ix][iy] += kernel[jx][jy] * val
    return cdata


def SpBinning(data, Bin):
    if Bin == 1: return data
    dataout = NP.zeros(int((len(data)+Bin-1)/Bin), NP.float)
    for i in range(int(len(data)/Bin)):
        ii = i*Bin
        dataout[i] = data[ii:ii+Bin].mean()
    if len(data)%Bin != 0:
        dataout[-1] = data[ii+Bin:-1].mean()
    return dataout

def getWidenLineList(nChan, mChan, pChan, Lines):
    Tmp, Out = [], []
    #print Lines, nChan, mChan, pChan, len(Lines)
    for i in range(0, len(Lines), 2):
        Tmp.append([Lines[i]-mChan, Lines[i+1]+pChan])
    if len(Tmp) > 1:
        for i in range(len(Tmp)-1, 0, -1):
            if Tmp[i-1][1] >= Tmp[i][0]-1:
                Tmp[i-1][1] = Tmp[i][1]
                Tmp.pop(i)
    for i in range(len(Tmp)):
        Out.append(Tmp[i][0])
        Out.append(Tmp[i][1])
    del Tmp
    if Out[0] < 0: Out[0] = 0
    if Out[-1] > nChan - 2: Out[-1] = nChan - 1
    return Out

def parseEdge(edge):
    if len(edge) == 2:
        (EdgeL, EdgeR) = edge
    else:
        EdgeL = edge[0]
        EdgeR = edge[0]
    return(EdgeL, EdgeR)


#def mergeResembleLines(ProR):
#    if len(ProR[2]) <= 1: return ProR
#    Tmp = []
#    for i in range(len(ProR[2])):
#        Tmp.append([ProR[2][i][0]+ProR[2][i][1], ProR[2][i][1]-ProR[2][i][0]])
#    Out = []
#    flag = NP.ones(len(ProR[2]))
#    for i in range(len(ProR[2])):
#        for j in range(i+1, len(ProR[2])):
#            # Width: Tmp[i][1], Center*2: Tmp[i][0]
#            if flag[j] == 1 and flag[i] == 1 and \
#               min(Tmp[i][1],Tmp[j][1])*1.5>max(Tmp[i][1],Tmp[j][1]) and \
#               abs(Tmp[i][0]-Tmp[j][0])<max(Tmp[i][1],Tmp[j][1]):
#                if Tmp[i][1]<Tmp[j][1]: flag[i] = 0
#                else: flag[j] = 0
#    for i in range(len(Tmp)):
#        if flag[i] == 1: Out.append(ProR[2][i])
#    del Tmp
#    ProR[2] = Out
#    return ProR



def createExportTable(tbName, nrow):
    tbLoc = gentools(['tb'])[0]

    desc = dict()

    desc['Row'] = {
        'comment': 'Row number',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer'
        }

    desc['Ant'] = {
        'comment': 'Antenna IDr',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer'
        }

    desc['FitFunc'] = {
        'comment': 'Baseline Fitting Function',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'string'
        }

    desc['SummaryFlag'] = {
        'comment': 'Summary Flag applied',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean'
        }

    desc['Sections'] = {
        'comment': 'Spectral baseline section coefficients',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer',
        'ndim': 2
        }

    desc['LineMask'] = {
        'comment': 'Line detected region',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer',
        'ndim': 2
        }

    desc['SectionCoefficients'] = {
        'comment': 'Spectral baseline section coefficients',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'double',
        'ndim': 2
        }

    desc['Statistics'] = {
        'comment': 'Spectral baseline RMS',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'double',
        'ndim': 1
        }

    desc['StatisticsFlags'] = {
        'comment': 'Statistics Flags by category',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean',
        'ndim': 1
        }

    desc['PermanentFlags'] = {
        'comment': 'Permanent Flags by category',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean',
        'ndim': 1
        }

    tbLoc.create(tbName, tabledesc=desc, nrow=nrow)

    return tbLoc




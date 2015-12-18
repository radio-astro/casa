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
# SDCalibration.py
#
# Class for single dish calibration
#
# -- 2008/8/26 Takeshi Nakazato
#
#    2008/8/26 TN    Only position switch is supported
#                     Limited use: GBT data filled by PKSreader
#
#    2009/3/11 TN    APEX data calibration (single-point position switch only)
#
#    2009/4/24 TN    APEX data calibration (position switch, OTF raster)
#
#    2009/xx/xx TN   APEX data calibration (frequency switch)
#
#    2009/11/13 TN   ALMA data without calibration metadata (just ON and OFF)
#
#
# $Revision: 1.10.2.2.2.1 $
# $Date: 2012/09/27 06:06:36 $
# $Author: tnakazat $
#
#import SDTool as SDT
#import casac
from SDTool import SDLogger, is_scantable, is_ms, dec_engines_logging, dec_engines_logfile
from taskinit import gentools

class SDCalibration( SDLogger ):
    """
    Class for single dish calibration
    """

    ###
    # initialization
    ###
    def __init__( self, filename, caltype='none', tsys=0.0, tcal=0.0, tau=0.0,
                  casaversion=0, LogLevel=2, LogFile=False ):
        """
        Create calibration object from filename
        Output (calibrated) data is written into filename+'_cal'
        Parameters:
            filename:   the name of an asap table
                        or the name of measurement set
            caltype:    calibration type ('ps',...)
            tsys:       system temperature [K]
            tcal:       calibration temperature [K]
            tau:        atmospheric optical depth
        """
        import os
        import sys
        from asap import rcParams

        # local tool
        #self._tbtool = casac.homefinder.find_home_by_name('tableHome')
        #self._tb = self._tbtool.create()
        #self._metool = casac.homefinder.find_home_by_name('measuresHome')
        #self._me = self._metool.create()
        self._tb,self._me = gentools(['tb','me'])
        
        # avoid memory occupation
        rcParams['scantable.storage']='disk'
        
        # logging
        SDLogger.__init__( self, level=LogLevel, origin='SDCalibration', fileout=False, Global=True )
        self.setFileOut( LogFile )
        
        # input
        filename = os.path.expandvars(filename)
        filename = os.path.expanduser(filename)
        self.__filein = filename
        self.__tabletype = self.gettabletype()
        # NOTE: self.__tablein is always scantable
        self.__tablein = self.importdata(LogLevel=LogLevel, LogFile=LogFile)
        
        # output
        self.__tableout = None
        self.__calib = False
        
        # calibration type is set automatically if 'none' is given
        if ( caltype != 'none' ):
            self.__caltype = caltype
        else:
            self.__caltype = self.getcaltype(LogLevel=LogLevel, LogFile=LogFile)
            
        # calibration parameters
        if ( tsys > 0.0 ):
            self.__tsys = tsys
        else:
            self.__tsys = self.gettsys(LogLevel=LogLevel, LogFile=LogFile)
        if ( tcal > 0.0 ):
            self.__tcal = tcal
        else:
            self.__tcal = self.gettcal(LogLevel=LogLevel, LogFile=LogFile)
        if ( tau > 0.0 ):
            self.__tau = tau
        else:
            self.__tau = self.gettau(LogLevel=LogLevel, LogFile=LogFile)
        self.tcalidlist=[]
        self.addtcal={}
        self.xfs = False
        self.ifdic = {}
        self.__tsyslist=[]
        self.__tsysidlist=[]

        # CASA version
        self.casaversion=casaversion

        # for WVR scan
        self.ifnos = None
        self.WVRScan = None

    ###
    # finalization
    ###
    def __del__( self ):
        """
        Destructor
        """
        #del self._tbtool
        del self._tb
        #del self._metool
        del self._me
##         if self.__tablein is not None:
##             del self.__tablein
##         if self.__tableout is not None:
##             del self.__tableout
        
    ###
    # print data summary
    ###
    @dec_engines_logfile
    @dec_engines_logging
    def summary( self, LogLevel=2, LogFile=False ):
        """
        Print input data summary
        """
        origin = 'summary()'
        #####self.setFileOut( LogFile )
        self.LogMessage('INFO', Origin=origin, Msg='Data Summary:')
        #self.LogMessage('INFO', Origin=origin, Msg=self.__tablein._summary())
        self.__tablein._summary()
        
    ###
    # calibration
    ###
    @dec_engines_logfile
    @dec_engines_logging
    def calibrate( self, LogLevel=2, LogFile=False ):
        """
        Do calibration
        """
        origin = 'calibrate()'
        #####self.setFileOut( self.fileOut )
        
        # return if already calibrated
        if ( self.__calib ):
            self.LogMessage( 'INFO', Origin=origin, Msg='Data already calibrated. Nothing to do.' )
            return

        # detect WVR scan
        self._detectWVRScan()

        if ( self.__caltype == 'ps' ):
            # position switch data calibration
            self.pscal(LogLevel=LogLevel, LogFile=LogFile)
        elif ( self.__caltype == 'fs' ):
            # frequency switch data calibration
            self.fscal(LogLevel=LogLevel, LogFile=LogFile)
        elif ( self.__caltype == 'nod' ):
            # nod data
            self.nodcal(LogLevel=LogLevel, LogFile=LogFile)
        elif ( self.__caltype == 'otf' ):
            # OTF data
            self.otfcal(LogLevel=LogLevel, LogFile=LogFile)
        elif ( self.__caltype == 'wob' ):
            # wobbler or nutator switching
            self.wobcal(LogLevel=LogLevel, LogFile=LogFile)
        elif ( self.__caltype == 'fold' ):
            # just fold the spectra
            self.fold()
        else:
            self.LogMessage('INFO', Origin=origin, Msg='Not implemented yet.')

        # self.__calib=True if calibrated
        if ( self.__tableout is not None ):
            self.__calib = True
        
    ###
    # save data
    ###
    @dec_engines_logfile
    @dec_engines_logging
    def save( self, outfile=None, LogLevel=2, LogFile=False ):
        """
        Save calibrated result as ASAP Table
        """
        origin = 'save()'
        #self.setFileOut( self.fileOut )
        
        if ( not self.__calib ):
            self.LogMessage('INFO', Origin=origin, Msg='Data is not calibrated yet. Do calibrate first.')
            return

        if ( outfile == None ):
            outfile = self.__filein.rstrip('/') + '_cal'
        if len(self.WVRScan) == 0:
            self.__tableout.save( outfile, format='ASAP', overwrite=True )
        else:
            from asap import selector
            ifnos = self.__tableout.getifnos()
            iflist = []
            for ifno in ifnos:
                if ifno not in self.WVRScan:
                    iflist.append( ifno )
            sel = selector()
            sel.set_ifs( iflist )
            self.__tableout.set_selection( sel )
            self.__tableout.save( outfile, format='ASAP', overwrite=True )
        self.LogMessage( 'INFO', Origin=origin, Msg='saved to %s' % outfile )

        # spectral unit should be Kelvin after the calibration
        self._tb.open( outfile, nomodify=False )
        self._tb.putkeyword( 'FluxUnit', 'K' )
        antname=self._tb.getkeyword( 'AntennaName' )
        self._tb.flush()
        self._tb.close()

        # remove _pson, _wobon, _fshi, _fslo, ... from SRCNAME
        if ( antname == 'APEX-12m' or antname.find( 'DV' ) != -1 or antname.find( 'PM' ) != -1 ):
            calstr=''
            if ( self.__caltype == 'ps' or self.__caltype == 'otf' ):
                calstr = '_pson'
            elif ( self.__caltype == 'wob' ):
                calstr = '_wobon'
            elif ( self.__caltype == 'fs' ):
                calstr = '_fslo'
            self.LogMessage( 'DEBUG', Origin=origin, Msg='Remove %s from SRCNAME' %(calstr) )
            self._tb.open( outfile, nomodify=False )
            srcname=self._tb.getcol('SRCNAME')
            srcname=srcname.tolist()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='before strip: srcname[0]=%s'%srcname[0] )
            for i in xrange(len(srcname)):
                ss=srcname[i]
                spos=ss.find(calstr)
                if spos!=-1:
                    srcname[i]=ss[0:ss.find(calstr)]
            self.LogMessage( 'DEBUG', Origin=origin, Msg='after strip: srcname[0]=%s'%srcname[0] )
            self._tb.putcol( 'SRCNAME', srcname )
            self._tb.flush()
            self._tb.close()

        # update TCAL and TCAL_ID
        if ( len(self.tcalidlist) != 0 ):
            if ( len(self.addtcal) != 0 ):
                self._tb.open( outfile+'/TCAL', nomodify=False )
                idx = self._tb.nrows()
                self._tb.addrows( len(self.addtcal) )
                for i in xrange( len(self.addtcal) ):
                    key = self.addtcal.keys()[i]
                    self._tb.putcell('ID',idx,idx)
                    self._tb.putcell('TIME',idx,self.addtcal[key][0])
                    self._tb.putcell('TCAL',idx,self.addtcal[key][1])
                    self.tcalidlist[key]=idx
                    idx=idx+1
                self._tb.close()
            self._tb.open( outfile, nomodify=False )
            for i in xrange( self._tb.nrows() ):
                self._tb.putcell( 'TCAL_ID', i, int(self.tcalidlist[i]) )
            self._tb.close()

        # update FREQ_ID
##         if ( self.xfs ):
##             self._tb.open( outfile, nomodify=False )
##             freqidcol = self._tb.getcol( 'FREQ_ID' )
##             for i in xrange( len(freqidcol) ):
##                 for j in self.ifdic.keys():
##                     idlo = self.ifdic[j][0]
##                     idhi = self.ifdic[j][1]
##                     if ( freqidcol[i] == idlo or freqidcol[i] == idhi ):
##                         freqidcol[i] = j
##             self._tb.putcol( 'FREQ_ID', freqidcol )
##             self._tb.close()

        # update TSYS
        if ( len(self.__tsysidlist) != 0 ):
            self._tb.open( outfile, nomodify=False )
            for i in xrange(len(self.__tsysidlist)):
                idx = self.__tsysidlist[i]
                if ( type(idx) != list ):
                    self._tb.putcell('TSYS',i,[self.__tsyslist[idx]])
                else:
                    tsys0 = self.__tsyslist[idx[0]]
                    tsys1 = self.__tsyslist[idx[1]]
                    x0 = idx[2]
                    x1 = idx[3]
                    tsys = (tsys1-tsys0)/(x0+x1)*x0+tsys0
                    self._tb.putcell('TSYS',i,[tsys])
            self._tb.close()

    ###
    # check if data was calibrated or not
    ###
    def isCalibrated( self ):
        return self.__calib
    
    ###
    # position switch calibration
    ###
    @dec_engines_logfile
    @dec_engines_logging
    def pscal( self, onstring='pson', offstring='psoff', LogLevel=2, LogFile=False ):
        """
        Calibrate Position Switch data.
        """
        origin = 'pscal()'
        #####self.setFileOut( self.fileOut )
        
        import os
        import numpy
        from asap._asap import srctype as SrcType
        from asap import scantable,asapmath,selector
        self.LogMessage('INFO', Origin=origin, Msg='Calibrating position switch data')

        # set srctype integer
        skyid = int(SrcType.sky)
        hotid = int(SrcType.hot)
        offid = int(SrcType.psoff)
        onid = int(SrcType.pson)

        # get antenna name
        antname = self.__tablein.get_antennaname()

        # GBT data
        if ( antname == 'GBT' ):
            isPsCal = False
            if ( self.__tablein.get_scan('*_ps*') is not None ):
                isPsCal = True
            else:
                sel = selector()
                sel.set_query( 'SRCTYPE == %s'%offid )
                try:
                    self.__tablein.set_selection( sel )
                    isPsCal = True
                except:
                    isPsCal = False
            if isPsCal:
                # GBT position switch data filled by PKSMS2reader
                self.LogMessage('INFO', Origin=origin, Msg='GBT data: using ASAP Tools (calps)')
                scannos = list( self.__tablein.getscannos() )
                tcal = self.__tcal
                if ( type(tcal) == list ):
                    tcal = tcal[0][0]
                tsys = self.__tsys
                if ( type(tsys) == list ):
                    tsys = 0.0
                tau = self.__tau
                if ( type(tau) == list ):
                    tau = tau[0][0]
                self.__tableout = asapmath.calps( self.__tablein, scannos, tsysval=tsys, tauval=tau, tcalval=tcal )
            else:
                # other
                self.LogMessage('INFO', Origin=origin, Msg='Not implemented yet.')
        # APEX or ALMA data
        else:
            if self.WVRScan is not None or len(self.WVRScan) > 0:
                sel = selector()
                sel.set_query('IFNO NOT IN %s'%(list(self.WVRScan)))
                self.__tablein.set_selection( sel )
                self.__tableout = asapmath.calibrate( self.__tablein, calmode='ps' )
                self.__tablein.set_selection()
            else:
                self.__tableout = asapmath.calibrate( self.__tablein, calmode='ps' )
##             isAPEX = ( antname.find('APEX') != -1 )
##             if isAPEX:
##                 self.__tableout = asapmath.apexcal( self.__tablein, calmode='ps' )
##             else:
##                 self.__tableout = asapmath.almacal( self.__tablein, calmode='ps' )
##             ifnos = self.ifnos
##             iflist = ifnos.tolist()
##             self.LogMessage( 'INFO', Origin=origin, Msg='%s data'%antname )
##             self._tb.open( self.__filein )
##             srcnamecol = self._tb.getcol( 'SRCNAME' )
##             srctypecol = self._tb.getcol( 'SRCTYPE' )
##             cyclecol = self._tb.getcol( 'CYCLENO' )
##             tcalidcol = self._tb.getcol( 'TCAL_ID' )
##             ifnocol = self._tb.getcol( 'IFNO' )
##             timestamps = self._tb.getcol( 'TIME' )
##             self._tb.close()
            
##             # get tcal id and time stamp
##             #ifnos = numpy.unique( ifnocol )
##             tmp1 = []
##             tmp2 = []
##             for i in ifnos:
##                 tmp1.append( [] )
##                 tmp2.append( [] )
##             for i in xrange( srcnamecol.shape[0] ):
##                 if ( srcnamecol[i].find( '_sky' ) != -1 and cyclecol[i] == 0 ):
##                     for j in xrange( len(ifnos) ):
##                         if ( ifnocol[i] == ifnos[j] ):
##                             tmp1[j].append( tcalidcol[i] )
##                             tmp2[j].append( float(timestamps[i]) )
##                             break
##             tcalid = {}
##             tcaltime = {}
##             for i in xrange(len(ifnos)):
##                 tcalid[ifnos[i]] = tmp1[i]
##                 tcaltime[ifnos[i]] = tmp2[i]
##             self.LogMessage( 'DEBUG', Origin=origin, Msg='tcalid = %s' % tcalid )
##             self.LogMessage( 'DEBUG', Origin=origin, Msg='tcaltime = %s' % tcaltime )
##             del timestamps
##             del tmp1
##             del tmp2
##             # get hot and sky
##             sel = selector()
##             isAPEX = ( antname.find('APEX') != -1 )
##             useSrcName = ( srcnamecol[0].find('_ps') != -1 or srcnamecol[0].find('_wob') != -1 )
##             if isAPEX:
##                 if useSrcName:
##                     sel.set_name( '*_sky' )
##                 else:
##                     sel.set_query( 'SRCTYPE == %s'%int(SrcType.sky) )
##                 sel.set_ifs( iflist )
##                 self.__tablein.set_unit( 'channel' ) # average_time works for channel only
##                 self.__tablein.set_selection( sel )
##                 asky = asapmath.average_time( self.__tablein, scanav=True )
##                 vorg=rcParams['verbose']
##                 if self.casaversion > 302:
##                     asaplog.disable()
##                 else:
##                     rcParams['verbose']=False
##                 self.__tsyslist=asky.get_tsys()
##                 if self.casaversion > 302:
##                     asaplog.enable()
##                 else:
##                     rcParams['verbose']=vorg
##                 nrsky = asky.nrow()
##                 self.LogMessage( 'DEBUG', Origin=origin, Msg='number of sky scan = %s' % nrsky )
##                 self.__tablein.set_selection()
##                 sel.reset()
##                 if useSrcName:
##                     sel.set_name( '*_hot' )
##                 else:
##                     sel.set_query( 'SRCTYPE == %s'%int(SrcType.hot) )
##                 sel.set_ifs( iflist )
##                 self.__tablein.set_selection( sel )
##                 ahot = asapmath.average_time( self.__tablein, scanav=True )
##                 nrhot = ahot.nrow()
##                 self.LogMessage( 'DEBUG', Origin=origin, Msg='number of hot scan = %s' % nrhot )
##                 self.__tablein.set_selection()
##                 sel.reset()
##             else:
##                 asky = None
##                 ahot = None
                
##             if isAPEX or useSrcName:
##                 sel.set_name( '*_%s'%offstring )
##             else:
##                 sel.set_query( 'SRCTYPE == %s'%offid )
##             try:
##                 sel.set_ifs( iflist )
##                 self.__tablein.set_selection( sel )
##                 soff=self.__tablein.copy()
##                 ifnos = soff.getifnos()
##                 beamnos = soff.getbeamnos()
##                 polnos = soff.getpolnos()
##                 # Specific data modification for OSF data
##                 # 2009/11/13 Takeshi Nakazato
##                 tmpname='tmp.SDCalibration.calibrate.asap'
##                 soff.save(tmpname,overwrite=True)
##                 self._tb.open(tmpname,nomodify=False)
##                 timecol=self._tb.getcol('TIME')
##                 ifnocol=self._tb.getcol('IFNO')
##                 polnocol=self._tb.getcol('POLNO')
##                 beamnocol=self._tb.getcol('BEAMNO')
##                 scancol=self._tb.getcol('SCANNO')
##                 dirval=self._tb.getcell('DIRECTION',0)
##                 for ii in ifnos:
##                     if ii in self.WVRScan:
##                         continue
##                     for ib in beamnos:
##                         for ip in polnos:
##                             idx=[]
##                             for irow in xrange(self._tb.nrows()):
##                                 self._tb.putcell('DIRECTION',irow,dirval)
##                                 if ( (ifnocol[irow] == ii) and (beamnocol[irow] == ib) and (polnocol[irow] == ip) ):
##                                     idx.append(irow)
##                             interval=0.0
##                             for irow in xrange(self._tb.nrows()):
##                                 if (ifnocol[irow] == ii):
##                                     interval=self._tb.getcell('INTERVAL',irow)/86400.0
##                                     break
##                             if len(idx) == 0:
##                                 continue
##                             stimecol = timecol.take( idx )
##                             timesep = stimecol[1:] - stimecol[:-1]
##                             gaplist=[]
##                             for irow in xrange(len(timesep)):
##                                 gap=timesep[irow]/interval
##                                 if gap>1.1:
##                                     gaplist.append(irow)
##                             self.LogMessage('DEBUG',Origin=origin, Msg='gaplist=%s'%gaplist)
##                             newid=scancol[0]
##                             for irow in xrange(len(idx)):
##                                 if gaplist.count(irow-1)!=0:
##                                     newid = newid + 1
##                                 scancol[idx[irow]]=newid
##                 self._tb.putcol('SCANNO',scancol)
##                 self._tb.flush()
##                 self._tb.close()
##                 del soff
##                 del timecol
##                 del ifnocol
##                 del polnocol
##                 del beamnocol
##                 del scancol
##                 soff=scantable(tmpname,False).copy()
##                 aoff = asapmath.average_time( soff, scanav=True, weight = 'tint' )
##                 del soff
##                 if os.path.exists(tmpname):
##                     os.system('rm -rf %s'%tmpname)
##                 #
##                 nroff = aoff.nrow()
##                 self.LogMessage( 'DEBUG', Origin=origin, Msg='number of off scan = %s' % nroff )
##             except Exception, e:
##                 if e.message=='Selection contains no data. Not applying it.':
##                     self.LogMessage('WARNING',Origin=origin, Msg='data does not contain reference scan. Calibration may be already done.')
##                     self.__tableout=self.__tablein
##                     aoff = None
##                     return
##                 else:
##                     tmpname='tmp.SDCalibration.calibrate.asap'
##                     if os.path.exists(tmpname):
##                         os.system('rm -rf %s'%tmpname)
##                     raise Exception, e
##             self.__tablein.set_selection()
##             sel.reset()
##             if isAPEX or useSrcName:
##                 sel.set_name( '*_%s'%onstring )
##             else:
##                 sel.set_query( 'SRCTYPE == %s' %onid )
##             sel.set_ifs( iflist )
##             self.__tablein.set_selection( sel )
##             self.__tableout = self.__tablein.copy()
##             nron = self.__tableout.nrow()
##             self.LogMessage( 'DEBUG', Origin=origin, Msg='number of on scan = %s' % nron )
##             self.__tablein.set_selection()
##             sel.reset()
##             self.__tsysidlist=[]
##             # loop on IF, BEAM, POL, SCAN
##             for i in xrange(nron):
##                 sref=self.__tableout.get_time(i,True)
##                 ii=self.__tableout.getif(i)
##                 if ii in self.WVRScan:
##                     continue
##                 ib=self.__tableout.getbeam(i)
##                 ip=self.__tableout.getpol(i)
##                 ic=self.__tableout.getscan(i)
##                 sel.reset()
##                 sel.set_ifs(ii)
##                 sel.set_beams(ib)
##                 sel.set_polarisations(ip)
##                 aoff.set_selection(sel)
##                 self.LogMessage('DEBUG',Origin=origin, Msg='search off scan for row %s (scanno=%s)' % (i,ic) )
##                 (idoff,spoff)=self.getspectrum( sref, aoff, LogLevel=LogLevel, LogFile=LogFile )
##                 spon=numpy.array(self.__tableout._getspectrum(i))
##                 if asky is not None and ahot is not None:
##                     asky.set_selection(sel)
##                     self.LogMessage('DEBUG',Origin=origin, Msg='search sky scan for row %s (scanno=%s)' % (i,ic) )
##                     (idsky,spsky)=self.getspectrum( sref, asky, LogLevel=LogLevel, LogFile=LogFile )
##                     ahot.set_selection(sel)
##                     self.LogMessage('DEBUG',Origin=origin, Msg='search hot scan for row %s (scanno=%s)' % (i,ic) )
##                     (idhot,sphot)=self.getspectrum( sref, ahot, LogLevel=LogLevel, LogFile=LogFile )
##                     [tcal,idx]=self.gettcalval( sref, tcalid[ii], tcaltime[ii], LogLevel=LogLevel, LogFile=LogFile )
##                     if antname == 'APEX-12m':
##                         # APEX calibration
##                         ta=((spon-spoff)/spoff)*(spsky/(sphot-spsky))*tcal
##                     else:
##                         # Classical chopper-wheel method (Ulich & Haas 1976)
##                         ta=(spon-spoff)/(sphot-spsky)*tcal
##                     self.__tsysidlist.append(idsky)
##                     self.tcalidlist.append(idx)
##                     asky.set_selection()
##                     ahot.set_selection()
##                 else:
##                     (idsky,spsky)=(-1,[])
##                     (idhot,spsky)=(-1,[])
##                     [tcal,idx]=[[0.0],-1]
##                     # Tsys times on-off/off
##                     tsys=self.__tablein._gettsys(i)
##                     ta=tsys*(spon-spoff)/spoff
##                     #self.tcalidlist.append(0)
##                 # set calibrated spectrum
##                 self.__tableout._setspectrum( ta, i )
##                 aoff.set_selection()
##                 if ( idx == -1 ):
##                     t=self.__tableout.get_time(i)
##                     ep = self._me.epoch( 'utc', t )
##                     t = '%s'%(ep['m0']['value'])
##                     self.addtcal[i] = [t,tcal]
##                     self.LogMessage('DEBUG',Origin=origin, Msg='add self.addtcal: %s'%([t,tcal]))
##             ### end of the loop on IF, BEAM, POL, SCAN
##             del aoff
            
    ###
    # frequency switch calibration
    ###
    @dec_engines_logfile
    @dec_engines_logging
    def fscal( self, LogLevel=2, LogFile=False ):
        """
        Calibrate Frequency Switch data.
        """
        origin = 'fscal()'
        #####self.setFileOut( self.fileOut )
        
        import numpy
        from asap._asap import srctype as SrcType
        from asap import scantable,asapmath,selector
        self.LogMessage('INFO', Origin=origin, Msg='Calibrating frequency switch data')
        antname = self.__tablein.get_antennaname()
        # GBT data
        if ( antname == 'GBT' ):
            symmetric = False 
            isFsCal = False
            if ( self.__tablein.get_scan('*_fs*') is not None ):
                isFsCal = True
            else:
                sel = selector()
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.fsoff) )
                try:
                    self.__tablein.set_selection( sel )
                    isFsCal = True
                except:
                    isFsCal = False
            if isFsCal:
                # GBT frequency switch data filled by PKSMS2reader
                self.LogMessage('INFO', Origin=origin, Msg='Using ASAP Tools: calfs')
                scannos = list( self.__tablein.getscannos() )
                tcal = self.__tcal
                if ( type(tcal) == list ):
                    tcal = tcal[0][0]
                tsys = self.__tsys
                if ( type(tsys) == list ):
                    tsys = 0.0
                tau = self.__tau
                if ( type(tau) == list ):
                    tau = tau[0][0]
                self.__tableout = calfs( self.__tablein, scannos, tsysval=tsys, tauval=tau, tcalval=tcal )
            else:
                # other
                self.LogMessage('INFO', Origin=origin, Msg='Not implemented yet.')
        # APEX or ALMA data
        else:
            ifnos = self.ifnos
            iflist = ifnos.tolist()
            symmetric = True 
            self._tb.open( self.__filein )
            #ftab = self._tb.getkeyword('FREQUENCIES').split()[-1]
            ftab = self._tb.getkeyword('FREQUENCIES').lstrip('Table: ')
            srcnamecol = self._tb.getcol( 'SRCNAME' )
            cyclecol = self._tb.getcol( 'CYCLENO' )
            tcalidcol = self._tb.getcol( 'TCAL_ID' )
            ifnocol = self._tb.getcol( 'IFNO' )
            ifidcol = self._tb.getcol( 'FREQ_ID' )
            timestamps = self._tb.getcol( 'TIME' )
            self._tb.close()
            useSrcName = ( srcnamecol[0].find('_fslo') != -1 )
            # get tcal id and time stamp
            #ifnos = numpy.unique( ifnocol )
            tmp1 = []
            tmp2 = []
            for i in ifnos:
                tmp1.append( [] )
                tmp2.append( [] )
            for i in xrange( srcnamecol.shape[0] ):
                if ( srcnamecol[i].find( '_sky' ) != -1 and cyclecol[i] < 2 ):
                    for j in xrange( len(ifnos) ):
                        if ( ifnocol[i] == ifnos[j] ):
                            tmp1[j].append( tcalidcol[i] )
                            tmp2[j].append( float(timestamps[i]) )
                            break
            self.LogMessage( 'DEBUG', Origin=origin, Msg='tmp1=%s' % tmp1 )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='tmp2=%s' % tmp2 )
            tcalid = {}
            tcaltime = {}
            for i in xrange(len(ifnos)):
                tcalid[ifnos[i]] = tmp1[i]
                tcaltime[ifnos[i]] = tmp2[i]
            self.LogMessage( 'DEBUG', Origin=origin, Msg='tcalid = %s' % tcalid )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='tcaltime = %s' % tcaltime )
            self._tb.open( ftab )
            ifdic1 = {} #key: ifno, value: ifid
            ifdic2 = {} #key: ifid, value: ifno
            for i in xrange( len( ifnocol ) ):
                if ifnocol[i] in self.WVRScan:
                    continue
                if ( not ifdic1.has_key( ifnocol[i] ) ):
                    ifdic1[ifnocol[i]] = ifidcol[i]
                if ( not ifdic2.has_key( ifidcol[i] ) ):
                    ifdic2[ifidcol[i]] = ifnocol[i]
                if ( len( ifdic1 ) >= self._tb.nrows() and len( ifdic2 ) >= self._tb.nrows() ):
                    break
            self.LogMessage( 'DEBUG', Origin=origin, Msg='ifdic1=%s' % ifdic1 )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='ifdic2=%s' % ifdic2 )
            fidcol = self._tb.getcol( 'ID' )
            refpix = self._tb.getcol( 'REFPIX' )
            refval = self._tb.getcol( 'REFVAL' )
            sorted = refval.copy()
            sorted.sort()
            incr = self._tb.getcol( 'INCREMENT' )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='id    : %s' % fidcol )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='refpix: %s' % refpix )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='refval: %s' % refval )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='sorted: %s' % sorted )
            self.ifdic={}
            fthrow={}
            # self.ifdic
            #   keys: unswitched frequency setting (FREQ_ID)
            #   values: self.ifdic[key][0] -> lower throw FREQ_ID
            #           self.ifdic[key][1] -> higher throw FREQ_ID
            # fthrow
            #   keys: unswitched frequency setting (FREQ_ID)
            #   values: fthrow[key][0] -> frequency throw in frequency
            #           fthrow[key][1] -> frequency throw in channel
            if ( len(numpy.unique(refpix)) == 1 ):
                for i in xrange( self._tb.nrows()/3 ):
                    kkey=fidcol[refval.tolist().index(sorted[3*i+1])]
                    self.LogMessage( 'DEBUG', Origin=origin, Msg='key=%s' % kkey )
                    vval = [fidcol[refval.tolist().index(sorted[3*i])],fidcol[refval.tolist().index(sorted[3*i+2])]]
                    self.LogMessage( 'DEBUG', Origin=origin, Msg='value=%s' % vval )
                    if ( vval[0] in ifdic2.values() and vval[1] in ifdic2.values() ):
                        self.ifdic[kkey]=vval
                        fthr = sorted[3*i+1] - sorted[3*i]
                        chthr = fthr / incr[refval.tolist().index(sorted[3*i])] 
                        fthrow[kkey] = [fthr, chthr]
            self._tb.close()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='self.ifdic=%s' % self.ifdic )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='fthrow=%s' % fthrow )
            del timestamps
            del tmp1
            del tmp2
            # get hot and sky
            sel = selector()
            if useSrcName:
                sel.set_name( '*_fslo_sky' )
            else:
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.flosky) )
            sel.set_ifs( iflist )
            self.__tablein.set_unit( 'channel' ) # average_time works for channel only
            self.__tablein.set_selection( sel )
            askylo = asapmath.average_time( self.__tablein, scanav=True )
            nrskylo = askylo.nrow()
            vorg=rcParams['verbose']
            if self.casaversion > 302:
                asaplog.disable()
            else:
                rcParams['verbose']=False
            self.__tsyslist = asskylo.get_tsys()
            if self.casaversion > 302:
                asaplog.enable()
            else:
                rcParams['verbose']=vorg
            self.LogMessage( 'DEBUG', Origin=origin, Msg='number of sky (low freq.) scan = %s' % nrskylo )
            self.__tablein.set_selection()
            sel.reset()
            if useSrcName:
                sel.set_name( '*_fshi_sky' )
            else:
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.fhisky) )
            sel.set_ifs( iflist )
            self.__tablein.set_unit( 'channel' ) # average_time works for channel only
            self.__tablein.set_selection( sel )
            askyhi = asapmath.average_time( self.__tablein, scanav=True )
            nrskyhi = askyhi.nrow()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='number of sky (high freq.) scan = %s' % nrskyhi )
            self.__tablein.set_selection()
            sel.reset()
            if useSrcName:
                sel.set_name( '*_fslo_hot' )
            else:
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.flohot) )
            sel.set_ifs( iflist )
            self.__tablein.set_selection( sel )
            ahotlo = asapmath.average_time( self.__tablein, scanav=True )
            nrhotlo = ahotlo.nrow()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='number of hot (low freq.) scan = %s' % nrhotlo )
            self.__tablein.set_selection()
            sel.reset()
            if useSrcName:
                sel.set_name( '*_fshi_hot' )
            else:
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.fhihot) )
            sel.set_ifs( iflist )
            self.__tablein.set_selection( sel )
            ahothi = asapmath.average_time( self.__tablein, scanav=True )
            nrhothi = ahothi.nrow()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='number of hot (high freq.) scan = %s' % nrhothi )
            self.__tablein.set_selection()
            sel.reset()
            # get off scan
            if useSrcName:
                sel.set_name( '*_fslo_off' )
            else:
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.flooff) )
            sel.set_ifs( iflist )
            self.__tablein.set_selection( sel )
            aofflo = asapmath.average_time( self.__tablein, scanav=True )
            nrofflo = aofflo.nrow()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='number of off (low freq.) scan = %s' % nrofflo )
            self.__tablein.set_selection()
            sel.reset()
            if useSrcName:
                sel.set_name( '*_fshi_off' )
            else:
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.fhioff) )
            sel.set_ifs( iflist )
            self.__tablein.set_selection( sel )
            aoffhi = asapmath.average_time( self.__tablein, scanav=True )
            nroffhi = aoffhi.nrow()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='number of off (high freq.) scan = %s' % nroffhi )
            self.__tablein.set_selection()
            sel.reset()
            # get fslo and fshi
            self.__tablein.set_selection()
            if useSrcName:
                sel.set_name( '*_fslo' )
            else:
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.fslo) )
            sel.set_ifs( iflist )
            self.__tablein.set_selection( sel )
            slo = self.__tablein.copy()
            nrlo= slo.nrow()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='number of low freq. scan = %s' % nrlo )
            self.__tablein.set_selection()
            sel.reset()
            if useSrcName:
                sel.set_name( '*_fshi' )
            else:
                sel.set_query( 'SRCTYPE == %s'%int(SrcType.fshi) )
            sel.set_ifs( iflist )
            self.__tablein.set_selection( sel )
            shi = self.__tablein.copy()
            nrhi = shi.nrow()
            self.LogMessage( 'DEBUG', Origin=origin, Msg='number of high freq. scan = %s' % nrhi )
            self.__tablein.set_selection()
            sel.reset()
            self.__tsysidlist = []
            ### loop on IF, BEAM, POL, SCAN for lower frequency
            for i in xrange(nrlo):
                sref=slo.get_time(i,True)
                ii=slo.getif(i)
                if ii in self.WVRScan:
                    continue
                ib=slo.getbeam(i)
                ip=slo.getpol(i)
                ic=slo.getscan(i)
                sel.reset()
                sel.set_ifs(ii)
                sel.set_beams(ib)
                sel.set_polarisations(ip)
                aofflo.set_selection(sel)
                askylo.set_selection(sel)
                ahotlo.set_selection(sel)
                sel.set_scans(ic)
                self.LogMessage('DEBUG',Origin=origin, Msg='search sky scan for row %s (scanno=%s)' % (i,ic) )
                (idsky,spsky)=self.getspectrum( sref, askylo, LogLevel=LogLevel, LogFile=LogFile )
                self.LogMessage('DEBUG',Origin=origin, Msg='search hot scan for row %s (scanno=%s)' % (i,ic) )
                (idhot,sphot)=self.getspectrum( sref, ahotlo, LogLevel=LogLevel, LogFile=LogFile )
                spon=numpy.array(slo._getspectrum(i))
                self.LogMessage('DEBUG',Origin=origin, Msg='search off scan for row %s (scanno=%s)' % (i,ic) )
                (idoff,spoff)=self.getspectrum( sref, aofflo , LogLevel=LogLevel, LogFile=LogFile)
                self.LogMessage('DEBUG',Origin=origin, Msg='search tcal for row %s (scanno=%s)' % (i,ic) )
                [tcal,idx]=self.gettcalval( sref, tcalid[ii], tcaltime[ii], LogLevel=LogLevel, LogFile=LogFile )
                # Classical chopper-wheel method (Ulich & Haas 1976)
                # ta=(spon-spoff)/(sphot-spsky)*tcal
                # APEX calibration
                ta=((spon-spoff)/spoff)*(spsky/(sphot-spsky))*tcal
                slo._setspectrum( ta, i )
                setf.__tsysidlist.append(idsky)
                aofflo.set_selection()
                askylo.set_selection()
                ahotlo.set_selection()
            ### end of the loop on IF, BEAM, POL, SCAN for lower frequency
            ### loop on IF, BEAM, POL, SCAN for higher frequency
            for i in xrange(nrhi):
                sref=shi.get_time(i,True)
                ii=shi.getif(i)
                if ii in self.WVRScan:
                    continue
                ib=shi.getbeam(i)
                ip=shi.getpol(i)
                ic=shi.getscan(i)
                sel.reset()
                sel.set_ifs(ii)
                sel.set_beams(ib)
                sel.set_polarisations(ip)
                aoffhi.set_selection(sel)
                askyhi.set_selection(sel)
                ahothi.set_selection(sel)
                sel.set_scans(ic)
                self.LogMessage('DEBUG',Origin=origin, Msg='search sky scan for row %s (scanno=%s)' % (i,ic) )
                (idsky,spsky)=self.getspectrum( sref, askyhi, LogLevel=LogLevel, LogFile=LogFile )
                self.LogMessage('DEBUG',Origin=origin, Msg='search hot scan for row %s (scanno=%s)' % (i,ic) )
                (idhot,sphot)=self.getspectrum( sref, ahothi, LogLevel=LogLevel, LogFile=LogFile )
                spon=numpy.array(shi._getspectrum(i))
                self.LogMessage('DEBUG',Origin=origin, Msg='search off scan for row %s (scanno=%s)' % (i,ic) )
                (idoff,spoff)=self.getspectrum( sref, aoffhi, LogLevel=LogLevel, LogFile=LogFile )
                self.LogMessage('DEBUG',Origin=origin, Msg='search tcal for row %s (scanno=%s)' % (i,ic) )
                [tcal,idx]=self.gettcalval( sref, tcalid[ii], tcaltime[ii], LogLevel=LogLevel, LogFile=LogFile )
                # Classical chopper-wheel method (Ulich & Haas 1976)
                # ta=(spon-spoff)/(sphot-spsky)*tcal
                # APEX calibration
                ta=((spon-spoff)/spoff)*(spsky/(sphot-spsky))*tcal
                shi._setspectrum( ta, i )
                aoffhi.set_selection()
                askyhi.set_selection()
                ahothi.set_selection()
            ### end of the loop on IF, BEAM, POL, SCAN for higher frequency
            ### merge spectra
            beamnos = slo.getbeamnos()
            polnos = slo.getpolnos()
            scannos = slo.getscannos()
            for i in self.ifdic.keys():
                for ib in beamnos:
                    for ip in polnos:
                        for isc in scannos:
                            sel.reset()
                            sel.set_ifs( int(ifdic2[self.ifdic[i][0]]) )
                            sel.set_beam( ib )
                            sel.set_polarizations( ip )
                            sel.set_scan( isc )
                            slo.set_selection( sel )
                            sel.reset()
                            sel.set_ifs( int(ifdic2[self.ifdic[i][1]]) )
                            sel.set_beam( ib )
                            sel.set_polarizations( ip )
                            sel.set_scan( isc )
                            shi.set_selection( sel )
                            for irow in xrange( slo.nrow() ):
                                splo = numpy.array(slo._getspectrum( irow ))
                                #tsyslo = numpy.array(slo._gettsys( irow ))
                                sphi = numpy.array(shi._getspectrum( irow ))
                                #tsyshi = numpy.array(shi._gettsys( irow ))
                                spec = sphi - splo
                                shi._setspectrum( spec, irow )
                            slo.set_selection()
                            shi.set_selection()
            # change frequency setting to unswitched one
            tmpname = 'SDCalibration.fscal.tmp'
            shi.save( tmpname, 'ASAP', True )
            self._tb.open( tmpname, nomodify=False )
            freqidcol=self._tb.getcol('FREQ_ID')
            for irow in xrange(self._tb.nrows()):
                for ids in self.ifdic.keys():
                    if freqidcol[irow] == self.ifdic[ids][1]:
                        freqidcol[irow] = ids
                        break
            self._tb.putcol( 'FREQ_ID', freqidcol )
            self._tb.flush()
            self._tb.close()
            del self.__tableout
            self.__tableout = scantable( tmpname, False ).copy()
            os.system( 'rm -rf '+tmpname )
                            
                
            ### folding
            self.fold( symmetric=symmetic, throw=fthrow, LogLevel=LogLevel, LogFile=LogFile )
            
    @dec_engines_logfile
    @dec_engines_logging
    def fold( self, symmetric=True, fthrow=3.75e6, LogLevel=2, LogFile=False ):
        """
        folding
        """
        origin = 'fold()'
        #####self.setFileOut( self.fileOut )
##         self.setLogLevel( LogLevel )

        import os
        import numpy
        from asap import selector, scantable

        if self.__tableout == None:
            # create self.__tableout if not exist, just try folding
            self.__tableout = self.__tablein.copy()
            self.__calib = True
        else:
            # if self.__tableout exists, self.__calib must be True
            if not self.__calib:
                self.LogMessage( 'WARNING', Origin=origin, Msg='Data should be calibrated before folding. Return.' )
                return

        # if fthrow is given as double
        if type(fthrow) is not dict:
            ffthrow = fthrow
            fthrow = {}
            fthrow[0] = [ffthrow, None]

        # get IFNO FREQ_ID connection
        # ifdic:
        #     key: IFNO
        #     value: FREQ_ID
        ifnos = self.__tableout.getifnos()
        ifdic = {}
        tmpname = 'SDCalibration.fold.tmp'
        self.__tableout.save( tmpname, overwrite=True )
        self._tb.open( tmpname )
        for ii in ifnos:
            tbsel = self._tb.query( 'IFNO == %s'%ii )
            ifdic[ii] = tbsel.getcell( 'FREQ_ID', 0 )
            tbsel.close()
            del tbsel
        self._tb.close()
        os.system( 'rm -rf '+tmpname )

        chdel = {}
        for i in ifnos:
            # for each IFNO
            sel = selector()
            sel.set_ifs( i )
            self.__tableout.set_selection( sel )
            unitorg = self.__tableout.get_unit()
            self.__tableout.set_unit( 'Hz' )
            abc = self.__tableout._getabcissa( 0 )
            self.__tableout.set_unit( unitorg )
            nchan = len( abc )
            df = abc[1] - abc[0]
            #df = -4882.81
            self.LogMessage( 'DEBUG', Origin=origin, Msg='df=%s'%df )
            bw = abs(df) * nchan
            #self.__tableout.set_selection()
            if ( fthrow[ifdic[i]][0] >= bw ):
                self.LogMessage( 'INFO', Origin=origin, Msg='No overlapped region, no folding' )
                return
            
            self.LogMessage( 'INFO', Origin=origin, Msg='do folding' )
            
            # fill fthrow[FREQ_ID][1] if not filled
            if fthrow[ifdic[i]][1] == None:
                fthrow[ifdic[i]][1] = fthrow[ifdic[i]][0] / df

            # for symmetric throw, fthrow should be > 0
            if symmetric:
                fthrow[ifdic[i]][0] = abs(fthrow[ifdic[i]][0])
                fthrow[ifdic[i]][1] = abs(fthrow[ifdic[i]][1])

            self.LogMessage( 'DEBUG', Origin=origin, Msg='fthrow=%s'%fthrow )

            # shift and fold spectra, flagtra, ...
            chthint = int(abs(fthrow[ifdic[i]][1]))
            chthdec = abs(fthrow[ifdic[i]][1]) - chthint
            if chthdec > 0.5:
                chthint = chthint + 1
                chthdec = chthdec - 1
            elif chthdec < -0.5:
                chthint = chthint - 1
                chthdec = chthdec + 1
            self.LogMessage( 'DEBUG', Origin=origin, Msg='chthint=%s, chthdec=%s'%(chthint,chthdec) )
            newtsys = []
            for irow in xrange( self.__tableout.nrow() ):
                # spectra
                spec = numpy.array(self.__tableout._getspectrum( irow ))
                spfold = None
                if symmetric:
                    if df > 0:
                        spl = self._shift( spec, -chthint, -chthdec )
                        sph = self._shift( spec, chthint, chthdec )
                    else:
                        spl = self._shift( spec, chthint, chthdec )
                        sph = self._shift( spec, -chthint, -chthdec )
                    spfold = 0.5 * ( sph - spl )
                    chdel[i] = [chthint+1,'both'] 
                elif fthrow[ifdic[i]][0] > 0:
                    # higher frequency throw
                    if df > 0:
                        spl = spec
                        sph = self._shift( spec, chthint, chthdec )
                        chdel[i] = [chthint+1,'left']
                    else:
                        spl = spec
                        sph = self._shift( spec, -chthint, -chthdec )
                        chdel[i] = [chthint+1,'right']
                    spfold = 0.5 * ( spl - sph )
                elif fthrow[ifdic[i]][0] < 0:
                    # lower frequency throw
                    if df > 0:
                        spl = self._shift( spec, -chthint, -chthdec )
                        sph = spec
                        chdel[i] = [chthint+1,'right']
                    else:
                        spl = self._shift( spec, chthint, chthdec )
                        sph = spec
                        chdel[i] = [chthint+1,'left']
                    spfold = 0.5 * ( sph - spl )
                self.__tableout._setspectrum( spfold, irow )

                # flagtra
                if ( irow is 0 ):
                    # If one spectra has bad channel, others might be...
                    # flag: True is unflagged, False is flagged
                    flag = numpy.array(self.__tableout._getmask( irow ))
                    if ( all(flag) == False ):
                        self.__tableout.flag( unflag=True )
                        if symmetric:
                            if df > 0:
                                fll = self._shift( flag, -chthint, -chthdec )
                                flh = self._shift( flag, chthint, chthdec )
                            else:
                                fll = self._shift( flag, chthint, chthdec )
                                flh = self._shift( flag, -chthint, -chthdec )
                            flfold = numpy.logical_and( flh, fll )
                        elif fthrow[ifdic[i]][0] > 0:
                            # higher frequency throw
                            if df > 0:
                                fll = flag
                                flh = self._shift( flag, chthint, chthdec )
                            else:
                                fll = flag
                                flh = self._shift( flag, -chthint, -chthdec )
                            flfold = numpy.logical_and( flh, fll )
                        elif fthrow[ifdic[i]][0] < 0:
                            # lower frequency throw
                            if df > 0:
                                fll = self._shift( flag, -chthint, -chthdec )
                                flh = flag
                            else:
                                fll = self._shift( flag, chthint, chthdec )
                                flh = flag
                            flfold = numpy.logical_and( flh, fll )
                        # invert flag: True is flagged, False is unflagged
                        flfold = ( flfold == False )
                        self.__tableout.flag( flfold.tolist() )

                # tsys (necessary?)
##                 tsys = numpy.array(slo._gettsys( irow ))
##                 if ( tsys.shape is not () and tsys.shape[0] == nchan ):
##                     tsysfold = None
##                     if symmetric:
##                         if df > 0:
##                             tsysl = self._shift( tsys, -chthint, -chthdec )
##                             tsysh = self._shift( tsys, chthint, chthdec )
##                         else:
##                             tsysl = self._shift( tsys, chthint, chthdec )
##                             tsysh = self._shift( tsys, -chthint, -chthdec )
##                         tsysfold = 0.5 * ( tsysh - tsysl )
##                     elif fthrow[ifdic[i]][0] > 0:
##                         # higher frequency throw
##                         if df > 0:
##                             tsysl = tsys
##                             tsysh = self._shift( tsys, chthint, chthdec )
##                         else:
##                             tsysl = tsys
##                             tsysh = self._shift( tsys, -chthint, -chthdec )
##                         tsysfold = 0.5 * ( tsysl - tsysh )
##                     elif fthrow[ifdic[i]][0] < 0:
##                         # lower frequency throw
##                         if df > 0:
##                             tsysl = self._shift( tsys, -chthint, -chthdec )
##                             tsysh = tsys
##                         else:
##                             tsysl = self._shift( tsys, chthint, chthdec )
##                             tsysh = tsys
##                         tsysfold = 0.5 * ( tsysh - tsysl )
            self.__tableout.set_selection()
            sel.reset()
            del sel
        self.__tableout.save( tmpname, overwrite=True )
        #tb2 = self._tbtool.create()
        tb2 = gentools(['tb'])[0]
        tb2.open( tmpname+'/TCAL', nomodify=False )
        self._tb.open( tmpname, nomodify=False )
        # modify nChan and Bandwidth
        nchan = self._tb.getkeyword( 'nChan' )
        bw = self._tb.getkeyword( 'Bandwidth' )
        chw = bw / nchan
        if chdel[i][1] == 'both':
            bw = bw - chw * chdel[ifnos[0]][0] * 2
            nchan = nchan - chdel[ifnos[0]][0] * 2
        else:
            bw = bw - chw * chdel[ifnos[0]][0]
            nchan = nchan - chdel[ifnos[0]][0]
        self._tb.putkeyword( 'Bandwidth', bw )
        self._tb.putkeyword( 'nChan', nchan )

        # for each IFNO
        rowids = {}
        newtcal = {}
        ifnocol = self._tb.getcol( 'IFNO' )
        for i in ifnos:
            rowids[i] = []
        for irow in xrange(self._tb.nrows()):
            rowids[ifnocol[irow]].append( irow )
        for i in ifnos:
            for irow in rowids[i]:
                # modify SPECTRA and FLAGTRA
                sp = self._tb.getcell( 'SPECTRA', irow )
                spchan = len(sp)
                fl = self._tb.getcell( 'FLAGTRA', irow )
                if chdel[i][1] == 'both':
                    sp = sp[chdel[i][0]:-chdel[i][0]]
                    fl = fl[chdel[i][0]:-chdel[i][0]]
                elif chdel[i][1] == 'left':
                    sp = sp[chdel[i][0]:]
                    fl = fl[chdel[i][0]:]
                elif chdel[i][1] == 'right':
                    sp = sp[:-chdel[i][0]]
                    fl = fl[:-chdel[i][0]]
                self._tb.putcell( 'SPECTRA', irow, sp )
                self._tb.putcell( 'FLAGTRA', irow, fl )

                # modify TSYS if required
                tsys = self._tb.getcell( 'TSYS', irow )
                if len(tsys) == spchan:
                    if chdel[i][1] == 'both':
                        tsys = tsys[chdel[i][0]:-chdel[i][0]]
                    elif chdel[i][1] == 'left':
                        tsys = tsys[chdel[i][0]:]
                    elif chdel[i][1] == 'right':
                        tsys = tsys[:-chdel[i][0]]
                    self._tb.putcell( 'TSYS', irow, tsys )
                    
                # modify TCAL if required
                tcalid = self._tb.getcell( 'TCAL_ID', irow )
                tcal = tb2.getcell( 'TCAL', tcalid )
                if len(tcal) == spchan:
                    if chdel[i][1] == 'both':
                        tcal = tcal[chdel[i][0]:-chdel[i][0]]
                    elif chdel[i][1] == 'left':
                        tcal = tcal[chdel[i][0]:]
                    elif chdel[i][1] == 'right':
                        tcal = tcal[:-chdel[i][0]]
                    tb2.putcell( 'TCAL', tcalid, tcal )
        self._tb.flush()
        tb2.flush()
        self._tb.close()
        tb2.close()
        del tb2

        del self.__tableout
        self.__tableout = scantable( tmpname, False ).copy()
        os.system( 'rm -rf '+tmpname )
        if self.casaversion > 302:
            asaplog.disable()
        else:
            rcParams['verbose']=False
        self.__tsyslist = self.__tableout.get_tsys()
        if self.casaversion > 302:
            asaplog.enable()
        else:
            rcParams['verbose']=vorg
        self.__tsysidlist = range( self.__tableout.nrow() )
        
        self.xfs = True

                    
    ###
    # calibrate nod data
    ###
    @dec_engines_logfile
    @dec_engines_logging
    def nodcal( self, LogLevel=2, LogFile=False ):
        """
        Calibrate nod data.
        """
        origin = 'nodcal()'
        #####self.setFileOut( self.fileOut )
        
        self.LogMessage('INFO', Origin=origin, Msg='Calibrating nod data')

    ###
    # calibrate OTF data
    ###
    @dec_engines_logfile
    @dec_engines_logging
    def otfcal( self, LogLevel=2, LogFile=False ):
        """
        Calibrate OTF data.
        """
        origin = 'otfcal()'
        #####self.setFileOut( self.fileOut )
        
        self.LogMessage('INFO', Origin=origin, Msg='Calibrating OTF data')
        antname = self.__tablein.get_antennaname()
        # 2011/11/8 TN
        # Comment out 
        # APEX data
        #if ( antname == 'APEX-12m' or antname.find( 'DV' ) != -1 or antname.find( 'PM' ) != -1 ):
        self.pscal(LogLevel=LogLevel, LogFile=LogFile)

    ###
    # calibrate wobbler or nutator switching data
    ###
    @dec_engines_logfile
    @dec_engines_logging
    def wobcal( self, LogLevel=2, LogFile=False ):
        """
        Calibrate wobbler or nutator switching data.
        """
        origin = 'wobcal()'
        #####self.setFileOut( self.fileOut )
        
        self.LogMessage('INFO', Origin=origin, Msg='Calibrating wobbler or nutator switching data')
        onstring = 'wobon'
        offstring = 'woboff'
        self.pscal( onstring=onstring, offstring=offstring, LogLevel=LogLevel, LogFile=LogFile )

    ###
    # examine table type
    ###
    #@dec_engines_logfile
    def gettabletype( self, LogLevel=2, LogFile=False ):
        """
        Get table type
        """
        origin = 'gettabletype()'
        #####self.setFileOut( self.fileOut )
        
        import re
        import os
        type = 'unknown'
        
        if os.path.isdir( self.__filein ):
##             if os.path.exists( self.__filein+'/table.f1' ):
##                 type = 'ms2'
##             elif os.path.exists( self.__filein+'/table.info' ):
##                 type = 'asap'
            if is_ms( self.__filein ):
                type = 'ms2'
            elif is_scantable( self.__filein ):
                type = 'asap'
            elif os.path.exists( self.__filein+'/ASDM.xml' ):
                type = 'asdm'
        elif re.search( '\.fits$', self.__filein.lower() ) is not None:
            type = 'fits'
            
        self.LogMessage( 'INFO', Origin=origin, Msg='Table type is %s' % type )
        return type
    
    ###
    # examine calibration type
    ###
    #@dec_engines_logfile
    def getcaltype( self, LogLevel=2, LogFile=False ):
        """
        Examine calibration type from input table
        """
        origin = 'gettcaltype()'
        #####self.setFileOut( self.fileOut )
        
        import os
        import numpy
        import xml.dom.minidom as DOM
        from asap._asap import srctype as SrcType

        type = 'unknown'
        if self.__tabletype == 'asdm':
            # ASDM
            # NOTE: I don't know how I can specify calibration type
            scanTable = self.__filein+('/Scan.xml')
            dom3 = DOM.parse( scanTable )
            rootnode = dom3.getElementsByTagName( 'scanIntent' )[0]
            intelm = rootnode
            intstr = intelm.childNodes[0].data.encode( 'UTF-8' )
            if ( intstr.find( 'OBSERVE_TARGET' ) != -1 ):
                type = 'otf'
            dom3.unlink()
        elif self.__tabletype == 'ms2':
            # MS2
            self._tb.open(self.__filein)
            #statetable = self._tb.getkeyword('STATE').split()[-1]
            statetable = self._tb.getkeyword('STATE').lstrip('Table: ')
            self._tb.close()
            if os.path.exists( statetable ):
                self._tb.open( statetable )
                obsmode = self._tb.getcol( 'OBS_MODE' )[0]               
                if ( obsmode.find( 'PSWITCH' ) != -1 ):
                    type = 'ps'
                elif ( obsmode.find( 'FSWITCH' ) != -1 ):
                    type = 'fs'
        elif self.__tabletype == 'fits':
            # FITS
            # NOTE: I have no way to find calibration type
            type = 'ps'
        elif self.__tabletype == 'asap':
            self._tb.open( self.__filein )
            srctypecol = self._tb.getcol('SRCTYPE')
            srctypes = numpy.unique( srctypecol )
            if ( self.__tablein.get_antennaname() == 'GBT' ):
                if ( ( self.__tablein.get_scan('*_ps*') is not None )
                     or ( numpy.searchsorted( srctypes, int(SrcType.psoff) ) != len(srctypes) ) ):
                    type = 'ps'
                elif ( ( self.__tablein.get_scan('*_fs*') is not None )
                       or ( numpy.searchsorted( srctypes, int(SrcType.fsoff) ) != len(srctypes) ) ):
                    type = 'fs'
                elif ( ( self.__tablein.get_scan('*_nod*') is not None )
                       or ( numpy.searchsorted( srctypes, int(SrcType.nod) ) != len(srctypes) ) ):
                    type = 'nod'
            elif ( self.__tablein.get_antennaname() == 'APEX-12m' ):
                self._tb.open( self.__filein )
                obstype = self._tb.getkeyword( 'Obstype' )
                if ( obstype.find( 'FSW' ) != -1 ):
                    type = 'fs'
                elif ( obstype.find( 'OTF' ) != -1 ):
                    type = 'otf'
                elif ( obstype.find( 'WOBSW' ) != -1 ):
                    type = 'wob'
                elif ( obstype.find( 'TOTP' ) != -1 ):
                    type = 'ps'
            else:
                self._tb.open( self.__filein )
                obstype = self._tb.getkeyword( 'Obstype' )
                if ( obstype.find( 'PSWITCH' ) != -1 ):
                    type = 'ps'
                elif ( obstype.find( 'FSWITCH' ) != -1 ):
                    type = 'fs'
                #elif ( obstype.find( 'OBSERVE_TARGET' ) != -1 ):
                elif ( obstype.find( 'ON_SOURCE' ) != -1 or obstype.find( 'OFF_SOURCE' ) != -1 ):
                    # ALMA OSF data (OTF is default obs. mode)
                    type = 'otf' 
        self.LogMessage( 'INFO', Origin=origin, Msg='Calibration type is %s' % type )
        return type
        
    ###
    # get system temperature
    ###
    def gettsys( self, LogLevel=2, LogFile=False ):
        """
        Get system temperature
        """
        val = 0.0
        
        # from memory
        tsysval = []
        for i in xrange( self.__tablein.nrow() ):
            tsysval.append( self.__tablein._gettsys( i ) )
        if tsysval is not None:
            val = tsysval
        return val

    ###
    # get calibration temperature
    ###
    #@dec_engines_logfile
    def gettcal( self, LogLevel=2, LogFile=False ):
        """
        Get calibration temperature
        """
        origin = 'gettcal()'
        #####self.setFileOut( self.fileOut )
        
        import os
        import xml.dom.minidom as DOM
        val = 0.0
        
        # from memory
        # there is no way to get Tcal from scantable
        
        # from disk
        if ( self.__tabletype == 'ms2' ):
            self._tb.open(self.__filein)
            #syscaltable = self._tb.getkeyword('SYSCAL').split()[-1]
            syscaltable = self._tb.getkeyword('SYSCAL').lstrip('Table: ')
            self._tb.close()
            self._tb.open( syscaltable )
            # TODO: assign Tcal for each observation row
            #       Tcal are defined for each time and each spectral window
            tcalcol = self._tb.getcol( 'TCAL' )
            if tcalcol is not None:
                val = tcalcol
        elif ( self.__tabletype == 'asap' ):
            self._tb.open(self.__filein)
            #tcaltable = self._tb.getkeyword('TCAL').split()[-1]
            tcaltable = self._tb.getkeyword('TCAL').lstrip('Table: ')
            self._tb.close()
            self._tb.open( tcaltable )
            # TODO: assign Tcal for each observation row
            #       Tcal are defined for each time
            # tcalcol = self._tb.getcol( 'TCAL' )
            tcalcol = []
            for i in xrange( self._tb.nrows() ):
                tcalcol.append( self._tb.getcell( 'TCAL', i ) )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='len(tcalcol) = %s' % len(tcalcol) )
            if ( len( tcalcol ) != 0 ):
                val = tcalcol
            self._tb.close()
        elif ( self.__tabletype == 'asdm' ):
            syscaltable = self.__filein+'/SysCal.xml'
            val = []
            if ( os.path.exists( syscaltable ) ):
                dom3 = DOM.parse( syscaltable )
                rootnode = dom3.documentElement
                tcalelm = rootnode.getElementsByTagName( 'tcal' )
                for i in xrange( len(tcalelm) ):
                    tcalstr = tcalelm[i].childNodes[0].data.encode( 'UTF-8' )
                    strs = tcalstr.split()
                    dim = int(strs.pop( 0 ))
                    if dim == 1:
                        axislen = int(strs.pop( 0 ))
                        for j in xrange( axislen ):
                            val.append( float(strs[j]) )
                    else:
                        self.LogMessage( 'INFO', Origin=origin, Msg='gettau: tcal should be 1-dimensional!' )
                        val = 0.0
                dom3.unlink()
            else:
                self.LogMessage( 'INFO', Origin=origin, Msg=syscaltable+' does not exist.' )
                val = 0.0
        elif ( self.__tabletype == 'fits' ):
            # I don't know how I can retrieve Tsys from header
            self.LogMessage( 'INFO', Origin=origin, Msg='I don\'t know how I can retrieve Tsys from FITS' )
        
        return val

    ###
    # get atmospheric optical depth
    ###
    #@dec_engines_logfile
    def gettau( self, LogLevel=2, LogFile=False ):
        """
        Get atmospheric optical depth
        """
        origin = 'gettau()'
        #####self.setFileOut( self.fileOut )
        
        import os
        import xml.dom.minidom as DOM
        val = 0.0
        
        # from memory
        # there is no way to get tau from scantable
        
        # from disk
        # if ( self.__tabletype == 'ms2' ):
        # Idon't know how I can retrieve tau from the table
        # elif ( self.__tabletype == 'asap' ):
        # Idon't know how I can retrieve tau from the table
        if ( self.__tabletype == 'asdm' ):
            val = []
            calatmtable = self.__filein+'/CalAtmosphere.xml'
            if ( os.path.exists( calatmtable ) ):
                dom3 = DOM.parse( calatmtable )
                rootnode = dom3.documentElement
                tauelm = rootnode.getElementsByTagName( 'tauSpectrum' )
                for i in xrange( len(tauelm) ):
                    taustr = tauelm[i].childNodes[0].data.encode( 'UTF-8' )
                    strs = taustr.split()
                    dim = int(strs.pop( 0 ))
                    if dim == 2:
                        xlen = int(strs.pop( 0 ))
                        ylen = int(strs.pop( 0 ))
                        # NOTE: I'm not sure how data are sorted
                        for j in xrange( xlen ):
                            pval = []
                            for k in xrange( ylen ):
                                pval.append( float(strs[k*ylen+j]) )
                            val.append( pval() )
                    else:
                        self.LogMessage( 'INFO', Origin=origin, Msg='gettau: tauSpectrum should be 2-dimensional!' )
                        val = 0.0
                dom3.unlink()
            else:
                self.LogMessage( 'INFO', Origin=origin, Msg=calatmtable+' does not exist.' )
                val = 0.0
        # elif ( self.__tabletype == 'fits' ):
        # I don't know how I can retrieve Tsys from header
        
        # NOTE: return value is scalar at the moment
        self.LogMessage( 'INFO', Origin=origin, Msg='tau = %s' %val )
        return val

    ###
    # import data
    ###
    #@dec_engines_logfile
    def importdata( self, LogLevel=2, LogFile=False ):
        """
        Import data
        """
        origin = 'importdata()'
        #####self.setFileOut( self.fileOut )
        
        import os
        from tasks import importasdm
        from asap import scantable
        s = None
        if ( self.__tabletype == 'ms2' ):
            s = scantable( self.__filein, average=False, getpt=True )
        elif ( self.__tabletype == 'asap' ):
            s = scantable( self.__filein, average=False )
        elif ( self.__tabletype == 'fits' ):
            s = scantable( self.__filein, average=False )
        elif ( self.__tabletype == 'asdm' ):
            # execute importasdm
            fileorg = self.__filein
            # default( importasdm )
            asdm = self.__filein.rstrip('/')
            importasdm( asdm=asdm, corr_mode='all', srt='all', time_sampling='all', ocorr_mode='ao', async=False )
            self.__filein = asdm + '.ms'
            s = scantable( self.__filein, average=False, getpt=True )
            s.set_freqframe( 'TOPO' )
            # remove ms2 file
            os.system( '\\rm -rf ' + self.__filein )
            os.system( '\\rm -rf ' + self.__filein + '.flagversions' )
            # set back to original value
            self.__filein = fileorg
        else:
            self.LogMessage( 'ERROR', Origin=origin, Msg='failed to import data.' )
            
        self.LogMessage( 'DEBUG', Origin=origin, Msg='s.nrow()=%s'%s.nrow())
        return s

    ###
    # Get appropriate spectrum
    ###
    #@dec_engines_logfile
    def getspectrum( self, reftime, scan, mode='linear', LogLevel=2, LogFile=False ):
        """
        getspectrum( self, ref, scan, mode='proxim' )
        
        reftime --- reference time
        scan    --- target scantable
        mode    --- indicate how to get spectrum
                    'proxim'  :  get spectrum from the row which is a proximate
                                 time with respect to ref. (default)
                    'immedi'  :  get spectrum from the row which is an immediate
                                 time with respect to ref.
                    'nearest' :  get spectrum from the row which have the nearest
                                 time with respect to ref. 
                    'linear'  :  get sprctrum by linearly interpolating spectra
                                 between the rows which corresponds to the
                                 proximate and the immediate with respect to ref.
        """
        origin = 'getspectrum()'
        #####self.setFileOut( self.fileOut )
        
        import datetime
        import numpy
        if ( scan.nrow() == 1 ):
            self.LogMessage('DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (0,scan.getscan(0)) )
            return (0, numpy.array( scan._getspectrum(0) ))
        else:
            if ( mode == 'nearest' ):
                #reftime=ref.get_time(0,True)
                dtmin=datetime.timedelta.max
                idx = -1
                for i in xrange(scan.nrow()):
                    t=scan.get_time(i,True)
                    dt=abs(t-reftime)
                    if ( dtmin > dt ):
                        dtmin=dt
                        idx=i
                if ( idx==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the nearest. return spectrum on the first row.' )
                    idx=0
                self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx,scan.getscan(idx)) )
                return (idx, numpy.array(scan._getspectrum(idx)))
            elif ( mode == 'proxim' ):
                #reftime=ref.get_time(0,True)
                dtmin=datetime.timedelta.max
                idx = -1
                for i in xrange(scan.nrow()):
                    t=scan.get_time(i,True)
                    if ( t <= reftime ):
                        dt=abs(t-reftime)
                        if ( dtmin > dt ):
                            dtmin=dt
                            idx=i
                if ( idx==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the proximate. try immediate.' )
                    dtmin=datetime.timedelta.max
                    for i in xrange( scan.nrow() ):
                        t=scan.get_time(i,True)
                        if ( t > reftime ):
                            dt = abs(t-reftime)
                            if ( dtmin > dt ):
                                dtmin=dt
                                idx = i
                    if ( idx == -1 ):
                        self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the immediate. return spectrum on the first row.' )
                        idx = 0
                self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx,scan.getscan(idx)) )
                return (idx, numpy.array(scan._getspectrum(idx)))
            elif ( mode == 'immedi' ):
                #reftime=ref.get_time(0,True)
                dtmin=datetime.timedelta.max
                idx = -1
                for i in xrange(scan.nrow()):
                    t=scan.get_time(i,True)
                    if ( t >= reftime ):
                        dt=abs(t-reftime)
                        if ( dtmin > dt ):
                            dtmin=dt
                            idx=i
                if ( idx==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the immediate. try proximate.' )
                    dtmin=datetime.timedelta.max
                    for i in xrange( scan.nrow() ):
                        t = scan.get_time(i,True)
                        if ( t < reftime ):
                            dt = abs(t-reftime)
                            if ( dtmin > dt ):
                                dtmin=dt
                                idx = i
                    if ( idx == -1 ):
                        self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the proximate. return spectrum on the first row.' )
                        idx=0
                self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx,scan.getscan(idx)) )
                return (idx, numpy.array(scan._getspectrum(idx)))
            elif ( mode == 'linear' ):
                #reftime=ref.get_time(0,True)
                dtmin1=datetime.timedelta.max
                dtmin2=datetime.timedelta.max
                idx1=-1
                idx2=-1
                for i in xrange(scan.nrow()):
                    t=scan.get_time(i,True)
                    if ( t <= reftime ):
                        # proximate
                        dt=abs(t-reftime)
                        if ( dtmin1 > dt ):
                            dtmin1=dt
                            idx1=i
                    else:
                        # immediate
                        dt=abs(t-reftime)
                        if ( dtmin2 > dt ):
                            dtmin2=dt
                            idx2=i
                if ( idx1==-1 and idx2 == -1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to interpolate. return spectrum on the first row.' )
                    return (0, numpy.array(scan._getspectrum(0)))
                elif ( idx1==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to interpolate. return the nearest.' )
                    self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx2,scan.getscan(idx2)) )
                    return (idx2, numpy.array(scan._getspectrum(idx2)))
                elif ( idx2==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to interpolate. return the nearest.' )
                    self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx1,scan.getscan(idx1)) )
                    return (idx1, numpy.array(scan._getspectrum(idx1)))
                else:
                    proximate=numpy.array(scan._getspectrum(idx1))
                    immediate=numpy.array(scan._getspectrum(idx2))
                    tprox=scan.get_time(idx1,True)
                    timme=scan.get_time(idx2,True)
                    dt1=reftime-tprox
                    dt2=timme-reftime
                    if ( dt1.days != 0 and dt2.days != 0 ):
                        if ( dt1 > dt2 ):
                            self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx2,scan.getscan(idx2)) )
                            return (idx2, immediate)
                        else:
                            self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx1,scan.getscan(idx1)) )
                            return (idx1, proximate)
                    elif ( dt1.days != 0 ):
                        self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx2,scan.getscan(idx2)) )
                        return (idx2, immediate)
                    elif ( dt2.days != 0 ):
                        self.LogMessage( 'DEBUG', Origin=origin, Msg='use row %s (scanno=%s)' % (idx1,scan.getscan(idx1)) )
                        return (idx1, proximate)
                    else:
                        self.LogMessage( 'DEBUG', Origin=origin, Msg='interpolate between %s and %s (scanno: %s, %s)' %(idx1,idx2,scan.getscan(idx1),scan.getscan(idx2)) )
                        dt1sec=dt1.seconds+1.0e-6*dt1.microseconds
                        dt2sec=dt2.seconds+1.0e-6*dt2.microseconds
                        interp=(immediate-proximate)/(dt1sec+dt2sec)*dt1sec+proximate
                        return ([idx1,idx2, dt1sec, dt2sec], interp)
        self.LogMessage( 'ERROR', Origin=origin, Msg='failed to get appropriate spectrum.' )
        return (-1, numpy.array( [], float ))

    ###
    # Get appropriate tcal
    ###
    #@dec_engines_logfile
    def gettcalval( self, reftime, index, tstamp, mode='linear', LogLevel=2, LogFile=False ):
        """
        gettcalval( self, ref, scan, mode='proxim' )
        
        reftime --- reference time
        index   --- tcal indexes
        tstamp  --- time stamp
        mode    --- indicate how to get tcal
                    'proxim'  :  get tcal from the row which is a proximate
                                 time with respect to ref. (default)
                    'immedi'  :  get tcal from the row which is an immediate
                                 time with respect to ref.
                    'nearest' :  get tcal from the row which have the nearest
                                 time with respect to ref. 
                    'linear'  :  get tcal by linearly interpolating spectra
                                 between the rows which corresponds to the
                                 proximate and the immediate with respect to ref.
        """
        origin = 'gettcalval()'
        #####self.setFileOut( self.fileOut )
        
        import numpy
        ep = self._me.epoch( 'utc', reftime.strftime('%Y/%m/%d/%H:%M:%S') )
        reftime = ep['m0']['value']
        self.LogMessage('DEBUG', Origin=origin, Msg='reftime = %s'%(reftime))
        if ( len(index) == 1 ):
            self.LogMessage('DEBUG', Origin=origin, Msg='use id %s' % (index[0]) )
            return [numpy.array(self.__tcal[index[0]]),index[0]]
        else:
            if ( mode == 'nearest' ):
                dtmin=10000000
                idx = -1
                for i in xrange(len(tstamp)):
                    t=tstamp[i]
                    dt=abs(t-reftime)
                    if ( dtmin > dt ):
                        dtmin=dt
                        idx=i
                if ( idx==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the nearest. return first tcal.' )
                    idx=0
                self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx]) )
                return [numpy.array(self.__tcal[index[idx]]),index[idx]]
            elif ( mode == 'proxim' ):
                dtmin=10000000
                idx = -1
                for i in xrange(len(tstamp)):
                    t=tstamp[i]
                    if ( t <= reftime ):
                        dt=abs(t-reftime)
                        if ( dtmin > dt ):
                            dtmin=dt
                            idx=i
                if ( idx==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the proximate. try immediate.' )
                    dtmin=10000000
                    for i in xrange(len(tstamp)):
                        t=tstamp[i]
                        if ( t > reftime ):
                            dt = abs(t-reftime)
                            if ( dtmin > dt ):
                                dtmin=dt
                                idx = i
                    if ( idx == -1 ):
                        self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the immediate. return first tcal.' )
                        idx = 0
                self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx]) )
                return [numpy.array(self.__tcal[index[idx]]),index[idx]]
            elif ( mode == 'immedi' ):
                dtmin=10000000
                idx = -1
                for i in xrange(len(tstamp)):
                    t=tstamp[i]
                    if ( t >= reftime ):
                        dt=abs(t-reftime)
                        if ( dtmin > dt ):
                            dtmin=dt
                            idx=i
                if ( idx==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the immediate. try proximate.' )
                    dtmin=10000000
                    for i in xrange(len(tstamp)):
                        t = tstamp[i]
                        if ( t < reftime ):
                            dt = abs(t-reftime)
                            if ( dtmin > dt ):
                                dtmin=dt
                                idx = i
                    if ( idx == -1 ):
                        self.LogMessage( 'WARNING', Origin=origin, Msg='failed to find the proximate. return first tcal.' )
                        idx=0
                self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx]) )
                return [numpy.array(self.__tcal[index[idx]]),index[idx]]
            elif ( mode == 'linear' ):
                dtmin1=10000000
                dtmin2=10000000
                idx1=-1
                idx2=-1
                for i in xrange(len(tstamp)):
                    t=tstamp[i]
                    if ( t <= reftime ):
                        # proximate
                        dt=abs(t-reftime)
                        if ( dtmin1 > dt ):
                            dtmin1=dt
                            idx1=i
                    else:
                        # immediate
                        dt=abs(t-reftime)
                        if ( dtmin2 > dt ):
                            dtmin2=dt
                            idx2=i
                if ( idx1==-1 and idx2 == -1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to interpolate. return first tcal.' )
                    return [numpy.array(self.__tcal[index[0]]),index[0]]
                elif ( idx1==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to interpolate. return the nearest.' )
                    self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx2]) )
                    return [numpy.array(self.__tcal[index[idx2]]),index[idx2]]
                elif ( idx2==-1 ):
                    self.LogMessage( 'WARNING', Origin=origin, Msg='failed to interpolate. return the nearest.' )
                    self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx1]) )
                    return [numpy.array(self.__tcal[index[idx1]]),index[idx1]]
                else:
                    proximate=self.__tcal[index[idx1]]
                    immediate=self.__tcal[index[idx2]]
                    tprox=tstamp[idx1]
                    timme=tstamp[idx2]
                    dt1=reftime-tprox
                    dt2=timme-reftime
                    if ( abs( dt1 ) > 1 and abs( dt2 ) > 1 ):
                        if ( dt1 > dt2 ):
                            self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx2]) )
                            return [numpy.array(immediate),index[idx2]]
                        else:
                            self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx1]) )
                            return [numpy.array(proximate),index[idx1]]
                    elif ( abs( dt1 ) > 1 ):
                        self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx2]) )
                        return [numpy.array(immediate),index[idx2]]
                    elif ( abs( dt2 ) > 1 ):
                        self.LogMessage( 'DEBUG', Origin=origin, Msg='use id %s' % (index[idx1]) )
                        return [numpy.array(proximate),index[idx1]]
                    else:
                        self.LogMessage( 'DEBUG', Origin=origin, Msg='interpolate between %s and %s' %(index[idx1],index[idx2]) )
                        immediate = numpy.array( immediate )
                        proximate = numpy.array( proximate )
                        interp=(immediate-proximate)/(dt1+dt2)*dt1+proximate
                        return [interp,-1]
        self.LogMessage( 'ERROR', Origin=origin, Msg='failed to get appropriate tcal.' )
        return [numpy.array( [], float ),99999999]

    @dec_engines_logfile
    def average( self, weight='tint', LogLevel=2, LogFile=False ):
        """
        Average data using asap tool.
        If caltype is 'otf', this does nothing.
        
        weight --- weighting scheme
                   'none'   : no weight
                   'var'    : 1/var(spec) weighted
                   'tsys'   : 1/Tsys**2 weightd
                   'tint'   : integration time weighted
                   'tintsys': Tint/Tsys**2 weighted
                   'median' : median averaging
        """
        origin = 'average()'
        #####self.setFileOut( self.fileOut )
        
        from asap import selector, average_time
        if ( self.__caltype == 'otf' ):
            self.LogMessage( 'INFO', Origin=origin, Msg='average() is disabled for OTF data.' )
        else:
            self.LogMessage( 'INFO', Origin=origin, Msg='averaging dumped data for each scan.' )
            atsys = []
            sumtsys = []
            rowids = []
            scannos = self.__tableout.getscannos()
            ifnos = self.__tableout.getifnos()
            beamnos = self.__tableout.getbeamnos()
            polnos = self.__tableout.getpolnos()
            stmp = self.__tableout.copy()
            sel = selector()
            for irow in xrange(stmp.nrow()):
                scanno = stmp.getscan(irow)
                ifno = stmp.getif(irow)
                beamno = stmp.getbeam(irow)
                polno = stmp.getpol(irow)
                cycleno = stmp.getcycle(irow)
                if ( rowids.count( [scanno, ifno, beamno, polno] ) == 0 ):
                    rowids.append( [scanno, ifno, beamno, polno] )
                    sumtsys.append( [0, 0.0] )
                idx = rowids.index( [scanno, ifno, beamno, polno] )
                sumtsys[idx][0] = sumtsys[idx][0] + 1
                tsysid=self.__tsysidlist[irow]
                if type(tsysid)!=list:
                    sumtsys[idx][1] = sumtsys[idx][1] + self.__tsyslist[tsysid]
                else:
                    tsys0=self.__tsyslist[tsysid[0]]
                    tsys1=self.__tsyslist[tsysid[1]]
                    x0=tsysid[2]
                    x1=tsysid[3]
                    tsys = (tsys1-tsys0)/(x0+x1)*x0+tsys0
                    sumtsys[idx][1] = sumtsys[idx][1] + tsys
            for idx in xrange( len(sumtsys) ):
                atsys.append( sumtsys[idx][1] / float( sumtsys[idx][0] ) )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='rowids=%s' % rowids )
            self.LogMessage( 'DEBUG', Origin=origin, Msg='atsys=%s' %atsys )
            
            averaged_scan=average_time( self.__tableout, scanav=True, weight=weight )
            self.__tableout=averaged_scan
            self.__tsyslist = []
            for i in xrange( averaged_scan.nrow() ):
                scanno = averaged_scan.getscan(i)
                ifno = averaged_scan.getif(i)
                beamno = averaged_scan.getbeam(i)
                polno = averaged_scan.getpol(i)
                self.LogMessage('DEBUG',Origin=origin, Msg='i=%s: scanno=%s, ifno=%s, beamno=%s, polno=%s'%(i,scanno,ifno,beamno,polno))
                for j in xrange( len(rowids) ):
                    if ( scanno == rowids[j][0] and ifno == rowids[j][1]
                         and beamno == rowids[j][2] and polno == rowids[j][3] ):
                        self.__tsyslist.append( atsys[j] )
            self.__tsysidlist = range(len(self.__tsyslist))
            self.LogMessage( 'DEBUG', Origin=origin, Msg='self.__tsyslist=%s' %self.__tsyslist )
            

    def _shift( self, arr, ishift, fshift ):
        """
        Shift array rightward.

        If negative value is given, shift leftward.
        """
        import numpy
        nchan = len(arr)
        sarr = numpy.concatenate([arr[-ishift:],arr[:-ishift]])
        oarr = sarr.copy()
##         if type(arr[0]) == bool:
##             if fshift < 0:
##                 for ich in xrange( nchan-1 ):
##                     sarr[ich] = oarr[ich+1] and oarr[ich]
##                 sarr[nchan-1] = oarr[0] and oarr[nchan-1]
##             elif fshift > 0:
##                 sarr[0] = oarr[nchan-1] and oarr[0]
##                 for ich in xrange( 1, nchan ):
##                     sarr[ich] = oarr[ich-1] and oarr[ich]
##         else:
##             if fshift < 0:
##                 for ich in xrange( nchan-1 ):
##                     sarr[ich] = fshift * oarr[ich+1] + ( 1.0 - fshift ) * oarr[ich]
##                 sarr[nchan-1] = fshift * oarr[0] + ( 1.0 - fshift ) * oarr[nchan-1]
##             elif fshift > 0:
##                 sarr[0] = fshift * oarr[nchan-1] + ( 1.0 - fshift ) * oarr[0]
##                 for ich in xrange( 1, nchan ):
##                     sarr[ich] = fshift * oarr[ich-1] + ( 1.0 - fshift ) * oarr[ich]
        return sarr


    def _detectWVRScan( self ):
        """
        """
        origin = '_detectWVRScan()'
        
        import numpy
        WVRScan=[]
        widx=[]
        ifnos = numpy.array( self.__tablein.getifnos() )
        for iIF in xrange(len(ifnos)):
            # 2010/05/19 Takeshi Nakazato
            # skip if nchan is 4
            try:
                if self.__tablein.nchan(iIF) == 4:
                    self.LogMessage('INFO',Origin=origin, Msg='Skip IF=%s since it is WVR data'%iIF)
                    WVRScan.append(ifnos[iIF])
                    widx.append(iIF)
            except RuntimeError, e:
                self.LogMessage('INFO',Origin=origin, Msg='Skip IF=%s because of no data'%iIF)
                continue
        self.ifnos = numpy.delete( ifnos, widx )
        # set self.WVRScan anyway
        self.WVRScan = WVRScan

        return

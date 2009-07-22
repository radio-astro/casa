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

from asap import *
from tasks import *
import os
import re
import casac
import numpy
import xml.dom.minidom as DOM
import SDTool as SDT
import datetime

class SDCalibration:
    """
    Class for single dish calibration
    """

    ###
    # initialization
    ###
    def __init__( self, filename, caltype='none', tsys=0.0, tcal=0.0, tau=0.0,
                  LogLevel=1, LogFile='' ):
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
        # avoid memory occupation
        #rcParams['scantable.storage']='disk'
        # logging
        self.__loglevel = LogLevel
        self.__logfile = LogFile
        # input
        filename = os.path.expandvars(filename)
        filename = os.path.expanduser(filename)
        self.__filein = filename
        self.__tabletype = self.gettabletype()
        # NOTE: self.__tablein is always scantable
        self.__tablein = self.importdata()   
        # output
        self.__tableout = None
        self.__calib = False
        # calibration type is set automatically if 'none' is given
        if ( caltype != 'none' ):
            self.__caltype = caltype
        else:
            self.__caltype = self.getcaltype()
        # calibration parameters
        if ( tsys > 0.0 ):
            self.__tsys = tsys
        else:
            self.__tsys = self.gettsys()
        if ( tcal > 0.0 ):
            self.__tcal = tcal
        else:
            self.__tcal = self.gettcal()
        if ( tau > 0.0 ):
            self.__tau = tau
        else:
            self.__tau = self.gettau()


    ###
    # finalization
    ###
    def __del__( self ):
        """
        Destructor
        """
        if self.__tablein is not None:
            del self.__tablein
        if self.__tableout is not None:
            del self.__tableout
        
    ###
    # print data summary
    ###
    def summary( self ):
        """
        Print input data summary
        """
        print >> self.__logfile, 'Data Summary: '
        print >> self.__logfile, self.__tablein
        
    ###
    # calibration
    ###
    def calibrate( self ):
        """
        Do calibration
        NOTE: frequency switching is not implemented yet
        """
        # return if already calibrated
        if ( self.__calib ):
            self.log( 'INFO', msg='Data already calibrated. Nothing to do.' )
            return

        if ( self.__caltype == 'ps' ):
            # position switch data calibration
            self.pscal()
        elif ( self.__caltype == 'fs' ):
            # frequency switch data calibration
            self.fscal()
        elif ( self.__caltype == 'nod' ):
            # nod data
            self.nodcal()
        elif ( self.__caltype == 'otf' ):
            # OTF data
            self.otfcal()
        elif ( self.__caltype == 'wob' ):
            # wobbler or nutator switching
            self.wobcal()
        else:
            self.log('INFO', msg='Not implemented yet.')

        # self.__calib=True if calibrated
        if ( self.__tableout is not None ):
            self.__calib = True
        
    ###
    # save data
    ###
    def save( self, outfile=None ):
        """
        Save calibrated result as ASAP Table
        """
        if ( not self.__calib ):
            self.log('INFO', msg='Data is not calibrated yet. Do calibrate first.')
            return

        if ( outfile == None ):
            outfile = self.__filein.rstrip('/') + '_cal'
        self.__tableout.save( outfile, format='ASAP', overwrite=True )
        # spectral unit should be K after the calibration
        tbtool=casac.homefinder.find_home_by_name('tableHome')
        tb=tbtool.create()
        tb.open( outfile, nomodify=False )
        tb.putkeyword( 'FluxUnit', 'K' )
        antname=tb.getkeyword( 'AntennaName' )
        tb.flush()
        tb.close()
        # remove _pson, _wobon, _fshi, _fslo, ... from SRCNAME
        if ( antname == 'APEX-12m' ):
            calstr=''
            if ( self.__caltype == 'ps' or self.__caltype == 'otf' ):
                calstr = '_pson'
            elif ( self.__caltype == 'wob' ):
                calstr = '_wobon'
            self.log( 'DEBUG', msg='Remove %s from SRCNAME' %(calstr) )
            tb.open( outfile, nomodify=False )
            srcname=tb.getcol('SRCNAME')
            for i in range(len(srcname)):
                srcname[i] = srcname[i].rstrip( calstr )
            tb.putcol( 'SRCNAME', srcname )
            tb.flush()
            tb.close()

    ###
    # check if data was calibrated or not
    ###
    def isCalibrated( self ):
        return self.__calib

    ###
    # position switch calibration
    ###
    def pscal( self ):
        """
        Calibrate Position Switch data.
        """
        tbtool = casac.homefinder.find_home_by_name('tableHome')
        self.log('INFO', msg='Calibrating position switch data')
        statetable = self.__filein + '/STATE'
        antname = self.__tablein.get_antennaname()
        # GBT data
        if ( antname == 'GBT' ):
            if ( self.__tablein.get_scan('*_ps*') is not None ):
                # GBT position switch data filled by PKSMS2reader
                self.log('INFO', msg='GBT data: using ASAP Tools (calps)')
                scannos = list( self.__tablein.getscannos() )
                tcal = self.__tcal
                if ( type(tcal) == list ):
                    tcal = tcal[0][0]
                tsys = self.__tsys
                if ( type(tsys) == list ):
                    #if ( type(tsys[0]) == list ):
                    #    tsys = tsys[0][0]
                    #else:
                    #    tsys = tsys[0]
                    tsys = 0.0
                tau = self.__tau
                if ( type(tau) == list ):
                    tau = tau[0][0]
                self.__tableout = calps( self.__tablein, scannos, tsysval=tsys, tauval=tau, tcalval=tcal )
            elif os.path.exists( statetable ):
                # GBT position switch data (MS2 raw data)
                self.log('INFO', msg='GBT data: using ASAP Tools (calps)')
                self.log('INFO', msg='Rename source name')
                # rename source name to fit calps()
                t = tbtool.create()
                t.open( statetable )
                obsmodecol = t.getcol( 'OBS_MODE' )
                calnumcol = t.getcol( 'CAL' )
                t.close()
                t.open( self.__filein )
                stateidcol = t.getcol( 'STATE_ID' )
                t.close()
                tmptable = self.__filein.rstrip('/') + '_TMP'
                self.__tablein.save( tmptable, format='ASAP' )
                t.open( tmptable )
                srcnamecol = t.getcol( 'SRCNAME' )
                if ( len( srcnamecol ) != 2 * len( stateidcol ) ):
                    SDT.Logmessage('ERROR', msg='Mehod failed.' )
                    t.close()
                    del t
                    return
                for irow in range( len(stateidcol) ):
                    obsmode = obsmodecol[stateidcol[irow]]
                    modes = obsmode.split(':')
                    calnum = calnumcol[stateidcol[irow]]
                    str = ''
                    if ( modes[0] != 'OffOn' ):
                        SDT.Logmessage('ERROR', msg='Input data is not position switch mode. Set appropriate caltype.' )
                        t.close()
                        del t
                        return
                    if ( modes[1] == 'PSWITCHOFF' ):
                        str = str + '_psr'
                    elif ( modes[1] == 'PSWITCHON' ):
                        str = str + '_ps'
                    if ( calnum != 0 ):
                        str = str + '_calon'
                    srcnamecol[2*irow] += str
                    srcnamecol[2*irow+1] += str
                t.putcol( columnname='SRCNAME', value=srcnamecol )
                t.close()
                del t
                del self.__tablein
                self.__tablein = scantable( tmptable, average=False )
                os.system( 'rm -rf %s' % (tmptable) )
                scannos = list( self.__tablein.getscannos() )
                # useA SAP tool
                tcal = self.__tcal
                if ( type(tcal) == list ):
                    tcal = tcal[0][0]
                tsys = self.__tsys
                if ( type(tsys) == list ):
                    #if ( type(tsys[0]) == list ):
                    #    tsys = tsys[0][0]
                    #else:
                    #    tsys = tsys[0]
                    tsys = 0.0
                tau = self.__tau
                if ( type(tau) == list ):
                    tau = tau[0][0]
                self.__tableout = calps( self.__tablein, scannos, tsysval=tsys, tauval=tau, tcalval=tcal )
            else:
                # other
                self.log('INFO', msg='Not implemented yet.')
        # APEX data
        elif ( antname == 'APEX-12m' ):
            self.log( 'INFO', msg='APEX data' )
            # identify scan type (single-point or raster or map)
            tb = tbtool.create()
            tb.open( self.__filein )
            otype = 'none'
            obstype = tb.getkeyword( 'Obstype' )
            if ( obstype.find( 'SINGLE' ) != -1 ):
                otype = 'single'
            elif ( obstype.find( 'MAP' ) != -1 ):
                otype = 'map'
            srcnamecol = tb.getcol( 'SRCNAME' )
            cyclecol = tb.getcol( 'CYCLENO' )
            tcalidcol = tb.getcol( 'TCAL_ID' )
            tb.close()
            # get tcal id
            tb.open( self.__filein )
            tcalid = []
            for i in range( srcnamecol.shape[0] ):
                if ( srcnamecol[i].find( '_pson' ) != -1 ):
                    tcalid.append( tcalidcol[i] )
            self.log( 'DEBUG', msg='tcalid = %s' % tcalid )
            tcalid = numpy.array( tcalid )
            tb.close()
            del tb
            # get hot and sky
            sel = selector()
            sel.set_name( '*_sky' )
            #s = self.__tablein
            self.__tablein.set_unit( 'channel' ) # average_time works for channel only
            self.__tablein.set_selection( sel )
            asky = asapmath.average_time( self.__tablein, scanav=True )
            nrsky = asky.nrow()
            self.log( 'DEBUG', 'number of sky scan = %s' % nrsky )
            self.__tablein.set_selection()
            sel.reset()
            sel.set_name( '*_hot' )
            self.__tablein.set_selection( sel )
            ahot = asapmath.average_time( self.__tablein, scanav=True )
            nrhot = ahot.nrow()
            self.log( 'DEBUG', 'number of hot scan = %s' % nrhot )
            # get on and off
            sel.reset()
            self.__tablein.set_selection()
            sel.set_name( '*_psoff' )
            self.__tablein.set_selection( sel )
            aoff = asapmath.average_time( self.__tablein, scanav=True, weight = 'tint' )
            nroff = aoff.nrow()
            self.log( 'DEBUG', 'number of off scan = %s' % nroff )
            self.__tablein.set_selection()
            sel.reset()
            sel.set_name( '*_pson' )
            self.__tablein.set_selection( sel )
            self.__tableout = self.__tablein.copy()
            nron = self.__tableout.nrow()
            self.log( 'DEBUG', 'number of on scan = %s' % nron )
            self.__tablein.set_selection()
            sel.reset()
            #del s
            #self.__tablein = None
            # loop on IF, BEAM, POL, SCAN
            for i in range(nron):
                sref=self.__tableout.get_time(i,True)
                ii=self.__tableout.getif(i)
                ib=self.__tableout.getbeam(i)
                ip=self.__tableout.getpol(i)
                ic=self.__tableout.getscan(i)
                sel.reset()
                sel.set_ifs(ii)
                sel.set_beams(ib)
                sel.set_polarisations(ip)
                aoff.set_selection(sel)
                asky.set_selection(sel)
                ahot.set_selection(sel)
                sel.set_scans(ic)
                self.log('DEBUG',msg='search sky scan for row %s (scanno=%s)' % (i,ic) )
                spsky=self.getspectrum( sref, asky )
                self.log('DEBUG',msg='search hot scan for row %s (scanno=%s)' % (i,ic) )
                sphot=self.getspectrum( sref, ahot )
                spon=numpy.array(self.__tableout._getspectrum(i))
                self.log('DEBUG',msg='search off scan for row %s (scanno=%s)' % (i,ic) )
                spoff=self.getspectrum( sref, aoff )
                # Classical chopper-wheel method (Ulich & Haas 1976)
                #ta=(spon-spoff)/(sphot-spsky)*self.__tcal[tcalid[i]]
                # APEX calibration
                ta=((spon-spoff)/spoff)*(spsky/(sphot-spsky))*self.__tcal[tcalid[i]]
                self.__tableout._setspectrum( ta, i )
                aoff.set_selection()
                asky.set_selection()
                ahot.set_selection()
            # end of the loop on IF, BEAM, POL, SCAN

    ###
    # frequency switch calibration
    ###
    def fscal( self ):
        """
        Calibrate Frequency Switch data.
        """
        self.log('INFO', msg='Calibrating frequency switch data')
        statetable = self.__filein + '/STATE'
        if ( self.__tablein.get_scan('*_fs*') is not None ):
            # GBT position switch data filled by PKSMS2reader
            self.log('INFO', msg='Using ASAP Tools: calfs')
            scannos = list( self.__tablein.getscannos() )
            tcal = self.__tcal
            if ( type(tcal) == list ):
                tcal = tcal[0][0]
            tsys = self.__tsys
            if ( type(tsys) == list ):
                #if ( type(tsys[0]) == list ):
                #    tsys = tsys[0][0]
                #else:
                #    tsys = tsys[0]
                tsys = 0.0
            tau = self.__tau
            if ( type(tau) == list ):
                tau = tau[0][0]
            self.__tableout = calfs( self.__tablein, scannos, tsysval=tsys, tauval=tau, tcalval=tcal )
        else:
            # other
            self.log('INFO', msg='Not implemented yet.')

    ###
    # calibrate nod data
    ###
    def nodcal( self ):
        """
        Calibrate nod data.
        """
        self.log('INFO', msg='Calibrating nod data')

    ###
    # calibrate OTF data
    ###
    def otfcal( self ):
        """
        Calibrate OTF data.
        """
        #tbtool = casac.homefinder.find_home_by_name('tableHome')
        self.log('INFO', msg='Calibrating OTF data')
        statetable = self.__filein + '/STATE'
        antname = self.__tablein.get_antennaname()
        # APEX data
        if ( antname == 'APEX-12m' ):
            self.pscal()
        #del tbtool
        
    ###
    # calibrate wobbler or nutator switching data
    ###
    def wobcal( self ):
        """
        Calibrate wobbler or nutator switching data.
        """
        self.log('INFO', msg='Calibrating wobbler or nutator switching data')
        
    ###
    # examine table type
    ###
    def gettabletype( self ):
        """
        Get table type
        """
        type = 'unknown'
        
        if os.path.isdir( self.__filein ):
            if os.path.exists( self.__filein+'/table.f1' ):
                type = 'ms2'
            elif os.path.exists( self.__filein+'/table.info' ):
                type = 'asap'
            elif os.path.exists( self.__filein+'/ASDM.xml' ):
                type = 'asdm'
        elif re.search( '\.fits$', self.__filein.lower() ) is not None:
            type = 'fits'

        self.log( 'INFO', 'Table type is %s' % type )
        return type
    
    ###
    # examine calibration type
    ###
    def getcaltype( self ):
        """
        Examine calibration type from input table
        """
        tbtool = casac.homefinder.find_home_by_name('tableHome')
        type = 'unknown'

        # the case if self.__tablein has been created by PKSReader
        if ( self.__tablein.get_antennaname() == 'GBT' ):
            if ( self.__tablein.get_scan('*_ps*') is not None ):
                type = 'ps'
            elif ( self.__tablein.get_scan('*_fs*') is not None ):
                type = 'fs'
            elif ( self.__tablein.get_scan('*_nod*') is not None ):
                type = 'nod'
        elif self.__tabletype == 'ms2':
            statetable = self.__filein+'/STATE'
            if os.path.exists( statetable ):
                t = tbtool.create()
                t.open( statetable )
                obsmode = t.getcol( 'OBS_MODE' )[0]               
                if ( obsmode.find( 'PSWITCH' ) != -1 ):
                    type = 'ps'
                elif ( obsmode.find( 'FSWITCH' ) != -1 ):
                    type = 'fs'
                del t
        # APEX data (ASAP table)
        elif ( self.__tablein.get_antennaname() == 'APEX-12m' ):
            t = tbtool.create()
            t.open( self.__filein )
            obstype = t.getkeyword( 'Obstype' )
            if ( obstype.find( 'FSW' ) != -1 ):
                type = 'fs'
            elif ( obstype.find( 'OTF' ) != -1 ):
                type = 'otf'
            elif ( obstype.find( 'WOBSW' ) != -1 ):
                type = 'wob'
            elif ( obstype.find( 'TOTP' ) != -1 ):
                type = 'ps'
            del t
        # other ASAP table
        elif self.__tabletype == 'asap':
            t = tbtool.create()
            t.open( self.__filein )
            obstype = t.getkeyword( 'Obstype' )
            if ( obstype.find( 'PSWITCH' ) != -1 ):
                type = 'ps'
            elif ( obstype.find( 'FSWITCH' ) != -1 ):
                type = 'fs'
            del t
        # ASDM
        elif self.__tabletype == 'asdm':
            # NOTE: I don't know how I can specify calibration type
            scantable = self.__filein+('/Scan.xml')
            dom3 = DOM.parse( scantable )
            rootnode = dom3.getElementsByTagName( 'scanIntent' )[0]
            #intelm = rootnode.getElementByTagName( 'scanIntent' )[0]
            intelm = rootnode
            intstr = intelm.childNodes[0].data.encode( 'UTF-8' )
            if ( intstr.find( 'PSWITCH' ) != -1 ):
                type = 'ps'
            elif ( intstr.find( 'FSWITCH' ) != -1 ):
                type = 'fs'
            dom3.unlink()
            
        self.log( 'INFO', 'Calibration type is %s' % type )
        #del tbtool
        return type
        
    ###
    # get system temperature
    ###
    def gettsys( self ):
        """
        Get system temperature
        """
        #tbtool = casac.homefinder.find_home_by_name('tableHome')
        val = 0.0
        
        # from memory
        tsysval = []
        for i in range( self.__tablein.nrow() ):
            tsysval.append( self.__tablein._gettsys( i ) )
        if tsysval is not None:
            val = tsysval
        #del tbtool
        return val

    ###
    # get calibration temperature
    ###
    def gettcal( self ):
        """
        Get calibration temperature
        """
        tbtool = casac.homefinder.find_home_by_name('tableHome')
        val = 0.0

        # from memory
        # there is no way to get Tcal from scantable
        
        # from disk
        if ( self.__tabletype == 'ms2' ):
            syscaltable = self.__filein+'/SYSCAL'
            t = tbtool.create()
            t.open( syscaltable )
            # TODO: assign Tcal for each observation row
            #       Tcal are defined for each time and each spectral window
            tcalcol = t.getcol( 'TCAL' )
            if tcalcol is not None:
                val = tcalcol
            del t
        elif ( self.__tabletype == 'asap' ):
            tcaltable = self.__filein+'/TCAL'
            t = tbtool.create()
            t.open( tcaltable )
            # TODO: assign Tcal for each observation row
            #       Tcal are defined for each time
            #tcalcol = t.getcol( 'TCAL' )
            tcalcol = []
            for i in range( t.nrows() ):
                tcalcol.append( t.getcell( 'TCAL', i ) )
            #self.log( 'DEBUG', msg='tcalcol = %s' % tcalcol )
            self.log( 'DEBUG', msg='len(tcalcol) = %s' % len(tcalcol) )
            if ( len( tcalcol ) != 0 ):
                val = tcalcol
            t.close()
            del t
        elif ( self.__tabletype == 'asdm' ):
            syscaltable = self.__filein+'/SysCal.xml'
            val = []
            if ( os.path.exists( syscaltable ) ):
                dom3 = DOM.parse( syscaltable )
                rootnode = dom3.documentElement
                tcalelm = rootnode.getElementsByTagName( 'tcal' )
                for i in range( len(tcalelm) ):
                    tcalstr = tcalelm[i].childNodes[0].data.encode( 'UTF-8' )
                    strs = tcalstr.split()
                    dim = int(strs.pop( 0 ))
                    if dim == 1:
                        axislen = int(strs.pop( 0 ))
                        for j in range( axislen ):
                            val.append( float(strs[j]) )
                    else:
                        self.log( 'INFO', msg='gettau: tcal should be 1-dimensional!' )
                        val = 0.0
                dom3.unlink()
            else:
                self.log( 'INFO', msg=syscaltable+' does not exist.' )
                val = 0.0
        #elif ( self.__tabletype == 'fits' ):
            # I don't know how I can retrieve Tsys from header

        #del tbtool
        return val

    ###
    # get atmospheric optical depth
    ###
    def gettau( self ):
        """
        Get atmospheric optical depth
        """
        val = 0.0

        # from memory
        # there is no way to get tau from scantable

        # from disk
        #if ( self.__tabletype == 'ms2' ):
            # Idon't know how I can retrieve tau from the table
        #elif ( self.__tabletype == 'asap' ):
            # Idon't know how I can retrieve tau from the table
        if ( self.__tabletype == 'asdm' ):
            val = []
            calatmtable = self.__filein+'/CalAtmosphere.xml'
            if ( os.path.exists( calatmtable ) ):
                dom3 = DOM.parse( calatmtable )
                rootnode = dom3.documentElement
                tauelm = rootnode.getElementsByTagName( 'tauSpectrum' )
                for i in range( len(tauelm) ):
                    taustr = tauelm[i].childNodes[0].data.encode( 'UTF-8' )
                    strs = taustr.split()
                    dim = int(strs.pop( 0 ))
                    if dim == 2:
                        xlen = int(strs.pop( 0 ))
                        ylen = int(strs.pop( 0 ))
                        # NOTE: I'm not sure how data are sorted
                        for j in range( xlen ):
                            pval = []
                            for k in range( ylen ):
                                pval.append( float(strs[k*ylen+j]) )
                            val.append( pval() )
                    else:
                        self.log( 'INFO', msg='gettau: tauSpectrum should be 2-dimensional!' )
                        val = 0.0
                dom3.unlink()
            else:
                self.log( 'INFO', msg=calatmtable+' does not exist.' )
                val = 0.0
        #elif ( self.__tabletype == 'fits' ):
            # I don't know how I can retrieve Tsys from header

        # NOTE: return value is scalar at the moment
        self.log( 'INFO', msg='tau = %s' %val )
        return val

    ###
    # logging
    ###
    def log( self, head, msg='' ):
        """
        logging
        """
        SDT.LogMessage( head, self.__loglevel, self.__logfile, Msg=msg )


    ###
    # import data
    ###
    def importdata( self ):
        """
        Import data
        """
        s = None
        if ( self.__tabletype == 'ms2' ):
            s = scantable( self.__filein, average=False )
        elif ( self.__tabletype == 'asap' ):
            s = scantable( self.__filein, average=False )
        elif ( self.__tabletype == 'fits' ):
            s = scantable( self.__filein, average=False )
        elif ( self.__tabletype == 'asdm' ):
            # execute importasdm
            fileorg = self.__filein
            #default( importasdm )
            asdm = self.__filein.rstrip('/')
            importasdm( asdm=asdm, corr_mode='all', srt='all', time_sampling='all', ocorr_mode='ao', async=False )
            self.__filein = asdm + '.ms'
            s = scantable( self.__filein, average=False )
            s.set_freqframe( 'TOPO' )
            # remove ms2 file
            os.system( '\\rm -rf ' + self.__filein )
            os.system( '\\rm -rf ' + self.__filein + '.flagversions' )
            # set back to original value
            self.__filein = fileorg
        else:
            self.log( 'ERROR', msg='failed to import data.' )
        
        return s

    ###
    # Get appropriate spectrum
    ###
    #def getspectrum( self, ref, scan, mode='proxim' ):
    def getspectrum( self, reftime, scan, mode='proxim' ):
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
        #self.log('DEBUG', msg='scanno list:')
        #for i in range(scan.nrow()):
        #    self.log('DEBUG', msg='   row %s: scanno=%s' %(i,scan.getscan(i)))
        if ( scan.nrow() == 1 ):
            self.log('DEBUG', msg='use row %s (scanno=%s)' % (0,scan.getscan(0)) )
            return numpy.array( scan._getspectrum(0) )
        else:
            if ( mode == 'nearest' ):
                #reftime=ref.get_time(0,True)
                dtmin=datetime.timedelta.max
                idx = -1
                for i in range(scan.nrow()):
                    t=scan.get_time(i,True)
                    dt=abs(t-reftime)
                    if ( dtmin > dt ):
                        dtmin=dt
                        idx=i
                if ( idx==-1 ):
                    self.log( 'WARNING', msg='failed to find the nearest. return spectrum on the first row.' )
                    idx=0
                self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx,scan.getscan(idx)) )
                return numpy.array(scan._getspectrum(idx))
            elif ( mode == 'proxim' ):
                #reftime=ref.get_time(0,True)
                dtmin=datetime.timedelta.max
                idx = -1
                for i in range(scan.nrow()):
                    t=scan.get_time(i,True)
                    if ( t <= reftime ):
                        dt=abs(t-reftime)
                        if ( dtmin > dt ):
                            dtmin=dt
                            idx=i
                if ( idx==-1 ):
                    self.log( 'WARNING', msg='failed to find the proximate. try immediate.' )
                    dtmin=datetime.timedelta.max
                    for i in range( scan.nrow() ):
                        t=scan.get_time(i,True)
                        if ( t > reftime ):
                            dt = abs(t-reftime)
                            if ( dtmin > dt ):
                                dtmin=dt
                                idx = i
                    if ( idx == -1 ):
                        self.log( 'WARNING', msg='failed to find the immediate. return spectrum on the first row.' )
                        idx = 0
                self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx,scan.getscan(idx)) )
                return numpy.array(scan._getspectrum(idx))
            elif ( mode == 'immedi' ):
                #reftime=ref.get_time(0,True)
                dtmin=datetime.timedelta.max
                idx = -1
                for i in range(scan.nrow()):
                    t=scan.get_time(i,True)
                    if ( t >= reftime ):
                        dt=abs(t-reftime)
                        if ( dtmin > dt ):
                            dtmin=dt
                            idx=i
                if ( idx==-1 ):
                    self.log( 'WARNING', msg='failed to find the immediate. try proximate.' )
                    dtmin=datetime.timedelta.max
                    for i in range( scan.nrow() ):
                        t = scan.get_time(i,True)
                        if ( t < reftime ):
                            dt = abs(t-reftime)
                            if ( dtmin > dt ):
                                dtmin=dt
                                idx = i
                    if ( idx == -1 ):
                        self.log( 'WARNING', msg='failed to find the proximate. return spectrum on the first row.' )
                        idx=0
                self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx,scan.getscan(idx)) )
                return numpy.array(scan._getspectrum(idx))
            elif ( mode == 'linear' ):
                #reftime=ref.get_time(0,True)
                dtmin1=datetime.timedelta.max
                dtmin2=datetime.timedelta.max
                idx1=-1
                idx2=-1
                for i in range(scan.nrow()):
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
                    self.log( 'WARNING', msg='failed to interpolate. return spectrum on the first row.' )
                    return numpy.array(scan._getspectrum(0))
                elif ( idx1==-1 ):
                    self.log( 'WARNING', msg='failed to interpolate. return the nearest.' )
                    self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx2,scan.getscan(idx2)) )
                    return numpy.array(scan._getspectrum(idx2))
                elif ( idx2==-1 ):
                    self.log( 'WARNING', msg='failed to interpolate. return the nearest.' )
                    self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx1,scan.getscan(idx1)) )
                    return numpy.array(scan._getspectrum(idx1))
                else:
                    proximate=numpy.array(scan._getspectrum(idx1))
                    immediate=numpy.array(scan._getspectrum(idx2))
                    tprox=scan.get_time(idx1,True)
                    timme=scan.get_time(idx2,True)
                    dt1=reftime-tprox
                    dt2=timme-reftime
                    if ( dt1.days != 0 and dt2.days != 0 ):
                        if ( dt1 > dt2 ):
                            self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx2,scan.getscan(idx2)) )
                            return immediate
                        else:
                            self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx1,scan.getscan(idx1)) )
                            return proximate
                    elif ( dt1.days != 0 ):
                        self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx2,scan.getscan(idx2)) )
                        return immediate
                    elif ( dt2.days != 0 ):
                        self.log( 'DEBUG', msg='use row %s (scanno=%s)' % (idx1,scan.getscan(idx1)) )
                        return proximate
                    else:
                        self.log( 'DEBUG', msg='interpolate between %s and %s (scanno: %s, %s)' %(idx1,idx2,scan.getscan(idx1),scan.getscan(idx2)) )
                        interp=(immediate-proximate)/(dt1+dt2)*dt1+proximate
                        return interp
        self.log( 'ERROR', msg='failed to get appropriate spectrum.' )
        return numpy.array( [], float )

    def average( self, weight='tint' ):
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
        if ( self.__caltype == 'otf' ):
            self.log( 'INFO', msg='average() is disabled for OTF data.' )
        else:
            self.log( 'INFO', msg='averaging dumped data for each scan.' )
            averaged_scan=average_time( self.__tableout, scanav=True, weight=weight )
            self.__tableout=averaged_scan

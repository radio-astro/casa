from __future__ import absolute_import

import os
import re
import numpy

import asap as sd

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.domain.datatable import DataTableImpl as DataTable
from pipeline.domain.datatable import DataTableColumnMaskList as ColMaskList

from ..common import science_spw, mjd_to_datestring, TableSelector

LOG = infrastructure.get_logger(__name__)

class DataTableReader(object):
    def __init__(self, context, table_name):
        self.context = context
        self.table_name = table_name
        self.datatable = DataTable(name=self.table_name)
        self.vAnt = 0

    def get_datatable(self):
        return self.datatable

    def set_name(self, name):
        self.name = name.rstrip('/')

    def detect_target_spw(self):
        st = self.context.observing_run.get_scantable(os.path.basename(self.name))
        return list(science_spw(st.spectral_window))
                
    def execute(self, dry_run=True):
        if dry_run:
            return
        
        name = self.name
        spwids = self.detect_target_spw()
        
        Rad2Deg = 180. / 3.141592653
        
        LOG.info('name=%s'%(name))
        if self.datatable.has_key('FILENAMES'):
            filenames = self.datatable.getkeyword('FILENAMES')
            filenames = numpy.concatenate((filenames,[name]))
        else:
            filenames = [name]
        self.datatable.putkeyword('FILENAMES',filenames)
        s = sd.scantable(name, average=False)
        selector = sd.selector(ifs=spwids)
        s.set_selection(selector)
        nrow = s.nrow()
        npol = s.npol()
        nbeam = s.nbeam()
        nif = s.nif()
        vorg=sd.rcParams['verbose']
        sd.rcParams['verbose']=False
        sd.asaplog.disable()
        Tsys = s.get_tsys()
        sd.rcParams['verbose']=vorg
        sd.asaplog.enable()
        if s.get_azimuth()[0] == 0: s.recalc_azel()
        
        #with casatools.TableReader(name) as tb:
        with TableSelector(name, 'IFNO IN %s'%(list(spwids))) as tb:
            rows = tb.rownumbers()
            Texpt = tb.getcol('INTERVAL')
            Tmjd = tb.getcol('TIME')
            # ASAP doesn't know the rows for cal are included in s.nrow()
            # nrow = len(Ttime)
            Tscan = tb.getcol('SCANNO')
            Tif = tb.getcol('IFNO')
            Tpol = tb.getcol('POLNO')
            Tbeam = tb.getcol('BEAMNO')
            Tsrctype = tb.getcol('SRCTYPE')
            Tflagrow = tb.getcol('FLAGROW')
            # 2009/10/19 nchan for scantable is not correctly set
            NchanArray = numpy.zeros(nrow, numpy.int)
            for row in range(nrow):
                NchanArray[row] = len(tb.getcell('SPECTRA', row))

        # 2011/10/23 GK List of DataTable for multiple antennas
        #self.datatable = {}
        # Save file name to be able to load the special setup needed for the
        # flagging based on the expected RMS.
        # 2011/10/23 GK List of DataTable for multiple antennas
        #if self.datatable.has_key('FileName') == False: self.datatable['FileName']  = ['']
        #self.datatable.putkeyword('FileName',[''])

        #self.datatable['FileName'] = rawFile
        #self.datatable['FileName'].append(rawFile)

        #if 'FileName' in self.datatable.tb.keywordnames():
        #    l = self.datatable.getkeyword('FileName').tolist()
        #    self.datatable.putkeyword('FileName',l+[rawFile])
        #else:
        #    self.datatable.putkeyword('FileName',[rawFile])

##         rawFileList.append(rawFile)
        #self.Row2ID[self.vAnt] = {}
##         outfile = open(TableOut, 'w')
##         outfile.write("!ID,Row,Scan,IF,Pol,Beam,Date,MJD,ElapsedTime,ExpTime,RA,Dec,Az,El,Nchan,Tsys,TargetName,AntennaID\n")

        # 2011/10/23 GK List of DataTable for multiple antennas
        #ID = len(self.datatable)-1
        ID = len(self.datatable)
        #ID = len(self.datatable)
##         self.LogMessage('INFO',Msg='ID=%s'%(ID))
        LOG.info('ID=%s'%(ID))
        #ID = 0
        ROWs = []
        IDs = []
        # 2009/7/16 to speed-up, get values as a list
        # 2011/11/8 get_direction -> get_directionval
        Sdir = s.get_directionval()
        #Sdir = s.get_direction()
        Ssrc = s.get_sourcename()
        Saz = s.get_azimuth()
        Sel = s.get_elevation()

        s.set_selection()
        del s


        # 2012/08/31 Temporary
##         if os.path.isdir(outTbl):
##             os.system('rm -rf %s' % outTbl)
##         TBL = createExportTable(outTbl, nrow)
        #TBL = gentools(['tb'])[0]
        #TBL.open(self.TableOut, nomodify=False)
        #TBL.addrows(nrow)

        self.datatable.addrows( nrow )
        # column based storing
        #intArr = numpy.arange(nrow,dtype=int)
        #self.datatable.putcol('ROW',intArr,startrow=ID)
        self.datatable.putcol('ROW',rows,startrow=ID)
        self.datatable.putcol('SCAN',Tscan,startrow=ID)
        self.datatable.putcol('IF',Tif,startrow=ID)
        self.datatable.putcol('POL',Tpol,startrow=ID)
        self.datatable.putcol('BEAM',Tbeam,startrow=ID)
        self.datatable.putcol('TIME',Tmjd,startrow=ID)
        self.datatable.putcol('ELAPSED',(Tmjd-Tmjd[0])*86400.0,startrow=ID)
        self.datatable.putcol('EXPOSURE',Texpt,startrow=ID)
        dirNP = numpy.array(Sdir,dtype=float) * Rad2Deg
        self.datatable.putcol('RA',dirNP[:,0],startrow=ID)
        self.datatable.putcol('DEC',dirNP[:,1],startrow=ID)
        azNP = numpy.array(Saz,dtype=float) * Rad2Deg
        self.datatable.putcol('AZ',azNP,startrow=ID)
        elNP = numpy.array(Sel,dtype=float) * Rad2Deg
        self.datatable.putcol('EL',elNP,startrow=ID)
        self.datatable.putcol('NCHAN',NchanArray,startrow=ID)
        self.datatable.putcol('TSYS',Tsys,startrow=ID)
        self.datatable.putcol('TARGET',Ssrc,startrow=ID)
        #intArr[:] = 1
        intArr = numpy.ones(nrow, dtype=int)
        self.datatable.putcol('FLAG_SUMMARY',intArr,startrow=ID)
        intArr[:] = 0
        self.datatable.putcol('NMASK',intArr,startrow=ID)
        intArr[:] = -1
        self.datatable.putcol('NOCHANGE',intArr,startrow=ID)
        self.datatable.putcol('POSGRP',intArr,startrow=ID)
        self.datatable.putcol('TIMEGRP_S',intArr,startrow=ID)
        self.datatable.putcol('TIMEGRP_L',intArr,startrow=ID)
        intArr[:] = self.vAnt
        self.datatable.putcol('ANTENNA',intArr,startrow=ID)
        self.datatable.putcol('SRCTYPE',Tsrctype,startrow=ID)
        # row base storing
        stats = [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0]
        flags = [1, 1, 1, 1, 1, 1, 1]
        pflags = [1, 1, 1]
        masklist = ColMaskList.NoMask
        for x in xrange(nrow):
            # FLAGROW is mapped into UserFlag (PermanentFlag[2])
            # NOTE: data is valid if Tflagrow is 0
            #       data is valid if pflags[2] is 1
            pflags[2] = 1 if Tflagrow[x] == 0 else 0
            sDate = mjd_to_datestring(Tmjd[x],unit='day')
            self.datatable.putcell('DATE',ID,sDate)
            self.datatable.putcell('STATISTICS',ID,stats)
            self.datatable.putcell('FLAG',ID,flags)
            self.datatable.putcell('FLAG_PERMANENT',ID,pflags)
            self.datatable.putcell('MASKLIST',ID,masklist)
            ID += 1

        self.vAnt += 1

def _detect_target_spw(spectral_windows):
    # exclude spws for WVR and square-law detector
    exclude_name = ['SQLD', 'WVR']
    for (spwid, spw) in spectral_windows.items():
        spw_name = spw.name
        if all([spw_name.find(name) == -1 for name in exclude_name]):
            yield spwid

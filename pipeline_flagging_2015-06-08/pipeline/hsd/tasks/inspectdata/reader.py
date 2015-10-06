from __future__ import absolute_import

import os
import re
import numpy

import asap as sd

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.domain.datatable import DataTableImpl as DataTable
from pipeline.domain.datatable import DataTableColumnMaskList as ColMaskList
from pipeline.domain.datatable import OnlineFlagIndex

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
        exclude_types = list(self.detect_exclude_type())
        
        Rad2Deg = 180. / 3.141592653
        
        LOG.info('name=%s'%(name))
        if self.datatable.has_key('FILENAMES'):
            filenames = self.datatable.getkeyword('FILENAMES')
            filenames = numpy.concatenate((filenames,[name]))
        else:
            filenames = [name]
        self.datatable.putkeyword('FILENAMES',filenames)

        storage_save = sd.rcParams['scantable.storage']
        try:
            # to reduce memory usage, use disk storage mode
            sd.rcParams['scantable.storage'] = 'disk'
            s = sd.scantable(name, average=False)
            selector = sd.selector(ifs=spwids)
            selector.set_query('SRCTYPE NOT IN %s'%(exclude_types))
            s.set_selection(selector)
            nrow = s.nrow()
            npol = s.npol()
            nbeam = s.nbeam()
            nif = s.nif()
            nchan_map = dict([(n,s.nchan(n)) for n in s.getifnos()])
            #vorg=sd.rcParams['verbose']
            #sd.rcParams['verbose']=False
            #sd.asaplog.disable()
            #Tsys = s.get_tsys()
            #sd.rcParams['verbose']=vorg
            #sd.asaplog.enable()
            #if s.get_azimuth()[0] == 0: s.recalc_azel()
            if s.get_azimuth(0) == 0: s.recalc_azel()

            # 2009/7/16 to speed-up, get values as a list
            # 2011/11/8 get_direction -> get_directionval
            #Sdir = s.get_directionval()
            ##Sdir = s.get_direction()
            #Ssrc = s.get_sourcename()
            #Saz = s.get_azimuth()
            #Sel = s.get_elevation()

            s.set_selection()
            del s
        finally:
            sd.rcParams['scantable.storage'] = storage_save
        
        #with casatools.TableReader(name) as tb:
        with TableSelector(name, 'IFNO IN %s and SRCTYPE NOT IN %s'%(spwids, exclude_types)) as tb:
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
            Tdirection = tb.getcol('DIRECTION')
            Tdirection *= Rad2Deg
            Taz = tb.getcol('AZIMUTH')
            Taz *= Rad2Deg
            Tel = tb.getcol('ELEVATION')
            Tel *= Rad2Deg
            Tsrc = tb.getcol('SRCNAME')
            # This is equivalent to sd.scantable.get_tsys() that
            # returns first element of Tsys array for each row
            Tsys = tb.getcolslice('TSYS', [0], [0], [1]).squeeze()
            # 2009/10/19 nchan for scantable is not correctly set
            #NchanArray = numpy.zeros(nrow, numpy.int)
            #for row in range(nrow):
            #    NchanArray[row] = len(tb.getcell('SPECTRA', row))
            NchanArray = numpy.fromiter((nchan_map[n] for n in Tif), dtype=numpy.int)   

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
        #dirNP = numpy.array(Sdir,dtype=float) * Rad2Deg
        #self.datatable.putcol('RA',dirNP[:,0],startrow=ID)
        #self.datatable.putcol('DEC',dirNP[:,1],startrow=ID)
        self.datatable.putcol('RA',Tdirection[0],startrow=ID)
        self.datatable.putcol('DEC',Tdirection[1],startrow=ID)
        #azNP = numpy.array(Saz,dtype=float) * Rad2Deg
        #self.datatable.putcol('AZ',azNP,startrow=ID)
        self.datatable.putcol('AZ',Taz,startrow=ID)
        #elNP = numpy.array(Sel,dtype=float) * Rad2Deg
        #self.datatable.putcol('EL',elNP,startrow=ID)
        self.datatable.putcol('EL',Tel,startrow=ID)
        self.datatable.putcol('NCHAN',NchanArray,startrow=ID)
        self.datatable.putcol('TSYS',Tsys,startrow=ID)
        #self.datatable.putcol('TARGET',Ssrc,startrow=ID)
        self.datatable.putcol('TARGET',Tsrc,startrow=ID)
        #intArr[:] = 1
        intArr = numpy.ones(nrow, dtype=int)
        self.datatable.putcol('FLAG_SUMMARY',intArr,startrow=ID)
        intArr[:] = 0
        self.datatable.putcol('NMASK',intArr,startrow=ID)
        intArr[:] = -1
        self.datatable.putcol('NOCHANGE',intArr,startrow=ID)
        self.datatable.putcol('POSGRP',intArr,startrow=ID)
        intArr[:] = self.vAnt
        self.datatable.putcol('ANTENNA',intArr,startrow=ID)
        self.datatable.putcol('SRCTYPE',Tsrctype,startrow=ID)
        # row base storing
        stats = [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0]
        flags = [1, 1, 1, 1, 1, 1, 1]
        pflags = [1, 1, 1, 1]
        masklist = ColMaskList.NoMask
        for x in xrange(nrow):
            # FLAGROW is mapped into OnlineFlag (PermanentFlag[3])
            # NOTE: data is valid if Tflagrow is 0
            #       data is valid if pflags[3] is 1
            pflags[OnlineFlagIndex] = 1 if Tflagrow[x] == 0 else 0
            sDate = mjd_to_datestring(Tmjd[x],unit='day')
            self.datatable.putcell('DATE',ID,sDate)
            self.datatable.putcell('STATISTICS',ID,stats)
            self.datatable.putcell('FLAG',ID,flags)
            self.datatable.putcell('FLAG_PERMANENT',ID,pflags)
            self.datatable.putcell('MASKLIST',ID,masklist)
            ID += 1

        self.vAnt += 1

    def detect_exclude_type(self):
        return map(int, [sd.srctype.poncal, sd.srctype.poffcal])

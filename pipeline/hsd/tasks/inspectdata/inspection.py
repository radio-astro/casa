from __future__ import absolute_import

import re

import pipeline.infrastructure.api as api
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.jobrequest import casa_tasks
from pipeline.domain.datatable import DataTableImpl as DataTable
#from . import common
from pipeline.hsd.tasks.common import common

LOG = logging.get_logger(__name__)

import os
import asap as sd
import numpy 

class SDInspectDataInputs(common.SingleDishInputs):
    """
    Inputs for single dish calibraton
    """
    def __init__(self, context, infiles=None):
        self._init_properties(vars())
        

class SDInspectDataResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(SDInspectDataResults,self).__init__(task, success, outcome)

    def __getstate__(self):
        mydict = self.__dict__.copy()
        outcome = mydict['outcome'].copy()
        outcome['instance'] = None
        mydict['outcome'] = outcome
        return mydict

    def __setstate__(self, d):
        self.__dict__ = d
        name = self.__dict__['outcome']['name']
        if self.__dict__['outcome']['instance'] is None:
            datatable = DataTable(name)
            self.__dict__['outcome']['instance'] = datatable
        
    
    def merge_with_context(self, context):
        context.observing_run.set_datatable(**self.outcome)
        super(SDInspectDataResults,self).merge_with_context(context)

    def _outcome_name(self):
        name = self.outcome['name']
        return os.path.abspath(os.path.expanduser(os.path.expandvars(name)))


class SDInspectData(common.SingleDishTaskTemplate):
    Inputs = SDInspectDataInputs

    def prepare(self):
        # use plain table for scantable
        storage_save = sd.rcParams['scantable.storage']
        sd.rcParams['scantable.storage'] = 'disk'

        # inputs 
        inputs = self.inputs

        # create DataTable under context directory
        table_name = os.path.join(inputs.context.name,'DataTable.tbl')

        if os.path.exists(table_name):
            # if DataTable already exists, remove it
            LOG.info('remove existing DataTable...')
            os.system('rm -rf %s'%(table_name))
        
        self.DataTable = DataTable(name=table_name)
        LOG.info('table_name=%s'%(table_name))


        # loop over infiles
        self.vAnt = 0
        self.Row2ID = {}
        infiles = inputs.infiles
        if isinstance(infiles, list):
            for f in infiles:
                self._read_metadata(f)
        else:
            self._read_metadata(infiles)
        
        # finally, export DataTable
        self.DataTable.exportdata(minimal=False)

        # done, restore scantable.storage
        sd.rcParams['scantable.storage'] = storage_save

        outcome = {}
        outcome['name'] = table_name
        outcome['instance'] = self.DataTable

        result = SDInspectDataResults(success=True, outcome=outcome)

        return result

    def analyse(self, result):

        return result


    def _read_metadata(self, name):
        Rad2Deg = 180. / 3.141592653
        
        LOG.info('name=%s'%(name))
        if self.DataTable.has_key('FILENAMES'):
            filenames = self.DataTable.getkeyword('FILENAMES')
            filenames = numpy.concatenate((filenames,[name]))
        else:
            filenames = [name]
        self.DataTable.putkeyword('FILENAMES',filenames)
        s = sd.scantable(name, average=False)
        nrow = s.nrow()
        npol = s.npol()
        nbeam = s.nbeam()
        nif = s.nif()
        vorg=sd.rcParams['verbose']
        sd.rcParams['verbose']=False
        if self.casa_version > 302:
            sd.asaplog.disable()
        Tsys = s.get_tsys()
        sd.rcParams['verbose']=vorg
        if self.casa_version > 302:
            sd.asaplog.enable()
        if s.get_azimuth()[0] == 0: s.recalc_azel()
        
        with casatools.TableReader(name) as tb:
            Texpt = tb.getcol('INTERVAL')
            # ASAP doesn't know the rows for cal are included in s.nrow()
            # nrow = len(Ttime)
            Tscan = tb.getcol('SCANNO')
            Tif = tb.getcol('IFNO')
            Tpol = tb.getcol('POLNO')
            Tbeam = tb.getcol('BEAMNO')
            Tsrctype = tb.getcol('SRCTYPE')
            # 2009/10/19 nchan for scantable is not correctly set
            NchanArray = numpy.zeros(nrow, numpy.int)
            for row in range(nrow):
                NchanArray[row] = len(tb.getcell('SPECTRA', row))

        # 2011/10/23 GK List of DataTable for multiple antennas
        #self.DataTable = {}
        # Save file name to be able to load the special setup needed for the
        # flagging based on the expected RMS.
        # 2011/10/23 GK List of DataTable for multiple antennas
        #if self.DataTable.has_key('FileName') == False: self.DataTable['FileName']  = ['']
        #self.DataTable.putkeyword('FileName',[''])

        #self.DataTable['FileName'] = rawFile
        #self.DataTable['FileName'].append(rawFile)

        #if 'FileName' in self.DataTable.tb.keywordnames():
        #    l = self.DataTable.getkeyword('FileName').tolist()
        #    self.DataTable.putkeyword('FileName',l+[rawFile])
        #else:
        #    self.DataTable.putkeyword('FileName',[rawFile])

##         rawFileList.append(rawFile)
        self.Row2ID[self.vAnt] = {}
##         outfile = open(TableOut, 'w')
##         outfile.write("!ID,Row,Scan,IF,Pol,Beam,Date,MJD,ElapsedTime,ExpTime,RA,Dec,Az,El,Nchan,Tsys,TargetName,AntennaID\n")

        # 2011/10/23 GK List of DataTable for multiple antennas
        #ID = len(self.DataTable)-1
        ID = len(self.DataTable)
        #ID = len(self.DataTable)
##         self.LogMessage('INFO',Msg='ID=%s'%(ID))
        LOG.info('ID=%s'%(ID))
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
##         if os.path.isdir(outTbl):
##             os.system('rm -rf %s' % outTbl)
##         TBL = createExportTable(outTbl, nrow)
        #TBL = gentools(['tb'])[0]
        #TBL.open(self.TableOut, nomodify=False)
        #TBL.addrows(nrow)

        self.DataTable.addrows( nrow )
        # column based storing
        intArr = numpy.arange(nrow,dtype=int)
        self.DataTable.putcol('ROW',intArr,startrow=ID)
        self.DataTable.putcol('SCAN',Tscan,startrow=ID)
        self.DataTable.putcol('IF',Tif,startrow=ID)
        self.DataTable.putcol('POL',Tpol,startrow=ID)
        self.DataTable.putcol('BEAM',Tbeam,startrow=ID)
        self.DataTable.putcol('EXPOSURE',Texpt,startrow=ID)
        dirNP = numpy.array(Sdir,dtype=float) * Rad2Deg
        self.DataTable.putcol('RA',dirNP[:,0],startrow=ID)
        self.DataTable.putcol('DEC',dirNP[:,1],startrow=ID)
        azNP = numpy.array(Saz,dtype=float) * Rad2Deg
        self.DataTable.putcol('AZ',azNP,startrow=ID)
        elNP = numpy.array(Sel,dtype=float) * Rad2Deg
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
        self.DataTable.putcol('POSGRP',intArr,startrow=ID)
        self.DataTable.putcol('TIMEGRP_S',intArr,startrow=ID)
        self.DataTable.putcol('TIMEGRP_L',intArr,startrow=ID)
        intArr[:] = self.vAnt
        self.DataTable.putcol('ANTENNA',intArr,startrow=ID)
        self.DataTable.putcol('SRCTYPE',Tsrctype,startrow=ID)
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
            qTime = casatools.quanta.quantity(sTime)
            MJD = qTime['value']
            if x == 0: MJD0 = MJD
            self.DataTable.putcell('DATE',ID,sDate)
            self.DataTable.putcell('TIME',ID,MJD)
            self.DataTable.putcell('ELAPSED',ID,(MJD-MJD0)*86400.0)
            self.DataTable.putcell('STATISTICS',ID,stats)
            self.DataTable.putcell('FLAG',ID,flags)
            self.DataTable.putcell('FLAG_PERMANENT',ID,pflags)
            self.DataTable.putcell('MASKLIST',ID,masklist)
##             if Ssrc[x].find('_calon') < 0:
##                 outfile.write("%d,%d,%d,%d,%d,%d,%s,%.8f,%.3f,%.3f,%.8f,%.8f,%.3f,%.3f,%d,%f,%s,%d\n" % \
##                          (ID, x, Tscan[x], Tif[x], Tpol[x], Tbeam[x], \
##                          sDate, MJD, (MJD - MJD0) * 86400., Texpt[x], \
##                           dirNP[x,0],dirNP[x,1], \
##                           azNP[x], elNP[x], \
##                          NchanArray[x], Tsys[x], Ssrc[x], vAnt))

##                 TBL.putcol('Row', int(x), int(x), 1, 1)
##                 TBL.putcol('Ant', vAnt, int(x), 1, 1)
##                 ROWs.append(int(x))
##                 IDs.append(int(ID))
##                 self.Row2ID[vAnt][int(x)] = int(ID)
            ID += 1

        self.vAnt += 1


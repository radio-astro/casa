#
# ALMA - Atacama Large Milliiter Array (c) European Southern Observatory, 2002
# Copyright by ESO (in the framework of the ALMA collaboration), All rights
# reserved
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

#
# $Revision: 1.1.2.6 $
# $Date: 2013/03/01 05:07:45 $
# $Author: tnakazat $
#
import os
import time
import numpy
import math
import re
import collections
import shutil
import itertools

#import memory_profiler

from taskinit import gentools

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

def __coldesc( vtype, option, maxlen,
             ndim, comment, unit=None ):
    d={'dataManagerGroup': 'StandardStMan',
       'dataManagerType': 'StandardStMan'}
    d['valueType'] = vtype
    d['option'] = option
    if ndim > 0:
        d['ndim'] = ndim
    d['maxlen'] = maxlen
    d['comment'] = comment
    if unit is not None:
        d['keywords'] = {'UNIT': unit }
    return d

# Description for data table columns as dictionary.
# Each value is a tuple containing:
#
#    (valueType,option,maxlen,ndim,comment[,unit])
#
# dataManagerGroup and dataManagerType is always 'StandardStMan'.
def __tabledescro():
    TD_DESC_RO = [
    #    __coldesc('integer', 0, 0, -1, 'Primary key'),
        __coldesc('integer', 0, 0, -1, 'Row number'),
        __coldesc('integer', 0, 0, -1, 'Scan number'),
        __coldesc('integer', 0, 0, -1, 'IF number'),
        __coldesc('integer', 0, 0, -1, 'Number of Polarizations'),
        __coldesc('integer', 0, 0, -1, 'Beam number'),
        __coldesc('string',  0, 0, -1, 'Date'),
        __coldesc('double',  0, 0, -1, 'Time in MJD', 'd'),
        __coldesc('double',  0, 0, -1, 'Elapsed time since first scan' 'd'),
        __coldesc('double',  0, 0, -1, 'Exposure time', 's'),
        __coldesc('double',  0, 0, -1, 'Right Ascension', 'deg'),
        __coldesc('double',  0, 0, -1, 'Declination', 'deg'),
        __coldesc('double',  0, 0, -1, 'Azimuth', 'deg'),
        __coldesc('double',  0, 0, -1, 'Elevation', 'deg'),
        __coldesc('integer', 0, 0, -1, 'Number of channels'),
        __coldesc('double',  0, 0,  1, 'Tsys', 'K'),
        __coldesc('string',  0, 0, -1, 'Target name'),
        __coldesc('integer', 0, 0, -1, 'Antenna index'),
        __coldesc('integer', 0, 0, -1, 'Source type enum'),
        __coldesc('integer', 0, 0, -1, 'MS index'),
        __coldesc('integer', 0, 0, -1, 'Field ID')
        ]

    name = [
        'ROW', 'SCAN', 'IF', 'NPOL', 'BEAM', 'DATE',
        'TIME', 'ELAPSED', 'EXPOSURE', 'RA', 'DEC',
        'AZ', 'EL', 'NCHAN', 'TSYS', 'TARGET', 'ANTENNA',
        'SRCTYPE', 'MS', 'FIELD_ID'
        ]
    return dict( itertools.izip(name,TD_DESC_RO) )

def __tabledescrw():
    TD_DESC_RW = [
        __coldesc('double',  0, 0,  2, 'Statistics'),
        __coldesc('integer', 0, 0,  2, 'Flgas'),
        __coldesc('integer', 0, 0,  2, 'Permanent flags'),
        __coldesc('integer', 0, 0,  1, 'Actual flag'),
        __coldesc('integer', 0, 0, -1, 'Number of mask regions'),
        __coldesc('integer', 0, 0,  2, 'List of mask ranges'),
        __coldesc('integer', 0, 0, -1, 'Unchanged row or not'),
        __coldesc('integer', 0, 0, -1, 'Position group id')#,
        ]

    name = [
        'STATISTICS', 'FLAG', 'FLAG_PERMANENT',
        'FLAG_SUMMARY', 'NMASK', 'MASKLIST', 'NOCHANGE',
        'POSGRP']
    return dict( itertools.izip(name,TD_DESC_RW) )

TABLEDESC_RO = __tabledescro()
TABLEDESC_RW = __tabledescrw()

def create_table(table, name, desc, memtype='plain', nrow=0):
    ret = table.create(name, desc, memtype=memtype, nrow=nrow)
    assert ret == True
    for (_colname, _coldesc) in desc.iteritems():
        if _coldesc.has_key('keywords'):
            table.putcolkeywords(_colname, _coldesc['keywords'])

# FLAG_PERMANENT Layout
WeatherFlagIndex = 0
TsysFlagIndex = 1
UserFlagIndex = 2
OnlineFlagIndex = 3

def absolute_path(name):
    return os.path.abspath(os.path.expanduser(os.path.expandvars(name)))

def timetable_key(table_type, antenna, spw, polarization=None, ms=None, field_id=None):
    key = 'TIMETABLE_%s'%(table_type)
    if ms is not None:
        key = key + '_%s'%(ms.replace('.','_'))
    if field_id is not None:
        key = key + '_FIELD%s'%(field_id)
    key = key + '_ANT%s_SPW%s'%(antenna, spw)
    if polarization is not None:
        key = key + '_POL%s'%(polarization) 
    return key   
    
class DataTableImpl( object ):
    """
    DataTable is an object to hold meta data of scantable on memory. 
    
    row layout: [Row, Scan, IF, Pol, Beam, Date, Time, ElapsedTime,
                   0,    1,  2,   3,    4,    5,    6,            7,
                 Exptime, RA, DEC, Az, El, nchan, Tsys, TargetName, 
                       8,  9,  10, 11, 12,    13,   14,         15,
                 Statistics, Flags, PermanentFlags, SummaryFlag, Nmask, MaskList, NoChange, Ant]
                         16,    17,             18,          19,    20,       21,       22,  23
    Statistics: DataTable[ID][16] =
                [LowFreqRMS, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, ExpectedRMS, ExpectedRMS]
                          0,      1,      2,          3,          4,           5,           6
    Flags: DataTable[ID][17] =
                [LowFrRMSFlag, PostFitRMSFlag, PreFitRMSFlag, PostFitRMSdiff, PreFitRMSdiff, PostFitExpRMSFlag, PreFitExpRMSFlag]
                            0,              1,             2,              3,             4,                 5,                6
    PermanentFlags: DataTable[ID][18] =
                [WeatherFlag, TsysFlag, UserFlag]
                           0,        1,        2
    Note for Flags: 1 is valid, 0 is invalid
    """ 
    @classmethod
    def get_rotable_name(cls, datatable_name):
        return os.path.join(datatable_name, 'RO')
    
    @classmethod
    def get_rwtable_name(cls, datatable_name):
        return os.path.join(datatable_name, 'RW')
    
    def __init__(self, name=None, readonly=None):
        """
        name (optional) -- name of DataTable
        """
        # unique memory table name
        timestamp = ('%f'%(time.time())).replace('.','')
        self.memtable1 = 'DataTableImplRO%s.MemoryTable'%(timestamp)
        self.memtable2 = 'DataTableImplRW%s.MemoryTable'%(timestamp)
        self.plaintable = ''
        self.cols = {}
        
        (self.tb1,self.tb2) = gentools( ['tb','tb'] )
        self.isopened = False
        if name is None or len(name) == 0:
            if readonly is None:
                readonly = False
            self._create(readonly=readonly)
        elif not os.path.exists(name):
            if readonly is None:
                readonly = False
            self._create(readonly=readonly)
            self.plaintable = absolute_path(name)
        else:
            if readonly is None:
                readonly = True
            self.importdata2(name=name, minimal=False, readonly=readonly)

    def __del__( self ):
        # make sure that table is closed
        #LOG.debug('__del__ close CASA table...')
        self.cols.clear()
        self._close()

    def __len__( self ):
        return self.nrow

    @property
    def nrow(self):
        if self.isopened:
            return self.tb1.nrows()
        else:
            return 0

    @property
    def name(self):
        return self.plaintable

    @property
    def position_group_id(self):
        key = 'POSGRP_REP'
        if self.has_key(key):
            return numpy.max(numpy.fromiter((int(x) for x in self.getkeyword(key).keys()), dtype=numpy.int32)) + 1
        else:
            return 0
    
    @property
    def time_group_id_small(self):
        return self.__get_time_group_id(True)
    
    @property
    def time_group_id_large(self):
        return self.__get_time_group_id(False)
                    
    def __get_time_group_id(self, small=True):
        if small:
            subkey = 'SMALL'
        else:
            subkey = 'LARGE'
        pattern = '^TIMETABLE_%s_.*'%(subkey)
        if numpy.any(numpy.fromiter((re.match(pattern, x) is not None for x in self.keywordnames()), dtype=bool)):
            group_id = 0
            for key in self.tb2.keywordnames():
                if re.match(pattern, key) is not None:
                    max_id = numpy.max(numpy.fromiter((int(x) for x in self.getkeyword(key).keys()), dtype=numpy.int32)) + 1
                    group_id = max(group_id, max_id)
            return group_id
        else:
            return 0
        
    def get_row_index_simple(self, col, val):
        vals = self.getcol(col)
        return [i for i in xrange(self.nrow) if vals[i] == val]

    def get_row_index(self, msid, antenna, ifno, polno=None):
        mses = self.getcol('MS')
        ants = self.getcol('ANTENNA')
        ifs = self.getcol('IF')
        if polno is None:
            ref = [msid, antenna, ifno]
            return [i for i in xrange(self.nrow) if [mses[i], ants[i], ifs[i]] == ref]
        else:
            pols = self.getcol('POL')
            ref = [msid, antenna, ifno, polno]
            return [i for i in xrange(self.nrow) if [mses[i], ants[i], ifs[i], pols[i]] == ref]

    def has_key(self, name):
        return (name in self.tb2.keywordnames())

    def addrows( self, nrow ):
        self.tb1.addrows( nrow )
        self.tb2.addrows( nrow )

        # reset self.plaintable since memory table and corresponding
        # plain table have different nrows
        self.plaintable == ''

    def colnames( self ):
        return self.cols.keys()

    def getcol( self, name, startrow=0, nrow=-1, rowincr=1 ):
        return self.cols[name].getcol(startrow,nrow,rowincr)

    def putcol( self, name, val, startrow=0, nrow=-1, rowincr=1  ):
        self.cols[name].putcol(val,startrow,nrow,rowincr)

    def getcell( self, name, idx ):
        return self.cols[name].getcell(idx)

    def putcell( self, name, idx, val ):
        """
        name -- column name
        idx -- row index
        val -- value to be put
        """
        self.cols[name].putcell(idx,val)

    def getcolslice(self, name, blc, trc, incr, startrow=0, nrow=-1, rowincr=1):
        return self.cols[name].getcolslice(blc, trc, incr, startrow, nrow, rowincr)
        
    def putcolslice(self, name, value, blc, trc, incr, startrow=0, nrow=-1, rowincr=1):
        self.cols[name].putcolslice(value, blc, trc, incr, startrow, nrow, rowincr)
        
    def getcellslice(self, name, rownr, blc, trc, incr):
        return self.cols[name].getcellslice(rownr, blc, trc, incr)
    
    def putcellslice(self, name, rownr, value, blc, trc, incr):
        self.cols[name].putcellslice(rownr, value, blc, trc, incr)
        
    def getcolkeyword(self, columnname, keyword):
        if columnname in TABLEDESC_RO.keys():
            return self.tb1.getcolkeyword(columnname, keyword)
        else:
            return self.tb2.getcolkeyword(columnname, keyword)

    def putkeyword( self, name, val ):
        """
        name -- keyword name
        val -- keyword value
        """
        self.tb2.putkeyword( name, val )

    def getkeyword( self, name ):
        """
        name -- keyword name
        """
        return self.tb2.getkeyword( name )
    
    def keywordnames(self):
        """
        return table keyword names
        """
        return self.tb2.keywordnames()

    def importdata( self, name, minimal=True, readonly=True ):
        """
        name -- name of DataTable to be imported
        """
        LOG.debug('Importing DataTable from %s...'%(name))

        # copy input table to memory
        self._copyfrom( name, minimal )
        self.plaintable = absolute_path(name)
        self.__init_cols(readonly=readonly)

    def importdata2( self, name, minimal=True, readonly=True ):
        """
        name -- name of DataTable to be imported
        """
        LOG.debug('Importing DataTable from %s...'%(name))

        # copy input table to memory
        self._copyfrom2( name, minimal )
        self.plaintable = absolute_path(name)
        self.__init_cols(readonly=readonly)

    def sync(self, minimal=True):
        """
        Sync with DataTable on disk.
        """
        self.importdata(name=self.plaintable, minimal=minimal)

    def exportdata( self, name=None, minimal=True, overwrite=False ):
        """
        name -- name of exported DataTable
        overwrite -- overwrite existing DataTable
        """
        if name is None or len(name) == 0:
            if len(self.plaintable) == 0:
                raise IOError('You have to specify name of export table')
            else:
                name = self.plaintable
                overwrite = True
        
        LOG.debug('Exporting DataTable to %s...'%(name))
        # overwrite check
        abspath = absolute_path(name)
        basename = os.path.basename(abspath)
        if not os.path.exists(abspath):
            os.mkdir(abspath)
        elif overwrite:
            LOG.debug('Overwrite existing DataTable %s...'%(name))
            #os.system( 'rm -rf %s/*'%(abspath) )
        else:
            raise IOError('The file %s exists.'%(name))

        # save
        if not minimal or not os.path.exists( os.path.join(abspath,'RO') ):
            #LOG.trace('Exporting RO table')
            if os.path.exists(self.tb1.name()):
                # self.tb1 seems to be plain table, nothing to be done
                pass
            else:
                # tb1 is memory table
                tbloc = self.tb1.copy( os.path.join(abspath,'RO'), deep=True,
                                   valuecopy=True, returnobject=True )
                tbloc.close()
        #LOG.trace('Exporting RW table')
        tbloc = self.tb2.copy( os.path.join(abspath,'RW'), deep=True,
                               valuecopy=True, returnobject=True )
        tbloc.close()
        self.plaintable = abspath

    def _create( self, readonly=False ):
        self._close()
        create_table(self.tb1, self.memtable1, TABLEDESC_RO, 'memory', self.nrow)
        create_table(self.tb2, self.memtable2, TABLEDESC_RW, 'memory', self.nrow)
        self.isopened = True
        self.__init_cols(readonly=readonly)

    def __init_cols(self, readonly=True):
        self.cols.clear()
        if readonly:
            RO_COLUMN = RODataTableColumn
            RW_COLUMN = RWDataTableColumn
        else:
            RO_COLUMN = RWDataTableColumn
            RW_COLUMN = RWDataTableColumn
        type_map = {'integer': int,
                    'double': float,
                    'string': str}
        datatype = lambda desc: list if desc.has_key('ndim') and desc['ndim'] > 0 \
                                  else type_map[desc['valueType']]
        for (k,v) in TABLEDESC_RO.iteritems():
            self.cols[k] = RO_COLUMN(self.tb1,k,datatype(v))
        for (k,v) in TABLEDESC_RW.iteritems():
            if k == 'MASKLIST':
                self.cols[k] = DataTableColumnMaskList(self.tb2)
            elif k == 'NOCHANGE':
                self.cols[k] = DataTableColumnNoChange(self.tb2)
            else:
                self.cols[k] = RW_COLUMN(self.tb2,k,datatype(v))

    def _close( self ):
        if self.isopened:
            self.tb1.close()
            self.tb2.close()
            self.isopened = False

    def _copyfrom( self, name, minimal=True ):
        self._close()
        abspath = absolute_path(name)
        if not minimal or abspath != self.plaintable:
            with casatools.TableReader(os.path.join(name,'RO')) as tb:
                self.tb1 = tb.copy( self.memtable1, deep=True,
                                    valuecopy=True, memorytable=True,
                                    returnobject=True )
        with casatools.TableReader(os.path.join(name,'RW')) as tb:
            self.tb2 = tb.copy( self.memtable2, deep=True,
                                valuecopy=True, memorytable=True,
                                returnobject=True )
        self.isopened = True

    def _copyfrom2( self, name, minimal=True ):
        self._close()
        abspath = absolute_path(name)
        if not minimal or abspath != self.plaintable:
            #with casatools.TableReader(os.path.join(name,'RO')) as tb:
            #    self.tb1 = tb.copy( self.memtable1, deep=True,
            #                        valuecopy=True, memorytable=True,
            #                        returnobject=True )
            self.tb1 = casatools.casac.table()
            self.tb1.open(os.path.join(name, 'RO'), nomodify=False)
        with casatools.TableReader(os.path.join(name,'RW')) as tb:
            self.tb2 = tb.copy( self.memtable2, deep=True,
                                valuecopy=True, memorytable=True,
                                returnobject=True )
        self.isopened = True

    def get_posdict(self, ant, spw, pol):
        posgrp_list = self.getkeyword('POSGRP_LIST')
        try:
            mygrp = posgrp_list[str(ant)][str(spw)][str(pol)]
        except KeyError, e:
            raise KeyError('ant %s spw %s pol %s not in reduction group list'%(ant,spw,pol))
        except Exception, e:
            raise e
        
        posgrp_rep = self.getkeyword('POSGRP_REP')
        rows = self.getcol('ROW')
        posgrp = self.getcol('POSGRP')
        posdict = {}
        for (k,v) in posgrp_rep.iteritems():
            if int(k) not in mygrp:
                continue
            key = rows[v]
            posdict[key] = [[],[]]
            
        for idx in xrange(len(posgrp)):
            grp = posgrp[idx]
            if grp not in mygrp:
                continue
            row = rows[idx]
            rep = posgrp_rep[str(grp)]
            key = rows[rep]
            posdict[key][0].append(row)
            posdict[key][1].append(idx)
            if row != key:
                posdict[row] = [[-1,key],[rep]]

        return posdict
    
    def set_timetable(self, ant, spw, pol, mygrp, timegrp_s, timegrp_l, ms=None, field_id=None):
        # time table format
        # TimeTable: [TimeTableSmallGap, TimeTableLargeGap]
        # TimeTableXXXGap: [[[row0, row1, ...], [idx0, idx1, ...]], ...]
        LOG.info('set_timetable start')
        start_time = time.time()

        mygrp_s = mygrp['small']
        mygrp_l = mygrp['large']
        rows = self.getcol('ROW')

        timedic_s = construct_timegroup(rows, set(mygrp_s), timegrp_s)
        timedic_l = construct_timegroup(rows, set(mygrp_l), timegrp_l)
        timetable_s = [timedic_s[idx] for idx in mygrp_s]
        timetable_l = [timedic_l[idx] for idx in mygrp_l]
        timetable = [timetable_s, timetable_l]
        end_time = time.time()
        LOG.info('construct timetable: Elapsed time %s sec'%(end_time - start_time))
        
        LOG.debug('timetable=%s'%(timetable))
        
        # put time table to table keyword
        start_time2 = time.time()
        key_small = timetable_key('SMALL', ant, spw, pol, ms, field_id)
        key_large = timetable_key('LARGE', ant, spw, pol, ms, field_id)
        keys = self.tb2.keywordnames()
        LOG.debug('add time table: keys for small gap \'%s\' large gap \'%s\''%(key_small,key_large))
        dictify = lambda x:  dict([(str(i), t) for (i,t) in enumerate(x)])
        if key_small not in keys or key_large not in keys:
            self.putkeyword(key_small, dictify(timetable[0]))
            self.putkeyword(key_large, dictify(timetable[1]))
            
        end_time = time.time()
        LOG.info('put timetable: Elapsed time %s sec'%(end_time - start_time2))
        LOG.info('set get_timetable end: Elapsed time %s sec'%(end_time - start_time))

    def get_timetable(self, ant, spw, pol, ms=None, field_id=None):
        LOG.trace('new get_timetable start')
        start_time = time.time()
        key_small = timetable_key('SMALL', ant, spw, pol, ms, field_id)
        key_large = timetable_key('LARGE', ant, spw, pol, ms, field_id)
        keys = self.tb2.keywordnames()
        LOG.debug('get time table: keys for small gap \'%s\' large gap \'%s\''%(key_small,key_large))
        if key_small in keys and key_large in keys:
            ttdict_small = self.getkeyword(key_small)
            ttdict_large = self.getkeyword(key_large)
            timetable_small = [ttdict_small[str(i)].tolist() for i in xrange(len(ttdict_small))]
            timetable_large = [ttdict_large[str(i)].tolist() for i in xrange(len(ttdict_large))]
            timetable = [timetable_small, timetable_large]
        else:
            raise RuntimeError('time table for Antenna %s spw %s pol %s is not configured properly'%(ant,spw,pol))
        end_time = time.time()
        LOG.trace('new get_timetable end: Elapsed time %s sec'%(end_time - start_time))
            
        return timetable
        
            
    def get_timegap(self, ant, spw, pol, asrow=True, ms=None, field_id=None):
        timegap_s = self.getkeyword('TIMEGAP_S')
        timegap_l = self.getkeyword('TIMEGAP_L')
        if ms is None:
            try:
                mygap_s = timegap_s[str(ant)][str(spw)][str(pol)]
                mygap_l = timegap_l[str(ant)][str(spw)][str(pol)]
            except KeyError, e:
                raise KeyError('ant %s spw %s pol %s not in reduction group list'%(ant,spw,pol))
            except Exception, e:
                raise e
        else:
            try:
                mygap_s = timegap_s[ms.basename.replace('.','_')][str(ant)][str(spw)][str(field_id)]
                mygap_l = timegap_l[ms.basename.replace('.','_')][str(ant)][str(spw)][str(field_id)]
            except KeyError, e:
                raise KeyError('ms %s field %s ant %s spw %s not in reduction group list'%(ms.basename,field_id,ant,spw))
            except Exception, e:
                raise e
            

        if asrow:
            rows = self.getcol('ROW')
            timegap = [[],[]]
            for idx in mygap_s:
                timegap[0].append(rows[idx])
            for idx in mygap_l:
                timegap[1].append(rows[idx])
        else:
            timegap = [mygap_s, mygap_l]
        return timegap
    
    def _update_tsys(self, context, infile, tsystable, spwmap, to_fieldid, gainfield):
        """
        Transfer Tsys values in a Tsys calibration table and fill Tsys
        values in DataTable.
        Tsys in cal table are averaged by channels taking into account
        of FLAG and linearly interpolated in time to derive values which
        corresponds to TIME in DataTable.

        Arguments
            context: pipeline context
            infile: the name of input MS
            tsystable: the name of Tsys calibration table
            spwmap: the list of SPW mapping
            to_fieldid: FIELD_ID of data table to which Tsys is transferred
            gainfield: how to find FIELD form which Tsys is extracted in cal table.
        """
        with casatools.TableReader(tsystable) as tb:
            spws = tb.getcol('SPECTRAL_WINDOW_ID')
            times = tb.getcol('TIME')
            fieldids = tb.getcol('FIELD_ID')
            antids = tb.getcol('ANTENNA1')
            tsys_masked = {}
            for i in xrange(tb.nrows()):
                tsys = tb.getcell('FPARAM',i)
                flag = tb.getcell('FLAG',i)
                tsys_masked[i] = numpy.ma.masked_array(tsys, mask=(flag==True))

        file_list = self.getkeyword('FILENAMES').tolist()
        msid = file_list.index(os.path.abspath(infile.rstrip('/')))
        msobj = context.observing_run.get_ms(infile)
        to_antids = [a.id for a in msobj.antennas]
        from_fields = []
        if gainfield.upper() != 'NEAREST':
            from_fields = [ fld.id for fld in msobj.get_fields(gainfield) ]
            
        def map_spwchans(atm_spw, science_spw):
            """
            Map the channel ID ranges of ATMCal spw that covers frequency range of a science spw
            Arguments: spw object of ATMCal and science spws
            """
            atm_freqs = numpy.array(atm_spw.channels.chan_freqs)
            min_chan = numpy.where(abs(atm_freqs-float(science_spw.min_frequency.value))==min(abs(atm_freqs-float(science_spw.min_frequency.value))))[0][0]
            max_chan = numpy.where(abs(atm_freqs-float(science_spw.max_frequency.value))==min(abs(atm_freqs-float(science_spw.max_frequency.value))))[0][-1]
            start_atmchan = min(min_chan, max_chan)
            end_atmchan = max(min_chan, max_chan)
            #LOG.trace('calculate_average_tsys:   satrt_atmchan == %d' % start_atmchan)
            #LOG.trace('calculate_average_tsys:   end_atmchan == %d' % end_atmchan)
            if end_atmchan == start_atmchan:
                end_atmchan = start_atmchan + 1
            return start_atmchan, end_atmchan

        for spw_to, spw_from in enumerate(spwmap):
            atm_spw = msobj.get_spectral_window(spw_from)
            science_spw = msobj.get_spectral_window(spw_to)
            start_atmchan, end_atmchan = map_spwchans(atm_spw, science_spw)
            #LOG.trace("Transfer Tsys from spw %d (chans: %d~%d) to %d" % (spw_from, start_atmchan, end_atmchan, spw_to))
            for ant_to in to_antids:
                # select caltable row id by SPW and ANT
                cal_idxs = numpy.where(numpy.logical_and(spws==spw_from, antids==ant_to))[0]
                if len(cal_idxs)==0:
                    continue
                dtrows = self.get_row_index(msid, ant_to, spw_to, None)
                time_sel = times.take(cal_idxs) #in sec
                field_sel = fieldids.take(cal_idxs)
                for dt_id in dtrows:
                    tref = self.getcell('TIME', dt_id)*86400 # day->sec
                    if gainfield=='':
                        cal_field_idxs = cal_idxs
                    else:
                        if gainfield.upper() == 'NEAREST':
                            from_fields = field_sel.take(numpy.where(time_sel-tref==min(time_sel-tref)))[0]
                        # select caltable row id by SPW, ANT, and gain field
                        cal_field_idxs = cal_idxs.take(numpy.where([fid in from_fields for fid in field_sel])[0])
                    if len(cal_field_idxs)==0:
                        continue
                    # the array, atsys, is in shape of len(cal_field_idxs) x npol unlike the other arrays.
                    atsys = numpy.array([tsys_masked[i][:,start_atmchan:end_atmchan+1].mean(axis=1).data for i in cal_field_idxs])
                    #LOG.trace("cal_field_ids=%s" % cal_field_idxs)
                    #LOG.trace('atsys = %s' % str(atsys))
                    if atsys.shape[0] == 1: #only one Tsys measurement selected
                        self.putcell('TSYS', dt_id, atsys[0,:])
                    else:
                        tsys_time = times.take(cal_field_idxs) #in sec
                        itsys = [ _interpolate(atsys[:,ipol], tsys_time, tref) \
                                 for ipol in range(atsys.shape[-1]) ]
                        self.putcell('TSYS', dt_id, itsys)

    #@memory_profiler.profile
    def _update_flag(self, context, infile):
        """
        Read MS and update online flag status of DataTable.
        Arguments:
            context: pipeline context instance
            infile: the name of MS to transfer flag from
        NOTE this method should be called before applying the other flags.
        """
        LOG.info('Updating online flag for %s'%(os.path.basename(infile)))
        msobj = context.observing_run.get_ms(name=os.path.abspath(infile))
        msidx = None
        for i in xrange(len(context.observing_run.measurement_sets)):
            if msobj == context.observing_run.measurement_sets[i]:
                msidx = i
                break
        LOG.info('MS idx=%d'%(msidx))
            
        # back to previous impl. with reduced memory usage
        # (performance degraded)
        ms_ids = self.getcol('MS')
        dt_rows = numpy.where(ms_ids == msidx)[0]
        del ms_ids
        rows = self.getcol('ROW')
        ms_rows = rows[dt_rows] 
        with casatools.TableReader(infile) as tb:
            #for dt_row in index[0]:
            for dt_row, ms_row in itertools.izip(dt_rows, ms_rows):
                #ms_row = rows[dt_row]
                flag = tb.getcell('FLAG', ms_row)
                rowflag = tb.getcell('FLAG_ROW', ms_row)
                #irow += 1
                npol = flag.shape[0]
                online_flag = numpy.empty((npol, 1,), dtype=numpy.int32)
                if rowflag == True:
                    online_flag[:] = 0
                else:
                    for ipol in range(npol):
                        online_flag[ipol,0] = 0 if flag[ipol].all() else 1
                self.putcellslice('FLAG_PERMANENT', int(dt_row), online_flag,
                                  blc=[0, OnlineFlagIndex], trc=[npol-1, OnlineFlagIndex], 
                                  incr=[1,1])

class RODataTableColumn( object ):
    def __init__( self, table, name, dtype ):
        self.tb = table
        self.name = name
        self.caster_get = dtype

    def __repr__(self):
        return '%s("%s","%s")'%(self.__class__.__name__,self.name,self.caster_get)

    def getcell( self, idx ):
        return self.tb.getcell(self.name, idx)

    def getcol( self, startrow=0, nrow=-1, rowincr=1 ):
        return self.tb.getcol(self.name, startrow, nrow, rowincr)
    
    def getcellslice(self, rownr, blc, trc, incr):
        return self.tb.getcellslice(self.name, rownr, blc, trc, incr)
    
    def getcolslice(self, blc, trc, incr, startrow=0, nrow=-1, rowincr=1):
        return self.tb.getcolslice(self.name, blc, trc, incr, startrow, nrow, rowincr)

    def putcell( self, idx, val ):
        self.__raise()

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        self.__raise()

    def putcellslice(self, rownr, value, blc, trc, incr):
        self.__raise()
    
    def putcolslice(self, value, blc, trc, incr, startrow=0, nrow=-1, rowincr=1):
        self.__raise()

    def __raise( self ):
        raise NotImplementedError( 'column %s is read-only'%(self.name) )
    
class RWDataTableColumn( RODataTableColumn ):
    def __init__( self, table, name, dtype ):
        super(RWDataTableColumn,self).__init__(table, name, dtype)
        if dtype == list:
            self.caster_put = numpy.asarray
        else:
            self.caster_put = dtype

    def putcell( self, idx, val ):
        self.tb.putcell(self.name, int(idx), self.caster_put(val))

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        self.tb.putcol(self.name,numpy.asarray(val),int(startrow),int(nrow),int(rowincr))
        
    def putcellslice(self, rownr, value, blc, trc, incr):
        return self.tb.putcellslice(self.name, rownr, value, blc, trc, incr)
    
    def putcolslice(self, value, blc, trc, incr, startrow=0, nrow=-1, rowincr=1):
        return self.tb.putcolslice(self.name, value, blc, trc, incr, startrow, nrow, rowincr)

class DataTableColumnNoChange( RWDataTableColumn ):
    def __init__( self, table):
        super(RWDataTableColumn,self).__init__(table, "NOCHANGE", int)

    def putcell( self, idx, val ):
        if type(val)==bool:
            v = -1
        else:
            v = val
        self.tb.putcell(self.name, int(idx), int(v))

class DataTableColumnMaskList( RWDataTableColumn ):
    NoMask = numpy.zeros((1,2), dtype=numpy.int32) - 1#[[-1,-1]]

    def __init__( self, table ):
        super(RWDataTableColumn,self).__init__(table, "MASKLIST", list)

    def getcell( self, idx ):
        v = self.tb.getcell(self.name, int(idx))
        if sum(v[0]) < 0:
            return numpy.zeros(0, dtype=numpy.int32)
        else:
            return v

    def getcol( self, startrow=0, nrow=-1, rowincr=1 ):
        """
        Note: returned array has shape (nrow,nmask), in  
              contrast to (nmask,nrow) for return value of 
              tb.getcol().
        """
        if nrow < 0:
            nrow = self.tb.nrows()
        ret = collections.defaultdict(list)
        idx=0
        for i in xrange(startrow,nrow,rowincr):
            tMASKLIST = self.getcell(i)
            if len(tMASKLIST)==1 and tMASKLIST[0][0]==0 and \
                   tMASKLIST[0][1]==0:
                ret[idx] = tMASKLIST
            idx += 1
        return ret

    def putcell( self, idx, val ):
        if len(val)==0:
            v = self.NoMask
        else:
            v = val
        self.tb.putcell(self.name, int(idx), numpy.asarray(v))

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        """
        Note: input array should have shape (nrow,nmask), in  
              contrast to (nmask,nrow) for tb.putcol()
        """
        if nrow < 0:
            nrow = min(startrow+len(val)*rowincr,self.tb.nrows())
        idx = 0
        for i in xrange(startrow,nrow,rowincr):
            self.putcell(i,numpy.asarray(val[idx]))
            idx += 1

def _interpolate(v, t, tref):
    n = len(t)
    idx = -1
    for i in xrange(n):
        if t[i] >= tref:
            break
        idx += 1
    if idx < 0:
        return v[0]
    elif idx >= n-1:
        return v[-1]
    else:
        t1 = t[idx+1] - tref
        t0 = tref - t[idx]
        return (v[idx+1] * t0 + v[idx] * t1) / (t[idx+1] - t[idx]) 

def construct_timegroup(rows, group_id_list, group_association_list):
    timetable_dict = dict(map(lambda x: (x, [[],[]]), group_id_list))
    for (idx, group_id) in enumerate(group_association_list):
        if group_id not in group_id_list:
            continue
        timetable_dict[group_id][0].append(rows[idx])
        timetable_dict[group_id][1].append(idx)
    return timetable_dict  

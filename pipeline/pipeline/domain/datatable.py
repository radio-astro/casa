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
#    (valueType,option,ndim,maxlen,comment[,unit])
#
# dataManagerGroup and dataManagerType is always 'StandardStMan'.
TD_DESC_RO = [
#    __coldesc('integer', 0, 0, -1, 'Primary key'),
    __coldesc('integer', 0, 0, -1, 'Row number'),
    __coldesc('integer', 0, 0, -1, 'Scan number'),
    __coldesc('integer', 0, 0, -1, 'IF number'),
    __coldesc('integer', 0, 0, -1, 'Pol number'),
    __coldesc('integer', 0, 0, -1, 'Beam number'),
    __coldesc('string',  0, 0, -1, 'Date'),
    __coldesc('double',  0, 0, -1, 'Time in MJD', 's'),
    __coldesc('double',  0, 0, -1, 'Elapsed time since first scan' 's'),
    __coldesc('double',  0, 0, -1, 'Exposure time', 's'),
    __coldesc('double',  0, 0, -1, 'Right Ascension', 'deg'),
    __coldesc('double',  0, 0, -1, 'Declination', 'deg'),
    __coldesc('double',  0, 0, -1, 'Azimuth', 'deg'),
    __coldesc('double',  0, 0, -1, 'Elevation', 'deg'),
    __coldesc('integer', 0, 0, -1, 'Number of channels'),
    __coldesc('double',  0, 0, -1, 'Tsys', 'K'),
    __coldesc('string',  0, 0, -1, 'Target name'),
    __coldesc('integer', 0, 0, -1, 'Antenna index'),
    __coldesc('integer', 0, 0, -1, 'Source type enum'),
    __coldesc('integer', 0, 0, -1, 'MS index'),
    __coldesc('integer', 0, 0, -1, 'Field ID')
    ]

TD_DESC_RW = [
    __coldesc('double',  0, 0,  1, 'Statistics'),
    __coldesc('integer', 0, 0,  1, 'Flgas'),
    __coldesc('integer', 0, 0,  1, 'Permanent flags'),
    __coldesc('integer', 0, 0, -1, 'Actual flag'),
    __coldesc('integer', 0, 0, -1, 'Number of mask regions'),
    __coldesc('integer', 0, 0,  2, 'List of mask ranges'),
    __coldesc('integer', 0, 0, -1, 'Unchanged row or not'),
    __coldesc('integer', 0, 0, -1, 'Position group id')#,
    ]

def __tabledescro():
    name = [
        'ROW', 'SCAN', 'IF', 'POL', 'BEAM', 'DATE',
        'TIME', 'ELAPSED', 'EXPOSURE', 'RA', 'DEC',
        'AZ', 'EL', 'NCHAN', 'TSYS', 'TARGET', 'ANTENNA',
        'SRCTYPE', 'MS', 'FIELD_ID'
        ]
    return dict( zip(name,TD_DESC_RO) )

def __tabledescrw():
    name = [
        'STATISTICS', 'FLAG', 'FLAG_PERMANENT',
        'FLAG_SUMMARY', 'NMASK', 'MASKLIST', 'NOCHANGE',
        'POSGRP']
    return dict( zip(name,TD_DESC_RW) )

TABLEDESC_RO = __tabledescro()
TABLEDESC_RW = __tabledescrw()

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
            self.importdata(name=name, minimal=False, readonly=readonly)

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

    def get_row_index(self, antenna, ifno, polno=None):
        ants = self.getcol('ANTENNA')
        ifs = self.getcol('IF')
        if polno is None:
            ref = [antenna, ifno]
            return [i for i in xrange(self.nrow) if [ants[i], ifs[i]] == ref]
        else:
            pols = self.getcol('POL')
            ref = [antenna, ifno, polno]
            return [i for i in xrange(self.nrow) if [ants[i], ifs[i], pols[i]] == ref]

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
        self.tb1.create( tablename=self.memtable1,
                         tabledesc=TABLEDESC_RO,
                         memtype='memory',
                         nrow=self.nrow )
        self.tb2.create( tablename=self.memtable2,
                         tabledesc=TABLEDESC_RW,
                         memtype='memory',
                         nrow=self.nrow )
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
        dtype = lambda desc: list if desc.has_key('ndim') and desc['ndim'] > 0 \
                                  else type_map[desc['valueType']]
        for (k,v) in TABLEDESC_RO.items():
            self.cols[k] = RO_COLUMN(self.tb1,k,dtype(v))
        for (k,v) in TABLEDESC_RW.items():
            if k == 'MASKLIST':
                self.cols[k] = DataTableColumnMaskList(self.tb2)
            elif k == 'NOCHANGE':
                self.cols[k] = DataTableColumnNoChange(self.tb2)
            else:
                self.cols[k] = RW_COLUMN(self.tb2,k,dtype(v))

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
        for (k,v) in posgrp_rep.items():
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
        LOG.info('new get_timetable start')
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
        LOG.info('new get_timetable end: Elapsed time %s sec'%(end_time - start_time))
            
        return timetable
        
            
    def get_timegap(self, ant, spw, pol, asrow=True):
        timegap_s = self.getkeyword('TIMEGAP_S')
        timegap_l = self.getkeyword('TIMEGAP_L')
        try:
            mygap_s = timegap_s[str(ant)][str(spw)][str(pol)]
            mygap_l = timegap_l[str(ant)][str(spw)][str(pol)]
        except KeyError, e:
            raise KeyError('ant %s spw %s pol %s not in reduction group list'%(ant,spw,pol))
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
    
    def _update_tsys(self,context, infile, tsystable, ifmap):
        with casatools.TableReader(tsystable) as tb:
            ifnos = tb.getcol('IFNO')
            polnos = tb.getcol('POLNO')
            times = tb.getcol('TIME')
            tsys = {}
            for i in xrange(tb.nrows()):
                tsys[i] = tb.getcell('TSYS',i)
        
        if_from = ifmap.keys()
        pollist = numpy.unique(polnos)
        file_list = self.getkeyword('FILENAMES').tolist()
        ant = file_list.index(os.path.basename(infile.rstrip('/')))

        spectral_windows = context.observing_run[ant].spectral_window
                
        for ifno_from in if_from:
            for polno in pollist:
                indices = numpy.where(numpy.logical_and(ifnos==ifno_from,polnos==polno))[0]
                if len(indices) == 0:
                    continue
                
                for ifno_to in ifmap[ifno_from]:
                    # 2015/07/22 TN
                    # Averaged Tsys is always applied due to change in CAS-7653
                    #start_chan, end_chan = map_spwchans(spectral_windows[ifno_from], 
                    #                                    spectral_windows[ifno_to])
                    #atsys = [numpy.mean(tsys[i][start_chan:end_chan])
                    atsys = [tsys[i][0]
                             for i in indices]
                    LOG.debug('atsys = %s' % atsys)
                    rows = self.get_row_index(ant, ifno_to, polno)
                    if len(atsys) == 1:
                        for row in rows:
                            self.tb1.putcell('TSYS', row, atsys[0])
                    else:
                        tsys_time = times.take(indices)
                        for row in rows:
                            tref = self.tb1.getcell('TIME', row)
                            itsys = _interpolate(atsys, tsys_time, tref)
                            self.tb1.putcell('TSYS', row, itsys)

    def _update_flag(self, context, infile):
        LOG.info('Updating online flag for %s'%(os.path.basename(infile)))
        antenna = context.observing_run.st_names.index(os.path.basename(infile))
        LOG.info('antenna=%s'%(antenna))
        antennas = self.tb1.getcol('ANTENNA')
        rows = self.tb1.getcol('ROW')
        flag_permanent = self.tb2.getcol('FLAG_PERMANENT')
        online_flag = flag_permanent[OnlineFlagIndex,:]           
        index = numpy.where(antennas == antenna)
        myrows = rows[index]
        with casatools.TableReader(infile) as tb:
            #for i in index:
            #    row = rows[i]
            #    LOG.info('index %s row %s'%(i,row))
            for i in index[0]:
                row = rows[i]
                flagrow = tb.getcell('FLAGROW', row)
                flag = tb.getcell('FLAGTRA', row)
                online_flag[i] = (flagrow == 0 and any(flag == 0))
        self.tb2.putcol('FLAG_PERMANENT', flag_permanent)
        
def map_spwchans(atm_spw, science_spw):
    atm_nchan = atm_spw.nchan
    atm_freq_min, atm_freq_max, atm_incr = atm_spw.freq_min, atm_spw.freq_max, atm_spw.increment
    science_freq_min, science_freq_max = atm_spw.freq_min, atm_spw.freq_max
    if atm_incr < 0: # LSB
        get_channel = lambda ref, freq, incr, offset: int(math.floor((ref - freq)/abs(incr) + offset))
        LOG.trace('--- LSB ----')
        start_atmchan = max(0, get_channel(atm_freq_max, science_freq_max, atm_incr, 0.5))
        end_atmchan = min(atm_nchan, get_channel(atm_freq_max, science_freq_min, atm_incr, -0.5) + 1)
    elif(atm_incr > 0): # USB
        get_channel = lambda ref, freq, incr, offset: math.floor((freq - ref)/abs(incr) + offset)
        LOG.trace('--- USB ----')
        start_atmchan = max(0, get_channel(atm_freq_min, science_freq_min, atm_incr, 0.5))
        end_atmchan = min(atm_nchan, get_channel(atm_freq_min, science_freq_max, atm_incr, -0.5) + 1)
    else:
        raise RuntimeError, 'error: atm_increment should not be 0'
    
    LOG.trace('calculate_average_tsys:   satrt_atmchan == %d' % start_atmchan)
    LOG.trace('calculate_average_tsys:   end_atmchan == %d' % end_atmchan)

    if end_atmchan == start_atmchan:
        end_atmchan = start_atmchan + 1
    
    return start_atmchan, end_atmchan
    
# def calculate_average_tsys(atm_nchan,atm_freq_min,atm_freq_max,target_freq_min,target_freq_max,atm_increment,tsyslist) :
#     
#     if atm_increment < 0: # LSB
#         get_channel = lambda ref, freq, incr, offset: int(math.floor((ref - freq)/abs(incr) + offset))
#         LOG.trace('--- LSB ----')
#         if atm_freq_min < target_freq_min and target_freq_max < atm_freq_max:
#             # start_atmchan
#             start_atmchan = get_channel(atm_freq_max, target_freq_max, atm_increment, 0.5)
#             LOG.trace('calculate_average_tsys:   satrt_atmchan == %d' % start_atmchan)
#                 
#             # end_atmchan
#             end_atmchan = get_channel(atm_freq_max, target_freq_min, atm_increment, -0.5)
#             LOG.trace('calculate_average_tsys:   end_atmchan == %d' % end_atmchan) + 1
#         else:
#             start_atmchan = 0
#             end_atmchan = atm_nchan
#         
#     elif(atm_increment > 0): # USB)
#         get_channel = lambda ref, freq, incr, offset: math.floor((freq - ref)/abs(incr) + offset)
#         LOG.trace('--- USB ----')
#         if(atm_freq_min < target_freq_min and target_freq_max < atm_freq_max):
#             # start_atmchan
#             start_atmchan = get_channel(atm_freq_min, target_freq_min, atm_increment, 0.5)
#             LOG.trace('calculate_average_tsys:   satrt_atmchan == %d' % start_atmchan)
#                 
#             # end_atmchan
#             end_atmchan = get_channel(atm_freq_min, target_freq_max, atm_increment, -0.5) + 1
#             LOG.trace('calculate_average_tsys:   end_atmchan == %d' % end_atmchan)
#         else:
#             start_atmchan = 0
#             end_atmchan = atm_nchan
#     else:
#         raise RuntimeError, 'error: atm_increment should not be 0'
#         
#     LOG.debug('calculate_average_tsys:   end_atmchan - start_atmchan = %d ' % (end_atmchan - start_atmchan))
#     if start_atmchan - end_atmchan > 1:
#         averaged_tsys = numpy.mean(tsyslist[start_atmchan:end_atmchan])
#     else:
#         averaged_tsys = numpy.mean(tsyslist)
#     return averaged_tsys


class RODataTableColumn( object ):
    def __init__( self, table, name, dtype ):
        self.tb = table
        self.name = name
        self.caster_get = dtype

    def __repr__(self):
        return '%s("%s","%s")'%(self.__class__.__name__,self.name,self.caster_get)

    def getcell( self, idx ):
        return self.caster_get(self.tb.getcell(self.name, idx))

    def getcol( self, startrow=0, nrow=-1, rowincr=1 ):
        return self.tb.getcol(self.name, startrow, nrow, rowincr).tolist()

    def putcell( self, idx, val ):
        self.__raise()

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        self.__raise()

    def __raise( self ):
        raise NotImplementedError( 'column %s is read-only'%(self.name) )
    
class RWDataTableColumn( RODataTableColumn ):
    def __init__( self, table, name, dtype ):
        super(RWDataTableColumn,self).__init__(table, name, dtype)
        if dtype == list:
            self.caster_put = numpy.array
        else:
            self.caster_put = dtype

    def putcell( self, idx, val ):
        self.tb.putcell(self.name, int(idx), self.caster_put(val))

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        self.tb.putcol(self.name,numpy.array(val),int(startrow),int(nrow),int(rowincr))
        
class DataTableColumnNoChange( RWDataTableColumn ):
    def __init__( self, table):
        super(RWDataTableColumn,self).__init__(table, "NOCHANGE", int)

    def getcell( self, idx ):
        v = self.tb.getcell(self.name, int(idx))
        if v < 0:
            return False
        else:
            return int(v)

    def getcol( self, startrow=0, nrow=-1, rowincr=1 ):
        if nrow < 0:
            nrow = self.tb.nrows()
        ret = [0 for i in xrange(startrow,nrow,rowincr)]
        idx = 0
        for i in xrange(startrow,nrow,rowincr):
            tNOCHANGE = self.tb.getcell(self.name,i)
            ret[idx] = False if tNOCHANGE < 0 else tNOCHANGE
            idx += 1
        return ret

    def putcell( self, idx, val ):
        if type(val)==bool:
            v = -1
        else:
            v = val
        self.tb.putcell(self.name, int(idx), int(v))

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        if nrow < 0:
            nrow = min(startrow+len(val)*rowincr,self.tb.nrows())
        idx = 0
        for i in xrange(startrow,nrow,rowincr):
            self.putcell(i,val[idx])
            idx += 1

class DataTableColumnMaskList( RWDataTableColumn ):
    NoMask = [[-1,-1]]

    def __init__( self, table ):
        super(RWDataTableColumn,self).__init__(table, "MASKLIST", list)

    def getcell( self, idx ):
        v = self.tb.getcell(self.name, int(idx))
        if sum(v[0]) < 0:
            return []
        else:
            return v.tolist() 

    def getcol( self, startrow=0, nrow=-1, rowincr=1 ):
        """
        Note: returned array has shape (nrow,nmask), in  
              contrast to (nmask,nrow) for return value of 
              tb.getcol().
        """
        if nrow < 0:
            nrow = self.tb.nrows()
        ret = [[] for i in xrange(startrow,nrow,rowincr)]
        idx=0
        for i in xrange(startrow,nrow,rowincr):
            tMASKLIST = list(self.getcell(i))
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
        self.tb.putcell(self.name, int(idx), numpy.array(v))

    def putcol( self, val, startrow=0, nrow=-1, rowincr=1 ):
        """
        Note: input array should have shape (nrow,nmask), in  
              contrast to (nmask,nrow) for tb.putcol()
        """
        if nrow < 0:
            nrow = min(startrow+len(val)*rowincr,self.tb.nrows())
        idx = 0
        for i in xrange(startrow,nrow,rowincr):
            self.putcell(i,numpy.array(val[idx]))
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

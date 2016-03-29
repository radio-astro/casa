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
from pipeline.domain.datatable import absolute_path

from ..common import science_spw, mjd_to_datestring, TableSelector

LOG = infrastructure.get_logger(__name__)

def get_value_in_deg(quantity):
    qa = casatools.quanta
    return qa.getvalue(qa.convert(quantity, 'deg'))

class MetaDataReader(object):
    def __init__(self, ms, table_name):
        """
        mses -- list of measurementset domain objects
        table_name -- name of DataTable
        """
        self.ms = ms
        self.table_name = table_name
        self.datatable = DataTable(name=self.table_name, readonly=False)
        self.vAnt = 0
        self.appended_row = 0
        
    @property
    def name(self):
        return self.ms.name
        
    def get_datatable(self):
        return self.datatable

    def detect_target_spw(self):
        if not hasattr(self, 'name'):
            return []
        
        ms = self.ms
        spws = ms.get_spectral_windows(science_windows_only=True)
        return map(lambda x: x.id, spws)
    
    def detect_target_data_desc(self):
        science_windows = self.detect_target_spw()
        ms = self.ms
        def _g():
            for spwid in science_windows:
                dd = ms.get_data_description(spw=spwid)
                assert dd is not None
                yield dd.id
        dds = numpy.fromiter(_g(), dtype=numpy.int32)
        return dds
                
    def execute(self, dry_run=True):
        if dry_run:
            return
        
        # name of the MS
        name = self.name
        spwids = self.detect_target_spw()
        nchan_map = dict([(spwid,self.ms.get_spectral_window(spwid).num_channels) for spwid in spwids])
        ddids = self.detect_target_data_desc()
        #exclude_types = list(self.detect_exclude_type())
        
        #Rad2Deg = 180. / 3.141592653
        
        LOG.info('name=%s'%(name))
        if self.datatable.has_key('FILENAMES'):
            filenames = self.datatable.getkeyword('FILENAMES')
            if name in filenames:
                # the data is already registered, return
                self.appended_row = 0
                return
            filenames = numpy.concatenate((filenames,[name]))
        else:
            filenames = [name]
        ms_id = len(filenames) - 1
        self.datatable.putkeyword('FILENAMES',filenames)


        ms = self.ms
        assert ms is not None
        spwsel = ','.join(map(str, spwids))
        target = 'TARGET'
        assert target in ms.intents
        states = [s for s in ms.states if target in s.intents]
        obs_modes = set()
        for s in states:
            modes = set(s.get_obs_mode_for_intent(target))
            obs_modes.update(modes)
        state_ids = set()
        with casatools.MSReader(name) as msreader:
            for obs_mode in obs_modes:
                msreader.msselect({'spw': spwsel, 'scanintent': obs_mode}, onlyparse=True)
                indices = msreader.msselectedindices()
                state_ids.update(indices['stateid'])
        state_ids = numpy.fromiter(state_ids, dtype=numpy.int32)
        
        with TableSelector(name, 'ANTENNA1 == ANTENNA2 && FEED1 == FEED2 && DATA_DESC_ID IN %s && STATE_ID IN %s'%(list(ddids), list(state_ids))) as tb:
            nrow = tb.nrows()
            rows = tb.rownumbers()
            Texpt = tb.getcol('INTERVAL')
            Tmjd = tb.getcol('TIME')
            time_meas = tb.getcolkeyword('TIME', 'MEASINFO')
            time_frame = time_meas['Ref']
            Tscan = tb.getcol('SCAN_NUMBER')
            TDD = tb.getcol('DATA_DESC_ID')
            ddspwmap = numpy.vectorize(lambda x: ms.get_data_description(id=x).spw.id, otypes=[numpy.int32])
            Tif = ddspwmap(TDD)
            Tpol = numpy.zeros(nrow, dtype=numpy.int32)
            Tant = tb.getcol('ANTENNA1')
            Tbeam = tb.getcol('FEED1')
            Tsrctype = Tpol.copy()
            Tflagrow = tb.getcol('FLAG_ROW')
            field_ids = tb.getcol('FIELD_ID')
            getsourcename = numpy.vectorize(lambda x: ms.get_fields(x)[0].source.name, otypes=['string'])
            Tsrc = getsourcename(field_ids)
            # set dummy value to TSYS 
            # since it will be overwritten in hsd_applycal stage
            Tsys = numpy.ones(nrow, dtype=numpy.float32)
            NchanArray = numpy.fromiter((nchan_map[n] for n in Tif), dtype=numpy.int)   

        ID = len(self.datatable)
        LOG.info('ID=%s'%(ID))
        #ROWs = []
        #IDs = []

        self.datatable.addrows( nrow )
        # column based storing
        self.datatable.putcol('ROW',rows,startrow=ID)
        self.datatable.putcol('SCAN',Tscan,startrow=ID)
        self.datatable.putcol('IF',Tif,startrow=ID)
        self.datatable.putcol('POL',Tpol,startrow=ID)
        self.datatable.putcol('BEAM',Tbeam,startrow=ID)
        self.datatable.putcol('TIME',Tmjd/86400.0,startrow=ID)
        self.datatable.putcol('ELAPSED',Tmjd-Tmjd[0],startrow=ID)
        self.datatable.putcol('EXPOSURE',Texpt,startrow=ID)
        Tra = numpy.zeros(nrow, dtype=numpy.float64)
        Tdec = numpy.zeros(nrow, dtype=numpy.float64)
        Taz = numpy.zeros(nrow, dtype=numpy.float64)
        Tel = numpy.zeros(nrow, dtype=numpy.float64)
        index = numpy.argsort(Tant)
        LOG.info('Start reading direction (convert if necessary). It may take a while.')
        with casatools.MSMDReader(name) as msmd:
            nprogress = 5000
            iprogress = 0
            for irow in index:
                iprogress += 1
                if iprogress >= nprogress and iprogress % nprogress == 0:
                    print '%s/%s'%(iprogress,nrow)
                row = rows[irow]
                mjd_in_sec = Tmjd[irow]
                me = casatools.measures
                qa = casatools.quanta
                mepoch = me.epoch(rf=time_frame, v0=qa.quantity(mjd_in_sec, 's'))
                antenna_id = Tant[irow]
                antennas = self.ms.get_antenna(antenna_id)
                assert len(antennas) == 1
                antenna_domain = antennas[0]
                mposition = antenna_domain.position
                pointing_directions = msmd.pointingdirection(row)
                pointing_direction = pointing_directions['antenna1']['pointingdirection'] # antenna2 should be the same
                lon = pointing_direction['m0']
                lat = pointing_direction['m1']
                ref = pointing_direction['refer']
                if ref in ['AZEL', 'AZELGEO']:
                    if irow == 0:
                        LOG.info('Require direction conversion from %s to J2000'%(ref))
                        
                    Taz[irow] = get_value_in_deg(lon)
                    Tel[irow] = get_value_in_deg(lat)
                    
                    # conversion to J2000
                    ra, dec = self.direction_convert(pointing_direction, mepoch, mposition, outframe='J2000')                    
                    Tra[irow] = get_value_in_deg(ra)
                    Tdec[irow] = get_value_in_deg(dec)
                elif ref in ['J2000']:
                    if irow == 0:
                        LOG.info('Require direction conversion from %s to AZELGEO'%(ref))
                        
                    Tra[irow] = get_value_in_deg(lon)
                    Tdec[irow] = get_value_in_deg(lat)
                    
                    # conversion to AZELGEO
                    az, el = self.direction_convert(pointing_direction, mepoch, mposition, outframe='AZELGEO')                  
                    Taz[irow] = get_value_in_deg(az)
                    Tel[irow] = get_value_in_deg(el)
                else:
                    if irow == 0:
                        LOG.info('Require direction conversion from %s to J2000 as well as to AZELGEO'%(ref))
                        
                    # conversion to J2000
                    ra, dec = self.direction_convert(pointing_direction, mepoch, mposition, outframe='J2000')
                    Tra[irow] = get_value_in_deg(ra)
                    Tdec[irow] = get_value_in_deg(dec)

                    # conversion to AZELGEO
                    az, el = self.direction_convert(pointing_direction, mepoch, mposition, outframe='AZELGEO')
                    Taz[irow] = get_value_in_deg(az)
                    Tel[irow] = get_value_in_deg(el)
        LOG.info('Done reading direction (convert if necessary).')
        self.datatable.putcol('RA',Tra,startrow=ID)
        self.datatable.putcol('DEC',Tdec,startrow=ID)
        self.datatable.putcol('AZ',Taz,startrow=ID)
        self.datatable.putcol('EL',Tel,startrow=ID)
        self.datatable.putcol('NCHAN',NchanArray,startrow=ID)
        self.datatable.putcol('TSYS',Tsys,startrow=ID)
        self.datatable.putcol('TARGET',Tsrc,startrow=ID)
        intArr = numpy.ones(nrow, dtype=int)
        self.datatable.putcol('FLAG_SUMMARY',intArr,startrow=ID)
        intArr[:] = 0
        self.datatable.putcol('NMASK',intArr,startrow=ID)
        intArr[:] = -1
        self.datatable.putcol('NOCHANGE',intArr,startrow=ID)
        self.datatable.putcol('POSGRP',intArr,startrow=ID)
        #Tant += self.vAnt
        self.datatable.putcol('ANTENNA',Tant,startrow=ID)
        self.datatable.putcol('SRCTYPE',Tsrctype,startrow=ID)
        intArr[:] = ms_id
        self.datatable.putcol('MS', intArr, startrow=ID)
        
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

        num_antenna = len(self.ms.antennas)
        self.vAnt += num_antenna
        
        self.appended_row = nrow

    def detect_exclude_type(self):
        return map(int, [sd.srctype.poncal, sd.srctype.poffcal])
    
    def direction_convert(self, direction, mepoch, mposition, outframe):
        direction_type = direction['type']
        assert direction_type == 'direction'
        inframe = direction['refer']
        
        # if outframe is same as input direction reference, just return 
        # direction as it is
        if outframe == inframe:
            return direction
        
        # conversion using measures tool
        me = casatools.measures
        me.doframe(mepoch)
        me.doframe(mposition)
        out_direction = me.measure(direction, outframe)
        return out_direction['m0'], out_direction['m1']
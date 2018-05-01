from __future__ import absolute_import

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.domain.datatable import DataTableImpl as DataTable
from pipeline.domain.datatable import DataTableColumnMaskList as ColMaskList
from pipeline.domain.datatable import OnlineFlagIndex

from ..common import mjd_to_datestring, TableSelector

LOG = infrastructure.get_logger(__name__)


def get_value_in_deg(quantity):
    qa = casatools.quanta
    return qa.getvalue(qa.convert(quantity, 'deg'))


def get_state_id(ms, spw, intent):
    states = (s for s in ms.states if intent in s.intents)
    obs_modes = set()
    for s in states:
        modes = set(s.get_obs_mode_for_intent(intent))
        obs_modes.update(modes)
    state_ids = set()
    with casatools.MSReader(ms.name) as msreader:
        for obs_mode in obs_modes:
            msreader.msselect({'spw': spw, 'scanintent': obs_mode}, onlyparse=True)
            indices = msreader.msselectedindices()
            state_ids.update(indices['stateid'])
            # Need to reset explicitly after CASA 5.3. See CAS-11088 for detail.
            msreader.selectinit(reset=True)
    return numpy.fromiter(state_ids, dtype=numpy.int32)
        

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
        nchan_map = dict([(spwid, self.ms.get_spectral_window(spwid).num_channels) for spwid in spwids])
        ddids = self.detect_target_data_desc()
        
        #Rad2Deg = 180. / 3.141592653
        
        LOG.info('name=%s' % name)
        if self.datatable.haskeyword('FILENAMES'):
            filenames = self.datatable.getkeyword('FILENAMES')
            if name in filenames:
                # the data is already registered, return
                self.appended_row = 0
                return
            filenames = numpy.concatenate((filenames, [name]))
        else:
            filenames = [name]
        ms_id = len(filenames) - 1
        self.datatable.putkeyword('FILENAMES', filenames)

        # 2018/04/18 TN
        # CAS-10874 single dish pipeline should use ICRS instead of J2000
        # For consistency throuout the single dish pipeline, direction 
        # reference frame is stored in the DataTable 
        outref = self._get_outref()
        # register direction reference to datatable
        self.datatable.direction_ref = outref
        azelref = self._get_azelref()
        #LOG.info('outref="{0}" azelref="{1}"'.format(outref, azelref))

        ms = self.ms
        assert ms is not None
        spwsel = ','.join(map(str, spwids))
        target = 'TARGET'
        reference = 'REFERENCE'
        assert target in ms.intents
        #assert reference in ms.intents
        target_states = get_state_id(ms, spwsel, target)
        reference_states = get_state_id(ms, spwsel, reference)
#         target_states = (s for s in ms.states if target in s.intents)
#         reference_states = (s for s in ms.states if reference in s.intents)
#         target_obs_modes = set()
#         for s in target_states:
#             modes = set(s.get_obs_mode_for_intent(target))
#             target_obs_modes.update(modes)
#             modes = set(s.get_obs_mode_for_intent(reference))
#             obs_modes.update(modes)
#         state_ids = set()
#         with casatools.MSReader(name) as msreader:
#             for obs_mode in obs_modes:
#                 msreader.msselect({'spw': spwsel, 'scanintent': obs_mode}, onlyparse=True)
#                 indices = msreader.msselectedindices()
#                 state_ids.update(indices['stateid'])
#         state_ids = numpy.fromiter(state_ids, dtype=numpy.int32)
        state_ids = numpy.concatenate([target_states, reference_states])
        
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
            ddnpolmap = numpy.vectorize(lambda x: ms.get_data_description(id=x).num_polarizations, otypes=[numpy.int32])
            Tif = ddspwmap(TDD)
            Tpol = ddnpolmap(TDD)
            Tant = tb.getcol('ANTENNA1')
            Tbeam = tb.getcol('FEED1')
            Tsrctype = numpy.fromiter((0 if i in target_states else 1 for i in tb.getcol('STATE_ID')), dtype=numpy.int32)
            Tflagrow = tb.getcol('FLAG_ROW')
            Tflag = numpy.fromiter((numpy.all(tb.getcell('FLAG', i) == True) for i in xrange(nrow)), dtype=bool)
            Tflagrow = numpy.logical_or(Tflagrow, Tflag)
            field_ids = tb.getcol('FIELD_ID')
            getsourcename = numpy.vectorize(lambda x: ms.get_fields(x)[0].source.name, otypes=['string'])
            Tsrc = getsourcename(field_ids)
            NchanArray = numpy.fromiter((nchan_map[n] for n in Tif), dtype=numpy.int)   

        ID = len(self.datatable)
        LOG.info('ID=%s' % ID)
        #ROWs = []
        #IDs = []

        self.datatable.addrows(nrow)
        # column based storing
        self.datatable.putcol('ROW', rows, startrow=ID)
        self.datatable.putcol('SCAN', Tscan, startrow=ID)
        self.datatable.putcol('IF', Tif, startrow=ID)
        self.datatable.putcol('NPOL', Tpol, startrow=ID)
        self.datatable.putcol('BEAM', Tbeam, startrow=ID)
        self.datatable.putcol('TIME', Tmjd/86400.0, startrow=ID)
        self.datatable.putcol('ELAPSED', Tmjd-Tmjd[0], startrow=ID)
        self.datatable.putcol('EXPOSURE', Texpt, startrow=ID)
        self.datatable.putcol('FIELD_ID', field_ids, startrow=ID)
        Tra = numpy.zeros(nrow, dtype=numpy.float64)
        Tdec = numpy.zeros(nrow, dtype=numpy.float64)
        Taz = numpy.zeros(nrow, dtype=numpy.float64)
        Tel = numpy.zeros(nrow, dtype=numpy.float64)
        index = numpy.lexsort((Tant, Tmjd))
        LOG.info('Start reading direction (convert if necessary). It may take a while.')
        with casatools.MSMDReader(name) as msmd:
            nprogress = 5000
            iprogress = 0
            last_mjd = None
            last_antenna = None
            last_result = None
            for irow in index:
                iprogress += 1
                if iprogress >= nprogress and iprogress % nprogress == 0:
                    print('{}/{}'.format(iprogress, nrow))
                row = rows[irow]
                mjd_in_sec = Tmjd[irow]
                antenna_id = Tant[irow]
                if mjd_in_sec == last_mjd and antenna_id == last_antenna:
                    Taz[irow] = last_result[0]
                    Tel[irow] = last_result[1]
                    Tra[irow] = last_result[2]
                    Tdec[irow] = last_result[3]
                    continue
                me = casatools.measures
                qa = casatools.quanta
                mepoch = me.epoch(rf=time_frame, v0=qa.quantity(mjd_in_sec, 's'))
                antennas = self.ms.get_antenna(antenna_id)
                assert len(antennas) == 1
                antenna_domain = antennas[0]
                mposition = antenna_domain.position
                pointing_directions = msmd.pointingdirection(row, interpolate=True)
                pointing_direction = pointing_directions['antenna1']['pointingdirection']  # antenna2 should be the same
                lon = pointing_direction['m0']
                lat = pointing_direction['m1']
                ref = pointing_direction['refer']
                # 2018/04/18 TN
                # CAS-10874 single dish pipeline should use ICRS instead of J2000
                if ref in [azelref]:
                    if irow == 0:
                        LOG.info('Require direction conversion from {0} to {1}'.format(ref, outref))
                        
                    Taz[irow] = get_value_in_deg(lon)
                    Tel[irow] = get_value_in_deg(lat)
                    
                    # conversion to J2000
                    ra, dec = direction_convert(pointing_direction, mepoch, mposition, outframe=outref)                    
                    Tra[irow] = get_value_in_deg(ra)
                    Tdec[irow] = get_value_in_deg(dec)
                elif ref in [outref]:
                    if irow == 0:
                        LOG.info('Require direction conversion from {0} to {1}'.format(ref, azelref))
                        
                    Tra[irow] = get_value_in_deg(lon)
                    Tdec[irow] = get_value_in_deg(lat)
                    
                    # conversion to AZELGEO
                    az, el = direction_convert(pointing_direction, mepoch, mposition, outframe=azelref)                  
                    Taz[irow] = get_value_in_deg(az)
                    Tel[irow] = get_value_in_deg(el)
                else:
                    if irow == 0:
                        LOG.info('Require direction conversion from {0} to {1} as well as to {2}'.format(ref, outref,
                                                                                                         azelref))
                        
                    # conversion to J2000
                    ra, dec = direction_convert(pointing_direction, mepoch, mposition, outframe=outref)
                    Tra[irow] = get_value_in_deg(ra)
                    Tdec[irow] = get_value_in_deg(dec)

                    # conversion to AZELGEO
                    az, el = direction_convert(pointing_direction, mepoch, mposition, outframe=azelref)
                    Taz[irow] = get_value_in_deg(az)
                    Tel[irow] = get_value_in_deg(el)
                last_mjd = mjd_in_sec
                last_antenna = antenna_id
                last_result = (Taz[irow], Tel[irow], Tra[irow], Tdec[irow],)
        LOG.info('Done reading direction (convert if necessary).')
        self.datatable.putcol('RA', Tra, startrow=ID)
        self.datatable.putcol('DEC', Tdec, startrow=ID)
        self.datatable.putcol('AZ', Taz, startrow=ID)
        self.datatable.putcol('EL', Tel, startrow=ID)
        self.datatable.putcol('NCHAN', NchanArray, startrow=ID)
        self.datatable.putcol('TARGET', Tsrc, startrow=ID)
        intArr = numpy.zeros(nrow, dtype=int)
        self.datatable.putcol('NMASK', intArr, startrow=ID)
        intArr[:] = -1
        self.datatable.putcol('NOCHANGE', intArr, startrow=ID)
        self.datatable.putcol('POSGRP', intArr, startrow=ID)
        self.datatable.putcol('ANTENNA', Tant, startrow=ID)
        self.datatable.putcol('SRCTYPE', Tsrctype, startrow=ID)
        intArr[:] = ms_id
        self.datatable.putcol('MS', intArr, startrow=ID)
        
        # row base storing
        masklist = ColMaskList.NoMask
        
        # Tsys will be overwritten in applycal stage
        tsys_template = numpy.ones(4, dtype=numpy.float32)
        
        flag_summary_template = numpy.ones(4, dtype=numpy.int32)
        stats_template = numpy.zeros((4, 7), dtype=numpy.int32) - 1
        flags_template = numpy.ones((4, 7), dtype=numpy.int32)
        pflags_template = numpy.ones((4, 4), dtype=numpy.int32)
        for x in xrange(nrow):
            # FLAGROW is mapped into OnlineFlag (PermanentFlag[3])
            # NOTE: data is valid if Tflagrow is 0
            #       data is valid if pflags[3] is 1
            pflags_template[:, OnlineFlagIndex] = 1 if Tflagrow[x] == 0 else 0
            sDate = mjd_to_datestring(Tmjd[x]/86400.0, unit='day')
            self.datatable.putcell('DATE', ID, sDate)
            self.datatable.putcell('MASKLIST', ID, masklist)

            # polarization dependent arrays
            npol = self.datatable.getcell('NPOL', ID)
            self.datatable.putcell('STATISTICS', ID, stats_template[:npol])
            self.datatable.putcell('FLAG', ID, flags_template[:npol])
            self.datatable.putcell('FLAG_PERMANENT', ID, pflags_template[:npol])
            self.datatable.putcell('FLAG_SUMMARY', ID, flag_summary_template[:npol])
            self.datatable.putcell('TSYS', ID, tsys_template[:npol])
            ID += 1

        num_antenna = len(self.ms.antennas)
        self.vAnt += num_antenna
        
        self.appended_row = nrow
        
    def _get_outref(self):
        outref = None
        
        if self.ms.representative_target[0] is not None:
            # if ms has representative target, take reference from that
            LOG.info(
                'Use direction reference for representative target "{0}".'.format(self.ms.representative_target[0]))
            representative_source_name = self.ms.representative_target[0]
            dirrefs = numpy.unique([f.mdirection['refer'] for f in self.ms.fields
                                    if f.source.name == representative_source_name])
            if len(dirrefs) == 0:
                raise RuntimeError('Failed to get direction reference for representative source.')
        else:
            # if representative target is not given, take reference from one of the targets
            dirrefs = numpy.unique([f.mdirection['refer'] for f in self.ms.fields if 'TARGET' in f.intents])
            if len(dirrefs) == 0:
                # no target field exists, something wrong
                raise RuntimeError('No TARGET field exists.')
        if len(dirrefs) == 1:
            outref = dirrefs[0]
        else:
            # direction reference is not unique, search desired ref 
            if 'ICRS' in dirrefs:
                outref = 'ICRS'
            elif 'J2000' in dirrefs:
                outref = 'J2000'
            else:
                # use first one
                outref = dirrefs[0]
        if outref is None:
            raise RuntimeError('Failed to get direction reference for TARGET.')
        
        return outref
        
    @staticmethod
    def _get_azelref():
        return 'AZELGEO'


def direction_convert(direction, mepoch, mposition, outframe):
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

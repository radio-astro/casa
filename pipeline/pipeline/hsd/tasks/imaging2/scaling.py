from __future__ import absolute_import

import os
import csv
import numpy

import pipeline.infrastructure as infrastructure
from pipeline.infrastructure import casatools
from .. import common

LOG = infrastructure.get_logger(__name__)

class IntensityScalingInputs(common.SingleDishInputs):
    """
    Intensity Scaling for each MS 
    """
    def __init__(self, context, infiles=None, reffile=None):
        self._init_properties(vars())
        self._to_list(['infiles'])
        
class IntensityScalingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(IntensityScalingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(IntensityScalingResults, self).merge_with_context(context)
        
    def _outcome_name(self):
        return self.outcome


class IntensityScaling(common.SingleDishTaskTemplate):
    Inputs = IntensityScalingInputs
    
    def prepare(self):
        reffile = self.inputs.reffile
        
        if reffile is None or not os.path.exists(reffile):
            LOG.info('No scaling factors available. Use 1.0 for all antennas.')
        else:
            # do scaling
            # read scaling factor list
            factors_list = read_scaling_factor(reffile)
            
            # generate scaling factor dictionary
            factors = rearrange_factors_list(factors_list)
                        
            # apply scaling factor to the data
            self._apply_scaling_factors(factors)
        self._change_unit()

        result = IntensityScalingResults(task=self.__class__,
                                 success=True,
                                 outcome=None)
        result.task = self.__class__

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result
    
    def analyse(self, result):
        return result
    
    def _apply_scaling_factors(self, factors):
        infiles = self.inputs.infiles
        context = self.inputs.context
        for st in context.observing_run:
            if st.basename in infiles:
                # try to apply scaling factor
                vis = st.ms.basename
                ant = st.antenna.name
                #ms_name = st.exported_ms
                if factors.has_key(vis):
                    if factors[vis].has_key(ant):
                        for (spwid, spw) in st.spectral_window.items():
                            if spw.nchan == 4:
                                # skip WVR
                                continue
                            elif spw.is_target:
                                if factors[vis][ant].has_key(spwid):
                                    factors_per_pol = factors[vis][ant][spwid]
                                    self._do_scaling(st, spwid, factors_per_pol)
                                else:
                                    LOG.info('Scaling factor for (%s,%s,%s) is missing. Use 1.0' % \
                                             (vis,ant,spwid))
                    else:
                        LOG.info('Scaling factor for (%s,%s) is missing. Use 1.0.'%(vis,ant))
                else:
                    LOG.info('Scaling factor for %s is missing. Use 1.0'%(vis))
        
    def _do_scaling(self, st, spwid, factors):
        vis = st.ms.basename
        ms_name = st.exported_ms
        ms_abspath = os.path.join(os.path.dirname(st.name), ms_name)
        stokes_enum = numpy.array(['Undefined',
                         'I',
                         'Q',
                         'U',
                         'V',
                         'RR',
                         'RL',
                         'LR',
                         'LL',
                         'XX',
                         'XY',
                         'YX',
                         'YY',
                         'RX',
                         'RY',
                         'LX',
                         'LY',
                         'XR',
                         'XL',
                         'YR',
                         'YL',
                         'PP',
                         'PQ',
                         'QP',
                         'QQ',
                         'RCircular',
                         'LCircular',
                         'Linear',
                         'Ptotal',
                         'Plinear',
                         'PFtotal',
                         'PFlinear',
                         'Pangle'])
        #LOG.info('Applying %s to (%s,%s)'%(factors, ms_name, spwid))
        with casatools.TableReader(os.path.join(ms_abspath, 'DATA_DESCRIPTION')) as tb:
            spwid_list = tb.getcol('SPECTRAL_WINDOW_ID')
            data_desc_id = list(spwid_list).index(spwid)
            polid = tb.getcell('POLARIZATION_ID', data_desc_id)
            
        with casatools.TableReader(os.path.join(ms_abspath, 'POLARIZATION')) as tb:
            corr_type = map(lambda x: stokes_enum[x], tb.getcell('CORR_TYPE',polid))
            
        with casatools.TableReader(ms_abspath, nomodify=False) as tb:
            LOG.debug('DATA_DESC_ID=%s'%(data_desc_id))
            LOG.debug('CORR_TYPE=%s'%(corr_type))
            factor = []
            for corr in corr_type:
                if factors.has_key(corr):
                    factor.append(factors[corr])
                else:
                    LOG.info('Scaling factor for (%s, %s, %s) is missing. Use 1.0'%(vis,spwid,corr))
                    factor.append(1.0)
            LOG.info('Applying %s to (%s,%s)'%(factor, ms_name, spwid))
            tsel = tb.query('DATA_DESC_ID==%s'%(data_desc_id))
            chunk_size = 1000
            nrows = tsel.nrows()
            num_chunk = nrows / chunk_size + 1 if nrows % chunk_size > 0 else 0 
            startrow = 0
            for ichunk in xrange(num_chunk):
                nrow = min(chunk_size, nrows - startrow)
                sp = tsel.getcol('FLOAT_DATA', startrow, nrow)
                for ipol in xrange(len(factor)):
                    sp[ipol,::] *= factor[ipol]
                tsel.putcol('FLOAT_DATA', sp, startrow, nrow)
                startrow += nrow    
            tsel.close()   
            
    def _change_unit(self):
        infiles = self.inputs.infiles
        context = self.inputs.context
        for st in context.observing_run:
            if st.basename in infiles:
                ms_abspath = os.path.join(os.path.dirname(st.name), st.exported_ms)
                with casatools.TableReader(ms_abspath, nomodify=False) as tb:
                    tb.putcolkeyword('FLOAT_DATA', 'UNIT', 'Jy')

def read_scaling_factor(reffile):
    factor_list = []
    with open(reffile, 'r') as f:
        reader = csv.reader(f)
        # Check if first line is header or not
        line = reader.next()
        if line[0].strip() == 'MS':
            # must be a header
            pass
        elif len(line) == 5:
            # may be a data record
            factor_list.append(line)
        else:
            LOG.error('%s is invalid format'%(reffile))
        for line in reader:
            if len(line) == 0:
                continue
            elif len(line) == 5:
                factor_list.append(line)
            else:
                LOG.error('%s is invalid format'%(reffile))
    return factor_list

def rearrange_factors_list(factors_list):
    """
    Rearrange scaling factor list to dictionary which looks like 
    {'MS': {'Ant': {'spw': {'pol': factor}}}}
    """
    factors = {}
    for (vis, ant, spw, pol, _factor) in factors_list:
        spwid = int(spw)
        factor = float(_factor)
        if factors.has_key(vis):
            if factors[vis].has_key(ant):
                if factors[vis][ant].has_key(spwid):
                    if factors[vis][ant][spwid].has_key(pol):
                        LOG.info('There are duplicate rows in reffile, use %s instead of %s for (%s,%s,%s,%s)'%\
                                 (factors[vis][ant][spwid][pol],factor,vis,ant,spwid,pol))
                        factors[vis][ant][spwid][pol] = factor
                    else:
                        factors[vis][ant][spwid][pol] = factor
                else:
                    factors[vis][ant][spwid] = {pol: factor}
            else:
                factors[vis][ant] = {spwid: {pol: factor}}
        else:
            factors[vis] = {ant: {spwid: {pol: factor}}}

    return factors
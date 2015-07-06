from __future__ import absolute_import

import os
import csv
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casatools
from .. import common
from . import jyperkreader

LOG = infrastructure.get_logger(__name__)

class IntensityScalingInputs(common.SingleDishInputs):
    """
    Intensity Scaling for each MS 
    """
    def __init__(self, context, infiles=None, reffile=None, mustapply=None, spec_unit=None):
        self._init_properties(vars())
        self._to_list(['infiles'])
        
    reffile = basetask.property_with_default('reffile', 'jyperk.csv')
        
class IntensityScalingResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(IntensityScalingResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(IntensityScalingResults, self).merge_with_context(context)
        
    def _outcome_name(self):
        return self.outcome


class IntensityScaling(common.SingleDishTaskTemplate):
    Inputs = IntensityScalingInputs
    
    @common.datatable_setter
    def prepare(self):
        mustapply = self.inputs.mustapply
        logfunc = LOG.error if mustapply else LOG.warn
        any_failed = False
        if self.inputs.reffile is None or not os.path.exists(self.inputs.reffile):
            factors = None
            reffile = self.inputs.reffile
            if mustapply:
                logfunc('No scaling factors available. Use 1.0 for all antennas.')
            any_failed = mustapply
        else:
            # do scaling
            # read scaling factor list
            #factors_list = read_scaling_factor(reffile)
            reffile = os.path.abspath(os.path.expandvars(os.path.expanduser(self.inputs.reffile)))
 
            factors_list = jyperkreader.read(self.inputs.context, reffile)
            LOG.debug('factors_list=%s'%(factors_list))
            
            # generate scaling factor dictionary
            factors = rearrange_factors_list(factors_list)
                        
            if not mustapply: LOG.warn("Applying Jy/K factor to AmpCal sources.")
            # apply scaling factor to the data
            any_failed = self._apply_scaling_factors(factors, mustapply)
        spec_unit = 'Jy' if self.inputs.spec_unit is None else self.inputs.spec_unit
        self._change_unit(spec_unit)

        outcome = {'factors': factors,
                   'reffile': reffile,
                   'must_apply': mustapply,
                   'factormissing': any_failed}
        result = IntensityScalingResults(task=self.__class__,
                                 success=True,
                                 outcome=outcome)
        result.task = self.__class__

        if self.inputs.context.subtask_counter is 0: 
            result.stage_number = self.inputs.context.task_counter - 1
        else:
            result.stage_number = self.inputs.context.task_counter 

        return result
    
    def analyse(self, result):
        return result
    
    def _apply_scaling_factors(self, factors, mustapply):
        infiles = self.inputs.infiles
        context = self.inputs.context
        any_failed = False
        logfunc = LOG.error if mustapply else LOG.warn
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
                            elif spw.is_target and spw.nchan > 1:
                                if factors[vis][ant].has_key(spwid):
                                    factors_per_pol = factors[vis][ant][spwid]
                                    self._do_scaling(st, spwid, factors_per_pol)
                                else:
                                    logfunc('Scaling factor for (%s,%s,%s) is missing. Use 1.0' % \
                                             (vis,ant,spwid))
                                    any_failed = mustapply
                    else:
                        logfunc('Scaling factor for (%s,%s) is missing. Use 1.0.'%(vis,ant))
                        any_failed = mustapply
                else:
                    logfunc('Scaling factor for %s is missing. Use 1.0'%(vis))
                    any_failed = mustapply
        return any_failed

    def _do_scaling(self, st, spwid, factors):
        vis = st.ms.basename
        ms_name = st.exported_ms
        ms_abspath = os.path.join(os.path.dirname(st.name), ms_name)
        antenna = st.antenna.name
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
        
        # factor for Stokes I can be applied to 'XX', 'YY', 'RR', 'LL' if 
        # specific factor for those correlations don't exist
        compatible_corrtype = dict(((k,[k]) for k in stokes_enum))
        for corr in ['XX', 'YY', 'RR', 'LL']:
            compatible_corrtype[corr].append('I')
        
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
                elif len(compatible_corrtype[corr]) > 1:
                    # try factor for another correlation type 
                    compatible_factor_found = False
                    for ccorr in compatible_corrtype[corr][1:]:
                        if factors.has_key(ccorr):
                            factor.append(factors[ccorr])
                            compatible_factor_found = True
                            break
                    if compatible_factor_found == False:
                        LOG.info('Scaling factor for (%s, %s, %s, %s) is missing. Use 1.0'%(vis,antenna,spwid,corr))
                        factor.append(1.0)
                else:
                    LOG.info('Scaling factor for (%s, %s, %s, %s) is missing. Use 1.0'%(vis,antenna,spwid,corr))
                    factor.append(1.0)
            LOG.info('Applying %s to (%s, %s,%s)'%(factor, ms_name, antenna, spwid))
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
            
    def _change_unit(self, spec_unit):
        infiles = self.inputs.infiles
        context = self.inputs.context
        for st in context.observing_run:
            if st.basename in infiles:
                ms_abspath = os.path.join(os.path.dirname(st.name), st.exported_ms)
                with casatools.TableReader(ms_abspath, nomodify=False) as tb:
                    tb.putcolkeyword('FLOAT_DATA', 'UNIT', spec_unit)

def read_scaling_factor(reffile):
    factor_list = []
    with open(reffile, 'r') as f:
        reader = csv.reader(f)
        # Check if first line is header or not
        line = reader.next()
        if line[0].strip().upper() == 'MS':
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

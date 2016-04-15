from __future__ import absolute_import

import os
import types

from pipeline.hif.heuristics import caltable as caltable_heuristic

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary

from . import jyperkreader
from . import worker

LOG = infrastructure.get_logger(__name__)

class SDK2JyCalInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, infiles=None, caltable=None,
                 reffile=None):
        vis=infiles
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        if type(self.vis) is not types.ListType:
            self.vis = [ self.vis ]

    @property
    def caltable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary 
        if type(self.infiles) is types.ListType:
            return self._handle_multiple_vis('caltable')
        
        # Get the name.
        if callable(self._caltable):
            casa_args = self._get_partial_task_args()
            return self._caltable(output_dir=self.output_dir,
                                  stage=self.context.stage,
                                  **casa_args)
        return self._caltable
    
    @caltable.setter
    def caltable(self, value):
        if value is None:
            value = caltable_heuristic.AmpCaltable()
        self._caltable = value

    @property
    def caltype(self):
        return 'amp'

    @property
    def reffile(self):
        return self._reffile
    
    @reffile.setter
    def reffile(self, value):
        if value is None:
            value =  'jyperk.csv'
        self._reffile = value

    # Avoids circular dependency on caltable.
    def _get_partial_task_args(self):
        return {'vis'     : self.vis,
                'caltype' : self.caltype}

class SDK2JyCalResults(basetask.Results):
    def __init__(self, final=[], pool=[], reffile=None, factors={},
                 all_ok=False):
        super(SDK2JyCalResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.error = set()
        self.reffile = reffile
        self.factors=factors
        self.all_ok = all_ok

    def merge_with_context(self, context):
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:            
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):
        # Format the Tsyscal results.
        s = 'SDK2JyCalResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s

class SDK2JyCal(basetask.StandardTaskTemplate):
    Inputs = SDK2JyCalInputs    

    def prepare(self):
        inputs = self.inputs

        if self.inputs.reffile is None or not os.path.exists(self.inputs.reffile):
            LOG.error('No scaling factors available')
            return SDK2JyCalResults(pool=[])
        # read scaling factor list
        reffile = os.path.abspath(os.path.expandvars(os.path.expanduser(inputs.reffile)))
        factors_list = jyperkreader.read(inputs.context, reffile)
        LOG.debug('factors_list=%s'%(factors_list))
        
        # generate scaling factor dictionary
        factors = rearrange_factors_list(factors_list)

        callist = []
        valid_factors = {}
        all_factors_ok = True
        # Loop over MS and generate a caltable per MS
        k2jycal_inputs = worker.SDK2JyCalWorker.Inputs(inputs.context, inputs.output_dir, inputs.vis,
                                                       inputs.caltable, factors)
        k2jycal_task = worker.SDK2JyCalWorker(k2jycal_inputs)
        k2jycal_result = self._executor.execute(k2jycal_task)
        if k2jycal_result.calapp is not None:
            callist.append(k2jycal_result.calapp)
        valid_factors[k2jycal_result.vis] = k2jycal_result.ms_factors
        all_factors_ok &= k2jycal_result.factors_ok

        return SDK2JyCalResults(pool=callist, reffile=reffile,
                                factors=valid_factors, all_ok = all_factors_ok)

    def analyse(self, result):
        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk

        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        return result

def rearrange_factors_list(factors_list):
    """
    Rearrange scaling factor list to dictionary which looks like 
    {'MS': {'spw': {'Ant': {'pol': factor}}}}
    """
    factors = {}
    for (vis, ant, spw, pol, _factor) in factors_list:
        spwid = int(spw)
        factor = float(_factor)
        if factors.has_key(vis):
            if factors[vis].has_key(spwid):
                if factors[vis][spwid].has_key(ant):
                    if factors[vis][spwid][ant].has_key(pol):
                        LOG.info('There are duplicate rows in reffile, use %s instead of %s for (%s,%s,%s,%s)'%\

                                 (factors[vis][spwid][ant][pol],factor,vis,spwid,ant,pol))
                        factors[vis][spwid][ant][pol] = factor
                    else:
                        factors[vis][spwid][ant][pol] = factor
                else:
                    factors[vis][spwid][ant] = {pol: factor}
            else:
                factors[vis][spwid] = {ant: {pol: factor}}
        else:
            factors[vis] = {spwid: {ant: {pol: factor}}}

    return factors


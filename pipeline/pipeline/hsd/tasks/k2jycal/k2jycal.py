from __future__ import absolute_import

import os
import types
from numpy import sqrt

from pipeline.hif.heuristics import caltable as caltable_heuristic

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks

from . import jyperkreader

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
    def ms(self):
        return self._ms

    @ms.setter
    def ms(self, value):
        if value is None:
            value = self.infiles[0]
        self._ms = value

    @property
    def caltable(self):
        # # The value of caltable is ms-dependent, so test for multiple
        # # measurement sets and listify the results if necessary 
        # if type(self.infiles) is types.ListType:
        #     return self._handle_multiple_vis('caltable')
        
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
        return {'vis'     : self.ms,
                'caltype' : self.caltype}

    # Convert to CASA gencal task arguments.
    def to_casa_args(self):
        return {'vis'      : self.ms,
                'caltable' : self.caltable,
                'caltype'  : self.caltype}


class SDK2JyCal(basetask.StandardTaskTemplate):
    Inputs = SDK2JyCalInputs    

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        if self.inputs.reffile is None or not os.path.exists(self.inputs.reffile):
            LOG.error('No scaling factors available')
            return resultobjects.TsyscalResults(pool=[])
        # read scaling factor list
        #factors_list = read_scaling_factor(reffile)
        reffile = os.path.abspath(os.path.expandvars(os.path.expanduser(inputs.reffile)))
        factors_list = jyperkreader.read(inputs.context, reffile)
        LOG.debug('factors_list=%s'%(factors_list))
        
        # generate scaling factor dictionary
        factors = rearrange_factors_list(factors_list)
        polmap = {'XX': 'X', 'YY': 'Y', 'I': ''}

        callist = []
        # Loop over MS and generate a caltable per MS
        for msname in self.inputs.vis:
            if not os.path.exists(msname):
                LOG.error("Could not find MS '%s'" % msname)
                continue
            msname = os.path.basename(msname)
            if msname not in factors.keys():
                LOG.error("%s does not have factors for MS '%s'" % (inputs.reffile, msname))
                continue
            inputs.ms = msname
            
            # make a note of the current inputs state before we start fiddling
            # with it. This origin will be attached to the final CalApplication.
            origin = callibrary.CalAppOrigin(task=SDK2JyCal, 
                                             inputs=inputs.to_casa_args())
            common_params = inputs.to_casa_args()
            ms_factor = factors[msname]
            for spw, spw_factor in ms_factor.items():
                for ant, ant_factor in spw_factor.items():
                    # handle anonymous antenna
                    if ant.upper()=='ANONYMOUS': ant=''
                    # map polarization
                    pol_list = ant_factor.keys()
                    pols=str(',').join(map(polmap.get, pol_list))
                    gain_factor = [ 1./sqrt(ant_factor[pol]) for pol in pol_list ]
                    gencal_job = casa_tasks.gencal(spw=str(spw), antenna=ant, pol=pols,
                                                   parameter=gain_factor,
                                                   **common_params)
                    self._executor.execute(gencal_job)
            # generate callibrary for the caltable
            calto = callibrary.CalTo(vis=common_params['vis'])
            calfrom = callibrary.CalFrom(common_params['caltable'], caltype=inputs.caltype,
                                         gainfield='', spwmap=None, interp='nearest,nearest')
            calapp = callibrary.CalApplication(calto, calfrom, origin)
            callist.append(calapp)


        return SDK2JyCalResults(pool=callist, factors=factors)

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

class SDK2JyCalResults(basetask.Results):
    def __init__(self, final=[], pool=[], factors={}):
        super(SDK2JyCalResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.factors=factors
        self.error = set()

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

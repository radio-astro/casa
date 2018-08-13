from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as caltable_heuristic
from pipeline.infrastructure import task_registry
from . import jyperkreader
from . import worker

LOG = infrastructure.get_logger(__name__)


class SDK2JyCalInputs(vdp.StandardInputs):

    reffile = vdp.VisDependentProperty(default='jyperk.csv')

    @vdp.VisDependentProperty
    def infiles(self):
        return self.vis

    @infiles.convert
    def infiles(self, value):
        self.vis = value
        return value

    @vdp.VisDependentProperty
    def caltable(self):
        """
        Get the caltable argument for these inputs.

        If set to a table-naming heuristic, this should give a sensible name
        considering the current CASA task arguments.
        """
        namer = caltable_heuristic.AmpCaltable()
        # ignore caltable to avoid circular reference
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir,
                               stage=self.context.stage, **casa_args)
    
    def __init__(self, context, output_dir=None, infiles=None, caltable=None,
                 reffile=None):
        super(SDK2JyCalInputs, self).__init__()

        # context and vis/infiles must be set first so that properties that require
        # domain objects can be function
        self.context = context
        self.infiles = infiles
        self.output_dir = output_dir

        # set the properties to the values given as input arguments
        self.caltable = caltable
        self.reffile = reffile


class SDK2JyCalResults(basetask.Results):
    def __init__(self, vis=None, final=[], pool=[], reffile=None, factors={},
                 all_ok=False):
        super(SDK2JyCalResults, self).__init__()

        self.vis = vis
        self.pool = pool[:]
        self.final = final[:]
        self.error = set()
        self.reffile = reffile
        self.factors = factors
        self.all_ok = all_ok

    def merge_with_context(self, context):
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:            
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)
        # merge k2jy factor to context assing the value as an attribute of MS
        for vis, valid_k2jy in self.factors.iteritems():
            msobj = context.observing_run.get_ms(name=vis)
            msobj.k2jy_factor = {}
            for spwid, spw_k2jy in valid_k2jy.iteritems():
                for ant, ant_k2jy in spw_k2jy.iteritems():
                    for pol, pol_k2jy in ant_k2jy.iteritems():
                        msobj.k2jy_factor[(spwid, ant, pol)] = pol_k2jy

    def __repr__(self):
        # Format the Tsyscal results.
        s = 'SDK2JyCalResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s


@task_registry.set_equivalent_casa_task('hsd_k2jycal')
@task_registry.set_casa_commands_comment('The Kelvin to Jy calibration tables are generated.')
class SDK2JyCal(basetask.StandardTaskTemplate):
    Inputs = SDK2JyCalInputs    

    def prepare(self):
        inputs = self.inputs

        if not os.path.exists(self.inputs.reffile):
            LOG.error('No scaling factors available')
            return SDK2JyCalResults(vis=os.path.basename(inputs.vis), pool=[])
        # read scaling factor list
        reffile = os.path.abspath(os.path.expandvars(os.path.expanduser(inputs.reffile)))
        factors_list = jyperkreader.read(inputs.context, reffile)
        LOG.debug('factors_list=%s' % factors_list)
        
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

        return SDK2JyCalResults(vis=k2jycal_result.vis, pool=callist, reffile=reffile,
                                factors=valid_factors, all_ok=all_factors_ok)

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
        if vis in factors:
            if spwid in factors[vis]:
                if ant in factors[vis][spwid]:
                    if pol in factors[vis][spwid][ant]:
                        LOG.info('There are duplicate rows in reffile, use %s instead of %s for (%s,%s,%s,%s)' %
                                 (factors[vis][spwid][ant][pol], factor, vis, spwid, ant, pol))
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

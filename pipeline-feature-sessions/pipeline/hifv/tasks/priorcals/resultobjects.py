from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class PriorcalsResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, gc_result=None, oc_result=None,
                 rq_result=None,  antpos_result=None, antcorrect=None, tecmaps_result=None, sw_result=None):

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

        super(PriorcalsResults, self).__init__()
        
        self.vis = None
        self.pool = pool
        self.final = final
        self.preceding = preceding
        self.error = set()

        self.gc_result = gc_result
        self.oc_result = oc_result
        self.rq_result = rq_result
        self.sw_result = sw_result
        self.antpos_result = antpos_result
        self.antcorrect = antcorrect
        self.tecmaps_result = tecmaps_result
        #print self.antcorrect
        
    def merge_with_context(self, context):
        if self.gc_result:
            try:
                self.gc_result.merge_with_context(context)
                LOG.info("Priorcals:  Merged gain curves cal")
            except:
                LOG.warn("No gain curves table written.")

        if self.oc_result:
            try:
                self.oc_result.merge_with_context(context)
                LOG.info("Priorcals:  Merged Opac cal")
            except:
                LOG.warn("No opacities table written.")

        if self.rq_result:
            try:
                self.rq_result.merge_with_context(context)
                LOG.info("Priorcals:  Requantizer gains")
            except:
                LOG.warn("No rq gains table written.")

        if self.antpos_result:
            try:
                self.antpos_result.merge_with_context(context)
                LOG.info("Priorcals: Antenna positions corrections.")
            except:
                LOG.warn('No antenna position corrections.')

        if self.tecmaps_result:
            try:
                self.tecmaps_result.merge_with_context(context)
                LOG.info("Priorcals: TEC Maps.")
            except:
                LOG.warn('No TEC Maps table written.')

        if self.sw_result:
            try:
                #self.sw_result.merge_with_context(context)
                LOG.info("Priorcals: Switched Power caltable written to disk but not merged with context callibrary")
            except:
                LOG.warn('No Switched Power table written.')
                
        
                
        return        
        #if not self.final:
        #    LOG.error('No results to merge')
        #    return

        #for calapp in self.final:
        #    LOG.debug('Adding calibration to callibrary:\n'
        #              '%s\n%s' % (calapp.calto, calapp.calfrom))
        #    context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):

        # Format the Priorcal results text output.
        s = 'Priorcal Results:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
        return s

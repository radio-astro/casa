from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

LOG = infrastructure.get_logger(__name__)


class PriorcalsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], gc_result=None, oc_result=None, 
        rq_result=None,  antpos_result=None):

        super(PriorcalsResults, self).__init__()
        
        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

        self.gc_result = gc_result
        self.oc_result = oc_result
        self.rq_result = rq_result
        #self.sw_result = sw_result
        self.antpos_result = antpos_result
        
    def merge_with_context(self, context):
        if self.gc_result:
            self.gc_result.merge_with_context(context)
            LOG.info("Priorcals:  Merged gain curves cal")

        if self.oc_result:
            self.oc_result.merge_with_context(context)
            LOG.info("Priorcals:  Merged Opac cal")

        if self.rq_result:
            self.rq_result.merge_with_context(context)
            LOG.info("Priorcals:  Requantizer gains")
            
        #if self.sw_result:
        #    self.sw_result.merge_with_context(context)
        #    LOG.info("Priorcals:  Switched Power gains")

        if self.antpos_result:
            try:
                self.antpos_result.merge_with_context(context)
                LOG.info("Priorcals: Antenna positions corrections")
            except:
                LOG.warn('No antenna position corrections.')
                
        return        
        #if not self.final:
        #    LOG.error('No results to merge')
        #    return

        #for calapp in self.final:
        #    LOG.debug('Adding calibration to callibrary:\n'
        #              '%s\n%s' % (calapp.calto, calapp.calfrom))
        #    context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):

	# Format the Tsyscal results.
        s = 'Priorcal Results:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
	return s

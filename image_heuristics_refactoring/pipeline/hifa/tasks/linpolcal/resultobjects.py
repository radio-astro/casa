from __future__ import absolute_import

import collections
import os.path

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.displays as displays

LOG = infrastructure.get_logger(__name__)


class LinpolcalResult(basetask.Results):
    def __init__(self, final=[], pool=[]):
        super(LinpolcalResult, self).__init__()        
        self.pool = pool[:]
        self.final = final[:]
        self.error = set()

    def merge_with_context(self, context):
        print 'LinpolCalResult.merge_with_context'
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:
            calto = calapp.calto
            LOG.info('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calto, calapp.calfrom))
            context.callibrary.add(calto, calapp.calfrom)
    
    def __repr__(self):
        s = 'LinpolcalResult:\n'
        for calapplication in self.final:
            print calapplication
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
              spw=calapplication.spw, 
              vis=os.path.basename(calapplication.vis),
              name=calapplication.gaintable)
        return s


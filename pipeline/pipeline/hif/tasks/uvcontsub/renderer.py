import collections
import os

import pipeline.infrastructure
#import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

#LOG = logging.get_logger(__name__)

# Reuse this definition
UVcontFitApplication = collections.namedtuple('UVcontFitApplication', 
                                            'ms gaintable solint fitorder intent spw') 


class T2_4MDetailsUVcontFitRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='uvcontfit.mako',
                 description='UV continuum subtraction',
                 always_rerender=False):
        super(T2_4MDetailsUVcontFitRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        applications = []

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)

            applications.extend(self.get_uvtable_applications(context, result, ms))

        ctx.update({
            'applications': applications
        })

    def get_uvtable_applications(self, context, result, ms):
        applications = []
        
        for calapp in result.final:
            solint = result.inputs['solint']

            if solint == 'inf':
                solint = 'Infinite'
            
            # Convert solint=int to a real integration time. 
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed.
            #if solint == 'int':
                #in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                           #for dt in utils.get_intervals(context, calapp)]
                #solint = 'Per integration (%s)' % utils.commafy(in_secs, quotes=False, conjunction='or')
            
            gaintable = os.path.basename(calapp.gaintable)
            spw = ', '.join(calapp.spw.split(','))

            to_intent = ', '.join(calapp.intent.split(','))
            if to_intent == '':
                to_intent = 'ALL'

            fitorder = '%d' % result.inputs['fitorder']
            a = UVcontFitApplication(ms.basename, gaintable, solint, fitorder,
                                   to_intent, spw)
            applications.append(a)

        return applications

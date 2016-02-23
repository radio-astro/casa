import collections
import os

import pipeline.infrastructure
#import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

#LOG = logging.get_logger(__name__)

SpwMaps = collections.namedtuple('SpwMaps', 'ms spwmap')
# Reuse this definition
SpwPhaseupApplication = collections.namedtuple('SpwPhaseupApplication', 
                                            'ms gaintable calmode solint intent spw') 


class T2_4MDetailsSpwPhaseupRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='spwphaseup.mako',
                 description='Spw phase offsets calibration',
                 always_rerender=False):
        super(T2_4MDetailsSpwPhaseupRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        spwmaps = []
        applications = []

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)

            if result.combine_spwmap:
                spwmap = result.combine_spwmap
            else:
                spwmap = result.phaseup_spwmap
            spwmaps.append(SpwMaps(ms.basename, spwmap))

            applications.extend(self.get_gaincal_applications(context, result.phaseup_result, ms))

        ctx.update({
            'spwmaps'     : spwmaps,
            'applications': applications
        })

    def get_gaincal_applications(self, context, result, ms):
        applications = []
        
        calmode_map = {'p':'Phase only',
                       'a':'Amplitude only',
                       'ap':'Phase and amplitude'}
        
        for calapp in result.final:
            solint = calapp.origin.inputs['solint']

            if solint == 'inf':
                solint = 'Infinite'
            
            # Convert solint=int to a real integration time. 
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed.
            if solint == 'int':
                in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                           for dt in utils.get_intervals(context, calapp)]
                solint = 'Per integration (%s)' % utils.commafy(in_secs, quotes=False, conjunction='or')
            
            gaintable = os.path.basename(calapp.gaintable)
            spw = ', '.join(calapp.spw.split(','))

            to_intent = ', '.join(calapp.intent.split(','))
            if to_intent == '':
                to_intent = 'ALL'

            calmode = calapp.origin.inputs['calmode']
            calmode = calmode_map.get(calmode, calmode)
            a = SpwPhaseupApplication(ms.basename, gaintable, solint, calmode,
                                   to_intent, spw)
            applications.append(a)

        return applications


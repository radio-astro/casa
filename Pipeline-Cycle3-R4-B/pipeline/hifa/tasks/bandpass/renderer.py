'''
Created on 11 Sep 2014

@author: sjw
'''
import pipeline.hif.tasks.bandpass.renderer as baserenderer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


class T2_4MDetailsBandpassRenderer(baserenderer.T2_4MDetailsBandpassRenderer):
    """
    T2_4MDetailsBandpassRenderer generates the detailed T2_4M-level plots and
    output specific to the bandpass calibration task.
    """
    def __init__(self, uri='bandpass.mako', 
                 description='Phase-up bandpass calibration',
                 always_rerender=False):
        super(T2_4MDetailsBandpassRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def get_phaseup_applications(self, context, result, ms):
        hm_phaseup = result.inputs.get('hm_phaseup', 'N/A')
        if not hm_phaseup:
            return []
        
        calmode_map = {'p':'Phase only',
                       'a':'Amplitude only',
                       'ap':'Phase and amplitude'}
        
        # identify phaseup from 'preceding' list attached to result
        phaseup_calapps = [] 
        for previous_result in result.preceding:
            for calapp in previous_result:
                l = [cf for cf in calapp.calfrom if cf.caltype == 'gaincal']
                if l and calapp not in phaseup_calapps:
                    phaseup_calapps.append(calapp)
                
        applications = []
        for calapp in phaseup_calapps:
            solint = calapp.origin.inputs['solint']

            if solint == 'inf':
                solint = 'Infinite'
            
            # Convert solint=int to a real integration time. 
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed by the MS.
            if solint == 'int':
                in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                           for dt in utils.get_intervals(context, calapp)]
                solint = 'Per integration (%s)' % utils.commafy(in_secs, 
                                                                quotes=False, 
                                                                conjunction='or')
            
            calmode = calapp.origin.inputs.get('calmode', 'N/A')
            calmode = calmode_map.get(calmode, calmode)
            minblperant = calapp.origin.inputs.get('minblperant', 'N/A')
            minsnr = calapp.origin.inputs.get('minsnr', 'N/A')
            flagged = 'TODO'
            phaseupbw = result.inputs.get('phaseupbw', 'N/A')

            a = baserenderer.PhaseupApplication(ms.basename, calmode, solint,
                                                minblperant, minsnr, flagged,
                                                phaseupbw)
            applications.append(a)

        return applications
